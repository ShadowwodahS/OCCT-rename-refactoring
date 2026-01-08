// Created on: 1992-01-20
// Created by: Remi GILET
// Copyright (c) 1992-1999 Matra Datavision
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

#include <ElCLib.hxx>
#include <Geom2dGcc_CurveTool.hxx>
#include <Geom2dGcc_FunctionTanCuCuCu.hxx>
#include <gp.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <math_Matrix.hxx>
#include <Standard_ConstructionError.hxx>

void Geom2dGcc_FunctionTanCuCuCu::InitDerivative(const math_Vector& X,
                                                 gp_Pnt2d&          Point1,
                                                 gp_Pnt2d&          Point2,
                                                 gp_Pnt2d&          Point3,
                                                 gp_Vec2d&          Tan1,
                                                 gp_Vec2d&          Tan2,
                                                 gp_Vec2d&          Tan3,
                                                 gp_Vec2d&          D21,
                                                 gp_Vec2d&          D22,
                                                 gp_Vec2d&          D23)
{
  switch (TheType)
  {
    case Geom2dGcc_CiCuCu: {
      ElCLib1::D2(X(1), Circ1, Point1, Tan1, D21);
      CurveTool3::D2(Curv2, X(2), Point2, Tan2, D22);
      CurveTool3::D2(Curv3, X(3), Point3, Tan3, D23);
    }
    break;
    case Geom2dGcc_CiCiCu: {
      ElCLib1::D2(X(1), Circ1, Point1, Tan1, D21);
      ElCLib1::D2(X(2), Circ2, Point2, Tan2, D22);
      CurveTool3::D2(Curv3, X(3), Point3, Tan3, D23);
    }
    break;
    case Geom2dGcc_CiLiCu: {
      ElCLib1::D2(X(1), Circ1, Point1, Tan1, D21);
      ElCLib1::D1(X(2), Lin2, Point2, Tan2);
      D22 = gp_Vec2d(0., 0.);
      CurveTool3::D2(Curv3, X(3), Point3, Tan3, D23);
    }
    break;
    case Geom2dGcc_LiCuCu: {
      ElCLib1::D1(X(1), Lin1, Point1, Tan1);
      D21 = gp_Vec2d(0., 0.);
      CurveTool3::D2(Curv2, X(2), Point2, Tan2, D22);
      CurveTool3::D2(Curv3, X(3), Point3, Tan3, D23);
    }
    break;
    case Geom2dGcc_LiLiCu: {
      ElCLib1::D1(X(1), Lin1, Point1, Tan1);
      D21 = gp_Vec2d(0., 0.);
      ElCLib1::D1(X(2), Lin2, Point2, Tan2);
      D22 = gp_Vec2d(0., 0.);
      CurveTool3::D2(Curv3, X(3), Point3, Tan3, D23);
    }
    break;
    case Geom2dGcc_CuCuCu: {
      CurveTool3::D2(Curv1, X(1), Point1, Tan1, D21);
      CurveTool3::D2(Curv2, X(2), Point2, Tan2, D22);
      CurveTool3::D2(Curv3, X(3), Point3, Tan3, D23);
    }
    break;
    default: {
      throw Standard_ConstructionError();
    }
  }
}

Geom2dGcc_FunctionTanCuCuCu::Geom2dGcc_FunctionTanCuCuCu(const Geom2dAdaptor_Curve& C1,
                                                         const Geom2dAdaptor_Curve& C2,
                                                         const Geom2dAdaptor_Curve& C3)
{
  Curv1   = C1;
  Curv2   = C2;
  Curv3   = C3;
  TheType = Geom2dGcc_CuCuCu;
}

Geom2dGcc_FunctionTanCuCuCu::Geom2dGcc_FunctionTanCuCuCu(const gp_Circ2d&           C1,
                                                         const Geom2dAdaptor_Curve& C2,
                                                         const Geom2dAdaptor_Curve& C3)
{
  Circ1   = C1;
  Curv2   = C2;
  Curv3   = C3;
  TheType = Geom2dGcc_CiCuCu;
}

Geom2dGcc_FunctionTanCuCuCu::Geom2dGcc_FunctionTanCuCuCu(const gp_Circ2d&           C1,
                                                         const gp_Circ2d&           C2,
                                                         const Geom2dAdaptor_Curve& C3)
{
  Circ1   = C1;
  Circ2   = C2;
  Curv3   = C3;
  TheType = Geom2dGcc_CiCiCu;
}

Geom2dGcc_FunctionTanCuCuCu::Geom2dGcc_FunctionTanCuCuCu(const gp_Circ2d&           C1,
                                                         const gp_Lin2d&            L2,
                                                         const Geom2dAdaptor_Curve& C3)
{
  Circ1   = C1;
  Lin2    = L2;
  Curv3   = C3;
  TheType = Geom2dGcc_CiLiCu;
}

Geom2dGcc_FunctionTanCuCuCu::Geom2dGcc_FunctionTanCuCuCu(const gp_Lin2d&            L1,
                                                         const gp_Lin2d&            L2,
                                                         const Geom2dAdaptor_Curve& C3)
{
  Lin1    = L1;
  Lin2    = L2;
  Curv3   = C3;
  TheType = Geom2dGcc_LiLiCu;
}

Geom2dGcc_FunctionTanCuCuCu::Geom2dGcc_FunctionTanCuCuCu(const gp_Lin2d&            L1,
                                                         const Geom2dAdaptor_Curve& C2,
                                                         const Geom2dAdaptor_Curve& C3)
{
  Lin1    = L1;
  Curv2   = C2;
  Curv3   = C3;
  TheType = Geom2dGcc_LiCuCu;
}

//==========================================================================

Standard_Integer Geom2dGcc_FunctionTanCuCuCu::NbVariables() const
{
  return 3;
}

Standard_Integer Geom2dGcc_FunctionTanCuCuCu::NbEquations() const
{
  return 3;
}

Standard_Boolean Geom2dGcc_FunctionTanCuCuCu::Value(const math_Vector& X, math_Vector& Fval)
{
  gp_Pnt2d Point1;
  gp_Pnt2d Point2;
  gp_Pnt2d Point3;
  gp_Vec2d Tan1;
  gp_Vec2d Tan2;
  gp_Vec2d Tan3;
  gp_Vec2d D21;
  gp_Vec2d D22;
  gp_Vec2d D23;
  InitDerivative(X, Point1, Point2, Point3, Tan1, Tan2, Tan3, D21, D22, D23);
  // pipj (normes) et PiPj (non Normes).
  Coords2d         P1P2(gp_Vec2d(Point1, Point2).XY());
  Coords2d         P2P3(gp_Vec2d(Point2, Point3).XY());
  Coords2d         P3P1(gp_Vec2d(Point3, Point1).XY());
  Standard_Real NorP1P2 = P1P2.Modulus();
  Standard_Real NorP2P3 = P2P3.Modulus();
  Standard_Real NorP3P1 = P3P1.Modulus();
  Coords2d         p1p2, p2p3, p3p1;
  if (NorP1P2 >= gp1::Resolution())
  {
    p1p2 = P1P2 / NorP1P2;
  }
  else
  {
    p1p2 = Coords2d(0., 0.);
  }
  if (NorP2P3 >= gp1::Resolution())
  {
    p2p3 = P2P3 / NorP2P3;
  }
  else
  {
    p2p3 = Coords2d(0., 0.);
  }
  if (NorP3P1 >= gp1::Resolution())
  {
    p3p1 = P3P1 / NorP3P1;
  }
  else
  {
    p3p1 = Coords2d(0., 0.);
  }
  // derivees premieres non normees Deriv1ui.
  Coords2d Deriv1u1(Tan1.XY());
  Coords2d Deriv1u2(Tan2.XY());
  Coords2d Deriv1u3(Tan3.XY());
  // normales aux courbes.
  Standard_Real nnor1 = Deriv1u1.Modulus();
  Standard_Real nnor2 = Deriv1u2.Modulus();
  Standard_Real nnor3 = Deriv1u3.Modulus();
  Coords2d         Nor1(-Deriv1u1.Y(), Deriv1u1.X());
  Coords2d         Nor2(-Deriv1u2.Y(), Deriv1u2.X());
  Coords2d         Nor3(-Deriv1u3.Y(), Deriv1u3.X());
  Coords2d         nor1, nor2, nor3;
  if (nnor1 >= gp1::Resolution())
  {
    nor1 = Nor1 / nnor1;
  }
  else
  {
    nor1 = Coords2d(0., 0.);
  }
  if (nnor2 >= gp1::Resolution())
  {
    nor2 = Nor2 / nnor2;
  }
  else
  {
    nor2 = Coords2d(0., 0.);
  }
  if (nnor3 >= gp1::Resolution())
  {
    nor3 = Nor3 / nnor3;
  }
  else
  {
    nor3 = Coords2d(0., 0.);
  }
  // determination des signes pour les produits scalaires.
  Standard_Real signe1 = 1.;
  Standard_Real signe2 = 1.;
  Standard_Real signe3 = 1.;
  gp_Pnt2d      Pcenter(Coords2d(Point1.XY() + Point2.XY() + Point3.XY()) / 3.);
  Coords2d         fic1(Pcenter.XY() - Point1.XY());
  Coords2d         fic2(Pcenter.XY() - Point2.XY());
  Coords2d         fic3(Pcenter.XY() - Point3.XY());
  Standard_Real pscal11 = nor1.Dot(fic1);
  Standard_Real pscal22 = nor2.Dot(fic2);
  Standard_Real pscal33 = nor3.Dot(fic3);
  if (pscal11 <= 0.)
  {
    signe1 = -1;
  }
  if (pscal22 <= 0.)
  {
    signe2 = -1;
  }
  if (pscal33 <= 0.)
  {
    signe3 = -1;
  }
  // Fonctions Fui.
  // ==============
  Fval(1) = signe1 * nor1.Dot(p1p2) + signe2 * nor2.Dot(p1p2);
  Fval(2) = signe2 * nor2.Dot(p2p3) + signe3 * nor3.Dot(p2p3);
  Fval(3) = signe3 * nor3.Dot(p3p1) + signe1 * nor1.Dot(p3p1);
  return Standard_True;
}

Standard_Boolean Geom2dGcc_FunctionTanCuCuCu::Derivatives(const math_Vector&, math_Matrix&)
{
#if 0
  gp_Pnt2d Point1;
  gp_Pnt2d Point2;
  gp_Pnt2d Point3;
  gp_Vec2d Tan1;
  gp_Vec2d Tan2;
  gp_Vec2d Tan3;
  gp_Vec2d D21;
  gp_Vec2d D22;
  gp_Vec2d D23;
  InitDerivative(X,Point1,Point2,Point3,Tan1,Tan2,Tan3,D21,D22,D23);
  //derivees premieres non normees Deriv1ui.
  Coords2d Deriv1u1(Tan1.XY());
  Coords2d Deriv1u2(Tan2.XY());
  Coords2d Deriv1u3(Tan3.XY());
  //pipj (normes) et PiPj (non Normes).
  Coords2d P1P2(gp_Vec2d(Point1,Point2).XY());
  Coords2d P2P3(gp_Vec2d(Point2,Point3).XY());
  Coords2d P3P1(gp_Vec2d(Point3,Point1).XY());
  Standard_Real NorP1P2 = P1P2.Modulus();
  Standard_Real NorP2P3 = P2P3.Modulus();
  Standard_Real NorP3P1 = P3P1.Modulus();
  Coords2d p1p2,p2p3,p3p1;
  if (NorP1P2 >= gp1::Resolution()) { p1p2 = P1P2/NorP1P2; }
  else { p1p2 = Coords2d(0.,0.); }
  if (NorP2P3 >= gp1::Resolution()) { p2p3 = P2P3/NorP2P3; }
  else { p2p3 = Coords2d(0.,0.); }
  if (NorP3P1 >= gp1::Resolution()) { p3p1 = P3P1/NorP3P1; }
  else { p3p1 = Coords2d(0.,0.); }
  //normales au courbes normees Nori et non nromees nori et norme des nori.
  Standard_Real nnor1 = Deriv1u1.Modulus();
  Standard_Real nnor2 = Deriv1u2.Modulus();
  Standard_Real nnor3 = Deriv1u3.Modulus();
  Coords2d Nor1(-Deriv1u1.Y(),Deriv1u1.X());
  Coords2d Nor2(-Deriv1u2.Y(),Deriv1u2.X());
  Coords2d Nor3(-Deriv1u3.Y(),Deriv1u3.X());
  Coords2d nor1,nor2,nor3;
  if (nnor1 >= gp1::Resolution()) { nor1 = Nor1/nnor1; }
  else { nor1 = Coords2d(0.,0.); }
  if (nnor2 >= gp1::Resolution()) { nor2 = Nor2/nnor2; }
  else { nor2 = Coords2d(0.,0.); }
  if (nnor3 >= gp1::Resolution()) { nor3 = Nor3/nnor3; }
  else { nor3 = Coords2d(0.,0.); }
  //derivees des normales.
  Coords2d NorD21,NorD22,NorD23;
  NorD21 = Coords2d(-D21.Y(),D21.X());
  NorD22 = Coords2d(-D22.Y(),D22.X());
  NorD23 = Coords2d(-D23.Y(),D23.X());
  //determination des signes pour les produits scalaires.
  Standard_Real signe1 = 1.;
  Standard_Real signe2 = 1.;
  Standard_Real signe3 = 1.;
  Coords2d P = Point1.XY();
  P += Point2.XY();
  P += Point3.XY();
  P /= 3.;
  gp_Pnt2d Pcenter(P);
  Coords2d fic1 = Pcenter.XY();
  fic1 -= Point1.XY();
  Coords2d fic2 = Pcenter.XY();
  fic2 -= Point2.XY();
  Coords2d fic3 = Pcenter.XY();
  fic3 -= Point3.XY();
  Standard_Real pscal11 = nor1.Dot(fic1);
  Standard_Real pscal22 = nor2.Dot(fic2);
  Standard_Real pscal33 = nor3.Dot(fic3);
  if (pscal11 <= 0.) { signe1 = -1; }
  if (pscal22 <= 0.) { signe2 = -1; }
  if (pscal33 <= 0.) { signe3 = -1; }

  // Derivees dFui/uj  1 <= ui <= 3 , 1 <= uj <= 3
  // =============================================
  Standard_Real partie1,partie2;
  if (nnor1 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1 = (signe1*NorD21/nnor1-(Nor1.Dot(NorD21)/(nnor1*nnor1*nnor1))
    *Nor1).Dot(p1p2); }
  if (NorP1P2 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((Deriv1u1.Dot(p1p2)/(NorP1P2*NorP1P2))*P1P2-Deriv1u1/NorP1P2)
    .Dot(signe1*nor1+signe2*nor2); }
  Deriv(1,1) = partie1 + partie2;
  if (nnor2 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=(signe2*NorD22/(nnor2)-(Nor2.Dot(NorD22)/(nnor2*nnor2*nnor2))
    *Nor2).Dot(p1p2); }
  if (NorP1P2 <= gp1::Resolution()) { partie2 = 0.; }
  else{partie2=((-Deriv1u2.Dot(p1p2)/(NorP1P2*NorP1P2))*P1P2+Deriv1u2/NorP1P2)
    .Dot(signe1*nor1+signe2*nor2); }
  Deriv(1,2) = partie1 + partie2;
  Deriv(1,3) = 0.;
  Deriv(2,1) = 0.;
  if (nnor2 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=(signe2*NorD22/(nnor2)-(Nor2.Dot(NorD22)/(nnor2*nnor2*nnor2))
    *Nor2).Dot(p2p3); }
  if (NorP2P3 <= gp1::Resolution()) { partie2 = 0.; }
  else { partie2=((Deriv1u2.Dot(p2p3)/(NorP2P3*NorP2P3))*P2P3-Deriv1u2/NorP2P3)
    .Dot(signe2*nor2+signe3*nor3); }
  Deriv(2,2) = partie1 +partie2;
  if (nnor3 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=(signe3*NorD23/(nnor3)-(Nor3.Dot(NorD23)/(nnor3*nnor3*nnor3))
    *Nor3).Dot(p2p3); }
  if (NorP2P3 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((-Deriv1u3.Dot(p2p3)/(NorP2P3*NorP2P3))*P2P3+Deriv1u3/NorP2P3)
    .Dot(signe2*nor2+signe3*nor3); }
  Deriv(2,3) = partie1 + partie2;
  if (nnor1 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1 =(signe1*NorD21/(nnor1)-(Nor1.Dot(NorD21)/(nnor1*nnor1*nnor1))
    *Nor1).Dot(p3p1); }
  if (NorP3P1 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((-Deriv1u1.Dot(p3p1)/(NorP3P1*NorP3P1))*P3P1+Deriv1u1/NorP3P1)
    .Dot(signe1*nor1+signe3*nor3); }
  Deriv(3,1) = partie1 + partie2;
  Deriv(3,2) = 0.;
  if (nnor3 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=(signe3*NorD23/(nnor3)-(Nor3.Dot(NorD23)/(nnor3*nnor3*nnor3))
    *Nor3).Dot(p3p1); }
  if (NorP3P1 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((Deriv1u3.Dot(p3p1)/(NorP3P1*NorP3P1))*P3P1-Deriv1u3/NorP3P1)
    .Dot(signe1*nor1+signe3*nor3); }
  Deriv(3,3) = partie1+partie2;
#endif
  return Standard_True;
}

Standard_Boolean Geom2dGcc_FunctionTanCuCuCu::Values(const math_Vector&, math_Vector&, math_Matrix&)
{
#if 0
  gp_Pnt2d Point1;
  gp_Pnt2d Point2;
  gp_Pnt2d Point3;
  gp_Vec2d Tan1;
  gp_Vec2d Tan2;
  gp_Vec2d Tan3;
  gp_Vec2d D21;
  gp_Vec2d D22;
  gp_Vec2d D23;
  InitDerivative(X,Point1,Point2,Point3,Tan1,Tan2,Tan3,D21,D22,D23);
  //derivees premieres non normees Deriv1ui.
  Coords2d Deriv1u1(Tan1.XY());
  Coords2d Deriv1u2(Tan2.XY());
  Coords2d Deriv1u3(Tan3.XY());
  //pipj (normes) et PiPj (non Normes).
  Coords2d P1P2(gp_Vec2d(Point1,Point2).XY());
  Coords2d P2P3(gp_Vec2d(Point2,Point3).XY());
  Coords2d P3P1(gp_Vec2d(Point3,Point1).XY());
  Standard_Real NorP1P2 = P1P2.Modulus();
  Standard_Real NorP2P3 = P2P3.Modulus();
  Standard_Real NorP3P1 = P3P1.Modulus();
  Coords2d p1p2,p2p3,p3p1;
  if (NorP1P2 >= gp1::Resolution()) { p1p2 = P1P2/NorP1P2; }
  else { p1p2 = Coords2d(0.,0.); }
  if (NorP2P3 >= gp1::Resolution()) { p2p3 = P2P3/NorP2P3; }
  else { p2p3 = Coords2d(0.,0.); }
  if (NorP3P1 >= gp1::Resolution()) { p3p1 = P3P1/NorP3P1; }
  else { p3p1 = Coords2d(0.,0.); }
  //normales au courbes normees Nori et non nromees nori et norme des nori.
  Standard_Real nnor1 = Deriv1u1.Modulus();
  Standard_Real nnor2 = Deriv1u2.Modulus();
  Standard_Real nnor3 = Deriv1u3.Modulus();
  Coords2d Nor1(-Deriv1u1.Y(),Deriv1u1.X());
  Coords2d Nor2(-Deriv1u2.Y(),Deriv1u2.X());
  Coords2d Nor3(-Deriv1u3.Y(),Deriv1u3.X());
  Coords2d nor1,nor2,nor3;
  if (nnor1 >= gp1::Resolution()) { nor1 = Nor1/nnor1; }
  else { nor1 = Coords2d(0.,0.); }
  if (nnor2 >= gp1::Resolution()) { nor2 = Nor2/nnor2; }
  else { nor2 = Coords2d(0.,0.); }
  if (nnor3 >= gp1::Resolution()) { nor3 = Nor3/nnor3; }
  else { nor3 = Coords2d(0.,0.); }
  //derivees des normales.
  Coords2d NorD21,NorD22,NorD23;
  NorD21 = Coords2d(-D21.Y(),D21.X());
  NorD22 = Coords2d(-D22.Y(),D22.X());
  NorD23 = Coords2d(-D23.Y(),D23.X());
  //determination des signes pour les produits scalaires.
  Standard_Real signe1 = 1.;
  Standard_Real signe2 = 1.;
  Standard_Real signe3 = 1.;
  Coords2d P = Point1.XY();
  P += Point2.XY();
  P += Point3.XY();
  P /= 3.;
  gp_Pnt2d Pcenter(P);
  Coords2d fic1 = Pcenter.XY();
  fic1 -= Point1.XY();
  Coords2d fic2 = Pcenter.XY();
  fic2 -= Point2.XY();
  Coords2d fic3 = Pcenter.XY();
  fic3 -= Point3.XY();
  Standard_Real pscal11 = nor1.Dot(fic1);
  Standard_Real pscal22 = nor2.Dot(fic2);
  Standard_Real pscal33 = nor3.Dot(fic3);
  if (pscal11 <= 0.) { signe1 = -1; }
  if (pscal22 <= 0.) { signe2 = -1; }
  if (pscal33 <= 0.) { signe3 = -1; }

  // Fonctions Fui.
  // ==============
  Fval(1) = signe1*nor1.Dot(p1p2)+signe2*nor2.Dot(p1p2);
  Fval(2) = signe2*nor2.Dot(p2p3)+signe3*nor3.Dot(p2p3);
  Fval(3) = signe3*nor3.Dot(p3p1)+signe1*nor1.Dot(p3p1);
  // Derivees dFui/uj  1 <= ui <= 3 , 1 <= uj <= 3
  // =============================================
  Standard_Real partie1,partie2;
  if (nnor1 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1 = signe1*(NorD21/nnor1-(Nor1.Dot(NorD21)/(nnor1*nnor1*nnor1))
    *Nor1).Dot(p1p2); }
  if (NorP1P2 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((Deriv1u1.Dot(p1p2)/(NorP1P2*NorP1P2))*P1P2-Deriv1u1/NorP1P2)
    .Dot(signe1*nor1+signe2*nor2); }
  Deriv(1,1) = partie1 + partie2;
  if (nnor2 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=signe2*(NorD22/(nnor2)-(Nor2.Dot(NorD22)/(nnor2*nnor2*nnor2))
    *Nor2).Dot(p1p2); }
  if (NorP1P2 <= gp1::Resolution()) { partie2 = 0.; }
  else{partie2=((-Deriv1u2.Dot(p1p2)/(NorP1P2*NorP1P2))*P1P2+Deriv1u2/NorP1P2)
    .Dot(signe1*nor1+signe2*nor2); }
  Deriv(1,2) = partie1 + partie2;
  Deriv(1,3) = 0.;
  Deriv(2,1) = 0.;
  if (nnor2 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=signe2*(NorD22/(nnor2)-(Nor2.Dot(NorD22)/(nnor2*nnor2*nnor2))
    *Nor2).Dot(p2p3); }
  if (NorP2P3 <= gp1::Resolution()) { partie2 = 0.; }
  else { partie2=((Deriv1u2.Dot(p2p3)/(NorP2P3*NorP2P3))*P2P3-Deriv1u2/NorP2P3)
    .Dot(signe2*nor2+signe3*nor3); }
  Deriv(2,2) = partie1 +partie2;
  if (nnor3 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=signe3*(NorD23/(nnor3)-(Nor3.Dot(NorD23)/(nnor3*nnor3*nnor3))
    *Nor3).Dot(p2p3); }
  if (NorP2P3 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((-Deriv1u3.Dot(p2p3)/(NorP2P3*NorP2P3))*P2P3+Deriv1u3/NorP2P3)
    .Dot(signe2*nor2+signe3*nor3); }
  Deriv(2,3) = partie1 + partie2;
  if (nnor1 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1 =signe1*(NorD21/(nnor1)-(Nor1.Dot(NorD21)/(nnor1*nnor1*nnor1))
    *Nor1).Dot(p3p1); }
  if (NorP3P1 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((-Deriv1u1.Dot(p3p1)/(NorP3P1*NorP3P1))*P3P1+Deriv1u1/NorP3P1)
    .Dot(signe1*nor1+signe3*nor3); }
  Deriv(3,1) = partie1 + partie2;
  Deriv(3,2) = 0.;
  if (nnor3 <= gp1::Resolution()) { partie1 = 0.; }
  else { partie1=signe3*(NorD23/(nnor3)-(Nor3.Dot(NorD23)/(nnor3*nnor3*nnor3))
    *Nor3).Dot(p3p1); }
  if (NorP3P1 <= gp1::Resolution()) { partie2 = 0.; }
  else {partie2=((Deriv1u3.Dot(p3p1)/(NorP3P1*NorP3P1))*P3P1-Deriv1u3/NorP3P1)
    .Dot(signe1*nor1+signe3*nor3); }
  Deriv(3,3) = partie1+partie2;
#endif
  return Standard_True;
}
