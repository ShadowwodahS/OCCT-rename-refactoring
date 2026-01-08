// Created on: 1998-11-13
// Created by: Jean-Michel BOULCOURT
// Copyright (c) 1998-1999 Matra Datavision
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

#ifndef _BRepProj_Projection_HeaderFile
#define _BRepProj_Projection_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <Standard_Integer.hxx>
#include <TopoDS_Wire.hxx>
class Dir3d;
class Point3d;

//! The  Projection   class provides  conical  and
//! cylindrical projections of  Edge  or  Wire  on
//! a Shape from TopoDS. The result will be a Edge
//! or  Wire  from  TopoDS.
class BRepProj_Projection
{
public:
  DEFINE_STANDARD_ALLOC

  //! Makes a Cylindrical projection of Wire om Shape
  Standard_EXPORT BRepProj_Projection(const TopoShape& Wire,
                                      const TopoShape& Shape,
                                      const Dir3d&       D);

  //! Makes a Conical projection of Wire om Shape
  Standard_EXPORT BRepProj_Projection(const TopoShape& Wire,
                                      const TopoShape& Shape,
                                      const Point3d&       P);

  //! returns False if the section failed
  Standard_Boolean IsDone() const;

  //! Resets the iterator by resulting wires.
  void Init();

  //! Returns True if there is a current result wire
  Standard_Boolean More() const;

  //! Move to the next result wire.
  void Next();

  //! Returns the current result wire.
  TopoWire Current() const;

  //! Returns the complete result as compound of wires.
  TopoCompound Shape() const;

protected:
private:
  //! Performs section of theShape by theTool
  //! and stores result in the fields.
  Standard_EXPORT void BuildSection(const TopoShape& Shape, const TopoShape& Tool);

  Standard_Boolean                  myIsDone;
  TopoShape                      myLsh;
  TopoCompound                   myShape;
  Handle(HSequenceOfShape) mySection;
  Standard_Integer                  myItr;
};

#include <BRepProj_Projection.lxx>

#endif // _BRepProj_Projection_HeaderFile
