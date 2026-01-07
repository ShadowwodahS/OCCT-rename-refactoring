// Created on: 1993-06-23
// Created by: Jean Yves LEBEY
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

#include <TopoDS_Shape.hxx>
#include <TopOpeBRepDS_Point.hxx>
#include <TopOpeBRepTool_ShapeTool.hxx>

//=================================================================================================

Point1::Point1()
    : myKeep(Standard_True)
{
}

//=================================================================================================

Point1::Point1(const Point3d& P, const Standard_Real T)
    : myPoint(P),
      myTolerance(T),
      myKeep(Standard_True)
{
}

//=================================================================================================

Point1::Point1(const TopoShape& S)
{
  myPoint     = ShapeTool::Pnt(S);
  myTolerance = ShapeTool::Tolerance(S);
}

//=================================================================================================

Standard_Boolean Point1::IsEqual(const Point1& P) const
{
  Standard_Real    t = Max(myTolerance, P.Tolerance());
  Standard_Boolean b = myPoint.IsEqual(P.Point(), t);
  return b;
}

//=================================================================================================

const Point3d& Point1::Point() const
{
  return myPoint;
}

//=================================================================================================

Point3d& Point1::ChangePoint()
{
  return myPoint;
}

//=================================================================================================

Standard_Real Point1::Tolerance() const
{
  return myTolerance;
}

//=================================================================================================

void Point1::Tolerance(const Standard_Real Tol)
{
  myTolerance = Tol;
}

//=================================================================================================

Standard_Boolean Point1::Keep() const
{
  return myKeep;
}

//=================================================================================================

void Point1::ChangeKeep(const Standard_Boolean b)
{
  myKeep = b;
}
