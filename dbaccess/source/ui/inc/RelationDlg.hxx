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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONDLG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONDLG_HXX

#include <boost/scoped_ptr.hpp>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
#include "JoinTableView.hxx"
#include "RelControliFace.hxx"
#include "moduledbu.hxx"

namespace dbaui
{
    class OJoinTableView;
    class OTableListBoxControl;
    class ORelationDialog : public ModalDialog
                            ,public IRelationControlInterface
    {
        OModuleClient                           m_aModuleClient;
        boost::scoped_ptr<OTableListBoxControl> m_xTableControl;
        OJoinTableView::OTableWindowMap*        m_pTableMap;

        RadioButton* m_pRB_NoCascUpd;
        RadioButton* m_pRB_CascUpd;
        RadioButton* m_pRB_CascUpdNull;
        RadioButton* m_pRB_CascUpdDefault;
        RadioButton* m_pRB_NoCascDel;
        RadioButton* m_pRB_CascDel;
        RadioButton* m_pRB_CascDelNull;
        RadioButton* m_pRB_CascDelDefault;

        OKButton*   m_pPB_OK;

        TTableConnectionData::value_type                                        m_pConnData;
        TTableConnectionData::value_type                                        m_pOrigConnData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

        sal_Bool                                                                    m_bTriedOneUpdate;

    public:
        ORelationDialog(OJoinTableView* pParent,
                        const TTableConnectionData::value_type& pConnectionData,
                        sal_Bool bAllowTableSelect = sal_False );
        virtual ~ORelationDialog();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection(){ return m_xConnection; }

        virtual short Execute();

        /** getTableMap gives acces to the table window map
            @retrun the table window from the join view
        */
        OJoinTableView::OTableWindowMap* getTableMap() const { return m_pTableMap; }

        /** getConnectionData returns the current connection data
            @return the current connectiondata
        */
        virtual TTableConnectionData::value_type getConnectionData() const;

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(sal_Bool _bValid);

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange();
    protected:
        void Init(const TTableConnectionData::value_type& _pConnectionData);

    private:
        DECL_LINK( OKClickHdl, Button* );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_RELATIONDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
