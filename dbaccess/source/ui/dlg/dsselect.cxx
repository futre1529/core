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

#include "dsselect.hxx"
#include "dbu_dlg.hrc"
#include <vcl/msgbox.hxx>
#include "localresaccess.hxx"
#include <tools/rcid.h>

#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include "dbustrings.hrc"
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <comphelper/processfactory.hxx>
#include "dsitems.hxx"
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>

namespace dbaui
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::comphelper;
ODatasourceSelectDialog::ODatasourceSelectDialog(Window* _pParent, const StringBag& _rDatasources, SfxItemSet* _pOutputSet)
    : ModalDialog(_pParent, "ChooseDataSourceDialog",
        "dbaccess/ui/choosedatasourcedialog.ui")
    , m_pOutputSet(_pOutputSet)
{
    get(m_pDatasource, "treeview");
    m_pDatasource->set_height_request(m_pDatasource->GetTextHeight() * 6);
    get(m_pOk, "ok");
    get(m_pCancel, "cancel");

    fillListBox(_rDatasources);
#ifdef HAVE_ODBC_ADMINISTRATION
    get(m_pManageDatasources, "organize");
    m_pManageDatasources->Show();

    // allow ODBC datasource managenment
    m_pManageDatasources->Show();
    m_pManageDatasources->Enable();
    m_pManageDatasources->SetClickHdl(LINK(this,ODatasourceSelectDialog,ManageClickHdl));
#endif
    m_pDatasource->SetDoubleClickHdl(LINK(this,ODatasourceSelectDialog,ListDblClickHdl));
}

ODatasourceSelectDialog::~ODatasourceSelectDialog()
{
}

IMPL_LINK( ODatasourceSelectDialog, ListDblClickHdl, ListBox *, pListBox )
{
    if (pListBox->GetSelectEntryCount())
        EndDialog(RET_OK);
    return 0;
}

bool ODatasourceSelectDialog::Close()
{
#ifdef HAVE_ODBC_ADMINISTRATION
    if ( m_pODBCManagement.get() && m_pODBCManagement->isRunning() )
        return sal_False;
#endif

    return ModalDialog::Close();
}

#ifdef HAVE_ODBC_ADMINISTRATION
IMPL_LINK_NOARG(ODatasourceSelectDialog, ManageClickHdl)
{
    if ( !m_pODBCManagement.get() )
        m_pODBCManagement.reset( new OOdbcManagement( LINK( this, ODatasourceSelectDialog, ManageProcessFinished ) ) );

    if ( !m_pODBCManagement->manageDataSources_async() )
    {
        // TODO: error message
        m_pDatasource->GrabFocus();
        m_pManageDatasources->Disable();
        return 1L;
    }

    m_pDatasource->Disable();
    m_pOk->Disable();
    m_pCancel->Disable();
    m_pManageDatasources->Disable();

    OSL_POSTCOND( m_pODBCManagement->isRunning(), "ODatasourceSelectDialog::ManageClickHdl: success, but not running - you were *fast*!" );
    return 0L;
}

IMPL_LINK( ODatasourceSelectDialog, ManageProcessFinished, void*, /**/ )
{
    StringBag aOdbcDatasources;
    OOdbcEnumeration aEnumeration;
    aEnumeration.getDatasourceNames( aOdbcDatasources );
    fillListBox( aOdbcDatasources );

    m_pDatasource->Enable();
    m_pOk->Enable();
    m_pCancel->Enable();
    m_pManageDatasources->Enable();

    return 0L;
}

#endif
void ODatasourceSelectDialog::fillListBox(const StringBag& _rDatasources)
{
    OUString sSelected;
    if (m_pDatasource->GetEntryCount())
         sSelected = m_pDatasource->GetSelectEntry();
    m_pDatasource->Clear();
    // fill the list
    for (   StringBag::const_iterator aDS = _rDatasources.begin();
            aDS != _rDatasources.end();
            ++aDS
        )
    {
        m_pDatasource->InsertEntry( *aDS );
    }

    if (m_pDatasource->GetEntryCount())
    {
        if (!sSelected.isEmpty())
            m_pDatasource->SelectEntry(sSelected);
        else        // select the first entry
            m_pDatasource->SelectEntryPos(0);
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
