// Created on: 2015-09-21
// Copyright (c) 2015 OPEN CASCADE SAS
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

#ifndef _GeomEvaluator_Surface_HeaderFile
#define _GeomEvaluator_Surface_HeaderFile

#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class Point3d;
class Vector3d;

//! Interface for calculation of values and derivatives for different kinds of surfaces.
//! Works both with adaptors and surfaces.
class Surface1 : public RefObject
{
public:
  Surface1() {}

  //! Value of surface
  virtual void D0(const Standard_Real theU, const Standard_Real theV, Point3d& theValue) const = 0;
  //! Value and first derivatives of surface
  virtual void D1(const Standard_Real theU,
                  const Standard_Real theV,
                  Point3d&             theValue,
                  Vector3d&             theD1U,
                  Vector3d&             theD1V) const = 0;
  //! Value, first and second derivatives of surface
  virtual void D2(const Standard_Real theU,
                  const Standard_Real theV,
                  Point3d&             theValue,
                  Vector3d&             theD1U,
                  Vector3d&             theD1V,
                  Vector3d&             theD2U,
                  Vector3d&             theD2V,
                  Vector3d&             theD2UV) const = 0;
  //! Value, first, second and third derivatives of surface
  virtual void D3(const Standard_Real theU,
                  const Standard_Real theV,
                  Point3d&             theValue,
                  Vector3d&             theD1U,
                  Vector3d&             theD1V,
                  Vector3d&             theD2U,
                  Vector3d&             theD2V,
                  Vector3d&             theD2UV,
                  Vector3d&             theD3U,
                  Vector3d&             theD3V,
                  Vector3d&             theD3UUV,
                  Vector3d&             theD3UVV) const = 0;
  //! Calculates N-th derivatives of surface, where N = theDerU + theDerV.
  //!
  //! Raises if N < 1 or theDerU < 0 or theDerV < 0
  virtual Vector3d DN(const Standard_Real    theU,
                    const Standard_Real    theV,
                    const Standard_Integer theDerU,
                    const Standard_Integer theDerV) const = 0;

  virtual Handle(Surface1) ShallowCopy() const = 0;

  DEFINE_STANDARD_RTTI_INLINE(Surface1, RefObject)
};

DEFINE_STANDARD_HANDLE(Surface1, RefObject)

#endif // _GeomEvaluator_Surface_HeaderFile
