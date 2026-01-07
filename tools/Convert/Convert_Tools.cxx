// Created on: 2020-01-25
// Created by: Natalia ERMOLAEVA
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

#include <inspector/Convert_Tools.hxx>
#include <inspector/Convert_TransientShape.hxx>

#include <AIS_Line.hxx>
#include <AIS_Plane.hxx>
#include <AIS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepPreviewAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <gp_XY.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Prs3d_PlaneAspect.hxx>
#include <Standard_Dump.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopoDS_Compound.hxx>

// =======================================================================
// function : ReadShape
// purpose :
// =======================================================================
TopoShape Convert_Tools::ReadShape(const AsciiString1& theFileName)
{
  TopoShape aShape;

  ShapeBuilder aBuilder;
  BRepTools1::Read(aShape, theFileName.ToCString(), aBuilder);
  return aShape;
}

//=======================================================================
// function : ConvertStreamToPresentations
// purpose  :
//=======================================================================
void Convert_Tools::ConvertStreamToPresentations(
  const Standard_SStream& theSStream,
  const Standard_Integer  theStartPos,
  const Standard_Integer /*theLastPos*/,
  NCollection_List<Handle(RefObject)>& thePresentations)
{
  int aStartPos = theStartPos;

  gp_XYZ aPoint;
  if (aPoint.InitFromJson(theSStream, aStartPos))
  {
    thePresentations.Append(new Convert_TransientShape(BRepBuilderAPI_MakeVertex(aPoint)));
    return;
  }

  Point3d aPnt;
  if (aPnt.InitFromJson(theSStream, aStartPos))
  {
    thePresentations.Append(new Convert_TransientShape(BRepBuilderAPI_MakeVertex(aPnt)));
    return;
  }

  Dir3d aDir;
  if (aDir.InitFromJson(theSStream, aStartPos))
  {
    gp_Lin            aLin(gp::Origin(), aDir);
    Handle(GeomLine) aGeomLine = new GeomLine(aLin);
    CreatePresentation(aGeomLine, thePresentations);
    return;
  }

  Frame3d anAx2;
  if (anAx2.InitFromJson(theSStream, aStartPos))
  {
    Handle(GeomPlane) aGeomPlane = new GeomPlane(gp_Ax3(anAx2));
    CreatePresentation(aGeomPlane, thePresentations);
    return;
  }

  gp_Ax3 anAx3; // should be after Frame3d
  if (anAx3.InitFromJson(theSStream, aStartPos))
  {
    Handle(GeomPlane) aGeomPlane = new GeomPlane(anAx3);
    CreatePresentation(aGeomPlane, thePresentations);
    return;
  }

  // should be after gp_Ax3
  Axis3d anAxis;
  if (anAxis.InitFromJson(theSStream, aStartPos))
  {
    thePresentations.Append(new Convert_TransientShape(
      EdgeMaker(anAxis.Location(),
                              anAxis.Location().Coord() + anAxis.Direction().XYZ())));
    return;
  }

  Transform3d aTrsf;
  if (aTrsf.InitFromJson(theSStream, aStartPos))
  {
    CreatePresentation(aTrsf, thePresentations);
    return;
  }

  Box2 aBox;
  if (aBox.InitFromJson(theSStream, aStartPos))
  {
    TopoShape aShape;
    if (Convert_Tools::CreateShape(aBox, aShape))
      thePresentations.Append(new Convert_TransientShape(aShape));
    return;
  }

  Select3D_BndBox3d aSelectBndBox;
  if (aSelectBndBox.InitFromJson(theSStream, aStartPos))
  {
    TopoShape aShape;

    Point3d aPntMin = Point3d(aSelectBndBox.CornerMin().x(),
                            aSelectBndBox.CornerMin().y(),
                            aSelectBndBox.CornerMin().z());
    Point3d aPntMax = Point3d(aSelectBndBox.CornerMax().x(),
                            aSelectBndBox.CornerMax().y(),
                            aSelectBndBox.CornerMax().z());
    if (CreateBoxShape(aPntMin, aPntMax, aShape))
      thePresentations.Append(new Convert_TransientShape(aShape));
    return;
  }
}

//=======================================================================
// function : ConvertStreamToColor
// purpose  :
//=======================================================================
Standard_Boolean Convert_Tools::ConvertStreamToColor(const Standard_SStream& theSStream,
                                                     Quantity_Color&         theColor)
{
  Standard_Integer   aStartPos = 1;
  Quantity_ColorRGBA aColorRGBA;
  if (aColorRGBA.InitFromJson(theSStream, aStartPos))
  {
    theColor = aColorRGBA.GetRGB();
    return Standard_True;
  }

  Quantity_Color aColor;
  if (aColor.InitFromJson(theSStream, aStartPos))
  {
    theColor = aColor;
    return Standard_True;
  }

  return Standard_False;
}

//=======================================================================
// function : CreateShape
// purpose  :
//=======================================================================
Standard_Boolean Convert_Tools::CreateShape(const Box2& theBoundingBox, TopoShape& theShape)
{
  if (theBoundingBox.IsVoid() || theBoundingBox.IsWhole())
    return Standard_False;

  Standard_Real aXmin, anYmin, aZmin, aXmax, anYmax, aZmax;
  theBoundingBox.Get(aXmin, anYmin, aZmin, aXmax, anYmax, aZmax);

  Point3d aPntMin = Point3d(aXmin, anYmin, aZmin);
  Point3d aPntMax = Point3d(aXmax, anYmax, aZmax);

  return CreateBoxShape(aPntMin, aPntMax, theShape);
}

//=======================================================================
// function : CreateShape
// purpose  :
//=======================================================================
Standard_Boolean Convert_Tools::CreateShape(const OrientedBox& theBoundingBox, TopoShape& theShape)
{
  if (theBoundingBox.IsVoid())
    return Standard_False;

  TColgp_Array1OfPnt anArrPnts(0, 8);
  theBoundingBox.GetVertex(&anArrPnts(0));

  ShapeBuilder    aBuilder;
  TopoCompound aCompound;
  aBuilder.MakeCompound(aCompound);

  aBuilder.Add(aCompound,
               EdgeMaker(Point3d(anArrPnts.Value(0)), Point3d(anArrPnts.Value(1))));
  aBuilder.Add(aCompound,
               EdgeMaker(Point3d(anArrPnts.Value(0)), Point3d(anArrPnts.Value(2))));
  aBuilder.Add(aCompound,
               EdgeMaker(Point3d(anArrPnts.Value(1)), Point3d(anArrPnts.Value(3))));
  aBuilder.Add(aCompound,
               EdgeMaker(Point3d(anArrPnts.Value(2)), Point3d(anArrPnts.Value(3))));

  theShape = aCompound;
  return Standard_True;
}

//=======================================================================
// function : CreateBoxShape
// purpose  :
//=======================================================================
Standard_Boolean Convert_Tools::CreateBoxShape(const Point3d& thePntMin,
                                               const Point3d& thePntMax,
                                               TopoShape& theShape)
{
  BRepPreviewAPI_MakeBox aMakeBox;
  aMakeBox.Init(thePntMin, thePntMax);
  theShape = aMakeBox.Shape();

  return Standard_True;
}

//=======================================================================
// function : CreatePresentation
// purpose  :
//=======================================================================
void Convert_Tools::CreatePresentation(
  const Handle(GeomLine)&                      theLine,
  NCollection_List<Handle(RefObject)>& thePresentations)
{
  Handle(AIS_Line) aLinePrs = new AIS_Line(theLine);
  aLinePrs->SetColor(Quantity_NOC_TOMATO);
  thePresentations.Append(aLinePrs);
}

//=======================================================================
// function : CreatePresentation
// purpose  :
//=======================================================================
void Convert_Tools::CreatePresentation(
  const Handle(GeomPlane)&                     thePlane,
  NCollection_List<Handle(RefObject)>& thePresentations)
{
  Handle(AIS_Plane) aPlanePrs = new AIS_Plane(thePlane);

  aPlanePrs->Attributes()->SetPlaneAspect(new Prs3d_PlaneAspect());
  Handle(Prs3d_PlaneAspect) aPlaneAspect = aPlanePrs->Attributes()->PlaneAspect();
  aPlaneAspect->SetPlaneLength(100, 100);
  aPlaneAspect->SetDisplayCenterArrow(Standard_True);
  aPlaneAspect->SetDisplayEdgesArrows(Standard_True);
  aPlaneAspect->SetArrowsSize(100);
  aPlaneAspect->SetArrowsLength(100);
  aPlaneAspect->SetDisplayCenterArrow(Standard_True);
  aPlaneAspect->SetDisplayEdges(Standard_True);

  aPlanePrs->SetColor(Quantity_NOC_WHITE);
  aPlanePrs->SetTransparency(0);

  thePresentations.Append(aPlanePrs);
}

//=======================================================================
// function : CreatePresentation
// purpose  :
//=======================================================================
void Convert_Tools::CreatePresentation(
  const Transform3d&                                theTrsf,
  NCollection_List<Handle(RefObject)>& thePresentations)
{
  Box2 aBox(Point3d(), Point3d(10., 10., 10));

  TopoShape aBoxShape;
  if (!Convert_Tools::CreateShape(aBox, aBoxShape))
    return;

  Handle(VisualShape) aSourcePrs = new VisualShape(aBoxShape);
  aSourcePrs->Attributes()->SetAutoTriangulation(Standard_False);
  aSourcePrs->SetColor(Quantity_NOC_WHITE);
  aSourcePrs->SetTransparency(0.5);
  thePresentations.Append(aSourcePrs);

  Handle(VisualShape) aTransformedPrs = new VisualShape(aBoxShape);
  aTransformedPrs->Attributes()->SetAutoTriangulation(Standard_False);
  aTransformedPrs->SetColor(Quantity_NOC_TOMATO);
  aTransformedPrs->SetTransparency(0.5);
  aTransformedPrs->SetLocalTransformation(theTrsf);
  thePresentations.Append(aTransformedPrs);
}
