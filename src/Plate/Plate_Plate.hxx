// Created on: 1995-10-18
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

#ifndef _Plate_Plate_HeaderFile
#define _Plate_Plate_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Integer.hxx>
#include <Plate_SequenceOfPinpointConstraint.hxx>
#include <Plate_SequenceOfLinearXYZConstraint.hxx>
#include <Plate_SequenceOfLinearScalarConstraint.hxx>
#include <TColgp_HArray2OfXYZ.hxx>
#include <TColgp_SequenceOfXY.hxx>
#include <Message_ProgressScope.hxx>

class PinpointConstraint;
class Plate_LinearXYZConstraint;
class Plate_LinearScalarConstraint;
class Plate_GlobalTranslationConstraint;
class Plate_LineConstraint;
class Plate_PlaneConstraint;
class Plate_SampledCurveConstraint;
class GtoCConstraint;
class FreeGtoCConstraint;
class Coords3d;
class Coords2d;
class math_Matrix;

//! This class implement a variational spline algorithm able
//! to define a two variable function satisfying some constraints
//! and minimizing an energy like criterion.
class PlateSurface
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT PlateSurface();

  Standard_EXPORT PlateSurface(const PlateSurface& Ref);

  Standard_EXPORT PlateSurface& Copy(const PlateSurface& Ref);

  PlateSurface& operator=(const PlateSurface& Ref) { return Copy(Ref); }

  Standard_EXPORT void Load(const PinpointConstraint& PConst);

  Standard_EXPORT void Load(const Plate_LinearXYZConstraint& LXYZConst);

  Standard_EXPORT void Load(const Plate_LinearScalarConstraint& LScalarConst);

  Standard_EXPORT void Load(const Plate_GlobalTranslationConstraint& GTConst);

  Standard_EXPORT void Load(const Plate_LineConstraint& LConst);

  Standard_EXPORT void Load(const Plate_PlaneConstraint& PConst);

  Standard_EXPORT void Load(const Plate_SampledCurveConstraint& SCConst);

  Standard_EXPORT void Load(const GtoCConstraint& GtoCConst);

  Standard_EXPORT void Load(const FreeGtoCConstraint& FGtoCConst);

  Standard_EXPORT void SolveTI(const Standard_Integer       ord         = 4,
                               const Standard_Real          anisotropie = 1.0,
                               const Message_ProgressRange& theProgress = Message_ProgressRange());

  //! returns True if all has been correctly done.
  Standard_EXPORT Standard_Boolean IsDone() const;

  Standard_EXPORT void destroy();

  ~PlateSurface() { destroy(); }

  //! reset the Plate in the initial state
  //! ( same as after Create())
  Standard_EXPORT void Init();

  Standard_EXPORT Coords3d Evaluate(const Coords2d& point2d) const;

  Standard_EXPORT Coords3d EvaluateDerivative(const Coords2d&           point2d,
                                            const Standard_Integer iu,
                                            const Standard_Integer iv) const;

  Standard_EXPORT void CoefPol(Handle(XYZGrid)& Coefs) const;

  Standard_EXPORT void SetPolynomialPartOnly(const Standard_Boolean PPOnly = Standard_True);

  Standard_EXPORT Standard_Integer Continuity() const;

  Standard_EXPORT void UVBox(Standard_Real& UMin,
                             Standard_Real& UMax,
                             Standard_Real& VMin,
                             Standard_Real& VMax) const;

  Standard_EXPORT void UVConstraints(TColgp_SequenceOfXY& Seq) const;

protected:
private:
  Standard_EXPORT Standard_Real SolEm(const Coords2d&           point2d,
                                      const Standard_Integer iu,
                                      const Standard_Integer iv) const;

  Standard_Real Polm(const Coords2d&           point2d,
                     const Standard_Integer iu,
                     const Standard_Integer iv,
                     const Standard_Integer idu,
                     const Standard_Integer idv) const;

  Standard_Integer& Deru(const Standard_Integer index) const;

  Standard_Integer& Derv(const Standard_Integer index) const;

  Coords3d& Solution(const Standard_Integer index) const;

  Coords2d& Points(const Standard_Integer index) const;

  Standard_EXPORT void SolveTI1(const Standard_Integer       IterationNumber,
                                const Message_ProgressRange& theProgress = Message_ProgressRange());

  Standard_EXPORT void SolveTI2(const Standard_Integer       IterationNumber,
                                const Message_ProgressRange& theProgress = Message_ProgressRange());

  Standard_EXPORT void SolveTI3(const Standard_Integer       IterationNumber,
                                const Message_ProgressRange& theProgress = Message_ProgressRange());

  Standard_EXPORT void fillXYZmatrix(math_Matrix&           mat,
                                     const Standard_Integer i0,
                                     const Standard_Integer j0,
                                     const Standard_Integer ncc1,
                                     const Standard_Integer ncc2) const;

  Standard_Integer                       order;
  Standard_Integer                       n_el;
  Standard_Integer                       n_dim;
  Standard_Address                       solution;
  Standard_Address                       points;
  Standard_Address                       deru;
  Standard_Address                       derv;
  Standard_Boolean                       OK;
  Plate_SequenceOfPinpointConstraint     myConstraints;
  Plate_SequenceOfLinearXYZConstraint    myLXYZConstraints;
  Plate_SequenceOfLinearScalarConstraint myLScalarConstraints;
  Standard_Real                          ddu[10];
  Standard_Real                          ddv[10];
  Standard_Integer                       maxConstraintOrder;
  Standard_Boolean                       PolynomialPartOnly;
  Standard_Real                          Uold;
  Standard_Real                          Vold;
  Standard_Real                          U2;
  Standard_Real                          R;
  Standard_Real                          L;
};

#include <Plate_Plate.lxx>

#endif // _Plate_Plate_HeaderFile
