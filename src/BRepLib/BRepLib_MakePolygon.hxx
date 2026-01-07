// Created on: 1993-07-29
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

#ifndef _BRepLib_MakePolygon_HeaderFile
#define _BRepLib_MakePolygon_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepLib_MakeShape.hxx>
class Point3d;
class TopoWire;

//! Class to build polygonal wires.
//!
//! A polygonal wire may be build from
//!
//! - 2,4,3 points.
//!
//! - 2,3,4 vertices.
//!
//! - any number of points.
//!
//! - any number of vertices.
//!
//! When a point or vertex is added to the  polygon if
//! it is identic  to the previous  point no  edge  is
//! built. The method added can be used to test it.
class BRepLib_MakePolygon : public BRepLib_MakeShape
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates an empty MakePolygon.
  Standard_EXPORT BRepLib_MakePolygon();

  Standard_EXPORT BRepLib_MakePolygon(const Point3d& P1, const Point3d& P2);

  Standard_EXPORT BRepLib_MakePolygon(const Point3d&          P1,
                                      const Point3d&          P2,
                                      const Point3d&          P3,
                                      const Standard_Boolean Close = Standard_False);

  Standard_EXPORT BRepLib_MakePolygon(const Point3d&          P1,
                                      const Point3d&          P2,
                                      const Point3d&          P3,
                                      const Point3d&          P4,
                                      const Standard_Boolean Close = Standard_False);

  Standard_EXPORT BRepLib_MakePolygon(const TopoVertex& V1, const TopoVertex& V2);

  Standard_EXPORT BRepLib_MakePolygon(const TopoVertex&   V1,
                                      const TopoVertex&   V2,
                                      const TopoVertex&   V3,
                                      const Standard_Boolean Close = Standard_False);

  Standard_EXPORT BRepLib_MakePolygon(const TopoVertex&   V1,
                                      const TopoVertex&   V2,
                                      const TopoVertex&   V3,
                                      const TopoVertex&   V4,
                                      const Standard_Boolean Close = Standard_False);

  Standard_EXPORT void Add(const Point3d& P);

  Standard_EXPORT void Add(const TopoVertex& V);

  //! Returns  True if  the last   vertex  or point  was
  //! successfully added.
  Standard_EXPORT Standard_Boolean Added() const;

  Standard_EXPORT void Close();

  Standard_EXPORT const TopoVertex& FirstVertex() const;

  Standard_EXPORT const TopoVertex& LastVertex() const;

  //! Returns the last edge added to the polygon.
  Standard_EXPORT const TopoEdge& Edge() const;
  Standard_EXPORT                    operator TopoEdge() const;

  Standard_EXPORT const TopoWire& Wire();
  Standard_EXPORT                    operator TopoWire();

protected:
private:
  TopoVertex myFirstVertex;
  TopoVertex myLastVertex;
  TopoEdge   myEdge;
};

#endif // _BRepLib_MakePolygon_HeaderFile
