// Created on: 1995-07-11
// Created by: Jacques GOUSSARD
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

#ifndef _LocOpe_SplitShape_HeaderFile
#define _LocOpe_SplitShape_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoEdge;
class TopoVertex;
class TopoWire;
class TopoFace;

//! Provides a tool to cut  :
//! - edges with a vertices,
//! - faces with wires,
//! and  rebuilds  the shape containing the edges and
//! the faces.
class LocOpe_SplitShape
{
public:
  DEFINE_STANDARD_ALLOC

  //! Empty constructor.
  LocOpe_SplitShape();

  //! Creates the process  with the shape <S>.
  LocOpe_SplitShape(const TopoShape& S);

  //! Initializes the process on the shape <S>.
  Standard_EXPORT void Init(const TopoShape& S);

  //! Tests if it is possible to split the edge <E>.
  Standard_EXPORT Standard_Boolean CanSplit(const TopoEdge& E) const;

  //! Adds the vertex <V> on the edge <E>, at parameter <P>.
  Standard_EXPORT void Add(const TopoVertex& V, const Standard_Real P, const TopoEdge& E);

  //! Adds the wire <W> on the face <F>.
  Standard_EXPORT Standard_Boolean Add(const TopoWire& W, const TopoFace& F);

  //! Adds the list of wires <Lwires> on the face <F>.
  Standard_EXPORT Standard_Boolean Add(const ShapeList& Lwires, const TopoFace& F);

  //! Returns the "original" shape.
  const TopoShape& Shape() const;

  //! Returns the list of descendant shapes of <S>.
  Standard_EXPORT const ShapeList& DescendantShapes(const TopoShape& S);

  //! Returns the "left" part defined by the wire <W> on
  //! the face <F>.   The  returned list of shape  is in
  //! fact  a list of faces. The  face <F> is considered
  //! with its topological  orientation  in the original
  //! shape.  <W> is considered with its orientation.
  Standard_EXPORT const ShapeList& LeftOf(const TopoWire& W, const TopoFace& F);

protected:
private:
  Standard_EXPORT Standard_Boolean AddOpenWire(const TopoWire& W, const TopoFace& F);

  Standard_EXPORT Standard_Boolean AddClosedWire(const TopoWire& W, const TopoFace& F);

  Standard_EXPORT void Put(const TopoShape& S);

  Standard_EXPORT Standard_Boolean Rebuild(const TopoShape& S);

  Standard_Boolean                   myDone;
  TopoShape                       myShape;
  TopTools_DataMapOfShapeListOfShape myMap;
  TopTools_MapOfShape                myDblE;
  ShapeList               myLeft;
};

#include <LocOpe_SplitShape.lxx>

#endif // _LocOpe_SplitShape_HeaderFile
