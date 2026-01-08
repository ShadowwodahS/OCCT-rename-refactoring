// Created on: 1994-03-18
// Created by: Jean Marc LACHAUME
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

#include <HatchGen_IntersectionPoint.hxx>

//=================================================================================================

IntersectionPoint2::IntersectionPoint2()
    : myIndex(0),
      myParam(RealLast()),
      myPosit(TopAbs_INTERNAL),
      myBefore(TopAbs_UNKNOWN),
      myAfter(TopAbs_UNKNOWN),
      mySegBeg(Standard_False),
      mySegEnd(Standard_False)
{
}

//=======================================================================
// Function : SetIndex
// Purpose  : Sets the index of the supporting curve.
//=======================================================================

void IntersectionPoint2::SetIndex(const Standard_Integer Index)
{
  myIndex = Index;
}

//=======================================================================
// Function : Index
// Purpose  : Returns the index of the supporting curve.
//=======================================================================

Standard_Integer IntersectionPoint2::Index() const
{
  return myIndex;
}

//=======================================================================
// Function : SetParameter
// Purpose  : Sets the parameter on the curve.
//=======================================================================

void IntersectionPoint2::SetParameter(const Standard_Real Parameter)
{
  myParam = Parameter;
}

//=======================================================================
// Function : Parameter
// Purpose  : Returns the parameter on the curve.
//=======================================================================

Standard_Real IntersectionPoint2::Parameter() const
{
  return myParam;
}

//=======================================================================
// Function : SetPosition
// Purpose  : Sets the position of the point on the curve.
//=======================================================================

void IntersectionPoint2::SetPosition(const TopAbs_Orientation Position1)
{
  myPosit = Position1;
}

//=======================================================================
// Function : Position1
// Purpose  : Returns the position of the point on the element.
//=======================================================================

TopAbs_Orientation IntersectionPoint2::Position1() const
{
  return myPosit;
}

//=======================================================================
// Function : SetStateBefore
// Purpose  : Sets the transition state before the intersection.
//=======================================================================

void IntersectionPoint2::SetStateBefore(const TopAbs_State State)
{
  myBefore = State;
}

//=======================================================================
// Function : StateBefore
// Purpose  : Returns the transition state before the intersection.
//=======================================================================

TopAbs_State IntersectionPoint2::StateBefore() const
{
  return myBefore;
}

//=======================================================================
// Function : SetStateAfter
// Purpose  : Sets the transition state after the intersection.
//=======================================================================

void IntersectionPoint2::SetStateAfter(const TopAbs_State State)
{
  myAfter = State;
}

//=======================================================================
// Function : StateAfter
// Purpose  : Returns the transition state after the intersection.
//=======================================================================

TopAbs_State IntersectionPoint2::StateAfter() const
{
  return myAfter;
}

//=======================================================================
// Function : SetSegmentBeginning
// Purpose  : Sets the flag that the point is the beginning of a segment.
//=======================================================================

void IntersectionPoint2::SetSegmentBeginning(const Standard_Boolean State)
{
  mySegBeg = State;
}

//=======================================================================
// Function : SegmentBeginning
// Purpose  : Returns the flag that the point is the beginning of a
//            segment.
//=======================================================================

Standard_Boolean IntersectionPoint2::SegmentBeginning() const
{
  return mySegBeg;
}

//=======================================================================
// Function : SetSegmentEnd
// Purpose  : Sets the flag that the point is the end of a segment.
//=======================================================================

void IntersectionPoint2::SetSegmentEnd(const Standard_Boolean State)
{
  mySegEnd = State;
}

//=======================================================================
// Function : SegmentEnd
// Purpose  : Returns the flag that the point is the end of a segment.
//=======================================================================

Standard_Boolean IntersectionPoint2::SegmentEnd() const
{
  return mySegEnd;
}
