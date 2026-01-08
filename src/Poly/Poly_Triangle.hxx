// Created on: 1995-03-06
// Created by: Laurent PAINNOT
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _Poly_Triangle_HeaderFile
#define _Poly_Triangle_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>
#include <Standard_Integer.hxx>
#include <Standard_OutOfRange.hxx>

//! Describes a component triangle of a triangulation (MeshTriangulation object).
//! A Triangle1 is defined by a triplet of nodes within [1, MeshTriangulation::NbNodes()] range.
//! Each node is an index in the table of nodes specific to an existing
//! triangulation of a shape, and represents a point on the surface.
class Triangle2
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructs a triangle and sets all indices to zero.
  Triangle2() { myNodes[0] = myNodes[1] = myNodes[2] = 0; }

  //! Constructs a triangle and sets its three indices,
  //! where these node values are indices in the table of nodes specific to an existing
  //! triangulation of a shape.
  Triangle2(const Standard_Integer theN1,
                const Standard_Integer theN2,
                const Standard_Integer theN3)
  {
    myNodes[0] = theN1;
    myNodes[1] = theN2;
    myNodes[2] = theN3;
  }

  //! Sets the value of the three nodes of this triangle.
  void Set(const Standard_Integer theN1, const Standard_Integer theN2, const Standard_Integer theN3)
  {
    myNodes[0] = theN1;
    myNodes[1] = theN2;
    myNodes[2] = theN3;
  }

  //! Sets the value of node with specified index of this triangle.
  //! Raises Standard_OutOfRange if index is not in 1,2,3
  void Set(const Standard_Integer theIndex, const Standard_Integer theNode)
  {
    Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 3,
                                 "Triangle2::Set(), invalid index");
    myNodes[theIndex - 1] = theNode;
  }

  //! Returns the node indices of this triangle.
  void Get(Standard_Integer& theN1, Standard_Integer& theN2, Standard_Integer& theN3) const
  {
    theN1 = myNodes[0];
    theN2 = myNodes[1];
    theN3 = myNodes[2];
  }

  //! Get the node of given Index.
  //! Raises OutOfRange from Standard1 if Index is not in 1,2,3
  Standard_Integer Value(const Standard_Integer theIndex) const
  {
    Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 3,
                                 "Triangle2::Value(), invalid index");
    return myNodes[theIndex - 1];
  }

  Standard_Integer operator()(const Standard_Integer Index) const { return Value(Index); }

  //! Get the node of given Index.
  //! Raises OutOfRange if Index is not in 1,2,3
  Standard_Integer& ChangeValue(const Standard_Integer theIndex)
  {
    Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 3,
                                 "Triangle2::ChangeValue(), invalid index");
    return myNodes[theIndex - 1];
  }

  Standard_Integer& operator()(const Standard_Integer Index) { return ChangeValue(Index); }

protected:
  Standard_Integer myNodes[3];
};

#endif // _Poly_Triangle_HeaderFile
