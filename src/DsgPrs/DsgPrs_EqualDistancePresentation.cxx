// Created on: 1998-01-27
// Created by: Julia GERASIMOVA
// Copyright (c) 1998-1999 Matra Datavision
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

#include <DsgPrs.hxx>
#include <DsgPrs_EqualDistancePresentation.hxx>
#include <ElCLib.hxx>
#include <gce_MakeDir.hxx>
#include <Geom_Plane.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_Group.hxx>
#include <Precision.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Text.hxx>
#include <TCollection_ExtendedString.hxx>

//=================================================================================
// function  : Add
//=================================================================================
void EqualDistancePresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                           const Handle(StyleDrawer)&       aDrawer,
                                           const Point3d&                     Point1,
                                           const Point3d&                     Point2,
                                           const Point3d&                     Point3,
                                           const Point3d&                     Point4,
                                           const Handle(GeomPlane)&         Plane1)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  // Line between two middles
  Point3d Middle12((Point1.XYZ() + Point2.XYZ()) * 0.5),
    Middle34((Point3.XYZ() + Point4.XYZ()) * 0.5);

  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(2);
  aPrims->AddVertex(Middle12);
  aPrims->AddVertex(Middle34);
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  // Add presentation of arrows (points)
  Dir3d aDir(0, 0, 1);
  DsgPrs1::ComputeSymbol(aPresentation, LA, Middle12, Middle34, aDir, aDir, DsgPrs_AS_BOTHPT);
  // ota -- begin --
  // Two small lines in the middle of this line
  Point3d        Middle((Middle12.XYZ() + Middle34.XYZ()) * 0.5), aTextPos;
  Standard_Real Dist = Middle12.Distance(Middle34);
  Standard_Real SmallDist;
  Dir3d        LineDir, OrtDir;
  Vector3d        LineVec, OrtVec;

  if (Dist > Precision1::Confusion())
  {
    SmallDist = Dist * 0.05; // 1/20.0 part
    if (SmallDist <= Precision1::Confusion())
      SmallDist = Dist;
    LineDir = DirectionBuilder(Middle12, Middle34);
    OrtDir  = Plane1->Pln().Axis().Direction() ^ LineDir;
    LineVec = Vector3d(LineDir) * SmallDist;
    OrtVec  = Vector3d(OrtDir) * SmallDist;

    aTextPos = Middle.Translated(OrtVec);
  }
  else
  {
    Vector3d Vec1(Middle, Point1);

    if (Vec1.SquareMagnitude() > Precision1::Confusion() * Precision1::Confusion())
    {
      Standard_Real Angle  = Vector3d(Middle, Point1).Angle(Vector3d(Middle, Point3));
      Point3d        MidPnt = Point1.Rotated(Plane1->Pln().Axis(), Angle * 0.5);
      OrtDir               = DirectionBuilder(Middle, MidPnt);
      LineDir              = OrtDir ^ Plane1->Pln().Axis().Direction();

      Standard_Real Distance = Point1.Distance(Point2);
      SmallDist              = Distance * 0.05; // 1/20.0
      if (SmallDist <= Precision1::Confusion())
        SmallDist = Distance;

      OrtVec  = Vector3d(OrtDir) * SmallDist;
      LineVec = Vector3d(LineDir) * SmallDist;
    }
    else
    {
      SmallDist = 5.0;
      OrtVec    = Vector3d(Plane1->Pln().XAxis().Direction()) * SmallDist;
      LineVec   = Vector3d(Plane1->Pln().YAxis().Direction()) * SmallDist;
    }
    aTextPos = Middle.Translated(OrtVec);
  }

  UtfString aText("==");

  // Draw1 the text
  Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, aTextPos);
}

//==================================================================================
// function  : AddInterval
// purpose   : is used for presentation of interval between two lines or two points,
//            or between one line and one point.
//==================================================================================
void EqualDistancePresentation::AddInterval(const Handle(Prs3d_Presentation)& aPresentation,
                                                   const Handle(StyleDrawer)&       aDrawer,
                                                   const Point3d&                     aPoint1,
                                                   const Point3d&                     aPoint2,
                                                   const Dir3d&                     aDirection,
                                                   const Point3d&                     aPosition,
                                                   const DsgPrs_ArrowSide            anArrowSide,
                                                   Point3d&                           aProj1,
                                                   Point3d&                           aProj2)
{
  const Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  gp_Lin L1(aPoint1, aDirection);
  gp_Lin L2(aPoint2, aDirection);
  aProj1 = ElCLib1::Value(ElCLib1::Parameter(L1, aPosition), L1);
  aProj2 = ElCLib1::Value(ElCLib1::Parameter(L2, aPosition), L2);

  Handle(Graphic3d_ArrayOfPolylines) aPrims = new Graphic3d_ArrayOfPolylines(4);
  aPrims->AddVertex(aPoint1);
  aPrims->AddVertex(aProj1);
  aPrims->AddVertex(aProj2);
  aPrims->AddVertex(aPoint2);
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  // add arrows presentation
  Dir3d aDir(aProj2.XYZ() - aProj1.XYZ());

  DsgPrs1::ComputeSymbol(aPresentation, LA, aProj1, aProj2, aDir.Reversed(), aDir, anArrowSide);
}

//========================================================================
// function : AddIntervalBetweenTwoArcs
// purpose  : is used for presentation of interval between two arcs. One
//            of the arcs can have a zero radius (being a point really)
//========================================================================
void EqualDistancePresentation::AddIntervalBetweenTwoArcs(
  const Handle(Prs3d_Presentation)& aPresentation,
  const Handle(StyleDrawer)&       aDrawer,
  const gp_Circ&                    aCirc1,
  const gp_Circ&                    aCirc2,
  const Point3d&                     aPoint1,
  const Point3d&                     aPoint2,
  const Point3d&                     aPoint3,
  const Point3d&                     aPoint4,
  const DsgPrs_ArrowSide            anArrowSide)
{
  const Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Standard_Real aPar11, aPar12, aPar21, aPar22;
  if (aCirc1.Radius() > Precision1::Confusion())
  {
    aPar11 = ElCLib1::Parameter(aCirc1, aPoint1);
    aPar12 = ElCLib1::Parameter(aCirc1, aPoint2);
  }
  else
  {
    aPar11 = M_PI;
    aPar12 = M_PI;
  }
  if (aCirc2.Radius() > Precision1::Confusion())
  {
    aPar21 = ElCLib1::Parameter(aCirc2, aPoint3);
    aPar22 = ElCLib1::Parameter(aCirc2, aPoint4);
  }
  else
  {
    aPar21 = M_PI;
    aPar22 = M_PI;
  }

  Handle(Graphic3d_ArrayOfPrimitives) aPrims = new Graphic3d_ArrayOfSegments(2);
  aPrims->AddVertex(aPoint2);
  aPrims->AddVertex(aPoint4);
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  Standard_Integer i, aNodeNb;
  Standard_Real    aDelta, aCurPar;
  if (aPar12 < aPar11)
    aPar12 += 2. * M_PI;
  if (Abs(aPar12 - aPar11) > Precision1::Confusion())
  {
    aNodeNb = Standard_Integer(Max(Abs(aPar12 - aPar11) * 50. / M_PI + 0.5, 4.));
    aDelta  = (aPar12 - aPar11) / aNodeNb;
    aCurPar = aPar11;

    aPrims = new Graphic3d_ArrayOfPolylines(aNodeNb + 1);
    for (i = 1; i <= aNodeNb; aCurPar += aDelta, i++)
      aPrims->AddVertex(ElCLib1::Value(aCurPar, aCirc1));
    aPrims->AddVertex(aPoint2);
    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }
  if (aPar22 < aPar21)
    aPar22 += 2. * M_PI;
  if (Abs(aPar22 - aPar21) > Precision1::Confusion())
  {
    aNodeNb = Standard_Integer(Max(Abs(aPar22 - aPar21) * 50. / M_PI + 0.5, 4.));
    aDelta  = (aPar22 - aPar21) / aNodeNb;
    aCurPar = aPar21;

    aPrims = new Graphic3d_ArrayOfPolylines(aNodeNb + 1);
    for (i = 1; i <= aNodeNb; aCurPar += aDelta, i++)
      aPrims->AddVertex(ElCLib1::Value(aCurPar, aCirc2));
    aPrims->AddVertex(aPoint4);
    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }

  // get the direction of interval
  Dir3d DirOfArrow;
  if (aPoint4.Distance(aPoint2) > Precision1::Confusion())
  {
    DirOfArrow.SetXYZ(aPoint4.XYZ() - aPoint2.XYZ());
  }
  else
  {
    // Let's take the radius direction
    Point3d aCenter = aCirc1.Location();
    if (aPoint4.Distance(aCenter) < Precision1::Confusion())
      return;
    DirOfArrow.SetXYZ(aPoint4.XYZ() - aCenter.XYZ());
  }

  // Add presentation of arrows
  DsgPrs1::ComputeSymbol(aPresentation,
                        LA,
                        aPoint2,
                        aPoint4,
                        DirOfArrow.Reversed(),
                        DirOfArrow,
                        anArrowSide);
}

//-- ota -- end
