/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _ACCNOTEXTHYPERLINK_HXX
#define _ACCNOTEXTHYPERLINK_HXX

#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/implbase1.hxx>
#include <fmtinfmt.hxx>
#include <frame.hxx>
#include <layfrm.hxx>

#include "accnotextframe.hxx"

class SwAccessibleNoTextHyperlink :
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleHyperlink >
{
    friend class SwAccessibleNoTextFrame;

    ::rtl::Reference< SwAccessibleNoTextFrame > xFrame;
    const SwFrm *mpFrm;
    sal_uInt16 mnIndex;

    SwFrmFmt *GetFmt()
    {
        return ((SwLayoutFrm*)mpFrm)->GetFmt();
    }
public:

    SwAccessibleNoTextHyperlink( SwAccessibleNoTextFrame *p, const SwFrm* aFrm, sal_uInt16 nIndex = 0xFFFF );

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription(
                sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL
            getAccessibleActionKeyBinding( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleHyperlink
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionAnchor(
                sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionObject(
            sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getStartIndex()
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getEndIndex()
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL isValid(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception);
};

#endif

