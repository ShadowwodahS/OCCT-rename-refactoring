// Created on: 1991-09-09
// Created by: Michel Chauvat
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

// Evolutions   JCV Dec 1991 ajout de calculs de derivees et traitement
//              d'entites 2d
//              JCV Mars 1992 ajout method SetLinearForm

#define No_Standard_OutOfRange

#include <ElCLib.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Ax22d.hxx>
#include <gp_Circ.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Elips.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Hypr.hxx>
#include <gp_Hypr2d.hxx>
#include <gp_Lin.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Parab.hxx>
#include <gp_Parab2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>

static Standard_Real PIPI = M_PI + M_PI;

//=======================================================================
// function : InPeriod
// purpose  : Value theULast is never returned.
//          Example of some case (checked on WIN64 platform)
//          with some surface having period 2*PI = 6.2831853071795862.
//            Let theUFirst be equal to 6.1645624650899675. Then,
//          theULast must be equal to
//              6.1645624650899675+6.2831853071795862=12.4477477722695537.
//
//          However, real result is 12.447747772269555.
//          Therefore, new period value to adjust will be equal to
//              12.447747772269555-6.1645624650899675=6.2831853071795871.
//
//          As we can see, (6.2831853071795871 != 6.2831853071795862).
//
//          According to above said, this method should be used carefully.
//          In order to increase reliability of this method, input arguments
//          needs to be replaced with following:
//            (theU, theUFirst, thePeriod). theULast parameter is excess.
//=======================================================================
Standard_Real ElCLib1::InPeriod(const Standard_Real theU,
                               const Standard_Real theUFirst,
                               const Standard_Real theULast)
{
  if (Precision::IsInfinite(theU) || Precision::IsInfinite(theUFirst)
      || Precision::IsInfinite(theULast))
  { // In order to avoid FLT_Overflow exception
    return theU;
  }

  const Standard_Real aPeriod = theULast - theUFirst;

  if (aPeriod < Epsilon(theULast))
    return theU;

  return Max(theUFirst, theU + aPeriod * Ceiling((theUFirst - theU) / aPeriod));
}

//=================================================================================================

void ElCLib1::AdjustPeriodic(const Standard_Real UFirst,
                            const Standard_Real ULast,
                            const Standard_Real Preci,
                            Standard_Real&      U1,
                            Standard_Real&      U2)
{
  if (Precision::IsInfinite(UFirst) || Precision::IsInfinite(ULast))
  {
    U1 = UFirst;
    U2 = ULast;
    return;
  }

  Standard_Real period = ULast - UFirst;

  if (period < Epsilon(ULast))
  {
    // In order to avoid FLT_Overflow exception
    // (test bugs moddata_1 bug22757)
    U1 = UFirst;
    U2 = ULast;
    return;
  }

  U1 -= Floor((U1 - UFirst) / period) * period;
  if (ULast - U1 < Preci)
    U1 -= period;
  U2 -= Floor((U2 - U1) / period) * period;
  if (U2 - U1 < Preci)
    U2 += period;
}

//=================================================================================================

Point3d ElCLib1::LineValue(const Standard_Real U, const Axis3d& Pos)
{
  const Coords3d& ZDir = Pos.Direction().XYZ();
  const Coords3d& PLoc = Pos.Location().XYZ();
  return Point3d(U * ZDir.X() + PLoc.X(), U * ZDir.Y() + PLoc.Y(), U * ZDir.Z() + PLoc.Z());
}

//=================================================================================================

Point3d ElCLib1::CircleValue(const Standard_Real U, const Frame3d& Pos, const Standard_Real Radius)
{
  const Coords3d& XDir = Pos.XDirection().XYZ();
  const Coords3d& YDir = Pos.YDirection().XYZ();
  const Coords3d& PLoc = Pos.Location().XYZ();
  Standard_Real A1   = Radius * cos(U);
  Standard_Real A2   = Radius * sin(U);
  return Point3d(A1 * XDir.X() + A2 * YDir.X() + PLoc.X(),
                A1 * XDir.Y() + A2 * YDir.Y() + PLoc.Y(),
                A1 * XDir.Z() + A2 * YDir.Z() + PLoc.Z());
}

//=================================================================================================

Point3d ElCLib1::EllipseValue(const Standard_Real U,
                            const Frame3d&       Pos,
                            const Standard_Real MajorRadius,
                            const Standard_Real MinorRadius)
{
  const Coords3d& XDir = Pos.XDirection().XYZ();
  const Coords3d& YDir = Pos.YDirection().XYZ();
  const Coords3d& PLoc = Pos.Location().XYZ();
  Standard_Real A1   = MajorRadius * cos(U);
  Standard_Real A2   = MinorRadius * sin(U);
  return Point3d(A1 * XDir.X() + A2 * YDir.X() + PLoc.X(),
                A1 * XDir.Y() + A2 * YDir.Y() + PLoc.Y(),
                A1 * XDir.Z() + A2 * YDir.Z() + PLoc.Z());
}

//=================================================================================================

Point3d ElCLib1::HyperbolaValue(const Standard_Real U,
                              const Frame3d&       Pos,
                              const Standard_Real MajorRadius,
                              const Standard_Real MinorRadius)
{
  const Coords3d& XDir = Pos.XDirection().XYZ();
  const Coords3d& YDir = Pos.YDirection().XYZ();
  const Coords3d& PLoc = Pos.Location().XYZ();
  Standard_Real A1   = MajorRadius * Cosh(U);
  Standard_Real A2   = MinorRadius * Sinh(U);
  return Point3d(A1 * XDir.X() + A2 * YDir.X() + PLoc.X(),
                A1 * XDir.Y() + A2 * YDir.Y() + PLoc.Y(),
                A1 * XDir.Z() + A2 * YDir.Z() + PLoc.Z());
}

//=================================================================================================

Point3d ElCLib1::ParabolaValue(const Standard_Real U, const Frame3d& Pos, const Standard_Real Focal)
{
  if (Focal == 0.0)
  {
    const Coords3d& XDir = Pos.XDirection().XYZ();
    const Coords3d& PLoc = Pos.Location().XYZ();
    return Point3d(U * XDir.X() + PLoc.X(), U * XDir.Y() + PLoc.Y(), U * XDir.Z() + PLoc.Z());
  }
  const Coords3d& XDir = Pos.XDirection().XYZ();
  const Coords3d& YDir = Pos.YDirection().XYZ();
  const Coords3d& PLoc = Pos.Location().XYZ();
  Standard_Real A1   = U * U / (4.0 * Focal);
  return Point3d(A1 * XDir.X() + U * YDir.X() + PLoc.X(),
                A1 * XDir.Y() + U * YDir.Y() + PLoc.Y(),
                A1 * XDir.Z() + U * YDir.Z() + PLoc.Z());
}

//=================================================================================================

void ElCLib1::LineD1(const Standard_Real U, const Axis3d& Pos, Point3d& P, Vector3d& V1)
{
  Coords3d Coord = Pos.Direction().XYZ();
  V1.SetXYZ(Coord);
  Coord.SetLinearForm(U, Coord, Pos.Location().XYZ());
  P.SetXYZ(Coord);
}

//=================================================================================================

void ElCLib1::CircleD1(const Standard_Real U,
                      const Frame3d&       Pos,
                      const Standard_Real Radius,
                      Point3d&             P,
                      Vector3d&             V1)
{
  Standard_Real Xc = Radius * Cos(U);
  Standard_Real Yc = Radius * Sin(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point courant :
  Coord0.SetLinearForm(Xc, Coord1, Yc, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(-Yc, Coord1, Xc, Coord2);
  V1.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::EllipseD1(const Standard_Real U,
                       const Frame3d&       Pos,
                       const Standard_Real MajorRadius,
                       const Standard_Real MinorRadius,
                       Point3d&             P,
                       Vector3d&             V1)
{
  Standard_Real Xc = Cos(U);
  Standard_Real Yc = Sin(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point courant :
  Coord0.SetLinearForm(Xc * MajorRadius, Coord1, Yc * MinorRadius, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(-Yc * MajorRadius, Coord1, Xc * MinorRadius, Coord2);
  V1.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::HyperbolaD1(const Standard_Real U,
                         const Frame3d&       Pos,
                         const Standard_Real MajorRadius,
                         const Standard_Real MinorRadius,
                         Point3d&             P,
                         Vector3d&             V1)
{
  Standard_Real Xc = Cosh(U);
  Standard_Real Yc = Sinh(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point courant :
  Coord0.SetLinearForm(Xc * MajorRadius, Coord1, Yc * MinorRadius, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(Yc * MajorRadius, Coord1, Xc * MinorRadius, Coord2);
  V1.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::ParabolaD1(const Standard_Real U,
                        const Frame3d&       Pos,
                        const Standard_Real Focal,
                        Point3d&             P,
                        Vector3d&             V1)
{
  Coords3d Coord0;
  Coords3d Coord1(Pos.XDirection().XYZ());
  if (Focal == 0.0)
  { // Parabole degenere en une droite
    V1.SetXYZ(Coord1);
    Coord1.Multiply(U);
    Coord1.Add(Pos.Location().XYZ());
    P.SetXYZ(Coord1);
  }
  else
  {
    Coords3d Coord2(Pos.YDirection().XYZ());
    Coord0.SetLinearForm(U / (2.0 * Focal), Coord1, Coord2);
    V1.SetXYZ(Coord0);
    Coord0.SetLinearForm((U * U) / (4.0 * Focal), Coord1, U, Coord2, Pos.Location().XYZ());
    P.SetXYZ(Coord0);
  }
}

//=================================================================================================

void ElCLib1::CircleD2(const Standard_Real U,
                      const Frame3d&       Pos,
                      const Standard_Real Radius,
                      Point3d&             P,
                      Vector3d&             V1,
                      Vector3d&             V2)
{
  Standard_Real Xc = Radius * cos(U);
  Standard_Real Yc = Radius * sin(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point courant :
  Coord0.SetLinearForm(Xc, Coord1, Yc, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(-Yc, Coord1, Xc, Coord2);
  V1.SetXYZ(Coord0);
  // D2 :
  Coord0.SetLinearForm(-Xc, Coord1, -Yc, Coord2);
  V2.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::EllipseD2(const Standard_Real U,
                       const Frame3d&       Pos,
                       const Standard_Real MajorRadius,
                       const Standard_Real MinorRadius,
                       Point3d&             P,
                       Vector3d&             V1,
                       Vector3d&             V2)
{
  Standard_Real Xc = cos(U);
  Standard_Real Yc = sin(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point courant :
  Coord0.SetLinearForm(Xc * MajorRadius, Coord1, Yc * MinorRadius, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(-Yc * MajorRadius, Coord1, Xc * MinorRadius, Coord2);
  V1.SetXYZ(Coord0);
  // D2 :
  Coord0.SetLinearForm(-Xc * MajorRadius, Coord1, -Yc * MinorRadius, Coord2);
  V2.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::HyperbolaD2(const Standard_Real U,
                         const Frame3d&       Pos,
                         const Standard_Real MajorRadius,
                         const Standard_Real MinorRadius,
                         Point3d&             P,
                         Vector3d&             V1,
                         Vector3d&             V2)
{
  Standard_Real Xc = Cosh(U);
  Standard_Real Yc = Sinh(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());

  // Point courant et D2:
  Coord0.SetLinearForm(Xc * MajorRadius, Coord1, Yc * MinorRadius, Coord2);
  V2.SetXYZ(Coord0);
  Coord0.Add(Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(Yc * MajorRadius, Coord1, Xc * MinorRadius, Coord2);
  V1.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::ParabolaD2(const Standard_Real U,
                        const Frame3d&       Pos,
                        const Standard_Real Focal,
                        Point3d&             P,
                        Vector3d&             V1,
                        Vector3d&             V2)
{
  Coords3d Coord0(0.0, 0.0, 0.0);
  Coords3d Coord1(Pos.XDirection().XYZ());
  if (Focal == 0.0)
  {
    V2.SetCoord(0.0, 0.0, 0.0);
    V1.SetXYZ(Coord1);
    Coord1.Multiply(U);
    Coord1.Add(Pos.Location().XYZ());
    P.SetXYZ(Coord1); // was: P.SetXYZ (Coord0);
  }
  else
  {
    Coords3d Coord2(Pos.YDirection().XYZ());
    Coord0.SetLinearForm((U * U) / (4.0 * Focal), Coord1, U, Coord2, Pos.Location().XYZ());
    P.SetXYZ(Coord0);
    Coord0.SetLinearForm(U / (2.0 * Focal), Coord1, Coord2);
    V1.SetXYZ(Coord0);
    Coord1.Multiply(1.0 / (2.0 * Focal));
    V2.SetXYZ(Coord1);
  }
}

//=================================================================================================

void ElCLib1::CircleD3(const Standard_Real U,
                      const Frame3d&       Pos,
                      const Standard_Real Radius,
                      Point3d&             P,
                      Vector3d&             V1,
                      Vector3d&             V2,
                      Vector3d&             V3)
{
  Standard_Real Xc = Radius * cos(U);
  Standard_Real Yc = Radius * sin(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point Courant :
  Coord0.SetLinearForm(Xc, Coord1, Yc, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(-Yc, Coord1, Xc, Coord2);
  V1.SetXYZ(Coord0);
  // D2 :
  Coord0.SetLinearForm(-Xc, Coord1, -Yc, Coord2);
  V2.SetXYZ(Coord0);
  // D3 :
  Coord0.SetLinearForm(Yc, Coord1, -Xc, Coord2);
  V3.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::EllipseD3(const Standard_Real U,
                       const Frame3d&       Pos,
                       const Standard_Real MajorRadius,
                       const Standard_Real MinorRadius,
                       Point3d&             P,
                       Vector3d&             V1,
                       Vector3d&             V2,
                       Vector3d&             V3)
{
  Standard_Real Xc = cos(U);
  Standard_Real Yc = sin(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point Courant :
  Coord0.SetLinearForm(Xc * MajorRadius, Coord1, Yc * MinorRadius, Coord2, Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 :
  Coord0.SetLinearForm(-Yc * MajorRadius, Coord1, Xc * MinorRadius, Coord2);
  V1.SetXYZ(Coord0);
  // D2 :
  Coord0.SetLinearForm(-Xc * MajorRadius, Coord1, -Yc * MinorRadius, Coord2);
  V2.SetXYZ(Coord0);
  // D3
  Coord0.SetLinearForm(Yc * MajorRadius, Coord1, -Xc * MinorRadius, Coord2);
  V3.SetXYZ(Coord0);
}

//=================================================================================================

void ElCLib1::HyperbolaD3(const Standard_Real U,
                         const Frame3d&       Pos,
                         const Standard_Real MajorRadius,
                         const Standard_Real MinorRadius,
                         Point3d&             P,
                         Vector3d&             V1,
                         Vector3d&             V2,
                         Vector3d&             V3)
{
  Standard_Real Xc = Cosh(U);
  Standard_Real Yc = Sinh(U);
  Coords3d        Coord0;
  Coords3d        Coord1(Pos.XDirection().XYZ());
  Coords3d        Coord2(Pos.YDirection().XYZ());
  // Point courant et D2 :
  Coord0.SetLinearForm(Xc * MajorRadius, Coord1, Yc * MinorRadius, Coord2);
  V2.SetXYZ(Coord0);
  Coord0.Add(Pos.Location().XYZ());
  P.SetXYZ(Coord0);
  // D1 et D3 :
  Coord0.SetLinearForm(Yc * MajorRadius, Coord1, Xc * MinorRadius, Coord2);
  V1.SetXYZ(Coord0);
  V3.SetXYZ(Coord0);
}

//=================================================================================================

gp_Pnt2d ElCLib1::LineValue(const Standard_Real U, const gp_Ax2d& Pos)
{
  const Coords2d& ZDir = Pos.Direction().XY();
  const Coords2d& PLoc = Pos.Location().XY();
  return gp_Pnt2d(U * ZDir.X() + PLoc.X(), U * ZDir.Y() + PLoc.Y());
}

//=================================================================================================

gp_Pnt2d ElCLib1::CircleValue(const Standard_Real U, const Ax22d& Pos, const Standard_Real Radius)
{
  const Coords2d&  XDir = Pos.XDirection().XY();
  const Coords2d&  YDir = Pos.YDirection().XY();
  const Coords2d&  PLoc = Pos.Location().XY();
  Standard_Real A1   = Radius * cos(U);
  Standard_Real A2   = Radius * sin(U);
  return gp_Pnt2d(A1 * XDir.X() + A2 * YDir.X() + PLoc.X(),
                  A1 * XDir.Y() + A2 * YDir.Y() + PLoc.Y());
}

//=================================================================================================

gp_Pnt2d ElCLib1::EllipseValue(const Standard_Real U,
                              const Ax22d&     Pos,
                              const Standard_Real MajorRadius,
                              const Standard_Real MinorRadius)
{
  const Coords2d&  XDir = Pos.XDirection().XY();
  const Coords2d&  YDir = Pos.YDirection().XY();
  const Coords2d&  PLoc = Pos.Location().XY();
  Standard_Real A1   = MajorRadius * cos(U);
  Standard_Real A2   = MinorRadius * sin(U);
  return gp_Pnt2d(A1 * XDir.X() + A2 * YDir.X() + PLoc.X(),
                  A1 * XDir.Y() + A2 * YDir.Y() + PLoc.Y());
}

//=================================================================================================

gp_Pnt2d ElCLib1::HyperbolaValue(const Standard_Real U,
                                const Ax22d&     Pos,
                                const Standard_Real MajorRadius,
                                const Standard_Real MinorRadius)
{
  const Coords2d&  XDir = Pos.XDirection().XY();
  const Coords2d&  YDir = Pos.YDirection().XY();
  const Coords2d&  PLoc = Pos.Location().XY();
  Standard_Real A1   = MajorRadius * Cosh(U);
  Standard_Real A2   = MinorRadius * Sinh(U);
  return gp_Pnt2d(A1 * XDir.X() + A2 * YDir.X() + PLoc.X(),
                  A1 * XDir.Y() + A2 * YDir.Y() + PLoc.Y());
}

//=================================================================================================

gp_Pnt2d ElCLib1::ParabolaValue(const Standard_Real U,
                               const Ax22d&     Pos,
                               const Standard_Real Focal)
{
  if (Focal == 0.0)
  {
    const Coords2d& XDir = Pos.XDirection().XY();
    const Coords2d& PLoc = Pos.Location().XY();
    return gp_Pnt2d(U * XDir.X() + PLoc.X(), U * XDir.Y() + PLoc.Y());
  }
  const Coords2d&  XDir = Pos.XDirection().XY();
  const Coords2d&  YDir = Pos.YDirection().XY();
  const Coords2d&  PLoc = Pos.Location().XY();
  Standard_Real A1   = U * U / (4.0 * Focal);
  return gp_Pnt2d(A1 * XDir.X() + U * YDir.X() + PLoc.X(), A1 * XDir.Y() + U * YDir.Y() + PLoc.Y());
}

//=================================================================================================

void ElCLib1::LineD1(const Standard_Real U, const gp_Ax2d& Pos, gp_Pnt2d& P, gp_Vec2d& V1)
{
  Coords2d Coord = Pos.Direction().XY();
  V1.SetXY(Coord);
  Coord.SetLinearForm(U, Coord, Pos.Location().XY());
  P.SetXY(Coord);
}

//=================================================================================================

void ElCLib1::CircleD1(const Standard_Real U,
                      const Ax22d&     Pos,
                      const Standard_Real Radius,
                      gp_Pnt2d&           P,
                      gp_Vec2d&           V1)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = Radius * cos(U);
  Standard_Real Yc = Radius * sin(U);
  // Point courant :
  Vxy.SetLinearForm(Xc, Xdir, Yc, Ydir, Pos.Location().XY());
  P.SetXY(Vxy);
  // V1 :
  Vxy.SetLinearForm(-Yc, Xdir, Xc, Ydir);
  V1.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::EllipseD1(const Standard_Real U,
                       const Ax22d&     Pos,
                       const Standard_Real MajorRadius,
                       const Standard_Real MinorRadius,
                       gp_Pnt2d&           P,
                       gp_Vec2d&           V1)
{
  Coords2d         Vxy;
  Coords2d         Xdir((Pos.XDirection()).XY());
  Coords2d         Ydir((Pos.YDirection()).XY());
  Standard_Real Xc = cos(U);
  Standard_Real Yc = sin(U);
  // Point courant :
  Vxy.SetLinearForm(Xc * MajorRadius, Xdir, Yc * MinorRadius, Ydir, Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(-Yc * MajorRadius, Xdir, Xc * MinorRadius, Ydir);
  V1.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::HyperbolaD1(const Standard_Real U,
                         const Ax22d&     Pos,
                         const Standard_Real MajorRadius,
                         const Standard_Real MinorRadius,
                         gp_Pnt2d&           P,
                         gp_Vec2d&           V1)
{
  Coords2d         Vxy;
  Coords2d         Xdir((Pos.XDirection()).XY());
  Coords2d         Ydir((Pos.YDirection()).XY());
  Standard_Real Xc = Cosh(U);
  Standard_Real Yc = Sinh(U);
  // Point courant :
  Vxy.SetLinearForm(Xc * MajorRadius, Xdir, Yc * MinorRadius, Ydir, Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(Yc * MajorRadius, Xdir, Xc * MinorRadius, Ydir);
  V1.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::ParabolaD1(const Standard_Real U,
                        const Ax22d&     Pos,
                        const Standard_Real Focal,
                        gp_Pnt2d&           P,
                        gp_Vec2d&           V1)
{
  Coords2d Vxy;
  Coords2d Xdir(Pos.XDirection().XY());
  if (Focal == 0.0)
  { // Parabole degenere en une droite
    V1.SetXY(Xdir);
    Vxy.SetLinearForm(U, Xdir, Pos.Location().XY());
  }
  else
  {
    Coords2d Ydir(Pos.YDirection().XY());
    Vxy.SetLinearForm(U / (2.0 * Focal), Xdir, Ydir);
    V1.SetXY(Vxy);
    Vxy.SetLinearForm((U * U) / (4.0 * Focal), Xdir, U, Ydir, Pos.Location().XY());
  }
  P.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::CircleD2(const Standard_Real U,
                      const Ax22d&     Pos,
                      const Standard_Real Radius,
                      gp_Pnt2d&           P,
                      gp_Vec2d&           V1,
                      gp_Vec2d&           V2)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = Radius * cos(U);
  Standard_Real Yc = Radius * sin(U);
  // V2 :
  Vxy.SetLinearForm(Xc, Xdir, Yc, Ydir);
  V2.SetXY(Vxy);
  V2.Reverse();
  Vxy.Add(Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(-Yc, Xdir, Xc, Ydir);
  V1.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::EllipseD2(const Standard_Real U,
                       const Ax22d&     Pos,
                       const Standard_Real MajorRadius,
                       const Standard_Real MinorRadius,
                       gp_Pnt2d&           P,
                       gp_Vec2d&           V1,
                       gp_Vec2d&           V2)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = cos(U);
  Standard_Real Yc = sin(U);

  // V2 :
  Vxy.SetLinearForm(Xc * MajorRadius, Xdir, Yc * MinorRadius, Ydir);
  V2.SetXY(Vxy);
  V2.Reverse();

  // Point courant :
  Vxy.Add(Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(-Yc * MajorRadius, Xdir, Xc * MinorRadius, Ydir);
  V1.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::HyperbolaD2(const Standard_Real U,
                         const Ax22d&     Pos,
                         const Standard_Real MajorRadius,
                         const Standard_Real MinorRadius,
                         gp_Pnt2d&           P,
                         gp_Vec2d&           V1,
                         gp_Vec2d&           V2)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = Cosh(U);
  Standard_Real Yc = Sinh(U);

  // V2 :
  Vxy.SetLinearForm(Xc * MajorRadius, Xdir, Yc * MinorRadius, Ydir);
  V2.SetXY(Vxy);

  // Point courant :
  Vxy.Add(Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(Yc * MajorRadius, Xdir, Xc * MinorRadius, Ydir);
  V1.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::ParabolaD2(const Standard_Real U,
                        const Ax22d&     Pos,
                        const Standard_Real Focal,
                        gp_Pnt2d&           P,
                        gp_Vec2d&           V1,
                        gp_Vec2d&           V2)
{
  Coords2d Vxy;
  Coords2d Xdir(Pos.XDirection().XY());
  if (Focal == 0.0)
  {
    V2.SetCoord(0.0, 0.0);
    V1.SetXY(Xdir);
    Vxy.SetLinearForm(U, Xdir, Pos.Location().XY());
  }
  else
  {
    Coords2d Ydir(Pos.YDirection().XY());
    Vxy = Xdir.Multiplied(1.0 / (2.0 * Focal));
    V2.SetXY(Vxy);
    Vxy.SetLinearForm(U, Vxy, Ydir);
    V1.SetXY(Vxy);
    Vxy.SetLinearForm(U * U / (4.0 * Focal), Xdir, U, Ydir);
    Vxy.Add(Pos.Location().XY());
  }
  P.SetXY(Vxy);
}

//=================================================================================================

void ElCLib1::CircleD3(const Standard_Real U,
                      const Ax22d&     Pos,
                      const Standard_Real Radius,
                      gp_Pnt2d&           P,
                      gp_Vec2d&           V1,
                      gp_Vec2d&           V2,
                      gp_Vec2d&           V3)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = Radius * cos(U);
  Standard_Real Yc = Radius * sin(U);

  // V2 :
  Vxy.SetLinearForm(Xc, Xdir, Yc, Ydir);
  V2.SetXY(Vxy);
  V2.Reverse();

  // Point courant :
  Vxy.Add(Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(-Yc, Xdir, Xc, Ydir);
  V1.SetXY(Vxy);

  // V3 :
  V3.SetXY(Vxy);
  V3.Reverse();
}

//=================================================================================================

void ElCLib1::EllipseD3(const Standard_Real U,
                       const Ax22d&     Pos,
                       const Standard_Real MajorRadius,
                       const Standard_Real MinorRadius,
                       gp_Pnt2d&           P,
                       gp_Vec2d&           V1,
                       gp_Vec2d&           V2,
                       gp_Vec2d&           V3)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = cos(U);
  Standard_Real Yc = sin(U);

  // V2 :
  Vxy.SetLinearForm(Xc * MajorRadius, Xdir, Yc * MinorRadius, Ydir);
  V2.SetXY(Vxy);
  V2.Reverse();

  // Point courant :
  Vxy.Add(Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(-Yc * MajorRadius, Xdir, Xc * MinorRadius, Ydir);
  V1.SetXY(Vxy);

  // V3 :
  V3.SetXY(Vxy);
  V3.Reverse();
}

//=================================================================================================

void ElCLib1::HyperbolaD3(const Standard_Real U,
                         const Ax22d&     Pos,
                         const Standard_Real MajorRadius,
                         const Standard_Real MinorRadius,
                         gp_Pnt2d&           P,
                         gp_Vec2d&           V1,
                         gp_Vec2d&           V2,
                         gp_Vec2d&           V3)
{
  Coords2d         Vxy;
  Coords2d         Xdir(Pos.XDirection().XY());
  Coords2d         Ydir(Pos.YDirection().XY());
  Standard_Real Xc = Cosh(U);
  Standard_Real Yc = Sinh(U);

  // V2 :
  Vxy.SetLinearForm(Xc * MajorRadius, Xdir, Yc * MinorRadius, Ydir);
  V2.SetXY(Vxy);

  // Point courant :
  Vxy.Add(Pos.Location().XY());
  P.SetXY(Vxy);

  // V1 :
  Vxy.SetLinearForm(Yc * MajorRadius, Xdir, Xc * MinorRadius, Ydir);
  V1.SetXY(Vxy);

  // V3 :
  V3.SetXY(Vxy);
}

//=================================================================================================

Vector3d ElCLib1::LineDN(const Standard_Real, const Axis3d& Pos, const Standard_Integer N)
{
  if (N == 1)
  {
    return Vector3d(Pos.Direction());
  }
  return Vector3d(0., 0., 0.);
}

//=================================================================================================

Vector3d ElCLib1::CircleDN(const Standard_Real    U,
                        const Frame3d&          Pos,
                        const Standard_Real    Radius,
                        const Standard_Integer N)
{
  Standard_Real Xc = 0, Yc = 0;
  if (N == 1)
  {
    Xc = Radius * -sin(U);
    Yc = Radius * cos(U);
  }
  else if ((N + 2) % 4 == 0)
  {
    Xc = Radius * -cos(U);
    Yc = Radius * -sin(U);
  }
  else if ((N + 1) % 4 == 0)
  {
    Xc = Radius * sin(U);
    Yc = Radius * -cos(U);
  }
  else if (N % 4 == 0)
  {
    Xc = Radius * cos(U);
    Yc = Radius * sin(U);
  }
  else if ((N - 1) % 4 == 0)
  {
    Xc = Radius * -sin(U);
    Yc = Radius * cos(U);
  }
  Coords3d Coord1(Pos.XDirection().XYZ());
  Coord1.SetLinearForm(Xc, Coord1, Yc, Pos.YDirection().XYZ());
  return Vector3d(Coord1);
}

//=================================================================================================

Vector3d ElCLib1::EllipseDN(const Standard_Real    U,
                         const Frame3d&          Pos,
                         const Standard_Real    MajorRadius,
                         const Standard_Real    MinorRadius,
                         const Standard_Integer N)
{
  Standard_Real Xc = 0, Yc = 0;
  if (N == 1)
  {
    Xc = MajorRadius * -sin(U);
    Yc = MinorRadius * cos(U);
  }
  else if ((N + 2) % 4 == 0)
  {
    Xc = MajorRadius * -cos(U);
    Yc = MinorRadius * -sin(U);
  }
  else if ((N + 1) % 4 == 0)
  {
    Xc = MajorRadius * sin(U);
    Yc = MinorRadius * -cos(U);
  }
  else if (N % 4 == 0)
  {
    Xc = MajorRadius * cos(U);
    Yc = MinorRadius * sin(U);
  }
  else if ((N - 1) % 4 == 0)
  {
    Xc = MajorRadius * -sin(U);
    Yc = MinorRadius * cos(U);
  }
  Coords3d Coord1(Pos.XDirection().XYZ());
  Coord1.SetLinearForm(Xc, Coord1, Yc, Pos.YDirection().XYZ());
  return Vector3d(Coord1);
}

//=================================================================================================

Vector3d ElCLib1::HyperbolaDN(const Standard_Real    U,
                           const Frame3d&          Pos,
                           const Standard_Real    MajorRadius,
                           const Standard_Real    MinorRadius,
                           const Standard_Integer N)
{
  Standard_Real Xc = 0, Yc = 0;
  if (IsOdd(N))
  {
    Xc = MajorRadius * Sinh(U);
    Yc = MinorRadius * Cosh(U);
  }
  else if (IsEven(N))
  {
    Xc = MajorRadius * Cosh(U);
    Yc = MinorRadius * Sinh(U);
  }
  Coords3d Coord1(Pos.XDirection().XYZ());
  Coord1.SetLinearForm(Xc, Coord1, Yc, Pos.YDirection().XYZ());
  return Vector3d(Coord1);
}

//=================================================================================================

Vector3d ElCLib1::ParabolaDN(const Standard_Real    U,
                          const Frame3d&          Pos,
                          const Standard_Real    Focal,
                          const Standard_Integer N)
{
  if (N <= 2)
  {
    Coords3d Coord1(Pos.XDirection().XYZ());
    if (N == 1)
    {
      if (Focal == 0.0)
      {
        return Vector3d(Coord1);
      }
      else
      {
        Coord1.SetLinearForm(U / (2.0 * Focal), Coord1, Pos.YDirection().XYZ());
        return Vector3d(Coord1);
      }
    }
    else if (N == 2)
    {
      if (Focal == 0.0)
      {
        return Vector3d(0.0, 0.0, 0.0);
      }
      else
      {
        Coord1.Multiply(1.0 / (2.0 * Focal));
        return Vector3d(Coord1);
      }
    }
  }
  return Vector3d(0., 0., 0.);
}

//=================================================================================================

gp_Vec2d ElCLib1::LineDN(const Standard_Real, const gp_Ax2d& Pos, const Standard_Integer N)
{
  if (N == 1)
  {
    return gp_Vec2d(Pos.Direction());
  }
  return gp_Vec2d(0.0, 0.0);
}

//=================================================================================================

gp_Vec2d ElCLib1::CircleDN(const Standard_Real    U,
                          const Ax22d&        Pos,
                          const Standard_Real    Radius,
                          const Standard_Integer N)
{
  Standard_Real Xc = 0, Yc = 0;
  if (N == 1)
  {
    Xc = Radius * -sin(U);
    Yc = Radius * cos(U);
  }
  else if ((N + 2) % 4 == 0)
  {
    Xc = Radius * -cos(U);
    Yc = Radius * -sin(U);
  }
  else if ((N + 1) % 4 == 0)
  {
    Xc = Radius * sin(U);
    Yc = Radius * -cos(U);
  }
  else if (N % 4 == 0)
  {
    Xc = Radius * cos(U);
    Yc = Radius * sin(U);
  }
  else if ((N - 1) % 4 == 0)
  {
    Xc = Radius * -sin(U);
    Yc = Radius * cos(U);
  }
  Coords2d Xdir(Pos.XDirection().XY());
  Coords2d Ydir(Pos.YDirection().XY());
  Xdir.SetLinearForm(Xc, Xdir, Yc, Ydir);
  return gp_Vec2d(Xdir);
}

//=================================================================================================

gp_Vec2d ElCLib1::EllipseDN(const Standard_Real    U,
                           const Ax22d&        Pos,
                           const Standard_Real    MajorRadius,
                           const Standard_Real    MinorRadius,
                           const Standard_Integer N)
{
  Standard_Real Xc = 0, Yc = 0;
  if (N == 1)
  {
    Xc = MajorRadius * -sin(U);
    Yc = MinorRadius * cos(U);
  }
  else if ((N + 2) % 4 == 0)
  {
    Xc = MajorRadius * -cos(U);
    Yc = MinorRadius * -sin(U);
  }
  else if ((N + 1) % 4 == 0)
  {
    Xc = MajorRadius * sin(U);
    Yc = MinorRadius * -cos(U);
  }
  else if (N % 4 == 0)
  {
    Xc = MajorRadius * cos(U);
    Yc = MinorRadius * sin(U);
  }
  else if ((N - 1) % 4 == 0)
  {
    Xc = MajorRadius * -sin(U);
    Yc = MinorRadius * cos(U);
  }
  Coords2d Xdir(Pos.XDirection().XY());
  Coords2d Ydir(Pos.YDirection().XY());
  Xdir.SetLinearForm(Xc, Xdir, Yc, Ydir);
  return gp_Vec2d(Xdir);
}

//=================================================================================================

gp_Vec2d ElCLib1::HyperbolaDN(const Standard_Real    U,
                             const Ax22d&        Pos,
                             const Standard_Real    MajorRadius,
                             const Standard_Real    MinorRadius,
                             const Standard_Integer N)
{
  Standard_Real Xc = 0, Yc = 0;
  if (IsOdd(N))
  {
    Xc = MajorRadius * Sinh(U);
    Yc = MinorRadius * Cosh(U);
  }
  else if (IsEven(N))
  {
    Xc = MajorRadius * Cosh(U);
    Yc = MinorRadius * Sinh(U);
  }
  Coords2d Xdir(Pos.XDirection().XY());
  Coords2d Ydir(Pos.YDirection().XY());
  Xdir.SetLinearForm(Xc, Xdir, Yc, Ydir);
  return gp_Vec2d(Xdir);
}

//=================================================================================================

gp_Vec2d ElCLib1::ParabolaDN(const Standard_Real    U,
                            const Ax22d&        Pos,
                            const Standard_Real    Focal,
                            const Standard_Integer N)
{
  if (N <= 2)
  {
    Coords2d Xdir(Pos.XDirection().XY());
    if (N == 1)
    {
      if (Focal == 0.0)
      {
        return gp_Vec2d(Xdir);
      }
      else
      {
        Coords2d Ydir(Pos.YDirection().XY());
        Xdir.SetLinearForm(U / (2.0 * Focal), Xdir, Ydir);
        return gp_Vec2d(Xdir);
      }
    }
    else if (N == 2)
    {
      if (Focal == 0.0)
      {
        return gp_Vec2d(0.0, 0.0);
      }
      else
      {
        Xdir.Multiply(1.0 / (2.0 * Focal));
        return gp_Vec2d(Xdir);
      }
    }
  }
  return gp_Vec2d(0.0, 0.0);
}

//=================================================================================================

Standard_Real ElCLib1::LineParameter(const Axis3d& L, const Point3d& P)
{
  return (P.XYZ() - L.Location().XYZ()).Dot(L.Direction().XYZ());
}

//=================================================================================================

Standard_Real ElCLib1::CircleParameter(const Frame3d& Pos, const Point3d& P)
{
  Vector3d aVec(Pos.Location(), P);
  if (aVec.SquareMagnitude() < gp1::Resolution())
    // coinciding points -> infinite number of parameters
    return 0.0;

  const Dir3d& dir = Pos.Direction();
  // Project vector on circle's plane
  Coords3d aVProj = dir.XYZ().CrossCrossed(aVec.XYZ(), dir.XYZ());

  if (aVProj.SquareModulus() < gp1::Resolution())
    return 0.0;

  // Angle between X direction and projected vector
  Standard_Real Teta = (Pos.XDirection()).AngleWithRef(aVProj, dir);

  if (Teta < -1.e-16)
    Teta += PIPI;
  else if (Teta < 0)
    Teta = 0;
  return Teta;
}

//=================================================================================================

Standard_Real ElCLib1::EllipseParameter(const Frame3d&       Pos,
                                       const Standard_Real MajorRadius,
                                       const Standard_Real MinorRadius,
                                       const Point3d&       P)
{
  Coords3d        OP    = P.XYZ() - Pos.Location().XYZ();
  Coords3d        xaxis = Pos.XDirection().XYZ();
  Coords3d        yaxis = Pos.YDirection().XYZ();
  Standard_Real NY    = OP.Dot(yaxis);
  Standard_Real NX    = OP.Dot(xaxis);

  if ((Abs(NX) <= gp1::Resolution()) && (Abs(NY) <= gp1::Resolution()))
    //-- The point P is on the Axis of the Ellipse.
    return (0.0);

  yaxis.Multiply(NY * (MajorRadius / MinorRadius));
  Coords3d Om = xaxis.Multiplied(NX);
  Om.Add(yaxis);
  Standard_Real Teta = Vector3d(xaxis).AngleWithRef(Vector3d(Om), Vector3d(Pos.Direction()));
  if (Teta < -1.e-16)
    Teta += PIPI;
  else if (Teta < 0)
    Teta = 0;
  return Teta;
}

//=================================================================================================

Standard_Real ElCLib1::HyperbolaParameter(const Frame3d& Pos,
                                         const Standard_Real,
                                         const Standard_Real MinorRadius,
                                         const Point3d&       P)
{
  Standard_Real sht = Vector3d(Pos.Location(), P).Dot(Vector3d(Pos.YDirection())) / MinorRadius;

#if defined(__QNX__)
  return std::asinh(sht);
#else
  return asinh(sht);
#endif
}

//=================================================================================================

Standard_Real ElCLib1::ParabolaParameter(const Frame3d& Pos, const Point3d& P)
{
  return Vector3d(Pos.Location(), P).Dot(Vector3d(Pos.YDirection()));
}

//=================================================================================================

Standard_Real ElCLib1::LineParameter(const gp_Ax2d& L, const gp_Pnt2d& P)
{
  Coords2d Coord = P.XY();
  Coord.Subtract(L.Location().XY());
  return Coord.Dot(L.Direction().XY());
}

//=================================================================================================

Standard_Real ElCLib1::CircleParameter(const Ax22d& Pos, const gp_Pnt2d& P)
{
  Standard_Real Teta = (Pos.XDirection()).Angle(gp_Vec2d(Pos.Location(), P));
  Teta               = ((Pos.XDirection() ^ Pos.YDirection()) >= 0.0) ? Teta : -Teta;
  if (Teta < -1.e-16)
    Teta += PIPI;
  else if (Teta < 0)
    Teta = 0;
  return Teta;
}

//=================================================================================================

Standard_Real ElCLib1::EllipseParameter(const Ax22d&     Pos,
                                       const Standard_Real MajorRadius,
                                       const Standard_Real MinorRadius,
                                       const gp_Pnt2d&     P)
{
  Coords2d OP = P.XY();
  OP.Subtract(Pos.Location().XY());
  Coords2d xaxis = Pos.XDirection().XY();
  Coords2d yaxis = Pos.YDirection().XY();
  Coords2d Om    = xaxis.Multiplied(OP.Dot(xaxis));
  yaxis.Multiply((OP.Dot(yaxis)) * (MajorRadius / MinorRadius));
  Om.Add(yaxis);
  Standard_Real Teta = gp_Vec2d(xaxis).Angle(gp_Vec2d(Om));
  Teta               = ((Pos.XDirection() ^ Pos.YDirection()) >= 0.0) ? Teta : -Teta;
  if (Teta < -1.e-16)
    Teta += PIPI;
  else if (Teta < 0)
    Teta = 0;
  return Teta;
}

//=================================================================================================

Standard_Real ElCLib1::HyperbolaParameter(const Ax22d& Pos,
                                         const Standard_Real,
                                         const Standard_Real MinorRadius,
                                         const gp_Pnt2d&     P)
{
  gp_Vec2d      V(Pos.YDirection().XY());
  Standard_Real sht = gp_Vec2d(Pos.Location(), P).Dot(V) / MinorRadius;
#if defined(__QNX__)
  return std::asinh(sht);
#else
  return asinh(sht);
#endif
}

//=================================================================================================

Standard_Real ElCLib1::ParabolaParameter(const Ax22d& Pos, const gp_Pnt2d& P)
{
  gp_Vec2d Directrix(Pos.YDirection().XY());
  return gp_Vec2d(Pos.Location(), P).Dot(Directrix);
}

//=================================================================================================

Point3d ElCLib1::To3d(const Frame3d& Pos, const gp_Pnt2d& P)
{
  Coords3d Vxy = Pos.XDirection().XYZ();
  Vxy.SetLinearForm(P.X(), Vxy, P.Y(), Pos.YDirection().XYZ(), Pos.Location().XYZ());
  return Point3d(Vxy);
}

//=================================================================================================

Dir3d ElCLib1::To3d(const Frame3d& Pos, const gp_Dir2d& V)
{
  Vector3d Vx = Pos.XDirection();
  Vector3d Vy = Pos.YDirection();
  Vx.Multiply(V.X());
  Vy.Multiply(V.Y());
  Vx.Add(Vy);
  return Dir3d(Vx);
}

//=================================================================================================

Vector3d ElCLib1::To3d(const Frame3d& Pos, const gp_Vec2d& V)
{
  Vector3d Vx = Pos.XDirection();
  Vector3d Vy = Pos.YDirection();
  Vx.Multiply(V.X());
  Vy.Multiply(V.Y());
  Vx.Add(Vy);
  return Vx;
}

//=================================================================================================

Axis3d ElCLib1::To3d(const Frame3d& Pos, const gp_Ax2d& A)
{
  Point3d P = ElCLib1::To3d(Pos, A.Location());
  Vector3d V = ElCLib1::To3d(Pos, A.Direction());
  return Axis3d(P, V);
}

//=================================================================================================

Frame3d ElCLib1::To3d(const Frame3d& Pos, const Ax22d& A)
{
  Point3d P  = ElCLib1::To3d(Pos, A.Location());
  Vector3d VX = ElCLib1::To3d(Pos, A.XDirection());
  Vector3d VY = ElCLib1::To3d(Pos, A.YDirection());
  return Frame3d(P, VX.Crossed(VY), VX);
}

//=================================================================================================

gp_Lin ElCLib1::To3d(const Frame3d& Pos, const gp_Lin2d& L)
{
  return gp_Lin(ElCLib1::To3d(Pos, L.Position1()));
}

//=================================================================================================

gp_Circ ElCLib1::To3d(const Frame3d& Pos, const gp_Circ2d& C)
{
  return gp_Circ(ElCLib1::To3d(Pos, C.Axis()), C.Radius());
}

//=================================================================================================

gp_Elips ElCLib1::To3d(const Frame3d& Pos, const gp_Elips2d& E)
{
  return gp_Elips(ElCLib1::To3d(Pos, E.Axis()), E.MajorRadius(), E.MinorRadius());
}

//=================================================================================================

gp_Hypr ElCLib1::To3d(const Frame3d& Pos, const gp_Hypr2d& H)
{
  return gp_Hypr(ElCLib1::To3d(Pos, H.Axis()), H.MajorRadius(), H.MinorRadius());
}

//=================================================================================================

gp_Parab ElCLib1::To3d(const Frame3d& Pos, const gp_Parab2d& Prb)
{
  return gp_Parab(ElCLib1::To3d(Pos, Prb.Axis()), Prb.Focal());
}
