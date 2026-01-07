// Created on: 2004-05-20
// Created by: Sergey ZARITCHNY
// Copyright (c) 2004-2014 OPEN CASCADE SAS
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

#include <BinTools.hxx>
#include <BinTools_CurveSet.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_Line.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Lin.hxx>
#include <gp_Parab.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Message_ProgressScope.hxx>

#define LINE 1
#define CIRCLE 2
#define ELLIPSE 3
#define PARABOLA 4
#define HYPERBOLA 5
#define BEZIER 6
#define BSPLINE 7
#define TRIMMED 8
#define OFFSET 9

//=================================================================================================

CurveBinarySet::CurveBinarySet() {}

//=================================================================================================

void CurveBinarySet::Clear()
{
  myMap.Clear();
}

//=================================================================================================

Standard_Integer CurveBinarySet::Add(const Handle(GeomCurve3d)& C)
{
  return (C.IsNull()) ? 0 : myMap.Add(C);
}

//=================================================================================================

Handle(GeomCurve3d) CurveBinarySet::Curve(const Standard_Integer I) const
{
  if (I == 0)
  {
    Handle(GeomCurve3d) dummy;
    return dummy;
  }
  else
    return Handle(GeomCurve3d)::DownCast(myMap(I));
}

//=================================================================================================

Standard_Integer CurveBinarySet::Index(const Handle(GeomCurve3d)& S) const
{
  return S.IsNull() ? 0 : myMap.FindIndex(S);
}

//=======================================================================
// function : operator << (GeomLine)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(GeomLine)& L)
{
  OS << (Standard_Byte)LINE;
  gp_Lin C = L->Lin();
  OS << C.Location();  // Pnt
  OS << C.Direction(); // Dir
  return OS;
}

//=======================================================================
// function :  operator <<(GeomCircle)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(GeomCircle)& CC)
{
  OS << (Standard_Byte)CIRCLE;
  gp_Circ C = CC->Circ();
  OS << C.Location();
  OS << C.Axis().Direction();
  OS << C.XAxis().Direction();
  OS << C.YAxis().Direction();
  OS << C.Radius();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_Ellipse)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_Ellipse)& E)
{
  OS << (Standard_Byte)ELLIPSE;
  gp_Elips C = E->Elips();
  OS << C.Location();
  OS << C.Axis().Direction();
  OS << C.XAxis().Direction();
  OS << C.YAxis().Direction();
  OS << C.MajorRadius();
  OS << C.MinorRadius();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_Parabola)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_Parabola)& P)
{
  OS << (Standard_Byte)PARABOLA;
  gp_Parab C = P->Parab();
  OS << C.Location();
  OS << C.Axis().Direction();
  OS << C.XAxis().Direction();
  OS << C.YAxis().Direction();
  OS << C.Focal();
  return OS;
}

//=======================================================================
// function : operator <<(Geom_Hyperbola)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_Hyperbola)& H)
{
  OS << (Standard_Byte)HYPERBOLA;
  gp_Hypr C = H->Hypr();
  OS << C.Location();
  OS << C.Axis().Direction();
  OS << C.XAxis().Direction();
  OS << C.YAxis().Direction();
  OS << C.MajorRadius();
  OS << C.MinorRadius();
  return OS;
}

//=======================================================================
// function : operator <<(BezierCurve3d)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(BezierCurve3d)& B)
{
  OS << (Standard_Byte)BEZIER;
  Standard_Boolean aRational = B->IsRational() ? 1 : 0;
  OS << aRational; // rational
  // poles and weights
  Standard_Integer i, aDegree = B->Degree();
  OS << (Standard_ExtCharacter)aDegree; //<< Degree
  for (i = 1; i <= aDegree + 1; i++)
  {
    OS << B->Pole(i); // Pnt
    if (aRational)
      OS << B->Weight(i); // Real
  }
  return OS;
}

//=======================================================================
// function : operator <<(BSplineCurve3d)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(BSplineCurve3d)& B)
{
  OS << (Standard_Byte)BSPLINE;
  Standard_Boolean aRational = B->IsRational() ? 1 : 0;
  OS << aRational; // rational
  Standard_Boolean aPeriodic = B->IsPeriodic() ? 1 : 0;
  OS << aPeriodic; // periodic
  // poles and weights
  Standard_Integer i, aDegree, aNbPoles, aNbKnots;
  aDegree  = B->Degree();
  aNbPoles = B->NbPoles();
  aNbKnots = B->NbKnots();
  OS << (Standard_ExtCharacter)aDegree;
  OS << aNbPoles;
  OS << aNbKnots;
  for (i = 1; i <= aNbPoles; i++)
  {
    OS << B->Pole(i); // Pnt
    if (aRational)
      OS << B->Weight(i);
  }

  for (i = 1; i <= aNbKnots; i++)
  {
    OS << B->Knot(i);
    OS << B->Multiplicity(i);
  }
  return OS;
}

//=======================================================================
// function : operator <<(Geom_TrimmedCurve)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_TrimmedCurve)& C)
{
  OS << (Standard_Byte)TRIMMED;
  OS << C->FirstParameter();
  OS << C->LastParameter();
  CurveBinarySet::WriteCurve(C->BasisCurve(), OS);
  return OS;
}

//=======================================================================
// function : operator <<(Geom_OffsetCurve)
// purpose  :
//=======================================================================

static BinaryOutputStream& operator<<(BinaryOutputStream& OS, const Handle(Geom_OffsetCurve)& C)
{
  OS << (Standard_Byte)OFFSET;
  OS << C->Offset(); // Offset
  OS << C->Direction();
  CurveBinarySet::WriteCurve(C->BasisCurve(), OS);
  return OS;
}

//=================================================================================================

void CurveBinarySet::WriteCurve(const Handle(GeomCurve3d)& C, BinaryOutputStream& OS)
{
  Handle(TypeInfo) TheType = C->DynamicType();
  try
  {
    OCC_CATCH_SIGNALS
    if (TheType == STANDARD_TYPE(GeomLine))
    {
      OS << Handle(GeomLine)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(GeomCircle))
    {
      OS << Handle(GeomCircle)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(Geom_Ellipse))
    {
      OS << Handle(Geom_Ellipse)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(Geom_Parabola))
    {
      OS << Handle(Geom_Parabola)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(Geom_Hyperbola))
    {
      OS << Handle(Geom_Hyperbola)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(BezierCurve3d))
    {
      OS << Handle(BezierCurve3d)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(BSplineCurve3d))
    {
      OS << Handle(BSplineCurve3d)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(Geom_TrimmedCurve))
    {
      OS << Handle(Geom_TrimmedCurve)::DownCast(C);
    }
    else if (TheType == STANDARD_TYPE(Geom_OffsetCurve))
    {
      OS << Handle(Geom_OffsetCurve)::DownCast(C);
    }
    else
    {
      throw ExceptionBase("UNKNOWN CURVE TYPE");
    }
  }
  catch (ExceptionBase const& anException)
  {
    Standard_SStream aMsg;
    aMsg << "EXCEPTION in CurveBinarySet::WriteCurve(..)" << std::endl;
    aMsg << anException << std::endl;
    throw ExceptionBase(aMsg.str().c_str());
  }
}

//=================================================================================================

void CurveBinarySet::Write(Standard_OStream& OS, const Message_ProgressRange& theRange) const
{
  Standard_Integer      i, nbcurv = myMap.Extent();
  Message_ProgressScope aPS(theRange, "Writing curves", nbcurv);
  OS << "Curves " << nbcurv << "\n";
  BinaryOutputStream aStream(OS);
  for (i = 1; i <= nbcurv && aPS.More(); i++, aPS.Next())
  {
    WriteCurve(Handle(GeomCurve3d)::DownCast(myMap(i)), aStream);
  }
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Point3d& P)
{
  Standard_Real X = 0., Y = 0., Z = 0.;
  BinTools1::GetReal(IS, X);
  BinTools1::GetReal(IS, Y);
  BinTools1::GetReal(IS, Z);
  P.SetCoord(X, Y, Z);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Dir3d& D)
{
  Standard_Real X = 0., Y = 0., Z = 0.;
  BinTools1::GetReal(IS, X);
  BinTools1::GetReal(IS, Y);
  BinTools1::GetReal(IS, Z);
  D.SetCoord(X, Y, Z);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(GeomLine)& L)
{
  Point3d P(0., 0., 0.);
  Dir3d AX(1., 0., 0.);
  IS >> P >> AX;
  L = new GeomLine(P, AX);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(GeomCircle)& C)
{
  Point3d        P(0., 0., 0.);
  Dir3d        A(1., 0., 0.), AX(1., 0., 0.), AY(1., 0., 0.);
  Standard_Real R = 0.;
  IS >> P >> A >> AX >> AY;
  BinTools1::GetReal(IS, R);
  C = new GeomCircle(Frame3d(P, A, AX), R);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_Ellipse)& E)
{
  Point3d        P(0., 0., 0.);
  Dir3d        A(1., 0., 0.), AX(1., 0., 0.), AY(1., 0., 0.);
  Standard_Real R1 = 0., R2 = 0.;
  IS >> P >> A >> AX >> AY;
  BinTools1::GetReal(IS, R1);
  BinTools1::GetReal(IS, R2);
  E = new Geom_Ellipse(Frame3d(P, A, AX), R1, R2);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_Parabola)& C)
{
  Point3d        P(0., 0., 0.);
  Dir3d        A(1., 0., 0.), AX(1., 0., 0.), AY(1., 0., 0.);
  Standard_Real R1 = 0.;
  IS >> P >> A >> AX >> AY;
  BinTools1::GetReal(IS, R1);
  C = new Geom_Parabola(Frame3d(P, A, AX), R1);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_Hyperbola)& H)
{
  Point3d        P(0., 0., 0.);
  Dir3d        A(1., 0., 0.), AX(1., 0., 0.), AY(1., 0., 0);
  Standard_Real R1 = 0., R2 = 0.;
  IS >> P >> A >> AX >> AY;
  BinTools1::GetReal(IS, R1);
  BinTools1::GetReal(IS, R2);
  H = new Geom_Hyperbola(Frame3d(P, A, AX), R1, R2);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(BezierCurve3d)& B)
{
  Standard_Boolean rational = Standard_False;
  BinTools1::GetBool(IS, rational);

  // poles and weights
  Standard_Integer i = 0, degree = 0;
  // degree;
  Standard_ExtCharacter aVal = '\0';
  BinTools1::GetExtChar(IS, aVal);
  degree = (Standard_Integer)aVal;

  TColgp_Array1OfPnt   poles(1, degree + 1);
  TColStd_Array1OfReal weights(1, degree + 1);

  for (i = 1; i <= degree + 1; i++)
  {
    IS >> poles(i); // Pnt
    if (rational)
      // weights(i);
      BinTools1::GetReal(IS, weights(i));
  }

  if (rational)
    B = new BezierCurve3d(poles, weights);
  else
    B = new BezierCurve3d(poles);

  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(BSplineCurve3d)& B)
{

  Standard_Boolean rational = Standard_False, periodic = Standard_False;
  BinTools1::GetBool(IS, rational);
  BinTools1::GetBool(IS, periodic);

  // poles and weights
  Standard_Integer      i = 0, degree = 0, nbpoles = 0, nbknots = 0;
  Standard_ExtCharacter aVal = '\0';
  BinTools1::GetExtChar(IS, aVal);
  degree = (Standard_Integer)aVal;

  BinTools1::GetInteger(IS, nbpoles);

  BinTools1::GetInteger(IS, nbknots);

  TColgp_Array1OfPnt   poles(1, nbpoles);
  TColStd_Array1OfReal weights(1, nbpoles);

  for (i = 1; i <= nbpoles; i++)
  {
    IS >> poles(i); // Pnt
    if (rational)
      BinTools1::GetReal(IS, weights(i));
  }

  TColStd_Array1OfReal    knots(1, nbknots);
  TColStd_Array1OfInteger mults(1, nbknots);

  for (i = 1; i <= nbknots; i++)
  {
    BinTools1::GetReal(IS, knots(i));
    BinTools1::GetInteger(IS, mults(i));
  }

  if (rational)
    B = new BSplineCurve3d(poles, weights, knots, mults, degree, periodic);
  else
    B = new BSplineCurve3d(poles, knots, mults, degree, periodic);

  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_TrimmedCurve)& C)
{
  Standard_Real p1 = 0., p2 = 0.;
  BinTools1::GetReal(IS, p1); // FirstParameter
  BinTools1::GetReal(IS, p2); // LastParameter
  Handle(GeomCurve3d) BC;
  CurveBinarySet::ReadCurve(IS, BC);
  C = new Geom_TrimmedCurve(BC, p1, p2);
  return IS;
}

//=================================================================================================

static Standard_IStream& operator>>(Standard_IStream& IS, Handle(Geom_OffsetCurve)& C)
{
  Standard_Real p = 0.;
  BinTools1::GetReal(IS, p); // Offset
  Dir3d D(1., 0., 0.);
  IS >> D;
  Handle(GeomCurve3d) BC;
  CurveBinarySet::ReadCurve(IS, BC);
  C = new Geom_OffsetCurve(BC, p, D);
  return IS;
}

//=================================================================================================

Standard_IStream& CurveBinarySet::ReadCurve(Standard_IStream& IS, Handle(GeomCurve3d)& C)
{
  try
  {
    OCC_CATCH_SIGNALS
    const Standard_Byte ctype = (Standard_Byte)IS.get();

    switch (ctype)
    {

      case LINE: {
        Handle(GeomLine) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case CIRCLE: {
        Handle(GeomCircle) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case ELLIPSE: {
        Handle(Geom_Ellipse) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case PARABOLA: {
        Handle(Geom_Parabola) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case HYPERBOLA: {
        Handle(Geom_Hyperbola) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case BEZIER: {
        Handle(BezierCurve3d) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case BSPLINE: {
        Handle(BSplineCurve3d) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case TRIMMED: {
        Handle(Geom_TrimmedCurve) CC;
        IS >> CC;
        C = CC;
      }
      break;

      case OFFSET: {
        Handle(Geom_OffsetCurve) CC;
        IS >> CC;
        C = CC;
      }
      break;

      default: {
        C = NULL;
        throw ExceptionBase("UNKNOWN CURVE TYPE");
      }
    }
  }
  catch (ExceptionBase const& anException)
  {
    C = NULL;
    Standard_SStream aMsg;
    aMsg << "EXCEPTION in CurveBinarySet::ReadCurve(..)" << std::endl;
    aMsg << anException << std::endl;
    throw ExceptionBase(aMsg.str().c_str());
  }
  return IS;
}

//=================================================================================================

void CurveBinarySet::Read(Standard_IStream& IS, const Message_ProgressRange& theRange)
{
  char buffer[255];
  IS >> buffer;
  if (IS.fail() || strcmp(buffer, "Curves"))
  {
    Standard_SStream aMsg;
    aMsg << "CurveBinarySet::Read:  Not a Curve table" << std::endl;
#ifdef OCCT_DEBUG
    std::cout << "CurveSet buffer: " << buffer << std::endl;
#endif
    throw ExceptionBase(aMsg.str().c_str());
    return;
  }

  Handle(GeomCurve3d) C;
  Standard_Integer   i, nbcurve;
  IS >> nbcurve;

  Message_ProgressScope aPS(theRange, "Reading curves", nbcurve);

  IS.get(); // remove <lf>
  for (i = 1; i <= nbcurve && aPS.More(); i++, aPS.Next())
  {
    CurveBinarySet::ReadCurve(IS, C);
    myMap.Add(C);
  }
}
