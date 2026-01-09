// Created on: 1998-01-20
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
#include <DsgPrs_EqualRadiusPresentation.hxx>
#include <gce_MakeDir.hxx>
#include <Geom_Plane.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <Graphic3d_Group.hxx>
#include <Precision.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Text.hxx>
#include <TCollection_ExtendedString.hxx>

void EqualRadiusPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                         const Handle(StyleDrawer)&       aDrawer,
                                         const Point3d&                     FirstCenter,
                                         const Point3d&                     SecondCenter,
                                         const Point3d&                     FirstPoint,
                                         const Point3d&                     SecondPoint,
                                         const Handle(GeomPlane)&         Plane1)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Handle(Graphic3d_ArrayOfPolylines) aPrims = new Graphic3d_ArrayOfPolylines(4);
  aPrims->AddVertex(FirstPoint);
  aPrims->AddVertex(FirstCenter);
  aPrims->AddVertex(SecondCenter);
  aPrims->AddVertex(SecondPoint);
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  // Add presentation of arrows
  Dir3d FirstDir  = DirectionBuilder(FirstCenter, FirstPoint),
         SecondDir = DirectionBuilder(SecondCenter, SecondPoint);
  DsgPrs1::ComputeSymbol(aPresentation,
                        LA,
                        FirstCenter,
                        FirstPoint,
                        FirstDir.Reversed(),
                        FirstDir,
                        DsgPrs_AS_FIRSTPT_LASTAR);
  DsgPrs1::ComputeSymbol(aPresentation,
                        LA,
                        SecondCenter,
                        SecondPoint,
                        SecondDir.Reversed(),
                        SecondDir,
                        DsgPrs_AS_FIRSTPT_LASTAR);

  // ota === begin ===
  Point3d        Middle((FirstCenter.XYZ() + SecondCenter.XYZ()) * 0.5), aTextPos;
  Standard_Real SmallDist;
  // Mark of constraint
  UtfString aText("==");

  Standard_Real Dist = FirstCenter.Distance(SecondCenter);
  if (Dist > Precision1::Confusion())
  {
    SmallDist = Dist * 0.05; // take 1/20 part of length;
    if (SmallDist <= Precision1::Confusion())
      SmallDist = Dist;
    Dir3d LineDir = DirectionBuilder(FirstCenter, SecondCenter);
    Dir3d OrtDir  = Plane1->Pln().Axis().Direction() ^ LineDir;

    Vector3d OrtVec = Vector3d(OrtDir) * SmallDist;

    // Compute the text position
    aTextPos = Middle.Translated(OrtVec);
  }
  else
  {
    Standard_Real Rad = Max(FirstCenter.Distance(FirstPoint), SecondCenter.Distance(SecondPoint));

    SmallDist = Rad * 0.05; // take 1/20 part of length;
    if (SmallDist <= Precision1::Confusion())
      SmallDist = Rad;

    Vector3d aVec(SmallDist, SmallDist, SmallDist);

    // Compute the text position
    aTextPos = FirstCenter.Translated(aVec);
  }

  // Draw1 the text
  Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, aTextPos);
  // ota === end ===
}
