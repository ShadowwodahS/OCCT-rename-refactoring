// Created on: 1999-09-20
// Created by: Peter KURNEV
// Copyright (c) 1999-1999 Matra Datavision
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

#include <TopOpeBRepDS_ShapeWithState.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

//=================================================================================================

ShapeWithState::ShapeWithState()
    : myState(TopAbs_UNKNOWN),
      myIsSplitted(Standard_False)
{
}

//=================================================================================================

const ShapeList& ShapeWithState::Part(const TopAbs_State aState) const
{
  static ShapeList myEmptyListOfShape;
  switch (aState)
  {
    case TopAbs_IN:
      return myPartIn;
    case TopAbs_OUT:
      return myPartOut;
    case TopAbs_ON:
      return myPartOn;
    default:
      return myEmptyListOfShape;
  }
}

//=================================================================================================

void ShapeWithState::AddPart(const TopoShape& aShape, const TopAbs_State aState)
{
  switch (aState)
  {
    case TopAbs_IN:
      myPartIn.Append(aShape);
      break;
    case TopAbs_OUT:
      myPartOut.Append(aShape);
      break;
    case TopAbs_ON:
      myPartOn.Append(aShape);
      break;
    default:
      break;
  }
}

//=================================================================================================

void ShapeWithState::AddParts(const ShapeList& aListOfShape,
                                           const TopAbs_State          aState)
{
  TopTools_ListIteratorOfListOfShape anIt(aListOfShape);

  switch (aState)
  {
    case TopAbs_IN:
      for (; anIt.More(); anIt.Next())
      {
        myPartIn.Append(anIt.Value());
      }
      break;
    case TopAbs_OUT:
      for (; anIt.More(); anIt.Next())
      {
        myPartOut.Append(anIt.Value());
      }
      break;
    case TopAbs_ON:
      for (; anIt.More(); anIt.Next())
      {
        myPartOn.Append(anIt.Value());
      }
      break;

    default:
      break;
  }
}

//=================================================================================================

void ShapeWithState::SetState(const TopAbs_State aState)
{
  myState = aState;
}

//=================================================================================================

TopAbs_State ShapeWithState::State() const
{
  return myState;
}

//=================================================================================================

void ShapeWithState::SetIsSplitted(const Standard_Boolean aFlag)
{
  myIsSplitted = aFlag;
}

//=================================================================================================

Standard_Boolean ShapeWithState::IsSplitted() const
{
  return myIsSplitted;
}
