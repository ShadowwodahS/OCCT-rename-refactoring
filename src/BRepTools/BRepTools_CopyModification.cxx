// Copyright (c) 1999-2022 OPEN CASCADE SAS
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

#include <BRepTools_CopyModification.hxx>

#include <BRep_Tool.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepTools_CopyModification, BRepTools_Modification)

//=================================================================================================

BRepTools_CopyModification::BRepTools_CopyModification(const Standard_Boolean copyGeom,
                                                       const Standard_Boolean copyMesh)
    : myCopyGeom(copyGeom),
      myCopyMesh(copyMesh)
{
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewSurface(const TopoFace&    theFace,
                                                        Handle(GeomSurface)& theSurf,
                                                        TopLoc_Location&      theLoc,
                                                        Standard_Real&        theTol,
                                                        Standard_Boolean&     theRevWires,
                                                        Standard_Boolean&     theRevFace)
{
  theSurf     = BRepInspector::Surface(theFace, theLoc);
  theTol      = BRepInspector::Tolerance(theFace);
  theRevWires = theRevFace = Standard_False;

  if (!theSurf.IsNull() && myCopyGeom)
    theSurf = Handle(GeomSurface)::DownCast(theSurf->Copy());

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewTriangulation(const TopoFace&          theFace,
                                                              Handle(MeshTriangulation)& theTri)
{
  if (!myCopyMesh && BRepInspector::IsGeometric(theFace))
  {
    return Standard_False;
  }

  TopLoc_Location aLoc;
  theTri = BRepInspector::Triangulation(theFace, aLoc);

  if (theTri.IsNull())
    return Standard_False;

  // mesh is copied if and only if the geometry need to be copied too
  if (myCopyGeom)
    theTri = theTri->Copy();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewCurve(const TopoEdge&  theEdge,
                                                      Handle(GeomCurve3d)& theCurve,
                                                      TopLoc_Location&    theLoc,
                                                      Standard_Real&      theTol)
{
  Standard_Real aFirst, aLast;
  theCurve = BRepInspector::Curve(theEdge, theLoc, aFirst, aLast);
  theTol   = BRepInspector::Tolerance(theEdge);

  if (!theCurve.IsNull() && myCopyGeom)
    theCurve = Handle(GeomCurve3d)::DownCast(theCurve->Copy());

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewPolygon(const TopoEdge&      theEdge,
                                                        Handle(Poly_Polygon3D)& thePoly)
{
  if (!myCopyMesh && BRepInspector::IsGeometric(theEdge))
  {
    return Standard_False;
  }

  TopLoc_Location aLoc;
  thePoly = BRepInspector::Polygon3D(theEdge, aLoc);

  if (thePoly.IsNull())
    return Standard_False;

  // polygon is copied if and only if the geometry need to be copied too
  if (myCopyGeom)
    thePoly = thePoly->Copy();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewPolygonOnTriangulation(
  const TopoEdge&                   theEdge,
  const TopoFace&                   theFace,
  Handle(Poly_PolygonOnTriangulation)& thePoly)
{
  if (!myCopyMesh && BRepInspector::IsGeometric(theEdge))
  {
    return Standard_False;
  }

  TopLoc_Location            aLoc;
  Handle(MeshTriangulation) aTria = BRepInspector::Triangulation(theFace, aLoc);
  thePoly                          = BRepInspector::PolygonOnTriangulation(theEdge, aTria, aLoc);

  if (thePoly.IsNull())
    return Standard_False;

  // polygon is copied if and only if the geometry need to be copied too
  if (myCopyGeom)
    thePoly = thePoly->Copy();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewPoint(const TopoVertex& theVertex,
                                                      Point3d&              thePnt,
                                                      Standard_Real&       theTol)
{
  thePnt = BRepInspector::Pnt(theVertex);
  theTol = BRepInspector::Tolerance(theVertex);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewCurve2d(const TopoEdge& theEdge,
                                                        const TopoFace& theFace,
                                                        const TopoEdge&,
                                                        const TopoFace&,
                                                        Handle(GeomCurve2d)& theCurve,
                                                        Standard_Real&        theTol)
{
  theTol = BRepInspector::Tolerance(theEdge);
  Standard_Real aFirst, aLast;
  theCurve = BRepInspector::CurveOnSurface(theEdge, theFace, aFirst, aLast);

  if (!theCurve.IsNull() && myCopyGeom)
    theCurve = Handle(GeomCurve2d)::DownCast(theCurve->Copy());

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_CopyModification::NewParameter(const TopoVertex& theVertex,
                                                          const TopoEdge&   theEdge,
                                                          Standard_Real&       thePnt,
                                                          Standard_Real&       theTol)
{
  if (theVertex.IsNull())
    return Standard_False; // infinite edge may have Null vertex

  theTol = BRepInspector::Tolerance(theVertex);
  thePnt = BRepInspector::Parameter(theVertex, theEdge);

  return Standard_True;
}

//=================================================================================================

GeomAbs_Shape BRepTools_CopyModification::Continuity(const TopoEdge& theEdge,
                                                     const TopoFace& theFace1,
                                                     const TopoFace& theFace2,
                                                     const TopoEdge&,
                                                     const TopoFace&,
                                                     const TopoFace&)
{
  return BRepInspector::Continuity(theEdge, theFace1, theFace2);
}
