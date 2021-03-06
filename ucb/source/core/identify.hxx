/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _IDENTIFY_HXX
#define _IDENTIFY_HXX

#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase1.hxx>


class ContentIdentifier : public cppu::WeakImplHelper1 <
    css::ucb::XContentIdentifier >
{
public:
    ContentIdentifier( const OUString& ContentId );
    virtual ~ContentIdentifier();

    // XContentIdentifier
    virtual OUString SAL_CALL getContentIdentifier()
        throw( css::uno::RuntimeException, std::exception );
    virtual OUString SAL_CALL getContentProviderScheme()
        throw( css::uno::RuntimeException, std::exception );

private:
    OUString m_aContentId;
    OUString m_aProviderScheme;
};

#endif /* !_IDENTIFY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
