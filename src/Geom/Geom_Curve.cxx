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
#include <Geom_UndefinedDerivative.hxx>
#include <Geom_UndefinedValue.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomCurve3d, Geometry3)

typedef GeomCurve3d Curve;

//=================================================================================================

Handle(GeomCurve3d) GeomCurve3d::Reversed() const
{
  Handle(GeomCurve3d) C = Handle(GeomCurve3d)::DownCast(Copy());
  C->Reverse();
  return C;
}

//=================================================================================================

Standard_Real GeomCurve3d::Period() const
{
  Standard_NoSuchObject_Raise_if(!IsPeriodic(), "GeomCurve3d::Period");

  return (LastParameter() - FirstParameter());
}

//=================================================================================================

Point3d GeomCurve3d::Value(const Standard_Real U) const
{
  Point3d P;
  D0(U, P);
  return P;
}

//=================================================================================================

Standard_Real GeomCurve3d::TransformedParameter(const Standard_Real U, const Transform3d&) const
{
  return U;
}

//=================================================================================================

Standard_Real GeomCurve3d::ParametricTransformation(const Transform3d&) const
{
  return 1.;
}

//=================================================================================================

void GeomCurve3d::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, Geometry3)
}
