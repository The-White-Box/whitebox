// Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// COM interface smart pointer.

#ifndef WB_BASE_WIN_COM_COM_PTR_H_
#define WB_BASE_WIN_COM_COM_PTR_H_

#include <ObjBase.h>
#include <comip.h>

#include <type_traits>

namespace wb::base::win::com {

/**
 * @brief COM interface concept.
 * @tparam TInterface Interface which should be COM one.
 */
template <typename TInterface>
concept com_interface =
    std::is_abstract_v<TInterface> && std::is_base_of_v<IUnknown, TInterface>;

/**
 * @brief COM smart pointer with automatic IID deducing from TInterface.
 */
template <com_interface TInterface, const IID *TIid = &__uuidof(TInterface)>
class com_ptr : public _com_ptr_t<_com_IIID<TInterface, TIid>> {};

}  // namespace wb::base::win::com

#endif  // !WB_BASE_WIN_COM_COM_PTR_H_
