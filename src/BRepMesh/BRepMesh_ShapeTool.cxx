// Created on: 2016-04-19
// Copyright (c) 2016 OPEN CASCADE SAS
// Created by: Oleg AGASHIN
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

#include <BRepMesh_ShapeTool.hxx>
#include <IMeshData_Edge.hxx>
#include <IMeshData_PCurve.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>
#include <BRep_Builder.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <Precision.hxx>
#include <Bnd_Box.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepMesh_ShapeTool, RefObject)

namespace
{
//! Auxiliary struct to take a tolerance of edge.
struct EdgeTolerance
{
  static Standard_Real Get(const TopoShape& theEdge)
  {
    return BRepInspector::Tolerance(TopoDS::Edge(theEdge));
  }
};

//! Auxiliary struct to take a tolerance of vertex.
struct VertexTolerance
{
  static Standard_Real Get(const TopoShape& theVertex)
  {
    return BRepInspector::Tolerance(TopoDS::Vertex(theVertex));
  }
};

//! Returns maximum tolerance of face element of the specified type.
template <TopAbs_ShapeEnum ShapeType, class ToleranceExtractor>
Standard_Real MaxTolerance(const TopoFace& theFace)
{
  Standard_Real   aMaxTolerance = RealFirst();
  ShapeExplorer aExplorer(theFace, ShapeType);
  for (; aExplorer.More(); aExplorer.Next())
  {
    Standard_Real aTolerance = ToleranceExtractor::Get(aExplorer.Current());
    if (aTolerance > aMaxTolerance)
      aMaxTolerance = aTolerance;
  }

  return aMaxTolerance;
}
} // namespace

//=================================================================================================

Standard_Real BRepMesh_ShapeTool::MaxFaceTolerance(const TopoFace& theFace)
{
  Standard_Real aMaxTolerance = BRepInspector::Tolerance(theFace);

  Standard_Real aTolerance = Max(MaxTolerance<TopAbs_EDGE, EdgeTolerance>(theFace),
                                 MaxTolerance<TopAbs_VERTEX, VertexTolerance>(theFace));

  return Max(aMaxTolerance, aTolerance);
}

//=================================================================================================

void BRepMesh_ShapeTool::BoxMaxDimension(const Box2& theBox, Standard_Real& theMaxDimension)
{
  if (theBox.IsVoid())
    return;

  Standard_Real aMinX, aMinY, aMinZ, aMaxX, aMaxY, aMaxZ;
  theBox.Get(aMinX, aMinY, aMinZ, aMaxX, aMaxY, aMaxZ);

  theMaxDimension = Max(aMaxX - aMinX, Max(aMaxY - aMinY, aMaxZ - aMinZ));
}

//=================================================================================================

void BRepMesh_ShapeTool::CheckAndUpdateFlags(const IMeshData::IEdgeHandle&   theEdge,
                                             const IMeshData::IPCurveHandle& thePCurve)
{
  if (!theEdge->GetSameParam() && !theEdge->GetSameRange() && theEdge->GetDegenerated())
  {
    // Nothing to do worse.
    return;
  }

  const TopoEdge& aEdge = theEdge->GetEdge();
  const TopoFace& aFace = thePCurve->GetFace()->GetFace();

  Handle(GeomCurve3d) aCurve;
  Standard_Real      aFirstParam, aLastParam;
  Range(aEdge, aCurve, aFirstParam, aLastParam);
  if (aCurve.IsNull())
  {
    theEdge->SetDegenerated(Standard_True);
    return;
  }

  BRepAdaptor_Curve aCurveOnSurf(aEdge, aFace);
  if (theEdge->GetSameParam() || theEdge->GetSameRange())
  {
    if (theEdge->GetSameRange())
    {
      const Standard_Real aDiffFirst = aCurveOnSurf.FirstParameter() - aFirstParam;
      const Standard_Real aDiffLast  = aCurveOnSurf.LastParameter() - aLastParam;
      theEdge->SetSameRange(Abs(aDiffFirst) < Precision::PConfusion()
                            && Abs(aDiffLast) < Precision::PConfusion());

      if (!theEdge->GetSameRange())
      {
        theEdge->SetSameParam(Standard_False);
      }
    }
  }

  if (!theEdge->GetDegenerated() /* || theEdge->GetSameParam ()*/)
  {
    TopoVertex aStartVertex, aEndVertex;
    TopExp1::Vertices(aEdge, aStartVertex, aEndVertex);
    if (aStartVertex.IsNull() || aEndVertex.IsNull())
    {
      theEdge->SetDegenerated(Standard_True);
      return;
    }

    if (aStartVertex.IsSame(aEndVertex))
    {
      const Standard_Integer aPointsNb        = 20;
      const Standard_Real    aVertexTolerance = BRepInspector::Tolerance(aStartVertex);
      const Standard_Real    aDu              = (aLastParam - aFirstParam) / aPointsNb;
      // const Standard_Real    aEdgeTolerance     = BRepInspector::Tolerance (aEdge);
      // const Standard_Real    aSqEdgeTolerance   = aEdgeTolerance * aEdgeTolerance;

      Point3d aPrevPnt;
      aCurve->D0(aFirstParam, aPrevPnt);

      Standard_Real aLength = 0.0;
      for (Standard_Integer i = 1; i <= aPointsNb; ++i)
      {
        const Standard_Real aParameter = aFirstParam + i * aDu;
        // Calculation of the length of the edge in 3D
        // in order to check degenerativity
        Point3d aPnt;
        aCurve->D0(aParameter, aPnt);
        aLength += aPrevPnt.Distance(aPnt);

        // if (theEdge->GetSameParam ())
        //{
        //   // Check that points taken at the 3d and pcurve using
        //   // same parameter are within tolerance of an edge.
        //   Point3d aPntOnSurf;
        //   aCurveOnSurf.D0 (aParameter, aPntOnSurf);
        //   theEdge->SetSameParam (aPnt.SquareDistance (aPntOnSurf) < aSqEdgeTolerance);
        // }

        if (aLength > aVertexTolerance /*&& !theEdge->GetSameParam()*/)
        {
          break;
        }

        aPrevPnt = aPnt;
      }

      theEdge->SetDegenerated(aLength < aVertexTolerance);
    }
  }
}

//=================================================================================================

void BRepMesh_ShapeTool::AddInFace(const TopoFace&          theFace,
                                   Handle(MeshTriangulation)& theTriangulation)
{
  const TopLoc_Location& aLoc = theFace.Location();
  if (!aLoc.IsIdentity())
  {
    Transform3d aTrsf = aLoc.Transformation();
    aTrsf.Invert();
    for (Standard_Integer aNodeIter = 1; aNodeIter <= theTriangulation->NbNodes(); ++aNodeIter)
    {
      Point3d aNode = theTriangulation->Node(aNodeIter);
      aNode.Transform(aTrsf);
      theTriangulation->SetNode(aNodeIter, aNode);
    }
  }

  ShapeBuilder aBuilder;
  aBuilder.UpdateFace(theFace, theTriangulation);
}

//=================================================================================================

void BRepMesh_ShapeTool::NullifyFace(const TopoFace& theFace)
{
  ShapeBuilder aBuilder;
  aBuilder.UpdateFace(theFace, Handle(MeshTriangulation)());
}

//=================================================================================================

void BRepMesh_ShapeTool::NullifyEdge(const TopoEdge&                theEdge,
                                     const Handle(MeshTriangulation)& theTriangulation,
                                     const TopLoc_Location&            theLocation)
{
  UpdateEdge(theEdge, Handle(Poly_PolygonOnTriangulation)(), theTriangulation, theLocation);
}

//=================================================================================================

void BRepMesh_ShapeTool::NullifyEdge(const TopoEdge& theEdge, const TopLoc_Location& theLocation)
{
  ShapeBuilder aBuilder;
  aBuilder.UpdateEdge(theEdge, Handle(Poly_Polygon3D)(), theLocation);
}

//=================================================================================================

void BRepMesh_ShapeTool::UpdateEdge(const TopoEdge&                         theEdge,
                                    const Handle(Poly_PolygonOnTriangulation)& thePolygon,
                                    const Handle(MeshTriangulation)&          theTriangulation,
                                    const TopLoc_Location&                     theLocation)
{
  ShapeBuilder aBuilder;
  aBuilder.UpdateEdge(theEdge, thePolygon, theTriangulation, theLocation);
}

//=================================================================================================

void BRepMesh_ShapeTool::UpdateEdge(const TopoEdge&                         theEdge,
                                    const Handle(Poly_PolygonOnTriangulation)& thePolygon1,
                                    const Handle(Poly_PolygonOnTriangulation)& thePolygon2,
                                    const Handle(MeshTriangulation)&          theTriangulation,
                                    const TopLoc_Location&                     theLocation)
{
  ShapeBuilder aBuilder;
  aBuilder.UpdateEdge(theEdge, thePolygon1, thePolygon2, theTriangulation, theLocation);
}

//=================================================================================================

void BRepMesh_ShapeTool::UpdateEdge(const TopoEdge&            theEdge,
                                    const Handle(Poly_Polygon3D)& thePolygon)
{
  ShapeBuilder aBuilder;
  aBuilder.UpdateEdge(theEdge, thePolygon);
}

//=================================================================================================

Point3d BRepMesh_ShapeTool::UseLocation(const Point3d& thePnt, const TopLoc_Location& theLoc)
{
  if (theLoc.IsIdentity())
  {
    return thePnt;
  }

  return thePnt.Transformed(theLoc.Transformation());
}

//=================================================================================================

Standard_Boolean BRepMesh_ShapeTool::UVPoints(const TopoEdge&     theEdge,
                                              const TopoFace&     theFace,
                                              gp_Pnt2d&              theFirstPoint2d,
                                              gp_Pnt2d&              theLastPoint2d,
                                              const Standard_Boolean isConsiderOrientation)
{

  Handle(GeomCurve2d) aCurve2d;
  Standard_Real        aFirstParam, aLastParam;
  if (!Range(theEdge, theFace, aCurve2d, aFirstParam, aLastParam, isConsiderOrientation))
  {
    return Standard_False;
  }

  aCurve2d->D0(aFirstParam, theFirstPoint2d);
  aCurve2d->D0(aLastParam, theLastPoint2d);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepMesh_ShapeTool::Range(const TopoEdge&     theEdge,
                                           const TopoFace&     theFace,
                                           Handle(GeomCurve2d)&  thePCurve,
                                           Standard_Real&         theFirstParam,
                                           Standard_Real&         theLastParam,
                                           const Standard_Boolean isConsiderOrientation)
{

  Edge1 aEdge;
  return aEdge
    .PCurve(theEdge, theFace, thePCurve, theFirstParam, theLastParam, isConsiderOrientation);
}

//=================================================================================================

Standard_Boolean BRepMesh_ShapeTool::Range(const TopoEdge&     theEdge,
                                           Handle(GeomCurve3d)&    theCurve,
                                           Standard_Real&         theFirstParam,
                                           Standard_Real&         theLastParam,
                                           const Standard_Boolean isConsiderOrientation)
{

  Edge1 aEdge;
  return aEdge.Curve3d(theEdge, theCurve, theFirstParam, theLastParam, isConsiderOrientation);
}
