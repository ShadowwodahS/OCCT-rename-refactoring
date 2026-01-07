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

#include <HLRTopoBRep_Data.hxx>
#include <HLRTopoBRep_ListOfVData.hxx>
#include <HLRTopoBRep_VData.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

//=================================================================================================

Data1::Data1()
{
  Clear();
}

//=================================================================================================

void Data1::Clear()
{
  myOldS.Clear();
  mySplE.Clear();
  myData.Clear();
  myOutV.Clear();
  myIntV.Clear();
  myEdgesVertices.Clear();
}

//=================================================================================================

void Data1::Clean() {}

//=================================================================================================

Standard_Boolean Data1::EdgeHasSplE(const TopoEdge& E) const
{
  if (!mySplE.IsBound(E))
    return Standard_False;
  return !mySplE(E).IsEmpty();
}

//=================================================================================================

Standard_Boolean Data1::FaceHasIntL(const TopoFace& F) const
{
  if (!myData.IsBound(F))
    return Standard_False;
  return !myData(F).FaceIntL().IsEmpty();
}

//=================================================================================================

Standard_Boolean Data1::FaceHasOutL(const TopoFace& F) const
{
  if (!myData.IsBound(F))
    return Standard_False;
  return !myData(F).FaceOutL().IsEmpty();
}

//=================================================================================================

Standard_Boolean Data1::FaceHasIsoL(const TopoFace& F) const
{
  if (!myData.IsBound(F))
    return Standard_False;
  return !myData(F).FaceIsoL().IsEmpty();
}

//=================================================================================================

Standard_Boolean Data1::IsSplEEdgeEdge(const TopoEdge& E1,
                                                  const TopoEdge& E2) const
{
  Standard_Boolean found = Standard_False;
  if (EdgeHasSplE(E1))
  {

    TopTools_ListIteratorOfListOfShape itS;
    for (itS.Initialize(EdgeSplE(E1)); itS.More() && !found; itS.Next())
      found = itS.Value().IsSame(E2);
  }
  else
    found = E1.IsSame(E2);
  return found;
}

//=================================================================================================

Standard_Boolean Data1::IsIntLFaceEdge(const TopoFace& F, const TopoEdge& E) const
{
  Standard_Boolean found = Standard_False;
  if (FaceHasIntL(F))
  {

    TopTools_ListIteratorOfListOfShape itE;
    for (itE.Initialize(FaceIntL(F)); itE.More() && !found; itE.Next())
    {
      found = IsSplEEdgeEdge(TopoDS::Edge(itE.Value()), E);
    }
  }
  return found;
}

//=================================================================================================

Standard_Boolean Data1::IsOutLFaceEdge(const TopoFace& F, const TopoEdge& E) const
{
  Standard_Boolean found = Standard_False;
  if (FaceHasOutL(F))
  {

    TopTools_ListIteratorOfListOfShape itE;
    for (itE.Initialize(FaceOutL(F)); itE.More() && !found; itE.Next())
    {
      found = IsSplEEdgeEdge(TopoDS::Edge(itE.Value()), E);
    }
  }
  return found;
}

//=================================================================================================

Standard_Boolean Data1::IsIsoLFaceEdge(const TopoFace& F, const TopoEdge& E) const
{
  Standard_Boolean found = Standard_False;
  if (FaceHasIsoL(F))
  {

    TopTools_ListIteratorOfListOfShape itE;
    for (itE.Initialize(FaceIsoL(F)); itE.More() && !found; itE.Next())
    {
      found = IsSplEEdgeEdge(TopoDS::Edge(itE.Value()), E);
    }
  }
  return found;
}

//=================================================================================================

TopoShape Data1::NewSOldS(const TopoShape& NewS) const
{
  if (myOldS.IsBound(NewS))
    return myOldS(NewS);
  else
    return NewS;
}

//=================================================================================================

void Data1::AddOldS(const TopoShape& NewS, const TopoShape& OldS)
{
  if (!myOldS.IsBound(NewS))
    myOldS.Bind(NewS, OldS);
}

//=================================================================================================

ShapeList& Data1::AddSplE(const TopoEdge& E)
{
  if (!mySplE.IsBound(E))
  {
    ShapeList empty;
    mySplE.Bind(E, empty);
  }
  return mySplE(E);
}

//=================================================================================================

ShapeList& Data1::AddIntL(const TopoFace& F)
{
  if (!myData.IsBound(F))
  {
    FaceData theData;
    myData.Bind(F, theData);
  }
  return myData(F).AddIntL();
}

//=================================================================================================

ShapeList& Data1::AddOutL(const TopoFace& F)
{
  if (!myData.IsBound(F))
  {
    FaceData theData;
    myData.Bind(F, theData);
  }
  return myData(F).AddOutL();
}

//=================================================================================================

ShapeList& Data1::AddIsoL(const TopoFace& F)
{
  if (!myData.IsBound(F))
  {
    FaceData theData;
    myData.Bind(F, theData);
  }
  return myData(F).AddIsoL();
}

//=================================================================================================

void Data1::InitEdge()
{
  myEIterator.Initialize(myEdgesVertices);

  while (myEIterator.More() && myEIterator.Value().IsEmpty())
    myEIterator.Next();
}

//=================================================================================================

void Data1::NextEdge()
{
  myEIterator.Next();

  while (myEIterator.More() && myEIterator.Value().IsEmpty())
    myEIterator.Next();
}

//=================================================================================================

void Data1::InitVertex(const TopoEdge& E)
{
  if (!myEdgesVertices.IsBound(E))
  {
    HLRTopoBRep_ListOfVData empty;
    myEdgesVertices.Bind(E, empty);
  }
  HLRTopoBRep_ListOfVData& L = myEdgesVertices(E);
  myVList                    = &L;
  myVIterator.Initialize(L);
}

//=================================================================================================

const TopoVertex& Data1::Vertex() const
{
  return TopoDS::Vertex(myVIterator.Value().Vertex());
}

//=================================================================================================

Standard_Real Data1::Parameter() const
{
  return myVIterator.Value().Parameter();
}

//=================================================================================================

void Data1::InsertBefore(const TopoVertex& V, const Standard_Real P)
{
  HLRTopoBRep_VData VD(P, V);
  myVList->InsertBefore(VD, myVIterator);
}

//=================================================================================================

void Data1::Append(const TopoVertex& V, const Standard_Real P)
{
  HLRTopoBRep_VData VD(P, V);
  myVList->Append(VD);
}
