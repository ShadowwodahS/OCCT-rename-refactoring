// Created on: 1997-04-21
// Created by: Denis PASCAL
// Copyright (c) 1997-1999 Matra Datavision
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

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Draw_Display.hxx>
#include <DrawDim.hxx>
#include <DrawDim_Distance.hxx>
#include <gp_Ax1.hxx>
#include <gp_Pln.hxx>
#include <gp_Vec.hxx>
#include <Standard_Type.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DrawDim_Distance, DrawDim_Dimension)

//=================================================================================================

DrawDim_Distance::DrawDim_Distance(const TopoFace& plane1, const TopoFace& plane2)
{
  myPlane1 = plane1;
  myPlane2 = plane2;
}

//=================================================================================================

DrawDim_Distance::DrawDim_Distance(const TopoFace& plane1)

{
  myPlane1 = plane1;
}

//=================================================================================================

const TopoFace& DrawDim_Distance::Plane1() const
{
  return myPlane1;
}

//=================================================================================================

void DrawDim_Distance::Plane1(const TopoFace& face)
{
  myPlane1 = face;
}

//=================================================================================================

const TopoFace& DrawDim_Distance::Plane2() const
{
  return myPlane2;
}

//=================================================================================================

void DrawDim_Distance::Plane2(const TopoFace& face)
{
  myPlane2 = face;
}

//=================================================================================================

void DrawDim_Distance::DrawOn(DrawDisplay& dis) const
{

  // compute the points and the direction
  BRepAdaptor_Surface surf1(myPlane1);

  // today we process only planar faces
  if (surf1.GetType() != GeomAbs_Plane)
    return;

  Axis3d anAx1 = surf1.Plane1().Axis();
  Vector3d V     = anAx1.Direction();

  // output
  Point3d FAttach; // first attach point
  Point3d SAttach; // second attach point

  // first point, try a vertex
  ShapeExplorer explo(myPlane1, TopAbs_VERTEX);
  if (explo.More())
  {
    FAttach = BRepInspector::Pnt(TopoDS::Vertex(explo.Current()));
  }
  else
  {
    // no vertex, use the origin
    FAttach = anAx1.Location();
  }

  if (!myPlane2.IsNull())
  {
    // translate the point until the second face
    BRepAdaptor_Surface surf2(myPlane2);
    surf2.D0(0, 0, SAttach);
    Standard_Real r = V.Dot(Vector3d(FAttach, SAttach));
    V *= r;
  }

  SAttach = FAttach;
  SAttach.Translate(V);

  // DISPLAY
  dis.Draw1(FAttach, SAttach);
  V *= 0.5;
  FAttach.Translate(V);
  dis.DrawMarker(FAttach, Draw_Losange);
  DrawText(FAttach, dis);
}
