// Created by: Peter KURNEV
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

#include <BOPTools_AlgoTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Curve.hxx>
#include <GeomAbs_CurveType.hxx>
#include <gp_Pnt.hxx>
#include <IntTools_Context.hxx>
#include <IntTools_Curve.hxx>
#include <IntTools_Range.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

//=================================================================================================

void AlgoTools::UpdateVertex(const TopoVertex& aVF, const TopoVertex& aNewVertex)
{
  Standard_Real aTolVF, aTolNewVertex, aDist, aNewTol;
  //
  Point3d aPVF        = BRepInspector::Pnt(aVF);
  Point3d aPNewVertex = BRepInspector::Pnt(aNewVertex);
  aTolVF             = BRepInspector::Tolerance(aVF);
  aTolNewVertex      = BRepInspector::Tolerance(aNewVertex);

  aDist   = aPVF.Distance(aPNewVertex);
  aNewTol = aDist + aTolNewVertex;

  if (aNewTol > aTolVF)
  {
    ShapeBuilder BB;
    BB.UpdateVertex(aVF, aNewTol + AlgoTools::DTolerance());
  }
}

//=================================================================================================

void AlgoTools::UpdateVertex(const TopoEdge&   aE,
                                      const Standard_Real  aT,
                                      const TopoVertex& aV)
{
  Standard_Real aTolV, aDist, aFirst, aLast;
  Point3d        aPc;

  Point3d aPv = BRepInspector::Pnt(aV);
  aTolV      = BRepInspector::Tolerance(aV);

  GeomAdaptor_Curve aCA(BRepInspector::Curve(aE, aFirst, aLast));
  aCA.D0(aT, aPc);
  aDist = aPv.Distance(aPc);
  if (aDist > aTolV)
  {
    ShapeBuilder BB;
    BB.UpdateVertex(aV, aDist + AlgoTools::DTolerance());
  }
}

//
//=================================================================================================

void AlgoTools::UpdateVertex(const IntTools_Curve& aC,
                                      const Standard_Real   aT,
                                      const TopoVertex&  aV)
{
  Standard_Real aTolV, aDist;
  Point3d        aPc;

  Point3d aPv = BRepInspector::Pnt(aV);
  aTolV      = BRepInspector::Tolerance(aV);

  GeomAdaptor_Curve aCA(aC.Curve());
  aCA.D0(aT, aPc);
  aDist = aPv.Distance(aPc);
  if (aDist > aTolV)
  {
    ShapeBuilder BB;
    BB.UpdateVertex(aV, aDist + AlgoTools::DTolerance());
  }
}

//=================================================================================================

void AlgoTools::MakeSectEdge(const IntTools_Curve& aIC,
                                      const TopoVertex&  aV1,
                                      const Standard_Real   aP1,
                                      const TopoVertex&  aV2,
                                      const Standard_Real   aP2,
                                      TopoEdge&          aNewEdge)
{
  const Handle(GeomCurve3d)& aC = aIC.Curve();

  EdgeMaker aMakeEdge(aC, aV1, aV2, aP1, aP2);

  const TopoEdge& aE = TopoDS::Edge(aMakeEdge.Shape());
  //
  // Range must be as it was !
  ShapeBuilder aBB;
  aBB.Range(aE, aP1, aP2);
  //
  aNewEdge = aE;
}

//=================================================================================================

TopoEdge AlgoTools::CopyEdge(const TopoEdge& theEdge)
{
  TopoEdge aNewEdge = TopoDS::Edge(theEdge.Oriented(TopAbs_FORWARD));
  aNewEdge.EmptyCopy();
  for (TopoDS_Iterator it(theEdge, Standard_False); it.More(); it.Next())
    ShapeBuilder().Add(aNewEdge, it.Value());
  aNewEdge.Orientation(theEdge.Orientation());
  return aNewEdge;
}

//=================================================================================================

void AlgoTools::MakeSplitEdge(const TopoEdge&   aE,
                                       const TopoVertex& aV1,
                                       const Standard_Real  aP1,
                                       const TopoVertex& aV2,
                                       const Standard_Real  aP2,
                                       TopoEdge&         aNewEdge)
{
  TopoEdge E = TopoDS::Edge(aE.Oriented(TopAbs_FORWARD));
  E.EmptyCopy();
  //
  ShapeBuilder BB;
  if (!aV1.IsNull())
  {
    if (aP1 < aP2)
    {
      BB.Add(E, TopoDS::Vertex(aV1.Oriented(TopAbs_FORWARD)));
    }
    else
    {
      BB.Add(E, TopoDS::Vertex(aV1.Oriented(TopAbs_REVERSED)));
    }
  }
  if (!aV2.IsNull())
  {
    if (aP1 < aP2)
    {
      BB.Add(E, TopoDS::Vertex(aV2.Oriented(TopAbs_REVERSED)));
    }
    else
    {
      BB.Add(E, TopoDS::Vertex(aV2.Oriented(TopAbs_FORWARD)));
    }
  }

  if (aP1 < aP2)
  {
    BB.Range(E, aP1, aP2);
  }
  else
  {
    BB.Range(E, aP2, aP1);
  }

  aNewEdge = E;
  aNewEdge.Orientation(aE.Orientation());
}

//=================================================================================================

void AlgoTools::MakeNewVertex(const TopoVertex& aV1,
                                       const TopoVertex& aV2,
                                       TopoVertex&       aNewVertex)
{
  Point3d        aPnt1 = BRepInspector::Pnt(aV1);
  Standard_Real aTol1 = BRepInspector::Tolerance(aV1);

  Point3d        aPnt2 = BRepInspector::Pnt(aV2);
  Standard_Real aTol2 = BRepInspector::Tolerance(aV2);

  Standard_Real aMaxTol, aDist;

  aDist   = aPnt1.Distance(aPnt2);
  aMaxTol = (aTol1 > aTol2) ? aTol1 : aTol2;
  aMaxTol = aMaxTol + 0.5 * aDist;

  const Coords3d& aXYZ1   = aPnt1.XYZ();
  const Coords3d& aXYZ2   = aPnt2.XYZ();
  Coords3d        aNewXYZ = 0.5 * (aXYZ1 + aXYZ2);

  Point3d       aNewPnt(aNewXYZ);
  ShapeBuilder aBB;
  aBB.MakeVertex(aNewVertex, aNewPnt, aMaxTol);
}

//=================================================================================================

void AlgoTools::MakeNewVertex(const Point3d&       aP,
                                       const Standard_Real aTol,
                                       TopoVertex&      aNewVertex)
{
  ShapeBuilder aBB;
  aBB.MakeVertex(aNewVertex, aP, aTol);
}

//=================================================================================================

void AlgoTools::MakeNewVertex(const TopoEdge&  aE1,
                                       const Standard_Real aParm1,
                                       const TopoEdge&  aE2,
                                       const Standard_Real aParm2,
                                       TopoVertex&      aNewVertex)
{
  Standard_Real aTol1, aTol2, aMaxTol, aDist;
  Point3d        aPnt1, aPnt2;

  PointOnEdge(aE1, aParm1, aPnt1);
  PointOnEdge(aE2, aParm2, aPnt2);

  aTol1 = BRepInspector::Tolerance(aE1);
  aTol2 = BRepInspector::Tolerance(aE2);

  aDist   = aPnt1.Distance(aPnt2);
  aMaxTol = (aTol1 > aTol2) ? aTol1 : aTol2;
  aMaxTol = aMaxTol + 0.5 * aDist;

  const Coords3d& aXYZ1   = aPnt1.XYZ();
  const Coords3d& aXYZ2   = aPnt2.XYZ();
  Coords3d        aNewXYZ = 0.5 * (aXYZ1 + aXYZ2);

  Point3d       aNewPnt(aNewXYZ);
  ShapeBuilder aBB;
  aBB.MakeVertex(aNewVertex, aNewPnt, aMaxTol);
}

//=================================================================================================

void AlgoTools::MakeNewVertex(const TopoEdge&  aE1,
                                       const Standard_Real aParm1,
                                       const TopoFace&  aF1,
                                       TopoVertex&      aNewVertex)
{
  Standard_Real aTol1, aTol2, aMaxTol;
  Point3d        aPnt;

  PointOnEdge(aE1, aParm1, aPnt);

  aTol1 = BRepInspector::Tolerance(aE1);
  aTol2 = BRepInspector::Tolerance(aF1);
  //
  aMaxTol = aTol1 + aTol2 + AlgoTools::DTolerance();
  //
  ShapeBuilder aBB;
  aBB.MakeVertex(aNewVertex, aPnt, aMaxTol);
}

//=================================================================================================

void AlgoTools::PointOnEdge(const TopoEdge& aE, const Standard_Real aParm, Point3d& aPnt)
{
  Standard_Real      f, l;
  Handle(GeomCurve3d) C1 = BRepInspector::Curve(aE, f, l);
  C1->D0(aParm, aPnt);
}

//=================================================================================================

void AlgoTools::CorrectRange(const TopoEdge&    aE1,
                                      const TopoEdge&    aE2,
                                      const IntToolsRange& aSR,
                                      IntToolsRange&       aNewSR)
{
  Standard_Integer  i;
  Standard_Real     aRes, aTolE1, aTolE2, aTF, aTL, dT;
  BRepAdaptor_Curve aBC;
  GeomAbs_CurveType aCT;
  Point3d            aP;
  Vector3d            aDer;
  //
  aNewSR = aSR;
  //
  aBC.Initialize(aE1);
  aCT = aBC.GetType();
  if (aCT == GeomAbs_Line)
  {
    return;
  }
  //
  dT  = Precision1::PConfusion();
  aTF = aSR.First();
  aTL = aSR.Last();
  //
  aTolE1 = BRepInspector::Tolerance(aE1);
  aTolE2 = BRepInspector::Tolerance(aE2);
  //
  for (i = 0; i < 2; ++i)
  {
    aRes = 2. * (aTolE1 + aTolE2);
    //
    if (aCT == GeomAbs_BezierCurve || aCT == GeomAbs_BSplineCurve || aCT == GeomAbs_OffsetCurve
        || aCT == GeomAbs_OtherCurve)
    {

      if (!i)
      {
        aBC.D1(aTF, aP, aDer);
      }
      else
      {
        aBC.D1(aTL, aP, aDer);
      }
      //
      Standard_Real aMgn = aDer.Magnitude();

      if (aMgn > 1.e-12)
      {
        aRes = aRes / aMgn;
      }
      else
      {
        aRes = aBC.Resolution(aRes);
      }
    } // if (aCT==GeomAbs_BezierCurve||...
    else
    {
      aRes = aBC.Resolution(aRes);
    }
    //
    if (!i)
    {
      aNewSR.SetFirst(aTF + aRes);
    }
    else
    {
      aNewSR.SetLast(aTL - aRes);
    }
    //
    if ((aNewSR.Last() - aNewSR.First()) < dT)
    {
      aNewSR = aSR;
    }
    // aNewSR=((aNewSR.Last()-aNewSR.First()) < dT) ? aSR : aNewSR;
  }
}

//=================================================================================================

void AlgoTools::CorrectRange(const TopoEdge&    aE,
                                      const TopoFace&    aF,
                                      const IntToolsRange& aSR,
                                      IntToolsRange&       aNewSR)
{
  Standard_Integer  i;
  Standard_Real     aRes, aTolF, aTF, aTL, dT;
  BRepAdaptor_Curve aBC;
  GeomAbs_CurveType aCT;
  Point3d            aP;
  Vector3d            aDer;
  //
  aNewSR = aSR;
  //
  dT  = Precision1::PConfusion();
  aTF = aSR.First();
  aTL = aSR.Last();
  //
  aBC.Initialize(aE);
  aCT = aBC.GetType();
  //
  aTolF = BRepInspector::Tolerance(aF);
  //
  for (i = 0; i < 2; ++i)
  {
    aRes = aTolF;

    if (aCT == GeomAbs_BezierCurve || aCT == GeomAbs_BSplineCurve || aCT == GeomAbs_OffsetCurve
        || aCT == GeomAbs_OtherCurve)
    {

      if (!i)
      {
        aBC.D1(aTF, aP, aDer);
      }
      else
      {
        aBC.D1(aTL, aP, aDer);
      }
      //
      Standard_Real aMgn = aDer.Magnitude();

      if (aMgn > 1.e-12)
      {
        aRes = aRes / aMgn;
      }
      else
      {
        aRes = aBC.Resolution(aRes);
      }
    } // if (aCT==GeomAbs_BezierCurve||...
    else
    {
      aRes = aBC.Resolution(aRes);
    }
    //
    if (!i)
    {
      aNewSR.SetFirst(aTF + aRes);
    }
    else
    {
      aNewSR.SetLast(aTL - aRes);
    }
    //
    if ((aNewSR.Last() - aNewSR.First()) < dT)
    {
      aNewSR = aSR;
    }
  }
}

namespace
{

//=======================================================================
// function : dimension
// purpose  : returns dimension of elementary shape
//=======================================================================
static Standard_Integer dimension(const TopoShape& theS)
{
  switch (theS.ShapeType())
  {
    case TopAbs_VERTEX:
      return 0;
    case TopAbs_EDGE:
    case TopAbs_WIRE:
      return 1;
    case TopAbs_FACE:
    case TopAbs_SHELL:
      return 2;
    case TopAbs_SOLID:
    case TopAbs_COMPSOLID:
      return 3;
    default:
      return -1;
  }
}

} // namespace

//=================================================================================================

void AlgoTools::Dimensions(const TopoShape& theS,
                                    Standard_Integer&   theDMin,
                                    Standard_Integer&   theDMax)
{
  theDMin = theDMax = dimension(theS);
  if (theDMax >= 0)
    return;

  ShapeList aLS;
  TopTools_MapOfShape  aMFence;
  TreatCompound(theS, aLS, &aMFence);
  if (aLS.IsEmpty())
  {
    // empty shape
    theDMin = theDMax = -1;
    return;
  }

  theDMin = 3;
  theDMax = 0;
  for (ShapeList::Iterator it(aLS); it.More(); it.Next())
  {
    Standard_Integer aDim = dimension(it.Value());
    if (aDim < theDMin)
      theDMin = aDim;
    if (aDim > theDMax)
      theDMax = aDim;
  }
}

//=================================================================================================

Standard_Integer AlgoTools::Dimension(const TopoShape& theS)
{
  Standard_Integer aDMin, aDMax;
  Dimensions(theS, aDMin, aDMax);
  return (aDMin == aDMax) ? aDMin : -1;
}

//=================================================================================================

void AlgoTools::TreatCompound(const TopoShape&   theS,
                                       ShapeList& theLS,
                                       TopTools_MapOfShape*  theMFence)
{
  TopAbs_ShapeEnum aType = theS.ShapeType();
  if (aType != TopAbs_COMPOUND)
  {
    if (!theMFence || theMFence->Add(theS))
    {
      theLS.Append(theS);
    }
    return;
  }

  for (TopoDS_Iterator it(theS); it.More(); it.Next())
  {
    TreatCompound(it.Value(), theLS, theMFence);
  }
}
