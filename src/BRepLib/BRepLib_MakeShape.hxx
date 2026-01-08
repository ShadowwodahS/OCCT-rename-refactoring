// Created on: 1993-07-21
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

#ifndef _BRepLib_MakeShape_HeaderFile
#define _BRepLib_MakeShape_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <BRepLib_Command.hxx>
#include <BRepLib_ShapeModification.hxx>
class TopoFace;
class TopoEdge;

//! This    is  the  root     class for     all  shape
//! constructions.  It stores the result.
//!
//! It  provides deferred methods to trace the history
//! of sub-shapes.
class BRepLib_MakeShape : public Command1
{
public:
  DEFINE_STANDARD_ALLOC

  //! This is  called by  Shape().  It does  nothing but
  //! may be redefined.
  Standard_EXPORT void Build();

  Standard_EXPORT const TopoShape& Shape();
  Standard_EXPORT                     operator TopoShape();

  //! returns the status of the Face after
  //! the shape creation.
  Standard_EXPORT virtual BRepLib_ShapeModification FaceStatus(const TopoFace& F) const;

  //! Returns True if the Face generates new topology.
  Standard_EXPORT virtual Standard_Boolean HasDescendants(const TopoFace& F) const;

  //! returns the list of generated Faces.
  Standard_EXPORT virtual const ShapeList& DescendantFaces(const TopoFace& F);

  //! returns the number of surfaces
  //! after the shape creation.
  Standard_EXPORT virtual Standard_Integer NbSurfaces() const;

  //! Return the faces created for surface I.
  Standard_EXPORT virtual const ShapeList& NewFaces(const Standard_Integer I);

  //! returns a list of the created faces
  //! from the edge <E>.
  Standard_EXPORT virtual const ShapeList& FacesFromEdges(const TopoEdge& E);

protected:
  Standard_EXPORT BRepLib_MakeShape();

  TopoShape         myShape;
  ShapeList myGenFaces;
  ShapeList myNewFaces;
  ShapeList myEdgFaces;

private:
};

#endif // _BRepLib_MakeShape_HeaderFile
