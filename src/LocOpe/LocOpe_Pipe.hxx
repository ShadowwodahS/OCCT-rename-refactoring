// Created on: 1996-09-04
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

#ifndef _LocOpe_Pipe_HeaderFile
#define _LocOpe_Pipe_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <BRepFill_Pipe.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
#include <TColgp_SequenceOfPnt.hxx>
class TopoWire;
class GeomCurve3d;

//! Defines a  pipe  (near from   Pipe from BRepFill1),
//! with modifications provided for the Pipe feature.
class LocOpe_Pipe
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT LocOpe_Pipe(const TopoWire& Spine, const TopoShape& Profile);

  const TopoShape& Spine() const;

  const TopoShape& Profile() const;

  const TopoShape& FirstShape() const;

  const TopoShape& LastShape() const;

  Standard_EXPORT const TopoShape& Shape() const;

  Standard_EXPORT const ShapeList& Shapes(const TopoShape& S);

  Standard_EXPORT const TColGeom_SequenceOfCurve& Curves(const TColgp_SequenceOfPnt& Spt);

  Standard_EXPORT Handle(GeomCurve3d) BarycCurve();

protected:
private:
  BRepFill_Pipe                      myPipe;
  TopTools_DataMapOfShapeListOfShape myMap;
  TopoShape                       myRes;
  ShapeList               myGShap;
  TColGeom_SequenceOfCurve           myCrvs;
  TopoShape                       myFirstShape;
  TopoShape                       myLastShape;
};

#include <LocOpe_Pipe.lxx>

#endif // _LocOpe_Pipe_HeaderFile
