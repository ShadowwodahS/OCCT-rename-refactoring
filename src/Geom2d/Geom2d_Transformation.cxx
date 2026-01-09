// Created on: 1993-03-24
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

#include <Geom2d_Transformation.hxx>
#include <gp_Ax2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf2d.hxx>
#include <gp_Vec2d.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Transformation2d, RefObject)

typedef Transformation2d Transformation;
typedef gp_Ax2d               Ax2d;
typedef gp_Pnt2d              Pnt2d;
typedef gp_TrsfForm           TrsfForm;
typedef gp_Vec2d              Vec2d;

Handle(Transformation2d) Transformation2d::Copy() const
{

  Handle(Transformation2d) T;
  T = new Transformation(gpTrsf2d);
  return T;
}

Transformation2d::Transformation2d() {}

Transformation2d::Transformation2d(const Transform2d& T)
    : gpTrsf2d(T)
{
}

Handle(Transformation2d) Transformation2d::Inverted() const
{

  return new Transformation(gpTrsf2d.Inverted());
}

Handle(Transformation2d) Transformation2d::Multiplied(

  const Handle(Transformation2d)& Other) const
{

  return new Transformation(gpTrsf2d.Multiplied(Other->Trsf2d()));
}

Handle(Transformation2d) Transformation2d::Powered(const Standard_Integer N) const
{

  Transform2d Temp = gpTrsf2d;
  Temp.Power(N);
  return new Transformation(Temp);
}

void Transformation2d::SetMirror(const gp_Pnt2d& P)
{

  gpTrsf2d.SetMirror(P);
}

void Transformation2d::SetMirror(const gp_Ax2d& A)
{

  gpTrsf2d.SetMirror(A);
}

void Transformation2d::SetRotation(const gp_Pnt2d& P, const Standard_Real Ang)
{

  gpTrsf2d.SetRotation(P, Ang);
}

void Transformation2d::SetScale(const gp_Pnt2d& P, const Standard_Real S)
{

  gpTrsf2d.SetScale(P, S);
}

void Transformation2d::SetTransformation(const gp_Ax2d& ToAxis)
{

  gpTrsf2d.SetTransformation(ToAxis);
}

void Transformation2d::SetTransformation(const gp_Ax2d& FromAxis1, const gp_Ax2d& ToAxis2)
{

  gpTrsf2d.SetTransformation(FromAxis1, ToAxis2);
}

void Transformation2d::SetTranslation(const gp_Vec2d& V)
{

  gpTrsf2d.SetTranslation(V);
}

void Transformation2d::SetTranslation(const gp_Pnt2d& P1, const gp_Pnt2d& P2)
{

  gpTrsf2d.SetTranslation(P1, P2);
}

void Transformation2d::SetTrsf2d(const Transform2d& T)
{
  gpTrsf2d = T;
}

Standard_Boolean Transformation2d::IsNegative() const
{

  return gpTrsf2d.IsNegative();
}

TrsfForm Transformation2d::Form() const
{
  return gpTrsf2d.Form();
}

Standard_Real Transformation2d::ScaleFactor() const
{

  return gpTrsf2d.ScaleFactor();
}

Transform2d Transformation2d::Trsf2d() const
{
  return gpTrsf2d;
}

Standard_Real Transformation2d::Value(const Standard_Integer Row,
                                           const Standard_Integer Col) const
{

  return gpTrsf2d.Value(Row, Col);
}

void Transformation2d::Invert()
{
  gpTrsf2d.Invert();
}

void Transformation2d::Transforms(Standard_Real& X, Standard_Real& Y) const
{

  gpTrsf2d.Transforms(X, Y);
}

void Transformation2d::Multiply(const Handle(Transformation2d)& Other)
{

  gpTrsf2d.Multiply(Other->Trsf2d());
}

void Transformation2d::Power(const Standard_Integer N)
{
  gpTrsf2d.Power(N);
}

void Transformation2d::PreMultiply(const Handle(Transformation2d)& Other)
{

  gpTrsf2d.PreMultiply(Other->Trsf2d());
}
