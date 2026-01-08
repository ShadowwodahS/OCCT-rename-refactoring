// Created on: 1995-08-07
// Created by: Modelistation
// Copyright (c) 1995-1999 Matra Datavision
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

#include <StdPrs_ShapeTool.hxx>

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_ListOfShape.hxx>

//=================================================================================================

StdPrs_ShapeTool::StdPrs_ShapeTool(const TopoShape&    theShape,
                                   const Standard_Boolean theAllVertices)
    : myShape(theShape)
{
  myEdgeMap.Clear();
  myVertexMap.Clear();
  TopExp1::MapShapesAndAncestors(theShape, TopAbs_EDGE, TopAbs_FACE, myEdgeMap);

  ShapeExplorer anExpl;
  if (theAllVertices)
  {
    for (anExpl.Init(theShape, TopAbs_VERTEX); anExpl.More(); anExpl.Next())
    {
      myVertexMap.Add(anExpl.Current());
    }
  }
  else
  {
    // Extracting isolated vertices
    for (anExpl.Init(theShape, TopAbs_VERTEX, TopAbs_EDGE); anExpl.More(); anExpl.Next())
    {
      myVertexMap.Add(anExpl.Current());
    }

    // Extracting internal vertices
    for (anExpl.Init(theShape, TopAbs_EDGE); anExpl.More(); anExpl.Next())
    {
      TopoDS_Iterator aIt(anExpl.Current(), Standard_False, Standard_True);
      for (; aIt.More(); aIt.Next())
      {
        const TopoShape& aV = aIt.Value();
        if (aV.Orientation() == TopAbs_INTERNAL)
        {
          myVertexMap.Add(aV);
        }
      }
    }
  }
}

//=================================================================================================

Box2 StdPrs_ShapeTool::FaceBound() const
{
  const TopoFace& F = TopoDS::Face(myFaceExplorer.Current());
  Box2            B;
  BRepBndLib1::Add(F, B);
  return B;
}

//=================================================================================================

Standard_Boolean StdPrs_ShapeTool::IsPlanarFace(const TopoFace& theFace)
{
  TopLoc_Location             l;
  const Handle(GeomSurface)& S = BRepInspector::Surface(theFace, l);
  if (S.IsNull())
  {
    return Standard_False;
  }

  Handle(TypeInfo) TheType = S->DynamicType();

  if (TheType == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    Handle(Geom_RectangularTrimmedSurface) RTS =
      Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
    TheType = RTS->BasisSurface()->DynamicType();
  }
  return (TheType == STANDARD_TYPE(GeomPlane));
}

//=================================================================================================

Box2 StdPrs_ShapeTool::CurveBound() const
{
  const TopoEdge& E = TopoDS::Edge(myEdgeMap.FindKey(myEdge));
  Box2            B;
  BRepBndLib1::Add(E, B);
  return B;
}

//=================================================================================================

Standard_Integer StdPrs_ShapeTool::Neighbours() const
{
  const ShapeList& L = myEdgeMap.FindFromIndex(myEdge);
  return L.Extent();
}

//=================================================================================================

Handle(TopTools_HSequenceOfShape) StdPrs_ShapeTool::FacesOfEdge() const
{
  Handle(TopTools_HSequenceOfShape) H = new TopTools_HSequenceOfShape();
  const ShapeList&       L = myEdgeMap.FindFromIndex(myEdge);
  for (TopTools_ListIteratorOfListOfShape LI(L); LI.More(); LI.Next())
  {
    H->Append(LI.Value());
  }
  return H;
}

//=================================================================================================

Standard_Boolean StdPrs_ShapeTool::HasSurface() const
{
  TopLoc_Location             l;
  const Handle(GeomSurface)& S = BRepInspector::Surface(GetFace(), l);
  return !S.IsNull();
}

//=================================================================================================

Handle(MeshTriangulation) StdPrs_ShapeTool::CurrentTriangulation(TopLoc_Location& l) const
{
  return BRepInspector::Triangulation(GetFace(), l);
}

//=================================================================================================

Standard_Boolean StdPrs_ShapeTool::HasCurve() const
{
  return BRepInspector::IsGeometric(GetCurve());
}

//=================================================================================================

void StdPrs_ShapeTool::PolygonOnTriangulation(Handle(Poly_PolygonOnTriangulation)& Indices,
                                              Handle(MeshTriangulation)&          T,
                                              TopLoc_Location&                     l) const
{
  BRepInspector::PolygonOnTriangulation(GetCurve(), Indices, T, l);
}

//=================================================================================================

Handle(Poly_Polygon3D) StdPrs_ShapeTool::Polygon3D(TopLoc_Location& l) const
{
  return BRepInspector::Polygon3D(GetCurve(), l);
}
