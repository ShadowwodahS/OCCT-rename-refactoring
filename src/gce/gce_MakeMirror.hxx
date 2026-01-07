// Created on: 1992-09-01
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

#ifndef _gce_MakeMirror_HeaderFile
#define _gce_MakeMirror_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <gp_Trsf.hxx>
class Point3d;
class Axis3d;
class gp_Lin;
class Dir3d;
class gp_Pln;
class Frame3d;

//! This class mplements elementary construction algorithms for a
//! symmetrical transformation in 3D space about a point,
//! axis or plane. The result is a Transform3d transformation.
//! A MakeMirror object provides a framework for:
//! -   defining the construction of the transformation,
//! -   implementing the construction algorithm, and
//! -   consulting the result.
class MirrorBuilder
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT MirrorBuilder(const Point3d& Point);

  Standard_EXPORT MirrorBuilder(const Axis3d& Axis);

  Standard_EXPORT MirrorBuilder(const gp_Lin& Line);

  //! Makes a symmetry transformation af axis defined by
  //! <Point> and <Direc>.
  Standard_EXPORT MirrorBuilder(const Point3d& Point, const Dir3d& Direc);

  //! Makes a symmetry transformation of plane <Plane>.
  Standard_EXPORT MirrorBuilder(const gp_Pln& Plane);

  //! Makes a symmetry transformation of plane <Plane>.
  Standard_EXPORT MirrorBuilder(const Frame3d& Plane);

  //! Returns the constructed transformation.
  Standard_EXPORT const Transform3d& Value() const;

  Standard_EXPORT const Transform3d& Operator() const;
  Standard_EXPORT                operator Transform3d() const;

protected:
private:
  Transform3d TheMirror;
};

#endif // _gce_MakeMirror_HeaderFile
