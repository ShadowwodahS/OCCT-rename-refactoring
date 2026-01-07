// Created on: 1996-02-15
// Created by: Jacques GOUSSARD
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

#include <LocOpe_FindEdgesInFace.hxx>

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTopAdaptor_TopolTool.hxx>
#include <ElCLib.hxx>
#include <ElSLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_MapOfShape.hxx>

//=================================================================================================

void LocOpe_FindEdgesInFace::Set(const TopoShape& Sh, const TopoFace& F)
{
  myShape = Sh;
  myFace  = F;
  myList.Clear();

  TopTools_MapOfShape   M;
  ShapeExplorer       exp, expf;
  Handle(GeomCurve3d)    C;
  Handle(GeomSurface)  S;
  TopLoc_Location       Loc;
  Standard_Real         f, l;
  Handle(TypeInfo) Tc, Ts;
  Standard_Boolean      ToAdd;
  gp_Pln                pl;
  gp_Cylinder           cy;

  constexpr Standard_Real Tol    = Precision::Confusion();
  constexpr Standard_Real TolAng = Precision::Angular();

  S  = BRepInspector::Surface(F);
  Ts = S->DynamicType();
  if (Ts == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    S  = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
    Ts = S->DynamicType();
  }

  if (Ts != STANDARD_TYPE(GeomPlane) && Ts != STANDARD_TYPE(Geom_CylindricalSurface))
  {
    return; // pour le moment
  }

  Handle(BRepAdaptor_Surface) HS = new BRepAdaptor_Surface(myFace);
  BRepTopAdaptor_TopolTool    TPT(HS);

  for (exp.Init(myShape, TopAbs_EDGE); exp.More(); exp.Next())
  {
    ToAdd                  = Standard_False;
    const TopoEdge& edg = TopoDS::Edge(exp.Current());
    if (M.Contains(edg))
    {
      continue;
    }

    for (expf.Init(myFace, TopAbs_EDGE); expf.More(); expf.Next())
    {
      if (expf.Current().IsSame(edg))
      {
        break;
      }
    }
    if (expf.More())
    { // partage d`edge
      M.Add(edg);
      myList.Append(edg);
      continue;
    }

    C  = BRepInspector::Curve(edg, Loc, f, l);
    C  = Handle(GeomCurve3d)::DownCast(C->Transformed(Loc.Transformation()));
    Tc = C->DynamicType();
    if (Tc == STANDARD_TYPE(Geom_TrimmedCurve))
    {
      C  = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
      Tc = C->DynamicType();
    }
    if (Tc != STANDARD_TYPE(GeomLine) && Tc != STANDARD_TYPE(GeomCircle))
    {
      continue; // pour le moment
    }
    if (Ts == STANDARD_TYPE(GeomPlane))
    {
      pl = Handle(GeomPlane)::DownCast(S)->Pln();
    }
    else
    {
      cy = Handle(Geom_CylindricalSurface)::DownCast(S)->Cylinder();
    }

    if (Tc == STANDARD_TYPE(GeomLine))
    {
      gp_Lin li = Handle(GeomLine)::DownCast(C)->Lin();
      if (Ts == STANDARD_TYPE(GeomPlane))
      {
        if (pl.Contains(li, Tol, TolAng))
        {
          ToAdd = Standard_True;
        }
      }
      else
      { // Ts == STANDARD_TYPE(Geom_CylindricalSurface)
        if (cy.Axis().IsParallel(li.Position(), TolAng)
            && Abs(li.Distance(cy.Location()) - cy.Radius()) < Tol)
        {
          ToAdd = Standard_True;
        }
      }
    }
    else
    { // Tt == STANDARD_TYPE(GeomCircle)
      gp_Circ ci = Handle(GeomCircle)::DownCast(C)->Circ();
      if (Ts == STANDARD_TYPE(GeomPlane))
      {
        if (pl.Position().IsCoplanar(ci.Position(), Tol, TolAng))
        {
          ToAdd = Standard_True;
        }
      }
      else
      {
        if (Abs(cy.Radius() - ci.Radius()) < Tol && cy.Axis().IsCoaxial(ci.Axis(), TolAng, Tol))
        {
          ToAdd = Standard_True;
        }
      }
    }

    if (ToAdd)
    {
      // On classifie 3 points.
      Point3d        p[3];
      Standard_Real U, V;
      p[0] = C->Value(f);
      p[1] = C->Value(l);
      p[2] = C->Value((f + l) / 2.);
      //      for (Standard_Integer i=0; i<3; i++) {
      Standard_Integer i;
      for (i = 0; i < 3; i++)
      {
        if (Ts == STANDARD_TYPE(GeomPlane))
        {
          ElSLib1::Parameters(pl, p[i], U, V);
        }
        else
        {
          ElSLib1::Parameters(cy, p[i], U, V);
        }
        if (TPT.Classify(gp_Pnt2d(U, V), Precision::Confusion()) == TopAbs_OUT)
        {
          break;
        }
      }
      if (i >= 3)
      {
        M.Add(edg);
        myList.Append(edg);
      }
    }
  }
}
