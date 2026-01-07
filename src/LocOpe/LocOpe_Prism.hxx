// Created on: 1997-02-24
// Created by: Jacques GOUSSARD
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _LocOpe_Prism_HeaderFile
#define _LocOpe_Prism_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <gp_Vec.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
class GeomCurve3d;

//! Defines a prism (using Prism from BRepSweep)
//! with modifications provided for the Prism feature.
class LocOpe_Prism
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT LocOpe_Prism();

  Standard_EXPORT LocOpe_Prism(const TopoShape& Base, const Vector3d& V);

  Standard_EXPORT LocOpe_Prism(const TopoShape& Base, const Vector3d& V, const Vector3d& Vectra);

  Standard_EXPORT void Perform(const TopoShape& Base, const Vector3d& V);

  Standard_EXPORT void Perform(const TopoShape& Base, const Vector3d& V, const Vector3d& Vtra);

  Standard_EXPORT const TopoShape& FirstShape() const;

  Standard_EXPORT const TopoShape& LastShape() const;

  Standard_EXPORT const TopoShape& Shape() const;

  Standard_EXPORT const ShapeList& Shapes(const TopoShape& S) const;

  Standard_EXPORT void Curves(TColGeom_SequenceOfCurve& SCurves) const;

  Standard_EXPORT Handle(GeomCurve3d) BarycCurve() const;

protected:
private:
  Standard_EXPORT void IntPerf();

  TopoShape                       myBase;
  Vector3d                             myVec;
  Vector3d                             myTra;
  Standard_Boolean                   myIsTrans;
  Standard_Boolean                   myDone;
  TopoShape                       myRes;
  TopoShape                       myFirstShape;
  TopoShape                       myLastShape;
  TopTools_DataMapOfShapeListOfShape myMap;
};

#endif // _LocOpe_Prism_HeaderFile
