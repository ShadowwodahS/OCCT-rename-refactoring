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

#ifndef _GeomEvaluator_OffsetCurve_HeaderFile
#define _GeomEvaluator_OffsetCurve_HeaderFile

#include <GeomAdaptor_Curve.hxx>
#include <GeomEvaluator_Curve.hxx>
#include <gp_Dir.hxx>

//! Allows to calculate values and derivatives for offset curves in 3D
class GeomEvaluator_OffsetCurve : public Curve6
{
public:
  //! Initialize evaluator by curve
  Standard_EXPORT GeomEvaluator_OffsetCurve(const Handle(GeomCurve3d)& theBase,
                                            const Standard_Real       theOffset,
                                            const Dir3d&             theDirection);
  //! Initialize evaluator by curve adaptor
  Standard_EXPORT GeomEvaluator_OffsetCurve(const Handle(GeomAdaptor_Curve)& theBase,
                                            const Standard_Real              theOffset,
                                            const Dir3d&                    theDirection);

  //! Change the offset value
  void SetOffsetValue(Standard_Real theOffset) { myOffset = theOffset; }

  void SetOffsetDirection(const Dir3d& theDirection) { myOffsetDir = theDirection; }

  //! Value of curve
  Standard_EXPORT void D0(const Standard_Real theU, Point3d& theValue) const Standard_OVERRIDE;
  //! Value and first derivatives of curve
  Standard_EXPORT void D1(const Standard_Real theU,
                          Point3d&             theValue,
                          Vector3d&             theD1) const Standard_OVERRIDE;
  //! Value, first and second derivatives of curve
  Standard_EXPORT void D2(const Standard_Real theU,
                          Point3d&             theValue,
                          Vector3d&             theD1,
                          Vector3d&             theD2) const Standard_OVERRIDE;
  //! Value, first, second and third derivatives of curve
  Standard_EXPORT void D3(const Standard_Real theU,
                          Point3d&             theValue,
                          Vector3d&             theD1,
                          Vector3d&             theD2,
                          Vector3d&             theD3) const Standard_OVERRIDE;
  //! Calculates N-th derivatives of curve, where N = theDeriv. Raises if N < 1
  Standard_EXPORT Vector3d DN(const Standard_Real    theU,
                            const Standard_Integer theDeriv) const Standard_OVERRIDE;

  Standard_EXPORT virtual Handle(Curve6) ShallowCopy() const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(GeomEvaluator_OffsetCurve, Curve6)

private:
  //! Recalculate D1 values of base curve into D0 value of offset curve
  void CalculateD0(Point3d& theValue, const Vector3d& theD1) const;
  //! Recalculate D2 values of base curve into D1 values of offset curve
  void CalculateD1(Point3d& theValue, Vector3d& theD1, const Vector3d& theD2) const;
  //! Recalculate D3 values of base curve into D2 values of offset curve
  void CalculateD2(Point3d&                theValue,
                   Vector3d&                theD1,
                   Vector3d&                theD2,
                   const Vector3d&          theD3,
                   const Standard_Boolean theIsDirChange) const;
  //! Recalculate D3 values of base curve into D3 values of offset curve
  void CalculateD3(Point3d&                theValue,
                   Vector3d&                theD1,
                   Vector3d&                theD2,
                   Vector3d&                theD3,
                   const Vector3d&          theD4,
                   const Standard_Boolean theIsDirChange) const;

  //! Calculate value of base curve/adaptor
  void BaseD0(const Standard_Real theU, Point3d& theValue) const;
  //! Calculate value and first derivatives of base curve/adaptor
  void BaseD1(const Standard_Real theU, Point3d& theValue, Vector3d& theD1) const;
  //! Calculate value, first and second derivatives of base curve/adaptor
  void BaseD2(const Standard_Real theU, Point3d& theValue, Vector3d& theD1, Vector3d& theD2) const;
  //! Calculate value, first, second and third derivatives of base curve/adaptor
  void BaseD3(const Standard_Real theU,
              Point3d&             theValue,
              Vector3d&             theD1,
              Vector3d&             theD2,
              Vector3d&             theD3) const;
  //! Calculate value and derivatives till 4th of base curve/adaptor
  void BaseD4(const Standard_Real theU,
              Point3d&             theValue,
              Vector3d&             theD1,
              Vector3d&             theD2,
              Vector3d&             theD3,
              Vector3d&             theD4) const;
  //! Calculate N-th derivative of base curve/adaptor
  Vector3d BaseDN(const Standard_Real theU, const Standard_Integer theDeriv) const;

  // Recalculate derivatives in the singular point
  // Returns true if the direction of derivatives is changed
  Standard_Boolean AdjustDerivative(const Standard_Integer theMaxDerivative,
                                    const Standard_Real    theU,
                                    Vector3d&                theD1,
                                    Vector3d&                theD2,
                                    Vector3d&                theD3,
                                    Vector3d&                theD4) const;

private:
  Handle(GeomCurve3d)        myBaseCurve;
  Handle(GeomAdaptor_Curve) myBaseAdaptor;

  Standard_Real myOffset;    ///< offset value
  Dir3d        myOffsetDir; ///< offset direction
};

DEFINE_STANDARD_HANDLE(GeomEvaluator_OffsetCurve, Curve6)

#endif // _GeomEvaluator_OffsetCurve_HeaderFile
