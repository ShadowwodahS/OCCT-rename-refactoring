// Created on: 1993-03-10
// Created by: JCV
// Copyright (c) 1993-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_UndefinedDerivative.hxx>
#include <Geom_UndefinedValue.hxx>
#include <gp_GTrsf2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomSurface, Geometry3)

typedef GeomSurface Surface;

//=================================================================================================

Handle(GeomSurface) GeomSurface::UReversed() const
{
  Handle(GeomSurface) S = Handle(GeomSurface)::DownCast(Copy());
  S->UReverse();
  return S;
}

//=================================================================================================

Handle(GeomSurface) GeomSurface::VReversed() const
{
  Handle(GeomSurface) S = Handle(GeomSurface)::DownCast(Copy());
  S->VReverse();
  return S;
}

//=================================================================================================

void GeomSurface::TransformParameters(Standard_Real&, Standard_Real&, const Transform3d&) const {}

//=================================================================================================

GeneralTransform2d GeomSurface::ParametricTransformation(const Transform3d&) const
{
  GeneralTransform2d dummy;
  return dummy;
}

//=================================================================================================

Standard_Real GeomSurface::UPeriod() const
{
  Standard_NoSuchObject_Raise_if(!IsUPeriodic(), "GeomSurface::UPeriod");

  Standard_Real U1, U2, V1, V2;
  Bounds(U1, U2, V1, V2);
  return (U2 - U1);
}

//=================================================================================================

Standard_Real GeomSurface::VPeriod() const
{
  Standard_NoSuchObject_Raise_if(!IsVPeriodic(), "GeomSurface::VPeriod");

  Standard_Real U1, U2, V1, V2;
  Bounds(U1, U2, V1, V2);
  return (V2 - V1);
}

//=================================================================================================

Point3d GeomSurface::Value(const Standard_Real U, const Standard_Real V) const
{
  Point3d P;
  D0(U, V, P);
  return P;
}

//=================================================================================================

void GeomSurface::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, Geometry3)
}
