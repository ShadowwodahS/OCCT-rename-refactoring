// Created on: 1993-01-19
// Created by: Remi LEQUETTE
// Copyright (c) 1993-1999 Matra Datavision
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

#define No_Standard_NoMoreObject
#define No_Standard_NoSuchObject
#define No_Standard_TypeMismatch

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

void TopExp1::MapShapes(const TopoShape&         S,
                       const TopAbs_ShapeEnum      T,
                       TopTools_IndexedMapOfShape& M)
{
  ShapeExplorer Ex(S, T);
  while (Ex.More())
  {
    M.Add(Ex.Current());
    Ex.Next();
  }
}

//=================================================================================================

void TopExp1::MapShapes(const TopoShape&         S,
                       TopTools_IndexedMapOfShape& M,
                       const Standard_Boolean      cumOri,
                       const Standard_Boolean      cumLoc)
{
  M.Add(S);
  TopoDS_Iterator It(S, cumOri, cumLoc);
  while (It.More())
  {
    MapShapes(It.Value(), M);
    It.Next();
  }
}

//=================================================================================================

void TopExp1::MapShapes(const TopoShape&    S,
                       TopTools_MapOfShape&   M,
                       const Standard_Boolean cumOri,
                       const Standard_Boolean cumLoc)
{
  M.Add(S);
  TopoDS_Iterator It(S, cumOri, cumLoc);
  for (; It.More(); It.Next())
    MapShapes(It.Value(), M);
}

//=================================================================================================

void TopExp1::MapShapesAndAncestors(const TopoShape&                        S,
                                   const TopAbs_ShapeEnum                     TS,
                                   const TopAbs_ShapeEnum                     TA,
                                   TopTools_IndexedDataMapOfShapeListOfShape& M)
{
  ShapeList empty;

  // visit ancestors
  ShapeExplorer exa(S, TA);
  while (exa.More())
  {
    // visit shapes
    const TopoShape& anc = exa.Current();
    ShapeExplorer     exs(anc, TS);
    while (exs.More())
    {
      Standard_Integer index = M.FindIndex(exs.Current());
      if (index == 0)
        index = M.Add(exs.Current(), empty);
      M(index).Append(anc);
      exs.Next();
    }
    exa.Next();
  }

  // visit shapes not under ancestors
  ShapeExplorer ex(S, TS, TA);
  while (ex.More())
  {
    Standard_Integer index = M.FindIndex(ex.Current());
    if (index == 0)
      index = M.Add(ex.Current(), empty);
    ex.Next();
  }
}

//=================================================================================================

void TopExp1::MapShapesAndUniqueAncestors(const TopoShape&                        S,
                                         const TopAbs_ShapeEnum                     TS,
                                         const TopAbs_ShapeEnum                     TA,
                                         TopTools_IndexedDataMapOfShapeListOfShape& M,
                                         const Standard_Boolean                     useOrientation)
{
  ShapeList empty;

  // visit ancestors
  ShapeExplorer exa(S, TA);
  while (exa.More())
  {
    // visit shapes
    const TopoShape& anc = exa.Current();
    ShapeExplorer     exs(anc, TS);
    while (exs.More())
    {
      Standard_Integer index = M.FindIndex(exs.Current());
      if (index == 0)
        index = M.Add(exs.Current(), empty);
      ShapeList& aList = M(index);
      // check if anc already exists in a list
      TopTools_ListIteratorOfListOfShape it(aList);
      for (; it.More(); it.Next())
        if (useOrientation ? anc.IsEqual(it.Value()) : anc.IsSame(it.Value()))
          break;
      if (!it.More())
        aList.Append(anc);
      exs.Next();
    }
    exa.Next();
  }

  // visit shapes not under ancestors
  ShapeExplorer ex(S, TS, TA);
  while (ex.More())
  {
    Standard_Integer index = M.FindIndex(ex.Current());
    if (index == 0)
      M.Add(ex.Current(), empty);
    ex.Next();
  }
}

//=================================================================================================

TopoVertex TopExp1::FirstVertex(const TopoEdge& E, const Standard_Boolean CumOri)
{
  TopoDS_Iterator ite(E, CumOri);
  while (ite.More())
  {
    if (ite.Value().Orientation() == TopAbs_FORWARD)
      return TopoDS::Vertex(ite.Value());
    ite.Next();
  }
  return TopoVertex();
}

//=================================================================================================

TopoVertex TopExp1::LastVertex(const TopoEdge& E, const Standard_Boolean CumOri)
{
  TopoDS_Iterator ite(E, CumOri);
  while (ite.More())
  {
    if (ite.Value().Orientation() == TopAbs_REVERSED)
      return TopoDS::Vertex(ite.Value());
    ite.Next();
  }
  return TopoVertex();
}

//=================================================================================================

void TopExp1::Vertices(const TopoEdge&     E,
                      TopoVertex&         Vfirst,
                      TopoVertex&         Vlast,
                      const Standard_Boolean CumOri)
{
  // minor optimization for case when Vfirst and Vlast are non-null:
  // at least for VC++ 10, it is faster if we use boolean flags than
  // if we nullify vertices at that point (see #27021)
  Standard_Boolean isFirstDefined = Standard_False;
  Standard_Boolean isLastDefined  = Standard_False;

  TopoDS_Iterator ite(E, CumOri);
  while (ite.More())
  {
    const TopoShape& aV = ite.Value();
    if (aV.Orientation() == TopAbs_FORWARD)
    {
      Vfirst         = TopoDS::Vertex(aV);
      isFirstDefined = Standard_True;
    }
    else if (aV.Orientation() == TopAbs_REVERSED)
    {
      Vlast         = TopoDS::Vertex(aV);
      isLastDefined = Standard_True;
    }
    ite.Next();
  }

  if (!isFirstDefined)
    Vfirst.Nullify();

  if (!isLastDefined)
    Vlast.Nullify();
}

//=================================================================================================

void TopExp1::Vertices(const TopoWire& W, TopoVertex& Vfirst, TopoVertex& Vlast)
{
  Vfirst = Vlast = TopoVertex(); // nullify

  TopTools_MapOfShape vmap;
  TopoDS_Iterator     it(W);
  TopoVertex       V1, V2;

  while (it.More())
  {
    const TopoEdge& E = TopoDS::Edge(it.Value());
    if (E.Orientation() == TopAbs_REVERSED)
      TopExp1::Vertices(E, V2, V1);
    else
      TopExp1::Vertices(E, V1, V2);
    // add or remove in the vertex map
    V1.Orientation(TopAbs_FORWARD);
    V2.Orientation(TopAbs_REVERSED);
    if (!vmap.Add(V1))
      vmap.Remove(V1);
    if (!vmap.Add(V2))
      vmap.Remove(V2);

    it.Next();
  }
  if (vmap.IsEmpty())
  { // closed
    TopoShape aLocalShape = V2.Oriented(TopAbs_FORWARD);
    Vfirst                   = TopoDS::Vertex(aLocalShape);
    aLocalShape              = V2.Oriented(TopAbs_REVERSED);
    Vlast                    = TopoDS::Vertex(aLocalShape);
    //    Vfirst = TopoDS::Vertex(V2.Oriented(TopAbs_FORWARD));
    //    Vlast  = TopoDS::Vertex(V2.Oriented(TopAbs_REVERSED));
  }
  else if (vmap.Extent() == 2)
  { // open
    TopTools_MapIteratorOfMapOfShape ite(vmap);

    while (ite.More() && ite.Key1().Orientation() != TopAbs_FORWARD)
      ite.Next();
    if (ite.More())
      Vfirst = TopoDS::Vertex(ite.Key1());
    ite.Initialize(vmap);
    while (ite.More() && ite.Key1().Orientation() != TopAbs_REVERSED)
      ite.Next();
    if (ite.More())
      Vlast = TopoDS::Vertex(ite.Key1());
  }
}

//=================================================================================================

Standard_Boolean TopExp1::CommonVertex(const TopoEdge& E1,
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
