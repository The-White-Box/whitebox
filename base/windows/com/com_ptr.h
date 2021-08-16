// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// COM interface smart pointer.

#ifndef WB_BASE_WINDOWS_COM_COM_PTR_H_
#define WB_BASE_WINDOWS_COM_COM_PTR_H_

#include <ObjBase.h>
#include <comip.h>

#include <type_traits>

namespace wb::base::windows::com {
/**
 * @brief COM interface concept.
 * @tparam TInterface Interface which should be COM one.
 */
template <typename TInterface>
using com_interface_concept =
    std::enable_if_t<std::is_abstract_v<TInterface> &&
                     std::is_base_of_v<IUnknown, TInterface>>;

/**
 * @brief COM smart pointer with automatic IID deducing from TInterface.
 */
template <typename TInterface, const IID *TIid = &__uuidof(TInterface),
          typename = com_interface_concept<TInterface>>
class com_ptr : public _com_ptr_t<_com_IIID<TInterface, TIid>> {};
}  // namespace wb::base::windows::com

#endif  // !WB_BASE_WINDOWS_COM_COM_PTR_H_
