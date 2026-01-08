// Created on: 1994-08-05
// Created by: Remi LEQUETTE
// Copyright (c) 1994-1999 Matra Datavision
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

#include <GeomAPI.hxx>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Plane.hxx>
#include <GeomAdaptor.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_Pln.hxx>
#include <ProjLib_ProjectedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>

//=================================================================================================

Handle(GeomCurve2d) GeomAPI1::To2d(const Handle(GeomCurve3d)& C, const gp_Pln& P)
{
  Handle(GeomCurve2d)        result;
  Handle(GeomAdaptor_Curve)   HC    = new GeomAdaptor_Curve(C);
  Handle(GeomPlane)          Plane1 = new GeomPlane(P);
  Handle(GeomAdaptor_Surface) HS    = new GeomAdaptor_Surface(Plane1);

  ProjLib_ProjectedCurve Proj(HS, HC);

  if (Proj.GetType() != GeomAbs_OffsetCurve && Proj.GetType() != GeomAbs_OtherCurve)
  {
    result = Geom2dAdaptor1::MakeCurve(Proj);
  }

  return result;
}

//=================================================================================================

Handle(GeomCurve3d) GeomAPI1::To3d(const Handle(GeomCurve2d)& C, const gp_Pln& P)
{
  Handle(Geom2dAdaptor_Curve) AHC = new Geom2dAdaptor_Curve(C);

  Handle(GeomPlane)          ThePlane = new GeomPlane(P);
  Handle(GeomAdaptor_Surface) AHS      = new GeomAdaptor_Surface(ThePlane);

  Adaptor3d_CurveOnSurface COS(AHC, AHS);
  return GeomAdaptor1::MakeCurve(COS);
}
