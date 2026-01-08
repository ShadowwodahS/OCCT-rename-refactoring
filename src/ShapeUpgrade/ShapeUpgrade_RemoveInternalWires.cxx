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

#include <Precision.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeUpgrade_RemoveInternalWires.hxx>
#include <Standard_Type.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShapeUpgrade_RemoveInternalWires, ShapeUpgrade_Tool)

//=================================================================================================

ShapeUpgrade_RemoveInternalWires::ShapeUpgrade_RemoveInternalWires()
{
  myMinArea                           = 0.;
  myRemoveFacesMode                   = Standard_True;
  myStatus                            = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  Handle(ShapeBuild_ReShape) aContext = new ShapeBuild_ReShape;
  SetContext(aContext);
}

//=================================================================================================

ShapeUpgrade_RemoveInternalWires::ShapeUpgrade_RemoveInternalWires(const TopoShape& theShape)
{
  Handle(ShapeBuild_ReShape) aContext = new ShapeBuild_ReShape;
  SetContext(aContext);
  Init(theShape);
}

//=================================================================================================

void ShapeUpgrade_RemoveInternalWires::Init(const TopoShape& theShape)
{
  myShape = theShape;
  Context()->Apply(theShape);
  TopExp1::MapShapesAndAncestors(myShape, TopAbs_EDGE, TopAbs_FACE, myEdgeFaces);
  myStatus          = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
  myMinArea         = 0.;
  myRemoveFacesMode = Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeUpgrade_RemoveInternalWires::Perform()
{
  Clear();
  if (myShape.IsNull())
  {
    myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    return Standard_False;
  }
  ShapeExplorer aExpF(myShape, TopAbs_FACE);
  for (; aExpF.More(); aExpF.Next())
  {
    TopoFace aF = TopoDS::Face(aExpF.Current());
    removeSmallWire(aF, TopoWire());
  }
  if (myRemoveFacesMode)
    removeSmallFaces();

  myResult = Context()->Apply(myShape);
  return Status(ShapeExtend_DONE);
}

//=================================================================================================

Standard_Boolean ShapeUpgrade_RemoveInternalWires::Perform(
  const TopTools_SequenceOfShape& theSeqShapes)
{
  if (myShape.IsNull())
  {
    myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL1);
    return Standard_False;
  }
  Clear();
  TopTools_IndexedDataMapOfShapeListOfShape aWireFaces;
  Standard_Integer                          i = 1, nb = theSeqShapes.Length();
  for (; i <= nb; i++)
  {
    const TopoShape& aS = theSeqShapes.Value(i);
    if (aS.ShapeType() == TopAbs_FACE)
      removeSmallWire(aS, TopoWire());
    else if (aS.ShapeType() == TopAbs_WIRE)
    {
      if (!aWireFaces.Extent())
        TopExp1::MapShapesAndAncestors(myShape, TopAbs_WIRE, TopAbs_FACE, aWireFaces);
      if (aWireFaces.Contains(aS))
      {
        const ShapeList&        alfaces = aWireFaces.FindFromKey(aS);
        TopTools_ListIteratorOfListOfShape liter(alfaces);
        for (; liter.More(); liter.Next())
          removeSmallWire(liter.Value(), aS);
      }
    }
  }
  if (myRemoveFacesMode)
    removeSmallFaces();
  myResult = Context()->Apply(myShape);
  return Status(ShapeExtend_DONE);
}

//=================================================================================================

void ShapeUpgrade_RemoveInternalWires::removeSmallWire(const TopoShape& theFace,
                                                       const TopoShape& theWire)
{
  TopoFace     aF     = TopoDS::Face(theFace);
  TopoWire     anOutW = ShapeAnalysis1::OuterWire(aF);
  TopoDS_Iterator aIt(aF);
  for (; aIt.More(); aIt.Next())
  {
    if (aIt.Value().ShapeType() != TopAbs_WIRE || aIt.Value().IsSame(anOutW))
      continue;
    // Handle(ShapeExtend_WireData) asewd = new  ShapeExtend_WireData();
    TopoWire aW = TopoDS::Wire(aIt.Value());
    if (!theWire.IsNull() && !theWire.IsSame(aW))
      continue;
    Standard_Real anArea = ShapeAnalysis1::ContourArea(aW);
    if (anArea < myMinArea - Precision1::Confusion())
    {
      Context()->Remove(aW);
      myRemoveWires.Append(aW);
      myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE1);
      if (!myRemoveFacesMode)
        continue;

      TopoDS_Iterator aIte(aW, Standard_False);
      for (; aIte.More(); aIte.Next())
      {
        const TopoShape& aE = aIte.Value();
        if (myRemoveEdges.IsBound(aE))
          myRemoveEdges.ChangeFind(aE).Append(aF);
        else
        {
          ShapeList alfaces;
          alfaces.Append(aF);
          myRemoveEdges.Bind(aE, alfaces);
        }
      }
    }
  }
}

//=================================================================================================

void ShapeUpgrade_RemoveInternalWires::removeSmallFaces()
{

  Standard_Integer i = 1;
  for (; i <= myRemoveWires.Length(); i++)
  {
    TopoShape               aWire = myRemoveWires.Value(i);
    TopoDS_Iterator            aIte(aWire, Standard_False);
    TopTools_IndexedMapOfShape aFaceCandidates;
    // collecting all faces containing edges from removed wire
    for (; aIte.More(); aIte.Next())
    {

      const TopoShape& aEdge = aIte.Value();
      if (!myEdgeFaces.Contains(aEdge))
      {
        myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_FAIL2);
        continue;
      }
      const ShapeList&        aLface1 = myEdgeFaces.FindFromKey(aEdge);
      const ShapeList&        aLface2 = myRemoveEdges.Find(aEdge);
      TopTools_ListIteratorOfListOfShape aliter(aLface1);
      TopTools_ListIteratorOfListOfShape aliter2(aLface2);
      for (; aliter.More(); aliter.Next())
      {
        TopoShape aF = Context()->Apply(aliter.Value());
        if (aF.IsNull())
          continue;
        Standard_Boolean isFind = Standard_False;
        for (; aliter2.More() && !isFind; aliter2.Next())
        {
          TopoShape aF2 = Context()->Apply(aliter2.Value());
          isFind           = aF.IsSame(aF2);
        }

        if (!isFind)
        {
          TopoWire      aWout   = ShapeAnalysis1::OuterWire(TopoDS::Face(aF));
          Standard_Boolean isOuter = Standard_False;
          TopoDS_Iterator  aIter(aWout, Standard_False);
          for (; aIter.More() && !isOuter; aIter.Next())
            isOuter = aEdge.IsSame(aIter.Value());
          if (isOuter)
            aFaceCandidates.Add(aF);
        }
      }
    }

    // remove faces which have outer wire consist of only
    // edges from removed wires and
    // seam edges for faces based on conic surface or
    // in the case of a few faces based on the same conic surface
    // the edges belogining these faces.
    Standard_Integer k = 1;
    for (; k <= aFaceCandidates.Extent(); k++)
    {
      TopoShape                 aF     = aFaceCandidates.FindKey(k);
      TopoWire                  anOutW = ShapeAnalysis1::OuterWire(TopoDS::Face(aF));
      Handle(ShapeExtend_WireData) asewd  = new ShapeExtend_WireData(anOutW);
      Standard_Integer             n = 1, nbE = asewd->NbEdges();
      Standard_Integer             nbNotRemoved = 0;
      for (; n <= nbE; n++)
      {
        if (asewd->IsSeam(n))
          continue;
        TopoEdge aE = asewd->Edge(n);
        if (!myRemoveEdges.IsBound(aE))
        {
          const ShapeList&        aLface3 = myEdgeFaces.FindFromKey(aE);
          TopTools_ListIteratorOfListOfShape aliter3(aLface3);
          for (; aliter3.More(); aliter3.Next())
          {
            TopoShape aF2 = Context()->Apply(aliter3.Value());
            if (aF2.IsNull())
              continue;
            if (!aF.IsSame(aF2) && !aFaceCandidates.Contains(aF2))
              nbNotRemoved++;
          }
        }
      }

      if (!nbNotRemoved)
      {
        Context()->Remove(aF);
        myRemovedFaces.Append(aF);
      }
    }
  }

  if (myRemovedFaces.Length())
    myStatus |= ShapeExtend1::EncodeStatus(ShapeExtend_DONE2);
}

//=================================================================================================

void ShapeUpgrade_RemoveInternalWires::Clear()
{
  myRemoveEdges.Clear();
  myRemovedFaces.Clear();
  myRemoveWires.Clear();
  myStatus = ShapeExtend1::EncodeStatus(ShapeExtend_OK);
}
