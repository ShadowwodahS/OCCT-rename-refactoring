// Created on: 1999-11-29
// Created by: Peter KURNEV
// Copyright (c) 1999 Matra Datavision
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

#ifndef _TopOpeBRepBuild_VertexInfo_HeaderFile
#define _TopOpeBRepBuild_VertexInfo_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_IndexedMapOfOrientedShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Standard_Integer.hxx>

class TopOpeBRepBuild_VertexInfo
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepBuild_VertexInfo();

  Standard_EXPORT void SetVertex(const TopoVertex& aV);

  Standard_EXPORT const TopoVertex& Vertex() const;

  Standard_EXPORT void SetSmart(const Standard_Boolean aFlag);

  Standard_EXPORT Standard_Boolean Smart() const;

  Standard_EXPORT Standard_Integer NbCases() const;

  Standard_EXPORT Standard_Integer FoundOut() const;

  Standard_EXPORT void AddIn(const TopoEdge& anE);

  Standard_EXPORT void AddOut(const TopoEdge& anE);

  Standard_EXPORT void SetCurrentIn(const TopoEdge& anE);

  Standard_EXPORT const TopTools_IndexedMapOfOrientedShape& EdgesIn() const;

  Standard_EXPORT const TopTools_IndexedMapOfOrientedShape& EdgesOut() const;

  Standard_EXPORT TopTools_IndexedMapOfOrientedShape& ChangeEdgesOut();

  Standard_EXPORT void Dump() const;

  Standard_EXPORT const TopoEdge& CurrentOut();

  Standard_EXPORT void AppendPassed(const TopoEdge& anE);

  Standard_EXPORT void RemovePassed();

  Standard_EXPORT const ShapeList& ListPassed() const;

  Standard_EXPORT void Prepare(const ShapeList& aL);

protected:
private:
  TopoVertex                      myVertex;
  TopoEdge                        myCurrent;
  TopoEdge                        myCurrentIn;
  Standard_Boolean                   mySmart;
  TopTools_IndexedMapOfOrientedShape myEdgesIn;
  TopTools_IndexedMapOfOrientedShape myEdgesOut;
  TopTools_IndexedMapOfOrientedShape myLocalEdgesOut;
  ShapeList               myEdgesPassed;
  Standard_Integer                   myFoundOut;
};

#endif // _TopOpeBRepBuild_VertexInfo_HeaderFile
