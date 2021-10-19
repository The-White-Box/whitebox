// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// The Windows security model enables you to control access to event, mutex,
// semaphore, and waitable timer objects.

#ifndef WB_BASE_WIN_SECURITY_OS_OBJECTS_ACCESS_RIGHTS_H_
#define WB_BASE_WIN_SECURITY_OS_OBJECTS_ACCESS_RIGHTS_H_

namespace wb::base::win::security {

/**
 * @brief Standard access rights used by all objects.
 */
enum class StandardAccessRightFlag : unsigned long {
  /**
   * @brief Default.
   */
  kNone = 0UL,
  /**
   * @brief Required to delete the object.
   */
  kDelete = 0x0001'0000UL,
  /**
   * @brief Required to read information in the security descriptor for the
   * object, not including the information in the SACL.  To read or write the
   * SACL, you must request the ACCESS_SYSTEM_SECURITY access right.
   */
  kReadControl = 0x0002'0000UL,
  /**
   * @brief The right to use the object for synchronization.  This enables a
   * thread to wait until the object is in the signaled state.
   */
  kSynchronize = 0x0010'0000UL,
  /**
   * @brief Required to modify the DACL in the security descriptor for the
   * object.
   */
  kWriteDacl = 0x0004'0000UL,
  /**
   * @brief Required to change the owner in the security descriptor for the
   * object.
   */
  kWriteOwner = 0x0008'0000UL
};

/**
 * @brief Object access rights.
 * @tparam TAdditionalRightFlag Additional rights to mix in.
 */
template <typename TAdditionalRightFlag>
class AccessRights {
  static_assert(
      std::is_enum_v<TAdditionalRightFlag> &&
          std::is_same_v<unsigned long,
                         std::underlying_type_t<TAdditionalRightFlag>>,
      "TAdditionalRightFlag should be enum of unsigned long values.");

 public:
  /**
   * @brief Creates system object access rights.
   * @param standard_rights Standard access rights.
   * @param additional_rights Additional access rights.
   * @return nothing.
   */
  constexpr AccessRights(StandardAccessRightFlag standard_rights,
                         TAdditionalRightFlag additional_rights) noexcept
      : standard_rights_{standard_rights},
        additional_rights_{additional_rights} {}

  /**
   * @brief Get native object access rights.
   * @return Object access rights.
   */
  constexpr unsigned long Value() const noexcept {
    return underlying_cast(standard_rights_) |
           underlying_cast(additional_rights_);
  }

  constexpr AccessRights(AccessRights &&r) noexcept = default;
  constexpr AccessRights &operator=(AccessRights &&r) noexcept = default;
  constexpr AccessRights(AccessRights &r) noexcept = default;
  constexpr AccessRights &operator=(AccessRights &r) noexcept = default;

 private:
  /**
   * @brief Standard access rights.
   */
  StandardAccessRightFlag standard_rights_;
  /**
   * @brief Additional access rights.
   */
  TAdditionalRightFlag additional_rights_;
};

/**
 * @brief Object-specific access rights for mutex objects.
 */
enum class ScopedMutexAccessRightFlag : unsigned long {
  /**
   * @brief None.
   */
  kNone = 0UL,
  /**
   * @brief All possible access rights for a mutex object.  Use this right only
   * if your application requires access beyond that granted by the standard
   * access rights.  Using this access right increases the possibility that your
   * application must be run by an Administrator.
   */
  kAllAccess = 0x001F'0001UL,
  /**
   * @brief Reserved.
   */
  kModifyState = 0x0000'0001UL
};

/**
 * @brief Mutex access rights.
 */
struct ScopedMutexAccessRights
    : public AccessRights<ScopedMutexAccessRightFlag> {
  constexpr ScopedMutexAccessRights(
      StandardAccessRightFlag standard_rights,
      ScopedMutexAccessRightFlag mutex_rights) noexcept
      : AccessRights<ScopedMutexAccessRightFlag>{standard_rights,
                                                 mutex_rights} {}

  constexpr ScopedMutexAccessRights(ScopedMutexAccessRights &&r) noexcept =
      default;
  constexpr ScopedMutexAccessRights &operator=(
      ScopedMutexAccessRights &&r) noexcept = default;
  constexpr ScopedMutexAccessRights(ScopedMutexAccessRights &r) noexcept =
      default;
  constexpr ScopedMutexAccessRights &operator=(
      ScopedMutexAccessRights &r) noexcept = default;
};

/**
 * @brief Default mutex access rights.
 */
static inline constexpr ScopedMutexAccessRights DefaultMutexAccessRights{
    StandardAccessRightFlag::kNone, ScopedMutexAccessRightFlag::kNone};

}  // namespace wb::base::win::security

#endif  // !WB_BASE_WIN_SECURITY_OS_OBJECTS_ACCESS_RIGHTS_H_