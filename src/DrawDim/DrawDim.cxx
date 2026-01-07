// Created on: 1996-01-10
// Created by: Denis PASCAL
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

#include <DrawDim.hxx>

#include <BRep_Tool.hxx>
#include <Draw_Appli.hxx>
#include <Draw_Text3D.hxx>
#include <ElCLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <gp_Circ.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

#ifdef _WIN32
Standard_IMPORT DrawViewer dout;
#endif

//=================================================================================================

void DrawDim1::AllCommands(DrawInterpreter& theCommands)
{
  PlanarDimensionCommands(theCommands);
}

//=================================================================================================

void DrawDim1::DrawShapeName(const TopoShape& ashape, const Standard_CString aname)
{
  Point3d                  position;
  AsciiString1 t(" ");
  switch (ashape.ShapeType())
  {
    case TopAbs_EDGE: {
      Standard_Real      f, l, parameter;
      Handle(GeomCurve3d) curve = BRepInspector::Curve(TopoDS::Edge(ashape), f, l);
      if (curve->IsKind(STANDARD_TYPE(GeomLine)))
      {
        parameter = (f + l) / 2.;
        position  = ElCLib1::Value(parameter, Handle(GeomLine)::DownCast(curve)->Lin());
      }
      else if (curve->IsKind(STANDARD_TYPE(GeomCircle)))
      {
        parameter = (f + l) / 2.;
        if (f > l)
          parameter = parameter + M_PI;
        position = ElCLib1::Value(parameter, Handle(GeomCircle)::DownCast(curve)->Circ());
      }
    }
    break;
    case TopAbs_VERTEX: {
      position = BRepInspector::Pnt(TopoDS::Vertex(ashape));
    }
    break;
    default:
      break;
  }
  t += aname; // Name();
  Handle(Draw_Text3D) text = new Draw_Text3D(position, t.ToCString(), Draw_blanc);
  dout << text;
}

//=================================================================================================

Standard_Boolean DrawDim1::Pln(const TopoFace& f, gp_Pln& p)
{
  Handle(GeomPlane) P = Handle(GeomPlane)::DownCast(BRepInspector::Surface(f));
  if (!P.IsNull())
  {
    p = P->Pln();
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean DrawDim1::Lin(const TopoEdge& e,
                              gp_Lin&            l,
                              Standard_Boolean&  infinite,
                              Standard_Real&     first,
                              Standard_Real&     last)
{
  Standard_Real     f1, l1;
  Handle(GeomLine) L = Handle(GeomLine)::DownCast(BRepInspector::Curve(e, f1, l1));
  if (!L.IsNull())
  {
    TopoVertex vf, vl;
    TopExp1::Vertices(TopoDS::Edge(e), vf, vl);
    if (vf.IsNull() && vl.IsNull())
    {
      infinite = Standard_True;
      l        = L->Lin();
      return Standard_True;
    }
    else if (vf.IsNull() || vl.IsNull())
    {
      throw Standard_DomainError("DrawDim1::Lin : semi infinite edge");
    }
    else
    {
      l        = L->Lin();
      infinite = Standard_True;
      first    = f1;
      last     = l1;
      return Standard_True;
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean DrawDim1::Circ(const TopoEdge& e,
                               gp_Circ&           c,
                               Standard_Real&     first,
                               Standard_Real&     last)
{
  Standard_Real       f1, l1;
  Handle(GeomCircle) C = Handle(GeomCircle)::DownCast(BRepInspector::Curve(e, f1, l1));
  if (!C.IsNull())
  {
    c     = C->Circ();
    first = f1;
    last  = l1;
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Point3d DrawDim1::Nearest(const TopoShape& ashape, const Point3d& apoint)
{
  Standard_Real   dist = RealLast();
  Standard_Real   curdist;
  Point3d          result;
  Point3d          curpnt;
  ShapeExplorer explo(ashape, TopAbs_VERTEX);
  while (explo.More())
  {
    curpnt  = BRepInspector::Pnt(TopoDS::Vertex(explo.Current()));
    curdist = apoint.Distance(curpnt);
    if (curdist < dist)
    {
      result = curpnt;
      dist   = curdist;
    }
    explo.Next();
  }
  return result;
}
