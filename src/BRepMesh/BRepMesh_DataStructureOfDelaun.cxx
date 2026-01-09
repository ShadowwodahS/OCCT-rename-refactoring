// Created on: 1993-05-11
// Created by: Didier PIFFAULT
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRepMesh_DataStructureOfDelaun.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepMesh_Edge.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <Standard_ErrorHandler.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepMesh_DataStructureOfDelaun, RefObject)

//=================================================================================================

BRepMesh_DataStructureOfDelaun::BRepMesh_DataStructureOfDelaun(
  const Handle(NCollection_IncAllocator)& theAllocator,
  const Standard_Integer                  theReservedNodeSize)
    : myAllocator(theAllocator),
      myNodes(new BRepMesh_VertexTool(myAllocator)),
      myNodeLinks(theReservedNodeSize * 3, myAllocator),
      myLinks(theReservedNodeSize * 3, myAllocator),
      myDelLinks(myAllocator),
      myElements(theReservedNodeSize * 2, myAllocator)
{
}

//=================================================================================================

Standard_Integer BRepMesh_DataStructureOfDelaun::AddNode(const Vertex& theNode,
                                                         const Standard_Boolean isForceAdd)
{
  const Standard_Integer aNodeId = myNodes->Add(theNode, isForceAdd);
  if (!myNodeLinks.IsBound(aNodeId))
    myNodeLinks.Bind(aNodeId, IMeshData::ListOfInteger(myAllocator));

  return aNodeId;
}

//=================================================================================================

Standard_Boolean BRepMesh_DataStructureOfDelaun::SubstituteNode(const Standard_Integer theIndex,
                                                                const Vertex& theNewNode)
{
  if (myNodes->FindIndex(theNewNode) != 0)
    return Standard_False;

  myNodes->Substitute(theIndex, theNewNode);
  return Standard_True;
}

//=================================================================================================

Standard_Integer BRepMesh_DataStructureOfDelaun::AddLink(const Edge3& theLink)
{
  Standard_Integer aLinkIndex = IndexOf(theLink);
  if (aLinkIndex > 0)
  {
    return theLink.IsSameOrientation(GetLink(aLinkIndex)) ? aLinkIndex : -aLinkIndex;
  }

  PairOfIndex aPair;
  if (!myDelLinks.IsEmpty())
  {
    aLinkIndex = myDelLinks.First();
    myLinks.Substitute(aLinkIndex, theLink, aPair);
    myDelLinks.RemoveFirst();
  }
  else
    aLinkIndex = myLinks.Add(theLink, aPair);

  const Standard_Integer aLinkId = Abs(aLinkIndex);
  linksConnectedTo(theLink.FirstNode()).Append(aLinkId);
  linksConnectedTo(theLink.LastNode()).Append(aLinkId);
  myLinksOfDomain.Add(aLinkIndex);

  return aLinkIndex;
}

//=================================================================================================

Standard_Boolean BRepMesh_DataStructureOfDelaun::SubstituteLink(const Standard_Integer theIndex,
                                                                const Edge3&   theNewLink)
{
  PairOfIndex aPair;
  Edge3        aLink = GetLink(theIndex);
  if (aLink.Movability() == BRepMesh_Deleted)
  {
    myLinks.Substitute(theIndex, theNewLink, aPair);
    return Standard_True;
  }

  if (IndexOf(theNewLink) != 0)
    return Standard_False;

  aLink.SetMovability(BRepMesh_Deleted);
  myLinks.Substitute(theIndex, aLink, aPair);
  cleanLink(theIndex, aLink);

  const Standard_Integer aLinkId = Abs(theIndex);
  linksConnectedTo(theNewLink.FirstNode()).Append(aLinkId);
  linksConnectedTo(theNewLink.LastNode()).Append(aLinkId);
  myLinks.Substitute(theIndex, theNewLink, aPair);

  return Standard_True;
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::RemoveLink(const Standard_Integer theIndex,
                                                const Standard_Boolean isForce)
{
  Edge3& aLink = (Edge3&)GetLink(theIndex);
  if (aLink.Movability() == BRepMesh_Deleted || (!isForce && aLink.Movability() != BRepMesh_Free)
      || ElementsConnectedTo(theIndex).Extent() != 0)
  {
    return;
  }

  cleanLink(theIndex, aLink);
  aLink.SetMovability(BRepMesh_Deleted);

  myLinksOfDomain.Remove(theIndex);
  myDelLinks.Append(theIndex);
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::cleanLink(const Standard_Integer theIndex,
                                               const Edge3&   theLink)
{
  for (Standard_Integer i = 0; i < 2; ++i)
  {
    const Standard_Integer aNodeId = (i == 0) ? theLink.FirstNode() : theLink.LastNode();

    IMeshData::ListOfInteger&          aLinkList = linksConnectedTo(aNodeId);
    IMeshData::ListOfInteger::Iterator aLinkIt(aLinkList);
    for (; aLinkIt.More(); aLinkIt.Next())
    {
      if (aLinkIt.Value() == theIndex)
      {
        aLinkList.Remove(aLinkIt);
        break;
      }
    }
  }
}

//=================================================================================================

Standard_Integer BRepMesh_DataStructureOfDelaun::AddElement(const Triangle3& theElement)
{
  myElements.Append(theElement);
  Standard_Integer aElementIndex = myElements.Size();
  myElementsOfDomain.Add(aElementIndex);

  const Standard_Integer(&e)[3] = theElement.myEdges;
  for (Standard_Integer i = 0; i < 3; ++i)
    myLinks(e[i]).Append(aElementIndex);

  return aElementIndex;
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::RemoveElement(const Standard_Integer theIndex)
{
  Triangle3& aElement = (Triangle3&)GetElement(theIndex);
  if (aElement.Movability() == BRepMesh_Deleted)
    return;

  cleanElement(theIndex, aElement);
  aElement.SetMovability(BRepMesh_Deleted);
  myElementsOfDomain.Remove(theIndex);
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::cleanElement(const Standard_Integer   theIndex,
                                                  const Triangle3& theElement)
{
  if (theElement.Movability() != BRepMesh_Free)
    return;

  const Standard_Integer(&e)[3] = theElement.myEdges;
  for (Standard_Integer i = 0; i < 3; ++i)
    removeElementIndex(theIndex, myLinks(e[i]));
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::removeElementIndex(const Standard_Integer theIndex,
                                                        PairOfIndex&  thePair)
{
  for (Standard_Integer i = 1, n = thePair.Extent(); i <= n; ++i)
  {
    if (thePair.Index(i) == theIndex)
    {
      thePair.RemoveIndex(i);
      return;
    }
  }
}

//=================================================================================================

Standard_Boolean BRepMesh_DataStructureOfDelaun::SubstituteElement(
  const Standard_Integer   theIndex,
  const Triangle3& theNewElement)
{
  const Triangle3& aElement = GetElement(theIndex);
  if (aElement.Movability() == BRepMesh_Deleted)
  {
    myElements(theIndex) = theNewElement;
    return Standard_True;
  }

  cleanElement(theIndex, aElement);
  // Warning: here new element and old element should have different Hash code
  myElements(theIndex) = theNewElement;

  const Standard_Integer(&e)[3] = theNewElement.myEdges;
  for (Standard_Integer i = 0; i < 3; ++i)
    myLinks(e[i]).Append(theIndex);

  return Standard_True;
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::ElementNodes(const Triangle3& theElement,
                                                  Standard_Integer (&theNodes)[3])
{
  const Standard_Integer(&e)[3] = theElement.myEdges;
  const Standard_Boolean(&o)[3] = theElement.myOrientations;

  const Edge3& aLink1 = GetLink(e[0]);
  if (o[0])
  {
    theNodes[0] = aLink1.FirstNode();
    theNodes[1] = aLink1.LastNode();
  }
  else
  {
    theNodes[1] = aLink1.FirstNode();
    theNodes[0] = aLink1.LastNode();
  }

  const Edge3& aLink2 = GetLink(e[2]);
  if (o[2])
    theNodes[2] = aLink2.FirstNode();
  else
    theNodes[2] = aLink2.LastNode();
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::ClearDomain()
{
  IMeshData::MapOfInteger           aFreeEdges;
  IMeshData::IteratorOfMapOfInteger aElementIt(myElementsOfDomain);
  for (; aElementIt.More(); aElementIt.Next())
  {
    const Standard_Integer aElementId = aElementIt.Key1();
    Triangle3&     aElement   = (Triangle3&)GetElement(aElementId);

    const Standard_Integer(&e)[3] = aElement.myEdges;

    for (Standard_Integer i = 0; i < 3; ++i)
      aFreeEdges.Add(e[i]);

    cleanElement(aElementId, aElement);
    aElement.SetMovability(BRepMesh_Deleted);
  }
  myElementsOfDomain.Clear();

  IMeshData::IteratorOfMapOfInteger aEdgeIt(aFreeEdges);
  for (; aEdgeIt.More(); aEdgeIt.Next())
    RemoveLink(aEdgeIt.Key1());
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::clearDeletedLinks()
{
  Standard_Integer aLastLiveItem = NbLinks();
  while (!myDelLinks.IsEmpty())
  {
    while (aLastLiveItem > 0)
    {
      if (GetLink(aLastLiveItem).Movability() != BRepMesh_Deleted)
        break;

      myLinks.RemoveLast();
      --aLastLiveItem;
    }

    Standard_Integer aDelItem = myDelLinks.First();
    myDelLinks.RemoveFirst();

    if (aDelItem > aLastLiveItem)
      continue;

    Edge3         aLink = GetLink(aLastLiveItem);
    PairOfIndex& aPair = myLinks(aLastLiveItem);

    myLinks.RemoveLast();
    myLinks.Substitute(aDelItem, aLink, aPair);

    myLinksOfDomain.Remove(aLastLiveItem);
    myLinksOfDomain.Add(aDelItem);
    --aLastLiveItem;

    const Standard_Integer             aLastLiveItemId = aLastLiveItem + 1;
    IMeshData::ListOfInteger::Iterator aLinkIt;
    // update link references
    for (Standard_Integer i = 0; i < 2; ++i)
    {
      const Standard_Integer aCurNodeId = (i == 0) ? aLink.FirstNode() : aLink.LastNode();

      for (aLinkIt.Init(linksConnectedTo(aCurNodeId)); aLinkIt.More(); aLinkIt.Next())
      {
        Standard_Integer& aLinkId = aLinkIt.ChangeValue();
        if (aLinkId == aLastLiveItemId)
        {
          aLinkId = aDelItem;
          break;
        }
      }
    }

    // update elements references
    for (Standard_Integer j = 1, jn = aPair.Extent(); j <= jn; ++j)
    {
      Standard_Integer         e[3];
      Standard_Boolean         o[3];
      const Triangle3& aElement = GetElement(aPair.Index(j));
      aElement.Edges(e, o);
      for (Standard_Integer i = 0; i < 3; ++i)
      {
        if (e[i] == aLastLiveItemId)
        {
          e[i] = aDelItem;
          break;
        }
      }

      myElements(aLinkIt.Value()) = Triangle3(e, o, aElement.Movability());
    }
  }
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::clearDeletedNodes()
{
  IMeshData::ListOfInteger& aDelNodes = (IMeshData::ListOfInteger&)myNodes->GetListOfDelNodes();

  Standard_Integer aLastLiveItem = NbNodes();
  while (!aDelNodes.IsEmpty())
  {
    while (aLastLiveItem > 0)
    {
      if (GetNode(aLastLiveItem).Movability() != BRepMesh_Deleted)
        break;

      myNodes->RemoveLast();
      --aLastLiveItem;
    }

    Standard_Integer aDelItem = aDelNodes.First();
    aDelNodes.RemoveFirst();

    if (aDelItem > aLastLiveItem)
      continue;

    Vertex           aNode     = GetNode(aLastLiveItem);
    IMeshData::ListOfInteger& aLinkList = linksConnectedTo(aLastLiveItem);

    myNodes->RemoveLast();
    --aLastLiveItem;

    myNodes->Substitute(aDelItem, aNode);
    myNodeLinks.ChangeFind(aDelItem) = aLinkList;

    const Standard_Integer             aLastLiveItemId = aLastLiveItem + 1;
    IMeshData::ListOfInteger::Iterator aLinkIt(aLinkList);
    for (; aLinkIt.More(); aLinkIt.Next())
    {
      const Standard_Integer aLinkId = aLinkIt.Value();
      const Edge3&   aLink   = GetLink(aLinkId);
      PairOfIndex&  aPair   = myLinks(aLinkId);

      Standard_Integer v[2] = {aLink.FirstNode(), aLink.LastNode()};
      if (v[0] == aLastLiveItemId)
        v[0] = aDelItem;
      else if (v[1] == aLastLiveItemId)
        v[1] = aDelItem;

      myLinks.Substitute(aLinkId, Edge3(v[0], v[1], aLink.Movability()), aPair);
    }
  }
}

//=================================================================================================

void BRepMesh_DataStructureOfDelaun::Statistics(Standard_OStream& theStream) const
{
  theStream << " Map of nodes : \n";
  myNodes->Statistics(theStream);
  theStream << "\n Deleted nodes : " << myNodes->GetListOfDelNodes().Extent() << std::endl;

  theStream << "\n\n Map of Links : \n";
  myLinks.Statistics(theStream);
  theStream << "\n Deleted links : " << myDelLinks.Extent() << std::endl;

  theStream << "\n\n Map of elements : \n";
  theStream << "\n Elements : " << myElements.Size() << std::endl;
}

//=======================================================================
// function : BRepMesh_Write
// purpose  :
//  Global function not declared in any public header, intended for use
//  from debugger prompt (Command Window in Visual Studio).
//
//  Stores the mesh data structure to BRep file with the given name.
//=======================================================================
Standard_CString BRepMesh_Dump(void* theMeshHandlePtr, Standard_CString theFileNameStr)
{
  if (theMeshHandlePtr == 0 || theFileNameStr == 0)
  {
    return "Error: file name or mesh data is null";
  }

  Handle(BRepMesh_DataStructureOfDelaun) aMeshData =
    *(Handle(BRepMesh_DataStructureOfDelaun)*)theMeshHandlePtr;

  if (aMeshData.IsNull())
    return "Error: mesh data is empty";

  TopoCompound aMesh;
  ShapeBuilder    aBuilder;
  aBuilder.MakeCompound(aMesh);

  try
  {
    OCC_CATCH_SIGNALS

    if (aMeshData->LinksOfDomain().IsEmpty())
    {
      const Standard_Integer aNodesNb = aMeshData->NbNodes();
      for (Standard_Integer i = 1; i <= aNodesNb; ++i)
      {
        const Coords2d& aNode = aMeshData->GetNode(i).Coord();
        Point3d       aPnt(aNode.X(), aNode.Y(), 0.);
        aBuilder.Add(aMesh, BRepBuilderAPI_MakeVertex(aPnt));
      }
    }
    else
    {
      IMeshData::IteratorOfMapOfInteger aLinksIt(aMeshData->LinksOfDomain());
      for (; aLinksIt.More(); aLinksIt.Next())
      {
        const Edge3& aLink = aMeshData->GetLink(aLinksIt.Key1());
        Point3d               aPnt[2];
        for (Standard_Integer i = 0; i < 2; ++i)
        {
          const Standard_Integer aNodeId = (i == 0) ? aLink.FirstNode() : aLink.LastNode();

          const Coords2d& aNode = aMeshData->GetNode(aNodeId).Coord();
          aPnt[i]            = Point3d(aNode.X(), aNode.Y(), 0.);
        }

        if (aPnt[0].SquareDistance(aPnt[1]) < Precision1::SquareConfusion())
          continue;

        aBuilder.Add(aMesh, EdgeMaker(aPnt[0], aPnt[1]));
      }
    }

    if (!BRepTools1::Write(aMesh, theFileNameStr))
      return "Error: write failed";
  }
  catch (ExceptionBase const& anException)
  {
    return anException.GetMessageString();
  }

  return theFileNameStr;
}

void BRepMesh_DataStructureOfDelaun::Dump(Standard_CString theFileNameStr)
{
  Handle(BRepMesh_DataStructureOfDelaun) aMeshData(this);
  BRepMesh_Dump((void*)&aMeshData, theFileNameStr);
}
