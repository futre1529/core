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


#include "comphelper_module.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <map>


using namespace com::sun::star;

typedef std::map< OUString, uno::Sequence<beans::PropertyValue> > NamedPropertyValues;

class NamedPropertyValuesContainer : public cppu::WeakImplHelper2< container::XNameContainer, lang::XServiceInfo >
{
public:
    NamedPropertyValuesContainer() throw();
    virtual ~NamedPropertyValuesContainer() throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception);

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException,
            ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

private:
    NamedPropertyValues maProperties;
};

NamedPropertyValuesContainer::NamedPropertyValuesContainer() throw()
{
}

NamedPropertyValuesContainer::~NamedPropertyValuesContainer() throw()
{
}

// XNameContainer
void SAL_CALL NamedPropertyValuesContainer::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    if( maProperties.find( aName ) != maProperties.end() )
        throw container::ElementExistException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps ) )
        throw lang::IllegalArgumentException();

    maProperties.insert(  NamedPropertyValues::value_type(aName ,aProps) );
}

void SAL_CALL NamedPropertyValuesContainer::removeByName( const OUString& Name )
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace
void SAL_CALL NamedPropertyValuesContainer::replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps) )
        throw lang::IllegalArgumentException();

    (*aIter).second = aProps;
}

// XNameAccess
::com::sun::star::uno::Any SAL_CALL NamedPropertyValuesContainer::getByName( const OUString& aName )
    throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Any aElement;

    aElement <<= (*aIter).second;

    return aElement;
}

::com::sun::star::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getElementNames(  )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.begin();
    const NamedPropertyValues::iterator aEnd = maProperties.end();

    uno::Sequence< OUString > aNames( maProperties.size() );
    OUString* pNames = aNames.getArray();

    while( aIter != aEnd )
    {
        *pNames++ = (*aIter++).first;
    }

    return aNames;
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasByName( const OUString& aName )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

// XElementAccess
::com::sun::star::uno::Type SAL_CALL NamedPropertyValuesContainer::getElementType(  )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return ::getCppuType((uno::Sequence<beans::PropertyValue> *)0);
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasElements(  )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return !maProperties.empty();
}

//XServiceInfo
OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getImplementationName_static();
}

OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName_static(  )
{
    return OUString( "NamedPropertyValuesContainer" );
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

::com::sun::star::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_static();
}

::com::sun::star::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames_static(  )
{
    const OUString aServiceName( "com.sun.star.document.NamedPropertyValues" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer::Create(
                SAL_UNUSED_PARAMETER const uno::Reference< uno::XComponentContext >&)
{
    return (cppu::OWeakObject*)new NamedPropertyValuesContainer();
}

void createRegistryInfo_NamedPropertyValuesContainer()
{
    static ::comphelper::module::OAutoRegistration< NamedPropertyValuesContainer > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
