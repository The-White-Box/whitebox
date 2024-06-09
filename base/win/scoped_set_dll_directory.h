// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// Adds a directory to the search path used to locate DLLs for the application
// in scope.

#ifndef WB_BASE_WIN_SCOPED_SET_DLL_DIRECTORY_H_
#define WB_BASE_WIN_SCOPED_SET_DLL_DIRECTORY_H_

#include <sal.h>

#include <cerrno>  // EINVAL
#include <string>

#include "base/deps/g3log/g3log.h"
#include "base/macroses.h"
#include "base/std2/system_error_ext.h"
#include "build/compiler_config.h"

extern "C" WB_ATTRIBUTE_DLL_IMPORT int __stdcall SetDllDirectoryA(
    _In_opt_ const char* lpPathName);

namespace wb::base::win {

/**
 * @brief Adds a directory to the search path used to locate DLLs for the
 * application.
 */
class ScopedSetDllDirectory {
 public:
  /**
   * @brief Adds a directory |path_name| to the search path used to locate DLLs
   * for the application.
   * @param path_name The directory to be added to the search path.  If this
   * parameter is an empty string (""), the call removes the current directory
   * from the default DLL search order.  If this parameter is nullptr, the
   * function restores the default search order.
   * @return ScopedSetDllDirectory.
   */
  [[nodiscard]] static std2::result<ScopedSetDllDirectory> New(
      _In_opt_ const char* path_name) noexcept {
    ScopedSetDllDirectory dll_directory{path_name};
    return !dll_directory.errno_code()
               ? std2::result<ScopedSetDllDirectory>{std::move(dll_directory)}
               : std2::result<ScopedSetDllDirectory>{
                     std::unexpect, dll_directory.errno_code()};
  }

  ScopedSetDllDirectory(ScopedSetDllDirectory&& d) noexcept
      : errno_code_{d.errno_code_} {
    d.errno_code_ = std2::posix_last_error_code(EINVAL);
  }
  ScopedSetDllDirectory& operator=(ScopedSetDllDirectory&&) = delete;

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedSetDllDirectory);

  ~ScopedSetDllDirectory() noexcept {
    if (!errno_code()) [[likely]] {
      const auto rc = ApplyDllDirectory(nullptr);
      G3CHECK(!rc);
    }
  }

 private:
  /**
   * @brief Errno of SetDllDirectory.
   */
  std::error_code errno_code_;

  /**
   * @brief Adds |dll_search_directory| path to DLL search path.
   * @param path_name DLL search directory.
   * @return nothing.
   */
  explicit ScopedSetDllDirectory(_In_opt_ const char* path_name) noexcept
      : errno_code_{ApplyDllDirectory(path_name)} {
    G3DCHECK(!errno_code());
  }

  /**
   * @brief Get error code.
   * @return Error code.
   */
  [[nodiscard]] std::error_code errno_code() const noexcept {
    return errno_code_;
  }

  /**
   * @brief Adds a directory |path_name| to the search path used to locate DLLs
   * for the application.
   * @param path_name The directory to be added to the search path.
   * @return Error code.
   */
  [[nodiscard]] static std::error_code ApplyDllDirectory(
      _In_opt_ const char* path_name) noexcept {
    return ::SetDllDirectoryA(path_name) ? std2::ok_code
                                         : std2::system_last_error_code();
  }
};

}  // namespace wb::base::win

#endif  // !WB_BASE_WIN_SCOPED_SET_DLL_DIRECTORY_H_
