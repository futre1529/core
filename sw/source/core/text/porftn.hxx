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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORFTN_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORFTN_HXX

#include "porfld.hxx"

class SwTxtFrm;
class SwTxtFtn;

/*************************************************************************
 *                      class SwFtnPortion
 *************************************************************************/

class SwFtnPortion : public SwFldPortion
{
    SwTxtFtn *pFtn;
    KSHORT nOrigHeight;
    // #i98418#
    bool mbPreferredScriptTypeSet;
    sal_uInt8 mnPreferredScriptType;
public:
    SwFtnPortion( const OUString &rExpand, SwTxtFtn *pFtn,
                  KSHORT nOrig = KSHRT_MAX );
    KSHORT& Orig() { return nOrigHeight; }

    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;
    virtual bool Format( SwTxtFormatInfo &rInf );

    // #i98418#
    void SetPreferredScriptType( sal_uInt8 nPreferredScriptType );

    const SwTxtFtn* GetTxtFtn() const { return pFtn; };
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwFtnNumPortion
 *************************************************************************/

class SwFtnNumPortion : public SwNumberPortion
{
public:
    SwFtnNumPortion( const OUString &rExpand, SwFont *pFntL )
         : SwNumberPortion( rExpand, pFntL, true, false, 0, false )
         { SetWhichPor( POR_FTNNUM ); }

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwQuoVadisPortion
 *************************************************************************/

class SwQuoVadisPortion : public SwFldPortion
{
    OUString   aErgo;
public:
    SwQuoVadisPortion( const OUString &rExp, const OUString& rStr );
    virtual bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;

    void SetNumber( const OUString& rStr ) { aErgo = rStr; }
    const OUString GetQuoTxt() const { return aExpand; }
    const OUString &GetContTxt() const { return aErgo; }

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const OUString &rExpand ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwErgoSumPortion
 *************************************************************************/

class SwErgoSumPortion : public SwFldPortion
{
public:
    SwErgoSumPortion( const OUString &rExp, const OUString& rStr );
    virtual sal_Int32 GetCrsrOfst( const KSHORT nOfst ) const;
    virtual bool Format( SwTxtFormatInfo &rInf );

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const OUString &rExpand ) const;
    OUTPUT_OPERATOR
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
