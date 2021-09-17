// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this wb code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Wrapper for shared library.

#ifndef WB_BASE_SCOPED_SHARED_LIBRARY_H_
#define WB_BASE_SCOPED_SHARED_LIBRARY_H_

#include <cstdint>
#include <memory>
#include <string>

#include "base/deps/g3log/g3log.h"
#include "base/std2/system_error_ext.h"
#include "base/std2/type_traits_ext.h"
#include "build/build_config.h"
#include "build/compiler_config.h"

#ifdef WB_OS_POSIX
#include <dlfcn.h>  // dlopen, etc.
#endif

#ifdef WB_OS_WIN
#include "base/win/system_error_ext.h"
using HANDLE = void *;

#ifdef _WIN64
using FARPROC = intptr_t(__stdcall *)();
#endif  // _WIN64

extern "C" WB_ATTRIBUTE_DLL_IMPORT _Ret_maybenull_
    HMODULE __stdcall LoadLibraryExA(_In_ const char *library_name,
                                     _Reserved_ HANDLE file,
                                     _In_ unsigned long flags);
extern "C" WB_ATTRIBUTE_DLL_IMPORT BOOL __stdcall FreeLibrary(
    _In_ HMODULE module);
extern "C" WB_ATTRIBUTE_DLL_IMPORT FARPROC __stdcall GetProcAddress(
    _In_ HMODULE module, _In_ const char *proc_name);
#endif  // WB_OS_WIN

namespace wb::base {
#ifdef WB_OS_WIN
/**
 * @brief DLL module descriptor.
 */
using module_descriptor = HINSTANCE__;
#elif defined(WB_OS_POSIX)
struct alignas(void *) MODULE_ {
  int unused;
  int unused2;
};
/**
 * @brief Shared library descriptor.  Do not use native void* here as it means
 * smb can accidentally delete void* ptr via module deleter which is not we
 * want.
 */
using module_descriptor = MODULE_;
#else  // WB_OS_POSIX
#error Please add shared library descriptor support for your platform.
#endif  // !WB_OS_WIN && !WB_OS_POSIX
}  // namespace wb::base

namespace std {
#ifdef WB_OS_WIN
/**
 * @brief Deleter to unload DLL on end of scope.
 */
template <>
struct default_delete<wb::base::module_descriptor> {
  // Use HMODULE here since module_descriptor is HMODULE.
  void operator()(_In_opt_ wb::base::module_descriptor *module) const noexcept {
    G3CHECK(!module || ::FreeLibrary(module) != 0);
  }
};
#elif defined(WB_OS_POSIX)
/**
 * @brief Deleter to unload shared library on end of scope.
 */
template <>
struct default_delete<wb::base::module_descriptor> {
  void operator()(wb::base::module_descriptor *module) const {
    const int dlclose_error_code{module ? ::dlclose(module) : 0};
    G3DCHECK(dlclose_error_code == 0);
  }
};
#else  // WB_OS_POSIX
#error Please add shared library default_delete support for your platform.
#endif  // !WB_OS_WIN && !WB_OS_POSIX
}  // namespace std

namespace wb::base {
/**
 * @brief Function pointer concept.
 * @tparam TPointer Pointer to check.
 * @tparam R Result type if TPointer is function pointer.
 */
template <typename TPointer, typename R>
using function_pointer_concept =
    std::enable_if_t<std2::is_function_pointer_v<TPointer>, R>;

/**
 * @brief Smart pointer with std::unique_ptr semantic for module lifecycle
 * handling.
 */
class ScopedSharedLibrary : private std::unique_ptr<module_descriptor> {
  // Define std::unique_ptr members to simplify usage.
  using std::unique_ptr<module_descriptor,
                        std::default_delete<module_descriptor>>::unique_ptr;

 public:
  // Check module loaded like this: if (!module) do_smth.
  using std::unique_ptr<module_descriptor,
                        std::default_delete<module_descriptor>>::operator bool;

  ScopedSharedLibrary(ScopedSharedLibrary &&p) = default;
  ScopedSharedLibrary &operator=(ScopedSharedLibrary &&p) = default;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedSharedLibrary);

#ifdef WB_OS_WIN
  /**
   * @brief Loads library with flags and gets (ScopedSharedLibrary, error_code).
   * @param library_name Library path.
   * @param load_flags Library load flags.
   * @return (ScopedSharedLibrary, error_code).
   */
  [[nodiscard]] static std2::result<ScopedSharedLibrary> FromLibraryOnPath(
      _In_ const std::string &library_path, _In_ unsigned load_flags) noexcept {
    const HMODULE library{
        ::LoadLibraryExA(library_path.c_str(), nullptr, load_flags)};
    return library != nullptr ? std2::result<ScopedSharedLibrary>(
                                    std::move(ScopedSharedLibrary{library}))
                              : std2::result<ScopedSharedLibrary>(
                                    windows::GetErrorCode(library));
  }

  /**
   * @brief Gets (address, error_code) of function in loaded library module.
   * @tparam T Parameter.
   * @param function_name Function name.
   * @return (address, error_code).
   */
  template <typename T>
  [[nodiscard]] function_pointer_concept<T, std2::result<T>> GetAddressAs(
      _In_z_ const char *function_name) const noexcept {
    WB_COMPILER_MSVC_BEGIN_WARNING_OVERRIDE_SCOPE()
      // C4191 'reinterpret_cast': unsafe conversion from 'FARPROC' to 'T'
      // Perfectly valid in this case.
      WB_COMPILER_MSVC_DISABLE_WARNING(4191)
      const auto *address =
          reinterpret_cast<T>(::GetProcAddress(get(), function_name));
    WB_COMPILER_MSVC_END_WARNING_OVERRIDE_SCOPE()
    return address != nullptr ? std2::result<T>(address)
                              : std2::result<T>(std2::GetThreadErrorCode());
  }
#elif defined(WB_OS_POSIX)
  /**
   * @brief Loads library with flags and gets (ScopedSharedLibrary, error_code).
   * @param library_path Library path.
   * @param load_flags Library load flags.
   * @return (ScopedSharedLibrary, error_code).
   */
  [[nodiscard]] static std2::result<ScopedSharedLibrary> FromLibraryOnPath(
      const std::string &library_path, int load_flags) noexcept {
    void *library{::dlopen(library_path.c_str(), load_flags)};
    return library != nullptr
               ? std2::result<ScopedSharedLibrary>(
                     ScopedSharedLibrary{reinterpret_cast<MODULE_ *>(library)})
               // Decided to use EINVAL here as likely args invalid or no DLL.
               : std2::result<ScopedSharedLibrary>(
                     std2::GetThreadErrorCode(EINVAL));
  }

  // Gets (address, error_code) of function |function_name| in loaded shared
  // library.
  template <typename T>
  [[nodiscard]] function_pointer_concept<T, std2::result<T>> GetAddressAs(
      const char *function_name) const noexcept {
    const auto address = reinterpret_cast<T>(::dlsym(get(), function_name));
    return address != nullptr ? std2::result<T>(address)
                              : std2::result<T>(std2::GetThreadErrorCode());
  }
#else  // !WB_OS_WIN && !defined(WB_OS_POSIX)
#error Please add shared library support for your platform.
#endif  // WB_OS_WIN
};
}  // namespace wb::base

#endif  // WB_BASE_SCOPED_SHARED_LIBRARY_H_
