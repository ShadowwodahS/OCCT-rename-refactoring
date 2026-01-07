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

#include <DBRep.hxx>
#include <DrawTrSurf.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Geometry.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <Interface_Macros.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>
#include <XSDRAW_Vars.hxx>

IMPLEMENT_STANDARD_RTTIEXT(XSDRAW_Vars, XSControl_Vars)

XSDRAW_Vars::XSDRAW_Vars() {}

void XSDRAW_Vars::Set(const Standard_CString name, const Handle(RefObject)& val)
{
  // char* nam = name;
  // selon type
  DeclareAndCast(Geom_Geometry, geom, val);
  if (!geom.IsNull())
  {
    DrawTrSurf1::Set(name, geom);
    return;
  }
  DeclareAndCast(GeomCurve2d, g2d, val);
  if (!g2d.IsNull())
  {
    DrawTrSurf1::Set(name, geom);
    return;
  }
  //  ??
}

/*
Handle(RefObject)  XSDRAW_Vars::Get (const Standard_CString name) const
{
  Handle(RefObject) val;
  if (!thevars->GetItem (name,val)) val.Nullify();
  return val;
}
*/

Handle(Geom_Geometry) XSDRAW_Vars::GetGeom(Standard_CString& name) const
{ // char* nam = name;
  return DrawTrSurf1::Get(name);
}

Handle(GeomCurve2d) XSDRAW_Vars::GetCurve2d(Standard_CString& name) const
{ // char* nam = name;
  return DrawTrSurf1::GetCurve2d(name);
}

Handle(GeomCurve3d) XSDRAW_Vars::GetCurve(Standard_CString& name) const
{ // char* nam = name;
  return DrawTrSurf1::GetCurve(name);
}

Handle(GeomSurface) XSDRAW_Vars::GetSurface(Standard_CString& name) const
{ // char* nam = name;
  return DrawTrSurf1::GetSurface(name);
}

void XSDRAW_Vars::SetPoint(const Standard_CString name, const Point3d& val)
{
  // char* nam = name;
  DrawTrSurf1::Set(name, val);
}

Standard_Boolean XSDRAW_Vars::GetPoint(Standard_CString& name, Point3d& pnt) const
{ // char* nam = name;
  return DrawTrSurf1::GetPoint(name, pnt);
}

void XSDRAW_Vars::SetPoint2d(const Standard_CString name, const gp_Pnt2d& val)
{
  // char* nam = name;
  DrawTrSurf1::Set(name, val);
}

Standard_Boolean XSDRAW_Vars::GetPoint2d(Standard_CString& name, gp_Pnt2d& pnt) const
{
  // char* nam = name;
  return DrawTrSurf1::GetPoint2d(name, pnt);
}

void XSDRAW_Vars::SetShape(const Standard_CString name, const TopoShape& val)
{
  DBRep1::Set(name, val);
}

TopoShape XSDRAW_Vars::GetShape(Standard_CString& name) const
{
  // char* nam = name;
  return DBRep1::Get(name);
}
