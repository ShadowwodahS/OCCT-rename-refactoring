// Created on: 1995-06-26
// Created by: Philippe DERVIEUX
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

#include <ChFi2d.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

//=================================================================================================

Standard_Boolean ChFi2d1::CommonVertex(const TopoEdge& E1,
                                      const TopoEdge& E2,
                                      TopoVertex&     V)
{
  TopoVertex firstVertex1, lastVertex1, firstVertex2, lastVertex2;
  TopExp1::Vertices(E1, firstVertex1, lastVertex1);
  TopExp1::Vertices(E2, firstVertex2, lastVertex2);

  if (firstVertex1.IsSame(firstVertex2) || firstVertex1.IsSame(lastVertex2))
  {
    V = firstVertex1;
    return Standard_True;
  }
  if (lastVertex1.IsSame(firstVertex2) || lastVertex1.IsSame(lastVertex2))
  {
    V = lastVertex1;
    return Standard_True;
  }
  return Standard_False;
} // CommonVertex

//=================================================================================================

ChFi2d_ConstructionError ChFi2d1::FindConnectedEdges(const TopoFace&   F,
                                                    const TopoVertex& V,
                                                    TopoEdge&         E1,
                                                    TopoEdge&         E2)
{
  TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
  TopExp1::MapShapesAndAncestors(F, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);

  if (vertexMap.Contains(V))
  {
    TopTools_ListIteratorOfListOfShape iterator(vertexMap.FindFromKey(V));
    if (iterator.More())
    {
      E1 = TopoDS::Edge(iterator.Value());
      iterator.Next();
    } // if ...
    else
      return ChFi2d_ConnexionError;
    if (iterator.More())
    {
      E2 = TopoDS::Edge(iterator.Value());
      iterator.Next();
    } // if ...
    else
      return ChFi2d_ConnexionError;

    if (iterator.More())
      return ChFi2d_ConnexionError;
  } // if (isFind)
  else
    return ChFi2d_ConnexionError;
  return ChFi2d_IsDone;
} // FindConnectedEdges
