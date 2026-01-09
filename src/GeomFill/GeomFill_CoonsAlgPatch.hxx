// Created on: 1995-12-04
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

#ifndef _GeomFill_CoonsAlgPatch_HeaderFile
#define _GeomFill_CoonsAlgPatch_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <gp_Pnt.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Integer.hxx>
class Boundary2;
class Function2;
class Vector3d;

class GeomFill_CoonsAlgPatch;
DEFINE_STANDARD_HANDLE(GeomFill_CoonsAlgPatch, RefObject)

//! Provides  evaluation   methods on an   algorithmic
//! patch (based on 4 Curves) defined by  its   boundaries and  blending
//! functions.
class GeomFill_CoonsAlgPatch : public RefObject
{

public:
  //! Constructs the  algorithmic   patch. By   Default  the
  //! constructed blending functions are linear.
  //! Warning: No control is done on the bounds.
  //! B1/B3 and B2/B4 must be same range and well oriented.
  Standard_EXPORT GeomFill_CoonsAlgPatch(const Handle(Boundary2)& B1,
                                         const Handle(Boundary2)& B2,
                                         const Handle(Boundary2)& B3,
                                         const Handle(Boundary2)& B4);

  //! Give the blending functions.
  Standard_EXPORT void Func(Handle(Function2)& f1, Handle(Function2)& f2) const;

  //! Set the blending functions.
  Standard_EXPORT void SetFunc(const Handle(Function2)& f1, const Handle(Function2)& f2);

  //! Computes  the  value   on the  algorithmic    patch at
  //! parameters U and V.
  Standard_EXPORT Point3d Value(const Standard_Real U, const Standard_Real V) const;

  //! Computes   the  d/dU   partial   derivative  on    the
  //! algorithmic patch at parameters U and V.
  Standard_EXPORT Vector3d D1U(const Standard_Real U, const Standard_Real V) const;

  //! Computes    the  d/dV    partial    derivative on  the
  //! algorithmic patch at parameters U and V.
  Standard_EXPORT Vector3d D1V(const Standard_Real U, const Standard_Real V) const;

  //! Computes the   d2/dUdV  partial  derivative   on   the
  //! algorithmic  patch made with linear blending functions
  //! at parameter U and V.
  Standard_EXPORT Vector3d DUV(const Standard_Real U, const Standard_Real V) const;

  Standard_EXPORT const Point3d& Corner(const Standard_Integer I) const;

  Standard_EXPORT const Handle(Boundary2)& Bound(const Standard_Integer I) const;

  Standard_EXPORT const Handle(Function2)& Func(const Standard_Integer I) const;

  DEFINE_STANDARD_RTTIEXT(GeomFill_CoonsAlgPatch, RefObject)

protected:
private:
  Handle(Boundary2) bound[4];
  Point3d                    c[4];
  Handle(Function2)      a[2];
};

#endif // _GeomFill_CoonsAlgPatch_HeaderFile
