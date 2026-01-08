// Created on: 1998-01-24
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

#include <PrsDim_EqualDistanceRelation.hxx>

#include <PrsDim.hxx>
#include <PrsDim_LengthDimension.hxx>
#include <Bnd_Box.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <DsgPrs_EqualDistancePresentation.hxx>
#include <ElCLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <Precision.hxx>
#include <Prs3d_ArrowAspect.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_Presentation.hxx>
#include <Select3D_SensitiveBox.hxx>
#include <Select3D_SensitivePoly.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_EqualDistanceRelation, PrsDim_Relation)

//=================================================================================================

PrsDim_EqualDistanceRelation::PrsDim_EqualDistanceRelation(const TopoShape&       aShape1,
                                                           const TopoShape&       aShape2,
                                                           const TopoShape&       aShape3,
                                                           const TopoShape&       aShape4,
                                                           const Handle(GeomPlane)& aPlane)
    : PrsDim_Relation()
{
  myFShape = aShape1;
  mySShape = aShape2;
  myShape3 = aShape3;
  myShape4 = aShape4;
  myPlane  = aPlane;

  // Temporary
  myArrowSize = 3.0; // set the concrete value
  mySymbolPrs = DsgPrs_AS_BOTHAR;
}

//=================================================================================================

void PrsDim_EqualDistanceRelation::Compute(const Handle(PrsMgr_PresentationManager)&,
                                           const Handle(Prs3d_Presentation)& aPresentation,
                                           const Standard_Integer)
{
  Point3d Position12 = myPosition, Position34 = myPosition;

  Handle(Prs3d_DimensionAspect) la  = myDrawer->DimensionAspect();
  Handle(Prs3d_ArrowAspect)     arr = la->ArrowAspect();
  arr->SetLength(myArrowSize);
  // -- ota -- begin
  if (!myAutomaticPosition)
  {
    Point3d aMiddle12((myPoint1.XYZ() + myPoint2.XYZ()) * 0.5);
    Point3d aMiddle34((myPoint3.XYZ() + myPoint4.XYZ()) * 0.5);

    if (myPosition.Distance(aMiddle12) > myPosition.Distance(aMiddle34))
      Position12.SetXYZ((myPoint1.XYZ() + myPoint2.XYZ()) * 0.5);
    else
      Position34.SetXYZ((myPoint3.XYZ() + myPoint4.XYZ()) * 0.5);
  }

  if (myFShape.ShapeType() == TopAbs_EDGE && mySShape.ShapeType() == TopAbs_EDGE)
    PrsDim_EqualDistanceRelation::ComputeTwoEdgesLength(aPresentation,
                                                        myDrawer,
                                                        myArrowSize,
                                                        TopoDS::Edge(myFShape),
                                                        TopoDS::Edge(mySShape),
                                                        myPlane,
                                                        myAutomaticPosition,
                                                        myIsSetBndBox,
                                                        myBndBox,
                                                        Position12,
                                                        myAttachPoint1,
                                                        myAttachPoint2,
                                                        myPoint1,
                                                        myPoint2,
                                                        mySymbolPrs);

  else if (myFShape.ShapeType() == TopAbs_VERTEX && mySShape.ShapeType() == TopAbs_VERTEX)
    PrsDim_EqualDistanceRelation::ComputeTwoVerticesLength(aPresentation,
                                                           myDrawer,
                                                           myArrowSize,
                                                           TopoDS::Vertex(myFShape),
                                                           TopoDS::Vertex(mySShape),
                                                           myPlane,
                                                           myAutomaticPosition,
                                                           myIsSetBndBox,
                                                           myBndBox,
                                                           PrsDim_TypeOfDist_Unknown,
                                                           Position12,
                                                           myAttachPoint1,
                                                           myAttachPoint2,
                                                           myPoint1,
                                                           myPoint2,
                                                           mySymbolPrs);
  else
    PrsDim_EqualDistanceRelation::ComputeOneEdgeOneVertexLength(aPresentation,
                                                                myDrawer,
                                                                myArrowSize,
                                                                myFShape,
                                                                mySShape,
                                                                myPlane,
                                                                myAutomaticPosition,
                                                                myIsSetBndBox,
                                                                myBndBox,
                                                                Position12,
                                                                myAttachPoint1,
                                                                myAttachPoint2,
                                                                myPoint1,
                                                                myPoint2,
                                                                mySymbolPrs);

  if (myShape3.ShapeType() == TopAbs_EDGE && myShape4.ShapeType() == TopAbs_EDGE)
    PrsDim_EqualDistanceRelation::ComputeTwoEdgesLength(aPresentation,
                                                        myDrawer,
                                                        myArrowSize,
                                                        TopoDS::Edge(myShape3),
                                                        TopoDS::Edge(myShape4),
                                                        myPlane,
                                                        myAutomaticPosition,
                                                        myIsSetBndBox,
                                                        myBndBox,
                                                        Position34,
                                                        myAttachPoint3,
                                                        myAttachPoint4,
                                                        myPoint3,
                                                        myPoint4,
                                                        mySymbolPrs);

  else if (myShape3.ShapeType() == TopAbs_VERTEX && myShape4.ShapeType() == TopAbs_VERTEX)
    PrsDim_EqualDistanceRelation::ComputeTwoVerticesLength(aPresentation,
                                                           myDrawer,
                                                           myArrowSize,
                                                           TopoDS::Vertex(myShape3),
                                                           TopoDS::Vertex(myShape4),
                                                           myPlane,
                                                           myAutomaticPosition,
                                                           myIsSetBndBox,
                                                           myBndBox,
                                                           PrsDim_TypeOfDist_Unknown,
                                                           Position34,
                                                           myAttachPoint3,
                                                           myAttachPoint4,
                                                           myPoint3,
                                                           myPoint4,
                                                           mySymbolPrs);

  else
    PrsDim_EqualDistanceRelation::ComputeOneEdgeOneVertexLength(aPresentation,
                                                                myDrawer,
                                                                myArrowSize,
                                                                myShape3,
                                                                myShape4,
                                                                myPlane,
                                                                myAutomaticPosition,
                                                                myIsSetBndBox,
                                                                myBndBox,
                                                                Position34,
                                                                myAttachPoint3,
                                                                myAttachPoint4,
                                                                myPoint3,
                                                                myPoint4,
                                                                mySymbolPrs);

  EqualDistancePresentation::Add(aPresentation,
                                        myDrawer,
                                        myPoint1,
                                        myPoint2,
                                        myPoint3,
                                        myPoint4,
                                        myPlane);
}

//=================================================================================================

void PrsDim_EqualDistanceRelation::ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                                    const Standard_Integer)
{
  Handle(SelectMgr_EntityOwner)     own = new SelectMgr_EntityOwner(this, 7);
  Handle(Select3D_SensitiveSegment) seg;

  seg = new Select3D_SensitiveSegment(own, myPoint1, myPoint2);
  aSelection->Add(seg);

  seg = new Select3D_SensitiveSegment(own, myPoint3, myPoint4);
  aSelection->Add(seg);

  // Line between two middles
  Point3d Middle12((myPoint1.XYZ() + myPoint2.XYZ()) * 0.5),
    Middle34((myPoint3.XYZ() + myPoint4.XYZ()) * 0.5);
  seg = new Select3D_SensitiveSegment(own, Middle12, Middle34);
  aSelection->Add(seg);

  Point3d                        Middle((Middle12.XYZ() + Middle34.XYZ()) * 0.5);
  Standard_Real                 SmallDist = .001;
  Handle(Select3D_SensitiveBox) box       = new Select3D_SensitiveBox(own,
                                                                Middle.X() - SmallDist,
                                                                Middle.Y() - SmallDist,
                                                                Middle.Z() - SmallDist,
                                                                Middle.X() + SmallDist,
                                                                Middle.Y() + SmallDist,
                                                                Middle.Z() + SmallDist);
  aSelection->Add(box);

  if (myFShape.ShapeType() == TopAbs_EDGE)
  {
    BRepAdaptor_Curve aCurve(TopoDS::Edge(myFShape));
    if (aCurve.GetType() == GeomAbs_Line)
    {
      // add sensetive element - line
      seg = new Select3D_SensitiveSegment(own, myAttachPoint1, myPoint1);
      aSelection->Add(seg);
    }
    else if (aCurve.GetType() == GeomAbs_Circle)
    {
      Handle(GeomCircle) aCircle  = Handle(GeomCircle)::DownCast(aCurve.Curve().Curve());
      Standard_Real       FirstPar = ElCLib1::Parameter(aCircle->Circ(), myAttachPoint1),
                    LastPar        = ElCLib1::Parameter(aCircle->Circ(), myPoint1);
      if (LastPar < FirstPar)
        LastPar += M_PI * 2;
      Handle(Select3D_SensitivePoly) circ =
        new Select3D_SensitivePoly(own, aCircle->Circ(), FirstPar, LastPar);
      aSelection->Add(circ);
    }
  }
  else
  {
    seg = new Select3D_SensitiveSegment(own, myAttachPoint1, myPoint1);
    aSelection->Add(seg);
  }

  if (mySShape.ShapeType() == TopAbs_EDGE)
  {
    BRepAdaptor_Curve aCurve(TopoDS::Edge(mySShape));
    if (aCurve.GetType() == GeomAbs_Line)
    {
      // add sensitive element - line
      seg = new Select3D_SensitiveSegment(own, myAttachPoint2, myPoint2);
      aSelection->Add(seg);
    }
    else if (aCurve.GetType() == GeomAbs_Circle)
    {
      Handle(GeomCircle) aCircle  = Handle(GeomCircle)::DownCast(aCurve.Curve().Curve());
      Standard_Real       FirstPar = ElCLib1::Parameter(aCircle->Circ(), myAttachPoint2),
                    LastPar        = ElCLib1::Parameter(aCircle->Circ(), myPoint2);
      if (LastPar < FirstPar)
        LastPar += M_PI * 2;
      Handle(Select3D_SensitivePoly) circ =
        new Select3D_SensitivePoly(own, aCircle->Circ(), FirstPar, LastPar);
      aSelection->Add(circ);
    }
  }
  else
  {
    seg = new Select3D_SensitiveSegment(own, myAttachPoint2, myPoint2);
    aSelection->Add(seg);
  }

  if (myShape3.ShapeType() == TopAbs_EDGE)
  {
    BRepAdaptor_Curve aCurve(TopoDS::Edge(myShape3));
    if (aCurve.GetType() == GeomAbs_Line)
    {
      // add sensitive element - line
      seg = new Select3D_SensitiveSegment(own, myAttachPoint3, myPoint3);
      aSelection->Add(seg);
    }
    else if (aCurve.GetType() == GeomAbs_Circle)
    {
      Handle(GeomCircle) aCircle  = Handle(GeomCircle)::DownCast(aCurve.Curve().Curve());
      Standard_Real       FirstPar = ElCLib1::Parameter(aCircle->Circ(), myAttachPoint3),
                    LastPar        = ElCLib1::Parameter(aCircle->Circ(), myPoint3);
      if (LastPar < FirstPar)
        LastPar += M_PI * 2;
      Handle(Select3D_SensitivePoly) circ =
        new Select3D_SensitivePoly(own, aCircle->Circ(), FirstPar, LastPar);
      aSelection->Add(circ);
    }
    else
    {
      seg = new Select3D_SensitiveSegment(own, myAttachPoint3, myPoint3);
      aSelection->Add(seg);
    }
  }
  else
  {
    seg = new Select3D_SensitiveSegment(own, myAttachPoint3, myPoint3);
    aSelection->Add(seg);
  }

  if (myShape4.ShapeType() == TopAbs_EDGE)
  {
    BRepAdaptor_Curve aCurve(TopoDS::Edge(myShape4));
    if (aCurve.GetType() == GeomAbs_Line)
    {
      // add sensitive element - line
      seg = new Select3D_SensitiveSegment(own, myAttachPoint4, myPoint4);
      aSelection->Add(seg);
    }
    else if (aCurve.GetType() == GeomAbs_Circle)
    {
      Handle(GeomCircle) aCircle  = Handle(GeomCircle)::DownCast(aCurve.Curve().Curve());
      Standard_Real       FirstPar = ElCLib1::Parameter(aCircle->Circ(), myAttachPoint4),
                    LastPar        = ElCLib1::Parameter(aCircle->Circ(), myPoint4);
      if (LastPar < FirstPar)
        LastPar += M_PI * 2;
      Handle(Select3D_SensitivePoly) circ =
        new Select3D_SensitivePoly(own, aCircle->Circ(), FirstPar, LastPar);
      aSelection->Add(circ);
    }
  }
  else
  {
    seg = new Select3D_SensitiveSegment(own, myAttachPoint4, myPoint4);
    aSelection->Add(seg);
  }
}

//=================================================================================================

void PrsDim_EqualDistanceRelation::ComputeTwoEdgesLength(
  const Handle(Prs3d_Presentation)& aPresentation,
  const Handle(StyleDrawer)&       aDrawer,
  const Standard_Real               ArrowSize,
  const TopoEdge&                FirstEdge,
  const TopoEdge&                SecondEdge,
  const Handle(GeomPlane)&         Plane1,
  const Standard_Boolean            AutomaticPos,
  const Standard_Boolean            IsSetBndBox,
  const Box2&                    BndBox,
  Point3d&                           Position1,
  Point3d&                           FirstAttach,
  Point3d&                           SecondAttach,
  Point3d&                           FirstExtreme,
  Point3d&                           SecondExtreme,
  DsgPrs_ArrowSide&                 SymbolPrs)
{
  Dir3d            DirAttach;
  BRepAdaptor_Curve cu1(FirstEdge);
  BRepAdaptor_Curve cu2(SecondEdge);

  // 3d lines
  Handle(GeomCurve3d) geom1, geom2;
  Point3d             ptat11, ptat12, ptat21, ptat22;

  Standard_Boolean   isInfinite1(Standard_False), isInfinite2(Standard_False);
  Handle(GeomCurve3d) extCurv;
  Standard_Real      arrsize = ArrowSize; // size
  Standard_Real      Val     = 0.;
  Standard_Boolean   isInPlane1, isInPlane2;

  if (!PrsDim1::ComputeGeometry(FirstEdge,
                               geom1,
                               ptat11,
                               ptat12,
                               extCurv,
                               isInfinite1,
                               isInPlane1,
                               Plane1))
    return;
  if (!PrsDim1::ComputeGeometry(SecondEdge,
                               geom2,
                               ptat21,
                               ptat22,
                               extCurv,
                               isInfinite2,
                               isInPlane2,
                               Plane1))
    return;

  aPresentation->SetInfiniteState(isInfinite1 || isInfinite2);

  if (cu1.GetType() == GeomAbs_Line && cu2.GetType() == GeomAbs_Line)
  {
    Handle(GeomLine) geom_lin1(Handle(GeomLine)::DownCast(geom1));
    Handle(GeomLine) geom_lin2(Handle(GeomLine)::DownCast(geom2));
    const gp_Lin&     l1 = geom_lin1->Lin();
    const gp_Lin&     l2 = geom_lin2->Lin();

    // Get Val value
    Val = l1.Distance(l2);

    DirAttach = l1.Direction();

    if (AutomaticPos)
    {
      // compute position of offset point
      Point3d        curpos;
      Standard_Real par1 = 0., par2 = 0.;
      if (!(isInfinite1 || isInfinite2))
      {
        par1 = ElCLib1::Parameter(l1, ptat11);
        par2 = ElCLib1::Parameter(l1, ptat21);
        if (par1 < par2)
        { // project ptat11 on l2
          Point3d p2 = ElCLib1::Value(ElCLib1::Parameter(l2, ptat11), l2);
          curpos.SetXYZ((ptat11.XYZ() + p2.XYZ()) * 0.5);
        }
        else
        { // project ptat21 on l1
          Point3d p2 = ElCLib1::Value(par2, l1);
          curpos.SetXYZ((ptat21.XYZ() + p2.XYZ()) * 0.5);
        }
      }
      else if (!isInfinite1)
      {
        par2      = ElCLib1::Parameter(l1, ptat21);
        Point3d p2 = ElCLib1::Value(par2, l1);
        curpos.SetXYZ((ptat21.XYZ() + p2.XYZ()) / 2.);
      }
      else if (!isInfinite2)
      {
        Point3d p2 = ElCLib1::Value(ElCLib1::Parameter(l2, ptat11), l2);
        curpos.SetXYZ((ptat11.XYZ() + p2.XYZ()) * 0.5);
      }
      else
        curpos.SetXYZ((l1.Location().XYZ() + l2.Location().XYZ()) * 0.5);

      // compute  offset
      Vector3d offset(DirAttach);
      offset = offset * ArrowSize * (-10.);
      curpos.Translate(offset);
      Position1 = curpos;
    }
    else
    { // project point on the plane
      Position1 = PrsDim1::ProjectPointOnPlane(Position1, Plane1->Pln());
    }

    // find attach points
    if (!isInfinite1)
    {
      if (Position1.Distance(ptat11) > Position1.Distance(ptat12))
        FirstAttach = ptat12;
      else
        FirstAttach = ptat11;
    }
    else
    {
      FirstAttach = ElCLib1::Value(ElCLib1::Parameter(l1, Position1), l1);
    }

    if (!isInfinite2)
    {
      if (Position1.Distance(ptat21) > Position1.Distance(ptat22))
        SecondAttach = ptat22;
      else
        SecondAttach = ptat21;
    }
    else
    {
      SecondAttach = ElCLib1::Value(ElCLib1::Parameter(l2, Position1), l2);
    }

    constexpr Standard_Real confusion(Precision::Confusion());
    if (arrsize < confusion)
      arrsize = Val * 0.1;
    if (Abs(Val) <= confusion)
    {
      arrsize = 0.;
    }

    Handle(Prs3d_DimensionAspect) la  = aDrawer->DimensionAspect();
    Handle(Prs3d_ArrowAspect)     arr = la->ArrowAspect();
    arr->SetLength(arrsize);
    arr = la->ArrowAspect();
    arr->SetLength(arrsize);

    if (AutomaticPos && IsSetBndBox)
      Position1 = PrsDim1::TranslatePointToBound(Position1, DirAttach, BndBox);

    EqualDistancePresentation::AddInterval(aPresentation,
                                                  aDrawer,
                                                  FirstAttach,
                                                  SecondAttach,
                                                  DirAttach,
                                                  Position1,
                                                  SymbolPrs,
                                                  FirstExtreme,
                                                  SecondExtreme);
  }
  if (cu1.GetType() == GeomAbs_Circle && cu2.GetType() == GeomAbs_Circle)
  {
    // Get first and last points of circles
    Handle(GeomCircle) aCir1(Handle(GeomCircle)::DownCast(geom1));
    Handle(GeomCircle) aCir2(Handle(GeomCircle)::DownCast(geom2));
    gp_Circ             aCirc1 = aCir1->Circ();
    gp_Circ             aCirc2 = aCir2->Circ();

    // To avoid circles with different orientation
    constexpr Standard_Real aTol = Precision::Confusion();
    if (aCirc2.Axis().IsOpposite(aCirc1.Axis(), aTol)
        || aCirc2.XAxis().IsOpposite(aCirc1.XAxis(), aTol)
        || aCirc2.YAxis().IsOpposite(aCirc1.YAxis(), aTol))
    {
      aCirc2.SetPosition(aCirc1.Position1());
      aCirc2.SetAxis(aCirc1.Axis());
    }

    if (AutomaticPos)
    {
      Standard_Real par1 = 0, par2 = 0;
      gp_Pln        aPln = Plane1->Pln();
      // Project ptat12 and ptat22 on constraint plane
      Point3d PrPnt12 = PrsDim1::ProjectPointOnPlane(ptat12, aPln);
      Point3d PrPnt22 = PrsDim1::ProjectPointOnPlane(ptat22, aPln);
      // Project circles center on constraint plane
      Point3d PrCenter = PrsDim1::ProjectPointOnPlane(aCirc1.Location(), aPln);

      Dir3d XDir = aPln.XAxis().Direction();
      Dir3d YDir = aPln.YAxis().Direction();

      if (PrPnt12.Distance(PrCenter) > Precision::Confusion())
      {
        Dir3d        aDir1(PrPnt12.XYZ() - PrCenter.XYZ());
        Standard_Real anAngle = aDir1.Angle(XDir); // Get the angle in range [0, M_PI]
        if (aDir1.Dot(YDir) < 0)
          anAngle = 2 * M_PI - anAngle;
        par1 = anAngle;
      }

      if (PrPnt22.Distance(PrCenter) > Precision::Confusion())
      {
        Dir3d        aDir2(PrPnt22.XYZ() - PrCenter.XYZ());
        Standard_Real anAngle = aDir2.Angle(XDir); // Get the angle in range [0, M_PI]
        if (aDir2.Dot(YDir) < 0)
          anAngle = 2 * M_PI - anAngle;
        par2 = anAngle;
      }

      if (par1 > par2)
      {
        FirstExtreme        = ptat12;
        Standard_Real aPar1 = ElCLib1::Parameter(aCirc2, ptat12);
        SecondExtreme       = ElCLib1::Value(aPar1, aCirc2);
      }
      else
      {
        Standard_Real aPar2 = ElCLib1::Parameter(aCirc1, ptat22);
        FirstExtreme        = ElCLib1::Value(aPar2, aCirc1);
        SecondExtreme       = ptat22;
      }
    }
    else
    {
      Standard_Real pospar = ElCLib1::Parameter(aCirc1, Position1);
      FirstExtreme         = ElCLib1::Value(pospar, aCirc1);
      pospar               = ElCLib1::Parameter(aCirc2, Position1);
      SecondExtreme        = ElCLib1::Value(pospar, aCirc2);
    }

    EqualDistancePresentation::AddIntervalBetweenTwoArcs(aPresentation,
                                                                aDrawer,
                                                                aCirc1,
                                                                aCirc2,
                                                                ptat12,
                                                                FirstExtreme,
                                                                ptat22,
                                                                SecondExtreme,
                                                                SymbolPrs);

    FirstAttach  = ptat12;
    SecondAttach = ptat22; // assign attach points
    Position1.SetXYZ((FirstAttach.XYZ() + SecondAttach.XYZ()) * 0.5);
  }

  if (arrsize < Precision::Confusion())
    arrsize = Val * 0.1;
  if (Abs(Val) <= Precision::Confusion())
  {
    arrsize = 0.;
  }

  //  Point3d pf, pl;
  if (!isInPlane1)
  {
    PrsDim1::ComputeProjEdgePresentation(aPresentation, aDrawer, FirstEdge, geom1, ptat11, ptat12);
  }
  if (!isInPlane2)
  {
    PrsDim1::ComputeProjEdgePresentation(aPresentation, aDrawer, SecondEdge, geom2, ptat21, ptat22);
  }
}

//=================================================================================================

void PrsDim_EqualDistanceRelation::ComputeTwoVerticesLength(
  const Handle(Prs3d_Presentation)& aPresentation,
  const Handle(StyleDrawer)&       aDrawer,
  const Standard_Real               ArrowSize,
  const TopoVertex&              FirstVertex,
  const TopoVertex&              SecondVertex,
  const Handle(GeomPlane)&         Plane1,
  const Standard_Boolean            AutomaticPos,
  const Standard_Boolean            IsSetBndBox,
  const Box2&                    BndBox,
  const PrsDim_TypeOfDist           TypeDist,
  Point3d&                           Position1,
  Point3d&                           FirstAttach,
  Point3d&                           SecondAttach,
  Point3d&                           FirstExtreme,
  Point3d&                           SecondExtreme,
  DsgPrs_ArrowSide&                 SymbolPrs)
{
  Standard_Boolean isOnPlane1, isOnPlane2;
  Dir3d           DirAttach;
  PrsDim1::ComputeGeometry(FirstVertex, FirstAttach, Plane1, isOnPlane1);
  PrsDim1::ComputeGeometry(SecondVertex, SecondAttach, Plane1, isOnPlane2);

  constexpr Standard_Real confusion(Precision::Confusion());
  Standard_Boolean        samePoint(FirstAttach.IsEqual(SecondAttach, confusion));

  if (TypeDist == PrsDim_TypeOfDist_Vertical)
    DirAttach = Plane1->Pln().XAxis().Direction();
  else if (TypeDist == PrsDim_TypeOfDist_Horizontal)
    DirAttach = Plane1->Pln().YAxis().Direction();
  else
  {
    if (!samePoint)
    {
      DirAttach.SetXYZ(SecondAttach.XYZ() - FirstAttach.XYZ());
      DirAttach.Rotate(Plane1->Pln().Axis(), M_PI / 2.);
    }
  }

  // size
  if (AutomaticPos)
  {
    if (!samePoint)
    {
      Point3d curpos((FirstAttach.XYZ() + SecondAttach.XYZ()) * 0.5);
      // make offset of curpos
      Vector3d offset(DirAttach);
      offset = offset * ArrowSize * (-10.);
      curpos.Translate(offset);
      Position1 = curpos;
    }
    else
    {
      Dir3d aDir = Plane1->Pln().Axis().Direction();
      Vector3d aVec(aDir.XYZ() * 10 * ArrowSize);
      // Position1 = Point3d(FirstAttach.XYZ()+Coords3d(1.,1.,1.)); // not correct
      Position1 = FirstAttach.Translated(aVec);
      Position1 = PrsDim1::ProjectPointOnPlane(Position1, Plane1->Pln()); // not needed really
      DirAttach.SetXYZ(Position1.XYZ() - FirstAttach.XYZ());
    }
  }
  else
  {
    Position1 = PrsDim1::ProjectPointOnPlane(Position1, Plane1->Pln());
  }

  Handle(Prs3d_DimensionAspect) la  = aDrawer->DimensionAspect();
  Handle(Prs3d_ArrowAspect)     arr = la->ArrowAspect();
  arr->SetLength(ArrowSize);
  arr = la->ArrowAspect();
  arr->SetLength(ArrowSize);

  if (AutomaticPos && IsSetBndBox)
    Position1 = PrsDim1::TranslatePointToBound(Position1, DirAttach, BndBox);

  EqualDistancePresentation::AddInterval(aPresentation,
                                                aDrawer,
                                                FirstAttach,
                                                SecondAttach,
                                                DirAttach,
                                                Position1,
                                                SymbolPrs,
                                                FirstExtreme,   // returned
                                                SecondExtreme); // returned

  // Compute projection
  if (!isOnPlane1)
    PrsDim1::ComputeProjVertexPresentation(aPresentation, aDrawer, FirstVertex, FirstAttach);
  if (!isOnPlane2)
    PrsDim1::ComputeProjVertexPresentation(aPresentation, aDrawer, SecondVertex, SecondAttach);
}

//=================================================================================================

void PrsDim_EqualDistanceRelation::ComputeOneEdgeOneVertexLength(
  const Handle(Prs3d_Presentation)& aPresentation,
  const Handle(StyleDrawer)&       aDrawer,
  const Standard_Real               ArrowSize,
  const TopoShape&               FirstShape,
  const TopoShape&               SecondShape,
  const Handle(GeomPlane)&         Plane1,
  const Standard_Boolean            AutomaticPos,
  const Standard_Boolean            IsSetBndBox,
  const Box2&                    BndBox,
  Point3d&                           Position1,
  Point3d&                           FirstAttach,
  Point3d&                           SecondAttach,
  Point3d&                           FirstExtreme,
  Point3d&                           SecondExtreme,
  DsgPrs_ArrowSide&                 SymbolPrs)
{
  TopoVertex thevertex;
  TopoEdge   theedge;

  Point3d             ptonedge1, ptonedge2;
  Handle(GeomCurve3d) aCurve;
  Handle(GeomCurve3d) extCurv;
  Standard_Boolean   isInfinite;
  Standard_Real      Val;
  Standard_Boolean   isOnPlanEdge, isOnPlanVertex;
  Standard_Integer   edgenum;

  if (FirstShape.ShapeType() == TopAbs_VERTEX)
  {
    thevertex = TopoDS::Vertex(FirstShape);
    theedge   = TopoDS::Edge(SecondShape);
    edgenum   = 2; // edge is the second shape
  }
  else
  {
    thevertex = TopoDS::Vertex(SecondShape);
    theedge   = TopoDS::Edge(FirstShape);
    edgenum   = 1; // edge is the first shape
  }
  if (!PrsDim1::ComputeGeometry(theedge,
                               aCurve,
                               ptonedge1,
                               ptonedge2,
                               extCurv,
                               isInfinite,
                               isOnPlanEdge,
                               Plane1))
    return;
  aPresentation->SetInfiniteState(isInfinite);
  PrsDim1::ComputeGeometry(thevertex, FirstAttach, Plane1, isOnPlanVertex);

  if (aCurve->IsInstance(STANDARD_TYPE(GeomLine)))
  {
    Handle(GeomLine) geom_lin(Handle(GeomLine)::DownCast(aCurve));
    const gp_Lin&     l = geom_lin->Lin();

    // computation of Val
    Val = l.Distance(FirstAttach);

    Dir3d DirAttach = l.Direction();
    // size
    Standard_Real arrsize = ArrowSize;
    if (Abs(Val) <= Precision::Confusion())
    {
      arrsize = 0.;
    }

    if (AutomaticPos)
    {
      Point3d p = ElCLib1::Value(ElCLib1::Parameter(l, FirstAttach), l);
      Point3d curpos((FirstAttach.XYZ() + p.XYZ()) * 0.5);
      // make offset
      Vector3d offset(DirAttach);
      offset = offset * ArrowSize * (-10.);
      curpos.Translate(offset);
      Position1 = curpos;
    }
    else
    { // project point on the plane
      Position1 = PrsDim1::ProjectPointOnPlane(Position1, Plane1->Pln());
    }

    if (!isInfinite)
    {
      if (Position1.Distance(ptonedge1) > Position1.Distance(ptonedge2))
        SecondAttach = ptonedge2;
      else
        SecondAttach = ptonedge1;
    }
    else
    {
      SecondAttach = ElCLib1::Value(ElCLib1::Parameter(l, Position1), l);
    }

    Handle(Prs3d_DimensionAspect) la  = aDrawer->DimensionAspect();
    Handle(Prs3d_ArrowAspect)     arr = la->ArrowAspect();
    arr->SetLength(arrsize);
    arr = la->ArrowAspect();
    arr->SetLength(arrsize);

    if (AutomaticPos && IsSetBndBox)
      Position1 = PrsDim1::TranslatePointToBound(Position1, DirAttach, BndBox);
    EqualDistancePresentation::AddInterval(aPresentation,
                                                  aDrawer,
                                                  FirstAttach,
                                                  SecondAttach,
                                                  DirAttach,
                                                  Position1,
                                                  SymbolPrs,
                                                  FirstExtreme,
                                                  SecondExtreme);
  }
  if (aCurve->IsInstance(STANDARD_TYPE(GeomCircle)))
  {
    gp_Circ aCirc1 = (Handle(GeomCircle)::DownCast(aCurve))->Circ();
    gp_Circ aCirc2(aCirc1);
    aCirc2.SetRadius(0); // create the second formal circle
    if (AutomaticPos)
    {
      SecondAttach = ptonedge2; // a vertex
      Position1.SetXYZ((SecondAttach.XYZ() + aCirc1.Location().XYZ()) * 0.5);
    }
    else
    {
      Standard_Real aPar = ElCLib1::Parameter(aCirc1, Position1);
      SecondAttach       = ElCLib1::Value(aPar, aCirc1);
    }

    Handle(GeomCircle) aCurve2 = new GeomCircle(aCirc2);
    EqualDistancePresentation::AddIntervalBetweenTwoArcs(aPresentation,
                                                                aDrawer,
                                                                aCirc1,    // circle or arc
                                                                aCirc2,    // really vertex
                                                                ptonedge2, // last point of aCirc1
                                                                SecondAttach,
                                                                FirstAttach, // vertex really
                                                                FirstAttach,
                                                                SymbolPrs);

    // Assign arc points
    if (edgenum == 1)
    {
      FirstExtreme  = SecondAttach;
      SecondExtreme = FirstAttach;
      SecondAttach  = FirstAttach;
      FirstAttach   = ptonedge2;
    }
    else
    { // vertex is the first shape, circle is sthe last.
      FirstExtreme  = FirstAttach;
      SecondExtreme = SecondAttach;
      SecondAttach  = ptonedge2;
    }
  }

  // computation of Val
  Val = FirstAttach.Distance(SecondAttach);

  // Display the pieces of attached to the curve if it is not
  //  in the WP
  if (!isOnPlanEdge)
  { // add presentation of projection of the edge in WP
    PrsDim1::ComputeProjEdgePresentation(aPresentation,
                                        aDrawer,
                                        theedge,
                                        aCurve,
                                        ptonedge1,
                                        ptonedge2);
  }
  if (!isOnPlanVertex)
  { // add presentation of projection of the vertex in WP
    PrsDim1::ComputeProjVertexPresentation(aPresentation, aDrawer, thevertex, FirstAttach);
  }
}

// -- ota -- end
