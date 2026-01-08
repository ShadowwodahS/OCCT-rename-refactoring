// Created on: 1992-11-06
// Created by: Remi LEQUETTE
// Copyright (c) 1992-1999 Matra Datavision
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

#include <BRepPrim_Revolution.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt2d.hxx>
#include <Precision.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>

//=================================================================================================

BRepPrim_Revolution::BRepPrim_Revolution(const Frame3d&               A,
                                         const Standard_Real         VMin,
                                         const Standard_Real         VMax,
                                         const Handle(GeomCurve3d)&   M,
                                         const Handle(GeomCurve2d)& PM)
    : BRepPrim_OneAxis(BRepPrim_Builder(), A, VMin, VMax),
      myMeridian(M),
      myPMeridian(PM)
{
}

//=================================================================================================

BRepPrim_Revolution::BRepPrim_Revolution(const Frame3d&       A,
                                         const Standard_Real VMin,
                                         const Standard_Real VMax)
    : BRepPrim_OneAxis(BRepPrim_Builder(), A, VMin, VMax)
{
}

//=================================================================================================

void BRepPrim_Revolution::Meridian(const Handle(GeomCurve3d)& M, const Handle(GeomCurve2d)& PM)
{
  myMeridian  = M;
  myPMeridian = PM;
}

//=================================================================================================

TopoFace BRepPrim_Revolution::MakeEmptyLateralFace() const
{
  Handle(Geom_SurfaceOfRevolution) S = new Geom_SurfaceOfRevolution(myMeridian, Axes().Axis());

  TopoFace F;
  myBuilder.Builder().MakeFace(F, S, Precision1::Confusion());
  return F;
}

//=================================================================================================

TopoEdge BRepPrim_Revolution::MakeEmptyMeridianEdge(const Standard_Real Ang) const
{
  TopoEdge        E;
  Handle(GeomCurve3d) C = Handle(GeomCurve3d)::DownCast(myMeridian->Copy());
  Transform3d            T;
  T.SetRotation(Axes().Axis(), Ang);
  C->Transform(T);
  myBuilder.Builder().MakeEdge(E, C, Precision1::Confusion());
  return E;
}

//=================================================================================================

gp_Pnt2d BRepPrim_Revolution::MeridianValue(const Standard_Real V) const
{
  return myPMeridian->Value(V);
}

//=================================================================================================

void BRepPrim_Revolution::SetMeridianPCurve(TopoEdge& E, const TopoFace& F) const
{
  myBuilder.Builder().UpdateEdge(E, myPMeridian, F, Precision1::Confusion());
}
