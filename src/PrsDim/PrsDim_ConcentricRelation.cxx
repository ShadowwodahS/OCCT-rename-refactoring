// Created on: 1996-12-05
// Created by: Flore Lantheaume/Odile Olivier
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

#include <PrsDim_ConcentricRelation.hxx>

#include <PrsDim.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <DsgPrs_ConcentricPresentation.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Plane.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Prs3d_Presentation.hxx>
#include <Select3D_SensitiveCircle.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_ConcentricRelation, PrsDim_Relation)

//=================================================================================================

PrsDim_ConcentricRelation::PrsDim_ConcentricRelation(const TopoShape&       aFShape,
                                                     const TopoShape&       aSShape,
                                                     const Handle(GeomPlane)& aPlane)
{
  myFShape = aFShape;
  mySShape = aSShape;
  myPlane  = aPlane;
  myDir    = aPlane->Pln().Axis().Direction();
}

//=================================================================================================

void PrsDim_ConcentricRelation::Compute(const Handle(PrsMgr_PresentationManager)&,
                                        const Handle(Prs3d_Presentation)& aPresentation,
                                        const Standard_Integer)
{
  TopAbs_ShapeEnum type2(mySShape.ShapeType());
  aPresentation->SetInfiniteState(Standard_True);
  switch (myFShape.ShapeType())
  {
    case TopAbs_EDGE: {
      if (type2 == TopAbs_EDGE)
        ComputeTwoEdgesConcentric(aPresentation);
      else if (type2 == TopAbs_VERTEX)
        ComputeEdgeVertexConcentric(aPresentation);
    }
    break;

    case TopAbs_VERTEX: {
      if (type2 == TopAbs_VERTEX)
        ComputeTwoVerticesConcentric(aPresentation);
      else if (type2 == TopAbs_EDGE)
        ComputeEdgeVertexConcentric(aPresentation);
    }
    break;
    default: {
      return;
    }
  }
}

//=================================================================================================

void PrsDim_ConcentricRelation::ComputeEdgeVertexConcentric(
  const Handle(Prs3d_Presentation)& aPresentation)
{
  TopoEdge   E;
  TopoVertex V;
  if (myFShape.ShapeType() == TopAbs_EDGE)
  {
    E = TopoDS::Edge(myFShape);
    V = TopoDS::Vertex(mySShape);
  }
  else
  {
    E = TopoDS::Edge(mySShape);
    V = TopoDS::Vertex(myFShape);
  }
  Point3d             p1, p2;
  Handle(GeomCurve3d) C;
  Handle(GeomCurve3d) extCurv;
  Standard_Boolean   isInfinite;
  Standard_Boolean   isOnPlanEdge, isOnPlanVertex;
  if (!PrsDim1::ComputeGeometry(E, C, p1, p2, extCurv, isInfinite, isOnPlanEdge, myPlane))
    return;
  Point3d P;
  PrsDim1::ComputeGeometry(V, P, myPlane, isOnPlanVertex);

  Handle(GeomCircle) CIRCLE(Handle(GeomCircle)::DownCast(C));
  myCenter = CIRCLE->Location();
  myRad    = Min(CIRCLE->Radius() / 5., 15.);
  Dir3d vec(p1.XYZ() - myCenter.XYZ());
  Vector3d vectrans(vec);
  myPnt = myCenter.Translated(vectrans.Multiplied(myRad));
  ConcentricPresentation::Add(aPresentation, myDrawer, myCenter, myRad, myDir, myPnt);
  if (!isOnPlanEdge)
    PrsDim1::ComputeProjEdgePresentation(aPresentation, myDrawer, E, CIRCLE, p1, p2);
  if (!isOnPlanVertex)
    PrsDim1::ComputeProjVertexPresentation(aPresentation, myDrawer, V, P);
}

//=================================================================================================

void PrsDim_ConcentricRelation::ComputeTwoVerticesConcentric(
  const Handle(Prs3d_Presentation)& aPresentation)
{
  TopoVertex V1, V2;
  V1 = TopoDS::Vertex(myFShape);
  V2 = TopoDS::Vertex(myFShape);
  Standard_Boolean isOnPlanVertex1(Standard_True), isOnPlanVertex2(Standard_True);
  Point3d           P1, P2;
  PrsDim1::ComputeGeometry(V1, P1, myPlane, isOnPlanVertex1);
  PrsDim1::ComputeGeometry(V2, P2, myPlane, isOnPlanVertex2);
  myCenter = P1;
  myRad    = 15.;
  Dir3d vec(myPlane->Pln().Position1().XDirection());
  Vector3d vectrans(vec);
  myPnt = myCenter.Translated(vectrans.Multiplied(myRad));
  ConcentricPresentation::Add(aPresentation, myDrawer, myCenter, myRad, myDir, myPnt);
  if (!isOnPlanVertex1)
    PrsDim1::ComputeProjVertexPresentation(aPresentation, myDrawer, V1, P1);
  if (!isOnPlanVertex2)
    PrsDim1::ComputeProjVertexPresentation(aPresentation, myDrawer, V2, P2);
}

//=================================================================================================

void PrsDim_ConcentricRelation::ComputeTwoEdgesConcentric(
  const Handle(Prs3d_Presentation)& aPresentation)
{
  BRepAdaptor_Curve curv1(TopoDS::Edge(myFShape));
  BRepAdaptor_Curve curv2(TopoDS::Edge(mySShape));

  Point3d             ptat11, ptat12, ptat21, ptat22;
  Handle(GeomCurve3d) geom1, geom2;
  Standard_Boolean   isInfinite1, isInfinite2;
  Handle(GeomCurve3d) extCurv;
  if (!PrsDim1::ComputeGeometry(TopoDS::Edge(myFShape),
                               TopoDS::Edge(mySShape),
                               myExtShape,
                               geom1,
                               geom2,
                               ptat11,
                               ptat12,
                               ptat21,
                               ptat22,
                               extCurv,
                               isInfinite1,
                               isInfinite2,
                               myPlane))
  {
    return;
  }

  Handle(GeomCircle) gcirc1(Handle(GeomCircle)::DownCast(geom1));
  Handle(GeomCircle) gcirc2(Handle(GeomCircle)::DownCast(geom2));

  myCenter = gcirc1->Location();

  // choose the radius equal to 1/5 of the smallest radius of
  // 2 circles. Limit is imposed ( 0.02 by chance)
  Standard_Real aRad1 = gcirc1->Radius();
  Standard_Real aRad2 = gcirc2->Radius();
  myRad               = (aRad1 > aRad2) ? aRad2 : aRad1;
  myRad /= 5;
  if (myRad > 15.)
    myRad = 15.;

  // Calculate a point of circle of radius myRad
  Dir3d vec(ptat11.XYZ() - myCenter.XYZ());
  Vector3d vectrans(vec);
  myPnt = myCenter.Translated(vectrans.Multiplied(myRad));

  ConcentricPresentation::Add(aPresentation, myDrawer, myCenter, myRad, myDir, myPnt);
  if ((myExtShape != 0) && !extCurv.IsNull())
  {
    Point3d pf, pl;
    if (myExtShape == 1)
    {
      if (!isInfinite1)
      {
        pf = ptat11;
        pl = ptat12;
      }
      ComputeProjEdgePresentation(aPresentation, TopoDS::Edge(myFShape), gcirc1, pf, pl);
    }
    else
    {
      if (!isInfinite2)
      {
        pf = ptat21;
        pl = ptat22;
      }
      ComputeProjEdgePresentation(aPresentation, TopoDS::Edge(mySShape), gcirc2, pf, pl);
    }
  }
}

//=================================================================================================

void PrsDim_ConcentricRelation::ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                                 const Standard_Integer)
{
  Handle(SelectMgr_EntityOwner) anOwner = new SelectMgr_EntityOwner(this, 7);

  // Creation of 2 sensitive circles

  // the greater
  Frame3d                           anAx(myCenter, myDir);
  gp_Circ                          aCirc(anAx, myRad);
  Handle(Select3D_SensitiveCircle) sensit = new Select3D_SensitiveCircle(anOwner, aCirc);
  aSelection->Add(sensit);

  // the smaller
  aCirc.SetRadius(myRad / 2);
  sensit = new Select3D_SensitiveCircle(anOwner, aCirc);
  aSelection->Add(sensit);

  // Creation of 2 segments sensitive for the cross
  Handle(Select3D_SensitiveSegment) seg;
  Point3d                            otherPnt = myPnt.Mirrored(myCenter);
  seg = new Select3D_SensitiveSegment(anOwner, otherPnt, myPnt);
  aSelection->Add(seg);

  Axis3d RotateAxis(myCenter, myDir);
  Point3d FPnt = myCenter.Rotated(RotateAxis, M_PI_2);
  Point3d SPnt = myCenter.Rotated(RotateAxis, -M_PI_2);
  seg         = new Select3D_SensitiveSegment(anOwner, FPnt, SPnt);
  aSelection->Add(seg);
}
