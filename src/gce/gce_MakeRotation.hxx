// Created on: 1992-08-26
// Created by: Remi GILET
// Copyright (c) 1992-1999 Matra Datavision
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

#ifndef _gce_MakeRotation_HeaderFile
#define _gce_MakeRotation_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <gp_Trsf.hxx>
class gp_Lin;
class Axis3d;
class Point3d;
class Dir3d;

//! This class implements elementary construction algorithms for a
//! rotation in 3D space. The result is a Transform3d transformation.
//! A MakeRotation object provides a framework for:
//! -   defining the construction of the transformation,
//! -   implementing the construction algorithm, and
//! -   consulting the result.
class RotationBuilder
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructs a rotation through angle Angle about the axis defined by the line Line.
  Standard_EXPORT RotationBuilder(const gp_Lin& Line, const Standard_Real Angle);

  //! Constructs a rotation through angle Angle about the axis defined by the axis Axis.
  Standard_EXPORT RotationBuilder(const Axis3d& Axis, const Standard_Real Angle);

  //! Constructs a rotation through angle Angle about the axis defined by:
  //! the point Point and the unit vector Direc.
  Standard_EXPORT RotationBuilder(const Point3d&       Point,
                                   const Dir3d&       Direc,
                                   const Standard_Real Angle);

  //! Returns the constructed transformation.
  Standard_EXPORT const Transform3d& Value() const;

  Standard_EXPORT const Transform3d& Operator() const;
  Standard_EXPORT                operator Transform3d() const;

protected:
private:
  Transform3d TheRotation;
};

#endif // _gce_MakeRotation_HeaderFile
