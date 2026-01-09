// Created on: 1995-10-19
// Created by: Andre LIEUTIER
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _Plate_D1_HeaderFile
#define _Plate_D1_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <gp_XYZ.hxx>

//! define an order 1 derivatives of a 3d valued
//! function of a 2d variable
class D1
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT D1(const Coords3d& du, const Coords3d& dv);

  Standard_EXPORT D1(const D1& ref);

  const Coords3d& DU() const;

  const Coords3d& DV() const;

  friend class GtoCConstraint;
  friend class FreeGtoCConstraint;

protected:
private:
  Coords3d Du;
  Coords3d Dv;
};

#include <Plate_D1.lxx>

#endif // _Plate_D1_HeaderFile
