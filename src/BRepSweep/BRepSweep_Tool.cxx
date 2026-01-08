// Created on: 1993-06-09
// Created by: Laurent BOURESCHE
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

#include <BRepSweep_Tool.hxx>
#include <TopExp.hxx>
#include <TopoDS_Shape.hxx>

//=================================================================================================

Tool6::Tool6(const TopoShape& aShape)
{
  TopExp1::MapShapes(aShape, myMap);
}

//=================================================================================================

Standard_Integer Tool6::NbShapes1() const
{
  return myMap.Extent();
}

//=================================================================================================

Standard_Integer Tool6::Index(const TopoShape& aShape) const
{
  if (!myMap.Contains(aShape))
    return 0;
  return myMap.FindIndex(aShape);
}

//=================================================================================================

TopoShape Tool6::Shape(const Standard_Integer anIndex) const
{
  return myMap.FindKey(anIndex);
}

//=================================================================================================

TopAbs_ShapeEnum Tool6::Type(const TopoShape& aShape) const
{
  return aShape.ShapeType();
}

//=================================================================================================

TopAbs_Orientation Tool6::Orientation(const TopoShape& aShape) const
{
  return aShape.Orientation();
}

//=================================================================================================

void Tool6::SetOrientation(TopoShape& aShape, const TopAbs_Orientation Or) const
{
  aShape.Orientation(Or);
}
