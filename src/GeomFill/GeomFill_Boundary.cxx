// Created on: 1995-11-03
// Created by: Laurent BOURESCHE
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

#include <GeomFill_Boundary.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Boundary2, RefObject)

//=================================================================================================

Boundary2::Boundary2(const Standard_Real Tol3d, const Standard_Real Tolang)
    : myT3d(Tol3d),
      myTang(Tolang)
{
}

//=================================================================================================

Standard_Boolean Boundary2::HasNormals() const
{
  return Standard_False;
}

//=================================================================================================

Vector3d Boundary2::Norm(const Standard_Real) const
{
  throw ExceptionBase("Boundary2::Norm : Undefined normals");
}

//=================================================================================================

void Boundary2::D1Norm(const Standard_Real, Vector3d&, Vector3d&) const
{
  throw ExceptionBase("Boundary2::Norm : Undefined normals");
}

//=================================================================================================

void Boundary2::Points(Point3d& PFirst, Point3d& PLast) const
{
  Standard_Real f, l;
  Bounds(f, l);
  PFirst = Value(f);
  PLast  = Value(l);
}

//=================================================================================================

Standard_Real Boundary2::Tol3d() const
{
  return myT3d;
}

//=================================================================================================

void Boundary2::Tol3d(const Standard_Real Tol)
{
  myT3d = Tol;
}

//=================================================================================================

Standard_Real Boundary2::Tolang() const
{
  return myTang;
}

//=================================================================================================

void Boundary2::Tolang(const Standard_Real Tol)
{
  myTang = Tol;
}
