// Created on: 1996-05-28
// Created by: Denis PASCAL
// Copyright (c) 1996-1999 Matra Datavision
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
#include <Draw_Display.hxx>
#include <DrawDim.hxx>
#include <DrawDim_Angle.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <IntAna_QuadQuadGeo.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DrawDim_Angle, DrawDim_Dimension)

//=================================================================================================

DrawDim_Angle::DrawDim_Angle(const TopoFace& plane1, const TopoFace& plane2)
{
  myPlane1 = plane1;
  myPlane2 = plane2;
}

//=================================================================================================

const TopoFace& DrawDim_Angle::Plane1() const
{
  return myPlane1;
}

//=================================================================================================

void DrawDim_Angle::Plane1(const TopoFace& plane)
{
  myPlane1 = plane;
}

//=================================================================================================

const TopoFace& DrawDim_Angle::Plane2() const
{
  return myPlane2;
}

//=================================================================================================

void DrawDim_Angle::Plane2(const TopoFace& plane)
{
  myPlane2 = plane;
}

//=================================================================================================

void DrawDim_Angle::DrawOn(DrawDisplay&) const
{

  // input
  TopoShape  myFShape = myPlane1;
  TopoShape  mySShape = myPlane2;
  Standard_Real myVal    = GetValue();
  Axis3d        myAxis;

  // output
  Point3d           myFAttach;
  Point3d           mySAttach;
  Point3d           myPosition(0., 0., 0.);
  Point3d           myCenter;
  Dir3d           myFDir;
  Dir3d           mySDir;
  Standard_Boolean myAutomaticPosition = Standard_True;

  // calculation of myAxis
  gp_Pln pln1, pln2;
  if (!DrawDim1::Pln(myPlane1, pln1))
    return;
  if (!DrawDim1::Pln(myPlane2, pln2))
    return;
  QuadQuadGeoIntersection ip(pln1, pln2, Precision1::Confusion(), Precision1::Angular());
  if (!ip.IsDone())
    return;

  Point3d curpos;
  Axis3d AxePos     = myAxis;
  Dir3d theAxisDir = AxePos.Direction();
  gp_Lin theaxis    = gp_Lin(myAxis);

  if (myAutomaticPosition)
  {
    ShapeExplorer explo1(myFShape, TopAbs_VERTEX);
    Standard_Real   curdist = 0;
    while (explo1.More())
    {
      TopoVertex vertref = TopoDS::Vertex(explo1.Current());
      Point3d        curpt   = BRepInspector::Pnt(vertref);
      if (theaxis.Distance(curpt) > curdist)
      {
        curdist   = theaxis.Distance(curpt);
        myFAttach = BRepInspector::Pnt(vertref);
      }
      explo1.Next();
    }
    curpos                 = myFAttach.Rotated(AxePos, myVal / 2.);
    myCenter               = ElCLib1::Value(ElCLib1::Parameter(theaxis, curpos), theaxis);
    Standard_Real thedista = myCenter.Distance(myFAttach);
    if (thedista > Precision1::Confusion())
    {
      curpos.Scale(myCenter, 1.05);
    }
    myPosition          = curpos;
    myAutomaticPosition = Standard_True;
  }
  else
  {
    curpos = myPosition;
    // myFAttach  = the point of myFShape closest to curpos (except for the case when this is a
    // point on the axis)
    Standard_Real   dist = RealLast();
    ShapeExplorer explo1(myFShape, TopAbs_VERTEX);
    Point3d          AxePosition = AxePos.Location();
    Vector3d          AxeVector(theAxisDir);
    Coords3d          AxeXYZ = AxeVector.XYZ();
    while (explo1.More())
    {
      Point3d curpt = BRepInspector::Pnt(TopoDS::Vertex(explo1.Current()));
      Vector3d curvec(AxePosition, curpt);
      Coords3d curXYZ = curvec.XYZ();
      Coords3d Norm(curXYZ.Crossed(AxeXYZ));

      if (Norm.Modulus() > gp1::Resolution())
      {
        Standard_Real curdist = curpos.Distance(curpt);
        if (curdist < dist)
        {
          myFAttach = curpt;
          dist      = curdist;
        }
      }
      explo1.Next();
    }
    myCenter = ElCLib1::Value(ElCLib1::Parameter(theaxis, myFAttach), theaxis);
  }

  mySAttach = myFAttach.Rotated(AxePos, myVal);

  Vector3d FVec(myCenter, myFAttach);
  myFDir.SetXYZ(FVec.XYZ());
  Vector3d SVec(myCenter, mySAttach);
  mySDir.SetXYZ(SVec.XYZ());

  if (!myAutomaticPosition)
  {
    // Projection of the position on the plane defined by myFDir mySDir and normal theAxisDir
    gp_Pln        aPln(myCenter, theAxisDir);
    Standard_Real U, V;
    ElSLib1::Parameters(aPln, curpos, U, V);
    curpos = ElSLib1::Value(U, V, aPln);
  }

  // DISPLAY
  // Add (myVal, myText,myCenter,myFAttach,mySAttach,myFDir,mySDir,theAxisDir,curpos)
}
