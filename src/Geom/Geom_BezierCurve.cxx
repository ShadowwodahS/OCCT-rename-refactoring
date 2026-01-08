// Created on: 1993-03-09
// Created by: JCV
// Copyright (c) 1993-1999 Matra Datavision
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

// Passage en classe persistante - 23/01/91
// Modif suite a la deuxieme revue de projet toolkit Geometry1 -23/01/91
// Infos :
// Actuellement pour les champs de la courbe le tableau des poles est
// declare de 1 a NbPoles et le tableau des poids est declare de 1 a NbPoles

// Revised RLE  Aug 19 1993
// Suppressed Swaps, added Init, removed typedefs

#define No_Standard_OutOfRange
#define No_Standard_DimensionError

#include <Geom_BezierCurve.hxx>
#include <Geom_Geometry.hxx>
#include <gp.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <PLib.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_DimensionError.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_Type.hxx>
#include <TColStd_Array1OfInteger.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BezierCurve3d, Geom_BoundedCurve)

//=======================================================================
// function : Rational
// purpose  : check rationality of an array of weights
//=======================================================================
static Standard_Boolean Rational(const TColStd_Array1OfReal& W)
{
  Standard_Integer i, n = W.Length();
  Standard_Boolean rat = Standard_False;
  for (i = 1; i < n; i++)
  {
    rat = Abs(W(i) - W(i + 1)) > gp1::Resolution();
    if (rat)
      break;
  }
  return rat;
}

//=================================================================================================

BezierCurve3d::BezierCurve3d(const TColgp_Array1OfPnt& Poles)
{
  Standard_Integer nbpoles = Poles.Length();
  if (nbpoles < 2 || nbpoles > (BezierCurve3d::MaxDegree() + 1))
    throw Standard_ConstructionError();
  //  copy the poles
  Handle(TColgp_HArray1OfPnt) npoles = new TColgp_HArray1OfPnt(1, nbpoles);

  npoles->ChangeArray1() = Poles;

  // Init non rational
  Init(npoles, Handle(TColStd_HArray1OfReal)());
}

//=================================================================================================

BezierCurve3d::BezierCurve3d(const TColgp_Array1OfPnt&   Poles,
                                   const TColStd_Array1OfReal& Weights)
{
  // copy the poles
  Standard_Integer nbpoles = Poles.Length();
  if (nbpoles < 2 || nbpoles > (BezierCurve3d::MaxDegree() + 1))
    throw Standard_ConstructionError();

  Handle(TColgp_HArray1OfPnt) npoles = new TColgp_HArray1OfPnt(1, nbpoles);

  npoles->ChangeArray1() = Poles;

  // check  the weights

  if (Weights.Length() != nbpoles)
    throw Standard_ConstructionError();

  Standard_Integer i;
  for (i = 1; i <= nbpoles; i++)
  {
    if (Weights(i) <= gp1::Resolution())
    {
      throw Standard_ConstructionError();
    }
  }

  // check really rational
  Standard_Boolean rat = Rational(Weights);

  // copy the weights
  Handle(TColStd_HArray1OfReal) nweights;
  if (rat)
  {
    nweights                 = new TColStd_HArray1OfReal(1, nbpoles);
    nweights->ChangeArray1() = Weights;
  }

  // Init
  Init(npoles, nweights);
}

//=======================================================================
// function : Increase
// purpose  : increase degree
//=======================================================================

void BezierCurve3d::Increase(const Standard_Integer Deg)
{
  if (Deg == Degree())
    return;

  if (Deg < Degree() || Deg > BezierCurve3d::MaxDegree())
    throw Standard_ConstructionError("BezierCurve3d::Increase");

  Handle(TColgp_HArray1OfPnt) npoles = new TColgp_HArray1OfPnt(1, Deg + 1);

  Handle(TColStd_HArray1OfReal) nweights;

  TColStd_Array1OfReal bidknots(1, 2);
  bidknots(1) = 0.;
  bidknots(2) = 1.;
  TColStd_Array1OfInteger bidmults(1, 2);
  bidmults.Init(Degree() + 1);

  if (IsRational())
  {
    nweights = new TColStd_HArray1OfReal(1, Deg + 1);
    BSplCLib1::IncreaseDegree(Degree(),
                             Deg,
                             0,
                             poles->Array1(),
                             &weights->Array1(),
                             bidknots,
                             bidmults,
                             npoles->ChangeArray1(),
                             &nweights->ChangeArray1(),
                             bidknots,
                             bidmults);
  }
  else
  {
    BSplCLib1::IncreaseDegree(Degree(),
                             Deg,
                             0,
                             poles->Array1(),
                             BSplCLib1::NoWeights(),
                             bidknots,
                             bidmults,
                             npoles->ChangeArray1(),
                             BSplCLib1::NoWeights(),
                             bidknots,
                             bidmults);
  }

  Init(npoles, nweights);
}

//=================================================================================================

Standard_Integer BezierCurve3d::MaxDegree()
{
  return BSplCLib1::MaxDegree();
}

//=================================================================================================

void BezierCurve3d::InsertPoleAfter(const Standard_Integer Index, const Point3d& P)
{
  InsertPoleAfter(Index, P, 1.);
}

//=================================================================================================

void BezierCurve3d::InsertPoleAfter(const Standard_Integer Index,
                                       const Point3d&          P,
                                       const Standard_Real    Weight)
{
  Standard_Integer nbpoles = NbPoles();

  if (nbpoles >= BezierCurve3d::MaxDegree() || Weight <= gp1::Resolution())
    throw Standard_ConstructionError("BezierCurve3d::InsertPoleAfter");

  if (Index < 0 || Index > nbpoles)
    throw Standard_OutOfRange("BezierCurve3d::InsertPoleAfter");

  Standard_Integer i;

  // Insert the pole
  Handle(TColgp_HArray1OfPnt) npoles = new TColgp_HArray1OfPnt(1, nbpoles + 1);

  TColgp_Array1OfPnt&       newpoles = npoles->ChangeArray1();
  const TColgp_Array1OfPnt& oldpoles = poles->Array1();

  for (i = 1; i <= Index; i++)
    newpoles(i) = oldpoles(i);

  newpoles(Index + 1) = P;

  for (i = Index + 1; i <= nbpoles; i++)
    newpoles(i + 1) = oldpoles(i);

  // Insert the weight
  Handle(TColStd_HArray1OfReal) nweights;
  Standard_Boolean              rat = IsRational() || Abs(Weight - 1.) > gp1::Resolution();

  if (rat)
  {
    nweights                         = new TColStd_HArray1OfReal(1, nbpoles + 1);
    TColStd_Array1OfReal& newweights = nweights->ChangeArray1();

    for (i = 1; i <= Index; i++)
      if (IsRational())
        newweights(i) = weights->Value(i);
      else
        newweights(i) = 1.;

    newweights(Index + 1) = Weight;

    for (i = Index + 1; i <= nbpoles; i++)
      if (IsRational())
        newweights(i + 1) = weights->Value(i);
      else
        newweights(i + 1) = 1.;
  }

  Init(npoles, nweights);
}

//=================================================================================================

void BezierCurve3d::InsertPoleBefore(const Standard_Integer Index, const Point3d& P)
{
  InsertPoleAfter(Index - 1, P);
}

//=================================================================================================

void BezierCurve3d::InsertPoleBefore(const Standard_Integer Index,
                                        const Point3d&          P,
                                        const Standard_Real    Weight)
{
  InsertPoleAfter(Index - 1, P, Weight);
}

//=================================================================================================

void BezierCurve3d::RemovePole(const Standard_Integer Index)
{
  Standard_Integer nbpoles = NbPoles();

  if (nbpoles <= 2)
    throw Standard_ConstructionError("BezierCurve3d::RemovePole");

  if (Index < 1 || Index > nbpoles)
    throw Standard_OutOfRange("BezierCurve3d::RemovePole");

  Standard_Integer i;

  // Remove the pole
  Handle(TColgp_HArray1OfPnt) npoles = new TColgp_HArray1OfPnt(1, nbpoles - 1);

  TColgp_Array1OfPnt&       newpoles = npoles->ChangeArray1();
  const TColgp_Array1OfPnt& oldpoles = poles->Array1();

  for (i = 1; i < Index; i++)
    newpoles(i) = oldpoles(i);

  for (i = Index + 1; i <= nbpoles; i++)
    newpoles(i - 1) = oldpoles(i);

  // Remove the weight
  Handle(TColStd_HArray1OfReal) nweights;

  if (IsRational())
  {
    nweights                               = new TColStd_HArray1OfReal(1, nbpoles - 1);
    TColStd_Array1OfReal&       newweights = nweights->ChangeArray1();
    const TColStd_Array1OfReal& oldweights = weights->Array1();

    for (i = 1; i < Index; i++)
      newweights(i) = oldweights(i);

    for (i = Index + 1; i <= nbpoles; i++)
      newweights(i - 1) = oldweights(i);
  }

  Init(npoles, nweights);
}

//=================================================================================================

void BezierCurve3d::Reverse()
{
  Point3d              P;
  Standard_Integer    i, nbpoles = NbPoles();
  TColgp_Array1OfPnt& cpoles = poles->ChangeArray1();

  // reverse poles
  for (i = 1; i <= nbpoles / 2; i++)
  {
    P                       = cpoles(i);
    cpoles(i)               = cpoles(nbpoles - i + 1);
    cpoles(nbpoles - i + 1) = P;
  }

  // reverse weights
  if (IsRational())
  {
    TColStd_Array1OfReal& cweights = weights->ChangeArray1();
    Standard_Real         w;
    for (i = 1; i <= nbpoles / 2; i++)
    {
      w                         = cweights(i);
      cweights(i)               = cweights(nbpoles - i + 1);
      cweights(nbpoles - i + 1) = w;
    }
  }
}

//=================================================================================================

Standard_Real BezierCurve3d::ReversedParameter(const Standard_Real U) const
{
  return (1. - U);
}

//=================================================================================================

void BezierCurve3d::Segment1(const Standard_Real U1, const Standard_Real U2)
{
  closed = (Abs(Value(U1).Distance(Value(U2))) <= Precision1::Confusion());

  TColStd_Array1OfReal bidflatknots(BSplCLib1::FlatBezierKnots(Degree()), 1, 2 * (Degree() + 1));
  TColgp_HArray1OfPnt  coeffs(1, poles->Size());
  if (IsRational())
  {
    TColStd_Array1OfReal wcoeffs(1, poles->Size());
    BSplCLib1::BuildCache(0.0,
                         1.0,
                         0,
                         Degree(),
                         bidflatknots,
                         poles->Array1(),
                         &weights->Array1(),
                         coeffs,
                         &wcoeffs);
    PLib1::Trimming(U1, U2, coeffs, &wcoeffs);
    PLib1::CoefficientsPoles(coeffs, &wcoeffs, poles->ChangeArray1(), &weights->ChangeArray1());
  }
  else
  {
    BSplCLib1::BuildCache(0.0,
                         1.0,
                         0,
                         Degree(),
                         bidflatknots,
                         poles->Array1(),
                         BSplCLib1::NoWeights(),
                         coeffs,
                         BSplCLib1::NoWeights());
    PLib1::Trimming(U1, U2, coeffs, PLib1::NoWeights());
    PLib1::CoefficientsPoles(coeffs, PLib1::NoWeights(), poles->ChangeArray1(), PLib1::NoWeights());
  }
}

//=================================================================================================

void BezierCurve3d::SetPole(const Standard_Integer Index, const Point3d& P)
{
  if (Index < 1 || Index > NbPoles())
    throw Standard_OutOfRange("BezierCurve3d::SetPole");

  TColgp_Array1OfPnt& cpoles = poles->ChangeArray1();
  cpoles(Index)              = P;

  if (Index == 1 || Index == cpoles.Length())
  {
    closed = (cpoles(1).Distance(cpoles(NbPoles())) <= Precision1::Confusion());
  }
}

//=================================================================================================

void BezierCurve3d::SetPole(const Standard_Integer Index,
                               const Point3d&          P,
                               const Standard_Real    Weight)
{
  SetPole(Index, P);
  SetWeight(Index, Weight);
}

//=================================================================================================

void BezierCurve3d::SetWeight(const Standard_Integer Index, const Standard_Real Weight)
{
  Standard_Integer nbpoles = NbPoles();

  if (Index < 1 || Index > nbpoles)
    throw Standard_OutOfRange("BezierCurve3d::SetWeight");
  if (Weight <= gp1::Resolution())
    throw Standard_ConstructionError("BezierCurve3d::SetWeight");

  // compute new rationality
  Standard_Boolean wasrat = IsRational();
  if (!wasrat)
  {
    // a weight of 1. does not turn to rational
    if (Abs(Weight - 1.) <= gp1::Resolution())
      return;

    // set weights of 1.
    weights = new TColStd_HArray1OfReal(1, nbpoles);
    weights->Init(1.);
  }

  TColStd_Array1OfReal& cweights = weights->ChangeArray1();
  cweights(Index)                = Weight;

  // is it turning into non rational
  if (wasrat && !Rational(cweights))
    weights.Nullify();
}

//=================================================================================================

Standard_Boolean BezierCurve3d::IsClosed() const
{
  return closed;
}

//=================================================================================================

Standard_Boolean BezierCurve3d::IsCN(const Standard_Integer) const
{
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BezierCurve3d::IsPeriodic() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BezierCurve3d::IsRational() const
{
  return !weights.IsNull();
}

//=================================================================================================

GeomAbs_Shape BezierCurve3d::Continuity() const
{
  return GeomAbs_CN;
}

//=================================================================================================

Standard_Integer BezierCurve3d::Degree() const
{
  return poles->Length() - 1;
}

//=================================================================================================

void BezierCurve3d::D0(const Standard_Real U, Point3d& P) const
{
  BSplCLib1::D0(U, Poles(), Weights(), P);
}

//=================================================================================================

void BezierCurve3d::D1(const Standard_Real U, Point3d& P, Vector3d& V1) const
{
  BSplCLib1::D1(U, Poles(), Weights(), P, V1);
}

//=================================================================================================

void BezierCurve3d::D2(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2) const
{
  BSplCLib1::D2(U, Poles(), Weights(), P, V1, V2);
}

//=================================================================================================

void BezierCurve3d::D3(const Standard_Real U,
                          Point3d&             P,
                          Vector3d&             V1,
                          Vector3d&             V2,
                          Vector3d&             V3) const
{
  BSplCLib1::D3(U, Poles(), Weights(), P, V1, V2, V3);
}

//=================================================================================================

Vector3d BezierCurve3d::DN(const Standard_Real U, const Standard_Integer N) const
{
  if (N < 1)
    throw Standard_RangeError("BezierCurve3d::DN");
  Vector3d V;

  TColStd_Array1OfReal bidknots(1, 2);
  bidknots(1) = 0.;
  bidknots(2) = 1.;
  TColStd_Array1OfInteger bidmults(1, 2);
  bidmults.Init(Degree() + 1);

  if (IsRational())
    //    BSplCLib1::DN(U,N,0,Degree(),0.,
    BSplCLib1::DN(U,
                 N,
                 0,
                 Degree(),
                 Standard_False,
                 poles->Array1(),
                 &weights->Array1(),
                 bidknots,
                 &bidmults,
                 V);
  else
    //    BSplCLib1::DN(U,N,0,Degree(),0.,
    BSplCLib1::DN(U,
                 N,
                 0,
                 Degree(),
                 Standard_False,
                 poles->Array1(),
                 BSplCLib1::NoWeights(),
                 bidknots,
                 &bidmults,
                 V);
  return V;
}

//=================================================================================================

Point3d BezierCurve3d::StartPoint() const
{
  return poles->Value(1);
}

//=================================================================================================

Point3d BezierCurve3d::EndPoint() const
{
  return poles->Value(poles->Upper());
}

//=================================================================================================

Standard_Real BezierCurve3d::FirstParameter() const
{
  return 0.0;
}

//=================================================================================================

Standard_Real BezierCurve3d::LastParameter() const
{
  return 1.0;
}

//=================================================================================================

Standard_Integer BezierCurve3d::NbPoles() const
{
  return poles->Length();
}

//=================================================================================================

const Point3d& BezierCurve3d::Pole(const Standard_Integer Index) const
{
  if (Index < 1 || Index > poles->Length())
    throw Standard_OutOfRange("BezierCurve3d::Pole");
  return poles->Value(Index);
}

//=================================================================================================

void BezierCurve3d::Poles(TColgp_Array1OfPnt& P) const
{
  if (P.Length() != poles->Length())
    throw Standard_DimensionError("BezierCurve3d::Poles");
  P = poles->Array1();
}

//=================================================================================================

const TColgp_Array1OfPnt& BezierCurve3d::Poles() const
{
  return poles->Array1();
}

//=================================================================================================

Standard_Real BezierCurve3d::Weight(const Standard_Integer Index) const
{
  if (Index < 1 || Index > poles->Length())
    throw Standard_OutOfRange("BezierCurve3d::Weight");
  if (IsRational())
    return weights->Value(Index);
  else
    return 1.;
}

//=================================================================================================

void BezierCurve3d::Weights(TColStd_Array1OfReal& W) const
{

  Standard_Integer nbpoles = NbPoles();
  if (W.Length() != nbpoles)
    throw Standard_DimensionError("BezierCurve3d::Weights");
  if (IsRational())
    W = weights->Array1();
  else
  {
    Standard_Integer i;
    for (i = 1; i <= nbpoles; i++)
      W(i) = 1.;
  }
}

//=================================================================================================

void BezierCurve3d::Transform(const Transform3d& T)
{
  Standard_Integer    nbpoles = NbPoles();
  TColgp_Array1OfPnt& cpoles  = poles->ChangeArray1();

  for (Standard_Integer i = 1; i <= nbpoles; i++)
    cpoles(i).Transform(T);
}

//=================================================================================================

void BezierCurve3d::Resolution(const Standard_Real Tolerance3D, Standard_Real& UTolerance)
{
  if (!maxderivinvok)
  {
    TColStd_Array1OfReal bidflatknots(BSplCLib1::FlatBezierKnots(Degree()), 1, 2 * (Degree() + 1));

    if (IsRational())
    {
      BSplCLib1::Resolution(poles->Array1(),
                           &weights->Array1(),
                           poles->Length(),
                           bidflatknots,
                           Degree(),
                           1.,
                           maxderivinv);
    }
    else
    {
      BSplCLib1::Resolution(poles->Array1(),
                           BSplCLib1::NoWeights(),
                           poles->Length(),
                           bidflatknots,
                           Degree(),
                           1.,
                           maxderivinv);
    }
    maxderivinvok = 1;
  }
  UTolerance = Tolerance3D * maxderivinv;
}

//=================================================================================================

Handle(Geom_Geometry) BezierCurve3d::Copy() const
{

  Handle(BezierCurve3d) C;
  if (IsRational())
    C = new BezierCurve3d(poles->Array1(), weights->Array1());
  else
    C = new BezierCurve3d(poles->Array1());
  return C;
}

//=================================================================================================

void BezierCurve3d::Init(const Handle(TColgp_HArray1OfPnt)&   Poles,
                            const Handle(TColStd_HArray1OfReal)& Weights)
{
  Standard_Integer nbpoles = Poles->Length();
  // closed ?
  const TColgp_Array1OfPnt& cpoles = Poles->Array1();
  closed                           = cpoles(1).Distance(cpoles(nbpoles)) <= Precision1::Confusion();

  // rational
  rational = !Weights.IsNull();

  // set fields
  poles = Poles;

  if (rational)
    weights = Weights;
  else
    weights.Nullify();
}

//=================================================================================================

void BezierCurve3d::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, Geom_BoundedCurve)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, rational)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, closed)
  if (!poles.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, poles->Size())
  if (!weights.IsNull())
    OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, weights->Size())

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, maxderivinv)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, maxderivinvok)
}
