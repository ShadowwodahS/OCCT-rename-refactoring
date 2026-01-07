// Created on: 1996-02-15
// Created by: Jacques GOUSSARD
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _LocOpe_FindEdges_HeaderFile
#define _LocOpe_FindEdges_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopTools_ListIteratorOfListOfShape.hxx>
class TopoEdge;

class LocOpe_FindEdges
{
public:
  DEFINE_STANDARD_ALLOC

  LocOpe_FindEdges();

  LocOpe_FindEdges(const TopoShape& FFrom, const TopoShape& FTo);

  Standard_EXPORT void Set(const TopoShape& FFrom, const TopoShape& FTo);

  void InitIterator();

  Standard_Boolean More() const;

  const TopoEdge& EdgeFrom() const;

  const TopoEdge& EdgeTo() const;

  void Next();

protected:
private:
  TopoShape                       myFFrom;
  TopoShape                       myFTo;
  ShapeList               myLFrom;
  ShapeList               myLTo;
  TopTools_ListIteratorOfListOfShape myItFrom;
  TopTools_ListIteratorOfListOfShape myItTo;
};

#include <LocOpe_FindEdges.lxx>

#endif // _LocOpe_FindEdges_HeaderFile
