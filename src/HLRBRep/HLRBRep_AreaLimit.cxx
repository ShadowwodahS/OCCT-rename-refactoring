// Created on: 1997-04-17
// Created by: Christophe MARION
// Copyright (c) 1997-1999 Matra Datavision
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

#include <HLRAlgo_Intersection.hxx>
#include <HLRBRep_AreaLimit.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(AreaLimit, RefObject)

//=================================================================================================

AreaLimit::AreaLimit(const Intersection3& V,
                                     const Standard_Boolean      Boundary,
                                     const Standard_Boolean      Interference,
                                     const TopAbs_State          StateBefore,
                                     const TopAbs_State          StateAfter,
                                     const TopAbs_State          EdgeBefore,
                                     const TopAbs_State          EdgeAfter)
    : myVertex(V),
      myBoundary(Boundary),
      myInterference(Interference),
      myStateBefore(StateBefore),
      myStateAfter(StateAfter),
      myEdgeBefore(EdgeBefore),
      myEdgeAfter(EdgeAfter)
{
}

//=================================================================================================

void AreaLimit::StateBefore(const TopAbs_State Stat)
{
  myStateBefore = Stat;
}

//=================================================================================================

void AreaLimit::StateAfter(const TopAbs_State Stat)
{
  myStateAfter = Stat;
}

//=================================================================================================

void AreaLimit::EdgeBefore(const TopAbs_State Stat)
{
  myEdgeBefore = Stat;
}

//=================================================================================================

void AreaLimit::EdgeAfter(const TopAbs_State Stat)
{
  myEdgeAfter = Stat;
}

//=================================================================================================

void AreaLimit::Previous(const Handle(AreaLimit)& P)
{
  myPrevious = P;
}

//=================================================================================================

void AreaLimit::Next(const Handle(AreaLimit)& N)
{
  myNext = N;
}

//=================================================================================================

const Intersection3& AreaLimit::Vertex() const
{
  return myVertex;
}

//=================================================================================================

Standard_Boolean AreaLimit::IsBoundary() const
{
  return myBoundary;
}

//=================================================================================================

Standard_Boolean AreaLimit::IsInterference() const
{
  return myInterference;
}

//=================================================================================================

TopAbs_State AreaLimit::StateBefore() const
{
  return myStateBefore;
}

//=================================================================================================

TopAbs_State AreaLimit::StateAfter() const
{
  return myStateAfter;
}

//=================================================================================================

TopAbs_State AreaLimit::EdgeBefore() const
{
  return myEdgeBefore;
}

//=================================================================================================

TopAbs_State AreaLimit::EdgeAfter() const
{
  return myEdgeAfter;
}

//=================================================================================================

Handle(AreaLimit) AreaLimit::Previous() const
{
  return myPrevious;
}

//=================================================================================================

Handle(AreaLimit) AreaLimit::Next() const
{
  return myNext;
}

//=================================================================================================

void AreaLimit::Clear()
{
  myPrevious.Nullify();
  myNext.Nullify();
}
