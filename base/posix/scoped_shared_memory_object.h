// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

#ifndef WB_BASE_POSIX_SCOPED_SHARED_MEMORY_OBJECT_H_
#define WB_BASE_POSIX_SCOPED_SHARED_MEMORY_OBJECT_H_

#include <fcntl.h>  // O_* constants.
#include <sys/mman.h>
#include <sys/stat.h>  // mode constants.

#include <array>
#include <climits>
#include <cstddef>  // byte.

#include "base/macroses.h"
#include "base/deps/g3log/g3log.h"
#include "base/posix/system_error_ext.h"
#include "base/std2/system_error_ext.h"

namespace wb::base::posix {

/**
 * Shared memory object open flags.
 */
enum class ScopedSharedMemoryObjectFlags : int {
  /**
   * Open the object for read access.  A shared memory object opened in this way
   * can only be mmap(2)ed for read (PROT_READ) access.
   */
  kReadonly = O_RDONLY,
  /**
   * Open the object for read-write access.
   */
  kReadWrite = O_RDWR,
  /**
   * Create the shared memory object if it does not exist.  The user and group
   * ownership of the object are taken from the corresponding effective IDs of
   * the calling process, and the object's permission bits are set according to
   * the low-order 9 bits of mode, except that those bits set in the process
   * file mode creation mask (see umask(2)) are cleared for the new object.
   *
   * A set of macro constants which can be used to define mode is listed in
   * open(2). (Symbolic definitions of these constants can be obtained by
   * including <sys/stat.h>.)
   *
   * A new shared memory object initially has zero length - the size of the
   * object can be set using ftruncate(2).  The newly allocated bytes of a
   * shared memory object are automatically initialized to 0.
   */
  kCreate = O_CREAT,
  /**
   * If kCreate was also specified, and a shared memory object with the given
   * name already exists, return an error.  The check for the existence of the
   * object, and its creation if it does not exist, are performed atomically.
   */
  kExclusive = O_EXCL,
  /**
   * If the shared memory object already exists, truncate it to zero bytes.
   */
  kTruncate = O_TRUNC
};

/**
 * Operator|.
 * @param left Left.
 * @param right Right.
 * @return left | right.
 */
[[nodiscard]] constexpr ScopedSharedMemoryObjectFlags operator|(
    ScopedSharedMemoryObjectFlags left,
    ScopedSharedMemoryObjectFlags right) noexcept {
  return static_cast<ScopedSharedMemoryObjectFlags>(
      base::underlying_cast(left) | base::underlying_cast(right));
}

/**
 * Operator&.
 * @param left Left.
 * @param right Right.
 * @return left & right.
 */
[[nodiscard]] constexpr ScopedSharedMemoryObjectFlags operator&(
    ScopedSharedMemoryObjectFlags left,
    ScopedSharedMemoryObjectFlags right) noexcept {
  return static_cast<ScopedSharedMemoryObjectFlags>(
      base::underlying_cast(left) & base::underlying_cast(right));
}

/**
 * Shared access mode flags.
 */
enum class ScopedAccessModeFlags : mode_t {
  /**
   * Owner can read.
   */
  kOwnerRead = S_IRUSR,
  /**
   * Owner can write.
   */
  kOwnerWrite = S_IWUSR,
  /**
   * Owner can execute.
   */
  kOwnerExecute = S_IXUSR,
  /**
   * Owner can read, write & execute.
   */
  kOwnerReadWriteExecute = kOwnerRead | kOwnerWrite | kOwnerExecute,

  /**
   * Owner group can read.
   */
  kGroupRead = S_IRGRP,
  /**
   * Owner group can write.
   */
  kGroupWrite = S_IWGRP,
  /**
   * Owner can group execute.
   */
  kGroupExecute = S_IXGRP,
  /**
   * Owner can group read, write & execute.
   */
  kGroupReadWriteExecute = kGroupRead | kGroupWrite | kGroupExecute,
};

/**
 * Operator|.
 * @param left Left.
 * @param right Right.
 * @return left | right.
 */
[[nodiscard]] constexpr ScopedAccessModeFlags operator|(
    ScopedAccessModeFlags left, ScopedAccessModeFlags right) noexcept {
  return static_cast<ScopedAccessModeFlags>(base::underlying_cast(left) |
                                            base::underlying_cast(right));
}

/**
 * Scoped shared memory object.
 */
class ScopedSharedMemoryObject {
 public:
  /**
   * Native shared memory object type.
   */
  using native_handle_type = int;

  /**
   * Create new shared memory object.
   * @param name Object name.
   * @param open_flags Open flags.
   * @param mode_flags Mode flags.
   * @return Shared memory object.
   */
  [[nodiscard]] static std2::result<ScopedSharedMemoryObject> New(
      std::string &&name, ScopedSharedMemoryObjectFlags open_flags,
      ScopedAccessModeFlags mode_flags) noexcept {
    G3CHECK(name.size() >= 2 && name.size() <= NAME_MAX && name.rfind('/') == 0)
        << "For portable use, a shared memory object should be "
           "identified by a name of the form /somename; that is, a "
           "null-terminated string of up to NAME_MAX (i.e., 255) "
           "characters consisting of an initial slash, followed by one "
           "or more characters, none of which are slashes.";
    G3CHECK((open_flags & ScopedSharedMemoryObjectFlags::kTruncate) !=
            ScopedSharedMemoryObjectFlags::kTruncate)
        << "POSIX leaves the behavior of the combination of O_RDONLY and "
           "O_TRUNC unspecified.";

    const int descriptor{::shm_open(name.c_str(), underlying_cast(open_flags),
                                    underlying_cast(mode_flags))};
    return descriptor >= 0
               ? std2::result<
                     ScopedSharedMemoryObject>{ScopedSharedMemoryObject{
                     std::move(name), descriptor}}
               : std2::result<ScopedSharedMemoryObject>{get_error(descriptor)};
  }

  ScopedSharedMemoryObject(ScopedSharedMemoryObject &&o) noexcept
      : name_{std::move(o.name_)}, descriptor_{o.descriptor_} {
    o.descriptor_ = -1;
  }
  ScopedSharedMemoryObject &operator=(ScopedSharedMemoryObject &&o) noexcept {
    std::swap(name_, o.name_);
    std::swap(descriptor_, o.descriptor_);
    return *this;
  }

  WB_NO_COPY_CTOR_AND_ASSIGNMENT(ScopedSharedMemoryObject);

  ~ScopedSharedMemoryObject() noexcept {
    G3DCHECK((!name_.empty() && descriptor_ >= 0) ||
             (name_.empty() && descriptor_ < 0));

    if (!name_.empty()) {
      const std::error_code rc{get_error(::shm_unlink(name_.c_str()))};
      G3PCHECK_E(!rc, rc) << "Unable to unlink shared memory object '" << name_
                          << "'.";
    }
  }

  /**
   * Get native handle.
   * @return Native handle.
   */
  [[nodiscard]] native_handle_type native_handle() const noexcept {
    return descriptor_;
  }

 private:
  /**
   * Object name.
   */
  std::string name_;
  /**
   * Object descriptor.
   */
  native_handle_type descriptor_;

  WB_ATTRIBUTE_UNUSED_FIELD
      std::array<std::byte, sizeof(char *) - sizeof(descriptor_)>
          pad_;

  /**
   * Create shared memory object.
   * @param name Object name.
   * @param descriptor Object descriptor.
   */
  ScopedSharedMemoryObject(std::string &&name, int descriptor) noexcept
      : name_{std::move(name)}, descriptor_{descriptor} {
    G3DCHECK(!name_.empty());
    G3DCHECK(descriptor >= 0);
  }
};

}  // namespace wb::base::posix

#endif  // !WB_BASE_POSIX_SCOPED_SHARED_MEMORY_OBJECT_H_
