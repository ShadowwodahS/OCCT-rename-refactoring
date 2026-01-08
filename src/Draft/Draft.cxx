// Created on: 1995-02-20
// Created by: Jacques GOUSSARD
// Copyright (c) 1995-1999 Matra Datavision
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

#include <Draft.hxx>

#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <ElSLib.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <gp_Dir.hxx>
#include <Precision.hxx>
#include <Standard_DomainError.hxx>
#include <TopoDS_Face.hxx>

//=================================================================================================

Standard_Real Draft1::Angle(const TopoFace& F, const Dir3d& D)
{

  TopLoc_Location       Lo;
  Handle(GeomSurface)  S     = BRepInspector::Surface(F, Lo);
  Handle(TypeInfo) TypeS = S->DynamicType();
  if (TypeS == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S     = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
    TypeS = S->DynamicType();
  }

  if (TypeS != STANDARD_TYPE(GeomPlane) && TypeS != STANDARD_TYPE(Geom_ConicalSurface)
      && TypeS != STANDARD_TYPE(Geom_CylindricalSurface))
  {
    throw Standard_DomainError();
  }

  Standard_Real Angle;
  S = Handle(GeomSurface)::DownCast(S->Transformed(Lo.Transformation()));
  if (TypeS == STANDARD_TYPE(GeomPlane))
  {
    Ax3 ax3(Handle(GeomPlane)::DownCast(S)->Pln().Position1());
    Vector3d normale(ax3.Direction());
    if (!ax3.Direct())
    {
      normale.Reverse();
    }
    if (F.Orientation() == TopAbs_REVERSED)
    {
      normale.Reverse();
    }
    Angle = ASin(normale.Dot(D));
  }
  else if (TypeS == STANDARD_TYPE(Geom_CylindricalSurface))
  {
    Cylinder1   Cy(Handle(Geom_CylindricalSurface)::DownCast(S)->Cylinder());
    Standard_Real testdir = D.Dot(Cy.Axis().Direction());
    if (Abs(testdir) <= 1. - Precision::Angular())
    {
      throw Standard_DomainError();
    }
    Angle = 0.;
  }
  else
  { // STANDARD_TYPE(Geom_ConicalSurface)
    Cone1       Co(Handle(Geom_ConicalSurface)::DownCast(S)->Cone());
    Standard_Real testdir = D.Dot(Co.Axis().Direction());
    if (Abs(testdir) <= 1. - Precision::Angular())
    {
      throw Standard_DomainError();
    }
    Standard_Real umin, umax, vmin, vmax;
    BRepTools1::UVBounds(F, umin, umax, vmin, vmax);
    Point3d ptbid;
    Vector3d d1u, d1v;
    ElSLib1::D1(umin + umax / 2., vmin + vmax / 2., Co, ptbid, d1u, d1v);
    d1u.Cross(d1v);
    d1u.Normalize();
    if (F.Orientation() == TopAbs_REVERSED)
    {
      d1u.Reverse();
    }
    Angle = ASin(d1u.Dot(D));
  }
  return Angle;
}
