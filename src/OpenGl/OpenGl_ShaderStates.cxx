// Created on: 2013-10-02
// Created by: Denis BOGOLEPOV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <OpenGl_ShaderStates.hxx>

//=================================================================================================

StateInterface::StateInterface()
    : myIndex(0)
{
  //
}

// =======================================================================
// function : ProjectionState1
// purpose  : Creates uninitialized projection state
// =======================================================================
ProjectionState1::ProjectionState1()
    : myInverseNeedUpdate(false)
{
  //
}

// =======================================================================
// function : Set
// purpose  : Sets new OCCT projection state
// =======================================================================
void ProjectionState1::Set(const OpenGl_Mat4& theProjectionMatrix)
{
  myProjectionMatrix  = theProjectionMatrix;
  myInverseNeedUpdate = true;
}

// =======================================================================
// function : ProjectionMatrixInverse
// purpose  : Returns inverse of current projection matrix
// =======================================================================
const OpenGl_Mat4& ProjectionState1::ProjectionMatrixInverse() const
{
  if (myInverseNeedUpdate)
  {
    myInverseNeedUpdate = false;
    myProjectionMatrix.Inverted(myProjectionMatrixInverse);
  }
  return myProjectionMatrixInverse;
}

// =======================================================================
// function : ModelWorldState1
// purpose  : Creates uninitialized model-world state
// =======================================================================
ModelWorldState1::ModelWorldState1()
    : myInverseNeedUpdate(false)
{
  //
}

// =======================================================================
// function : Set
// purpose  : Sets new model-world matrix
// =======================================================================
void ModelWorldState1::Set(const OpenGl_Mat4& theModelWorldMatrix)
{
  myModelWorldMatrix  = theModelWorldMatrix;
  myInverseNeedUpdate = true;
}

// =======================================================================
// function : ModelWorldMatrixInverse
// purpose  : Returns inverse of current model-world matrix
// =======================================================================
const OpenGl_Mat4& ModelWorldState1::ModelWorldMatrixInverse() const
{
  if (myInverseNeedUpdate)
  {
    myInverseNeedUpdate = false;
    myModelWorldMatrix.Inverted(myModelWorldMatrixInverse);
  }
  return myModelWorldMatrixInverse;
}

// =======================================================================
// function : WorldViewState1
// purpose  : Creates uninitialized world-view state
// =======================================================================
WorldViewState1::WorldViewState1()
    : myInverseNeedUpdate(false)
{
  //
}

// =======================================================================
// function : Set
// purpose  : Sets new world-view matrix
// =======================================================================
void WorldViewState1::Set(const OpenGl_Mat4& theWorldViewMatrix)
{
  myWorldViewMatrix   = theWorldViewMatrix;
  myInverseNeedUpdate = true;
}

// =======================================================================
// function : WorldViewMatrixInverse
// purpose  : Returns inverse of current world-view matrix
// =======================================================================
const OpenGl_Mat4& WorldViewState1::WorldViewMatrixInverse() const
{
  if (myInverseNeedUpdate)
  {
    myInverseNeedUpdate = false;
    myWorldViewMatrix.Inverted(myWorldViewMatrixInverse);
  }
  return myWorldViewMatrixInverse;
}

// =======================================================================
// function : ClippingState
// purpose  : Creates new clipping state
// =======================================================================
ClippingState::ClippingState()
    : myIndex(0),
      myNextIndex(1)
{
  //
}

// =======================================================================
// function : Update
// purpose  : Updates current state
// =======================================================================
void ClippingState::Update()
{
  myStateStack.Prepend(myIndex);
  myIndex = myNextIndex; // use myNextIndex here to handle properly Update() after Revert()
  ++myNextIndex;
}

// =======================================================================
// function : Revert
// purpose  : Reverts current state
// =======================================================================
void ClippingState::Revert()
{
  if (!myStateStack.IsEmpty())
  {
    myIndex = myStateStack.First();
    myStateStack.RemoveFirst();
  }
  else
  {
    myIndex = 0;
  }
}
