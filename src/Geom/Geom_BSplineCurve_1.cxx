// Created on: 1991-07-05
// Created by: JCV
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

#include <BSplCLib.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_UndefinedDerivative.hxx>
#include <gp.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_DimensionError.hxx>
#include <Standard_DomainError.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_RangeError.hxx>

#define POLES (poles->Array1())
#define KNOTS (knots->Array1())
#define FKNOTS (flatknots->Array1())
#define FMULTS (BSplCLib1::NoMults())

//=================================================================================================

Standard_Boolean BSplineCurve3d::IsCN(const Standard_Integer N) const
{
  Standard_RangeError_Raise_if(N < 0, "BSplineCurve3d::IsCN");

  switch (smooth)
  {
    case GeomAbs_CN:
      return Standard_True;
    case GeomAbs_C0:
      return N <= 0;
    case GeomAbs_G1:
      return N <= 0;
    case GeomAbs_C1:
      return N <= 1;
    case GeomAbs_G2:
      return N <= 1;
    case GeomAbs_C2:
      return N <= 2;
    case GeomAbs_C3:
      return N <= 3 ? Standard_True
                    : N <= deg
                             - BSplCLib1::MaxKnotMult(mults->Array1(),
                                                     mults->Lower() + 1,
                                                     mults->Upper() - 1);
    default:
      return Standard_False;
  }
}

//=================================================================================================

Standard_Boolean BSplineCurve3d::IsG1(const Standard_Real theTf,
                                         const Standard_Real theTl,
                                         const Standard_Real theAngTol) const
{
  if (IsCN(1))
  {
    return Standard_True;
  }

  Standard_Integer start = FirstUKnotIndex() + 1, finish = LastUKnotIndex() - 1;
  Standard_Integer aDeg = Degree();
  for (Standard_Integer aNKnot = start; aNKnot <= finish; aNKnot++)
  {
    const Standard_Real aTpar = Knot(aNKnot);

    if (aTpar < theTf)
      continue;
    if (aTpar > theTl)
      break;

    Standard_Integer mult = Multiplicity(aNKnot);
    if (mult < aDeg)
      continue;

    Point3d aP1, aP2;
    Vector3d aV1, aV2;
    LocalD1(aTpar, aNKnot - 1, aNKnot, aP1, aV1);
    LocalD1(aTpar, aNKnot, aNKnot + 1, aP2, aV2);

    if ((aV1.SquareMagnitude() <= gp1::Resolution()) || aV2.SquareMagnitude() <= gp1::Resolution())
    {
      return Standard_False;
    }

    if (Abs(aV1.Angle(aV2)) > theAngTol)
      return Standard_False;
  }

  if (!IsPeriodic())
    return Standard_True;

  const Standard_Real aFirstParam = FirstParameter(), aLastParam = LastParameter();

  if (((aFirstParam - theTf) * (theTl - aFirstParam) < 0.0)
      && ((aLastParam - theTf) * (theTl - aLastParam) < 0.0))
  {
    // Range [theTf, theTl] does not intersect curve boundaries
    return Standard_True;
  }

  // Curve is closed or periodic and range [theTf, theTl]
  // intersect curve boundary. Therefore, it is necessary to
  // check if curve is smooth in its first and last point.

  Point3d aP;
  Vector3d aV1, aV2;
  D1(Knot(FirstUKnotIndex()), aP, aV1);
  D1(Knot(LastUKnotIndex()), aP, aV2);

  if ((aV1.SquareMagnitude() <= gp1::Resolution()) || aV2.SquareMagnitude() <= gp1::Resolution())
  {
    return Standard_False;
  }

  if (Abs(aV1.Angle(aV2)) > theAngTol)
    return Standard_False;

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BSplineCurve3d::IsClosed() const
//-- { return (StartPoint().Distance (EndPoint())) <= gp1::Resolution (); }
{
  return (StartPoint().SquareDistance(EndPoint())) <= 1e-16;
}

//=================================================================================================

Standard_Boolean BSplineCurve3d::IsPeriodic() const
{
  return periodic;
}

//=================================================================================================

GeomAbs_Shape BSplineCurve3d::Continuity() const
{
  return smooth;
}

//=================================================================================================

Standard_Integer BSplineCurve3d::Degree() const
{
  return deg;
}

//=================================================================================================

void BSplineCurve3d::D0(const Standard_Real U, Point3d& P) const
{
  Standard_Integer aSpanIndex = 0;
  Standard_Real    aNewU(U);
  PeriodicNormalization(aNewU);
  BSplCLib1::LocateParameter(deg, knots->Array1(), &mults->Array1(), U, periodic, aSpanIndex, aNewU);
  if (aNewU < knots->Value(aSpanIndex))
    aSpanIndex--;

  BSplCLib1::D0(aNewU,
               aSpanIndex,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               knots->Array1(),
               &mults->Array1(),
               P);
}

//=================================================================================================

void BSplineCurve3d::D1(const Standard_Real U, Point3d& P, Vector3d& V1) const
{
  Standard_Integer aSpanIndex = 0;
  Standard_Real    aNewU(U);
  PeriodicNormalization(aNewU);
  BSplCLib1::LocateParameter(deg, knots->Array1(), &mults->Array1(), U, periodic, aSpanIndex, aNewU);
  if (aNewU < knots->Value(aSpanIndex))
    aSpanIndex--;

  BSplCLib1::D1(aNewU,
               aSpanIndex,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               knots->Array1(),
               &mults->Array1(),
               P,
               V1);
}

//=================================================================================================

void BSplineCurve3d::D2(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2) const
{
  Standard_Integer aSpanIndex = 0;
  Standard_Real    aNewU(U);
  PeriodicNormalization(aNewU);
  BSplCLib1::LocateParameter(deg, knots->Array1(), &mults->Array1(), U, periodic, aSpanIndex, aNewU);
  if (aNewU < knots->Value(aSpanIndex))
    aSpanIndex--;

  BSplCLib1::D2(aNewU,
               aSpanIndex,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               knots->Array1(),
               &mults->Array1(),
               P,
               V1,
               V2);
}

//=================================================================================================

void BSplineCurve3d::D3(const Standard_Real U,
                           Point3d&             P,
                           Vector3d&             V1,
                           Vector3d&             V2,
                           Vector3d&             V3) const
{
  Standard_Integer aSpanIndex = 0;
  Standard_Real    aNewU(U);
  PeriodicNormalization(aNewU);
  BSplCLib1::LocateParameter(deg, knots->Array1(), &mults->Array1(), U, periodic, aSpanIndex, aNewU);
  if (aNewU < knots->Value(aSpanIndex))
    aSpanIndex--;

  BSplCLib1::D3(aNewU,
               aSpanIndex,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               knots->Array1(),
               &mults->Array1(),
               P,
               V1,
               V2,
               V3);
}

//=================================================================================================

Vector3d BSplineCurve3d::DN(const Standard_Real U, const Standard_Integer N) const
{
  Vector3d V;
  BSplCLib1::DN(U,
               N,
               0,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               FKNOTS,
               FMULTS,
               V);
  return V;
}

//=================================================================================================

Point3d BSplineCurve3d::EndPoint() const
{
  if (mults->Value(knots->Upper()) == deg + 1)
    return poles->Value(poles->Upper());
  else
    return Value(LastParameter());
}

//=================================================================================================

Standard_Integer BSplineCurve3d::FirstUKnotIndex() const
{
  if (periodic)
    return 1;
  else
    return BSplCLib1::FirstUKnotIndex(deg, mults->Array1());
}

//=================================================================================================

Standard_Real BSplineCurve3d::FirstParameter() const
{
  return flatknots->Value(deg + 1);
}

//=================================================================================================

Standard_Real BSplineCurve3d::Knot(const Standard_Integer Index) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > knots->Length(), "BSplineCurve3d::Knot");
  return knots->Value(Index);
}

//=================================================================================================

GeomAbs_BSplKnotDistribution BSplineCurve3d::KnotDistribution() const
{
  return knotSet;
}

//=================================================================================================

void BSplineCurve3d::Knots(TColStd_Array1OfReal& K) const
{
  Standard_DomainError_Raise_if(K.Lower() < knots->Lower() || K.Upper() > knots->Upper(),
                                "BSplineCurve3d::Knots");
  for (Standard_Integer anIdx = K.Lower(); anIdx <= K.Upper(); anIdx++)
    K(anIdx) = knots->Value(anIdx);
}

const TColStd_Array1OfReal& BSplineCurve3d::Knots() const
{
  return knots->Array1();
}

//=================================================================================================

void BSplineCurve3d::KnotSequence(TColStd_Array1OfReal& K) const
{
  Standard_DomainError_Raise_if(K.Lower() < flatknots->Lower() || K.Upper() > flatknots->Upper(),
                                "BSplineCurve3d::KnotSequence");
  for (Standard_Integer anIdx = K.Lower(); anIdx <= K.Upper(); anIdx++)
    K(anIdx) = flatknots->Value(anIdx);
}

const TColStd_Array1OfReal& BSplineCurve3d::KnotSequence() const
{
  return flatknots->Array1();
}

//=================================================================================================

Standard_Integer BSplineCurve3d::LastUKnotIndex() const
{
  if (periodic)
    return knots->Length();
  else
    return BSplCLib1::LastUKnotIndex(deg, mults->Array1());
}

//=================================================================================================

Standard_Real BSplineCurve3d::LastParameter() const
{
  return flatknots->Value(flatknots->Upper() - deg);
}

//=================================================================================================

Point3d BSplineCurve3d::LocalValue(const Standard_Real    U,
                                     const Standard_Integer FromK1,
                                     const Standard_Integer ToK2) const
{
  Point3d P;
  LocalD0(U, FromK1, ToK2, P);
  return P;
}

//=================================================================================================

void BSplineCurve3d::LocalD0(const Standard_Real    U,
                                const Standard_Integer FromK1,
                                const Standard_Integer ToK2,
                                Point3d&                P) const
{
  Standard_DomainError_Raise_if(FromK1 == ToK2, "BSplineCurve3d::LocalValue");

  Standard_Real    u     = U;
  Standard_Integer index = 0;
  BSplCLib1::LocateParameter(deg, FKNOTS, U, periodic, FromK1, ToK2, index, u);
  index = BSplCLib1::FlatIndex(deg, index, mults->Array1(), periodic);
  BSplCLib1::D0(u,
               index,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               FKNOTS,
               FMULTS,
               P);
}

//=================================================================================================

void BSplineCurve3d::LocalD1(const Standard_Real    U,
                                const Standard_Integer FromK1,
                                const Standard_Integer ToK2,
                                Point3d&                P,
                                Vector3d&                V1) const
{
  Standard_DomainError_Raise_if(FromK1 == ToK2, "BSplineCurve3d::LocalD1");

  Standard_Real    u     = U;
  Standard_Integer index = 0;
  BSplCLib1::LocateParameter(deg, FKNOTS, U, periodic, FromK1, ToK2, index, u);
  index = BSplCLib1::FlatIndex(deg, index, mults->Array1(), periodic);
  BSplCLib1::D1(u,
               index,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               FKNOTS,
               FMULTS,
               P,
               V1);
}

//=================================================================================================

void BSplineCurve3d::LocalD2(const Standard_Real    U,
                                const Standard_Integer FromK1,
                                const Standard_Integer ToK2,
                                Point3d&                P,
                                Vector3d&                V1,
                                Vector3d&                V2) const
{
  Standard_DomainError_Raise_if(FromK1 == ToK2, "BSplineCurve3d::LocalD2");

  Standard_Real    u     = U;
  Standard_Integer index = 0;
  BSplCLib1::LocateParameter(deg, FKNOTS, U, periodic, FromK1, ToK2, index, u);
  index = BSplCLib1::FlatIndex(deg, index, mults->Array1(), periodic);
  BSplCLib1::D2(u,
               index,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               FKNOTS,
               FMULTS,
               P,
               V1,
               V2);
}

//=================================================================================================

void BSplineCurve3d::LocalD3(const Standard_Real    U,
                                const Standard_Integer FromK1,
                                const Standard_Integer ToK2,
                                Point3d&                P,
                                Vector3d&                V1,
                                Vector3d&                V2,
                                Vector3d&                V3) const
{
  Standard_DomainError_Raise_if(FromK1 == ToK2, "BSplineCurve3d::LocalD3");

  Standard_Real    u     = U;
  Standard_Integer index = 0;
  BSplCLib1::LocateParameter(deg, FKNOTS, U, periodic, FromK1, ToK2, index, u);
  index = BSplCLib1::FlatIndex(deg, index, mults->Array1(), periodic);
  BSplCLib1::D3(u,
               index,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               FKNOTS,
               FMULTS,
               P,
               V1,
               V2,
               V3);
}

//=================================================================================================

Vector3d BSplineCurve3d::LocalDN(const Standard_Real    U,
                                  const Standard_Integer FromK1,
                                  const Standard_Integer ToK2,
                                  const Standard_Integer N) const
{
  Standard_DomainError_Raise_if(FromK1 == ToK2, "BSplineCurve3d::LocalD3");

  Standard_Real    u     = U;
  Standard_Integer index = 0;
  BSplCLib1::LocateParameter(deg, FKNOTS, U, periodic, FromK1, ToK2, index, u);
  index = BSplCLib1::FlatIndex(deg, index, mults->Array1(), periodic);

  Vector3d V;
  BSplCLib1::DN(u,
               N,
               index,
               deg,
               periodic,
               POLES,
               rational ? &weights->Array1() : BSplCLib1::NoWeights(),
               FKNOTS,
               FMULTS,
               V);
  return V;
}

//=================================================================================================

Standard_Integer BSplineCurve3d::Multiplicity(const Standard_Integer Index) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > mults->Length(),
                               "BSplineCurve3d::Multiplicity");
  return mults->Value(Index);
}

//=================================================================================================

void BSplineCurve3d::Multiplicities(TColStd_Array1OfInteger& M) const
{
  Standard_DimensionError_Raise_if(M.Length() != mults->Length(),
                                   "BSplineCurve3d::Multiplicities");
  M = mults->Array1();
}

const TColStd_Array1OfInteger& BSplineCurve3d::Multiplicities() const
{
  return mults->Array1();
}

//=================================================================================================

Standard_Integer BSplineCurve3d::NbKnots() const
{
  return knots->Length();
}

//=================================================================================================

Standard_Integer BSplineCurve3d::NbPoles() const
{
  return poles->Length();
}

//=================================================================================================

const Point3d& BSplineCurve3d::Pole(const Standard_Integer Index) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > poles->Length(), "BSplineCurve3d::Pole");
  return poles->Value(Index);
}

//=================================================================================================

void BSplineCurve3d::Poles(TColgp_Array1OfPnt& P) const
{
  Standard_DimensionError_Raise_if(P.Length() != poles->Length(), "BSplineCurve3d::Poles");
  P = poles->Array1();
}

const TColgp_Array1OfPnt& BSplineCurve3d::Poles() const
{
  return poles->Array1();
}

//=================================================================================================

Point3d BSplineCurve3d::StartPoint() const
{
  if (mults->Value(1) == deg + 1)
    return poles->Value(1);
  else
    return Value(FirstParameter());
}

//=================================================================================================

Standard_Real BSplineCurve3d::Weight(const Standard_Integer Index) const
{
  Standard_OutOfRange_Raise_if(Index < 1 || Index > poles->Length(), "BSplineCurve3d::Weight");
  if (IsRational())
    return weights->Value(Index);
  else
    return 1.;
}

//=================================================================================================

void BSplineCurve3d::Weights(TColStd_Array1OfReal& W) const
{
  Standard_DimensionError_Raise_if(W.Length() != poles->Length(), "BSplineCurve3d::Weights");
  if (IsRational())
    W = weights->Array1();
  else
  {
    Standard_Integer i;

    for (i = W.Lower(); i <= W.Upper(); i++)
      W(i) = 1.;
  }
}

const TColStd_Array1OfReal* BSplineCurve3d::Weights() const
{
  if (IsRational())
    return &weights->Array1();
  return BSplCLib1::NoWeights();
}

//=================================================================================================

Standard_Boolean BSplineCurve3d::IsRational() const
{
  return !weights.IsNull();
}

//=================================================================================================

void BSplineCurve3d::Transform(const Transform3d& T)
{
  TColgp_Array1OfPnt& CPoles = poles->ChangeArray1();
  for (Standard_Integer I = 1; I <= CPoles.Length(); I++)
    CPoles(I).Transform(T);
  maxderivinvok = 0;
}

//=======================================================================
// function : LocateU
// purpose  :
// pmn : 30/01/97 mise en conformite avec le cdl, lorsque U est un noeud
// (PRO6988)
//=======================================================================

void BSplineCurve3d::LocateU(const Standard_Real    U,
                                const Standard_Real    ParametricTolerance,
                                Standard_Integer&      I1,
                                Standard_Integer&      I2,
                                const Standard_Boolean WithKnotRepetition) const
{
  Standard_Real                 NewU = U;
  Handle(TColStd_HArray1OfReal) TheKnots;
  if (WithKnotRepetition)
    TheKnots = flatknots;
  else
    TheKnots = knots;
  const TColStd_Array1OfReal& CKnots = TheKnots->Array1();

  PeriodicNormalization(NewU); // Attention a la periode

  Standard_Real UFirst               = CKnots(1);
  Standard_Real ULast                = CKnots(CKnots.Length());
  Standard_Real PParametricTolerance = Abs(ParametricTolerance);
  if (Abs(NewU - UFirst) <= PParametricTolerance)
  {
    I1 = I2 = 1;
  }
  else if (Abs(NewU - ULast) <= PParametricTolerance)
  {
    I1 = I2 = CKnots.Length();
  }
  else if (NewU < UFirst)
  {
    I2 = 1;
    I1 = 0;
  }
  else if (NewU > ULast)
  {
    I1 = CKnots.Length();
    I2 = I1 + 1;
  }
  else
  {
    I1 = 1;
    BSplCLib1::Hunt(CKnots, NewU, I1);
    I1 = Max(Min(I1, CKnots.Upper()), CKnots.Lower());
    while (I1 + 1 <= CKnots.Upper() && Abs(CKnots(I1 + 1) - NewU) <= PParametricTolerance)
    {
      I1++;
    }
    if (Abs(CKnots(I1) - NewU) <= PParametricTolerance)
    {
      I2 = I1;
    }
    else
    {
      I2 = I1 + 1;
    }
  }
}

//=================================================================================================

void BSplineCurve3d::Resolution(const Standard_Real Tolerance3D, Standard_Real& UTolerance)
{
  if (!maxderivinvok)
  {
    if (periodic)
    {
      Standard_Integer NbKnots, NbPoles;
      BSplCLib1::PrepareUnperiodize(deg, mults->Array1(), NbKnots, NbPoles);
      TColgp_Array1OfPnt   new_poles(1, NbPoles);
      TColStd_Array1OfReal new_weights(1, NbPoles);
      for (Standard_Integer ii = 1; ii <= NbPoles; ii++)
      {
        new_poles(ii) = poles->Array1()((ii - 1) % poles->Length() + 1);
      }
      if (rational)
      {
        for (Standard_Integer ii = 1; ii <= NbPoles; ii++)
        {
          new_weights(ii) = weights->Array1()((ii - 1) % poles->Length() + 1);
        }
      }
      BSplCLib1::Resolution(new_poles,
                           rational ? &new_weights : BSplCLib1::NoWeights(),
                           new_poles.Length(),
                           flatknots->Array1(),
                           deg,
                           1.,
                           maxderivinv);
    }
    else
    {
      BSplCLib1::Resolution(poles->Array1(),
                           rational ? &weights->Array1() : BSplCLib1::NoWeights(),
                           poles->Length(),
                           flatknots->Array1(),
                           deg,
                           1.,
                           maxderivinv);
    }
    maxderivinvok = 1;
  }
  UTolerance = Tolerance3D * maxderivinv;
}

//=================================================================================================

Standard_Boolean BSplineCurve3d::IsEqual(const Handle(BSplineCurve3d)& theOther,
                                            const Standard_Real              thePreci) const
{
  if (knots.IsNull() || poles.IsNull() || mults.IsNull())
    return Standard_False;
  if (deg != theOther->Degree())
    return Standard_False;
  if (knots->Length() != theOther->NbKnots() || poles->Length() != theOther->NbPoles())
    return Standard_False;

  Standard_Integer i = 1;
  for (i = 1; i <= poles->Length(); i++)
  {
    const Point3d& aPole1 = poles->Value(i);
    const Point3d& aPole2 = theOther->Pole(i);
    if (fabs(aPole1.X() - aPole2.X()) > thePreci || fabs(aPole1.Y() - aPole2.Y()) > thePreci
        || fabs(aPole1.Z() - aPole2.Z()) > thePreci)
      return Standard_False;
  }

  for (; i <= knots->Length(); i++)
  {
    if (fabs(knots->Value(i) - theOther->Knot(i)) > Precision1::Parametric(thePreci))
      return Standard_False;
  }

  for (i = 1; i <= mults->Length(); i++)
  {
    if (mults->Value(i) != theOther->Multiplicity(i))
      return Standard_False;
  }

  if (rational != theOther->IsRational())
    return Standard_False;

  if (!rational)
    return Standard_True;

  for (i = 1; i <= weights->Length(); i++)
  {
    if (fabs(Standard_Real(weights->Value(i) - theOther->Weight(i))) > Epsilon(weights->Value(i)))
      return Standard_False;
  }
  return Standard_True;
}
