// Created on: 1996-01-09
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

#ifndef _LocOpe_Generator_HeaderFile
#define _LocOpe_Generator_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
class GeneratedShape;
class TopoFace;

class LocOpe_Generator
{
public:
  DEFINE_STANDARD_ALLOC

  //! Empty constructor.
  LocOpe_Generator();

  //! Creates the algorithm on the shape <S>.
  LocOpe_Generator(const TopoShape& S);

  //! Initializes the algorithm on the shape <S>.
  void Init(const TopoShape& S);

  Standard_EXPORT void Perform(const Handle(GeneratedShape)& G);

  Standard_Boolean IsDone() const;

  //! Returns the new shape
  const TopoShape& ResultingShape() const;

  //! Returns the initial shape
  const TopoShape& Shape() const;

  //! Returns  the  descendant  face  of <F>.    <F> may
  //! belong to the original shape or to the "generated"
  //! shape.  The returned    face may be   a null shape
  //! (when <F> disappears).
  Standard_EXPORT const ShapeList& DescendantFace(const TopoFace& F);

protected:
private:
  TopoShape                       myShape;
  Handle(GeneratedShape)      myGen;
  Standard_Boolean                   myDone;
  TopoShape                       myRes;
  TopTools_DataMapOfShapeListOfShape myModShapes;
};

#include <LocOpe_Generator.lxx>

#endif // _LocOpe_Generator_HeaderFile
