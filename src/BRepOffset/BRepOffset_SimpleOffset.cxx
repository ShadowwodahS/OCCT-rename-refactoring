// Created on: 2016-10-14
// Created by: Alexander MALYSHEV
// Copyright (c) 1995-1999 Matra Datavision
// Copyright (c) 1999-2016 OPEN CASCADE SAS
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

// Include self.
#include <BRepOffset_SimpleOffset.hxx>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepLib.hxx>
#include <BRepLib_ValidateEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepOffset.hxx>
#include <Geom_OffsetSurface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <NCollection_Vector.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

//=================================================================================================

BRepOffset_SimpleOffset::BRepOffset_SimpleOffset(const TopoShape& theInputShape,
                                                 const Standard_Real theOffsetValue,
                                                 const Standard_Real theTolerance)
    : myOffsetValue(theOffsetValue),
      myTolerance(theTolerance)
{
  FillOffsetData(theInputShape);
}

//=================================================================================================

Standard_Boolean BRepOffset_SimpleOffset::NewSurface(const TopoFace&    F,
                                                     Handle(GeomSurface)& S,
                                                     TopLoc_Location&      L,
                                                     Standard_Real&        Tol,
                                                     Standard_Boolean&     RevWires,
                                                     Standard_Boolean&     RevFace)
{
  if (!myFaceInfo.IsBound(F))
    return Standard_False;

  const NewFaceData& aNFD = myFaceInfo.Find(F);

  S        = aNFD.myOffsetS;
  L        = aNFD.myL;
  Tol      = aNFD.myTol;
  RevWires = aNFD.myRevWires;
  RevFace  = aNFD.myRevFace;

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepOffset_SimpleOffset::NewCurve(const TopoEdge&  E,
                                                   Handle(GeomCurve3d)& C,
                                                   TopLoc_Location&    L,
                                                   Standard_Real&      Tol)
{
  if (!myEdgeInfo.IsBound(E))
    return Standard_False;

  const NewEdgeData& aNED = myEdgeInfo.Find(E);

  C   = aNED.myOffsetC;
  L   = aNED.myL;
  Tol = aNED.myTol;

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepOffset_SimpleOffset::NewPoint(const TopoVertex& V,
                                                   Point3d&              P,
                                                   Standard_Real&       Tol)
{
  if (!myVertexInfo.IsBound(V))
    return Standard_False;

  const NewVertexData1& aNVD = myVertexInfo.Find(V);

  P   = aNVD.myP;
  Tol = aNVD.myTol;

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepOffset_SimpleOffset::NewCurve2d(const TopoEdge& E,
                                                     const TopoFace& F,
                                                     const TopoEdge& /*NewE*/,
                                                     const TopoFace& /*NewF*/,
                                                     Handle(GeomCurve2d)& C,
                                                     Standard_Real&        Tol)
{
  // Use original pcurve.
  Standard_Real aF, aL;
  C   = BRepInspector::CurveOnSurface(E, F, aF, aL);
  Tol = BRepInspector::Tolerance(E);

  if (myEdgeInfo.IsBound(E))
    Tol = myEdgeInfo.Find(E).myTol;

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepOffset_SimpleOffset::NewParameter(const TopoVertex& V,
                                                       const TopoEdge&   E,
                                                       Standard_Real&       P,
                                                       Standard_Real&       Tol)
{
  // Use original parameter.
  P   = BRepInspector::Parameter(V, E);
  Tol = BRepInspector::Tolerance(V);

  if (myVertexInfo.IsBound(V))
    Tol = myVertexInfo.Find(V).myTol;

  return Standard_True;
}

//=================================================================================================

GeomAbs_Shape BRepOffset_SimpleOffset::Continuity(const TopoEdge& E,
                                                  const TopoFace& F1,
                                                  const TopoFace& F2,
                                                  const TopoEdge& /*NewE*/,
                                                  const TopoFace& /*NewF1*/,
                                                  const TopoFace& /*NewF2*/)
{
  // Compute result using original continuity.
  return BRepInspector::Continuity(E, F1, F2);
}

//=================================================================================================

void BRepOffset_SimpleOffset::FillOffsetData(const TopoShape& theShape)
{
  // Clears old data.
  myFaceInfo.Clear();
  myEdgeInfo.Clear();
  myVertexInfo.Clear();

  // Faces loop. Compute offset surface for each face.
  ShapeExplorer anExpSF(theShape, TopAbs_FACE);
  for (; anExpSF.More(); anExpSF.Next())
  {
    const TopoFace& aCurrFace = TopoDS::Face(anExpSF.Current());
    FillFaceData(aCurrFace);
  }

  // Iterate over edges to compute 3d curve.
  TopTools_IndexedDataMapOfShapeListOfShape aEdgeFaceMap;
  TopExp1::MapShapesAndAncestors(theShape, TopAbs_EDGE, TopAbs_FACE, aEdgeFaceMap);
  for (Standard_Integer anIdx = 1; anIdx <= aEdgeFaceMap.Size(); ++anIdx)
  {
    const TopoEdge& aCurrEdge = TopoDS::Edge(aEdgeFaceMap.FindKey(anIdx));
    FillEdgeData(aCurrEdge, aEdgeFaceMap, anIdx);
  }

  // Iterate over vertices to compute new vertex.
  TopTools_IndexedDataMapOfShapeListOfShape aVertexEdgeMap;
  TopExp1::MapShapesAndAncestors(theShape, TopAbs_VERTEX, TopAbs_EDGE, aVertexEdgeMap);
  for (Standard_Integer anIdx = 1; anIdx <= aVertexEdgeMap.Size(); ++anIdx)
  {
    const TopoVertex& aCurrVertex = TopoDS::Vertex(aVertexEdgeMap.FindKey(anIdx));
    FillVertexData(aCurrVertex, aVertexEdgeMap, anIdx);
  }
}

//=================================================================================================

void BRepOffset_SimpleOffset::FillFaceData(const TopoFace& theFace)
{
  NewFaceData aNFD;
  aNFD.myRevWires = Standard_False;
  aNFD.myRevFace  = Standard_False;
  aNFD.myTol      = BRepInspector::Tolerance(theFace);

  // Create offset surface.

  // Any existing transformation is applied to the surface.
  // New face will have null transformation.
  Handle(GeomSurface) aS = BRepInspector::Surface(theFace);
  aS                      = BRepOffset1::CollapseSingularities(aS, theFace, myTolerance);

  // Take into account face orientation.
  Standard_Real aMult = 1.0;
  if (theFace.Orientation() == TopAbs_REVERSED)
    aMult = -1.0;

  BRepOffset_Status aStatus; // set by BRepOffset1::Surface(), could be used to check result...
  aNFD.myOffsetS = BRepOffset1::Surface(aS, aMult * myOffsetValue, aStatus, Standard_True);
  aNFD.myL       = TopLoc_Location(); // Null transformation.

  // Save offset surface in map.
  myFaceInfo.Bind(theFace, aNFD);
}

//=================================================================================================

void BRepOffset_SimpleOffset::FillEdgeData(
  const TopoEdge&                               theEdge,
  const TopTools_IndexedDataMapOfShapeListOfShape& theEdgeFaceMap,
  const Standard_Integer                           theIdx)
{
  const ShapeList& aFacesList = theEdgeFaceMap(theIdx);

  if (aFacesList.Size() == 0)
    return; // Free edges are skipped.

  // Get offset surface.
  const TopoFace& aCurrFace = TopoDS::Face(aFacesList.First());

  if (!myFaceInfo.IsBound(aCurrFace))
    return;

  // No need to deal with transformation - it is applied in fill faces data method.
  const NewFaceData&   aNFD         = myFaceInfo.Find(aCurrFace);
  Handle(GeomSurface) anOffsetSurf = aNFD.myOffsetS;

  // Compute offset 3d curve.
  Standard_Real        aF, aL;
  Handle(GeomCurve2d) aC2d = BRepInspector::CurveOnSurface(theEdge, aCurrFace, aF, aL);

  EdgeMaker anEdgeMaker(aC2d, anOffsetSurf, aF, aL);
  TopoEdge             aNewEdge = anEdgeMaker.Edge();

  // Compute max tolerance. Vertex tolerance usage is taken from existing offset computation
  // algorithm. This piece of code significantly influences resulting performance.
  Standard_Real aTol = BRepInspector::MaxTolerance(theEdge, TopAbs_VERTEX);
  BRepLib1::BuildCurves3d(aNewEdge, aTol);

  NewEdgeData aNED;
  aNED.myOffsetC = BRepInspector::Curve(aNewEdge, aNED.myL, aF, aL);

  // Iterate over adjacent faces for the current edge and compute max deviation.
  Standard_Real                  anEdgeTol = 0.0;
  ShapeList::Iterator anIter(aFacesList);
  for (; !aNED.myOffsetC.IsNull() && anIter.More(); anIter.Next())
  {
    const TopoFace& aCurFace = TopoDS::Face(anIter.Value());

    if (!myFaceInfo.IsBound(aCurFace))
      continue;

    // Create offset curve on surface.
    const Handle(GeomCurve2d)      aC2dNew = BRepInspector::CurveOnSurface(theEdge, aCurFace, aF, aL);
    const Handle(Adaptor2d_Curve2d) aHCurve2d = new Geom2dAdaptor_Curve(aC2dNew, aF, aL);
    const Handle(Adaptor3d_Surface) aHSurface =
      new GeomAdaptor_Surface(myFaceInfo.Find(aCurFace).myOffsetS);
    const Handle(Adaptor3d_CurveOnSurface) aCurveOnSurf =
      new Adaptor3d_CurveOnSurface(aHCurve2d, aHSurface);

    // Extract 3d-curve (it is not null).
    const Handle(Adaptor3d_Curve) aCurve3d = new GeomAdaptor_Curve(aNED.myOffsetC, aF, aL);

    // It is necessary to compute maximal deviation (tolerance).
    BRepLib_ValidateEdge aValidateEdge(aCurve3d, aCurveOnSurf, Standard_True);
    aValidateEdge.Process();
    if (aValidateEdge.IsDone())
    {
      Standard_Real aMaxTol1 = aValidateEdge.GetMaxDistance();
      anEdgeTol              = Max(anEdgeTol, aMaxTol1);
    }
  }
  aNED.myTol = Max(BRepInspector::Tolerance(aNewEdge), anEdgeTol);

  // Save computed 3d curve in map.
  myEdgeInfo.Bind(theEdge, aNED);
}

//=================================================================================================

void BRepOffset_SimpleOffset::FillVertexData(
  const TopoVertex&                             theVertex,
  const TopTools_IndexedDataMapOfShapeListOfShape& theVertexEdgeMap,
  const Standard_Integer                           theIdx)
{
  // Algorithm:
  // Find adjacent edges for the given vertex.
  // Find corresponding end on the each adjacent edge.
  // Get offset points for founded end.
  // Set result vertex position as barycenter of founded points.

  Point3d aCurrPnt = BRepInspector::Pnt(theVertex);

  const ShapeList& aEdgesList = theVertexEdgeMap(theIdx);

  if (aEdgesList.Size() == 0)
    return; // Free verices are skipped.

  // Array to store offset points.
  NCollection_Vector<Point3d> anOffsetPointVec;

  Standard_Real aMaxEdgeTol = 0.0;

  // Iterate over adjacent edges.
  ShapeList::Iterator anIterEdges(aEdgesList);
  for (; anIterEdges.More(); anIterEdges.Next())
  {
    const TopoEdge& aCurrEdge = TopoDS::Edge(anIterEdges.Value());

    if (!myEdgeInfo.IsBound(aCurrEdge))
      continue; // Skip shared edges with wrong orientation.

    // Find the closest bound.
    Standard_Real      aF, aL;
    Handle(GeomCurve3d) aC3d = BRepInspector::Curve(aCurrEdge, aF, aL);

    // Protection from degenerated edges.
    if (aC3d.IsNull())
      continue;

    const Point3d aPntF = aC3d->Value(aF);
    const Point3d aPntL = aC3d->Value(aL);

    const Standard_Real aSqDistF = aPntF.SquareDistance(aCurrPnt);
    const Standard_Real aSqDistL = aPntL.SquareDistance(aCurrPnt);

    Standard_Real aMinParam = aF, aMaxParam = aL;
    if (aSqDistL < aSqDistF)
    {
      // Square distance to last point is closer.
      aMinParam = aL;
      aMaxParam = aF;
    }

    // Compute point on offset edge.
    const NewEdgeData&        aNED          = myEdgeInfo.Find(aCurrEdge);
    const Handle(GeomCurve3d)& anOffsetCurve = aNED.myOffsetC;
    const Point3d              anOffsetPoint = anOffsetCurve->Value(aMinParam);
    anOffsetPointVec.Append(anOffsetPoint);

    // Handle situation when edge is closed.
    TopoVertex aV1, aV2;
    TopExp1::Vertices(aCurrEdge, aV1, aV2);
    if (aV1.IsSame(aV2))
    {
      const Point3d anOffsetPointLast = anOffsetCurve->Value(aMaxParam);
      anOffsetPointVec.Append(anOffsetPointLast);
    }

    aMaxEdgeTol = Max(aMaxEdgeTol, aNED.myTol);
  }

  // NCollection_Vector starts from 0 by default.
  // It's better to use lower() and upper() in this case instead of direct indexes range.
  Point3d aCenter(0.0, 0.0, 0.0);
  for (Standard_Integer i = anOffsetPointVec.Lower(); i <= anOffsetPointVec.Upper(); ++i)
  {
    aCenter.SetXYZ(aCenter.XYZ() + anOffsetPointVec.Value(i).XYZ());
  }
  aCenter.SetXYZ(aCenter.XYZ() / anOffsetPointVec.Size());

  // Compute max distance.
  Standard_Real aSqMaxDist = 0.0;
  for (Standard_Integer i = anOffsetPointVec.Lower(); i <= anOffsetPointVec.Upper(); ++i)
  {
    const Standard_Real aSqDist = aCenter.SquareDistance(anOffsetPointVec.Value(i));
    if (aSqDist > aSqMaxDist)
      aSqMaxDist = aSqDist;
  }

  const Standard_Real aResTol = Max(aMaxEdgeTol, Sqrt(aSqMaxDist));

  const Standard_Real aMultCoeff = 1.001; // Avoid tolernace problems.
  NewVertexData1       aNVD;
  aNVD.myP   = aCenter;
  aNVD.myTol = aResTol * aMultCoeff;

  // Save computed vertex info.
  myVertexInfo.Bind(theVertex, aNVD);
}
