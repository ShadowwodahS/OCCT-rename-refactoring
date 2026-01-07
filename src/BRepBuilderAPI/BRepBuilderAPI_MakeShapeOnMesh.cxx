// Created on: 2022-06-30
// Created by: Alexander MALYSHEV
// Copyright (c) 2022-2022 OPEN CASCADE SAS
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

#include <BRepBuilderAPI_MakeShapeOnMesh.hxx>

#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <NCollection_IndexedDataMap.hxx>
#include <Standard_HashUtils.hxx>

namespace
{
//! Structure representing mesh edge.
struct Edge
{
  //! Constructor. Sets edge nodes.
  Edge(const Standard_Integer TheIdx1, const Standard_Integer TheIdx2)
      : Idx1(Min(TheIdx1, TheIdx2)),
        Idx2(Max(TheIdx1, TheIdx2))
  {
  }

  //! Comparison operator.
  Standard_Boolean operator<(const Edge& other) const
  {
    if (Idx1 < other.Idx1 || (Idx1 == other.Idx1 && Idx2 < other.Idx2))
    {
      return Standard_True;
    }

    return Standard_False;
  }

  bool operator==(const Edge& theOther) const
  {
    return theOther.Idx1 == Idx1 && theOther.Idx2 == Idx2;
  }

  //! First index. It is lower or equal than the second.
  Standard_Integer Idx1;

  //! Second index.
  Standard_Integer Idx2;
};
} // namespace

namespace std
{
template <>
struct hash<Edge>
{
  size_t operator()(const Edge& theEdge) const noexcept
  {
    // Combine two int values into a single hash value.
    int aCombination[2]{theEdge.Idx1, theEdge.Idx2};
    return opencascade::hashBytes(aCombination, sizeof(aCombination));
  }
};
} // namespace std

//=================================================================================================

void BRepBuilderAPI_MakeShapeOnMesh::Build(const Message_ProgressRange& theRange)
{
  // Generally, this method guarantees topology sharing by mapping mesh primitives
  // into topological counterparts.
  // mesh points -> topological vertices
  // mesh edges  -> topological edges

  // Cannot reconstruct anything from null or empty mesh.
  if (myMesh.IsNull() || myMesh->NbNodes() == 0 || myMesh->NbTriangles() == 0)
    return;

  const Standard_Integer aNbNodes     = myMesh->NbNodes();
  const Standard_Integer aNbTriangles = myMesh->NbTriangles();

  // We are going to have three loops: iterate once over nodes and iterate twice
  // over triangles of input mesh.
  Message_ProgressScope aPS(theRange,
                            "Per-facet shape construction",
                            Standard_Real(aNbNodes + 2 * aNbTriangles));

  // Build shared vertices.
  NCollection_IndexedDataMap<Standard_Integer, TopoVertex> aPnt2VertexMap;

  for (Standard_Integer i = 1; i <= aNbNodes; ++i)
  {
    aPS.Next();
    if (aPS.UserBreak())
      return;

    const Point3d        aP = myMesh->Node(i);
    const TopoVertex aV = BRepBuilderAPI_MakeVertex(aP);
    aPnt2VertexMap.Add(i, aV);
  }

  // Build shared edges.
  NCollection_IndexedDataMap<Edge, TopoEdge> anEdgeToTEgeMap;
  for (Standard_Integer i = 1; i <= aNbTriangles; ++i)
  {
    aPS.Next();
    if (aPS.UserBreak())
      return;

    Standard_Integer     anIdx[3];
    const Poly_Triangle& aTriangle = myMesh->Triangle(i);
    aTriangle.Get(anIdx[0], anIdx[1], anIdx[2]);

    // Skip degenerated triangles.
    if (anIdx[0] == anIdx[1] || anIdx[0] == anIdx[2] || anIdx[1] == anIdx[2])
      continue;

    const Point3d        aP1 = myMesh->Node(anIdx[0]);
    const Point3d        aP2 = myMesh->Node(anIdx[1]);
    const Point3d        aP3 = myMesh->Node(anIdx[2]);
    const Standard_Real aD1 = aP1.SquareDistance(aP2);
    const Standard_Real aD2 = aP1.SquareDistance(aP3);
    const Standard_Real aD3 = aP2.SquareDistance(aP3);
    if (aD1 < gp::Resolution() || aD2 < gp::Resolution() || aD3 < gp::Resolution())
    {
      continue;
    }

    // Edges are constructed in forward order for the existing normals orientation.
    // In MeshTriangulation, positive direction is defined as cross product:
    // (aV1, aV2) x (aV1, aV3).
    const TopoVertex& aV1 = aPnt2VertexMap.FindFromKey(anIdx[0]);
    const TopoVertex& aV2 = aPnt2VertexMap.FindFromKey(anIdx[1]);
    const TopoVertex& aV3 = aPnt2VertexMap.FindFromKey(anIdx[2]);

    const Edge aMeshEdge1(anIdx[0], anIdx[1]);
    const Edge aMeshEdge2(anIdx[1], anIdx[2]);
    const Edge aMeshEdge3(anIdx[2], anIdx[0]);

    EdgeMaker aMaker1(aV1, aV2);
    EdgeMaker aMaker2(aV2, aV3);
    EdgeMaker aMaker3(aV3, aV1);

    TopoEdge aTE1 = aMaker1.Edge();
    if (anIdx[1] < anIdx[0])
      aTE1.Reverse();

    TopoEdge aTE2 = aMaker2.Edge();
    if (anIdx[2] < anIdx[1])
      aTE2.Reverse();

    TopoEdge aTE3 = aMaker3.Edge();
    if (anIdx[0] < anIdx[2])
      aTE3.Reverse();

    anEdgeToTEgeMap.Add(aMeshEdge1, aTE1);
    anEdgeToTEgeMap.Add(aMeshEdge2, aTE2);
    anEdgeToTEgeMap.Add(aMeshEdge3, aTE3);
  }

  // Construct planar faces using shared topology.
  TopoCompound aResult;
  ShapeBuilder    aBB;
  aBB.MakeCompound(aResult);
  for (Standard_Integer i = 1; i <= aNbTriangles; ++i)
  {
    aPS.Next();
    if (aPS.UserBreak())
      return;

    Standard_Integer     anIdx[3];
    const Poly_Triangle& aTriangle = myMesh->Triangle(i);
    aTriangle.Get(anIdx[0], anIdx[1], anIdx[2]);

    const Edge             aMeshEdge1(anIdx[0], anIdx[1]);
    const Edge             aMeshEdge2(anIdx[1], anIdx[2]);
    const Edge             aMeshEdge3(anIdx[2], anIdx[0]);
    const Standard_Boolean isReversed1 = anIdx[1] < anIdx[0];
    const Standard_Boolean isReversed2 = anIdx[2] < anIdx[1];
    const Standard_Boolean isReversed3 = anIdx[0] < anIdx[2];

    // Edges can be skipped in case of mesh defects - topologically or geometrically
    // degenerated triangles.
    const Standard_Boolean aHasAllEdges = anEdgeToTEgeMap.Contains(aMeshEdge1)
                                          && anEdgeToTEgeMap.Contains(aMeshEdge2)
                                          && anEdgeToTEgeMap.Contains(aMeshEdge3);
    if (!aHasAllEdges)
      continue;

    TopoEdge aTEdge1 = anEdgeToTEgeMap.FindFromKey(aMeshEdge1);
    if (isReversed1)
      aTEdge1.Reverse();
    TopoEdge aTEdge2 = anEdgeToTEgeMap.FindFromKey(aMeshEdge2);
    if (isReversed2)
      aTEdge2.Reverse();
    TopoEdge aTEdge3 = anEdgeToTEgeMap.FindFromKey(aMeshEdge3);
    if (isReversed3)
      aTEdge3.Reverse();

    BRepBuilderAPI_MakeWire aWireMaker;
    aWireMaker.Add(aTEdge1);
    aWireMaker.Add(aTEdge2);
    aWireMaker.Add(aTEdge3);
    const TopoWire aWire = aWireMaker.Wire();

    // Construct plane explicitly since it is faster than automatic construction
    // within FaceMaker.
    BRepAdaptor_Curve aC1(aTEdge1);
    BRepAdaptor_Curve aC2(aTEdge2);
    const Dir3d      aD1 = aC1.Line().Direction();
    const Dir3d      aD2 = aC2.Line().Direction();
    gp_XYZ            aN  = aD1.XYZ().Crossed(aD2.XYZ());
    if (aN.SquareModulus() < Precision::SquareConfusion())
      continue;
    if (aTEdge1.Orientation() == TopAbs_REVERSED)
      aN.Reverse();
    if (aTEdge2.Orientation() == TopAbs_REVERSED)
      aN.Reverse();
    const Dir3d aNorm(aN);
    gp_Pln       aPln(myMesh->Node(anIdx[0]), aNorm);

    FaceMaker aFaceMaker(aPln, aWire);
    const TopoFace&      aFace = aFaceMaker.Face();

    aBB.Add(aResult, aFace);
  }

  this->Done();
  myShape = aResult;
}
