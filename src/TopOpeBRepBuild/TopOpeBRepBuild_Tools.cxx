// Created on: 1999-11-02
// Created by: Peter KURNEV
// Copyright (c) 1999-1999 Matra Datavision
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

#include <Adaptor2d_Curve2d.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <ProjLib_ProjectedCurve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepBuild_CorrectFace2d.hxx>
#include <TopOpeBRepBuild_Tools.hxx>
#include <TopOpeBRepDS_DataMapOfShapeState.hxx>
#include <TopOpeBRepDS_IndexedDataMapOfShapeWithState.hxx>
#include <TopOpeBRepDS_ShapeWithState.hxx>
#include <TopOpeBRepTool_2d.hxx>
#include <TopOpeBRepTool_ShapeClassifier.hxx>
#include <TopOpeBRepTool_TOOL.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#include <stdio.h>
// define parameter division number as 10*e^(-PI) = 0.43213918
const Standard_Real PAR_T = 0.43213918;

//=======================================================================
// function Tools3::FindState
// purpose  :
//=======================================================================
void Tools3::FindState(const TopoShape&                              aSubsh,
                                      const TopAbs_State                               aState,
                                      const TopAbs_ShapeEnum                           aSubshEnum,
                                      const TopTools_IndexedDataMapOfShapeListOfShape& aMapSubshAnc,
                                      TopTools_MapOfShape&              aMapProcessedSubsh,
                                      TopOpeBRepDS_DataMapOfShapeState& aMapSS)
{
  Standard_Integer                   i, nSub;
  const ShapeList&        aListOfShapes = aMapSubshAnc.FindFromKey(aSubsh);
  TopTools_ListIteratorOfListOfShape anIt(aListOfShapes);
  for (; anIt.More(); anIt.Next())
  {
    const TopoShape&        aS = anIt.Value();
    TopTools_IndexedMapOfShape aSubshMap;
    TopExp1::MapShapes(aS, aSubshEnum, aSubshMap);
    nSub = aSubshMap.Extent();
    for (i = 1; i <= nSub; i++)
    {
      const TopoShape& aSS = aSubshMap(i);
      if (!aMapProcessedSubsh.Contains(aSS))
      {
        aMapProcessedSubsh.Add(aSS);
        aMapSS.Bind(aSS, aState);
        FindState(aSS, aState, aSubshEnum, aMapSubshAnc, aMapProcessedSubsh, aMapSS);
      }
    }
  }
}

//=======================================================================
// function Tools3::PropagateState
// purpose  :
//=======================================================================
void Tools3::PropagateState(
  const TopOpeBRepDS_DataMapOfShapeState&      aSplShapesState,
  const TopTools_IndexedMapOfShape&            aShapesToRestMap,
  const TopAbs_ShapeEnum                       aSubshEnum, // Vertex
  const TopAbs_ShapeEnum                       aShapeEnum, // Edge
  TopOpeBRepTool_ShapeClassifier&              aShapeClassifier,
  TopOpeBRepDS_IndexedDataMapOfShapeWithState& aMapOfShapeWithState,
  const TopTools_MapOfShape&                   anAvoidSubshMap)
{
  Standard_Integer                 j, nSub, nRest;
  TopOpeBRepDS_DataMapOfShapeState aMapSS, aMapSS1;

  TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeState anItSS(aSplShapesState);
  for (; anItSS.More(); anItSS.Next())
  {
    const TopoShape&        aShape = anItSS.Key();
    TopAbs_State               aState = anItSS.Value();
    TopTools_IndexedMapOfShape aSubshapes;
    TopExp1::MapShapes(aShape, aSubshEnum, aSubshapes);
    nSub = aSubshapes.Extent();
    for (j = 1; j <= nSub; j++)
      if (!anAvoidSubshMap.Contains(aSubshapes(j))) // MSV: enforce subshapes avoidance
        aMapSS.Bind(aSubshapes(j), aState);
  }

  aMapSS1 = aMapSS;

  // 1. Build the Map of ShapesAndAncestors for ShapesToRest
  TopTools_IndexedDataMapOfShapeListOfShape aMapSubshAnc;
  nRest = aShapesToRestMap.Extent();
  for (j = 1; j <= nRest; j++)
    TopExp1::MapShapesAndAncestors(aShapesToRestMap(j), aSubshEnum, aShapeEnum, aMapSubshAnc);

  // 2. Make Map Of all subshapes  aMapSS
  TopTools_MapOfShape aProcessedSubshapes;
  anItSS.Initialize(aMapSS1);
  for (; anItSS.More(); anItSS.Next())
  {
    const TopoShape& aSubsh = anItSS.Key();
    TopAbs_State        aState = anItSS.Value();
    if (aMapSubshAnc.Contains(aSubsh))
    {
      aProcessedSubshapes.Add(aSubsh);
      FindState(aSubsh, aState, aSubshEnum, aMapSubshAnc, aProcessedSubshapes, aMapSS);
    }
  }

  // 3. Propagate the states on ShapesToRestMap
  TopoShape        aNullShape;
  TopTools_MapOfShape aNonPassedShapes;
  nRest = aShapesToRestMap.Extent();
  for (j = 1; j <= nRest; j++)
  {
    const TopoShape&        aS = aShapesToRestMap.FindKey(j);
    TopTools_IndexedMapOfShape aSubshMap;
    TopExp1::MapShapes(aS, aSubshEnum, aSubshMap);
    const TopoShape& aSubsh = aSubshMap(1);
    if (aMapSS.IsBound(aSubsh))
    {
      TopAbs_State aState = aMapSS.Find(aSubsh);

      if (aState == TopAbs_ON)
      {
        aState = aShapeClassifier.StateShapeReference(aS, aNullShape);
      }
      // Add the Rest Shape to aMapOfShapeWithState
      ShapeWithState aShapeWithState;
      aShapeWithState.SetState(aState);
      aShapeWithState.SetIsSplitted(Standard_False);
      aMapOfShapeWithState.Add(aS, aShapeWithState);
    }

    else
    {
      aNonPassedShapes.Add(aS);
    }
  }

  // 4. Define the states for aNonPassedShapes
  //   (for faces themselves and for theirs Wires, Edges):
  if (aNonPassedShapes.Extent())
  {
    // Build the Map of ShapesAndAncestors for aNonPassedShapes
    aMapSubshAnc.Clear();
    TopTools_MapIteratorOfMapOfShape aMapIt;
    aMapIt.Initialize(aNonPassedShapes);
    for (; aMapIt.More(); aMapIt.Next())
      TopExp1::MapShapesAndAncestors(aMapIt.Key(), aSubshEnum, aShapeEnum, aMapSubshAnc);

    aMapSS.Clear();
    aMapIt.Initialize(aNonPassedShapes);
    for (; aMapIt.More(); aMapIt.Next())
    {
      // Face
      const TopoShape& aNonPassedShape = aMapIt.Key();

      if (!aMapSS.IsBound(aNonPassedShape))
      {
        TopAbs_State aState = FindStateThroughVertex(aNonPassedShape,
                                                     aShapeClassifier,
                                                     aMapOfShapeWithState,
                                                     anAvoidSubshMap);
        aMapSS.Bind(aNonPassedShape, aState);

        // First Subshape
        TopTools_IndexedMapOfShape aTmpMap;
        TopExp1::MapShapes(aNonPassedShape, aSubshEnum, aTmpMap);
        TopoShape aFirstSubsh;
        for (j = 1; j <= aTmpMap.Extent() && aFirstSubsh.IsNull(); j++)
          if (!anAvoidSubshMap.Contains(aTmpMap(j)))
            aFirstSubsh = aTmpMap(j);
        if (aFirstSubsh.IsNull())
          continue;
        aMapSS.Bind(aFirstSubsh, aState);

        // Propagation of aState for subshapes
        TopTools_MapOfShape aMapProcessedSubsh;
        if (aSubshEnum == TopAbs_EDGE)
          FindState1(aFirstSubsh, aState, aMapSubshAnc, aMapProcessedSubsh, aMapSS);
        else // if (aSubshEnum==TopAbs_VERTEX)
          FindState2(aFirstSubsh, aState, aMapSubshAnc, aMapProcessedSubsh, aMapSS);
      }
    }

    // Fill aShapeWithState
    ShapeWithState aShapeWithState;
    aShapeWithState.SetIsSplitted(Standard_False);
    TopOpeBRepDS_DataMapIteratorOfDataMapOfShapeState anII(aMapSS);
    for (; anII.More(); anII.Next())
    {
      aShapeWithState.SetState(anII.Value());
      if (anII.Key().ShapeType() != TopAbs_VERTEX)
        aMapOfShapeWithState.Add(anII.Key(), aShapeWithState);
    }
  }
}

//=================================================================================================

void Tools3::FindState2(
  const TopoShape&                              aSubsh,
  const TopAbs_State                               aState,
  const TopTools_IndexedDataMapOfShapeListOfShape& aMapSubshAnc,
  TopTools_MapOfShape&                             aMapProcessedSubsh,
  TopOpeBRepDS_DataMapOfShapeState&                aMapSS)
{
  Standard_Integer                   i, nSub;
  const ShapeList&        aListOfShapes = aMapSubshAnc.FindFromKey(aSubsh);
  TopTools_ListIteratorOfListOfShape anIt(aListOfShapes);
  for (; anIt.More(); anIt.Next())
  {
    // Shape
    const TopoShape& aShape = anIt.Value();
    aMapSS.Bind(aShape, aState);

    // Subshape
    TopTools_IndexedMapOfShape aSubshMap;
    TopExp1::MapShapes(aShape, TopAbs_VERTEX, aSubshMap);
    nSub = aSubshMap.Extent();
    for (i = 1; i <= nSub; i++)
    {
      const TopoShape& aSS = aSubshMap(i);
      if (!aMapProcessedSubsh.Contains(aSS))
      {
        aMapProcessedSubsh.Add(aSS);
        aMapSS.Bind(aSS, aState);
        FindState2(aSS, aState, aMapSubshAnc, aMapProcessedSubsh, aMapSS);
      }
    }
  }
}

//=================================================================================================

void Tools3::FindState1(
  const TopoShape&                              aSubsh,
  const TopAbs_State                               aState,
  const TopTools_IndexedDataMapOfShapeListOfShape& aMapSubshAnc,
  TopTools_MapOfShape&                             aMapProcessedSubsh,
  TopOpeBRepDS_DataMapOfShapeState&                aMapSS)
{
  Standard_Integer                   i, nSub, j, nW;
  const ShapeList&        aListOfShapes = aMapSubshAnc.FindFromKey(aSubsh);
  TopTools_ListIteratorOfListOfShape anIt(aListOfShapes);
  for (; anIt.More(); anIt.Next())
  {
    // Face
    const TopoShape& aShape = anIt.Value();
    aMapSS.Bind(aShape, aState);
    // Wire
    TopTools_IndexedMapOfShape aWireMap;
    TopExp1::MapShapes(aShape, TopAbs_WIRE, aWireMap);
    nW = aWireMap.Extent();
    for (j = 1; j <= nW; j++)
      aMapSS.Bind(aWireMap(j), aState);
    // Edge
    TopTools_IndexedMapOfShape aSubshMap;
    TopExp1::MapShapes(aShape, TopAbs_EDGE, aSubshMap);
    nSub = aSubshMap.Extent();
    for (i = 1; i <= nSub; i++)
    {
      const TopoShape& aSS = aSubshMap(i);
      if (!aMapProcessedSubsh.Contains(aSS))
      {
        aMapProcessedSubsh.Add(aSS);
        aMapSS.Bind(aSS, aState);
        FindState1(aSS, aState, aMapSubshAnc, aMapProcessedSubsh, aMapSS);
      }
    }
  }
}

//=================================================================================================

TopAbs_State Tools3::FindStateThroughVertex(
  const TopoShape&                          aShape,
  TopOpeBRepTool_ShapeClassifier&              aShapeClassifier,
  TopOpeBRepDS_IndexedDataMapOfShapeWithState& aMapOfShapeWithState,
  const TopTools_MapOfShape&                   anAvoidSubshMap)
{
  TopTools_IndexedMapOfShape aSubshMap;
  TopExp1::MapShapes(aShape, TopAbs_VERTEX, aSubshMap);

  TopoShape     aSubsh;
  Standard_Integer i;
  for (i = 1; i <= aSubshMap.Extent() && aSubsh.IsNull(); i++)
    if (!anAvoidSubshMap.Contains(aSubshMap(i)))
      aSubsh = aSubshMap(i);
  if (aSubsh.IsNull())
  {
    // try an edge
    aSubshMap.Clear();
    TopExp1::MapShapes(aShape, TopAbs_EDGE, aSubshMap);
    for (i = 1; i <= aSubshMap.Extent() && aSubsh.IsNull(); i++)
      if (!anAvoidSubshMap.Contains(aSubshMap(i)))
        aSubsh = aSubshMap(i);
    if (aSubsh.IsNull())
    {
#ifdef OCCT_DEBUG
      std::cout << "FindStateThroughVertex: warning: all vertices are avoided" << std::endl;
#endif
      return TopAbs_UNKNOWN; // failure
    }
  }

  TopoShape                aNullShape;
  TopAbs_State                aState = aShapeClassifier.StateShapeReference(aSubsh, aNullShape);
  ShapeWithState aShapeWithState;
  aShapeWithState.SetState(aState);
  aShapeWithState.SetIsSplitted(Standard_False);
  aMapOfShapeWithState.Add(aShape, aShapeWithState);
  SpreadStateToChild(aShape, aState, aMapOfShapeWithState);
  return aState;
}

//=================================================================================================

void Tools3::SpreadStateToChild(
  const TopoShape&                          aShape,
  const TopAbs_State                           aState,
  TopOpeBRepDS_IndexedDataMapOfShapeWithState& aMapOfShapeWithState)
{
  TopTools_IndexedMapOfShape aChildMap;
  TopExp1::MapShapes(aShape, TopAbs_FACE, aChildMap);
  TopExp1::MapShapes(aShape, TopAbs_WIRE, aChildMap);
  TopExp1::MapShapes(aShape, TopAbs_EDGE, aChildMap);

  ShapeWithState aShapeWithState;
  aShapeWithState.SetState(aState);
  aShapeWithState.SetIsSplitted(Standard_False);

  Standard_Integer i, n = aChildMap.Extent();
  for (i = 1; i <= n; i++)
  {
    aMapOfShapeWithState.Add(aChildMap(i), aShapeWithState);
  }
}

//=================================================================================================

void Tools3::PropagateStateForWires(
  const TopTools_IndexedMapOfShape&            aFacesToRestMap,
  TopOpeBRepDS_IndexedDataMapOfShapeWithState& aMapOfShapeWithState)
{
  Standard_Integer i, j, nF, nW, k, nE;

  nF = aFacesToRestMap.Extent();
  for (i = 1; i <= nF; i++)
  {
    const TopoShape& aF = aFacesToRestMap(i);
    if (aMapOfShapeWithState.Contains(aF))
    {
      const ShapeWithState& aSWS = aMapOfShapeWithState.FindFromKey(aF);
      TopAbs_State                       aSt  = aSWS.State();

      TopTools_IndexedMapOfShape aWireMap;
      TopExp1::MapShapes(aF, TopAbs_WIRE, aWireMap);
      nW = aWireMap.Extent();
      for (j = 1; j <= nW; j++)
      {
        const TopoShape&         aW = aWireMap(j);
        ShapeWithState aWireSWS;
        aWireSWS.SetState(aSt);
        aWireSWS.SetIsSplitted(Standard_False);
        aMapOfShapeWithState.Add(aW, aWireSWS);

        TopTools_IndexedMapOfShape aEdgeMap;
        TopExp1::MapShapes(aW, TopAbs_EDGE, aEdgeMap);
        nE = aEdgeMap.Extent();
        for (k = 1; k <= nE; k++)
        {
          const TopoShape& aE = aEdgeMap(k);
          if (!aMapOfShapeWithState.Contains(aE))
          {
            ShapeWithState anEdgeSWS;
            anEdgeSWS.SetState(aSt);
            anEdgeSWS.SetIsSplitted(Standard_False);
            aMapOfShapeWithState.Add(aE, anEdgeSWS);
          }
        }
      }
    }
  }
}

//=================================================================================================

void Tools3::GetNormalToFaceOnEdge(const TopoFace& aFObj,
                                                  const TopoEdge& anEdgeObj,
                                                  Vector3d&            aNormal)
{
  const TopoEdge&   aEd = anEdgeObj;
  const TopoFace&   aFS = aFObj;
  Standard_Real        f2 = 0., l2 = 0., tolpc = 0., f = 0., l = 0., par = 0.;
  Handle(GeomCurve2d) C2D = FC2D_CurveOnSurface(aEd, aFS, f2, l2, tolpc, Standard_True);

  BRepAdaptor_Curve aCA(aEd);
  f   = aCA.FirstParameter();
  l   = aCA.LastParameter();
  par = f * PAR_T + (1 - PAR_T) * l;

  gp_Pnt2d aUV1;
  C2D->D0(par, aUV1);

  Point3d              aP;
  Vector3d              aTg1, aTg2;
  BRepAdaptor_Surface aSA1(aFS);
  aSA1.D1(aUV1.X(), aUV1.Y(), aP, aTg1, aTg2);
  aNormal = aTg1 ^ aTg2;
}

//=================================================================================================

void Tools3::GetNormalInNearestPoint(const TopoFace& F,
                                                    const TopoEdge& E,
                                                    Vector3d&            aNormal)
{
  Standard_Real f2 = 0., l2 = 0., tolpc = 0., par = 0.;

  gp_Vec2d aTangent;

  Handle(GeomCurve2d) C2D = FC2D_CurveOnSurface(E, F, f2, l2, tolpc, Standard_True);

  par = f2 * PAR_T + (1 - PAR_T) * l2;

  gp_Pnt2d aP;
  C2D->D1(par, aP, aTangent);

  Standard_Real Xnorm = -aTangent.Y();
  Standard_Real Ynorm = aTangent.X();

  Standard_Real step = TOOL1::minDUV(F);
  step *= 1e-2;

  gp_Vec2d aPV(aP.X(), aP.Y());
  gp_Dir2d aStepV(Xnorm, Ynorm);
  gp_Vec2d aNorm2d = aPV + gp_Vec2d(step * aStepV);

  Standard_Real newU = aNorm2d.X();
  Standard_Real newV = aNorm2d.Y();
  Vector3d        aTg1, aTg2;
  Point3d        aP1;

  BRepAdaptor_Surface BS(F);
  BS.D1(newU, newV, aP1, aTg1, aTg2);

  gp_Pnt2d                aP2d(newU, newV);
  BRepTopAdaptor_FClass2d FC(F, Precision::PConfusion());
  TopAbs_State            aState = FC.Perform(aP2d);

  // point out of face: try to go at another direction
  if (aState == TopAbs_OUT)
  {
    aStepV.Reverse();
    aNorm2d = aPV + gp_Vec2d(step * aStepV);

    newU = aNorm2d.X();
    newV = aNorm2d.Y();

    BS.D1(newU, newV, aP1, aTg1, aTg2);

    // in principle, we must check again
    //     aP2d.SetX(newU); aP2d.SetY(newV);
    //     BRepClass_FaceClassifier FC(Fex, aP2d, 1e-7);
    //     TopAbs_State aState = FC.State();
  }

  aNormal = aTg1 ^ aTg2;
}

//=================================================================================================

Standard_Boolean Tools3::GetTangentToEdgeEdge(const TopoFace&, // aFObj,
                                                             const TopoEdge& anEdgeObj,
                                                             const TopoEdge& aOriEObj,
                                                             Vector3d&            aTangent)
{

  if (BRepInspector::Degenerated(aOriEObj) || BRepInspector::Degenerated(anEdgeObj))
  {
    return Tools3::GetTangentToEdge(anEdgeObj, aTangent);
  }

  TopoEdge aEd = anEdgeObj, aEOri = aOriEObj;

  Standard_Real f = 0., l = 0., par = 0., parOri = 0.;

  BRepAdaptor_Curve aCA(aEd);
  BRepAdaptor_Curve aCAOri(aEOri);

  f = aCA.FirstParameter();
  l = aCA.LastParameter();

  par = f * PAR_T + (1 - PAR_T) * l;

  Point3d aP;
  Vector3d aTgPiece;
  aCA.D1(par, aP, aTgPiece);
  aTangent = aTgPiece;

  Point3d aPOri;
  Vector3d aTgOri;
  /////
  Handle(GeomCurve3d) GCOri      = aCAOri.Curve().Curve();
  Handle(GeomCurve3d) aCopyCurve = Handle(GeomCurve3d)::DownCast(GCOri->Copy());

  const TopLoc_Location& aLoc  = aEOri.Location();
  Transform3d                aTrsf = aLoc.Transformation();
  aCopyCurve->Transform(aTrsf);

  GeomAPI_ProjectPointOnCurve aPP(aP,
                                  aCopyCurve,
                                  aCopyCurve->FirstParameter(),
                                  aCopyCurve->LastParameter());
#ifdef OCCT_DEBUG
//  Point3d aNP = aPP.NearestPoint();
#endif
  parOri = aPP.LowerDistanceParameter();

  aCopyCurve->D1(parOri, aPOri, aTgOri); // aPOri must be equal aNP !
  // printf(" aNP  ={%lf, %lf, %lf}\n", aNP.X(), aNP.Y(), aNP.Z());
  // printf(" aPOri={%lf, %lf, %lf}\n", aPOri.X(), aPOri.Y(), aPOri.Z());
  if (aEd.Orientation() == TopAbs_REVERSED)
    aTangent.Reverse();

  if (aTgOri * aTgPiece < 0.)
  {
    aTangent.Reverse();
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean Tools3::GetTangentToEdge(const TopoEdge& anEdgeObj,
                                                         Vector3d&            aTangent)
{
  const TopoEdge& aEd = anEdgeObj;

  Standard_Real f = 0., l = 0., par = 0.;

  BRepAdaptor_Curve aCA(aEd);

  f = aCA.FirstParameter();
  l = aCA.LastParameter();

  par = f * PAR_T + (1 - PAR_T) * l;
  Point3d aP;
  aCA.D1(par, aP, aTangent);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean Tools3::GetAdjacentFace(
  const TopoShape&                              aFaceObj,
  const TopoShape&                              anEObj,
  const TopTools_IndexedDataMapOfShapeListOfShape& anEdgeFaceMap,
  TopoShape&                                    anAdjFaceObj)
{
  const ShapeList&        aListOfAdjFaces = anEdgeFaceMap.FindFromKey(anEObj);
  TopTools_ListIteratorOfListOfShape anIt(aListOfAdjFaces);
  TopoShape                       anAdjShape;
  for (; anIt.More(); anIt.Next())
  {
    if (anIt.Value() != aFaceObj)
    {
      anAdjShape = anIt.Value();
      break;
    }
  }

  if (!anAdjShape.IsNull())
  {
    anAdjFaceObj = TopoDS::Face(anAdjShape);
    return Standard_True;
  }
  else
  {
    return Standard_False;
  }
}

//=================================================================================================

void Tools3::UpdatePCurves(const TopoWire& aWire,
                                          const TopoFace& fromFace,
                                          const TopoFace& toFace)
{
  ShapeExplorer aExp(aWire, TopAbs_EDGE);

  for (; aExp.More(); aExp.Next())
  {
    TopoShape aEdge = aExp.Current();
    UpdateEdgeOnFace(TopoDS::Edge(aEdge), fromFace, toFace);
  }
}

//=================================================================================================

void Tools3::UpdateEdgeOnFace(const TopoEdge& aEdgeToUpdate,
                                             const TopoFace& fromFace,
                                             const TopoFace& toFace)
{
  ShapeBuilder BB;

  Standard_Real        tolE = BRepInspector::Tolerance(TopoDS::Edge(aEdgeToUpdate));
  Standard_Real        f2 = 0., l2 = 0., tolpc = 0.;
  Handle(GeomCurve2d) C2D;

  if (BRepInspector::Degenerated(aEdgeToUpdate))
  {
    // we can not compute PCurve for Degenerated Edge
    // so we take as it was on old face and after (in CorrectFace2D)
    //  we will adjust this PCurve
    C2D = FC2D_CurveOnSurface(aEdgeToUpdate, fromFace, f2, l2, tolpc, Standard_True);
    Standard_Real               tol    = Max(tolE, tolpc);
    Handle(GeomCurve2d)        C2Dn   = Handle(GeomCurve2d)::DownCast(C2D->Copy());
    Handle(Geom2d_TrimmedCurve) newC2D = new Geom2d_TrimmedCurve(C2Dn, f2, l2);
    BB.UpdateEdge(aEdgeToUpdate, newC2D, toFace, tol);
  }
  else
  { // not degenerated edge

    if (BRepInspector::IsClosed(aEdgeToUpdate, fromFace))
    {
      UpdateEdgeOnPeriodicalFace(aEdgeToUpdate, fromFace, toFace);
    }
    else
    {
      C2D               = FC2D_CurveOnSurface(aEdgeToUpdate, toFace, f2, l2, tolpc, Standard_True);
      Standard_Real tol = Max(tolE, tolpc);
      BB.UpdateEdge(aEdgeToUpdate, C2D, toFace, tol);
    }
  }
}

//=================================================================================================

void Tools3::UpdateEdgeOnPeriodicalFace(const TopoEdge& aEdgeToUpdate,
                                                       const TopoFace& fromFace,
                                                       const TopoFace& toFace)
{
  Standard_Boolean   DiffOriented = Standard_False;
  ShapeBuilder       BB;
  TopoEdge        newE  = aEdgeToUpdate; // newE.Orientation(TopAbs_FORWARD);
  const TopoFace& fFace = fromFace;      // fFace.Orientation(TopAbs_FORWARD);
  const TopoFace& tFace = toFace;        // tFace.Orientation(TopAbs_FORWARD);
  Standard_Real      fc = 0., lc = 0.;

  Handle(GeomCurve2d) cc = BRepInspector::CurveOnSurface(newE, tFace, fc, lc);

  if (!cc.IsNull())
  {
    // std::cout << "Pcurves exist" << std::endl;
    return;
  }

  Vector3d aN1, aN2;
  Tools3::GetNormalToFaceOnEdge(TopoDS::Face(fromFace),
                                               TopoDS::Edge(aEdgeToUpdate),
                                               aN1);

  Tools3::GetNormalToFaceOnEdge(TopoDS::Face(toFace),
                                               TopoDS::Edge(aEdgeToUpdate),
                                               aN2);

  if (aN1 * aN2 < 0)
    DiffOriented = Standard_True;

  Standard_Real tolE = BRepInspector::Tolerance(newE);
  Standard_Real f2 = 0., l2 = 0., tolpc = 0., tol = 0.;

  // first  PCurve
  Handle(GeomCurve2d) C2D = FC2D_CurveOnSurface(newE, tFace, f2, l2, tolpc, Standard_True);

  tol = Max(tolpc, tolE);

  BRepAdaptor_Surface aBAS(fFace);
  gp_Vec2d            aTrV;

  Standard_Real ff = 0., lf = 0., fr = 0., lr = 0.;
  gp_Pnt2d      aUVf, aUVr;

  Handle(GeomCurve2d) oldC2DFor = BRepInspector::CurveOnSurface(newE, // FC2D_CurveOnSurface(newE,
                                                             fFace,
                                                             ff,
                                                             lf); //, tolpc, Standard_True);
  newE.Reverse();
  Handle(GeomCurve2d) oldC2DRev = BRepInspector::CurveOnSurface(newE, // FC2D_CurveOnSurface(newE,
                                                             fFace,
                                                             fr,
                                                             lr); //, tolpc, Standard_True);

  oldC2DFor->D0(ff, aUVf);
  oldC2DRev->D0(fr, aUVr);

  if (!DiffOriented)
    aTrV = gp_Vec2d(aUVf, aUVr);
  else
    aTrV = gp_Vec2d(aUVr, aUVf);

  gp_Vec2d         aux(gp_Pnt2d(0., 0.), gp_Pnt2d(1., 1.));
  Standard_Real    scalar = aux * aTrV;
  Standard_Boolean dir    = (scalar >= 0.) ? Standard_True : Standard_False;

  // compute right order of pcurves
  gp_Vec2d aYVec(gp_Pnt2d(0., 0.), gp_Pnt2d(0., 1.));
  gp_Pnt2d aUVfv, aUVlv;
  C2D->D0(f2, aUVfv);
  C2D->D0(l2, aUVlv);
  gp_Vec2d C2DVec(aUVfv, aUVlv);

  Standard_Boolean firstOrder = Standard_True;
  scalar                      = aYVec * C2DVec;
  if (fabs(scalar) <= 1e-10)
  { // compute along X axe
    gp_Vec2d aXVec(gp_Pnt2d(0., 0.), gp_Pnt2d(1., 0.));
    scalar     = aXVec * C2DVec;
    firstOrder = (scalar >= 0.) ? Standard_True : Standard_False;
  }
  else
    firstOrder = (scalar > 0.) ? Standard_False : Standard_True;

  Handle(GeomCurve2d) aTrC = Handle(GeomCurve2d)::DownCast(C2D->Copy());
  aTrC->Translate(aTrV);

  if (dir)
  {
    if (firstOrder)
      BB.UpdateEdge(aEdgeToUpdate, C2D, aTrC, toFace, tol);
    else
      BB.UpdateEdge(aEdgeToUpdate, aTrC, C2D, toFace, tol);
  }
  else
  {
    if (!firstOrder)
      BB.UpdateEdge(aEdgeToUpdate, C2D, aTrC, toFace, tol);
    else
      BB.UpdateEdge(aEdgeToUpdate, aTrC, C2D, toFace, tol);
  }
}

//=================================================================================================

Standard_Boolean Tools3::IsDegEdgesTheSame(const TopoShape& anE1,
                                                          const TopoShape& anE2)
{
  TopTools_IndexedMapOfShape aVMap1, aVMap2;
  TopExp1::MapShapes(anE1, TopAbs_VERTEX, aVMap1);
  TopExp1::MapShapes(anE2, TopAbs_VERTEX, aVMap2);

  if (!aVMap1.Extent() || !aVMap2.Extent())
    return Standard_False;

  if (aVMap1(1).IsSame(aVMap2(1)))
    return Standard_True;
  else
    return Standard_False;
}

//=======================================================================
// function : NormalizeFace
// purpose  : remove all INTERNAL and EXTERNAL edges from the face
//=======================================================================
void Tools3::NormalizeFace(const TopoShape& oldFace, TopoShape& corrFace)
{
  Standard_Real tolF1;

  TopLoc_Location Loc;
  TopoFace     aF1 = TopoDS::Face(oldFace), aNewFace;

  aF1.Orientation(TopAbs_FORWARD);

  Handle(GeomSurface) Surf = BRepInspector::Surface(aF1, Loc);
  tolF1                     = BRepInspector::Tolerance(aF1);
  ShapeBuilder BB;
  BB.MakeFace(aNewFace, Surf, Loc, tolF1);

  ShapeExplorer aFExp(aF1, TopAbs_WIRE);
  for (; aFExp.More(); aFExp.Next())
  {
    Standard_Integer NbGoodEdges = 0;
    TopoShape     aWire       = aFExp.Current();
    aWire.Orientation(TopAbs_FORWARD);
    TopoWire aNewWire;

    BB.MakeWire(aNewWire);

    ShapeExplorer aWExp(aWire, TopAbs_EDGE);
    for (; aWExp.More(); aWExp.Next())
    {
      TopoShape anEdge = aWExp.Current();

      if (anEdge.Orientation() == TopAbs_EXTERNAL || anEdge.Orientation() == TopAbs_INTERNAL)
        continue;

      BB.Add(aNewWire, TopoDS::Edge(anEdge));
      NbGoodEdges++;
    }
    // keep wire  orientation
    aNewWire.Orientation(aFExp.Current().Orientation()); // aWire.Orientation());

    if (NbGoodEdges) // we add new wire only if it contains at least one edge
      BB.Add(aNewFace, aNewWire);
  }
  // keep face  orientation
  aNewFace.Orientation(oldFace.Orientation());

  corrFace = aNewFace;
}

//=======================================================================
// function : CorrectFace2d
// purpose  : adjust PCurves of periodical face in 2d
//=======================================================================
void Tools3::CorrectFace2d(const TopoShape&                       oldFace,
                                          TopoShape&                             corrFace,
                                          const TopTools_IndexedMapOfOrientedShape& aSourceShapes,
                                          TopTools_IndexedDataMapOfShapeShape& aMapOfCorrect2dEdges)
{
  TopOpeBRepBuild_CorrectFace2d aCorrectFace2d(TopoDS::Face(oldFace),
                                               aSourceShapes,
                                               aMapOfCorrect2dEdges);

  aCorrectFace2d.Perform();
  corrFace = oldFace;
}
