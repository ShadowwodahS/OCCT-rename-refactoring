// Created on: 1996-11-21
// Created by: Joelle CHAUVET
// Copyright (c) 1996-1999 Matra Datavision
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

// Modified:	Wed Mar  5 09:45:42 1997
//    by:	Joelle CHAUVET
//              G1134 : new methods RealBounds and Constraints
// Modified:	Mon Jun 16 15:22:41 1997
//    by:	Jerome LEMONIER
//              Correction de la methode D2 (faute de frappe dans le code)
//              Correction de la methode D1 (D0 inutile)

#include <Geom_Curve.hxx>
#include <Geom_Geometry.hxx>
#include <GeomPlate_Surface.hxx>
#include <gp_GTrsf2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XY.hxx>
#include <Plate_Plate.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomPlate_Surface, GeomSurface)

//=================================================================================================

GeomPlate_Surface::GeomPlate_Surface(const Handle(GeomSurface)& Surfinit,
                                     const PlateSurface&          Surfinter)
    : mySurfinter(Surfinter),
      mySurfinit(Surfinit),
      myUmin(0.0),
      myUmax(0.0),
      myVmin(0.0),
      myVmax(0.0)
{
}

//=================================================================================================

void GeomPlate_Surface::UReverse()
{
  // throw ExceptionBase("UReverse");
}

//=================================================================================================

Standard_Real GeomPlate_Surface::UReversedParameter(const Standard_Real U) const
{ // throw ExceptionBase("UReversedParameter");
  return (-U);
}

//=================================================================================================

void GeomPlate_Surface::VReverse()
{ // throw ExceptionBase("VReverse");
}

//=================================================================================================

Standard_Real GeomPlate_Surface::VReversedParameter(const Standard_Real V) const
{ // throw ExceptionBase("VReversedParameter");
  return (-V);
}

//=================================================================================================

// void GeomPlate_Surface::TransformParameters(Standard_Real& U, Standard_Real& V, const Transform3d& T)
// const
void GeomPlate_Surface::TransformParameters(Standard_Real&, Standard_Real&, const Transform3d&) const
{ // throw ExceptionBase("TransformParameters");
}

//=================================================================================================

// gp_GTrsf2d GeomPlate_Surface::ParametricTransformation(const Transform3d& T) const
gp_GTrsf2d GeomPlate_Surface::ParametricTransformation(const Transform3d&) const
{ // throw ExceptionBase("ParametricTransformation");
  return gp_GTrsf2d();
}

//=================================================================================================

void GeomPlate_Surface::Bounds(Standard_Real& U1,
                               Standard_Real& U2,
                               Standard_Real& V1,
                               Standard_Real& V2) const
{
  if (mySurfinit->DynamicType() == STANDARD_TYPE(GeomPlate_Surface))
    mySurfinit->Bounds(U1, U2, V1, V2);
  else
  {
    U1 = myUmin;
    U2 = myUmax;
    V1 = myVmin;
    V2 = myVmax;
  }
}

//=================================================================================================

Standard_Boolean GeomPlate_Surface::IsUClosed() const
{ // throw ExceptionBase("IsUClosed(");
  // return 1;
  return 0;
}

//=================================================================================================

Standard_Boolean GeomPlate_Surface::IsVClosed() const
{ // throw ExceptionBase("IsVClosed(");
  // return 1;
  return 0;
}

//=================================================================================================

Standard_Boolean GeomPlate_Surface::IsUPeriodic() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Real GeomPlate_Surface::UPeriod() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean GeomPlate_Surface::IsVPeriodic() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Real GeomPlate_Surface::VPeriod() const
{
  return Standard_False;
}

//=================================================================================================

// Handle(GeomCurve3d) GeomPlate_Surface::UIso(const Standard_Real U) const
Handle(GeomCurve3d) GeomPlate_Surface::UIso(const Standard_Real) const
{ // throw ExceptionBase("UIso");
  return Handle(GeomCurve3d)();
}

//=================================================================================================

// Handle(GeomCurve3d) GeomPlate_Surface::VIso(const Standard_Real V) const
Handle(GeomCurve3d) GeomPlate_Surface::VIso(const Standard_Real) const
{ // throw ExceptionBase("VIso");
  return Handle(GeomCurve3d)();
}

//=================================================================================================

GeomAbs_Shape GeomPlate_Surface::Continuity() const
{ // throw ExceptionBase("Continuity()");
  return GeomAbs_Shape();
}

//=================================================================================================

// Standard_Boolean GeomPlate_Surface::IsCNu(const Standard_Integer N) const
Standard_Boolean GeomPlate_Surface::IsCNu(const Standard_Integer) const
{
  throw ExceptionBase("IsCNu");
}

//=================================================================================================

// Standard_Boolean GeomPlate_Surface::IsCNv(const Standard_Integer N) const
Standard_Boolean GeomPlate_Surface::IsCNv(const Standard_Integer) const
{
  throw ExceptionBase("IsCNv");
}

//=================================================================================================

void GeomPlate_Surface::D0(const Standard_Real U, const Standard_Real V, Point3d& P) const
{
  Coords2d  P1(U, V);
  Point3d P2;
  mySurfinit->D0(U, V, P2);
  gp_XYZ P3; //=mySurfinter.Evaluate(P1);
  P3 = mySurfinter.Evaluate(P1);
  for (Standard_Integer i = 1; i <= 3; i++)
  {
    P.SetCoord(i, P3.Coord(i) + P2.Coord(i));
  }
}

//=================================================================================================

void GeomPlate_Surface::D1(const Standard_Real U,
                           const Standard_Real V,
                           Point3d&             P,
                           Vector3d&             D1U,
                           Vector3d&             D1V) const
{
  Coords2d  P1(U, V);
  Point3d P2;
  D0(U, V, P);
  Vector3d V1U, V1V;
  mySurfinit->D1(U, V, P2, V1U, V1V);
  gp_XYZ V2U = mySurfinter.EvaluateDerivative(P1, 1, 0);
  gp_XYZ V2V = mySurfinter.EvaluateDerivative(P1, 0, 1);
  for (Standard_Integer i = 1; i <= 3; i++)
  {
    D1U.SetCoord(i, V1U.Coord(i) + V2U.Coord(i));
    D1V.SetCoord(i, V1V.Coord(i) + V2V.Coord(i));
  }
}

//=================================================================================================

void GeomPlate_Surface::D2(const Standard_Real U,
                           const Standard_Real V,
                           Point3d&             P,
                           Vector3d&             D1U,
                           Vector3d&             D1V,
                           Vector3d&             D2U,
                           Vector3d&             D2V,
                           Vector3d&             D2UV) const
{
  Coords2d  P1(U, V);
  Point3d P2;

  Vector3d V1U, V1V, V1UV, vv, v;
  D1(U, V, P, D1U, D1V);
  mySurfinit->D2(U, V, P2, vv, v, V1U, V1V, V1UV);
  gp_XYZ V2U  = mySurfinter.EvaluateDerivative(P1, 2, 0);
  gp_XYZ V2V  = mySurfinter.EvaluateDerivative(P1, 0, 2);
  gp_XYZ V2UV = mySurfinter.EvaluateDerivative(P1, 1, 1);
  for (Standard_Integer i = 1; i <= 3; i++)
  {
    D2U.SetCoord(i, V1U.Coord(i) + V2U.Coord(i));
    D2V.SetCoord(i, V1V.Coord(i) + V2V.Coord(i));
    D2UV.SetCoord(i, V1UV.Coord(i) + V2UV.Coord(i));
  }
}

//=================================================================================================

// void GeomPlate_Surface::D3(const Standard_Real U, const Standard_Real V, Point3d& P, Vector3d& D1U,
// Vector3d& D1V, Vector3d& D2U, Vector3d& D2V, Vector3d& D2UV, Vector3d& D3U, Vector3d& D3V, Vector3d& D3UUV,
// Vector3d& D3UVV) const
void GeomPlate_Surface::D3(const Standard_Real,
                           const Standard_Real,
                           Point3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&,
                           Vector3d&) const
{
  throw ExceptionBase("D3");
}

//=================================================================================================

// Vector3d GeomPlate_Surface::DN(const Standard_Real U, const Standard_Real V, const Standard_Integer
// Nu, const Standard_Integer Nv) const
Vector3d GeomPlate_Surface::DN(const Standard_Real,
                             const Standard_Real,
                             const Standard_Integer,
                             const Standard_Integer) const
{
  throw ExceptionBase("DN");
}

//=================================================================================================

Handle(Geom_Geometry) GeomPlate_Surface::Copy() const
{
  Handle(GeomPlate_Surface) GPS = new GeomPlate_Surface(mySurfinit, mySurfinter);
  return GPS;
}

//=================================================================================================

// void GeomPlate_Surface::Transform(const Transform3d& T)
void GeomPlate_Surface::Transform(const Transform3d&)
{ // throw ExceptionBase("Transform");
}

//=================================================================================================

Handle(GeomSurface) GeomPlate_Surface::CallSurfinit() const

{
  return mySurfinit;
}

//=================================================================================================

void GeomPlate_Surface::SetBounds(const Standard_Real Umin,
                                  const Standard_Real Umax,
                                  const Standard_Real Vmin,
                                  const Standard_Real Vmax)
{
  if ((Umin > Umax) || (Vmin > Vmax))
    throw ExceptionBase("Bounds haven't the good sense");
  if ((Umin == Umax) || (Vmin == Vmax))
    throw ExceptionBase("Bounds are equal");
  myUmin = Umin;
  myUmax = Umax;
  myVmin = Vmin;
  myVmax = Vmax;
}

//=================================================================================================

void GeomPlate_Surface::RealBounds(Standard_Real& U1,
                                   Standard_Real& U2,
                                   Standard_Real& V1,
                                   Standard_Real& V2) const
{
  mySurfinter.UVBox(U1, U2, V1, V2);
}

//=================================================================================================

void GeomPlate_Surface::Constraints(TColgp_SequenceOfXY& Seq) const
{
  mySurfinter.UVConstraints(Seq);
}
