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
#include "svx/sidebar/PopupControl.hxx"
#include "LineWidthValueSet.hxx"
#include <svl/poolitem.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

class SfxBindings;

namespace svx { namespace sidebar {

class LinePropertyPanel;

class LineWidthControl
    : public svx::sidebar::PopupControl
{
public:
    LineWidthControl (Window* pParent, LinePropertyPanel& rPanel);
    virtual ~LineWidthControl (void);

    virtual void GetFocus();
    virtual void Paint(const Rectangle& rect);

    void SetWidthSelect( long lValue, bool bValuable, SfxMapUnit eMapUnit);
    bool IsCloseByEdit();
    long GetTmpCustomWidth();

private:
    LinePropertyPanel& mrLinePropertyPanel;
    SfxBindings*                        mpBindings;
    LineWidthValueSet maVSWidth;
    FixedText                           maFTCus;
    FixedText                           maFTWidth;
    MetricField                         maMFWidth;
    SfxMapUnit                          meMapUnit;
    OUString*                           rStr;
    OUString                            mstrPT;
    long                                mnCustomWidth;
    bool                                mbCustom;
    bool                                mbColseByEdit;
    long                                mnTmpCusomWidth;
    bool                                mbVSFocus;

    Image                               maIMGCus;
    Image                               maIMGCusGray;

    void Initialize();
    DECL_LINK(VSSelectHdl, void *);
    DECL_LINK(MFModifyHdl, void *);
};

} } // end of namespace svx::sidebar

// eof
