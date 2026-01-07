// Created on: 1993-07-23
// Created by: Remi LEQUETTE
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

#include <BRepBuilderAPI.hxx>
#include <BRepPrim_Revolution.hxx>
#include <BRepPrimAPI_MakeRevolution.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Plane.hxx>
#include <GeomProjLib.hxx>
#include <gp_Ax2.hxx>

//=================================================================================================

static Handle(GeomCurve2d) Project(const Handle(GeomCurve3d)& M, const gp_Ax3& Axis)
{
  Handle(GeomCurve2d) C;
  C = GeomProjLib::Curve2d(M, new GeomPlane(Axis));
  return C;
}

static Handle(GeomCurve2d) Project(const Handle(GeomCurve3d)& M)
{
  return Project(M, Frame3d(gp::Origin(), -gp::DY(), gp::DX()));
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Handle(GeomCurve3d)& Meridian)
    : myRevolution(gp::XOY(),
                   Meridian->FirstParameter(),
                   Meridian->LastParameter(),
                   Meridian,
                   Project(Meridian))
{
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Handle(GeomCurve3d)& Meridian,
                                                       const Standard_Real       angle)
    : myRevolution(Frame3d(Point3d(0, 0, 0), Dir3d(0, 0, 1), Dir3d(1, 0, 0)),
                   Meridian->FirstParameter(),
                   Meridian->LastParameter(),
                   Meridian,
                   Project(Meridian))
{
  myRevolution.Angle(angle);
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Handle(GeomCurve3d)& Meridian,
                                                       const Standard_Real       VMin,
                                                       const Standard_Real       VMax)
    : myRevolution(Frame3d(Point3d(0, 0, 0), Dir3d(0, 0, 1), Dir3d(1, 0, 0)),
                   VMin,
                   VMax,
                   Meridian,
                   Project(Meridian))
{
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Handle(GeomCurve3d)& Meridian,
                                                       const Standard_Real       VMin,
                                                       const Standard_Real       VMax,
                                                       const Standard_Real       angle)
    : myRevolution(Frame3d(Point3d(0, 0, 0), Dir3d(0, 0, 1), Dir3d(1, 0, 0)),
                   VMin,
                   VMax,
                   Meridian,
                   Project(Meridian))
{
  myRevolution.Angle(angle);
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Frame3d&             Axes,
                                                       const Handle(GeomCurve3d)& Meridian)
    : myRevolution(Axes,
                   Meridian->FirstParameter(),
                   Meridian->LastParameter(),
                   Meridian,
                   Project(Meridian))
{
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Frame3d&             Axes,
                                                       const Handle(GeomCurve3d)& Meridian,
                                                       const Standard_Real       angle)
    : myRevolution(Axes,
                   Meridian->FirstParameter(),
                   Meridian->LastParameter(),
                   Meridian,
                   Project(Meridian))
{
  myRevolution.Angle(angle);
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Frame3d&             Axes,
                                                       const Handle(GeomCurve3d)& Meridian,
                                                       const Standard_Real       VMin,
                                                       const Standard_Real       VMax)
    : myRevolution(Axes, VMin, VMax, Meridian, Project(Meridian))
{
}

//=================================================================================================

BRepPrimAPI_MakeRevolution::BRepPrimAPI_MakeRevolution(const Frame3d&             Axes,
                                                       const Handle(GeomCurve3d)& Meridian,
                                                       const Standard_Real       VMin,
                                                       const Standard_Real       VMax,
                                                       const Standard_Real       angle)
    : myRevolution(Axes, VMin, VMax, Meridian, Project(Meridian))
{
  myRevolution.Angle(angle);
}

//=================================================================================================

Standard_Address BRepPrimAPI_MakeRevolution::OneAxis()
{
  return &myRevolution;
}

//=================================================================================================

BRepPrim_Revolution& BRepPrimAPI_MakeRevolution::Revolution()
{
  return myRevolution;
}
