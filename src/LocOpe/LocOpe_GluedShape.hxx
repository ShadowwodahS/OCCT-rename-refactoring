// Created on: 1996-01-30
// Created by: Jacques GOUSSARD
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _LocOpe_GluedShape_HeaderFile
#define _LocOpe_GluedShape_HeaderFile

#include <Standard.hxx>

#include <TopTools_MapOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <LocOpe_GeneratedShape.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoFace;
class TopoEdge;
class TopoVertex;

class LocOpe_GluedShape;
DEFINE_STANDARD_HANDLE(LocOpe_GluedShape, LocOpe_GeneratedShape)

class LocOpe_GluedShape : public LocOpe_GeneratedShape
{

public:
  Standard_EXPORT LocOpe_GluedShape();

  Standard_EXPORT LocOpe_GluedShape(const TopoShape& S);

  Standard_EXPORT void Init(const TopoShape& S);

  Standard_EXPORT void GlueOnFace(const TopoFace& F);

  Standard_EXPORT const ShapeList& GeneratingEdges() Standard_OVERRIDE;

  //! Returns the  edge  created by  the  vertex <V>. If
  //! none, must return a null shape.
  Standard_EXPORT TopoEdge Generated(const TopoVertex& V) Standard_OVERRIDE;

  //! Returns the face created by the edge <E>. If none,
  //! must return a null shape.
  Standard_EXPORT TopoFace Generated(const TopoEdge& E) Standard_OVERRIDE;

  //! Returns  the  list of correctly oriented generated
  //! faces.
  Standard_EXPORT const ShapeList& OrientedFaces() Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(LocOpe_GluedShape, LocOpe_GeneratedShape)

protected:
private:
  Standard_EXPORT void MapEdgeAndVertices();

  TopoShape                 myShape;
  TopTools_MapOfShape          myMap;
  TopTools_DataMapOfShapeShape myGShape;
};

#endif // _LocOpe_GluedShape_HeaderFile
