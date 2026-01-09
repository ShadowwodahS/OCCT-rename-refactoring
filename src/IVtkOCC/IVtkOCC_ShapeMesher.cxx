// Created on: 2011-10-14
// Created by: Roman KOZLOV
// Copyright (c) 2011-2014 OPEN CASCADE SAS
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

#include <IVtkOCC_ShapeMesher.hxx>

#include <Bnd_Box.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Message.hxx>
#include <NCollection_Array1.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Prs3d.hxx>
#include <Prs3d_Drawer.hxx>
#include <Standard_ErrorHandler.hxx>
#include <StdPrs_Isolines.hxx>
#include <StdPrs_ToolTriangulatedShape.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IVtkOCC_ShapeMesher, IVtk_IShapeMesher)

//=================================================================================================

IVtkOCC_ShapeMesher::IVtkOCC_ShapeMesher()
{
  //
}

//=================================================================================================

IVtkOCC_ShapeMesher::~IVtkOCC_ShapeMesher()
{
  //
}

//=================================================================================================

void IVtkOCC_ShapeMesher::internalBuild()
{
  const TopoShape& anOcctShape = GetShapeObj()->GetShape();
  if (anOcctShape.IsNull())
  {
    return;
  }

  const Handle(StyleDrawer)& anOcctDrawer = GetShapeObj()->Attributes();
  const Standard_Real         aShapeDeflection =
    StdPrs_ToolTriangulatedShape::GetDeflection(anOcctShape, anOcctDrawer);
  if (anOcctDrawer->IsAutoTriangulation())
  {
    StdPrs_ToolTriangulatedShape::ClearOnOwnDeflectionChange(anOcctShape, anOcctDrawer, true);
    StdPrs_ToolTriangulatedShape::Tessellate(anOcctShape, anOcctDrawer);
  }
  for (ShapeExplorer aFaceIter(anOcctShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace& anOcctFace = TopoDS::Face(aFaceIter.Current());
    TopLoc_Location    aLoc;
    if (const Handle(MeshTriangulation)& anOcctTriangulation =
          BRepInspector::Triangulation(anOcctFace, aLoc))
    {
      StdPrs_ToolTriangulatedShape::ComputeNormals(anOcctFace, anOcctTriangulation);
    }
  }

  // Free vertices and free edges should always be shown.
  // Shared edges are needed in WF representation only.
  // TODO: how to filter free edges at visualization level????
  addFreeVertices();
  addEdges();

  // Build wireframe points and cells (lines for isolines)
  for (ShapeExplorer aFaceIter(anOcctShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace& anOcctFace = TopoDS::Face(aFaceIter.Current());
    try
    {
      OCC_CATCH_SIGNALS
      addWFFace(anOcctFace, GetShapeObj()->GetSubShapeId(anOcctFace), aShapeDeflection);
    }
    catch (const ExceptionBase& anException)
    {
      Message1::SendFail(AsciiString1(
                          "Error: addWireFrameFaces() wireframe presentation builder has failed (")
                        + anException.GetMessageString() + ")");
    }
  }

  // Build shaded representation (based on MeshTriangulation)
  for (ShapeExplorer aFaceIter(anOcctShape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
  {
    const TopoFace& anOcctFace = TopoDS::Face(aFaceIter.Current());
    addShadedFace(anOcctFace, GetShapeObj()->GetSubShapeId(anOcctFace));
  }
}

//=================================================================================================

const IVtkOCC_Shape::Handle IVtkOCC_ShapeMesher::GetShapeObj() const
{
  return IVtkOCC_Shape::Handle::DownCast(myShapeObj);
}

//=================================================================================================

Standard_Real IVtkOCC_ShapeMesher::GetDeflection() const
{
  const TopoShape& anOcctShape = GetShapeObj()->GetShape();
  return !anOcctShape.IsNull()
           ? StdPrs_ToolTriangulatedShape::GetDeflection(anOcctShape, GetShapeObj()->Attributes())
           : 0.0;
}

//=================================================================================================

Standard_Real IVtkOCC_ShapeMesher::GetDeviationCoeff() const
{
  if (IVtkOCC_Shape::Handle aShape = GetShapeObj())
  {
    return aShape->Attributes()->DeviationCoefficient();
  }
  return 0.0;
}

//=================================================================================================

Standard_Real IVtkOCC_ShapeMesher::GetDeviationAngle() const
{
  if (IVtkOCC_Shape::Handle aShape = GetShapeObj())
  {
    return aShape->Attributes()->DeviationAngle();
  }
  return 0.0;
}

//=================================================================================================

void IVtkOCC_ShapeMesher::addFreeVertices()
{
  TopTools_IndexedDataMapOfShapeListOfShape aVertexMap;
  TopExp1::MapShapesAndAncestors(GetShapeObj()->GetShape(), TopAbs_VERTEX, TopAbs_EDGE, aVertexMap);

  Standard_Integer aVertNum = aVertexMap.Extent();
  IVtk_MeshType    aType;
  for (Standard_Integer anIt = 1; anIt <= aVertNum; anIt++)
  {
    if (aVertexMap.FindFromIndex(anIt).IsEmpty())
    {
      aType = MT_FreeVertex;
    }
    else
    {
      aType = MT_SharedVertex;
    }
    const TopoVertex& aVertex = TopoDS::Vertex(aVertexMap.FindKey(anIt));
    addVertex(aVertex, GetShapeObj()->GetSubShapeId(aVertex), aType);
  }
}

//=================================================================================================

void IVtkOCC_ShapeMesher::addEdges()
{
  TopTools_IndexedDataMapOfShapeListOfShape anEdgesMap;
  TopExp1::MapShapesAndAncestors(GetShapeObj()->GetShape(), TopAbs_EDGE, TopAbs_FACE, anEdgesMap);
  int           aNbFaces;
  IVtk_MeshType aType;
  myEdgesTypes.Clear();

  TopTools_IndexedDataMapOfShapeListOfShape::Iterator aEdgeIt(anEdgesMap);
  for (; aEdgeIt.More(); aEdgeIt.Next())
  {
    const TopoEdge&          anOcctEdge = TopoDS::Edge(aEdgeIt.Key1());
    const ShapeList& aFaceList  = aEdgeIt.Value();
    aNbFaces                               = aFaceList.Extent();
    if (aNbFaces == 0)
    {
      aType = MT_FreeEdge;
    }
    else if (aNbFaces == 1)
    {
      aType = MT_BoundaryEdge;
    }
    else
    {
      aType = (aNbFaces >= 2) && (BRepInspector::MaxContinuity(anOcctEdge) > GeomAbs_G2)
                ? MT_SeamEdge
                : MT_SharedEdge;
    }
    addEdge(anOcctEdge, GetShapeObj()->GetSubShapeId(anOcctEdge), aType);
    myEdgesTypes.Bind(anOcctEdge, aType);
  }
}

//=================================================================================================

void IVtkOCC_ShapeMesher::addVertex(const TopoVertex& theVertex,
                                    const IVtk_IdType    theShapeId,
                                    const IVtk_MeshType  theMeshType)
{
  if (theVertex.IsNull())
  {
    return;
  }

  Point3d aPnt3d = BRepInspector::Pnt(theVertex);

  IVtk_PointId anId = myShapeData->InsertCoordinate(aPnt3d);
  myShapeData->InsertVertex(theShapeId, anId, theMeshType);
}

//=================================================================================================

void IVtkOCC_ShapeMesher::addEdge(const TopoEdge&  theEdge,
                                  const IVtk_IdType   theShapeId,
                                  const IVtk_MeshType theMeshType)
{
  if (theEdge.IsNull() || BRepInspector::Degenerated(theEdge))
  {
    return;
  }

  Handle(Poly_PolygonOnTriangulation) aPolyOnTriangulation;
  Handle(MeshTriangulation)          aTriangulation;
  TopLoc_Location                     aLoc;
  BRepInspector::PolygonOnTriangulation(theEdge, aPolyOnTriangulation, aTriangulation, aLoc, 1);
  if (!aPolyOnTriangulation.IsNull() && aPolyOnTriangulation->NbNodes() >= 2)
  {
    // prefer polygon on triangulation when defined
    const Transform3d aTrsf        = aLoc.Transformation();
    const bool    hasTransform = !aLoc.IsIdentity();

    IVtk_PointIdList       aPolyPointIds;
    const Standard_Integer aNbNodes = aPolyOnTriangulation->NbNodes();
    for (Standard_Integer aJ = 0; aJ < aNbNodes; aJ++)
    {
      const Standard_Integer aPntId = aPolyOnTriangulation->Node(aJ + 1);
      Point3d                 aPoint = aTriangulation->Node(aPntId);
      Dir3d aNorm = aTriangulation->HasNormals() ? aTriangulation->Normal(aPntId) : gp1::DZ();
      if (hasTransform)
      {
        aPoint.Transform(aTrsf);
        aNorm.Transform(aTrsf);
      }

      IVtk_PointId anId = myShapeData->InsertPoint(
        aPoint,
        Graphic3d_Vec3((float)aNorm.X(), (float)aNorm.Y(), (float)aNorm.Z()));
      aPolyPointIds.Append(anId);
    }
    myShapeData->InsertLine(theShapeId, &aPolyPointIds, theMeshType);
    return;
  }

  // try polygon 3d
  Handle(Poly_Polygon3D) aPoly3d = BRepInspector::Polygon3D(theEdge, aLoc);
  if (aPoly3d.IsNull() || aPoly3d->NbNodes() < 2)
  {
    return;
  }

  const Transform3d    anEdgeTransf = aLoc.Transformation();
  const bool       noTransform  = aLoc.IsIdentity();
  IVtk_PointIdList aPolyPointIds;
  for (Standard_Integer aNodeIter = 1; aNodeIter <= aPoly3d->NbNodes(); ++aNodeIter)
  {
    Point3d aPnt = aPoly3d->Nodes().Value(aNodeIter);
    if (!noTransform)
    {
      aPnt.Transform(anEdgeTransf);
    }

    const IVtk_PointId anId = myShapeData->InsertCoordinate(aPnt);
    aPolyPointIds.Append(anId);
  }
  myShapeData->InsertLine(theShapeId, &aPolyPointIds, theMeshType);
}

//=================================================================================================

void IVtkOCC_ShapeMesher::addWFFace(const TopoFace&  theFace,
                                    const IVtk_IdType   theShapeId,
                                    const Standard_Real theDeflection)
{
  if (theFace.IsNull())
  {
    return;
  }

  TopoFace aFaceToMesh = theFace;
  aFaceToMesh.Orientation(TopAbs_FORWARD);

  // Add face's edges here but with the face ID
  for (ShapeExplorer anEdgeIter(aFaceToMesh, TopAbs_EDGE); anEdgeIter.More(); anEdgeIter.Next())
  {
    const TopoEdge& anOcctEdge = TopoDS::Edge(anEdgeIter.Current());
    addEdge(anOcctEdge, theShapeId, myEdgesTypes(anOcctEdge));
  }

  TopLoc_Location             aLoc;
  const Handle(GeomSurface)& aGeomSurf = BRepInspector::Surface(aFaceToMesh, aLoc);
  if (aGeomSurf.IsNull())
  {
    return;
  }

  Prs3d_NListOfSequenceOfPnt aPolylines;
  StdPrs_Isolines::Add(theFace, GetShapeObj()->Attributes(), theDeflection, aPolylines, aPolylines);
  for (Prs3d_NListOfSequenceOfPnt::Iterator aPolyIter(aPolylines); aPolyIter.More();
       aPolyIter.Next())
  {
    const Handle(PointSequence2)& aPoints    = aPolyIter.Value();
    const Standard_Integer               theNbNodes = aPoints->Length();
    if (theNbNodes < 2)
    {
      continue;
    }

    IVtk_PointIdList aPolyPointIds;
    for (PointSequence2::Iterator aNodeIter(*aPoints); aNodeIter.More(); aNodeIter.Next())
    {
      const Point3d&      aPnt = aNodeIter.Value();
      const IVtk_PointId anId = myShapeData->InsertCoordinate(aPnt);
      aPolyPointIds.Append(anId);
    }

    myShapeData->InsertLine(theShapeId, &aPolyPointIds, MT_IsoLine);
  }
}

//=================================================================================================

void IVtkOCC_ShapeMesher::addShadedFace(const TopoFace& theFace, const IVtk_IdType theShapeId)
{
  if (theFace.IsNull())
  {
    return;
  }

  TopLoc_Location                   aLoc;
  const Handle(MeshTriangulation)& anOcctTriangulation = BRepInspector::Triangulation(theFace, aLoc);
  if (anOcctTriangulation.IsNull())
  {
    return;
  }

  // Determinant of transform matrix less then 0 means that mirror transform applied
  const Transform3d aTrsf        = aLoc.Transformation();
  const bool    hasTransform = !aLoc.IsIdentity();
  const bool    isMirrored   = aTrsf.VectorialPart().Determinant() < 0;

  // Get triangulation points.
  Standard_Integer aNbPoints = anOcctTriangulation->NbNodes();

  // Keep inserted points id's of triangulation in an array.
  NCollection_Array1<IVtk_PointId> aPointIds(1, aNbPoints);
  IVtk_PointId                     anId;
  for (Standard_Integer anI = 1; anI <= aNbPoints; anI++)
  {
    Point3d aPoint = anOcctTriangulation->Node(anI);
    Dir3d aNorm  = anOcctTriangulation->HasNormals() ? anOcctTriangulation->Normal(anI) : gp1::DZ();
    if ((theFace.Orientation() == TopAbs_REVERSED) ^ isMirrored)
    {
      aNorm.Reverse();
    }
    if (hasTransform)
    {
      aPoint.Transform(aTrsf);
      aNorm.Transform(aTrsf);
    }

    // Add a point into output shape data and keep its id in the array.
    anId = myShapeData->InsertPoint(
      aPoint,
      Graphic3d_Vec3((float)aNorm.X(), (float)aNorm.Y(), (float)aNorm.Z()));
    aPointIds.SetValue(anI, anId);
  }

  // Create triangles on the created triangulation points.
  const Standard_Integer aNbTriangles = anOcctTriangulation->NbTriangles();
  Standard_Integer       aN1, aN2, aN3;
  for (Standard_Integer anI = 1; anI <= aNbTriangles; anI++)
  {
    if (theFace.Orientation() == TopAbs_REVERSED)
    {
      anOcctTriangulation->Triangle1(anI).Get(aN1, aN3, aN2);
    }
    else
    {
      anOcctTriangulation->Triangle1(anI).Get(aN1, aN2, aN3);
    }

    // Insert new triangle on these points into output shape data.
    myShapeData->InsertTriangle(theShapeId,
                                aPointIds(aN1),
                                aPointIds(aN2),
                                aPointIds(aN3),
                                MT_ShadedFace);
  }
}
