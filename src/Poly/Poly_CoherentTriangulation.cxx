// Created on: 2007-12-08
// Created by: Alexander GRIGORIEV
// Copyright (c) 2007-2014 OPEN CASCADE SAS
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

#include <Poly_CoherentTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <NCollection_List.hxx>
#include <Precision.hxx>
#include <Standard_ProgramError.hxx>
#include <TShort_Array1OfShortReal.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Poly_CoherentTriangulation, RefObject)

//=======================================================================
// function : Poly_CoherentTriangulation
// purpose  : Empty constructor
//=======================================================================

Poly_CoherentTriangulation::Poly_CoherentTriangulation(
  const Handle(NCollection_BaseAllocator)& theAlloc)
    : myAlloc(theAlloc.IsNull() ? NCollection_BaseAllocator::CommonBaseAllocator() : theAlloc),
      myDeflection(0.)
{
}

//=================================================================================================

Poly_CoherentTriangulation::Poly_CoherentTriangulation(
  const Handle(MeshTriangulation)&        theTriangulation,
  const Handle(NCollection_BaseAllocator)& theAlloc)
    : myAlloc(theAlloc.IsNull() ? NCollection_BaseAllocator::CommonBaseAllocator() : theAlloc)
{
  if (theTriangulation.IsNull() == Standard_False)
  {
    const Standard_Integer nNodes = theTriangulation->NbNodes();
    Standard_Integer       i;

    // Copy the nodes
    for (i = 0; i < nNodes; i++)
    {
      const Standard_Integer anOldInd    = i + 1;
      const Standard_Integer aNewInd     = SetNode(theTriangulation->Node(anOldInd).XYZ(), i);
      Poly_CoherentNode&     aCopiedNode = myNodes(aNewInd);
      aCopiedNode.SetIndex(anOldInd);
    }

    // Copy the triangles
    for (i = 1; i <= theTriangulation->NbTriangles(); i++)
    {
      Standard_Integer iNode[3];
      theTriangulation->Triangle1(i).Get(iNode[0], iNode[1], iNode[2]);
      if (iNode[0] != iNode[1] && iNode[1] != iNode[2] && iNode[2] != iNode[0])
        AddTriangle(iNode[0] - 1, iNode[1] - 1, iNode[2] - 1);
    }

    // Copy UV coordinates of nodes
    if (theTriangulation->HasUVNodes())
    {
      for (i = 0; i < nNodes; i++)
      {
        const gp_Pnt2d anUV = theTriangulation->UVNode(i + 1);
        myNodes(i).SetUV(anUV.X(), anUV.Y());
      }
    }

    // Copy the normals at nodes
    if (theTriangulation->HasNormals())
    {
      gp_Vec3f aNormal;
      for (i = 0; i < nNodes; i++)
      {
        theTriangulation->Normal(i + 1, aNormal);
        myNodes(i).SetNormal(Coords3d(aNormal.x(), aNormal.y(), aNormal.z()));
      }
    }
    myDeflection = theTriangulation->Deflection();
  }
}

//=================================================================================================

Poly_CoherentTriangulation::~Poly_CoherentTriangulation()
{
  NCollection_Vector<Poly_CoherentNode>::Iterator anIter(myNodes);
  for (; anIter.More(); anIter.Next())
  {
    anIter.ChangeValue().Clear(myAlloc);
  }
}

//=================================================================================================

Handle(MeshTriangulation) Poly_CoherentTriangulation::GetTriangulation() const
{
  const Standard_Integer nNodes     = NNodes();
  const Standard_Integer nTriangles = NTriangles();
  if (nNodes == 0 || nTriangles == 0)
  {
    return Handle(MeshTriangulation)();
  }

  Handle(MeshTriangulation) aResult = new MeshTriangulation(nNodes, nTriangles, false);

  NCollection_Vector<Standard_Integer> vecNodeId;
  Standard_Integer                     aCount = 0;

  // Copy the nodes (3D and 2D coordinates)
  for (Standard_Integer i = 0; i < myNodes.Length(); i++)
  {
    const Poly_CoherentNode& aNode = myNodes(i);
    if (aNode.IsFreeNode())
    {
      vecNodeId.SetValue(i, 0);
      continue;
    }

    vecNodeId.SetValue(i, ++aCount);
    const Coords3d aNormal = aNode.GetNormal();
    if (aNormal.SquareModulus() > Precision1::Confusion())
    {
      aResult->AddNormals();
      aResult->SetNormal(aCount, Dir3d(aNormal));
    }

    aResult->SetNode(aCount, aNode);
    if (aNode.GetU() * aNode.GetU() + aNode.GetV() * aNode.GetV() > Precision1::Confusion())
    {
      aResult->AddUVNodes();
      aResult->SetUVNode(aCount, gp_Pnt2d(aNode.GetU(), aNode.GetV()));
    }
  }

  // Copy the triangles
  aCount = 0;
  for (NCollection_Vector<CoherentTriangle>::Iterator anIterT(myTriangles); anIterT.More();
       anIterT.Next())
  {
    const CoherentTriangle& aTri = anIterT.Value();
    if (!aTri.IsEmpty())
    {
      aResult->SetTriangle(
        ++aCount,
        Triangle2(vecNodeId(aTri.Node(0)), vecNodeId(aTri.Node(1)), vecNodeId(aTri.Node(2))));
    }
  }

  aResult->Deflection(myDeflection);
  return aResult;
}

//=======================================================================
// function : GetFreeNodes
// purpose  : Create a list of free nodes.
//=======================================================================

Standard_Boolean Poly_CoherentTriangulation::GetFreeNodes(
  NCollection_List<Standard_Integer>& lstNodes) const
{
  lstNodes.Clear();
  Standard_Integer i;
  for (i = 0; i < myNodes.Length(); i++)
  {
    const Poly_CoherentNode& aNode = myNodes(i);
    if (aNode.IsFreeNode())
      lstNodes.Append(i);
  }
  return !lstNodes.IsEmpty();
}

//=======================================================================
// function : RemoveDegenerated
// purpose  : Find and remove degenerated triangles in Triangulation.
//=======================================================================

Standard_Boolean Poly_CoherentTriangulation::RemoveDegenerated(
  const Standard_Real                                        theTol,
  NCollection_List<Poly_CoherentTriangulation::TwoIntegers1>* pLstRemovedNode)
{
  Standard_Boolean        aResult(Standard_False);
  const Standard_Real     aTol2  = theTol * theTol;
  const Standard_Integer  ind0[] = {2, 0, 1, 2, 0};
  const Standard_Integer* ind    = &ind0[1];
  if (pLstRemovedNode)
    pLstRemovedNode->Clear();

  // NCollection_Vector<CoherentTriangle>::Iterator anIterT(myTriangles);
  Poly_CoherentTriangulation::IteratorOfTriangle1 anIterT(this);
  for (; anIterT.More(); anIterT.Next())
  {
    CoherentTriangle& aTri     = anIterT.ChangeValue();
    Poly_CoherentNode*     pNode[3] = {&ChangeNode(aTri.Node(0)),
                                       &ChangeNode(aTri.Node(1)),
                                       &ChangeNode(aTri.Node(2))};
    const Standard_Real    aLen2[3] = {pNode[2]->Subtracted(*pNode[1]).SquareModulus(),
                                       pNode[0]->Subtracted(*pNode[2]).SquareModulus(),
                                       pNode[1]->Subtracted(*pNode[0]).SquareModulus()};
    for (Standard_Integer i = 0; i < 3; i++)
    {
      if (aLen2[i] < aTol2)
      {
        const Standard_Integer im1(aTri.Node(ind[i - 1]));
        const Standard_Integer ip1(aTri.Node(ind[i + 1]));

        // Disconnect from both neighbours
        RemoveTriangle(aTri);

        // Reconnect all triangles from Node(ind[i+1]) to Node(ind[i-1])
        for (;;)
        {
          CoherentTriPtr::Iterator anIterConn = pNode[ind[i + 1]]->TriangleIterator();
          if (!anIterConn.More())
          {
            break;
          }

          CoherentTriangle& aTriConn = anIterConn.ChangeValue();
          Standard_Integer aNewTriConn[]  = {aTriConn.Node(0), aTriConn.Node(1), aTriConn.Node(2)};
          if (&aTriConn != &aTri)
          {
            if (aNewTriConn[0] == ip1)
              aNewTriConn[0] = im1;
            else if (aNewTriConn[1] == ip1)
              aNewTriConn[1] = im1;
            else if (aNewTriConn[2] == ip1)
              aNewTriConn[2] = im1;

            RemoveTriangle(aTriConn);
            AddTriangle(aNewTriConn[0], aNewTriConn[1], aNewTriConn[2]);
          }
          else
          {
            anIterConn.Next();
            if (!anIterConn.More())
            {
              break;
            }
          }
        }
        if (pLstRemovedNode)
        {
          pLstRemovedNode->Append(TwoIntegers1(ip1, im1));
        }
        aResult = Standard_True;
        break;
      }
    }
  }
  return aResult;
}

//=================================================================================================

Poly_CoherentTriangulation::IteratorOfTriangle1::IteratorOfTriangle1(
  const Handle(Poly_CoherentTriangulation)& theTri)
{
  if (!theTri.IsNull())
  {
    Init(theTri->myTriangles);
    while (More())
    {
      const CoherentTriangle& aTri = Value();
      if (aTri.IsEmpty() == Standard_False)
        break;
      Poly_BaseIteratorOfCoherentTriangle::Next();
    }
  }
}

//=================================================================================================

void Poly_CoherentTriangulation::IteratorOfTriangle1::Next()
{
  Poly_BaseIteratorOfCoherentTriangle::Next();
  while (More())
  {
    const CoherentTriangle& aTri = Value();
    if (aTri.IsEmpty() == Standard_False)
      break;
    Poly_BaseIteratorOfCoherentTriangle::Next();
  }
}

//=================================================================================================

Poly_CoherentTriangulation::IteratorOfNode1::IteratorOfNode1(
  const Handle(Poly_CoherentTriangulation)& theTri)
{
  if (!theTri.IsNull())
  {
    Init(theTri->myNodes);
    while (More())
    {
      if (Value().IsFreeNode() == Standard_False)
        break;
      Poly_BaseIteratorOfCoherentNode::Next();
    }
  }
}

//=================================================================================================

void Poly_CoherentTriangulation::IteratorOfNode1::Next()
{
  Poly_BaseIteratorOfCoherentNode::Next();
  while (More())
  {
    if (Value().IsFreeNode() == Standard_False)
      break;
    Poly_BaseIteratorOfCoherentNode::Next();
  }
}

//=================================================================================================

Poly_CoherentTriangulation::IteratorOfLink1::IteratorOfLink1(
  const Handle(Poly_CoherentTriangulation)& theTri)
{
  if (!theTri.IsNull())
  {
    Init(theTri->myLinks);
    while (More())
    {
      if (Value().IsEmpty() == Standard_False)
        break;
      Poly_BaseIteratorOfCoherentLink::Next();
    }
  }
}

//=================================================================================================

void Poly_CoherentTriangulation::IteratorOfLink1::Next()
{
  Poly_BaseIteratorOfCoherentLink::Next();
  while (More())
  {
    if (Value().IsEmpty() == Standard_False)
      break;
    Poly_BaseIteratorOfCoherentLink::Next();
  }
}

//=================================================================================================

Standard_Integer Poly_CoherentTriangulation::NNodes() const
{
  Standard_Integer                                aCount(0);
  NCollection_Vector<Poly_CoherentNode>::Iterator anIter(myNodes);
  for (; anIter.More(); anIter.Next())
    if (anIter.Value().IsFreeNode() == Standard_False)
      aCount++;
  return aCount;
}

//=================================================================================================

Standard_Integer Poly_CoherentTriangulation::NTriangles() const
{
  Standard_Integer                                    aCount(0);
  NCollection_Vector<CoherentTriangle>::Iterator anIter(myTriangles);
  for (; anIter.More(); anIter.Next())
  {
    const CoherentTriangle& aTri = anIter.Value();
    if (aTri.IsEmpty() == Standard_False)
      aCount++;
  }
  return aCount;
}

//=================================================================================================

Standard_Integer Poly_CoherentTriangulation::NLinks() const
{
  Standard_Integer                                aCount(0);
  NCollection_Vector<CoherentLink>::Iterator anIter(myLinks);
  for (; anIter.More(); anIter.Next())
  {
    if (anIter.Value().IsEmpty() == Standard_False)
      aCount++;
  }
  return aCount;
}

//=================================================================================================

Standard_Integer Poly_CoherentTriangulation::SetNode(const Coords3d&          thePnt,
                                                     const Standard_Integer iNode)
{
  Standard_Integer aResult = myNodes.Length();
  if (iNode < 0)
    myNodes.Append(Poly_CoherentNode(thePnt));
  else
  {
    myNodes.SetValue(iNode, Poly_CoherentNode(thePnt));
    aResult = iNode;
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean Poly_CoherentTriangulation::RemoveTriangle(CoherentTriangle& theTriangle)
{
  Standard_Boolean aResult(Standard_False);
  for (Standard_Integer i = 0; i < 3; i++)
  {
    if (theTriangle.Node(i) >= 0)
    {
      Poly_CoherentNode& aNode = myNodes(theTriangle.Node(i));
      // If Links exist in this Triangulation, remove or update a Link1
      CoherentLink* aLink = const_cast<CoherentLink*>(theTriangle.mypLink[i]);
      if (aLink)
      {
        const CoherentTriangle* pTriOpp = theTriangle.GetConnectedTri(i);
        Standard_Boolean             toRemoveLink(Standard_True);
        if (pTriOpp != 0L)
        {
          // A neighbour is detected. If a Link1 exists on it, update it,
          // otherwise remove this link
          for (Standard_Integer j = 0; j < 3; j++)
          {
            if (aLink == pTriOpp->GetLink(j))
            {
              if (aLink->OppositeNode(0) == theTriangle.Node(i))
              {
                aLink->myOppositeNode[0] = -1;
                toRemoveLink             = Standard_False;
              }
              else if (aLink->OppositeNode(1) == theTriangle.Node(i))
              {
                aLink->myOppositeNode[1] = -1;
                toRemoveLink             = Standard_False;
              }
              break;
            }
          }
        }
        if (toRemoveLink)
          RemoveLink(*aLink);
      }
      if (aNode.RemoveTriangle(theTriangle, myAlloc))
      {
        theTriangle.myNodes[i] = -1;
        aResult                = Standard_True;
      }
    }
    theTriangle.RemoveConnection(i);
  }
  return aResult;
}

//=================================================================================================

CoherentTriangle* Poly_CoherentTriangulation::AddTriangle(const Standard_Integer iNode0,
                                                               const Standard_Integer iNode1,
                                                               const Standard_Integer iNode2)
{
  CoherentTriangle* pTriangle = 0L;
  if (iNode0 >= 0 && iNode1 >= 0 && iNode2 >= 0)
  {
    pTriangle = &myTriangles.Append(CoherentTriangle());
    ReplaceNodes(*pTriangle, iNode0, iNode1, iNode2);
  }
  return pTriangle;
}

//=================================================================================================

Standard_Boolean Poly_CoherentTriangulation::ReplaceNodes(CoherentTriangle& theTriangle,
                                                          const Standard_Integer iNode0,
                                                          const Standard_Integer iNode1,
                                                          const Standard_Integer iNode2)
{
  if (!theTriangle.IsEmpty())
    RemoveTriangle(theTriangle);
  if (iNode0 >= 0 && iNode1 >= 0 && iNode2 >= 0)
  {
    theTriangle = CoherentTriangle(iNode0, iNode1, iNode2);
    for (Standard_Integer i = 0; i < 3; i++)
    {
      Poly_CoherentNode&            aNode   = myNodes(theTriangle.Node(i));
      CoherentTriPtr::Iterator anIterT = aNode.TriangleIterator();
      for (; anIterT.More(); anIterT.Next())
      {
        anIterT.ChangeValue().SetConnection(theTriangle);
      }
      aNode.AddTriangle(theTriangle, myAlloc);
    }

    // If Links exist in this Triangulation, create or update a Link1
    if (myLinks.Length() > 0)
    {
      for (Standard_Integer i = 0; i < 3; i++)
      {
        const CoherentTriangle* pTriOpp = theTriangle.GetConnectedTri(i);
        Standard_Boolean             toAddLink(Standard_True);
        if (pTriOpp != 0L)
        {
          // A neighbour is detected. If a Link1 exists on it, update it,
          // otherwise create a new link.
          for (Standard_Integer j = 0; j < 3; j++)
          {
            if (theTriangle.Node(i) == pTriOpp->GetConnectedNode(j))
            {
              CoherentLink* aLink = const_cast<CoherentLink*>(pTriOpp->GetLink(j));
              if (aLink != 0L)
              {
                if (aLink->OppositeNode(0) == pTriOpp->Node(j))
                {
                  aLink->myOppositeNode[1] = theTriangle.Node(i);
                  toAddLink                = Standard_False;
                }
                else if (aLink->OppositeNode(1) == pTriOpp->Node(j))
                {
                  aLink->myOppositeNode[0] = theTriangle.Node(i);
                  toAddLink                = Standard_False;
                }
              }
            }
          }
        }
        if (toAddLink)
        {
          // No neighbor on this side, the new Link1 is created.
          AddLink(theTriangle, i);
        }
      }
    }
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void Poly_CoherentTriangulation::RemoveLink(CoherentLink& theLink)
{
  const CoherentTriangle* pTri[2] = {0L, 0L};
  if (FindTriangle(theLink, pTri))
  {
    for (Standard_Integer i = 0; i < 2; i++)
    {
      const Standard_Integer iNode = theLink.OppositeNode(i);
      if (iNode >= 0 && pTri[i] != 0L)
      {
        if (iNode == pTri[i]->Node(0))
          const_cast<CoherentTriangle*>(pTri[i])->mypLink[0] = 0L;
        else if (iNode == pTri[i]->Node(1))
          const_cast<CoherentTriangle*>(pTri[i])->mypLink[1] = 0L;
        else if (iNode == pTri[i]->Node(2))
          const_cast<CoherentTriangle*>(pTri[i])->mypLink[2] = 0L;
        else
          throw Standard_ProgramError("Poly_CoherentTriangulation::RemoveLink: "
                                      " wrong connectivity between triangles");
      }
    }
  }
  theLink = CoherentLink();
}

//=================================================================================================

CoherentLink* Poly_CoherentTriangulation::AddLink(const CoherentTriangle& theTri,
                                                       const Standard_Integer       theConn)
{
  CoherentLink* pLink = 0L;
  if (theTri.IsEmpty() == Standard_False)
  {
    pLink = &myLinks.Append(CoherentLink(theTri, theConn));
    const_cast<CoherentTriangle&>(theTri).mypLink[theConn] = pLink;
    const CoherentTriangle* pTriOpp                        = theTri.GetConnectedTri(theConn);

    if (!pTriOpp)
      return pLink;
    if (pTriOpp->IsEmpty())
      return pLink;

    if (pTriOpp)
    {
      if (pTriOpp->Node(0) == theTri.GetConnectedNode(theConn))
        const_cast<CoherentTriangle*>(pTriOpp)->mypLink[0] = pLink;
      else if (pTriOpp->Node(1) == theTri.GetConnectedNode(theConn))
        const_cast<CoherentTriangle*>(pTriOpp)->mypLink[1] = pLink;
      else if (pTriOpp->Node(2) == theTri.GetConnectedNode(theConn))
        const_cast<CoherentTriangle*>(pTriOpp)->mypLink[2] = pLink;
      else
        throw Standard_ProgramError("Poly_CoherentTriangulation::AddLink: "
                                    "Bad connectivity of triangles");
    }
  }
  return pLink;
}

//=================================================================================================

Standard_Boolean Poly_CoherentTriangulation::FindTriangle(
  const CoherentLink&     theLink,
  const CoherentTriangle* pTri[2]) const
{
  pTri[0]                       = 0L;
  pTri[1]                       = 0L;
  const Standard_Integer iNode0 = theLink.Node(0);
  if (theLink.IsEmpty() == Standard_False && iNode0 < myNodes.Length()
      && theLink.Node(1) < myNodes.Length())
  {
    CoherentTriPtr::Iterator anIter0 = myNodes(iNode0).TriangleIterator();
    for (; anIter0.More(); anIter0.Next())
    {
      const CoherentTriangle& aTri = anIter0.Value();
      if (aTri.Node(0) == iNode0)
      {
        if (aTri.Node(1) == theLink.Node(1))
          pTri[0] = &aTri;
        else if (aTri.Node(2) == theLink.Node(1))
          pTri[1] = &aTri;
      }
      else if (aTri.Node(1) == iNode0)
      {
        if (aTri.Node(2) == theLink.Node(1))
          pTri[0] = &aTri;
        else if (aTri.Node(0) == theLink.Node(1))
          pTri[1] = &aTri;
      }
      else if (aTri.Node(2) == iNode0)
      {
        if (aTri.Node(0) == theLink.Node(1))
          pTri[0] = &aTri;
        else if (aTri.Node(1) == theLink.Node(1))
          pTri[1] = &aTri;
      }
      else
        throw Standard_ProgramError("Poly_CoherentTriangulation::FindTriangle : "
                                    " Data incoherence detected");
      if (pTri[0] && pTri[1])
        break;
    }
  }
  return (pTri[0] != 0L || pTri[1] != 0L);
}

//=================================================================================================

Standard_Integer Poly_CoherentTriangulation::ComputeLinks()
{
  myLinks.Clear();
  NCollection_Vector<CoherentTriangle>::Iterator anIter(myTriangles);
  for (; anIter.More(); anIter.Next())
  {
    const CoherentTriangle& aTriangle = anIter.Value();

    if (aTriangle.IsEmpty())
      continue;

    if (aTriangle.Node(0) < aTriangle.Node(1))
      AddLink(aTriangle, 2);
    if (aTriangle.Node(1) < aTriangle.Node(2))
      AddLink(aTriangle, 0);
    if (aTriangle.Node(2) < aTriangle.Node(0))
      AddLink(aTriangle, 1);
  }
  // Above algorithm does not create all boundary links, so
  // it is necessary to check triangles and add absentee links
  anIter.Init(myTriangles);
  Standard_Integer i;
  for (; anIter.More(); anIter.Next())
  {
    CoherentTriangle& aTriangle = anIter.ChangeValue();

    if (aTriangle.IsEmpty())
      continue;

    for (i = 0; i < 3; ++i)
    {
      if (aTriangle.mypLink[i] == 0L)
      {
        AddLink(aTriangle, i);
      }
    }
  }
  return myLinks.Length();
}

//=================================================================================================

void Poly_CoherentTriangulation::ClearLinks()
{
  myLinks.Clear();
  NCollection_Vector<CoherentTriangle>::Iterator anIter(myTriangles);
  for (; anIter.More(); anIter.Next())
  {
    CoherentTriangle& aTriangle = anIter.ChangeValue();
    aTriangle.mypLink[0]             = 0L;
    aTriangle.mypLink[1]             = 0L;
    aTriangle.mypLink[2]             = 0L;
  }
}

//=================================================================================================

Handle(Poly_CoherentTriangulation) Poly_CoherentTriangulation::Clone(
  const Handle(NCollection_BaseAllocator)& theAlloc) const
{
  Handle(Poly_CoherentTriangulation) newTri;

  if (NTriangles() != 0 && NNodes() != 0)
  {
    Handle(MeshTriangulation) theTriangulation = GetTriangulation();
    newTri = new Poly_CoherentTriangulation(theTriangulation, theAlloc);
    newTri->SetDeflection(theTriangulation->Deflection());
  }

  return newTri;
}

//=================================================================================================

void Poly_CoherentTriangulation::Dump(Standard_OStream& theStream) const
{
  for (Standard_Integer iNode = 0; iNode < myNodes.Length(); iNode++)
  {
    const Poly_CoherentNode& aNode = myNodes(iNode);
    if (aNode.IsFreeNode())
      continue;
    theStream << "Node " << iNode;
    aNode.Dump(theStream);
  }
}
