// Created on: 1994-03-03
// Created by: Joelle CHAUVET
// Copyright (c) 1994-1999 Matra Datavision
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

#ifndef _BRepFill_HeaderFile
#define _BRepFill_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Boolean.hxx>
#include <TColStd_Array1OfReal.hxx>
class TopoFace;
class TopoEdge;
class TopoShell;
class TopoWire;
class TopoShape;
class gp_Ax3;
class Point3d;
class Vector3d;

class BRepFill1
{
public:
  DEFINE_STANDARD_ALLOC

  //! Computes a ruled surface between two edges.
  Standard_EXPORT static TopoFace Face(const TopoEdge& Edge1, const TopoEdge& Edge2);

  //! Computes a ruled surface between two wires.
  //! The wires must have the same number of edges.
  Standard_EXPORT static TopoShell Shell(const TopoWire& Wire1, const TopoWire& Wire2);

  //! Computes  <AxeProf>  as Follow. <Location> is
  //! the Position of the nearest vertex V  of <Profile>
  //! to <Spine>.<XDirection> is confused with the tangent
  //! to <Spine> at the projected point of V on the Spine.
  //! <Direction> is normal to <Spine>.
  //! <Spine> is a plane wire or a plane face.
  Standard_EXPORT static void Axe(const TopoShape& Spine,
                                  const TopoWire&  Profile,
                                  gp_Ax3&             AxeProf,
                                  Standard_Boolean&   ProfOnSpine,
                                  const Standard_Real Tol);

  //! Compute ACR on a  wire
  Standard_EXPORT static void ComputeACR(const TopoWire& wire, TColStd_Array1OfReal& ACR);

  //! Insert ACR on a  wire
  Standard_EXPORT static TopoWire InsertACR(const TopoWire&          wire,
                                               const TColStd_Array1OfReal& ACRcuts,
                                               const Standard_Real         prec);

private:
  //! Computes origins and orientation on a closed wire
  Standard_EXPORT static void SearchOrigin(TopoWire&        W,
                                           const Point3d&       P,
                                           const Vector3d&       V,
                                           const Standard_Real Tol);

private:
  friend class BRepFill_PipeShell;
};

#endif // _BRepFill_HeaderFile
