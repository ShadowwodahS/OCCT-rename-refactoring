// Created on: 2020-01-31
// Created by: Svetlana SHUTINA
// Copyright (c) 2020 OPEN CASCADE SAS
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

#include <BRepPreviewAPI_MakeBox.hxx>

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

//=================================================================================================

void BRepPreviewAPI_MakeBox::Build(const Message_ProgressRange& /*theRange*/)
{
  Point3d anLocation = myWedge.Axes().Location();

  Point3d aFirstPoint(anLocation.X(), anLocation.Y(), anLocation.Z());
  Point3d aSecondPoint(anLocation.X() + myWedge.GetXMax(),
                      anLocation.Y() + myWedge.GetYMax(),
                      anLocation.Z() + myWedge.GetZMax());

  Standard_Boolean aThinOnX = Abs(aFirstPoint.X() - aSecondPoint.X()) < Precision::Confusion();
  Standard_Boolean aThinOnY = Abs(aFirstPoint.Y() - aSecondPoint.Y()) < Precision::Confusion();
  Standard_Boolean aThinOnZ = Abs(aFirstPoint.Z() - aSecondPoint.Z()) < Precision::Confusion();

  Standard_Integer aPreviewType = (int)aThinOnX + (int)aThinOnY + (int)aThinOnZ;

  if (aPreviewType == 3) // thin box in all directions is a point
  {
    makeVertex(aFirstPoint);
  }
  else if (aPreviewType == 2) // thin box in two directions is a point
  {
    makeEdge(aFirstPoint, aSecondPoint);
  }
  // thin box in only one direction is a rectangular face
  else if (aPreviewType == 1)
  {
    Point3d aPnt1, aPnt2, aPnt3, aPnt4;
    if (aThinOnX)
    {
      aPnt1 = Point3d(aFirstPoint.X(), aFirstPoint.Y(), aFirstPoint.Z());
      aPnt2 = Point3d(aFirstPoint.X(), aSecondPoint.Y(), aFirstPoint.Z());
      aPnt3 = Point3d(aFirstPoint.X(), aSecondPoint.Y(), aSecondPoint.Z());
      aPnt4 = Point3d(aFirstPoint.X(), aFirstPoint.Y(), aSecondPoint.Z());
    }
    else if (aThinOnY)
    {
      aPnt1 = Point3d(aFirstPoint.X(), aFirstPoint.Y(), aFirstPoint.Z());
      aPnt2 = Point3d(aSecondPoint.X(), aFirstPoint.Y(), aFirstPoint.Z());
      aPnt3 = Point3d(aSecondPoint.X(), aFirstPoint.Y(), aSecondPoint.Z());
      aPnt4 = Point3d(aFirstPoint.X(), aFirstPoint.Y(), aSecondPoint.Z());
    }
    else if (aThinOnZ)
    {
      aPnt1 = Point3d(aFirstPoint.X(), aFirstPoint.Y(), aFirstPoint.Z());
      aPnt2 = Point3d(aSecondPoint.X(), aFirstPoint.Y(), aFirstPoint.Z());
      aPnt3 = Point3d(aSecondPoint.X(), aSecondPoint.Y(), aFirstPoint.Z());
      aPnt4 = Point3d(aFirstPoint.X(), aSecondPoint.Y(), aFirstPoint.Z());
    }

    makeRectangle(aPnt1, aPnt2, aPnt3, aPnt4);
  }

  if (!myShape.IsNull())
  {
    Done();
    return;
  }

  // box is a valid shape
  Solid();
}

//=================================================================================================

void BRepPreviewAPI_MakeBox::makeVertex(const Point3d& thePoint)
{
  myShape = BRepBuilderAPI_MakeVertex(thePoint);
}

//=================================================================================================

void BRepPreviewAPI_MakeBox::makeEdge(const Point3d& thePoint1, const Point3d& thePoint2)
{
  myShape = BRepBuilderAPI_MakeEdge(thePoint1, thePoint2);
}

//=================================================================================================

void BRepPreviewAPI_MakeBox::makeRectangle(const Point3d& thePnt1,
                                           const Point3d& thePnt2,
                                           const Point3d& thePnt3,
                                           const Point3d& thePnt4)
{
  TopoDS_Edge anEdge1 = BRepBuilderAPI_MakeEdge(thePnt1, thePnt2);
  TopoDS_Edge anEdge2 = BRepBuilderAPI_MakeEdge(thePnt2, thePnt3);
  TopoDS_Edge anEdge3 = BRepBuilderAPI_MakeEdge(thePnt3, thePnt4);
  TopoDS_Edge anEdge4 = BRepBuilderAPI_MakeEdge(thePnt4, thePnt1);

  BRepBuilderAPI_MakeWire aWire(anEdge1, anEdge2, anEdge3, anEdge4);
  BRepBuilderAPI_MakeFace aFace(aWire);

  myShape = aFace.Shape();
}
