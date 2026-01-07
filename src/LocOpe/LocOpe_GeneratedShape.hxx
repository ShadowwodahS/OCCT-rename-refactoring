// Created on: 1996-01-08
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

#ifndef _LocOpe_GeneratedShape_HeaderFile
#define _LocOpe_GeneratedShape_HeaderFile

#include <Standard.hxx>

#include <TopTools_ListOfShape.hxx>
#include <Standard_Transient.hxx>
class TopoEdge;
class TopoVertex;
class TopoFace;

class LocOpe_GeneratedShape;
DEFINE_STANDARD_HANDLE(LocOpe_GeneratedShape, RefObject)

class LocOpe_GeneratedShape : public RefObject
{

public:
  Standard_EXPORT virtual const ShapeList& GeneratingEdges() = 0;

  //! Returns the  edge  created by  the  vertex <V>. If
  //! none, must return a null shape.
  Standard_EXPORT virtual TopoEdge Generated(const TopoVertex& V) = 0;

  //! Returns the face created by the edge <E>. If none,
  //! must return a null shape.
  Standard_EXPORT virtual TopoFace Generated(const TopoEdge& E) = 0;

  //! Returns  the  list of correctly oriented generated
  //! faces.
  Standard_EXPORT virtual const ShapeList& OrientedFaces() = 0;

  DEFINE_STANDARD_RTTIEXT(LocOpe_GeneratedShape, RefObject)

protected:
  ShapeList myGEdges;
  ShapeList myList;

private:
};

#endif // _LocOpe_GeneratedShape_HeaderFile
