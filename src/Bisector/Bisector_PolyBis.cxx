// Created on: 1994-01-10
// Created by: Yves FRICAUD
// Copyright (c) 1994-1999 Matra Datavision
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

#include <Bisector_PolyBis.hxx>
#include <ElCLib.hxx>
#include <gp.hxx>
#include <gp_Trsf2d.hxx>

//=================================================================================================

PolyBis::PolyBis()
{
  nbPoints = 0;
}

//=================================================================================================

void PolyBis::Append(const PointOnBis& P)
{
  nbPoints++;
  thePoints[nbPoints] = P;
}

//=================================================================================================

Standard_Integer PolyBis::Length() const
{
  return nbPoints;
}

//=================================================================================================

Standard_Boolean PolyBis::IsEmpty() const
{
  return (nbPoints == 0);
}

//=================================================================================================

const PointOnBis& PolyBis::Value(const Standard_Integer Index) const
{
  return thePoints[Index];
}

//=================================================================================================

const PointOnBis& PolyBis::First() const
{
  return thePoints[1];
}

//=================================================================================================

const PointOnBis& PolyBis::Last() const
{
  return thePoints[nbPoints];
}

//=================================================================================================

// const PointOnBis& PolyBis::Points()
//{
//  return thePoints;
//}

//=================================================================================================

Standard_Integer PolyBis::Interval1(const Standard_Real U) const
{
  if (Last().ParamOnBis() - U < gp1::Resolution())
  {
    return nbPoints - 1;
  }
  Standard_Real dU = (Last().ParamOnBis() - First().ParamOnBis()) / (nbPoints - 1);
  if (dU <= gp1::Resolution())
    return 1;

  Standard_Integer IntU = Standard_Integer(Abs(U - First().ParamOnBis()) / dU);
  IntU++;

  if (thePoints[IntU].ParamOnBis() >= U)
  {
    for (Standard_Integer i = IntU; i >= 1; i--)
    {
      if (thePoints[i].ParamOnBis() <= U)
      {
        IntU = i;
        break;
      }
    }
  }
  else
  {
    for (Standard_Integer i = IntU; i <= nbPoints - 1; i++)
    {
      if (thePoints[i].ParamOnBis() >= U)
      {
        IntU = i - 1;
        break;
      }
    }
  }
  return IntU;
}

//=================================================================================================

void PolyBis::Transform(const Transform2d& T)
{
  for (Standard_Integer i = 1; i <= nbPoints; i++)
  {
    gp_Pnt2d P = thePoints[i].Point();
    P.Transform(T);
    thePoints[i].Point(P);
  }
}
