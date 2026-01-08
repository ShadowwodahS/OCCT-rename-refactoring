// Created on: 1998-04-15
// Created by: Stephanie HUMEAU
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

#include <GeomFill_DraftTrihedron.hxx>

#include <Adaptor3d_Curve.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomFill_TrihedronLaw.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_OutOfRange.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomFill_DraftTrihedron, GeomFill_TrihedronLaw)

//=======================================================================
// function : DDeriv
// purpose  : computes (F/|F|)''
//=======================================================================
static Vector3d DDeriv(const Vector3d& F, const Vector3d& DF, const Vector3d& D2F)
{
  Standard_Real Norma = F.Magnitude();

  Vector3d Result =
    (D2F - 2 * DF * (F * DF) / (Norma * Norma)) / Norma
    - F
        * ((DF.SquareMagnitude() + F * D2F - 3 * (F * DF) * (F * DF) / (Norma * Norma))
           / (Norma * Norma * Norma));
  return Result;
}

//=================================================================================================

GeomFill_DraftTrihedron::GeomFill_DraftTrihedron(const Vector3d& BiNormal, const Standard_Real Angle)
{
  B = BiNormal;
  B.Normalize();
  SetAngle(Angle);
}

//=================================================================================================

void GeomFill_DraftTrihedron::SetAngle(const Standard_Real Angle)
{
  myAngle = M_PI / 2 + Angle;
  myCos   = Cos(myAngle);
}

//=======================================================================
// function : D0
// purpose  : calculation of trihedron
//=======================================================================
Standard_Boolean GeomFill_DraftTrihedron::D0(const Standard_Real Param,
                                             Vector3d&             Tangent,
                                             Vector3d&             Normal,
                                             Vector3d&             BiNormal)
{
  Point3d P;
  Vector3d T;
  myTrimmed->D1(Param, P, T);
  T.Normalize();

  Vector3d        b     = T.Crossed(B);
  Standard_Real normb = b.Magnitude();

  b /= normb;
  if (normb < 1.e-12)
    return Standard_False;

  Vector3d v = b.Crossed(T);

  Standard_Real mu = myCos;
  mu               = myCos;

  // La Normal est portee par la regle
  Normal.SetLinearForm(Sqrt(1 - mu * mu), b, mu, v);

  // Le reste suit....
  // La tangente est perpendiculaire a la normale et a la direction de depouille
  Tangent = Normal.Crossed(B);
  Tangent.Normalize();

  BiNormal = Tangent;
  BiNormal.Cross(Normal);

  return Standard_True;
}

//=======================================================================
// function : D1
// purpose  :  calculation of trihedron and first derivative
//=======================================================================
Standard_Boolean GeomFill_DraftTrihedron::D1(const Standard_Real Param,
                                             Vector3d&             Tangent,
                                             Vector3d&             DTangent,
                                             Vector3d&             Normal,
                                             Vector3d&             DNormal,
                                             Vector3d&             BiNormal,
                                             Vector3d&             DBiNormal)
{
  Point3d P;
  Vector3d T, DT, aux;

  myTrimmed->D2(Param, P, T, aux);

  Standard_Real normT, normb;
  normT = T.Magnitude();
  T /= normT;
  DT.SetLinearForm(-(T.Dot(aux)), T, aux);
  DT /= normT;

  Vector3d db, b = T.Crossed(B);
  normb = b.Magnitude();
  if (normb < 1.e-12)
    return Standard_False;
  b /= normb;
  aux = DT.Crossed(B);
  db.SetLinearForm(-(b.Dot(aux)), b, aux);
  db /= normb;

  Vector3d v  = b.Crossed(T);
  Vector3d dv = db.Crossed(T) + b.Crossed(DT);

  Standard_Real mu = myCos;

  Normal.SetLinearForm(Sqrt(1 - mu * mu), b, mu, v);
  DNormal.SetLinearForm(Sqrt(1 - mu * mu), db, mu, dv);

  Tangent = Normal.Crossed(B);
  normT   = Tangent.Magnitude();
  Tangent /= normT;
  aux = DNormal.Crossed(B);
  DTangent.SetLinearForm(-(Tangent.Dot(aux)), Tangent, aux);
  DTangent /= normT;

  BiNormal = Tangent;
  BiNormal.Cross(Normal);
  DBiNormal.SetLinearForm(DTangent.Crossed(Normal), Tangent.Crossed(DNormal));

  return Standard_True;
}

//=======================================================================
// function : D2
// purpose  : calculation of trihedron and derivatives 1 et 2
//=======================================================================
Standard_Boolean GeomFill_DraftTrihedron::D2(const Standard_Real Param,
                                             Vector3d&             Tangent,
                                             Vector3d&             DTangent,
                                             Vector3d&             D2Tangent,
                                             Vector3d&             Normal,
                                             Vector3d&             DNormal,
                                             Vector3d&             D2Normal,
                                             Vector3d&             BiNormal,
                                             Vector3d&             DBiNormal,
                                             Vector3d&             D2BiNormal)
{
  Point3d        P;
  Vector3d        T, DT, D2T, aux, aux2;
  Standard_Real dot;

  myTrimmed->D3(Param, P, T, aux, aux2);

  Standard_Real normT, normb;

  D2T   = DDeriv(T, aux, aux2);
  normT = T.Magnitude();
  T /= normT;
  dot = T.Dot(aux);
  DT.SetLinearForm(-dot, T, aux);
  DT /= normT;

  Vector3d db, d2b, b = T.Crossed(B);
  normb = b.Magnitude();
  if (normb < 1.e-12)
    return Standard_False;

  aux  = DT.Crossed(B);
  aux2 = D2T.Crossed(B);
  d2b  = DDeriv(b, aux, aux2);
  b /= normb;
  dot = b.Dot(aux);
  db.SetLinearForm(-dot, b, aux);
  db /= normb;

  Vector3d v   = b.Crossed(T);
  Vector3d dv  = db.Crossed(T) + b.Crossed(DT);
  Vector3d d2v = d2b.Crossed(T) + 2 * db.Crossed(DT) + b.Crossed(D2T);

  Standard_Real mu = myCos, rac;
  rac              = Sqrt(1 - mu * mu);

  Normal.SetLinearForm(rac, b, mu, v);
  DNormal.SetLinearForm(rac, db, mu, dv);
  D2Normal.SetLinearForm(rac, d2b, mu, d2v);

  Tangent = Normal.Crossed(B);
  normT   = Tangent.Magnitude();

  aux       = DNormal.Crossed(B);
  aux2      = D2Normal.Crossed(B);
  D2Tangent = DDeriv(Tangent, aux, aux2);
  Tangent /= normT;
  dot = Tangent.Dot(aux);
  DTangent.SetLinearForm(-dot, Tangent, aux);
  DTangent /= normT;

  BiNormal = Tangent;
  BiNormal.Cross(Normal);
  DBiNormal.SetLinearForm(DTangent.Crossed(Normal), Tangent.Crossed(DNormal));
  D2BiNormal.SetLinearForm(1,
                           D2Tangent.Crossed(Normal),
                           2,
                           DTangent.Crossed(DNormal),
                           Tangent.Crossed(D2Normal));

  return Standard_True;
}

//=================================================================================================

Handle(GeomFill_TrihedronLaw) GeomFill_DraftTrihedron::Copy() const
{
  Handle(GeomFill_DraftTrihedron) copy = new (GeomFill_DraftTrihedron)(B, myAngle - M_PI / 2);
  copy->SetCurve(myCurve);
  return copy;
}

//=================================================================================================

Standard_Integer GeomFill_DraftTrihedron::NbIntervals(const GeomAbs_Shape S) const
{
  GeomAbs_Shape tmpS = GeomAbs_C0;
  switch (S)
  {
    case GeomAbs_C0:
      tmpS = GeomAbs_C2;
      break;
    case GeomAbs_C1:
      tmpS = GeomAbs_C3;
      break;
    case GeomAbs_C2:
    case GeomAbs_C3:
    case GeomAbs_CN:
      tmpS = GeomAbs_CN;
      break;
    default:
      throw Standard_OutOfRange();
  }

  return myCurve->NbIntervals(tmpS);
}

//=================================================================================================

void GeomFill_DraftTrihedron::Intervals(TColStd_Array1OfReal& TT, const GeomAbs_Shape S) const
{
  GeomAbs_Shape tmpS = GeomAbs_C0;
  switch (S)
  {
    case GeomAbs_C0:
      tmpS = GeomAbs_C2;
      break;
    case GeomAbs_C1:
      tmpS = GeomAbs_C3;
      break;
    case GeomAbs_C2:
    case GeomAbs_C3:
    case GeomAbs_CN:
      tmpS = GeomAbs_CN;
      break;
    default:
      throw Standard_OutOfRange();
  }

  myCurve->Intervals(TT, tmpS);
}

//=================================================================================================

void GeomFill_DraftTrihedron::GetAverageLaw(Vector3d& ATangent, Vector3d& ANormal, Vector3d& ABiNormal)
{
  Standard_Integer Num = 20; // order of digitalization
  Vector3d           T, N, BN;
  ATangent  = Vector3d(0, 0, 0);
  ANormal   = Vector3d(0, 0, 0);
  ABiNormal = Vector3d(0, 0, 0);

  Standard_Real Step = (myTrimmed->LastParameter() - myTrimmed->FirstParameter()) / Num;
  Standard_Real Param;
  for (Standard_Integer i = 0; i <= Num; i++)
  {
    Param = myTrimmed->FirstParameter() + i * Step;
    if (Param > myTrimmed->LastParameter())
      Param = myTrimmed->LastParameter();
    D0(Param, T, N, BN);
    ATangent += T;
    ANormal += N;
    ABiNormal += BN;
  }

  ANormal /= Num + 1;
  ABiNormal /= Num + 1;
  ATangent /= Num + 1;
}

//=================================================================================================

Standard_Boolean GeomFill_DraftTrihedron::IsConstant() const
{
  return (myCurve->GetType() == GeomAbs_Line);
}

//=================================================================================================

Standard_Boolean GeomFill_DraftTrihedron::IsOnlyBy3dCurve() const
{
  GeomAbs_CurveType TheType = myCurve->GetType();
  Axis3d            TheAxe;

  switch (TheType)
  {
    case GeomAbs_Circle: {
      TheAxe = myCurve->Circle().Axis();
      break;
    }
    case GeomAbs_Ellipse: {
      TheAxe = myCurve->Ellipse().Axis();
      break;
    }
    case GeomAbs_Hyperbola: {
      TheAxe = myCurve->Hyperbola().Axis();
      break;
    }
    case GeomAbs_Parabola: {
      TheAxe = myCurve->Parabola().Axis();
      break;
    }
    case GeomAbs_Line: { // La normale du plan de la courbe est il perpendiculaire a la BiNormale ?
      Vector3d V;
      V.SetXYZ(myCurve->Line().Direction().XYZ());
      return V.IsParallel(B, Precision1::Angular());
    }
    default:
      return Standard_False; // pas de risques
  }

  // La normale du plan de la courbe est il // a la BiNormale ?
  Vector3d V;
  V.SetXYZ(TheAxe.Direction().XYZ());
  return V.IsParallel(B, Precision1::Angular());
}
