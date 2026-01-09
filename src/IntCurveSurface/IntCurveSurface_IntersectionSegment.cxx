// Created on: 1993-04-07
// Created by: Laurent BUCHARD
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

#include <IntCurveSurface_IntersectionSegment.hxx>

IntersectionSegment::IntersectionSegment() {}

//================================================================================
IntersectionSegment::IntersectionSegment(
  const IntersectionPoint1& P1,
  const IntersectionPoint1& P2)
    : myP1(P1),
      myP2(P2)
{
}

//================================================================================
void IntersectionSegment::SetValues(const IntersectionPoint1& P1,
                                                    const IntersectionPoint1& P2)
{
  myP1 = P1;
  myP2 = P2;
}

//================================================================================
void IntersectionSegment::Values(IntersectionPoint1& P1,
                                                 IntersectionPoint1& P2) const
{
  P1 = myP1;
  P2 = myP2;
}

//================================================================================
void IntersectionSegment::FirstPoint(IntersectionPoint1& P1) const
{
  P1 = myP1;
}

//================================================================================
void IntersectionSegment::SecondPoint(IntersectionPoint1& P2) const
{
  P2 = myP2;
}

//================================================================================
const IntersectionPoint1& IntersectionSegment::FirstPoint() const
{
  return (myP1);
}

//================================================================================
const IntersectionPoint1& IntersectionSegment::SecondPoint() const
{
  return (myP2);
}

//================================================================================
void IntersectionSegment::Dump() const
{
  std::cout << "\nIntersectionSegment : " << std::endl;
  myP1.Dump();
  myP2.Dump();
  std::cout << std::endl;
}
