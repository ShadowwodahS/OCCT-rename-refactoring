// Created on: 2000-05-22
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <IntTools_Root.hxx>

//=================================================================================================

IntersectionRoot::IntersectionRoot()
    : myRoot(0.),
      myType(-1),
      myLayerHeight(0.),
      myStateBefore(TopAbs_UNKNOWN),
      myStateAfter(TopAbs_UNKNOWN),
      myt1(0.),
      myt2(0.),
      myf1(0.),
      myf2(0.)
{
}

//=================================================================================================

IntersectionRoot::IntersectionRoot(const Standard_Real aRoot, const Standard_Integer aType)
    : myLayerHeight(0.),
      myStateBefore(TopAbs_UNKNOWN),
      myStateAfter(TopAbs_UNKNOWN),
      myt1(0.),
      myt2(0.),
      myf1(0.),
      myf2(0.)
{
  myRoot = aRoot;
  myType = aType;
}

//=================================================================================================

void IntersectionRoot::SetRoot(const Standard_Real aRoot)
{
  myRoot = aRoot;
}

//=================================================================================================

void IntersectionRoot::SetType(const Standard_Integer aType)
{
  myType = aType;
}

//=================================================================================================

void IntersectionRoot::SetStateBefore(const TopAbs_State aState)
{
  myStateBefore = aState;
}

//=================================================================================================

void IntersectionRoot::SetStateAfter(const TopAbs_State aState)
{
  myStateAfter = aState;
}

//=================================================================================================

void IntersectionRoot::SetLayerHeight(const Standard_Real aHeight)
{
  myLayerHeight = aHeight;
}

//=================================================================================================

void IntersectionRoot::SetInterval(const Standard_Real t1,
                                const Standard_Real t2,
                                const Standard_Real f1,
                                const Standard_Real f2)
{
  myt1 = t1;
  myt2 = t2;
  myf1 = f1;
  myf2 = f2;
}

//=================================================================================================

void IntersectionRoot::Interval1(Standard_Real& t1,
                             Standard_Real& t2,
                             Standard_Real& f1,
                             Standard_Real& f2) const
{
  t1 = myt1;
  t2 = myt2;
  f1 = myf1;
  f2 = myf2;
}

//=================================================================================================

Standard_Real IntersectionRoot::Root() const
{
  return myRoot;
}

//=================================================================================================

Standard_Integer IntersectionRoot::Type() const
{
  return myType;
}

//=================================================================================================

TopAbs_State IntersectionRoot::StateBefore() const
{
  return myStateBefore;
}

//=================================================================================================

TopAbs_State IntersectionRoot::StateAfter() const
{
  return myStateAfter;
}

//=================================================================================================

Standard_Real IntersectionRoot::LayerHeight() const
{
  return myLayerHeight;
}

//=================================================================================================

Standard_Boolean IntersectionRoot::IsValid() const
{
  if (myStateBefore == TopAbs_OUT && myStateAfter == TopAbs_IN)
    return Standard_True;
  if (myStateBefore == TopAbs_OUT && myStateAfter == TopAbs_ON)
    return Standard_True;
  if (myStateBefore == TopAbs_ON && myStateAfter == TopAbs_OUT)
    return Standard_True;
  if (myStateBefore == TopAbs_IN && myStateAfter == TopAbs_OUT)
    return Standard_True;

  return Standard_False;
}
