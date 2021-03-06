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

#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/wall.hxx>

#include <svtools/stdctrl.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/treelistbox.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svl/eitem.hxx>

#include <unotools/charclass.hxx>
#include <tools/diagnose_ex.h>

#include "formdlgs.hrc"
#include "funcpage.hxx"
#include "formula/formula.hxx"
#include "formula/IFunctionDescription.hxx"
#include "formula/FormulaCompiler.hxx"
#include "formula/token.hxx"
#include "formula/tokenarray.hxx"
#include "formula/formdata.hxx"
#include "formula/formulahelper.hxx"
#include "structpg.hxx"
#include "parawin.hxx"
#include "ModuleHelper.hxx"
#include "ForResId.hrc"
#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <com/sun/star/sheet/FormulaMapGroupSpecialOffset.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <map>

#define TOKEN_OPEN  0
#define TOKEN_CLOSE 1
#define TOKEN_SEP   2
namespace formula
{
    using namespace ::com::sun::star;

    class OFormulaToken : public IFormulaToken
    {
        sal_Int32   m_nParaCount;
        bool        m_bIsFunction;

    public:
        OFormulaToken(bool _bFunction,sal_Int32 _nParaCount) : m_nParaCount(_nParaCount),m_bIsFunction(_bFunction){}

        virtual ~OFormulaToken() {}

        virtual bool isFunction() const { return m_bIsFunction; }
        virtual sal_uInt32 getArgumentCount() const { return m_nParaCount; }
    };


    class FormulaDlg_Impl
    {
    public:
        ::std::pair<RefButton*,RefEdit*>
                        RefInputStartBefore( RefEdit* pEdit, RefButton* pButton );
        void            RefInputStartAfter( RefEdit* pEdit, RefButton* pButton );
        void            RefInputDoneAfter( bool bForced );
        sal_Bool        CalcValue( const OUString& rStrExp, OUString& rStrResult );
        sal_Bool        CalcStruct( const OUString& rStrExp);
        void            UpdateValues();
        void            DeleteArgs();
        sal_Int32       GetFunctionPos(sal_Int32 nPos);
        void            ClearAllParas();

        void            MakeTree(IStructHelper* _pTree,SvTreeListEntry* pParent,FormulaToken* _pToken,long Count);
        void            fillTree(IStructHelper* _pTree);
        void            UpdateTokenArray( const OUString& rStrExp);
        OUString        RepairFormula(const OUString& aFormula);
        void            FillDialog(sal_Bool nFlag=sal_True);
        void            EditNextFunc( sal_Bool bForward, sal_Int32 nFStart=NOT_FOUND );
        void            EditThisFunc(sal_Int32 nFStart);

        void            StoreFormEditData(FormEditData* pEditData);

        void            UpdateArgInput( sal_uInt16 nOffset, sal_uInt16 nInput );
        void            Update();
        void            Update(const OUString& _sExp);

        void            SaveArg( sal_uInt16 nEd );
        void            UpdateSelection();
        void            DoEnter( bool bOk );
        void            FillListboxes();
        void            FillControls(sal_Bool &rbNext, sal_Bool &rbPrev);

        FormulaDlgMode  SetMeText(const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate);
        void            SetMeText(const OUString& _sText);
        bool            CheckMatrix(OUString& aFormula /*IN/OUT*/);

        void            SetEdSelection();

        bool            UpdateParaWin(Selection& _rSelection);
        void            UpdateParaWin(const Selection& _rSelection,const OUString& _sRefStr);

        void            SetData(sal_Int32 nFStart, sal_Int32 nNextFStart, sal_Int32 nNextFEnd, sal_Int32& PrivStart, sal_Int32& PrivEnd);
        void            PreNotify( NotifyEvent& rNEvt );

        RefEdit*        GetCurrRefEdit();

        const FormulaHelper& GetFormulaHelper() const;
        uno::Reference< sheet::XFormulaOpCodeMapper > GetFormulaOpCodeMapper() const;

        DECL_LINK( ModifyHdl, ParaWin* );
        DECL_LINK( FxHdl, ParaWin* );

        DECL_LINK(MatrixHdl, void *);
        DECL_LINK(FormulaHdl, void *);
        DECL_LINK(FormulaCursorHdl, void *);
        DECL_LINK( BtnHdl, PushButton* );
        DECL_LINK( GetEdFocusHdl, ArgInput* );
        DECL_LINK( GetFxFocusHdl, ArgInput* );
        DECL_LINK(DblClkHdl, void *);
        DECL_LINK(FuncSelHdl, void *);
        DECL_LINK(StructSelHdl, void *);
    public:
        mutable uno::Reference< sheet::XFormulaOpCodeMapper>    m_xOpCodeMapper;
        uno::Sequence< sheet::FormulaToken >                    m_aTokenList;
        ::std::auto_ptr<FormulaTokenArray>                      m_pTokenArray;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aSpecialOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pSpecialOpCodesEnd;
        mutable uno::Sequence< sheet::FormulaToken >            m_aSeparatorsOpCodes;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aFunctionOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pFunctionOpCodesEnd;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aUnaryOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pUnaryOpCodesEnd;
        mutable uno::Sequence< sheet::FormulaOpCodeMapEntry >   m_aBinaryOpCodes;
        mutable const sheet::FormulaOpCodeMapEntry*             m_pBinaryOpCodesEnd;
        ::std::vector< ::boost::shared_ptr<OFormulaToken> >     m_aTokens;
        ::std::map<FormulaToken*,sheet::FormulaToken>           m_aTokenMap;
        IFormulaEditorHelper*                                   m_pHelper;
        Dialog*  m_pParent;
        IControlReferenceHandler*  m_pDlg;
        TabControl      aTabCtrl;
        GroupBox        aGEdit;     //! MUST be placed before pParaWin for initializing
        ParaWin*        pParaWin;
        FixedText       aFtHeadLine;
        FixedInfo       aFtFuncName;
        FixedInfo       aFtFuncDesc;

        FixedText       aFtEditName;

        FixedText       aFtResult;
        ValWnd          aWndResult;

        FixedText       aFtFormula;
        EditBox         aMEFormula;

        CheckBox        aBtnMatrix;
        HelpButton      aBtnHelp;
        CancelButton    aBtnCancel;

        PushButton      aBtnBackward;
        PushButton      aBtnForward;
        OKButton        aBtnEnd;

        RefEdit     aEdRef;
        RefButton   aRefBtn;

        FixedText       aFtFormResult;
        ValWnd          aWndFormResult;

        RefEdit*        pTheRefEdit;
        RefButton*      pTheRefButton;
        FuncPage*       pFuncPage;
        StructPage*     pStructPage;
        OUString        aOldFormula;
        sal_Bool        bStructUpdate;
        MultiLineEdit*  pMEdit;
        sal_Bool        bUserMatrixFlag;
        Timer           aTimer;

        const OUString  aTitle1;
        const OUString  aTitle2;
        const OUString  aTxtEnd;
        const OUString  aTxtOk;     // behind aBtnEnd
        FormulaHelper   m_aFormulaHelper;

        OString    m_aEditHelpId;

        OString    aOldHelp;
        OString    aOldUnique;
        OString    aActivWinId;
        sal_Bool            bIsShutDown;

        Font            aFntBold;
        Font            aFntLight;
        sal_uInt16          nEdFocus;
        sal_Bool            bEditFlag;
        const IFunctionDescription* pFuncDesc;
        sal_Int32      nArgs;
        ::std::vector< OUString > m_aArguments;
        Selection       aFuncSel;

        FormulaDlg_Impl(Dialog* pParent
                        , bool _bSupportFunctionResult
                        , bool _bSupportResult
                        , bool _bSupportMatrix
                        ,IFormulaEditorHelper* _pHelper
                        ,const IFunctionManager* _pFunctionMgr
                        ,IControlReferenceHandler* _pDlg);
        ~FormulaDlg_Impl();

    };
FormulaDlg_Impl::FormulaDlg_Impl(Dialog* pParent
                                        , bool _bSupportFunctionResult
                                        , bool _bSupportResult
                                        , bool _bSupportMatrix
                                        ,IFormulaEditorHelper* _pHelper
                                        ,const IFunctionManager* _pFunctionMgr
                                        ,IControlReferenceHandler* _pDlg)
    :
    m_pHelper       (_pHelper),
    m_pParent       (pParent),
    m_pDlg          (_pDlg),
    aTabCtrl        ( pParent, ModuleRes( TC_FUNCTION ) ),
    aGEdit          ( pParent, ModuleRes( GB_EDIT ) ),
    aFtHeadLine     ( pParent, ModuleRes( FT_HEADLINE ) ),
    aFtFuncName     ( pParent, ModuleRes( FT_FUNCNAME ) ),
    aFtFuncDesc     ( pParent, ModuleRes( FT_FUNCDESC ) ),
    aFtEditName     ( pParent, ModuleRes( FT_EDITNAME ) ),
    aFtResult       ( pParent, ModuleRes( FT_RESULT ) ),
    aWndResult      ( pParent, ModuleRes( WND_RESULT ) ),

    aFtFormula      ( pParent, ModuleRes( FT_FORMULA ) ),
    aMEFormula      ( pParent, ModuleRes( ED_FORMULA ) ),
    aBtnMatrix      ( pParent, ModuleRes( BTN_MATRIX ) ),
    aBtnHelp        ( pParent, ModuleRes( BTN_HELP ) ),
    aBtnCancel      ( pParent, ModuleRes( BTN_CANCEL ) ),
    aBtnBackward    ( pParent, ModuleRes( BTN_BACKWARD ) ),
    aBtnForward     ( pParent, ModuleRes( BTN_FORWARD ) ),
    aBtnEnd         ( pParent, ModuleRes( BTN_END ) ),
    aEdRef          ( pParent, _pDlg, &aFtEditName, ModuleRes( ED_REF) ),
    aRefBtn         ( pParent, ModuleRes( RB_REF), &aEdRef, _pDlg ),
    aFtFormResult   ( pParent, ModuleRes( FT_FORMULA_RESULT)),
    aWndFormResult  ( pParent, ModuleRes( WND_FORMULA_RESULT)),
    pTheRefEdit     (NULL),
    pMEdit          (NULL),
    bUserMatrixFlag (sal_False),
    aTitle1         ( ModuleRes( STR_TITLE1 ) ),        // local resource
    aTitle2         ( ModuleRes( STR_TITLE2 ) ),        // local resource
    aTxtEnd         ( ModuleRes( STR_END ) ),           // local resource
    aTxtOk          ( aBtnEnd.GetText() ),
    m_aFormulaHelper(_pFunctionMgr),
    bIsShutDown     (sal_False),
    nEdFocus        (0),
    pFuncDesc       (NULL),
    nArgs           (0)
{
    pParaWin = new ParaWin( pParent,_pDlg, aGEdit.GetPosPixel());
    aGEdit.Hide();
    pParaWin->Hide();
    aFtEditName.Hide();
    aEdRef.Hide();
    aRefBtn.Hide();

    pMEdit = aMEFormula.GetEdit();

    aMEFormula.SetAccessibleName(aFtFormula.GetText());
    pMEdit->SetAccessibleName(aFtFormula.GetText());

    m_aEditHelpId = pMEdit->GetHelpId();
    pMEdit->SetUniqueId( m_aEditHelpId );

    bEditFlag=sal_False;
    bStructUpdate=sal_True;
    Point aPos=aGEdit.GetPosPixel();
    pParaWin->SetPosPixel(aPos);
    pParaWin->SetArgModifiedHdl(LINK( this, FormulaDlg_Impl, ModifyHdl ) );
    pParaWin->SetFxHdl(LINK( this, FormulaDlg_Impl, FxHdl ) );

    pFuncPage= new FuncPage( &aTabCtrl,_pFunctionMgr);
    pStructPage= new StructPage( &aTabCtrl);
    pFuncPage->Hide();
    pStructPage->Hide();
    aTabCtrl.SetTabPage( TP_FUNCTION, pFuncPage);
    aTabCtrl.SetTabPage( TP_STRUCT, pStructPage);

    aOldHelp = pParent->GetHelpId();                // HelpId from resource always for "Page 1"
    aOldUnique = pParent->GetUniqueId();

    aFtResult.Show( _bSupportResult );
    aWndResult.Show( _bSupportResult );

    aFtFormResult.Show( _bSupportFunctionResult );
    aWndFormResult.Show( _bSupportFunctionResult );

    if ( _bSupportMatrix )
        aBtnMatrix.SetClickHdl(LINK( this, FormulaDlg_Impl, MatrixHdl ) );
    else
        aBtnMatrix.Hide();

    aBtnCancel  .SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    aBtnEnd     .SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    aBtnForward .SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );
    aBtnBackward.SetClickHdl( LINK( this, FormulaDlg_Impl, BtnHdl ) );

    pFuncPage->SetDoubleClickHdl( LINK( this, FormulaDlg_Impl, DblClkHdl ) );
    pFuncPage->SetSelectHdl( LINK( this, FormulaDlg_Impl, FuncSelHdl) );
    pStructPage->SetSelectionHdl( LINK( this, FormulaDlg_Impl, StructSelHdl ) );
    pMEdit->SetModifyHdl( LINK( this, FormulaDlg_Impl, FormulaHdl ) );
    aMEFormula.SetSelChangedHdl( LINK( this, FormulaDlg_Impl, FormulaCursorHdl ) );

    aFntLight = aFtFormula.GetFont();
    aFntLight.SetTransparent( true );
    aFntBold = aFntLight;
    aFntBold.SetWeight( WEIGHT_BOLD );

    pParaWin->SetArgumentFonts(aFntBold,aFntLight);

    //  function description for choosing a function is no longer in a different color

    aFtHeadLine.SetFont(aFntBold);
    aFtFuncName.SetFont(aFntLight);
    aFtFuncDesc.SetFont(aFntLight);
}
FormulaDlg_Impl::~FormulaDlg_Impl()
{
    if(aTimer.IsActive())
    {
        aTimer.SetTimeoutHdl(Link());
        aTimer.Stop();
    }// if(aTimer.IsActive())
    bIsShutDown=sal_True;// Set it in order to PreNotify not to save GetFocus.

    aTabCtrl.RemovePage(TP_FUNCTION);
    aTabCtrl.RemovePage(TP_STRUCT);

    delete pStructPage;
    delete pFuncPage;
    delete pParaWin;
    DeleteArgs();
}

void FormulaDlg_Impl::StoreFormEditData(FormEditData* pData)
{
    if (pData) // it won't be destroyed via Close
    {
        pData->SetFStart(pMEdit->GetSelection().Min());
        pData->SetSelection(pMEdit->GetSelection());

        if(aTabCtrl.GetCurPageId()==TP_FUNCTION)
            pData->SetMode( (sal_uInt16) FORMULA_FORMDLG_FORMULA );
        else
            pData->SetMode( (sal_uInt16) FORMULA_FORMDLG_EDIT );
        pData->SetUndoStr(pMEdit->GetText());
        pData->SetMatrixFlag(aBtnMatrix.IsChecked());
    }
}


void FormulaDlg_Impl::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nSwitch=rNEvt.GetType();
    if(nSwitch==EVENT_GETFOCUS && !bIsShutDown)
    {
        Window* pWin=rNEvt.GetWindow();
        if(pWin!=NULL)
        {
            aActivWinId = pWin->GetUniqueId();
            if(aActivWinId.isEmpty())
            {
                Window* pParent=pWin->GetParent();
                while(pParent!=NULL)
                {
                    aActivWinId=pParent->GetUniqueId();

                    if(!aActivWinId.isEmpty()) break;

                    pParent=pParent->GetParent();
                }
            }
            if(!aActivWinId.isEmpty())
            {

                FormEditData* pData = m_pHelper->getFormEditData();

                if (pData && !aTimer.IsActive()) // won't be destroyed via Close
                {
                    pData->SetUniqueId(aActivWinId);
                }
            }
        }
    }
}
uno::Reference< sheet::XFormulaOpCodeMapper > FormulaDlg_Impl::GetFormulaOpCodeMapper() const
{
    if ( !m_xOpCodeMapper.is() )
    {
        m_xOpCodeMapper = m_pHelper->getFormulaOpCodeMapper();
        m_aFunctionOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::FUNCTIONS);
        m_pFunctionOpCodesEnd = m_aFunctionOpCodes.getConstArray() + m_aFunctionOpCodes.getLength();

        m_aUnaryOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::UNARY_OPERATORS);
        m_pUnaryOpCodesEnd = m_aUnaryOpCodes.getConstArray() + m_aUnaryOpCodes.getLength();

        m_aBinaryOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::BINARY_OPERATORS);
        m_pBinaryOpCodesEnd = m_aBinaryOpCodes.getConstArray() + m_aBinaryOpCodes.getLength();

        uno::Sequence< OUString > aArgs(3);
        aArgs[TOKEN_OPEN]   = "(";
        aArgs[TOKEN_CLOSE]  = ")";
        aArgs[TOKEN_SEP]    = ";";
        m_aSeparatorsOpCodes = m_xOpCodeMapper->getMappings(aArgs,sheet::FormulaLanguage::ODFF);

        m_aSpecialOpCodes = m_xOpCodeMapper->getAvailableMappings(sheet::FormulaLanguage::ODFF,sheet::FormulaMapGroup::SPECIAL);
        m_pSpecialOpCodesEnd = m_aSpecialOpCodes.getConstArray() + m_aSpecialOpCodes.getLength();
    } // if ( !m_xOpCodeMapper.is() )
    return m_xOpCodeMapper;
}

void FormulaDlg_Impl::DeleteArgs()
{
    ::std::vector< OUString>().swap(m_aArguments);
    nArgs = 0;
}
namespace
{
    // comparing two property instances
    struct OpCodeCompare : public ::std::binary_function< sheet::FormulaOpCodeMapEntry, sal_Int32 , bool >
    {
        bool operator() (const sheet::FormulaOpCodeMapEntry& x, sal_Int32 y) const
        {
            return x.Token.OpCode == y;
        }
    };
}

sal_Int32 FormulaDlg_Impl::GetFunctionPos(sal_Int32 nPos)
{
    if ( !m_aTokenList.hasElements() )
        return SAL_MAX_INT32;

    const sal_Unicode sep = m_pHelper->getFunctionManager()->getSingleToken(IFunctionManager::eSep);

    sal_Int32 nFuncPos = SAL_MAX_INT32;
    sal_Bool  bFlag = sal_False;
    OUString  aFormString = m_aFormulaHelper.GetCharClass()->uppercase(pMEdit->GetText());

    const uno::Reference< sheet::XFormulaParser > xParser(m_pHelper->getFormulaParser());
    const table::CellAddress aRefPos(m_pHelper->getReferencePosition());

    const sheet::FormulaToken* pIter = m_aTokenList.getConstArray();
    const sheet::FormulaToken* pEnd = pIter + m_aTokenList.getLength();
    try
    {
        sal_Int32 nTokPos = 1;
        sal_Int32 nOldTokPos = 1;
        sal_Int32 nPrevFuncPos = 1;
        short nBracketCount = 0;
        while ( pIter != pEnd )
        {
            const sal_Int32 eOp = pIter->OpCode;
            uno::Sequence<sheet::FormulaToken> aArgs(1);
            aArgs[0] = *pIter;
            const OUString aString = xParser->printFormula(aArgs, aRefPos);
            const sheet::FormulaToken* pNextToken = pIter + 1;

            if( !bUserMatrixFlag && FormulaCompiler::IsMatrixFunction((OpCode)eOp) )
            {
                aBtnMatrix.Check();
            }

            if( eOp == m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::PUSH].Token.OpCode ||
                eOp == m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::SPACES].Token.OpCode )
            {
                const sal_Int32 n1 = nTokPos < 0 ? -1 : aFormString.indexOf(sep, nTokPos);
                const sal_Int32 n2 = nTokPos < 0 ? -1 : aFormString.indexOf(')',nTokPos);
                sal_Int32 nXXX = nTokPos;
                if( n1 < n2 )
                {
                    nTokPos=n1;
                }
                else
                {
                    nTokPos=n2;
                }
                if( pNextToken != pEnd )
                {
                    aArgs[0] = *pNextToken;
                    const OUString a2String = xParser->printFormula(aArgs, aRefPos);
                    const sal_Int32 n3 = aFormString.indexOf(a2String,nXXX);
                    if ( n3 < nTokPos )
                        nTokPos = n3;
                }
            }
            else
            {
                nTokPos = nTokPos + aString.getLength();
            }

            if( eOp == m_aSeparatorsOpCodes[TOKEN_OPEN].OpCode )
            {
                nBracketCount++;
                bFlag = sal_True;
            }
            else if( eOp == m_aSeparatorsOpCodes[TOKEN_CLOSE].OpCode )
            {
                nBracketCount--;
                bFlag = sal_False;
                nFuncPos = nPrevFuncPos;
            }
            bool bIsFunction = ::std::find_if(m_aFunctionOpCodes.getConstArray(),m_pFunctionOpCodesEnd,::std::bind2nd(OpCodeCompare(),boost::cref(eOp))) != m_pFunctionOpCodesEnd;

            if( bIsFunction && m_aSpecialOpCodes[sheet::FormulaMapGroupSpecialOffset::SPACES].Token.OpCode != eOp )
            {
                nPrevFuncPos = nFuncPos;
                nFuncPos = nOldTokPos;
            }

            if( nOldTokPos <= nPos && nPos < nTokPos )
            {
                if( !bIsFunction )
                {
                    if( nBracketCount < 1 )
                    {
                        nFuncPos = pMEdit->GetText().getLength();
                    }
                    else if( !bFlag )
                    {
                        nFuncPos = nPrevFuncPos;
                    }
                }
                break;
            }

            pIter = pNextToken;
            nOldTokPos = nTokPos;
        } // while ( pIter != pEnd )
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL("Exception caught!");
    }

    return nFuncPos;
}

sal_Bool FormulaDlg_Impl::CalcValue( const OUString& rStrExp, OUString& rStrResult )
{
    sal_Bool bResult = sal_True;

    if ( !rStrExp.isEmpty() )
    {
        // Only calculate the value when there isn't any more keyboard input:

        if ( !Application::AnyInput( VCL_INPUT_KEYBOARD ) )
        {
            bResult = m_pHelper->calculateValue(rStrExp,rStrResult);
        }
        else
            bResult = sal_False;
    }

    return bResult;
}

void FormulaDlg_Impl::UpdateValues()
{
    OUString aStrResult;

    if ( CalcValue( pFuncDesc->getFormula( m_aArguments ), aStrResult ) )
        aWndResult.SetValue( aStrResult );

    aStrResult = "";
    if ( CalcValue(m_pHelper->getCurrentFormula(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult = "";
        aWndFormResult.SetValue( aStrResult );
    }
    CalcStruct(pMEdit->GetText());
}

sal_Bool FormulaDlg_Impl::CalcStruct( const OUString& rStrExp)
{
    sal_Bool bResult = sal_True;
    sal_Int32 nLength = rStrExp.getLength();

    if ( !rStrExp.isEmpty() && aOldFormula!=rStrExp && bStructUpdate)
    {
        // Only calculate the value when there isn't any more keyboard input:

        if ( !Application::AnyInput( VCL_INPUT_KEYBOARD ) )
        {
            pStructPage->ClearStruct();

            OUString aString=rStrExp;
            if(rStrExp[nLength-1] == '(')
            {
                aString = aString.copy(0, nLength-1);
            }

            aString = comphelper::string::remove(aString, '\n');
            OUString aStrResult;

            if ( CalcValue(aString, aStrResult ) )
                aWndFormResult.SetValue( aStrResult );

            UpdateTokenArray(aString);
            fillTree(pStructPage);

            aOldFormula = rStrExp;
            if(rStrExp[nLength-1] == '(')
                UpdateTokenArray(rStrExp);
        }
        else
            bResult = sal_False;
    }
    return bResult;
}


void FormulaDlg_Impl::MakeTree(IStructHelper* _pTree,SvTreeListEntry* pParent,FormulaToken* _pToken,long Count)
{
    if( _pToken != NULL && Count > 0 )
    {
        long nParas = _pToken->GetParamCount();
        OpCode eOp = _pToken->GetOpCode();

        // #i101512# for output, the original token is needed
        FormulaToken* pOrigToken = (_pToken->GetType() == svFAP) ? _pToken->GetFAPOrigToken() : _pToken;
        uno::Sequence<sheet::FormulaToken> aArgs(1);
        ::std::map<FormulaToken*,sheet::FormulaToken>::const_iterator itr = m_aTokenMap.find(pOrigToken);
        if (itr == m_aTokenMap.end())
            return;

        aArgs[0] = itr->second;
        try
        {
            const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
            const OUString aResult = m_pHelper->getFormulaParser()->printFormula(aArgs, aRefPos);

            if ( nParas > 0 )
            {
                SvTreeListEntry* pEntry;

                OUString aTest=_pTree->GetEntryText(pParent);

                if(aTest==aResult &&
                    (eOp==ocAdd || eOp==ocMul ||
                     eOp==ocAmpersand))
                {
                    pEntry=pParent;
                }
                else
                {
                    if(eOp==ocBad)
                    {
                        pEntry=_pTree->InsertEntry(aResult,pParent,STRUCT_ERROR,0,_pToken);
                    }
                    else
                    {
                        pEntry=_pTree->InsertEntry(aResult,pParent,STRUCT_FOLDER,0,_pToken);
                    }
                }

                MakeTree(_pTree,pEntry,m_pTokenArray->PrevRPN(),nParas);
                --Count;
                m_pTokenArray->NextRPN();
                MakeTree(_pTree,pParent,m_pTokenArray->PrevRPN(),Count);
            }
            else
            {
                if(eOp==ocBad)
                {
                    _pTree->InsertEntry(aResult,pParent,STRUCT_ERROR,0,_pToken);
                }
                else
                {
                    _pTree->InsertEntry(aResult,pParent,STRUCT_END,0,_pToken);
                }
                --Count;
                MakeTree(_pTree,pParent,m_pTokenArray->PrevRPN(),Count);
            }
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void FormulaDlg_Impl::fillTree(IStructHelper* _pTree)
{
    GetFormulaOpCodeMapper();
    FormulaToken* pToken = m_pTokenArray->LastRPN();

    if( pToken != NULL)
    {
        MakeTree(_pTree,NULL,pToken,1);
    }
}
void FormulaDlg_Impl::UpdateTokenArray( const OUString& rStrExp)
{
    m_aTokenMap.clear();
    m_aTokenList.realloc(0);
    try
    {
        const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
        m_aTokenList = m_pHelper->getFormulaParser()->parseFormula(rStrExp, aRefPos);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    GetFormulaOpCodeMapper(); // just to get it initialized
    m_pTokenArray = m_pHelper->convertToTokenArray(m_aTokenList);
    const sal_Int32 nLen = static_cast<sal_Int32>(m_pTokenArray->GetLen());
    FormulaToken** pTokens = m_pTokenArray->GetArray();
    if ( pTokens && nLen == m_aTokenList.getLength() )
    {
        for (sal_Int32 nPos=0; nPos<nLen; nPos++)
        {
            m_aTokenMap.insert(::std::map<FormulaToken*,sheet::FormulaToken>::value_type(pTokens[nPos],m_aTokenList[nPos]));
        }
    } // if ( pTokens && nLen == m_aTokenList.getLength() )

    FormulaCompiler aCompiler(*m_pTokenArray.get());
    aCompiler.SetCompileForFAP(true);   // #i101512# special handling is needed
    aCompiler.CompileTokenArray();
}

void FormulaDlg_Impl::FillDialog(sal_Bool nFlag)
{
    sal_Bool bNext=sal_True, bPrev=sal_True;
    if(nFlag)
        FillControls(bNext, bPrev);
    FillListboxes();
    if(nFlag)
    {
        aBtnBackward.Enable(bPrev);
        aBtnForward.Enable(bNext);
    }

    OUString aStrResult;

    if ( CalcValue(m_pHelper->getCurrentFormula(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult = "";
        aWndFormResult.SetValue( aStrResult );
    }
}


void FormulaDlg_Impl::FillListboxes()
{
    //  Switch between the "Pages"
    FormEditData* pData = m_pHelper->getFormEditData();
    OUString aNewTitle;
    //  1. Page: select function
    if ( pFuncDesc && pFuncDesc->getCategory() )
    {
        // We'll never have more than int32 max categories so this is safe ...
        if( pFuncPage->GetCategory() != static_cast<sal_Int32>(pFuncDesc->getCategory()->getNumber() + 1) )
            pFuncPage->SetCategory(pFuncDesc->getCategory()->getNumber() + 1);

        sal_Int32 nPos=pFuncPage->GetFuncPos(pFuncDesc);

        pFuncPage->SetFunction(nPos);
    }
    else if ( pData )
    {
        pFuncPage->SetCategory( pData->GetCatSel() );
        pFuncPage->SetFunction( pData->GetFuncSel() );
    }
    FuncSelHdl(NULL);

    m_pHelper->setDispatcherLock( true );// Activate Modal-Mode

    aNewTitle = aTitle1;

    //  HelpId for 1. page is the one from the resource
    m_pParent->SetHelpId( aOldHelp );
    m_pParent->SetUniqueId( aOldUnique );
}

void FormulaDlg_Impl::FillControls(sal_Bool &rbNext, sal_Bool &rbPrev)
{
    //  Switch between the "Pages"
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData )
        return;

    //  2. Page or Edit: show selected function

    sal_Int32  nFStart     = pData->GetFStart();
    OUString   aFormula    = m_pHelper->getCurrentFormula() + " )";
    sal_Int32  nNextFStart = nFStart;
    sal_Int32  nNextFEnd   = 0;

    DeleteArgs();
    const IFunctionDescription* pOldFuncDesc = pFuncDesc;
    sal_Bool bTestFlag = sal_False;

    if ( m_aFormulaHelper.GetNextFunc( aFormula, false,
                                     nNextFStart, &nNextFEnd, &pFuncDesc, &m_aArguments ) )
    {
        bTestFlag = (pOldFuncDesc != pFuncDesc);
        if(bTestFlag)
        {
            aFtHeadLine.Hide();
            aFtFuncName.Hide();
            aFtFuncDesc.Hide();
            pParaWin->SetFunctionDesc(pFuncDesc);
            aFtEditName.SetText( pFuncDesc->getFunctionName() );
            aFtEditName.Show();
            pParaWin->Show();
            const OString aHelpId = pFuncDesc->getHelpId();
            if ( !aHelpId.isEmpty() )
                pMEdit->SetHelpId(aHelpId);
        }

        sal_Int32 nOldStart, nOldEnd;
        m_pHelper->getSelection( nOldStart, nOldEnd );
        if ( nOldStart != nNextFStart || nOldEnd != nNextFEnd )
        {
            m_pHelper->setSelection( nNextFStart, nNextFEnd );
        }
        aFuncSel.Min() = nNextFStart;
        aFuncSel.Max() = nNextFEnd;

        if(!bEditFlag)
            pMEdit->SetText(m_pHelper->getCurrentFormula());
        sal_Int32 PrivStart, PrivEnd;
        m_pHelper->getSelection( PrivStart, PrivEnd);
        if(!bEditFlag)
            pMEdit->SetSelection( Selection(PrivStart, PrivEnd));

        nArgs = pFuncDesc->getSuppressedArgumentCount();
        sal_uInt16 nOffset = pData->GetOffset();
        nEdFocus = pData->GetEdFocus();

        //  Concatenate the Edit's for Focus-Control

        if(bTestFlag)
            pParaWin->SetArgumentOffset(nOffset);
        sal_uInt16 nActiv=0;
        sal_Int32   nArgPos  = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        sal_Int32   nEditPos = pMEdit->GetSelection().Min();
        sal_Bool    bFlag    = sal_False;

        for(sal_uInt16 i=0;i<nArgs;i++)
        {
            sal_Int32 nLength = m_aArguments[i].getLength()+1;
            pParaWin->SetArgument(i,m_aArguments[i]);
            if(nArgPos<=nEditPos && nEditPos<nArgPos+nLength)
            {
                nActiv=i;
                bFlag=sal_True;
            }
            nArgPos = nArgPos + nLength;
        }
        pParaWin->UpdateParas();

        if(bFlag)
        {
            pParaWin->SetActiveLine(nActiv);
        }

        UpdateValues();
    }
    else
    {
        aFtEditName.SetText("");
        pMEdit->SetHelpId( m_aEditHelpId );
    }
        //  Test, ob vorne/hinten noch mehr Funktionen sind

    sal_Int32 nTempStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
    rbNext = m_aFormulaHelper.GetNextFunc( aFormula, false, nTempStart );
    nTempStart = pMEdit->GetSelection().Min();
    pData->SetFStart(nTempStart);
    rbPrev = m_aFormulaHelper.GetNextFunc( aFormula, true, nTempStart );
}


void FormulaDlg_Impl::ClearAllParas()
{
    DeleteArgs();
    pFuncDesc = NULL;
    pParaWin->ClearAll();
    aWndResult.SetValue(OUString());
    aFtFuncName.SetText(OUString());
    FuncSelHdl(NULL);

    if(pFuncPage->IsVisible())
    {
        aFtEditName.Hide();
        pParaWin->Hide();

        aBtnForward.Enable(true); //@new
        aFtHeadLine.Show();
        aFtFuncName.Show();
        aFtFuncDesc.Show();
    }
}
OUString FormulaDlg_Impl::RepairFormula(const OUString& aFormula)
{
    OUString aResult('=');
    try
    {
        UpdateTokenArray(aFormula);

        if ( m_aTokenList.getLength() )
        {
            const table::CellAddress aRefPos(m_pHelper->getReferencePosition());
            const OUString sFormula(m_pHelper->getFormulaParser()->printFormula(m_aTokenList, aRefPos));
            if ( sFormula.isEmpty() || sFormula[0] != '=' )
                aResult += sFormula;
            else
                aResult = sFormula;

        }
    }
    catch(const uno::Exception& )
    {
        OSL_FAIL("Exception caught!");
    }
    return aResult;
}

void FormulaDlg_Impl::DoEnter(bool bOk)
{
    //  Accept input to the document or cancel
    if ( bOk)
    {
        //  remove dummy arguments
        OUString  aInputFormula = m_pHelper->getCurrentFormula();
        OUString  aString = RepairFormula(pMEdit->GetText());
        m_pHelper->setSelection(0, aInputFormula.getLength());
        m_pHelper->setCurrentFormula(aString);
    }

    m_pHelper->switchBack();

    m_pHelper->dispatch(bOk,aBtnMatrix.IsChecked());
    //  Clear data
    m_pHelper->deleteFormData();

    //  Close dialog
    m_pHelper->doClose(bOk);
}


IMPL_LINK( FormulaDlg_Impl, BtnHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnCancel )
    {
        DoEnter(false);                 // closes the Dialog
    }
    else if ( pBtn == &aBtnEnd )
    {
        DoEnter(true);                  // closes the Dialog
    }
    else if ( pBtn == &aBtnForward )
    {
        const IFunctionDescription* pDesc =pFuncPage->GetFuncDesc( pFuncPage->GetFunction() );

        if(pDesc==pFuncDesc || !pFuncPage->IsVisible())
            EditNextFunc( sal_True );
        else
        {
            DblClkHdl(pFuncPage);      //new
            aBtnForward.Enable(false); //new
        }
    }
    else if ( pBtn == &aBtnBackward )
    {
        bEditFlag=sal_False;
        aBtnForward.Enable(true);
        EditNextFunc( sal_False );
        aMEFormula.Invalidate();
        aMEFormula.Update();
    }


    return 0;
}




//                          Functions for 1. Page




// Handler for Listboxes

IMPL_LINK_NOARG(FormulaDlg_Impl, DblClkHdl)
{
    sal_Int32 nFunc = pFuncPage->GetFunction();

    //  ex-UpdateLRUList
    const IFunctionDescription* pDesc = pFuncPage->GetFuncDesc(nFunc);
    m_pHelper->insertEntryToLRUList(pDesc);

    OUString aFuncName = pFuncPage->GetSelFunctionName() + "()";
    m_pHelper->setCurrentFormula(aFuncName);
    pMEdit->ReplaceSelected(aFuncName);

    Selection aSel=pMEdit->GetSelection();
    aSel.Max()=aSel.Max()-1;
    pMEdit->SetSelection(aSel);

    FormulaHdl(pMEdit);

    aSel.Min()=aSel.Max();
    pMEdit->SetSelection(aSel);

    if(nArgs==0)
    {
        BtnHdl(&aBtnBackward);
    }

    pParaWin->SetEdFocus(0);
    aBtnForward.Enable(false); //@New

    return 0;
}



//                          Functions for right Page

void FormulaDlg_Impl::SetData(sal_Int32 nFStart, sal_Int32 nNextFStart, sal_Int32 nNextFEnd, sal_Int32& PrivStart, sal_Int32& PrivEnd)
{
    sal_Int32 nFEnd;

    // Notice and set new selection
    m_pHelper->getSelection( nFStart, nFEnd );
    m_pHelper->setSelection( nNextFStart, nNextFEnd );
    if(!bEditFlag)
        pMEdit->SetText(m_pHelper->getCurrentFormula());


    m_pHelper->getSelection( PrivStart, PrivEnd);
    if(!bEditFlag)
    {
        pMEdit->SetSelection( Selection(PrivStart, PrivEnd));
        aMEFormula.UpdateOldSel();
    }

    FormEditData* pData = m_pHelper->getFormEditData();
    pData->SetFStart( nNextFStart );
    pData->SetOffset( 0 );
    pData->SetEdFocus( 0 );

    FillDialog();
}

void FormulaDlg_Impl::EditThisFunc(sal_Int32 nFStart)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData) return;

    OUString aFormula = m_pHelper->getCurrentFormula();

    if(nFStart==NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    sal_Int32 nNextFStart  = nFStart;
    sal_Int32 nNextFEnd    = 0;

    sal_Bool bFound;

    bFound = m_aFormulaHelper.GetNextFunc( aFormula, false, nNextFStart, &nNextFEnd);
    if ( bFound )
    {
        sal_Int32 PrivStart, PrivEnd;
        SetData(nFStart, nNextFStart, nNextFEnd, PrivStart, PrivEnd);
        m_pHelper->showReference(aFormula.copy(PrivStart, PrivEnd-PrivStart));
    }
    else
    {
        ClearAllParas();
    }
}

void FormulaDlg_Impl::EditNextFunc( sal_Bool bForward, sal_Int32 nFStart )
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData)
        return;

    OUString aFormula = m_pHelper->getCurrentFormula();

    if(nFStart==NOT_FOUND)
    {
        nFStart = pData->GetFStart();
    }
    else
    {
        pData->SetFStart(nFStart);
    }

    sal_Int32 nNextFStart  = 0;
    sal_Int32 nNextFEnd    = 0;

    sal_Bool bFound;
    if ( bForward )
    {
        nNextFStart = m_aFormulaHelper.GetArgStart( aFormula, nFStart, 0 );
        bFound = m_aFormulaHelper.GetNextFunc( aFormula, false, nNextFStart, &nNextFEnd);
    }
    else
    {
        nNextFStart = nFStart;
        bFound = m_aFormulaHelper.GetNextFunc( aFormula, true, nNextFStart, &nNextFEnd);
    }

    if ( bFound )
    {
        sal_Int32 PrivStart, PrivEnd;
        SetData(nFStart, nNextFStart, nNextFEnd, PrivStart, PrivEnd);
    }
}

void FormulaDlg_Impl::SaveArg( sal_uInt16 nEd )
{
    if (nEd<nArgs)
    {
        for(sal_uInt16 i=0; i<=nEd; i++)
        {
            if ( m_aArguments[i].isEmpty() )
                m_aArguments[i] = " ";
        }
        if(!pParaWin->GetArgument(nEd).isEmpty())
            m_aArguments[nEd] = pParaWin->GetArgument(nEd);

        sal_uInt16 nClearPos=nEd+1;
        for(sal_uInt16 i=nEd+1; i<nArgs; i++)
        {
            if( !pParaWin->GetArgument(i).isEmpty() )
            {
                nClearPos=i+1;
            }
        }

        for(sal_uInt16 i=nClearPos; i<nArgs; i++)
        {
            m_aArguments[i] = "";
        }
    }
}

IMPL_LINK( FormulaDlg_Impl, FxHdl, ParaWin*, pPtr )
{
    if(pPtr==pParaWin)
    {
        aBtnForward.Enable(true); //@ In order to be able to input another function.
        aTabCtrl.SetCurPageId(TP_FUNCTION);

        OUString aUndoStr = m_pHelper->getCurrentFormula();       // it will be added before a ";"
        FormEditData* pData = m_pHelper->getFormEditData();
        if (!pData) return 0;

        sal_uInt16 nArgNo = pParaWin->GetActiveLine();
        nEdFocus=nArgNo;

        SaveArg(nArgNo);
        UpdateSelection();

        sal_Int32 nFormulaStrPos = pData->GetFStart();

        OUString aFormula = m_pHelper->getCurrentFormula();
        sal_Int32 n1 = m_aFormulaHelper.GetArgStart( aFormula, nFormulaStrPos, nEdFocus+pData->GetOffset() );

        pData->SetEdFocus( nEdFocus );
        pData->SaveValues();
        pData->SetMode( (sal_uInt16) FORMULA_FORMDLG_FORMULA );
        pData->SetFStart( n1 );
        pData->SetUndoStr( aUndoStr );
        ClearAllParas();

        FillDialog(sal_False);
        pFuncPage->SetFocus(); //There Parawin is not visible anymore
    }
    return 0;
}

IMPL_LINK( FormulaDlg_Impl, ModifyHdl, ParaWin*, pPtr )
{
    if(pPtr==pParaWin)
    {
        SaveArg(pParaWin->GetActiveLine());
        UpdateValues();

        UpdateSelection();
        CalcStruct(pMEdit->GetText());
    }
    return 0;
}

IMPL_LINK_NOARG(FormulaDlg_Impl, FormulaHdl)
{

    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData) return 0;

    bEditFlag=sal_True;
    OUString    aInputFormula=m_pHelper->getCurrentFormula();
    OUString    aString=pMEdit->GetText();

    Selection   aSel  = pMEdit->GetSelection();
    sal_Int32   nTest = 0;

    if(aString.isEmpty()) //in case everything was cleared
    {
        aString += "=";
        pMEdit->SetText(aString);
        aSel .Min() = 1;
        aSel .Max() = 1;
        pMEdit->SetSelection(aSel);
    }
    else if(aString[nTest]!='=') //in case it's replaced;
    {
        aString = "=" + aString;
        pMEdit->SetText(aString);
        aSel .Min() += 1;
        aSel .Max() += 1;
        pMEdit->SetSelection(aSel);
    }


    m_pHelper->setSelection(0, aInputFormula.getLength());
    m_pHelper->setCurrentFormula(aString);
    m_pHelper->setSelection(aSel.Min(), aSel.Max());

    sal_Int32 nPos = aSel.Min()-1;

    OUString aStrResult;

    if ( CalcValue(m_pHelper->getCurrentFormula(), aStrResult ) )
        aWndFormResult.SetValue( aStrResult );
    else
    {
        aStrResult = "";
        aWndFormResult.SetValue( aStrResult );
    }
    CalcStruct(aString);

    nPos=GetFunctionPos(nPos);

    if(nPos<aSel.Min()-1)
    {
        sal_Int32 nPos1 = aString.indexOf('(',nPos);
        EditNextFunc( sal_False, nPos1);
    }
    else
    {
        ClearAllParas();
    }

    m_pHelper->setSelection(aSel.Min(), aSel.Max());
    bEditFlag=sal_False;
    return 0;
}

IMPL_LINK_NOARG(FormulaDlg_Impl, FormulaCursorHdl)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    if (!pData) return 0;
    sal_Int32 nFStart = pData->GetFStart();

    bEditFlag=sal_True;

    OUString    aString=pMEdit->GetText();

    Selection   aSel =pMEdit->GetSelection();
    m_pHelper->setSelection(aSel.Min(), aSel.Max());

    if(aSel.Min()==0)
    {
        aSel.Min()=1;
        pMEdit->SetSelection(aSel);
    }

    if(aSel.Min() != aString.getLength())
    {
        sal_Int32 nPos = aSel.Min();

        nFStart=GetFunctionPos(nPos - 1);

        if(nFStart<nPos)
        {
            sal_Int32 nPos1 = m_aFormulaHelper.GetFunctionEnd(aString,nFStart);

            if(nPos1>nPos)
            {
                EditThisFunc(nFStart);
            }
            else
            {
                sal_Int32 n = nPos;
                short nCount=1;
                while(n>0)
                {
                   if(aString[n]==')')
                       nCount++;
                   else if(aString[n]=='(')
                       nCount--;
                   if(nCount==0) break;
                   n--;
                }
                if(nCount==0)
                {
                    nFStart=m_aFormulaHelper.GetFunctionStart(aString, n, true);
                    EditThisFunc(nFStart);
                }
                else
                {
                    ClearAllParas();
                }
            }
        }
        else
        {
            ClearAllParas();
        }
    }
    m_pHelper->setSelection(aSel.Min(), aSel.Max());

    bEditFlag=sal_False;
    return 0;
}

void FormulaDlg_Impl::UpdateSelection()
{
    m_pHelper->setSelection(aFuncSel.Min(), aFuncSel.Max());
    m_pHelper->setCurrentFormula( pFuncDesc->getFormula( m_aArguments ) );
    pMEdit->SetText(m_pHelper->getCurrentFormula());
    sal_Int32 PrivStart, PrivEnd;
    m_pHelper->getSelection( PrivStart, PrivEnd);
    aFuncSel.Min() = PrivStart;
    aFuncSel.Max() = PrivEnd;

    nArgs = pFuncDesc->getSuppressedArgumentCount();

    OUString aFormula=pMEdit->GetText();
    sal_Int32 nArgPos=m_aFormulaHelper.GetArgStart( aFormula,PrivStart,0);

    sal_uInt16 nPos=pParaWin->GetActiveLine();

    for(sal_uInt16 i=0;i<nPos;i++)
    {
        nArgPos += (m_aArguments[i].getLength() + 1);
    }
    sal_Int32 nLength= m_aArguments[nPos].getLength();

    Selection aSel(nArgPos,nArgPos+nLength);
    m_pHelper->setSelection((sal_uInt16)nArgPos,(sal_uInt16)(nArgPos+nLength));
    pMEdit->SetSelection(aSel);
    aMEFormula.UpdateOldSel();
}
::std::pair<RefButton*,RefEdit*> FormulaDlg_Impl::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    aEdRef.Show();
    pTheRefEdit = pEdit;
    pTheRefButton = pButton;

    if( pTheRefEdit )
    {
        aEdRef.SetRefString( pTheRefEdit->GetText() );
        aEdRef.SetSelection( pTheRefEdit->GetSelection() );
        aEdRef.SetHelpId( pTheRefEdit->GetHelpId() );
        aEdRef.SetUniqueId( pTheRefEdit->GetUniqueId() );
    }

    aRefBtn.Show( pButton != NULL );

    ::std::pair<RefButton*,RefEdit*> aPair;
    aPair.first = pButton ? &aRefBtn : NULL;
    aPair.second = &aEdRef;
    return aPair;
}
void FormulaDlg_Impl::RefInputStartAfter( RefEdit* /*pEdit*/, RefButton* /*pButton*/ )
{
    aRefBtn.SetEndImage();

    if( pTheRefEdit )
    {
        OUString aStr = aTitle2 + " " + aFtEditName.GetText() + "( ";

        if( pParaWin->GetActiveLine() > 0 )
            aStr += "...; ";
        aStr += pParaWin->GetActiveArgName();
        if( pParaWin->GetActiveLine() + 1 < nArgs )
            aStr += "; ...";
        aStr += " )";

        m_pParent->SetText( MnemonicGenerator::EraseAllMnemonicChars( aStr ) );
    }
}
void FormulaDlg_Impl::RefInputDoneAfter( bool bForced )
{
    aRefBtn.SetStartImage();
    if( bForced || !aRefBtn.IsVisible() )
    {
        aEdRef.Hide();
        aRefBtn.Hide();
        if( pTheRefEdit )
        {
            pTheRefEdit->SetRefString( aEdRef.GetText() );
            pTheRefEdit->GrabFocus();

            if( pTheRefButton )
                pTheRefButton->SetStartImage();

            sal_uInt16 nPrivActiv = pParaWin->GetActiveLine();
            pParaWin->SetArgument( nPrivActiv, aEdRef.GetText() );
            ModifyHdl( pParaWin );
            pTheRefEdit = NULL;
        }
        m_pParent->SetText( aTitle1 );
    }
}
RefEdit* FormulaDlg_Impl::GetCurrRefEdit()
{
    return aEdRef.IsVisible() ? &aEdRef : pParaWin->GetActiveEdit();
}
void FormulaDlg_Impl::Update()
{
    FormEditData* pData = m_pHelper->getFormEditData();
    const OUString sExpression = pMEdit->GetText();
    aOldFormula = "";
    UpdateTokenArray(sExpression);
    FormulaCursorHdl(&aMEFormula);
    CalcStruct(sExpression);
    if(pData->GetMode() == FORMULA_FORMDLG_FORMULA)
        aTabCtrl.SetCurPageId(TP_FUNCTION);
    else
        aTabCtrl.SetCurPageId(TP_STRUCT);
    aBtnMatrix.Check(pData->GetMatrixFlag());
}
void FormulaDlg_Impl::Update(const OUString& _sExp)
{
    CalcStruct(_sExp);
    FillDialog();
    FuncSelHdl(NULL);
}
void FormulaDlg_Impl::SetMeText(const OUString& _sText)
{
    FormEditData* pData = m_pHelper->getFormEditData();
    pMEdit->SetText(_sText);
    pMEdit->SetSelection( pData->GetSelection());
    aMEFormula.UpdateOldSel();
}
FormulaDlgMode FormulaDlg_Impl::SetMeText(const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate)
{
    FormulaDlgMode eMode = FORMULA_FORMDLG_FORMULA;
    if(!bEditFlag)
        pMEdit->SetText(_sText);

    if ( _bSelect || !bEditFlag )
        pMEdit->SetSelection( Selection(PrivStart, PrivEnd));
    if ( _bUpdate )
    {
        aMEFormula.UpdateOldSel();
        pMEdit->Invalidate();
        m_pHelper->showReference(pMEdit->GetSelected());
        eMode = FORMULA_FORMDLG_EDIT;

        aBtnMatrix.Check( bMatrix );
    } // if ( _bUpdate )
    return eMode;
}
bool FormulaDlg_Impl::CheckMatrix(OUString& aFormula)
{
    pMEdit->GrabFocus();
    sal_Int32 nLen = aFormula.getLength();
    bool bMatrix =  nLen > 3                    // Matrix-Formula
            && aFormula[0] == '{'
            && aFormula[1] == '='
            && aFormula[nLen-1] == '}';
    if ( bMatrix )
    {
        aFormula = aFormula.copy( 1, aFormula.getLength()-2 );
        aBtnMatrix.Check( bMatrix );
        aBtnMatrix.Disable();
    } // if ( bMatrix )

    aTabCtrl.SetCurPageId(TP_STRUCT);
    return bMatrix;
}
IMPL_LINK_NOARG(FormulaDlg_Impl, StructSelHdl)
{
    bStructUpdate=sal_False;
    if(pStructPage->IsVisible())    aBtnForward.Enable(false); //@New

    bStructUpdate=sal_True;
    return 0;
}
IMPL_LINK_NOARG(FormulaDlg_Impl, MatrixHdl)
{
    bUserMatrixFlag=sal_True;
    return 0;
}

IMPL_LINK_NOARG(FormulaDlg_Impl, FuncSelHdl)
{
    sal_Int32 nCat = pFuncPage->GetCategory();
    if ( nCat == LISTBOX_ENTRY_NOTFOUND ) nCat = 0;
    sal_Int32 nFunc = pFuncPage->GetFunction();
    if ( nFunc == LISTBOX_ENTRY_NOTFOUND ) nFunc = 0;

    if (   (pFuncPage->GetFunctionEntryCount() > 0)
        && (pFuncPage->GetFunction() != LISTBOX_ENTRY_NOTFOUND) )
    {
        const IFunctionDescription* pDesc =pFuncPage->GetFuncDesc( pFuncPage->GetFunction() );

        if(pDesc!=pFuncDesc) aBtnForward.Enable(true); //new

        if (pDesc)
        {
            pDesc->initArgumentInfo();      // full argument info is needed

            OUString aSig = pDesc->getSignature();
            aFtHeadLine.SetText( pDesc->getFunctionName() );
            aFtFuncName.SetText( aSig );
            aFtFuncDesc.SetText( pDesc->getDescription() );
        }
    }
    else
    {
        aFtHeadLine.SetText( OUString() );
        aFtFuncName.SetText( OUString() );
        aFtFuncDesc.SetText( OUString() );
    }
    return 0;
}

void FormulaDlg_Impl::UpdateParaWin(const Selection& _rSelection, const OUString& _sRefStr)
{
    Selection theSel = _rSelection;
    aEdRef.ReplaceSelected( _sRefStr );
    theSel.Max() = theSel.Min() + _sRefStr.getLength();
    aEdRef.SetSelection( theSel );


    // Manual Update of the results' fields:

    sal_uInt16 nPrivActiv = pParaWin->GetActiveLine();
    pParaWin->SetArgument(nPrivActiv,aEdRef.GetText());
    pParaWin->UpdateParas();

    Edit* pEd = GetCurrRefEdit();
    if( pEd != NULL )
        pEd->SetSelection( theSel );

    pParaWin->SetRefMode(sal_False);
}
bool FormulaDlg_Impl::UpdateParaWin(Selection& _rSelection)
{
    pParaWin->SetRefMode(sal_True);

    OUString      aStrEd;
    Edit* pEd = GetCurrRefEdit();
    if(pEd!=NULL && pTheRefEdit==NULL)
    {
        _rSelection=pEd->GetSelection();
        _rSelection.Justify();
        aStrEd=pEd->GetText();
        aEdRef.SetRefString(aStrEd);
        aEdRef.SetSelection( _rSelection );
    }
    else
    {
        _rSelection=aEdRef.GetSelection();
        _rSelection.Justify();
        aStrEd= aEdRef.GetText();
    }
    return pTheRefEdit == NULL;
}

void FormulaDlg_Impl::SetEdSelection()
{
    Edit* pEd = GetCurrRefEdit()/*aScParaWin.GetActiveEdit()*/;
    if( pEd )
    {
        Selection theSel = aEdRef.GetSelection();
        //  Edit may have the focus -> call ModifyHdl in addition
        //  to what's happening in GetFocus
        pEd->GetModifyHdl().Call(pEd);
        pEd->GrabFocus();
        pEd->SetSelection(theSel);
    } // if( pEd )
}

const FormulaHelper& FormulaDlg_Impl::GetFormulaHelper()  const
{
    return m_aFormulaHelper;
}

FormulaModalDialog::FormulaModalDialog( Window* pParent
                                            , bool _bSupportFunctionResult
                                            , bool _bSupportResult
                                            , bool _bSupportMatrix
                                            , IFunctionManager* _pFunctionMgr
                                            , IControlReferenceHandler* _pDlg ) :
        ModalDialog( pParent, ModuleRes(RID_FORMULADLG_FORMULA_MODAL) ),
        m_pImpl( new FormulaDlg_Impl(this,_bSupportFunctionResult
                                            , _bSupportResult
                                            , _bSupportMatrix
                                            ,this,_pFunctionMgr,_pDlg))
{
    FreeResource();
    SetText(m_pImpl->aTitle1);
}
FormulaModalDialog::~FormulaModalDialog()
{
}

void FormulaModalDialog::Update(const OUString& _sExp)
{
    m_pImpl->Update(_sExp);
}


void FormulaModalDialog::SetMeText(const OUString& _sText)
{
    m_pImpl->SetMeText(_sText);
}


bool FormulaModalDialog::CheckMatrix(OUString& aFormula)
{
    return m_pImpl->CheckMatrix(aFormula);
}

void FormulaModalDialog::Update()
{
    m_pImpl->Update();
}
::std::pair<RefButton*,RefEdit*> FormulaModalDialog::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    return m_pImpl->RefInputStartBefore( pEdit, pButton );
}
void FormulaModalDialog::RefInputStartAfter( RefEdit* pEdit, RefButton* pButton )
{
    m_pImpl->RefInputStartAfter( pEdit, pButton );
}
void FormulaModalDialog::RefInputDoneAfter( bool bForced )
{
    m_pImpl->RefInputDoneAfter( bForced );
}

void FormulaModalDialog::SetFocusWin(Window *pWin,const OString& nUniqueId)
{
    if(pWin->GetUniqueId()==nUniqueId)
    {
        pWin->GrabFocus();
    }
    else
    {
        sal_uInt16 nCount=pWin->GetChildCount();

        for(sal_uInt16 i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            SetFocusWin(pChild,nUniqueId);
        }
    }
}

bool FormulaModalDialog::PreNotify( NotifyEvent& rNEvt )
{
    m_pImpl->PreNotify( rNEvt );

    return ModalDialog::PreNotify(rNEvt);
}

void FormulaModalDialog::StoreFormEditData(FormEditData* pData)
{
    m_pImpl->StoreFormEditData(pData);
}


//      Initialisation / General functions  for Dialog

FormulaDlg::FormulaDlg( SfxBindings* pB, SfxChildWindow* pCW,
                             Window* pParent
                            , bool _bSupportFunctionResult
                            , bool _bSupportResult
                            , bool _bSupportMatrix
                            , IFunctionManager* _pFunctionMgr, IControlReferenceHandler* _pDlg ) :
        SfxModelessDialog( pB, pCW, pParent, ModuleRes(RID_FORMULADLG_FORMULA) ),
        m_pImpl( new FormulaDlg_Impl(this, _bSupportFunctionResult
                                            , _bSupportResult
                                            , _bSupportMatrix
                                            , this, _pFunctionMgr, _pDlg))
{
    FreeResource();
    //undo SfxModelessDialog HelpId clear hack
    reverseUniqueHelpIdHack(*this);
    SetText(m_pImpl->aTitle1);
}

FormulaDlg::~FormulaDlg()
{
}

void FormulaDlg::Update(const OUString& _sExp)
{
    m_pImpl->Update(_sExp);
}


void FormulaDlg::SetMeText(const OUString& _sText)
{
    m_pImpl->SetMeText(_sText);
}


FormulaDlgMode FormulaDlg::SetMeText(const OUString& _sText, sal_Int32 PrivStart, sal_Int32 PrivEnd, bool bMatrix, bool _bSelect, bool _bUpdate)
{
    return m_pImpl->SetMeText(_sText,PrivStart, PrivEnd,bMatrix,_bSelect,_bUpdate);
}

bool FormulaDlg::CheckMatrix(OUString& aFormula)
{
    return m_pImpl->CheckMatrix(aFormula);
}

OUString FormulaDlg::GetMeText() const
{
    return m_pImpl->pMEdit->GetText();
}

void FormulaDlg::Update()
{
    m_pImpl->Update();
    m_pImpl->aTimer.SetTimeout(200);
    m_pImpl->aTimer.SetTimeoutHdl(LINK( this, FormulaDlg, UpdateFocusHdl));
    m_pImpl->aTimer.Start();
}

void FormulaDlg::DoEnter(bool _bOk)
{
    m_pImpl->DoEnter(_bOk);
}
::std::pair<RefButton*,RefEdit*> FormulaDlg::RefInputStartBefore( RefEdit* pEdit, RefButton* pButton )
{
    return m_pImpl->RefInputStartBefore( pEdit, pButton );
}
void FormulaDlg::RefInputStartAfter( RefEdit* pEdit, RefButton* pButton )
{
    m_pImpl->RefInputStartAfter( pEdit, pButton );
}
void FormulaDlg::RefInputDoneAfter( bool bForced )
{
    m_pImpl->RefInputDoneAfter( bForced );
}

void FormulaDlg::SetFocusWin(Window *pWin,const OString& nUniqueId)
{
    if(pWin->GetUniqueId()==nUniqueId)
    {
        pWin->GrabFocus();
    }
    else
    {
        sal_uInt16 nCount=pWin->GetChildCount();

        for(sal_uInt16 i=0;i<nCount;i++)
        {
            Window* pChild=pWin->GetChild(i);
            SetFocusWin(pChild,nUniqueId);
        }
    }
}


bool FormulaDlg::PreNotify( NotifyEvent& rNEvt )
{
    m_pImpl->PreNotify( rNEvt );
    return SfxModelessDialog::PreNotify(rNEvt);
}

void FormulaDlg::disableOk()
{
    m_pImpl->aBtnEnd.Disable();
}

void FormulaDlg::StoreFormEditData(FormEditData* pData)
{
    m_pImpl->StoreFormEditData(pData);
}


const IFunctionDescription* FormulaDlg::getCurrentFunctionDescription() const
{
    OSL_VERIFY(!m_pImpl->pFuncDesc || m_pImpl->pFuncDesc->getSuppressedArgumentCount() == m_pImpl->nArgs);
    return m_pImpl->pFuncDesc;
}

void FormulaDlg::UpdateParaWin(const Selection& _rSelection,const OUString& _sRefStr)
{
    m_pImpl->UpdateParaWin(_rSelection,_sRefStr);
}
bool FormulaDlg::UpdateParaWin(Selection& _rSelection)
{
    return m_pImpl->UpdateParaWin(_rSelection);
}

RefEdit*    FormulaDlg::GetActiveEdit()
{
    return m_pImpl->pParaWin->GetActiveEdit();
}

const FormulaHelper& FormulaDlg::GetFormulaHelper() const
{
    return m_pImpl->GetFormulaHelper();
}

void FormulaDlg::SetEdSelection()
{
    m_pImpl->SetEdSelection();
}
IMPL_LINK_NOARG(FormulaDlg, UpdateFocusHdl)
{
    FormEditData* pData = m_pImpl->m_pHelper->getFormEditData();

    if (pData) // won't be destroyed via Close
    {
        m_pImpl->m_pHelper->setReferenceInput(pData);
        OString nUniqueId(pData->GetUniqueId());
        SetFocusWin(this,nUniqueId);
    }
    return 0;
}


void FormEditData::SaveValues()
{
    FormEditData* pTemp = new FormEditData(*this);

    Reset();
    pParent = pTemp;
}

void FormEditData::Reset()
{
    pParent = NULL;
    nMode = 0;
    nFStart = 0;
    nCatSel = 1;        //! oder 0 (zuletzt benutzte)
    nFuncSel = 0;
    nOffset = 0;
    nEdFocus = 0;
    bMatrix = false;
    aUniqueId=OString();
    aSelection.Min()=0;
    aSelection.Max()=0;
    aUndoStr = "";
}

const FormEditData& FormEditData::operator=( const FormEditData& r )
{
    pParent         = r.pParent;
    nMode           = r.nMode;
    nFStart         = r.nFStart;
    nCatSel         = r.nCatSel;
    nFuncSel        = r.nFuncSel;
    nOffset         = r.nOffset;
    nEdFocus        = r.nEdFocus;
    aUndoStr        = r.aUndoStr;
    bMatrix         = r.bMatrix ;
    aUniqueId       = r.aUniqueId;
    aSelection      = r.aSelection;
    return *this;
}

FormEditData::FormEditData()
{
    Reset();
}

FormEditData::~FormEditData()
{
    delete pParent;
}

FormEditData::FormEditData( const FormEditData& r )
{
    *this = r;
}


} // formula


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
