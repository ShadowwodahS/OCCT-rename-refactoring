// Created on: 1991-06-25
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

// 24-Aug-95 : xab removed C1 and C2 test : appeller  D1 et D2
//             avec discernement !
// 19-09-97  : JPI correction derivee seconde

#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Geometry.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_UndefinedDerivative.hxx>
#include <gp.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_NotImplemented.hxx>
#include <Standard_RangeError.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Geom_OffsetCurve, GeomCurve3d)

static const Standard_Real MyAngularToleranceForG1 = Precision1::Angular();

//=================================================================================================

Handle(Geometry3) Geom_OffsetCurve::Copy() const
{

  Handle(Geom_OffsetCurve) C;
  C = new Geom_OffsetCurve(basisCurve, offsetValue, direction);
  return C;
}

//=======================================================================
// function : Geom_OffsetCurve
// purpose  : Basis curve cannot be an Offset curve or trimmed from
//            offset curve.
//=======================================================================

Geom_OffsetCurve::Geom_OffsetCurve(const Handle(GeomCurve3d)& theCurve,
                                   const Standard_Real       theOffset,
                                   const Dir3d&             theDir,
                                   const Standard_Boolean    isTheNotCheckC0)
    : direction(theDir),
      offsetValue(theOffset)
{
  SetBasisCurve(theCurve, isTheNotCheckC0);
}

//=================================================================================================

void Geom_OffsetCurve::Reverse()
{
  basisCurve->Reverse();
  offsetValue = -offsetValue;
  myEvaluator->SetOffsetValue(offsetValue);
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::ReversedParameter(const Standard_Real U) const
{
  return basisCurve->ReversedParameter(U);
}

//=================================================================================================

const Dir3d& Geom_OffsetCurve::Direction() const
{
  return direction;
}

//=================================================================================================

void Geom_OffsetCurve::SetDirection(const Dir3d& V)
{
  direction = V;
  myEvaluator->SetOffsetDirection(direction);
}

//=================================================================================================

void Geom_OffsetCurve::SetOffsetValue(const Standard_Real D)
{
  offsetValue = D;
  myEvaluator->SetOffsetValue(offsetValue);
}

//=================================================================================================

Standard_Boolean Geom_OffsetCurve::IsPeriodic() const
{
  return basisCurve->IsPeriodic();
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::Period() const
{
  return basisCurve->Period();
}

//=================================================================================================

void Geom_OffsetCurve::SetBasisCurve(const Handle(GeomCurve3d)& C,
                                     const Standard_Boolean    isNotCheckC0)
{
  const Standard_Real aUf = C->FirstParameter(), aUl = C->LastParameter();
  Handle(GeomCurve3d)  aCheckingCurve = Handle(GeomCurve3d)::DownCast(C->Copy());
  Standard_Boolean    isTrimmed      = Standard_False;

  while (aCheckingCurve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve))
         || aCheckingCurve->IsKind(STANDARD_TYPE(Geom_OffsetCurve)))
  {
    if (aCheckingCurve->IsKind(STANDARD_TYPE(Geom_TrimmedCurve)))
    {
      Handle(Geom_TrimmedCurve) aTrimC = Handle(Geom_TrimmedCurve)::DownCast(aCheckingCurve);
      aCheckingCurve                   = aTrimC->BasisCurve();
      isTrimmed                        = Standard_True;
    }

    if (aCheckingCurve->IsKind(STANDARD_TYPE(Geom_OffsetCurve)))
    {
      Handle(Geom_OffsetCurve) aOC = Handle(Geom_OffsetCurve)::DownCast(aCheckingCurve);
      aCheckingCurve               = aOC->BasisCurve();
      Standard_Real PrevOff        = aOC->Offset();
      Vector3d        V1(aOC->Direction());
      Vector3d        V2(direction);
      Vector3d        Vdir(PrevOff * V1 + offsetValue * V2);

      if (offsetValue >= 0.)
      {
        offsetValue = Vdir.Magnitude();
        direction.SetXYZ(Vdir.XYZ());
      }
      else
      {
        offsetValue = -Vdir.Magnitude();
        direction.SetXYZ((-Vdir).XYZ());
      }
    }
  }

  myBasisCurveContinuity = aCheckingCurve->Continuity();

  Standard_Boolean isC0 = !isNotCheckC0 && (myBasisCurveContinuity == GeomAbs_C0);

  // Basis curve must be at least C1
  if (isC0 && aCheckingCurve->IsKind(STANDARD_TYPE(BSplineCurve3d)))
  {
    Handle(BSplineCurve3d) aBC = Handle(BSplineCurve3d)::DownCast(aCheckingCurve);
    if (aBC->IsG1(aUf, aUl, MyAngularToleranceForG1))
    {
      // Checking if basis curve has more smooth (C1, G2 and above) is not done.
      // It can be done in case of need.
      myBasisCurveContinuity = GeomAbs_G1;
      isC0                   = Standard_False;
    }

    // Raise exception if still C0
    if (isC0)
      throw Standard_ConstructionError("Offset on C0 curve");
  }
  //
  if (isTrimmed)
  {
    basisCurve = new Geom_TrimmedCurve(aCheckingCurve, aUf, aUl);
  }
  else
  {
    basisCurve = aCheckingCurve;
  }

  myEvaluator = new GeomEvaluator_OffsetCurve(basisCurve, offsetValue, direction);
}

//=================================================================================================

Handle(GeomCurve3d) Geom_OffsetCurve::BasisCurve() const
{
  return basisCurve;
}

//=================================================================================================

GeomAbs_Shape Geom_OffsetCurve::Continuity() const
{

  GeomAbs_Shape OffsetShape = GeomAbs_C0;
  switch (myBasisCurveContinuity)
  {
    case GeomAbs_C0:
      OffsetShape = GeomAbs_C0;
      break;
    case GeomAbs_C1:
      OffsetShape = GeomAbs_C0;
      break;
    case GeomAbs_C2:
      OffsetShape = GeomAbs_C1;
      break;
    case GeomAbs_C3:
      OffsetShape = GeomAbs_C2;
      break;
    case GeomAbs_CN:
      OffsetShape = GeomAbs_CN;
      break;
    case GeomAbs_G1:
      OffsetShape = GeomAbs_G1;
      break;
    case GeomAbs_G2:
      OffsetShape = GeomAbs_G2;
      break;
  }
  return OffsetShape;
}

//=================================================================================================

void Geom_OffsetCurve::D0(const Standard_Real U, Point3d& P) const
{
  myEvaluator->D0(U, P);
}

//=================================================================================================

void Geom_OffsetCurve::D1(const Standard_Real U, Point3d& P, Vector3d& V1) const
{
  myEvaluator->D1(U, P, V1);
}

//=================================================================================================

void Geom_OffsetCurve::D2(const Standard_Real U, Point3d& P, Vector3d& V1, Vector3d& V2) const
{
  myEvaluator->D2(U, P, V1, V2);
}

//=================================================================================================

void Geom_OffsetCurve::D3(const Standard_Real theU,
                          Point3d&             theP,
                          Vector3d&             theV1,
                          Vector3d&             theV2,
                          Vector3d&             theV3) const
{
  myEvaluator->D3(theU, theP, theV1, theV2, theV3);
}

//=================================================================================================

Vector3d Geom_OffsetCurve::DN(const Standard_Real U, const Standard_Integer N) const
{
  Standard_RangeError_Raise_if(N < 1,
                               "Exception: "
                               "Geom_OffsetCurve::DN(...). N<1.");

  Vector3d VN, Vtemp;
  Point3d Ptemp;
  switch (N)
  {
    case 1:
      D1(U, Ptemp, VN);
      break;
    case 2:
      D2(U, Ptemp, Vtemp, VN);
      break;
    case 3:
      D3(U, Ptemp, Vtemp, Vtemp, VN);
      break;
    default:
      throw Standard_NotImplemented(
        "Exception: "
        "Derivative order is greater than 3. Cannot compute of derivative.");
  }

  return VN;
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::FirstParameter() const
{
  return basisCurve->FirstParameter();
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::LastParameter() const
{
  return basisCurve->LastParameter();
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::Offset() const
{
  return offsetValue;
}

//=================================================================================================

Standard_Boolean Geom_OffsetCurve::IsClosed() const
{
  Point3d PF, PL;
  D0(FirstParameter(), PF);
  D0(LastParameter(), PL);
  return (PF.Distance(PL) <= gp1::Resolution());
}

//=================================================================================================

Standard_Boolean Geom_OffsetCurve::IsCN(const Standard_Integer N) const
{

  Standard_RangeError_Raise_if(N < 0, " ");
  return basisCurve->IsCN(N + 1);
}

//=================================================================================================

void Geom_OffsetCurve::Transform(const Transform3d& T)
{
  basisCurve->Transform(T);
  direction.Transform(T);
  offsetValue *= T.ScaleFactor();

  myEvaluator->SetOffsetValue(offsetValue);
  myEvaluator->SetOffsetDirection(direction);
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::TransformedParameter(const Standard_Real U, const Transform3d& T) const
{
  return basisCurve->TransformedParameter(U, T);
}

//=================================================================================================

Standard_Real Geom_OffsetCurve::ParametricTransformation(const Transform3d& T) const
{
  return basisCurve->ParametricTransformation(T);
}

//=================================================================================================

GeomAbs_Shape Geom_OffsetCurve::GetBasisCurveContinuity() const
{
  return myBasisCurveContinuity;
}

//=================================================================================================

void Geom_OffsetCurve::DumpJson(Standard_OStream& theOStream, Standard_Integer theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_BASE_CLASS(theOStream, theDepth, GeomCurve3d)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, basisCurve.get())
  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &direction)

  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, offsetValue)
  OCCT_DUMP_FIELD_VALUE_NUMERICAL(theOStream, myBasisCurveContinuity)
}
