// Created on: 1998-10-29
// Created by: Philippe MANGIN
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

#ifndef _GeomFill_PlanFunc_HeaderFile
#define _GeomFill_PlanFunc_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <gp_XYZ.hxx>
#include <gp_Pnt.hxx>
#include <math_FunctionWithDerivative.hxx>

class Vector3d;

class GeomFill_PlanFunc : public math_FunctionWithDerivative
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT GeomFill_PlanFunc(const Point3d&                  P,
                                    const Vector3d&                  V,
                                    const Handle(Adaptor3d_Curve)& C);

  //! computes the value <F>of the function for the variable <X>.
  //! Returns True if the calculation were successfully done,
  //! False otherwise.
  Standard_EXPORT virtual Standard_Boolean Value(const Standard_Real X,
                                                 Standard_Real&      F) Standard_OVERRIDE;

  //! computes the derivative <D> of the function
  //! for the variable <X>.
  //! Returns True if the calculation were successfully done,
  //! False otherwise.
  Standard_EXPORT virtual Standard_Boolean Derivative(const Standard_Real X,
                                                      Standard_Real&      D) Standard_OVERRIDE;

  //! computes the value <F> and the derivative <D> of the
  //! function for the variable <X>.
  //! Returns True if the calculation were successfully done,
  //! False otherwise.
  Standard_EXPORT virtual Standard_Boolean Values(const Standard_Real X,
                                                  Standard_Real&      F,
                                                  Standard_Real&      D) Standard_OVERRIDE;

  Standard_EXPORT void D2(const Standard_Real X,
                          Standard_Real&      F,
                          Standard_Real&      D1,
                          Standard_Real&      D2);

  Standard_EXPORT void DEDT(const Standard_Real X,
                            const Vector3d&       DP,
                            const Vector3d&       DV,
                            Standard_Real&      DF);

  Standard_EXPORT void D2E(const Standard_Real X,
                           const Vector3d&       DP,
                           const Vector3d&       D2P,
                           const Vector3d&       DV,
                           const Vector3d&       D2V,
                           Standard_Real&      DFDT,
                           Standard_Real&      D2FDT2,
                           Standard_Real&      D2FDTDX);

protected:
private:
  Coords3d                  myPnt;
  Coords3d                  myVec;
  Coords3d                  V;
  Point3d                  G;
  Handle(Adaptor3d_Curve) myCurve;
};

#endif // _GeomFill_PlanFunc_HeaderFile
