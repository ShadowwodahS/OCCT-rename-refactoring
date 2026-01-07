// Created on: 1992-10-07
// Created by: Laurent BUCHARD
// Copyright (c) 1992-1999 Matra Datavision
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

#include <IntAna2d_IntPoint.hxx>

IntersectionPoint2d::IntersectionPoint2d(const Standard_Real X,
                                     const Standard_Real Y,
                                     const Standard_Real U1,
                                     const Standard_Real U2)
    : myu1(U1),
      myu2(U2),
      myp(X, Y),
      myimplicit(Standard_False)
{
}

IntersectionPoint2d::IntersectionPoint2d(const Standard_Real X,
                                     const Standard_Real Y,
                                     const Standard_Real U1)
    : myu1(U1),
      myu2(RealLast()),
      myp(X, Y),
      myimplicit(Standard_True)
{
}

IntersectionPoint2d::IntersectionPoint2d()
    :

      myu1(RealLast()),
      myu2(RealLast()),
      myp(RealLast(), RealLast()),
      myimplicit(Standard_False)
{
}

void IntersectionPoint2d::SetValue(const Standard_Real X,
                                 const Standard_Real Y,
                                 const Standard_Real U1,
                                 const Standard_Real U2)
{

  myimplicit = Standard_False;
  myp.SetCoord(X, Y);
  myu1 = U1;
  myu2 = U2;
}

void IntersectionPoint2d::SetValue(const Standard_Real X,
                                 const Standard_Real Y,
                                 const Standard_Real U1)
{

  myimplicit = Standard_True;
  myp.SetCoord(X, Y);
  myu1 = U1;
  myu2 = RealLast();
}
