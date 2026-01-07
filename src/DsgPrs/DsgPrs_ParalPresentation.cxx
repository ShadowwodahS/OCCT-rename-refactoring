// Created on: 1995-11-28
// Created by: Jean-Pierre COMBE
// Copyright (c) 1995-1999 Matra Datavision
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
#include <DsgPrs_ParalPresentation.hxx>
#include <ElCLib.hxx>
#include <gce_MakeLin.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <Graphic3d_Group.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Text.hxx>
#include <TCollection_ExtendedString.hxx>

void DsgPrs_ParalPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                   const Handle(StyleDrawer)&       aDrawer,
                                   const UtfString& aText,
                                   const Point3d&                     AttachmentPoint1,
                                   const Point3d&                     AttachmentPoint2,
                                   const Dir3d&                     aDirection,
                                   const Point3d&                     OffsetPoint)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());
  gp_Lin        L1(AttachmentPoint1, aDirection);
  gp_Lin        L2(AttachmentPoint2, aDirection);
  Point3d        Proj1 = ElCLib1::Value(ElCLib1::Parameter(L1, OffsetPoint), L1);
  Point3d        Proj2 = ElCLib1::Value(ElCLib1::Parameter(L2, OffsetPoint), L2);
  gp_Lin        L3    = gce_MakeLin(Proj1, Proj2);
  Standard_Real parmin, parmax, parcur;
  parmin             = ElCLib1::Parameter(L3, Proj1);
  parmax             = parmin;
  parcur             = ElCLib1::Parameter(L3, Proj2);
  Standard_Real dist = Abs(parmin - parcur);
  if (parcur < parmin)
    parmin = parcur;
  if (parcur > parmax)
    parmax = parcur;
  parcur      = ElCLib1::Parameter(L3, OffsetPoint);
  Point3d offp = ElCLib1::Value(parcur, L3);

  Standard_Boolean outside = Standard_False;
  if (parcur < parmin)
  {
    parmin  = parcur;
    outside = Standard_True;
  }
  if (parcur > parmax)
  {
    parmax  = parcur;
    outside = Standard_True;
  }

  Point3d PointMin = ElCLib1::Value(parmin, L3);
  Point3d PointMax = ElCLib1::Value(parmax, L3);

  // processing of side : 1st group
  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(6);
  aPrims->AddVertex(PointMin);
  aPrims->AddVertex(PointMax);

  aPresentation->NewGroup();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  if (dist < (LA->ArrowAspect()->Length() + LA->ArrowAspect()->Length()))
    outside = Standard_True;
  Dir3d arrdir = L3.Direction().Reversed();
  if (outside)
    arrdir.Reverse();

  // arrow 1 : 2nd group
  Prs3d_Arrow::Draw1(aPresentation->CurrentGroup(),
                    Proj1,
                    arrdir,
                    LA->ArrowAspect()->Angle(),
                    LA->ArrowAspect()->Length());

  aPresentation->NewGroup();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  // arrow 2 : 3rd group
  Prs3d_Arrow::Draw1(aPresentation->CurrentGroup(),
                    Proj2,
                    arrdir.Reversed(),
                    LA->ArrowAspect()->Angle(),
                    LA->ArrowAspect()->Length());

  aPresentation->NewGroup();

  // text : 4th group
  Prs3d_Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, offp);

  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  // processing of call 1 : 5th group
  aPrims->AddVertex(AttachmentPoint1);
  aPrims->AddVertex(Proj1);

  // processing of call 2 : 6th group
  aPrims->AddVertex(AttachmentPoint2);
  aPrims->AddVertex(Proj2);

  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
}

//==========================================================================
// function : DsgPrs_ParalPresentation::Add
// purpose  : it is possible to choose the symbol of extremities of the face (arrow, point...)
//==========================================================================
void DsgPrs_ParalPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                   const Handle(StyleDrawer)&       aDrawer,
                                   const UtfString& aText,
                                   const Point3d&                     AttachmentPoint1,
                                   const Point3d&                     AttachmentPoint2,
                                   const Dir3d&                     aDirection,
                                   const Point3d&                     OffsetPoint,
                                   const DsgPrs_ArrowSide            ArrowPrs)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  gp_Lin        L1(AttachmentPoint1, aDirection);
  gp_Lin        L2(AttachmentPoint2, aDirection);
  Point3d        Proj1 = ElCLib1::Value(ElCLib1::Parameter(L1, OffsetPoint), L1);
  Point3d        Proj2 = ElCLib1::Value(ElCLib1::Parameter(L2, OffsetPoint), L2);
  gp_Lin        L3    = gce_MakeLin(Proj1, Proj2);
  Standard_Real parmin, parmax, parcur;
  parmin             = ElCLib1::Parameter(L3, Proj1);
  parmax             = parmin;
  parcur             = ElCLib1::Parameter(L3, Proj2);
  Standard_Real dist = Abs(parmin - parcur);
  if (parcur < parmin)
    parmin = parcur;
  if (parcur > parmax)
    parmax = parcur;
  parcur      = ElCLib1::Parameter(L3, OffsetPoint);
  Point3d offp = ElCLib1::Value(parcur, L3);

  Standard_Boolean outside = Standard_False;
  if (parcur < parmin)
  {
    parmin  = parcur;
    outside = Standard_True;
  }
  if (parcur > parmax)
  {
    parmax  = parcur;
    outside = Standard_True;
  }

  Point3d PointMin = ElCLib1::Value(parmin, L3);
  Point3d PointMax = ElCLib1::Value(parmax, L3);

  // processing of face
  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(6);
  aPrims->AddVertex(PointMin);
  aPrims->AddVertex(PointMax);

  if (dist < (LA->ArrowAspect()->Length() + LA->ArrowAspect()->Length()))
    outside = Standard_True;
  Dir3d arrdir = L3.Direction().Reversed();
  if (outside)
    arrdir.Reverse();

  // processing of call 1
  aPrims->AddVertex(AttachmentPoint1);
  aPrims->AddVertex(Proj1);

  // processing of call 2
  aPrims->AddVertex(AttachmentPoint2);
  aPrims->AddVertex(Proj2);

  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  // text
  Prs3d_Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, offp);

  // arrows
  DsgPrs::ComputeSymbol(aPresentation, LA, Proj1, Proj2, arrdir, arrdir.Reversed(), ArrowPrs);
}
