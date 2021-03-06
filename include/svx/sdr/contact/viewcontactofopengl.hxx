/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFOPENGL_HXX
#define INCLUDED_SVX_SDR_CONTACT_VIEWCONTACTOFOPENGL_HXX

#include <svx/sdr/contact/viewcontactofsdrobj.hxx>

class SdrOpenGLObj;

namespace sdr {
namespace contact {

class ViewContactOfOpenGL : public ViewContactOfSdrObj
{
public:
    explicit ViewContactOfOpenGL(SdrOpenGLObj& rOpenGLObj);
    virtual ~ViewContactOfOpenGL();

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence createViewIndependentPrimitive2DSequence() const;
};

} // namespace contact
} // namespace sdr

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
