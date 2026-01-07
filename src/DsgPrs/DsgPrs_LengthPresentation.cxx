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

#include <DsgPrs_LengthPresentation.hxx>

#include <DsgPrs.hxx>
#include <ElCLib.hxx>
#include <gce_MakeLin.hxx>
#include <Geom_Surface.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <Graphic3d_ArrayOfPolylines.hxx>
#include <Graphic3d_Group.hxx>
#include <Precision.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Presentation.hxx>
#include <Prs3d_Text.hxx>
#include <TCollection_ExtendedString.hxx>

void DsgPrs_LengthPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
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
  gp_Lin        L3    = Proj1.IsEqual(Proj2, Precision::Confusion()) ? gp_Lin(Proj1, aDirection)
                                                                     : gce_MakeLin(Proj1, Proj2);
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

  // face processing : 1st group
  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(6);
  aPrims->AddVertex(PointMin);
  aPrims->AddVertex(PointMax);

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

//==================================================================================
// function : Add
// purpose  : Adds presentation of length dimension between two planar faces
//==================================================================================

void DsgPrs_LengthPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                    const Handle(StyleDrawer)&       aDrawer,
                                    const UtfString& aText,
                                    const Point3d&                     AttachmentPoint1,
                                    const Point3d&                     AttachmentPoint2,
                                    const gp_Pln&                     PlaneOfFaces,
                                    const Dir3d&                     aDirection,
                                    const Point3d&                     OffsetPoint,
                                    const DsgPrs_ArrowSide            ArrowPrs)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Point3d EndOfArrow1, EndOfArrow2;
  Dir3d DirOfArrow1;

  DsgPrs::ComputePlanarFacesLengthPresentation(LA->ArrowAspect()->Length(),
                                               LA->ArrowAspect()->Length(),
                                               AttachmentPoint1,
                                               AttachmentPoint2,
                                               aDirection,
                                               OffsetPoint,
                                               PlaneOfFaces,
                                               EndOfArrow1,
                                               EndOfArrow2,
                                               DirOfArrow1);

  // Parameters for length's line
  gp_Lin        LengthLine(OffsetPoint, DirOfArrow1);
  Standard_Real Par1 = ElCLib1::Parameter(LengthLine, EndOfArrow1);
  Standard_Real Par2 = ElCLib1::Parameter(LengthLine, EndOfArrow2);
  Point3d        FirstPoint, LastPoint;
  if ((Par1 > 0.0 && Par2 > 0.0) || (Par1 < 0.0 && Par2 < 0.0))
  {
    FirstPoint = OffsetPoint;
    LastPoint  = (Abs(Par1) > Abs(Par2)) ? EndOfArrow1 : EndOfArrow2;
  }
  else
  {
    FirstPoint = EndOfArrow1;
    LastPoint  = EndOfArrow2;
  }

  // Creating the length's line
  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(6);

  aPrims->AddVertex(FirstPoint);
  aPrims->AddVertex(LastPoint);

  // Add presentation of arrows
  DsgPrs::ComputeSymbol(aPresentation,
                        LA,
                        EndOfArrow1,
                        EndOfArrow2,
                        DirOfArrow1,
                        DirOfArrow1.Reversed(),
                        ArrowPrs);

  // Drawing the text
  Prs3d_Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, OffsetPoint);

  // Line from AttachmentPoint1 to end of Arrow1
  aPrims->AddVertex(AttachmentPoint1);
  aPrims->AddVertex(EndOfArrow1);

  // Line from AttachmentPoint2 to end of Arrow2
  aPrims->AddVertex(AttachmentPoint2);
  aPrims->AddVertex(EndOfArrow2);

  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
}

//=========================================================================================
// function : Add
// purpose  : adds presentation of length between two edges, vertex and edge or two vertices
//=========================================================================================

void DsgPrs_LengthPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
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
  gp_Lin        L3    = Proj1.IsEqual(Proj2, Precision::Confusion()) ? gp_Lin(Proj1, aDirection)
                                                                     : gce_MakeLin(Proj1, Proj2);
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

  // processing of call  1
  aPrims->AddVertex(AttachmentPoint1);
  aPrims->AddVertex(Proj1);

  // processing of call 2
  aPrims->AddVertex(AttachmentPoint2);
  aPrims->AddVertex(Proj2);

  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  // text
  Prs3d_Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, offp);

  // symbols at the extremities of the face
  DsgPrs::ComputeSymbol(aPresentation, LA, Proj1, Proj2, arrdir, arrdir.Reversed(), ArrowPrs);
}

//==================================================================================
// function : Add
// purpose  : Adds presentation of length dimension between two curvilinear faces
//==================================================================================

void DsgPrs_LengthPresentation::Add(const Handle(Prs3d_Presentation)& aPresentation,
                                    const Handle(StyleDrawer)&       aDrawer,
                                    const UtfString& aText,
                                    const Handle(GeomSurface)&       SecondSurf,
                                    const Point3d&                     AttachmentPoint1,
                                    const Point3d&                     AttachmentPoint2,
                                    const Dir3d&                     aDirection,
                                    const Point3d&                     OffsetPoint,
                                    const DsgPrs_ArrowSide            ArrowPrs)
{
  Handle(Prs3d_DimensionAspect) LA = aDrawer->DimensionAspect();
  aPresentation->CurrentGroup()->SetPrimitivesAspect(LA->LineAspect()->Aspect());

  Point3d             EndOfArrow2;
  Dir3d             DirOfArrow1;
  Handle(GeomCurve3d) VCurve, UCurve;
  Standard_Real      FirstU, deltaU = 0.0e0, FirstV, deltaV = 0.0e0;

  DsgPrs::ComputeCurvilinearFacesLengthPresentation(LA->ArrowAspect()->Length(),
                                                    LA->ArrowAspect()->Length(),
                                                    SecondSurf,
                                                    AttachmentPoint1,
                                                    AttachmentPoint2,
                                                    aDirection,
                                                    EndOfArrow2,
                                                    DirOfArrow1,
                                                    VCurve,
                                                    UCurve,
                                                    FirstU,
                                                    deltaU,
                                                    FirstV,
                                                    deltaV);

  gp_Lin        LengthLine(OffsetPoint, DirOfArrow1);
  Standard_Real Par1 = ElCLib1::Parameter(LengthLine, AttachmentPoint1);
  Standard_Real Par2 = ElCLib1::Parameter(LengthLine, EndOfArrow2);
  Point3d        FirstPoint, LastPoint;
  if ((Par1 > 0.0 && Par2 > 0.0) || (Par1 < 0.0 && Par2 < 0.0))
  {
    FirstPoint = OffsetPoint;
    LastPoint  = (Abs(Par1) > Abs(Par2)) ? AttachmentPoint1 : EndOfArrow2;
  }
  else
  {
    FirstPoint = AttachmentPoint1;
    LastPoint  = EndOfArrow2;
  }

  // Creating the length's line
  Handle(Graphic3d_ArrayOfPrimitives) aPrims = new Graphic3d_ArrayOfSegments(2);
  aPrims->AddVertex(FirstPoint);
  aPrims->AddVertex(LastPoint);
  aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);

  // Add presentation of arrows
  DsgPrs::ComputeSymbol(aPresentation,
                        LA,
                        AttachmentPoint1,
                        EndOfArrow2,
                        DirOfArrow1,
                        DirOfArrow1.Reversed(),
                        ArrowPrs);

  // Drawing the text
  Prs3d_Text::Draw1(aPresentation->CurrentGroup(), LA->TextAspect(), aText, OffsetPoint);

  // Two curves from end of Arrow2 to AttachmentPoint2
  Standard_Real    Alpha, delta;
  Standard_Integer NodeNumber;

  Alpha = Abs(deltaU);
  if (Alpha > Precision::Angular() && Alpha < Precision::Infinite())
  {
    NodeNumber = Max(4, Standard_Integer(50. * Alpha / M_PI));
    delta      = deltaU / (Standard_Real)(NodeNumber - 1);
    aPrims     = new Graphic3d_ArrayOfPolylines(NodeNumber);
    for (Standard_Integer i = 1; i <= NodeNumber; i++, FirstU += delta)
      aPrims->AddVertex(VCurve->Value(FirstU));
    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }
  Alpha = Abs(deltaV);
  if (Alpha > Precision::Angular() && Alpha < Precision::Infinite())
  {
    NodeNumber = Max(4, Standard_Integer(50. * Alpha / M_PI));
    delta      = deltaV / (Standard_Real)(NodeNumber - 1);
    aPrims     = new Graphic3d_ArrayOfPolylines(NodeNumber);
    for (Standard_Integer i = 1; i <= NodeNumber; i++, FirstV += delta)
      aPrims->AddVertex(UCurve->Value(FirstV));
    aPresentation->CurrentGroup()->AddPrimitiveArray(aPrims);
  }
}

//================================
// Function:
// Purpose: Rob 26-mar-96
//=================================

void DsgPrs_LengthPresentation::Add(const Handle(Prs3d_Presentation)& aPrs,
                                    const Handle(StyleDrawer)&       aDrawer,
                                    const Point3d&                     Pt1,
                                    const Point3d&                     Pt2,
                                    const DsgPrs_ArrowSide            ArrowPrs)
{
  Handle(Graphic3d_ArrayOfSegments) aPrims = new Graphic3d_ArrayOfSegments(2);
  aPrims->AddVertex(Pt1);
  aPrims->AddVertex(Pt2);
  aPrs->CurrentGroup()->AddPrimitiveArray(aPrims);

  Vector3d V;
  switch (ArrowPrs)
  {
    case DsgPrs_AS_LASTAR:
      Prs3d_Arrow::Draw1(aPrs->CurrentGroup(),
                        Pt2,
                        Dir3d(Vector3d(Pt1, Pt2)),
                        aDrawer->DimensionAspect()->ArrowAspect()->Angle(),
                        aDrawer->DimensionAspect()->ArrowAspect()->Length());
      break;
    case DsgPrs_AS_FIRSTAR:
      Prs3d_Arrow::Draw1(aPrs->CurrentGroup(),
                        Pt1,
                        Dir3d(Vector3d(Pt2, Pt1)),
                        aDrawer->DimensionAspect()->ArrowAspect()->Angle(),
                        aDrawer->DimensionAspect()->ArrowAspect()->Length());
      break;
    case DsgPrs_AS_BOTHAR:
      V = Vector3d(Pt1, Pt2);
      Prs3d_Arrow::Draw1(aPrs->CurrentGroup(),
                        Pt2,
                        Dir3d(V),
                        aDrawer->DimensionAspect()->ArrowAspect()->Angle(),
                        aDrawer->DimensionAspect()->ArrowAspect()->Length());
      Prs3d_Arrow::Draw1(aPrs->CurrentGroup(),
                        Pt1,
                        Dir3d(V.Reversed()),
                        aDrawer->DimensionAspect()->ArrowAspect()->Angle(),
                        aDrawer->DimensionAspect()->ArrowAspect()->Length());
      break;
    default:
      break;
  }
}
