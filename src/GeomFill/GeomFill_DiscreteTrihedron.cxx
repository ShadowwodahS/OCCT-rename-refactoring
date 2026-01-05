// Created on: 2013-02-05
// Created by: Julia GERASIMOVA
// Copyright (c) 2001-2013 OPEN CASCADE SAS
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

#include <GeomFill_DiscreteTrihedron.hxx>

#include <Adaptor3d_Curve.hxx>
#include <GeomAbs_CurveType.hxx>
#include <GeomFill_Frenet.hxx>
#include <GeomFill_HSequenceOfAx2.hxx>
#include <GeomFill_TrihedronLaw.hxx>
#include <gp_Vec.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_Type.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HSequenceOfReal.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomFill_DiscreteTrihedron, GeomFill_TrihedronLaw)

static const Standard_Real TolConf = Precision::Confusion();

//=================================================================================================

GeomFill_DiscreteTrihedron::GeomFill_DiscreteTrihedron()
    : myUseFrenet(Standard_False)
{
  myFrenet     = new GeomFill_Frenet();
  myKnots      = new TColStd_HSequenceOfReal();
  myTrihedrons = new GeomFill_HSequenceOfAx2();
}

//=================================================================================================

Handle(GeomFill_TrihedronLaw) GeomFill_DiscreteTrihedron::Copy() const
{
  Handle(GeomFill_DiscreteTrihedron) copy = new (GeomFill_DiscreteTrihedron)();
  if (!myCurve.IsNull())
    copy->SetCurve(myCurve);
  return copy;
}

//=================================================================================================

Standard_Boolean GeomFill_DiscreteTrihedron::SetCurve(const Handle(Adaptor3d_Curve)& C)
{
  GeomFill_TrihedronLaw::SetCurve(C);
  if (!C.IsNull())
  {
    GeomAbs_CurveType type;
    type = C->GetType();
    switch (type)
    {
      case GeomAbs_Circle:
      case GeomAbs_Ellipse:
      case GeomAbs_Hyperbola:
      case GeomAbs_Parabola:
      case GeomAbs_Line: {
        // No problem
        myUseFrenet = Standard_True;
        myFrenet->SetCurve(C);
        break;
      }
      default: {
        myUseFrenet = Standard_False;
        // We have to fill <myKnots> and <myTrihedrons>
        Init();
        break;
      }
    }
  }
  return myUseFrenet;
}

//=================================================================================================

void GeomFill_DiscreteTrihedron::Init()
{
  Standard_Integer     NbIntervals = myTrimmed->NbIntervals(GeomAbs_CN);
  TColStd_Array1OfReal Knots(1, NbIntervals + 1);
  myTrimmed->Intervals(Knots, GeomAbs_CN);

  // Standard_Real Tol = Precision::Confusion();
  Standard_Integer NbSamples = 10;

  Standard_Integer i, j;
  for (i = 1; i <= NbIntervals; i++)
  {
    Standard_Real delta = (Knots(i + 1) - Knots(i)) / NbSamples;
    for (j = 0; j < NbSamples; j++)
    {
      Standard_Real Param = Knots(i) + j * delta;
      myKnots->Append(Param);
    }
  }
  myKnots->Append(Knots(NbIntervals + 1));

  Point3d        Origin(0., 0., 0.), Pnt, SubPnt;
  Vector3d        Tangent;
  Dir3d        TangDir;
  Standard_Real norm;
  for (i = 1; i <= myKnots->Length(); i++)
  {
    Standard_Real Param = myKnots->Value(i);
    myTrimmed->D1(Param, Pnt, Tangent);
    norm = Tangent.Magnitude();
    if (norm < TolConf)
    {
      Standard_Real subdelta = (myKnots->Value(i + 1) - myKnots->Value(i)) / NbSamples;
      if (subdelta < Precision::PConfusion())
        subdelta = myKnots->Value(i + 1) - myKnots->Value(i);
      SubPnt = myTrimmed->Value(Param + subdelta);
      Tangent.SetXYZ(SubPnt.XYZ() - Pnt.XYZ());
    }
    // Tangent.Normalize();
    TangDir = Tangent; // normalize;
    Tangent = TangDir;
    if (i == 1) // first point
    {
      Frame3d FirstAxis(Origin, TangDir);
      myTrihedrons->Append(FirstAxis);
    }
    else
    {
      Frame3d LastAxis       = myTrihedrons->Value(myTrihedrons->Length());
      Vector3d LastTangent    = LastAxis.Direction();
      Vector3d AxisOfRotation = LastTangent ^ Tangent;
      if (AxisOfRotation.Magnitude() <= gp::Resolution()) // tangents are equal or opposite
      {
        Standard_Real ScalarProduct = LastTangent * Tangent;
        if (ScalarProduct > 0.) // tangents are equal
          myTrihedrons->Append(LastAxis);
        else // tangents are opposite
        {
          Standard_Real NewParam = (myKnots->Value(i - 1) + myKnots->Value(i)) / 2.;
          if (NewParam - myKnots->Value(i - 1) < gp::Resolution())
            throw Standard_ConstructionError(
              "GeomFill_DiscreteTrihedron : impassable singularities on path curve");
          myKnots->InsertBefore(i, NewParam);
          i--;
        }
      }
      else // good value of angle
      {
        Standard_Real theAngle = LastTangent.AngleWithRef(Tangent, AxisOfRotation);
        Axis3d        theAxisOfRotation(Origin, AxisOfRotation);
        Frame3d        NewAxis = LastAxis.Rotated(theAxisOfRotation, theAngle);
        NewAxis.SetDirection(TangDir); // to prevent accumulation of floating computations error
        myTrihedrons->Append(NewAxis);
      }
    }
  }
}

//=================================================================================================

Standard_Boolean GeomFill_DiscreteTrihedron::D0(const Standard_Real Param,
                                                Vector3d&             Tangent,
                                                Vector3d&             Normal,
                                                Vector3d&             BiNormal)
{
  if (myUseFrenet)
  {
    myFrenet->D0(Param, Tangent, Normal, BiNormal);
  }
  else
  {
    // Locate <Param> in the sequence <myKnots>
    Standard_Integer        Index  = -1;
    constexpr Standard_Real TolPar = Precision::PConfusion();
    // Standard_Real TolConf = Precision::Confusion();
    Standard_Integer NbSamples = 10;
    Point3d           Origin(0., 0., 0.);

    Standard_Integer i;
    // Frame3d PrevAxis;
    // Standard_Real PrevParam;

    Standard_Integer I1, I2;
    I1 = 1;
    I2 = myKnots->Length();
    for (;;)
    {
      i = (I1 + I2) / 2;
      if (Param <= myKnots->Value(i))
        I2 = i;
      else
        I1 = i;
      if (I2 - I1 <= 1)
        break;
    }
    Index = I1;
    if (Abs(Param - myKnots->Value(I2)) < TolPar)
      Index = I2;

    Standard_Real PrevParam = myKnots->Value(Index);
    Frame3d        PrevAxis  = myTrihedrons->Value(Index);
    Frame3d        theAxis;
    if (Abs(Param - PrevParam) < TolPar)
      theAxis = PrevAxis;
    else //<Param> is between knots
    {
      myTrimmed->D1(Param, myPoint, Tangent);
      Standard_Real norm = Tangent.Magnitude();
      if (norm < TolConf)
      {
        Standard_Real subdelta = (myKnots->Value(Index + 1) - Param) / NbSamples;
        if (subdelta < Precision::PConfusion())
          subdelta = myKnots->Value(Index + 1) - Param;
        Point3d SubPnt = myTrimmed->Value(Param + subdelta);
        Tangent.SetXYZ(SubPnt.XYZ() - myPoint.XYZ());
      }
      // Tangent.Normalize();
      Dir3d TangDir(Tangent); // normalize;
      Tangent               = TangDir;
      Vector3d PrevTangent    = PrevAxis.Direction();
      Vector3d AxisOfRotation = PrevTangent ^ Tangent;
      if (AxisOfRotation.Magnitude() <= gp::Resolution()) // tangents are equal
      {
        // we assume that tangents can not be opposite
        theAxis = PrevAxis;
      }
      else // good value of angle
      {
        Standard_Real theAngle = PrevTangent.AngleWithRef(Tangent, AxisOfRotation);
        Axis3d        theAxisOfRotation(Origin, AxisOfRotation);
        theAxis = PrevAxis.Rotated(theAxisOfRotation, theAngle);
      }
      theAxis.SetDirection(TangDir); // to prevent accumulation of floating computations error
    } // end of else (Param is between knots)

    Tangent  = theAxis.Direction();
    Normal   = theAxis.XDirection();
    BiNormal = theAxis.YDirection();
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean GeomFill_DiscreteTrihedron::D1(const Standard_Real Param,
                                                Vector3d&             Tangent,
                                                Vector3d&             DTangent,
                                                Vector3d&             Normal,
                                                Vector3d&             DNormal,
                                                Vector3d&             BiNormal,
                                                Vector3d&             DBiNormal)
{
  if (myUseFrenet)
  {
    myFrenet->D1(Param, Tangent, DTangent, Normal, DNormal, BiNormal, DBiNormal);
  }
  else
  {
    D0(Param, Tangent, Normal, BiNormal);

    DTangent.SetCoord(0., 0., 0.);
    DNormal.SetCoord(0., 0., 0.);
    DBiNormal.SetCoord(0., 0., 0.);
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean GeomFill_DiscreteTrihedron::D2(const Standard_Real Param,
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
  if (myUseFrenet)
  {
    myFrenet->D2(Param,
                 Tangent,
                 DTangent,
                 D2Tangent,
                 Normal,
                 DNormal,
                 D2Normal,
                 BiNormal,
                 DBiNormal,
                 D2BiNormal);
  }
  else
  {
    D0(Param, Tangent, Normal, BiNormal);

    DTangent.SetCoord(0., 0., 0.);
    DNormal.SetCoord(0., 0., 0.);
    DBiNormal.SetCoord(0., 0., 0.);
    D2Tangent.SetCoord(0., 0., 0.);
    D2Normal.SetCoord(0., 0., 0.);
    D2BiNormal.SetCoord(0., 0., 0.);
  }
  return Standard_True;
}

//=================================================================================================

Standard_Integer GeomFill_DiscreteTrihedron::NbIntervals(const GeomAbs_Shape) const
{
  return (myTrimmed->NbIntervals(GeomAbs_CN));
}

//=================================================================================================

void GeomFill_DiscreteTrihedron::Intervals(TColStd_Array1OfReal& T, const GeomAbs_Shape) const
{
  myTrimmed->Intervals(T, GeomAbs_CN);
}

void GeomFill_DiscreteTrihedron::GetAverageLaw(Vector3d& ATangent, Vector3d& ANormal, Vector3d& ABiNormal)
{
  Standard_Integer Num = 20; // order of digitalization
  Vector3d           T, N, BN;
  ATangent           = Vector3d(0, 0, 0);
  ANormal            = Vector3d(0, 0, 0);
  ABiNormal          = Vector3d(0, 0, 0);
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
  ATangent /= Num + 1;
  ANormal /= Num + 1;

  ATangent.Normalize();
  ABiNormal = ATangent.Crossed(ANormal).Normalized();
  ANormal   = ABiNormal.Crossed(ATangent);
}

//=================================================================================================

Standard_Boolean GeomFill_DiscreteTrihedron::IsConstant() const
{
  return (myCurve->GetType() == GeomAbs_Line);
}

//=================================================================================================

Standard_Boolean GeomFill_DiscreteTrihedron::IsOnlyBy3dCurve() const
{
  return Standard_True;
}
