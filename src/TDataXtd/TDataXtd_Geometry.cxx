// Created on: 2009-04-06
// Copyright (c) 2009-2014 OPEN CASCADE SAS
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

#include <BRep_Tool.hxx>
#include <Geom_CartesianPoint.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Ax1.hxx>
#include <gp_Circ.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <Standard_Type.hxx>
#include <TDataXtd.hxx>
#include <TDataXtd_Geometry.hxx>
#include <TDF_Attribute.hxx>
#include <TDF_Label.hxx>
#include <TDF_RelocationTable.hxx>
#include <TNaming_Tool.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TDataXtd_Geometry, TDF_Attribute)

//=================================================================================================

const Standard_GUID& TDataXtd_Geometry::GetID()
{
  static Standard_GUID TDataXtd_GeometryID("2a96b604-ec8b-11d0-bee7-080009dc3333");
  return TDataXtd_GeometryID;
}

//=================================================================================================

Handle(TDataXtd_Geometry) TDataXtd_Geometry::Set(const DataLabel& L)
{
  Handle(TDataXtd_Geometry) A;
  if (!L.FindAttribute(TDataXtd_Geometry::GetID(), A))
  {
    A = new TDataXtd_Geometry();
    //    A->SetType(TDataXtd_ANY_GEOM);
    L.AddAttribute(A);
  }
  return A;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Point(const DataLabel& L, Point3d& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Point(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Point(const Handle(ShapeAttribute)& NS, Point3d& G)
{
  const TopoShape& shape = Tool11::GetShape(NS);
  if (shape.IsNull())
    return Standard_False;
  if (shape.ShapeType() == TopAbs_VERTEX)
  {
    const TopoVertex& vertex = TopoDS::Vertex(shape);
    G                           = BRepInspector::Pnt(TopoDS::Vertex(vertex));
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Axis(const DataLabel& L, Axis3d& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Axis(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Axis(const Handle(ShapeAttribute)& NS, Axis3d& G)
{
  gp_Lin lin;
  if (Line(NS, lin))
  {
    G = lin.Position1();
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Line(const DataLabel& L, gp_Lin& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Line(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Line(const Handle(ShapeAttribute)& NS, gp_Lin& G)
{
  const TopoShape& shape = Tool11::GetShape(NS);
  if (shape.IsNull())
    return Standard_False;
  if (shape.ShapeType() == TopAbs_EDGE)
  {
    const TopoEdge& edge = TopoDS::Edge(shape);
    Standard_Real      first, last;
    // TopLoc_Location loc;
    Handle(GeomCurve3d) curve = BRepInspector::Curve(edge, first, last);
    if (!curve.IsNull())
    {
      if (curve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
        curve = (Handle(Geom_TrimmedCurve)::DownCast(curve))->BasisCurve();
      Handle(GeomLine) C = Handle(GeomLine)::DownCast(curve);
      if (!C.IsNull())
      {
        G = C->Lin();
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Circle(const DataLabel& L, gp_Circ& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Circle(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Circle(const Handle(ShapeAttribute)& NS, gp_Circ& G)
{
  const TopoShape& shape = Tool11::GetShape(NS);
  if (shape.IsNull())
    return Standard_False;
  if (shape.ShapeType() == TopAbs_EDGE)
  {
    const TopoEdge& edge = TopoDS::Edge(shape);
    Standard_Real      first, last;
    // TopLoc_Location loc;
    Handle(GeomCurve3d) curve = BRepInspector::Curve(edge, first, last);
    if (!curve.IsNull())
    {
      if (curve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
        curve = (Handle(Geom_TrimmedCurve)::DownCast(curve))->BasisCurve();
      Handle(GeomCircle) C = Handle(GeomCircle)::DownCast(curve);
      if (!C.IsNull())
      {
        G = C->Circ();
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Ellipse(const DataLabel& L, gp_Elips& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Ellipse(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Ellipse(const Handle(ShapeAttribute)& NS, gp_Elips& G)
{
  const TopoShape& shape = Tool11::GetShape(NS);
  if (shape.IsNull())
    return Standard_False;
  if (shape.ShapeType() == TopAbs_EDGE)
  {
    const TopoEdge& edge = TopoDS::Edge(shape);
    Standard_Real      first, last;
    Handle(GeomCurve3d) curve = BRepInspector::Curve(edge, first, last);
    if (!curve.IsNull())
    {
      if (curve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
        curve = (Handle(Geom_TrimmedCurve)::DownCast(curve))->BasisCurve();
      Handle(Geom_Ellipse) C = Handle(Geom_Ellipse)::DownCast(curve);
      if (!C.IsNull())
      {
        G = C->Elips();
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Plane1(const DataLabel& L, gp_Pln& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Plane1(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Plane1(const Handle(ShapeAttribute)& NS, gp_Pln& G)
{
  const TopoShape& shape = Tool11::GetShape(NS);
  if (shape.IsNull())
    return Standard_False;
  if (shape.ShapeType() == TopAbs_FACE)
  {
    const TopoFace&   face    = TopoDS::Face(shape);
    Handle(GeomSurface) surface = BRepInspector::Surface(face);
    if (!surface.IsNull())
    {
      if (surface->IsInstance(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
        surface = Handle(Geom_RectangularTrimmedSurface)::DownCast(surface)->BasisSurface();
      Handle(GeomPlane) S = Handle(GeomPlane)::DownCast(surface);
      if (!S.IsNull())
      {
        G = S->Pln();
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Cylinder(const DataLabel& L, Cylinder1& G)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Cylinder(NS, G);
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TDataXtd_Geometry::Cylinder(const Handle(ShapeAttribute)& NS, Cylinder1& G)
{
  const TopoShape& shape = Tool11::GetShape(NS);
  if (shape.IsNull())
    return Standard_False;
  if (shape.ShapeType() == TopAbs_FACE)
  {
    const TopoFace&   face    = TopoDS::Face(shape);
    Handle(GeomSurface) surface = BRepInspector::Surface(face);
    if (!surface.IsNull())
    {
      if (surface->IsInstance(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
        surface = Handle(Geom_RectangularTrimmedSurface)::DownCast(surface)->BasisSurface();
      Handle(Geom_CylindricalSurface) S = Handle(Geom_CylindricalSurface)::DownCast(surface);
      if (!S.IsNull())
      {
        G = S->Cylinder();
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

TDataXtd_GeometryEnum TDataXtd_Geometry::Type(const DataLabel& L)
{
  Handle(ShapeAttribute) NS;
  if (L.FindAttribute(ShapeAttribute::GetID(), NS))
  {
    return Type(NS);
  }
  return TDataXtd_ANY_GEOM;
}

//=================================================================================================

TDataXtd_GeometryEnum TDataXtd_Geometry::Type(const Handle(ShapeAttribute)& NS)
{
  TDataXtd_GeometryEnum type(TDataXtd_ANY_GEOM);
  const TopoShape&   shape = Tool11::GetShape(NS);
  switch (shape.ShapeType())
  {
    case TopAbs_VERTEX: {
      type = TDataXtd_POINT;
      break;
    }
    case TopAbs_EDGE: {
      const TopoEdge& edge = TopoDS::Edge(shape);
      Standard_Real      first, last;
      // TopLoc_Location loc;
      Handle(GeomCurve3d) curve = BRepInspector::Curve(edge, first, last);
      if (!curve.IsNull())
      {
        if (curve->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
        {
          curve = (Handle(Geom_TrimmedCurve)::DownCast(curve))->BasisCurve();
        }
        if (curve->IsInstance(STANDARD_TYPE(GeomLine)))
        {
          type = TDataXtd_LINE;
        }
        else if (curve->IsInstance(STANDARD_TYPE(GeomCircle)))
        {
          type = TDataXtd_CIRCLE;
        }
        else if (curve->IsInstance(STANDARD_TYPE(Geom_Ellipse)))
        {
          type = TDataXtd_ELLIPSE;
        }
      }
#ifdef OCCT_DEBUG
      else
      {
        throw ExceptionBase("curve Null dans TDataXtd_Geometry");
      }
#endif
      break;
    }
    case TopAbs_FACE: {
      const TopoFace&   face    = TopoDS::Face(shape);
      Handle(GeomSurface) surface = BRepInspector::Surface(face);
      if (!surface.IsNull())
      {
        if (surface->IsInstance(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
        {
          surface = Handle(Geom_RectangularTrimmedSurface)::DownCast(surface)->BasisSurface();
        }
        if (surface->IsInstance(STANDARD_TYPE(Geom_CylindricalSurface)))
        {
          type = TDataXtd_CYLINDER;
        }
        else if (surface->IsInstance(STANDARD_TYPE(GeomPlane)))
        {
          type = TDataXtd_PLANE;
        }
      }
#ifdef OCCT_DEBUG
      else
      {
        throw ExceptionBase("surface Null dans TDataXtd_Geometry");
      }
#endif
      break;
    }
    default:
      break;
  }
  return type;
}

//=================================================================================================

TDataXtd_Geometry::TDataXtd_Geometry()
    : myType(TDataXtd_ANY_GEOM)
{
}

//=================================================================================================

TDataXtd_GeometryEnum TDataXtd_Geometry::GetType() const
{
  return myType;
}

//=================================================================================================

void TDataXtd_Geometry::SetType(const TDataXtd_GeometryEnum G)
{
  // OCC2932 correction
  if (myType == G)
    return;

  Backup();
  myType = G;
}

//=================================================================================================

const Standard_GUID& TDataXtd_Geometry::ID() const
{
  return GetID();
}

//=================================================================================================

Handle(TDF_Attribute) TDataXtd_Geometry::NewEmpty() const
{
  return new TDataXtd_Geometry();
}

//=================================================================================================

void TDataXtd_Geometry::Restore(const Handle(TDF_Attribute)& With)
{
  myType = Handle(TDataXtd_Geometry)::DownCast(With)->GetType();
}

//=================================================================================================

void TDataXtd_Geometry::Paste(const Handle(TDF_Attribute)& Into,
                              const Handle(RelocationTable1)&) const
{
  Handle(TDataXtd_Geometry)::DownCast(Into)->SetType(myType);
}

//=================================================================================================

Standard_OStream& TDataXtd_Geometry::Dump(Standard_OStream& anOS) const
{
  anOS << "Geometry1 ";
  TDataXtd1::Print(GetType(), anOS);
  return anOS;
}
