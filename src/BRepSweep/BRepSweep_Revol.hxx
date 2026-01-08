// Created on: 1993-06-22
// Created by: Laurent BOURESCHE
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

#ifndef _BRepSweep_Revol_HeaderFile
#define _BRepSweep_Revol_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <BRepSweep_Rotation.hxx>
#include <Standard_Boolean.hxx>
class TopoShape;
class Axis3d;
class SweepNumShape;
class TopLoc_Location;

//! Provides natural constructors to build BRepSweep
//! rotated swept Primitives.
class BRepSweep_Revol
{
public:
  DEFINE_STANDARD_ALLOC

  //! Builds the Revol of meridian S axis A  and angle D. If
  //! C is true S is copied.
  Standard_EXPORT BRepSweep_Revol(const TopoShape&    S,
                                  const Axis3d&          A,
                                  const Standard_Real    D,
                                  const Standard_Boolean C = Standard_False);

  //! Builds the Revol of meridian S  axis A and angle 2*Pi.
  //! If C is true S is copied.
  Standard_EXPORT BRepSweep_Revol(const TopoShape&    S,
                                  const Axis3d&          A,
                                  const Standard_Boolean C = Standard_False);

  //! Returns the TopoDS Shape attached to the Revol.
  Standard_EXPORT TopoShape Shape();

  //! Returns    the  TopoDS  Shape   generated  with  aGenS
  //! (subShape  of the generating shape).
  Standard_EXPORT TopoShape Shape(const TopoShape& aGenS);

  //! Returns the first shape of the revol  (coinciding with
  //! the generating shape).
  Standard_EXPORT TopoShape FirstShape();

  //! Returns the first shape of the revol  (coinciding with
  //! the generating shape).
  Standard_EXPORT TopoShape FirstShape(const TopoShape& aGenS);

  //! Returns the TopoDS Shape of the top of the prism.
  Standard_EXPORT TopoShape LastShape();

  //! Returns the  TopoDS  Shape of the top  of  the  prism.
  //! generated  with  aGenS  (subShape  of  the  generating
  //! shape).
  Standard_EXPORT TopoShape LastShape(const TopoShape& aGenS);

  //! returns the axis
  Standard_EXPORT Axis3d Axe() const;

  //! returns the angle.
  Standard_EXPORT Standard_Real Angle() const;

  //! Returns true if the aGenS is used in resulting Shape
  Standard_EXPORT Standard_Boolean IsUsed(const TopoShape& aGenS) const;

private:
  //! builds the NumShape.
  Standard_EXPORT SweepNumShape NumShape(const Standard_Real D) const;

  //! Builds the Location
  Standard_EXPORT TopLoc_Location Location(const Axis3d& Ax, const Standard_Real D) const;

  //! Builds the axis
  Standard_EXPORT Axis3d Axe(const Axis3d& Ax, const Standard_Real D) const;

  //! computes the angle.
  Standard_EXPORT Standard_Real Angle(const Standard_Real D) const;

  BRepSweep_Rotation myRotation;
};

#endif // _BRepSweep_Revol_HeaderFile
