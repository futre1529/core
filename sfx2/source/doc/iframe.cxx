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

#include <sal/config.h>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <cppuhelper/implbase6.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <svtools/miscopt.hxx>
#include <svl/ownlist.hxx>
#include <svl/itemprop.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

namespace {

class IFrameObject : public ::cppu::WeakImplHelper6 <
        css::util::XCloseable,
        css::lang::XEventListener,
        css::frame::XSynchronousFrameLoader,
        css::ui::dialogs::XExecutableDialog,
        css::lang::XServiceInfo,
        css::beans::XPropertySet >
{
    css::uno::Reference < css::uno::XComponentContext > mxContext;
    css::uno::Reference < css::frame::XFrame2 > mxFrame;
    css::uno::Reference < css::embed::XEmbeddedObject > mxObj;
    SfxItemPropertyMap  maPropMap;
    SfxFrameDescriptor  maFrmDescr;

public:
    IFrameObject(const css::uno::Reference < css::uno::XComponentContext>& rxContext, const css::uno::Sequence< css::uno::Any >& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);
    ~IFrameObject();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception)
    {
        return OUString("com.sun.star.comp.sfx2.IFrameObject");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception)
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception)
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = OUString("com.sun.star.frame.SpecialEmbeddedObject");
        return aSeq;
    }

    virtual sal_Bool SAL_CALL load( const css::uno::Sequence < css::beans::PropertyValue >& lDescriptor,
            const css::uno::Reference < css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL cancel() throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw( css::util::CloseVetoException, css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL addCloseListener( const css::uno::Reference < css::util::XCloseListener >& xListener ) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference < css::util::XCloseListener >& xListener ) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) ;
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (css::uno::RuntimeException, std::exception);
    virtual ::sal_Int16 SAL_CALL execute(  ) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL addVetoableChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception);
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception);
};

class IFrameWindow_Impl : public Window
{
    uno::Reference < frame::XFrame2 > mxFrame;
    sal_Bool                bBorder;

public:
    IFrameWindow_Impl( Window *pParent,
                       sal_Bool bHasBorder,
                       WinBits nWinBits = 0 );

public:
    sal_Bool        HasBorder() const { return bBorder; }
};

IFrameWindow_Impl::IFrameWindow_Impl( Window *pParent, sal_Bool bHasBorder, WinBits nWinBits )
    : Window( pParent, nWinBits | WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_DOCKBORDER )
    , bBorder(bHasBorder)
{
    if ( !bHasBorder )
        SetBorderStyle( WINDOW_BORDER_NOBORDER );
    else
        SetBorderStyle( WINDOW_BORDER_NORMAL );
}

#define PROPERTY_UNBOUND 0

#define WID_FRAME_URL                   1
#define WID_FRAME_NAME                  2
#define WID_FRAME_IS_AUTO_SCROLL        3
#define WID_FRAME_IS_SCROLLING_MODE     4
#define WID_FRAME_IS_BORDER             5
#define WID_FRAME_IS_AUTO_BORDER        6
#define WID_FRAME_MARGIN_WIDTH          7
#define WID_FRAME_MARGIN_HEIGHT         8

const SfxItemPropertyMapEntry* lcl_GetIFramePropertyMap_Impl()
{
    static const SfxItemPropertyMapEntry aIFramePropertyMap_Impl[] =
    {
        { OUString("FrameIsAutoBorder"),    WID_FRAME_IS_AUTO_BORDER,   ::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameIsAutoScroll"),    WID_FRAME_IS_AUTO_SCROLL,   ::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameIsBorder"),        WID_FRAME_IS_BORDER,        ::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameIsScrollingMode"), WID_FRAME_IS_SCROLLING_MODE,::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameMarginHeight"),    WID_FRAME_MARGIN_HEIGHT,    ::getCppuType( (sal_Int32*)0 ), PROPERTY_UNBOUND, 0 },
        { OUString("FrameMarginWidth"),     WID_FRAME_MARGIN_WIDTH,     ::getCppuType( (sal_Int32*)0 ), PROPERTY_UNBOUND, 0 },
        { OUString("FrameName"),            WID_FRAME_NAME,             ::getCppuType((const OUString*)0), PROPERTY_UNBOUND, 0 },
        { OUString("FrameURL"),             WID_FRAME_URL,              ::getCppuType((const OUString*)0), PROPERTY_UNBOUND, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aIFramePropertyMap_Impl;
}

IFrameObject::IFrameObject(const uno::Reference < uno::XComponentContext >& rxContext, const css::uno::Sequence< css::uno::Any >& aArguments)
    throw ( uno::Exception, uno::RuntimeException )
    : mxContext( rxContext )
    , maPropMap( lcl_GetIFramePropertyMap_Impl() )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= mxObj;
}

IFrameObject::~IFrameObject()
{
}

sal_Bool SAL_CALL IFrameObject::load(
    const uno::Sequence < com::sun::star::beans::PropertyValue >& /*lDescriptor*/,
    const uno::Reference < frame::XFrame >& xFrame )
throw( uno::RuntimeException, std::exception )
{
    if ( SvtMiscOptions().IsPluginsEnabled() )
    {
        DBG_ASSERT( !mxFrame.is(), "Frame already existing!" );
        Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        IFrameWindow_Impl* pWin = new IFrameWindow_Impl( pParent, maFrmDescr.IsFrameBorderOn() );
        pWin->SetSizePixel( pParent->GetOutputSizePixel() );
        pWin->SetBackground();
        pWin->Show();

        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );
        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );

        // we must destroy the IFrame before the parent is destroyed
        xWindow->addEventListener( this );

        mxFrame = frame::Frame::create( mxContext );
        uno::Reference < awt::XWindow > xWin( pWin->GetComponentInterface(), uno::UNO_QUERY );
        mxFrame->initialize( xWin );
        mxFrame->setName( maFrmDescr.GetName() );

        uno::Reference < frame::XFramesSupplier > xFramesSupplier( xFrame, uno::UNO_QUERY );
        if ( xFramesSupplier.is() )
            mxFrame->setCreator( xFramesSupplier );

        util::URL aTargetURL;
        aTargetURL.Complete = OUString( maFrmDescr.GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
        uno::Reference < util::XURLTransformer > xTrans( util::URLTransformer::create( mxContext ) );
        xTrans->parseStrict( aTargetURL );

        uno::Sequence < beans::PropertyValue > aProps(2);
        aProps[0].Name = "PluginMode";
        aProps[0].Value <<= (sal_Int16) 2;
        aProps[1].Name = "ReadOnly";
        aProps[1].Value <<= (sal_Bool) sal_True;
        uno::Reference < frame::XDispatch > xDisp = mxFrame->queryDispatch( aTargetURL, "_self", 0 );
        if ( xDisp.is() )
            xDisp->dispatch( aTargetURL, aProps );

        return sal_True;
    }

    return sal_False;
}

void SAL_CALL IFrameObject::cancel() throw( com::sun::star::uno::RuntimeException, std::exception )
{
    try
    {
        uno::Reference < util::XCloseable > xClose( mxFrame, uno::UNO_QUERY );
        if ( xClose.is() )
            xClose->close( sal_True );
        mxFrame = 0;
    }
    catch (const uno::Exception&)
    {
    }
}

void SAL_CALL IFrameObject::close( sal_Bool /*bDeliverOwnership*/ ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException, std::exception )
{
}

void SAL_CALL IFrameObject::addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException, std::exception )
{
}

void SAL_CALL IFrameObject::removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException, std::exception )
{
}

void SAL_CALL IFrameObject::disposing( const com::sun::star::lang::EventObject& ) throw (com::sun::star::uno::RuntimeException, std::exception)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL IFrameObject::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo = new SfxItemPropertySetInfo( maPropMap );
    return xInfo;
}

void SAL_CALL IFrameObject::setPropertyValue(const OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    switch( pEntry->nWID )
    {
    case WID_FRAME_URL:
    {
        OUString aURL;
        aAny >>= aURL;
        maFrmDescr.SetURL( aURL );
    }
    break;
    case WID_FRAME_NAME:
    {
        OUString aName;
        if ( aAny >>= aName )
            maFrmDescr.SetName( aName );
    }
    break;
    case WID_FRAME_IS_AUTO_SCROLL:
    {
        sal_Bool bIsAutoScroll = sal_Bool();
        if ( (aAny >>= bIsAutoScroll) && bIsAutoScroll )
            maFrmDescr.SetScrollingMode( ScrollingAuto );
    }
    break;
    case WID_FRAME_IS_SCROLLING_MODE:
    {
        sal_Bool bIsScroll = sal_Bool();
        if ( aAny >>= bIsScroll )
            maFrmDescr.SetScrollingMode( bIsScroll ? ScrollingYes : ScrollingNo );
    }
    break;
    case WID_FRAME_IS_BORDER:
    {
        sal_Bool bIsBorder = sal_Bool();
        if ( aAny >>= bIsBorder )
            maFrmDescr.SetFrameBorder( bIsBorder );
    }
    break;
    case WID_FRAME_IS_AUTO_BORDER:
    {
        sal_Bool bIsAutoBorder = sal_Bool();
        if ( (aAny >>= bIsAutoBorder) )
        {
            sal_Bool bBorder = maFrmDescr.IsFrameBorderOn();
            maFrmDescr.ResetBorder();
            if ( bIsAutoBorder )
                maFrmDescr.SetFrameBorder( bBorder );
        }
    }
    break;
    case WID_FRAME_MARGIN_WIDTH:
    {
        sal_Int32 nMargin = 0;
        Size aSize = maFrmDescr.GetMargin();
        if ( aAny >>= nMargin )
        {
            aSize.Width() = nMargin;
            maFrmDescr.SetMargin( aSize );
        }
    }
    break;
    case WID_FRAME_MARGIN_HEIGHT:
    {
        sal_Int32 nMargin = 0;
        Size aSize = maFrmDescr.GetMargin();
        if ( aAny >>= nMargin )
        {
            aSize.Height() = nMargin;
            maFrmDescr.SetMargin( aSize );
        }
    }
    break;
    default: ;
    }
}

uno::Any SAL_CALL IFrameObject::getPropertyValue(const OUString& aPropertyName)
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    uno::Any aAny;
    switch( pEntry->nWID )
    {
    case WID_FRAME_URL:
    {
        aAny <<= OUString( maFrmDescr.GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
    }
    break;
    case WID_FRAME_NAME:
    {
        aAny <<= OUString( maFrmDescr.GetName() );
    }
    break;
    case WID_FRAME_IS_AUTO_SCROLL:
    {
        sal_Bool bIsAutoScroll = ( maFrmDescr.GetScrollingMode() == ScrollingAuto );
        aAny <<= bIsAutoScroll;
    }
    break;
    case WID_FRAME_IS_SCROLLING_MODE:
    {
        sal_Bool bIsScroll = ( maFrmDescr.GetScrollingMode() == ScrollingYes );
        aAny <<= bIsScroll;
    }
    break;
    case WID_FRAME_IS_BORDER:
    {
        sal_Bool bIsBorder = maFrmDescr.IsFrameBorderOn();
        aAny <<= bIsBorder;
    }
    break;
    case WID_FRAME_IS_AUTO_BORDER:
    {
        sal_Bool bIsAutoBorder = !maFrmDescr.IsFrameBorderSet();
        aAny <<= bIsAutoBorder;
    }
    break;
    case WID_FRAME_MARGIN_WIDTH:
    {
        aAny <<= (sal_Int32 ) maFrmDescr.GetMargin().Width();
    }
    break;
    case WID_FRAME_MARGIN_HEIGHT:
    {
        aAny <<= (sal_Int32 ) maFrmDescr.GetMargin().Height();
    }
    default: ;
    }
    return aAny;
}

void SAL_CALL IFrameObject::addPropertyChangeListener(const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
}

void SAL_CALL IFrameObject::removePropertyChangeListener(const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
}

void SAL_CALL IFrameObject::addVetoableChangeListener(const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
}

void SAL_CALL IFrameObject::removeVetoableChangeListener(const OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException, std::exception )
{
}

::sal_Int16 SAL_CALL IFrameObject::execute() throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    VclAbstractDialog* pDlg = pFact->CreateEditObjectDialog( NULL, ".uno:InsertObjectFloatingFrame", mxObj );
    if ( pDlg )
        pDlg->Execute();
    return 0;
}

void SAL_CALL IFrameObject::setTitle( const OUString& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_sfx2_IFrameObject_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new IFrameObject(context, arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
