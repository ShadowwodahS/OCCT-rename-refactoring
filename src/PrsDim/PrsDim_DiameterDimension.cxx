// Created on: 1996-12-05
// Created by: Jacques MINOT/Odile Olivier/Sergey ZARITCHNY
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

#include <PrsDim_DiameterDimension.hxx>

#include <PrsDim.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <ElCLib.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Plane.hxx>
#include <gce_MakeDir.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_DiameterDimension, PrsDim_Dimension)

namespace
{
static const Standard_ExtCharacter THE_DIAMETER_SYMBOL(0x00D8);
}

//=================================================================================================

PrsDim_DiameterDimension::PrsDim_DiameterDimension(const gp_Circ& theCircle)
    : PrsDim_Dimension(PrsDim_KOD_DIAMETER)
{
  SetMeasuredGeometry(theCircle);
  SetSpecialSymbol(THE_DIAMETER_SYMBOL);
  SetDisplaySpecialSymbol(PrsDim_DisplaySpecialSymbol_Before);
  SetFlyout(0.0);
}

//=================================================================================================

PrsDim_DiameterDimension::PrsDim_DiameterDimension(const gp_Circ& theCircle, const gp_Pln& thePlane)
    : PrsDim_Dimension(PrsDim_KOD_DIAMETER)
{
  SetCustomPlane(thePlane);
  SetMeasuredGeometry(theCircle);
  SetSpecialSymbol(THE_DIAMETER_SYMBOL);
  SetDisplaySpecialSymbol(PrsDim_DisplaySpecialSymbol_Before);
  SetFlyout(0.0);
}

//=================================================================================================

PrsDim_DiameterDimension::PrsDim_DiameterDimension(const TopoShape& theShape)
    : PrsDim_Dimension(PrsDim_KOD_DIAMETER)
{
  SetMeasuredGeometry(theShape);
  SetSpecialSymbol(THE_DIAMETER_SYMBOL);
  SetDisplaySpecialSymbol(PrsDim_DisplaySpecialSymbol_Before);
  SetFlyout(0.0);
}

//=================================================================================================

PrsDim_DiameterDimension::PrsDim_DiameterDimension(const TopoShape& theShape,
                                                   const gp_Pln&       thePlane)
    : PrsDim_Dimension(PrsDim_KOD_DIAMETER)
{
  SetCustomPlane(thePlane);
  SetMeasuredGeometry(theShape);
  SetSpecialSymbol(THE_DIAMETER_SYMBOL);
  SetDisplaySpecialSymbol(PrsDim_DisplaySpecialSymbol_Before);
  SetFlyout(0.0);
}

//=================================================================================================

Point3d PrsDim_DiameterDimension::AnchorPoint()
{
  if (!IsValid())
  {
    return gp1::Origin();
  }

  return myAnchorPoint;
}

//=================================================================================================

void PrsDim_DiameterDimension::SetMeasuredGeometry(const gp_Circ& theCircle)
{
  myCircle          = theCircle;
  myGeometryType    = GeometryType_Edge;
  myShape           = BRepLib_MakeEdge(theCircle);
  myAnchorPoint     = gp1::Origin();
  myIsGeometryValid = IsValidCircle(myCircle);

  if (myIsGeometryValid && myIsPlaneCustom)
  {
    ComputeAnchorPoint();
  }
  else if (!myIsPlaneCustom)
  {
    ComputePlane();
    myAnchorPoint = ElCLib1::Value(0.0, myCircle);
  }

  SetToUpdate();
}

//=================================================================================================

void PrsDim_DiameterDimension::SetMeasuredGeometry(const TopoShape& theShape)
{
  Point3d           aDummyPnt(gp1::Origin());
  Standard_Boolean isClosed = Standard_False;

  myGeometryType    = GeometryType_UndefShapes;
  myShape           = theShape;
  myAnchorPoint     = gp1::Origin();
  myIsGeometryValid = InitCircularDimension(theShape, myCircle, aDummyPnt, isClosed)
                      && IsValidCircle(myCircle) && isClosed;

  if (myIsGeometryValid && myIsPlaneCustom)
  {
    ComputeAnchorPoint();
  }
  else if (!myIsPlaneCustom)
  {
    ComputePlane();
    myAnchorPoint = ElCLib1::Value(0.0, myCircle);
  }

  SetToUpdate();
}

//=================================================================================================

Standard_Boolean PrsDim_DiameterDimension::CheckPlane(const gp_Pln& thePlane) const
{
  // Check if the circle center point belongs to plane.
  if (!thePlane.Contains(myCircle.Location(), Precision1::Confusion()))
  {
    return Standard_False;
  }

  return Standard_True;
}

//=================================================================================================

void PrsDim_DiameterDimension::ComputePlane()
{
  if (!myIsGeometryValid)
  {
    return;
  }

  myPlane = gp_Pln(Ax3(myCircle.Position1()));
}

//=================================================================================================

void PrsDim_DiameterDimension::ComputeAnchorPoint()
{
  // Anchor point is an intersection of dimension plane and circle.
  Handle(GeomCircle) aCircle = new GeomCircle(myCircle);
  Handle(GeomPlane)  aPlane  = new GeomPlane(myPlane);
  GeomAPI_IntCS       anIntersector(aCircle, aPlane);
  if (!anIntersector.IsDone())
  {
    myIsGeometryValid = Standard_False;
    return;
  }

  // The circle lays on the plane.
  if (anIntersector.NbPoints() != 2)
  {
    myAnchorPoint     = ElCLib1::Value(0.0, myCircle);
    myIsGeometryValid = Standard_True;
    return;
  }

  Point3d aFirstPoint  = anIntersector.Point(1);
  Point3d aSecondPoint = anIntersector.Point(2);

  // Choose one of two intersection points that stands with
  // positive direction of flyout.
  // An anchor point is supposed to be the left attachment point.
  Dir3d aFirstDir = gce_MakeDir(aFirstPoint, myCircle.Location());
  Dir3d aDir      = myPlane.Axis().Direction() ^ aFirstDir;
  myAnchorPoint =
    (Vector3d(aDir) * Vector3d(myCircle.Position1().Direction()) > 0.0) ? aFirstPoint : aSecondPoint;
}

//=================================================================================================

const AsciiString1& PrsDim_DiameterDimension::GetModelUnits() const
{
  return myDrawer->DimLengthModelUnits();
}

//=================================================================================================

const AsciiString1& PrsDim_DiameterDimension::GetDisplayUnits() const
{
  return myDrawer->DimLengthDisplayUnits();
}

//=================================================================================================

void PrsDim_DiameterDimension::SetModelUnits(const AsciiString1& theUnits)
{
  myDrawer->SetDimLengthModelUnits(theUnits);
}

//=================================================================================================

void PrsDim_DiameterDimension::SetDisplayUnits(const AsciiString1& theUnits)
{
  myDrawer->SetDimLengthDisplayUnits(theUnits);
}

//=================================================================================================

Standard_Real PrsDim_DiameterDimension::ComputeValue() const
{
  if (!IsValid())
  {
    return 0.0;
  }

  return myCircle.Radius() * 2.0;
}

//=================================================================================================

void PrsDim_DiameterDimension::Compute(const Handle(PrsMgr_PresentationManager)&,
                                       const Handle(Prs3d_Presentation)& thePresentation,
                                       const Standard_Integer            theMode)
{
  mySelectionGeom.Clear(theMode);

  if (!IsValid())
  {
    return;
  }

  Point3d aFirstPnt(gp1::Origin());
  Point3d aSecondPnt(gp1::Origin());
  ComputeSidePoints(myCircle, aFirstPnt, aSecondPnt);

  DrawLinearDimension(thePresentation, theMode, aFirstPnt, aSecondPnt);
}

//=================================================================================================

void PrsDim_DiameterDimension::ComputeFlyoutSelection(
  const Handle(SelectionContainer)&   theSelection,
  const Handle(SelectMgr_EntityOwner)& theEntityOwner)
{
  if (!IsValid())
  {
    return;
  }

  Point3d aFirstPnt(gp1::Origin());
  Point3d aSecondPnt(gp1::Origin());
  ComputeSidePoints(myCircle, aFirstPnt, aSecondPnt);

  ComputeLinearFlyouts(theSelection, theEntityOwner, aFirstPnt, aSecondPnt);
}

//=================================================================================================

void PrsDim_DiameterDimension::ComputeSidePoints(const gp_Circ& theCircle,
                                                 Point3d&        theFirstPnt,
                                                 Point3d&        theSecondPnt)
{
  theFirstPnt = AnchorPoint();

  Vector3d aRadiusVector(theCircle.Location(), theFirstPnt);
  theSecondPnt = theCircle.Location().Translated(-aRadiusVector);
}

//=================================================================================================

Standard_Boolean PrsDim_DiameterDimension::IsValidCircle(const gp_Circ& theCircle) const
{
  return (theCircle.Radius() * 2.0) > Precision1::Confusion();
}

//=================================================================================================

Standard_Boolean PrsDim_DiameterDimension::IsValidAnchor(const gp_Circ& theCircle,
                                                         const Point3d&  theAnchor) const
{
  gp_Pln        aCirclePlane(theCircle.Location(), theCircle.Axis().Direction());
  Standard_Real anAnchorDist = theAnchor.Distance(theCircle.Location());
  Standard_Real aRadius      = myCircle.Radius();

  return Abs(anAnchorDist - aRadius) > Precision1::Confusion()
         && aCirclePlane.Contains(theAnchor, Precision1::Confusion());
}

//=================================================================================================

Point3d PrsDim_DiameterDimension::GetTextPosition() const
{
  if (IsTextPositionCustom())
  {
    return myFixedTextPosition;
  }

  // Counts text position according to the dimension parameters
  return GetTextPositionForLinear(myAnchorPoint, myCircle.Location());
}

//=================================================================================================

void PrsDim_DiameterDimension::SetTextPosition(const Point3d& theTextPos)
{
  if (!IsValid())
  {
    return;
  }

  myIsTextPositionFixed = Standard_True;
  myFixedTextPosition   = theTextPos;

  SetToUpdate();
}
