// Created on: 1998-06-03
// Created by: data exchange team
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

#ifndef _ShapeAnalysis_HeaderFile
#define _ShapeAnalysis_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Real.hxx>
#include <Standard_Boolean.hxx>
class TopoWire;
class TopoFace;
class ShapeExtend_WireData;
class TopoShape;
class TopoVertex;

//! This package is intended to analyze geometrical objects
//! and topological shapes. Analysis domain includes both
//! exploring geometrical and topological properties of
//! shapes and checking their conformance to Open CASCADE requirements.
//! The directions of analysis provided by tools of this package are:
//! computing quantities of subshapes,
//! computing parameters of points on curve and surface,
//! computing surface singularities,
//! checking edge and wire consistency,
//! checking edges order in the wire,
//! checking face bounds orientation,
//! checking small faces,
//! analyzing shape tolerances,
//! analyzing of free bounds of the shape.
class ShapeAnalysis1
{
public:
  DEFINE_STANDARD_ALLOC

  //! Returns positively oriented wire in the face.
  //! If there is no such wire - returns the last wire of the face.
  Standard_EXPORT static TopoWire OuterWire(const TopoFace& theFace);

  //! Returns a total area of 2d wire
  Standard_EXPORT static Standard_Real TotCross2D(const Handle(ShapeExtend_WireData)& sewd,
                                                  const TopoFace&                  aFace);

  //! Returns a total area of 3d wire
  Standard_EXPORT static Standard_Real ContourArea(const TopoWire& theWire);

  //! Returns True if <F> has outer bound.
  Standard_EXPORT static Standard_Boolean IsOuterBound(const TopoFace& face);

  //! Returns a shift required to move point
  //! <Val> to the range [ToVal-Period/2,ToVal+Period/2].
  //! This shift will be the divisible by Period.
  //! Intended for adjusting parameters on periodic surfaces.
  Standard_EXPORT static Standard_Real AdjustByPeriod(const Standard_Real Val,
                                                      const Standard_Real ToVal,
                                                      const Standard_Real Period);

  //! Returns a shift required to move point
  //! <Val> to the range [ValMin,ValMax].
  //! This shift will be the divisible by Period
  //! with Period = ValMax - ValMin.
  //! Intended for adjusting parameters on periodic surfaces.
  Standard_EXPORT static Standard_Real AdjustToPeriod(const Standard_Real Val,
                                                      const Standard_Real ValMin,
                                                      const Standard_Real ValMax);

  //! Finds the start and end vertices of the shape
  //! Shape can be of the following type:
  //! vertex: V1 and V2 are the same and equal to <shape>,
  //! edge  : V1 is start and V2 is end vertex (see Edge1
  //! methods FirstVertex and LastVertex),
  //! wire  : V1 is start vertex of the first edge, V2 is end vertex
  //! of the last edge (also see Edge1).
  //! If wire contains no edges V1 and V2 are nullified
  //! If none of the above V1 and V2 are nullified
  Standard_EXPORT static void FindBounds(const TopoShape& shape,
                                         TopoVertex&      V1,
                                         TopoVertex&      V2);

  //! Computes exact UV bounds of all wires on the face
  Standard_EXPORT static void GetFaceUVBounds(const TopoFace& F,
                                              Standard_Real&     Umin,
                                              Standard_Real&     Umax,
                                              Standard_Real&     Vmin,
                                              Standard_Real&     Vmax);
};

#endif // _ShapeAnalysis_HeaderFile
