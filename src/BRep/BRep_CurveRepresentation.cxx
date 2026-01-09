// Created on: 1993-07-06
// Created by: Remi LEQUETTE
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

#include <BRep_CurveRepresentation.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Poly_Polygon2D.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_DomainError.hxx>
#include <Standard_Type.hxx>
#include <TopLoc_Location.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRep_CurveRepresentation, RefObject)

//=================================================================================================

BRep_CurveRepresentation::BRep_CurveRepresentation(const TopLoc_Location& L)
    : myLocation(L)
{
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurve3D() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurveOnSurface() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurveOnClosedSurface() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsRegularity() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsCurveOnSurface(const Handle(GeomSurface)&,
                                                            const TopLoc_Location&) const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsRegularity(const Handle(GeomSurface)&,
                                                        const Handle(GeomSurface)&,
                                                        const TopLoc_Location&,
                                                        const TopLoc_Location&) const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygon3D() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnTriangulation() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnTriangulation(
  const Handle(MeshTriangulation)&,
  const TopLoc_Location&) const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnClosedTriangulation() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnClosedSurface() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnSurface() const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRep_CurveRepresentation::IsPolygonOnSurface(const Handle(GeomSurface)&,
                                                              const TopLoc_Location&) const
{
  return Standard_False;
}

//=================================================================================================

const Handle(GeomCurve3d)& BRep_CurveRepresentation::Curve3D() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Curve3D(const Handle(GeomCurve3d)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(GeomSurface)& BRep_CurveRepresentation::Surface() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(GeomCurve2d)& BRep_CurveRepresentation::PCurve() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(GeomCurve2d)& BRep_CurveRepresentation::PCurve2() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PCurve(const Handle(GeomCurve2d)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PCurve2(const Handle(GeomCurve2d)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const GeomAbs_Shape& BRep_CurveRepresentation::Continuity() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Continuity(const GeomAbs_Shape)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(GeomSurface)& BRep_CurveRepresentation::Surface2() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const TopLoc_Location& BRep_CurveRepresentation::Location2() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_Polygon3D)& BRep_CurveRepresentation::Polygon3D() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Polygon3D(const Handle(Poly_Polygon3D)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Polygon2D2)& BRep_CurveRepresentation::Polygon() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Polygon(const Handle(Polygon2D2)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_PolygonOnTriangulation)& BRep_CurveRepresentation::PolygonOnTriangulation2() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PolygonOnTriangulation2(const Handle(Poly_PolygonOnTriangulation)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::PolygonOnTriangulation(const Handle(Poly_PolygonOnTriangulation)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Poly_PolygonOnTriangulation)& BRep_CurveRepresentation::PolygonOnTriangulation() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(MeshTriangulation)& BRep_CurveRepresentation::Triangulation() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

const Handle(Polygon2D2)& BRep_CurveRepresentation::Polygon2() const
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::Polygon2(const Handle(Polygon2D2)&)
{
  throw Standard_DomainError("BRep_CurveRepresentation");
}

//=================================================================================================

void BRep_CurveRepresentation::DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth) const
{
  OCCT_DUMP_TRANSIENT_CLASS_BEGIN(theOStream)

  OCCT_DUMP_FIELD_VALUES_DUMPED(theOStream, theDepth, &myLocation)
}
