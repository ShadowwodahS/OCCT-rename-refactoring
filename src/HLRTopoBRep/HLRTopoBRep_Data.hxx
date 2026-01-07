// Created on: 1994-10-24
// Created by: Christophe MARION
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

#ifndef _HLRTopoBRep_Data_HeaderFile
#define _HLRTopoBRep_Data_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_DataMapOfShapeListOfShape.hxx>
#include <HLRTopoBRep_DataMapOfShapeFaceData.hxx>
#include <TopTools_MapOfShape.hxx>
#include <HLRTopoBRep_DataMapIteratorOfMapOfShapeListOfVData.hxx>
#include <HLRTopoBRep_ListIteratorOfListOfVData.hxx>
#include <Standard_Boolean.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoEdge;
class TopoFace;
class TopoShape;
class TopoVertex;

//! Stores  the results  of  the  OutLine and  IsoLine
//! processes.
class Data1
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT Data1();

  //! Clear of all the maps.
  Standard_EXPORT void Clear();

  //! Clear of all the data  not needed during and after
  //! the hiding process.
  Standard_EXPORT void Clean();

  //! Returns True if the Edge is split.
  Standard_EXPORT Standard_Boolean EdgeHasSplE(const TopoEdge& E) const;

  //! Returns True if the Face has internal outline.
  Standard_EXPORT Standard_Boolean FaceHasIntL(const TopoFace& F) const;

  //! Returns True if the Face has outlines on restriction.
  Standard_EXPORT Standard_Boolean FaceHasOutL(const TopoFace& F) const;

  //! Returns True if the Face has isolines.
  Standard_EXPORT Standard_Boolean FaceHasIsoL(const TopoFace& F) const;

  Standard_EXPORT Standard_Boolean IsSplEEdgeEdge(const TopoEdge& E1,
                                                  const TopoEdge& E2) const;

  Standard_EXPORT Standard_Boolean IsIntLFaceEdge(const TopoFace& F, const TopoEdge& E) const;

  Standard_EXPORT Standard_Boolean IsOutLFaceEdge(const TopoFace& F, const TopoEdge& E) const;

  Standard_EXPORT Standard_Boolean IsIsoLFaceEdge(const TopoFace& F, const TopoEdge& E) const;

  Standard_EXPORT TopoShape NewSOldS(const TopoShape& New) const;

  //! Returns the list of the edges.
  const ShapeList& EdgeSplE(const TopoEdge& E) const;

  //! Returns the list of the internal OutLines.
  const ShapeList& FaceIntL(const TopoFace& F) const;

  //! Returns the list of the OutLines on restriction.
  const ShapeList& FaceOutL(const TopoFace& F) const;

  //! Returns the list of the IsoLines.
  const ShapeList& FaceIsoL(const TopoFace& F) const;

  //! Returns  True   if V is  an   outline vertex  on a
  //! restriction.
  Standard_Boolean IsOutV(const TopoVertex& V) const;

  //! Returns True if V is an internal outline vertex.
  Standard_Boolean IsIntV(const TopoVertex& V) const;

  Standard_EXPORT void AddOldS(const TopoShape& NewS, const TopoShape& OldS);

  Standard_EXPORT ShapeList& AddSplE(const TopoEdge& E);

  Standard_EXPORT ShapeList& AddIntL(const TopoFace& F);

  Standard_EXPORT ShapeList& AddOutL(const TopoFace& F);

  Standard_EXPORT ShapeList& AddIsoL(const TopoFace& F);

  void AddOutV(const TopoVertex& V);

  void AddIntV(const TopoVertex& V);

  Standard_EXPORT void InitEdge();

  Standard_Boolean MoreEdge() const;

  Standard_EXPORT void NextEdge();

  const TopoEdge& Edge() const;

  //! Start an iteration on the vertices of E.
  Standard_EXPORT void InitVertex(const TopoEdge& E);

  Standard_Boolean MoreVertex() const;

  void NextVertex();

  Standard_EXPORT const TopoVertex& Vertex() const;

  Standard_EXPORT Standard_Real Parameter() const;

  //! Insert before the current position.
  Standard_EXPORT void InsertBefore(const TopoVertex& V, const Standard_Real P);

  Standard_EXPORT void Append(const TopoVertex& V, const Standard_Real P);

protected:
private:
  TopTools_DataMapOfShapeShape                       myOldS;
  TopTools_DataMapOfShapeListOfShape                 mySplE;
  HLRTopoBRep_DataMapOfShapeFaceData                 myData;
  TopTools_MapOfShape                                myOutV;
  TopTools_MapOfShape                                myIntV;
  HLRTopoBRep_MapOfShapeListOfVData                  myEdgesVertices;
  HLRTopoBRep_DataMapIteratorOfMapOfShapeListOfVData myEIterator;
  HLRTopoBRep_ListIteratorOfListOfVData              myVIterator;
  HLRTopoBRep_ListOfVData*                           myVList;
};

#include <HLRTopoBRep_Data.lxx>

#endif // _HLRTopoBRep_Data_HeaderFile
