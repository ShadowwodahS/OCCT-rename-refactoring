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

#ifndef _DrawDim_PlanarDistance_HeaderFile
#define _DrawDim_PlanarDistance_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <DrawDim_PlanarDimension.hxx>
class TopoFace;
class DrawDisplay;
class Point3d;
class TopoEdge;

class DrawDim_PlanarDistance;
DEFINE_STANDARD_HANDLE(DrawDim_PlanarDistance, DrawDim_PlanarDimension)

//! PlanarDistance point/point
//! PlanarDistance point/line
//! PlanarDistance line/line
class DrawDim_PlanarDistance : public DrawDim_PlanarDimension
{

public:
  Standard_EXPORT DrawDim_PlanarDistance(const TopoFace&  plane,
                                         const TopoShape& point1,
                                         const TopoShape& point2);

  Standard_EXPORT DrawDim_PlanarDistance(const TopoShape& geom1, const TopoShape& geom2);

  //! private
  Standard_EXPORT void DrawOn(DrawDisplay& dis) const Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(DrawDim_PlanarDistance, DrawDim_PlanarDimension)

protected:
private:
  Standard_EXPORT void Draw1(const Point3d& p, const TopoEdge& e, DrawDisplay& d) const;

  TopoShape myGeom1;
  TopoShape myGeom2;
};

#endif // _DrawDim_PlanarDistance_HeaderFile
