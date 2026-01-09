// Created on: 1993-03-10
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

// JCV 09/07/92 portage sur C1

#include <Geom_Geometry.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Geometry3, RefObject)

typedef Geometry3 Geometry1;
typedef Point3d        Pnt;
typedef Vector3d        Vec;
typedef Axis3d        Ax1;
typedef Frame3d        Ax2;
typedef Transform3d       Trsf;

Handle(Geometry3) Geometry3::Copy() const
{

  Handle(Geometry3) G;
  throw Standard_ConstructionError();
}

void Geometry3::Mirror(const Point3d& P)
{

  Trsf T;
  T.SetMirror(P);
  Transform(T);
}

void Geometry3::Mirror(const Axis3d& A1)
{

  Trsf T;
  T.SetMirror(A1);
  Transform(T);
}

void Geometry3::Mirror(const Frame3d& A2)
{

  Trsf T;
  T.SetMirror(A2);
  Transform(T);
}

void Geometry3::Rotate(const Axis3d& A1, const Standard_Real Ang)
{

  Trsf T;
  T.SetRotation(A1, Ang);
  Transform(T);
}

void Geometry3::Scale(const Point3d& P, const Standard_Real S)
{

  Trsf T;
  T.SetScale(P, S);
  Transform(T);
}

void Geometry3::Translate(const Vector3d& V)
{

  Trsf T;
  T.SetTranslation(V);
  Transform(T);
}

void Geometry3::Translate(const Point3d& P1, const Point3d& P2)
{

  Vec V(P1, P2);
  Translate(V);
}

Handle(Geometry3) Geometry3::Mirrored(const Point3d& P) const
{
  Handle(Geometry3) G = Copy();
  G->Mirror(P);
  return G;
}

Handle(Geometry3) Geometry3::Mirrored(const Axis3d& A1) const
{
  Handle(Geometry3) G = Copy();
  G->Mirror(A1);
  return G;
}

Handle(Geometry3) Geometry3::Mirrored(const Frame3d& A2) const
{
  Handle(Geometry3) G = Copy();
  G->Mirror(A2);
  return G;
}

Handle(Geometry3) Geometry3::Rotated(const Axis3d& A1, const Standard_Real Ang) const
{
  Handle(Geometry3) G = Copy();
  G->Rotate(A1, Ang);
  return G;
}

Handle(Geometry3) Geometry3::Scaled(const Point3d& P, const Standard_Real S) const
{
  Handle(Geometry3) G = Copy();
  G->Scale(P, S);
  return G;
}

Handle(Geometry3) Geometry3::Transformed(const Transform3d& T) const
{
  Handle(Geometry3) G = Copy();
  G->Transform(T);
  return G;
}

Handle(Geometry3) Geometry3::Translated(const Vector3d& V) const
{
  Handle(Geometry3) G = Copy();
  G->Translate(V);
  return G;
}

Handle(Geometry3) Geometry3::Translated(const Point3d& P1, const Point3d& P2) const
{
  Handle(Geometry3) G = Copy();
  G->Translate(P1, P2);
  return G;
}

void Geometry3::DumpJson(Standard_OStream& theOStream, Standard_Integer) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)
}
