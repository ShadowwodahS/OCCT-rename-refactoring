// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <Poly_ArrayOfNodes.hxx>

//=================================================================================================

NodeArray::NodeArray(const NodeArray& theOther)
    : NCollection_AliasedArray(theOther)
{
  //
}

//=================================================================================================

NodeArray::~NodeArray()
{
  //
}

//=================================================================================================

NodeArray& NodeArray::Assign(const NodeArray& theOther)
{
  if (&theOther == this)
  {
    return *this;
  }

  if (myStride == theOther.myStride)
  {
    // fast copy
    NCollection_AliasedArray::Assign(theOther);
    return *this;
  }

  // slow copy
  if (mySize != theOther.mySize)
  {
    throw Standard_DimensionMismatch("NodeArray::Assign(), arrays have different sizes");
  }
  for (int anIter = 0; anIter < mySize; ++anIter)
  {
    const Point3d aPnt = theOther.Value(anIter);
    SetValue(anIter, aPnt);
  }
  return *this;
}
