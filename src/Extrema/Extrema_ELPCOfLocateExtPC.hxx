// Created on: 1991-02-26
// Created by: Isabelle GRIGNON
// Copyright (c) 1991-1999 Matra Datavision
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

#ifndef _Extrema_ELPCOfLocateExtPC_HeaderFile
#define _Extrema_ELPCOfLocateExtPC_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <gp_Pnt.hxx>
#include <Extrema_ExtPElC.hxx>
#include <Extrema_SequenceOfPOnCurv.hxx>
#include <Extrema_EPCOfELPCOfLocateExtPC.hxx>
#include <Standard_Integer.hxx>
#include <GeomAbs_CurveType.hxx>
#include <TColStd_SequenceOfBoolean.hxx>
#include <TColStd_SequenceOfReal.hxx>
class StdFail_NotDone;
class Standard_OutOfRange;
class Standard_TypeMismatch;
class Curve5;
class CurveTool4;
class PointElCCurveExtrema;
class Point3d;
class Vector3d;
class PointOnCurve1;
class Extrema_EPCOfELPCOfLocateExtPC;
class Extrema_PCFOfEPCOfELPCOfLocateExtPC;

class Extrema_ELPCOfLocateExtPC
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT Extrema_ELPCOfLocateExtPC();

  //! It calculates all the distances.
  //! The function F(u)=distance(P,C(u)) has an extremum
  //! when g(u)=dF/du=0. The algorithm searches all the
  //! zeros inside the definition range of the curve.
  //! Zeros are searched between uinf and usup.
  //! Tol  is used to decide to stop the
  //! iterations according to the following condition:
  //! if n is the number of iterations,
  //! the algorithm stops when abs(F(Un)-F(Un-1)) < Tol.
  Standard_EXPORT Extrema_ELPCOfLocateExtPC(const Point3d&          P,
                                            const Curve5& C,
                                            const Standard_Real    Uinf,
                                            const Standard_Real    Usup,
                                            const Standard_Real    TolF = 1.0e-10);

  //! It calculates all the distances.
  //! The function F(u)=distance(P,C(u)) has an extremum
  //! when g(u)=dF/du=0. The algorithm searches all the
  //! zeros inside the definition range of the curve.
  //! Tol is used to decide to stop the
  //! iterations according to the following condition:
  //! if n is the number of iterations,
  //! the algorithm stops when abs(F(Un)-F(Un-1)) < Tol.
  Standard_EXPORT Extrema_ELPCOfLocateExtPC(const Point3d&          P,
                                            const Curve5& C,
                                            const Standard_Real    TolF = 1.0e-10);

  //! initializes the fields of the algorithm.
  Standard_EXPORT void Initialize(const Curve5& C,
                                  const Standard_Real    Uinf,
                                  const Standard_Real    Usup,
                                  const Standard_Real    TolF = 1.0e-10);

  //! An exception is raised if the fields have not been
  //! initialized.
  Standard_EXPORT void Perform(const Point3d& P);

  //! True if the distances are found.
  Standard_EXPORT Standard_Boolean IsDone() const;

  //! Returns the value of the <N>th extremum square distance.
  Standard_EXPORT Standard_Real SquareDistance(const Standard_Integer N) const;

  //! Returns the number of extremum distances.
  Standard_EXPORT Standard_Integer NbExt() const;

  //! Returns True if the <N>th extremum distance is a
  //! minimum.
  Standard_EXPORT Standard_Boolean IsMin(const Standard_Integer N) const;

  //! Returns the point of the <N>th extremum distance.
  Standard_EXPORT const PointOnCurve1& Point(const Standard_Integer N) const;

  //! if the curve is a trimmed curve,
  //! dist1 is a square distance between <P> and the point
  //! of parameter FirstParameter <P1> and
  //! dist2 is a square distance between <P> and the point
  //! of parameter LastParameter <P2>.
  Standard_EXPORT void TrimmedSquareDistances(Standard_Real& dist1,
                                              Standard_Real& dist2,
                                              Point3d&        P1,
                                              Point3d&        P2) const;

protected:
  Standard_EXPORT void IntervalPerform(const Point3d& P);

  Standard_EXPORT void AddSol(const Standard_Real    theU,
                              const Point3d&          theP,
                              const Standard_Real    theSqDist,
                              const Standard_Boolean isMin);

private:
  Standard_Address               myC;
  Point3d                         Pf;
  Point3d                         Pl;
  PointElCCurveExtrema                myExtPElC;
  Extrema_SequenceOfPOnCurv      mypoint;
  Standard_Boolean               mydone;
  Standard_Real                  mydist1;
  Standard_Real                  mydist2;
  Extrema_EPCOfELPCOfLocateExtPC myExtPC;
  Standard_Real                  mytolu;
  Standard_Real                  mytolf;
  Standard_Integer               mysample;
  Standard_Real                  myintuinf;
  Standard_Real                  myintusup;
  Standard_Real                  myuinf;
  Standard_Real                  myusup;
  GeomAbs_CurveType              type;
  TColStd_SequenceOfBoolean      myismin;
  TColStd_SequenceOfReal         mySqDist;
};

#endif // _Extrema_ELPCOfLocateExtPC_HeaderFile
