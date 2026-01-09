// Created on: 1992-12-02
// Created by: Isabelle GRIGNON
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

#ifndef _GProp_SelGProps_HeaderFile
#define _GProp_SelGProps_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <GProp_GProps.hxx>
class Cylinder1;
class Point3d;
class Cone1;
class Sphere3;
class gp_Torus;

//! Computes the global properties of a bounded
//! elementary surface in 3d (surface of the gp1 package)
class SelectionGeometricProperties : public GeometricProperties
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT SelectionGeometricProperties();

  Standard_EXPORT SelectionGeometricProperties(const Cylinder1&  S,
                                  const Standard_Real Alpha1,
                                  const Standard_Real Alpha2,
                                  const Standard_Real Z1,
                                  const Standard_Real Z2,
                                  const Point3d&       SLocation);

  Standard_EXPORT SelectionGeometricProperties(const Cone1&      S,
                                  const Standard_Real Alpha1,
                                  const Standard_Real Alpha2,
                                  const Standard_Real Z1,
                                  const Standard_Real Z2,
                                  const Point3d&       SLocation);

  Standard_EXPORT SelectionGeometricProperties(const Sphere3&    S,
                                  const Standard_Real Teta1,
                                  const Standard_Real Teta2,
                                  const Standard_Real Alpha1,
                                  const Standard_Real Alpha2,
                                  const Point3d&       SLocation);

  Standard_EXPORT SelectionGeometricProperties(const gp_Torus&     S,
                                  const Standard_Real Teta1,
                                  const Standard_Real Teta2,
                                  const Standard_Real Alpha1,
                                  const Standard_Real Alpha2,
                                  const Point3d&       SLocation);

  Standard_EXPORT void SetLocation(const Point3d& SLocation);

  Standard_EXPORT void Perform(const Cylinder1&  S,
                               const Standard_Real Alpha1,
                               const Standard_Real Alpha2,
                               const Standard_Real Z1,
                               const Standard_Real Z2);

  Standard_EXPORT void Perform(const Cone1&      S,
                               const Standard_Real Alpha1,
                               const Standard_Real Alpha2,
                               const Standard_Real Z1,
                               const Standard_Real Z2);

  Standard_EXPORT void Perform(const Sphere3&    S,
                               const Standard_Real Teta1,
                               const Standard_Real Teta2,
                               const Standard_Real Alpha1,
                               const Standard_Real Alpha2);

  Standard_EXPORT void Perform(const gp_Torus&     S,
                               const Standard_Real Teta1,
                               const Standard_Real Teta2,
                               const Standard_Real Alpha1,
                               const Standard_Real Alpha2);

protected:
private:
};

#endif // _GProp_SelGProps_HeaderFile
