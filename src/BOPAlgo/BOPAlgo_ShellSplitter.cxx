// Created by: Peter KURNEV
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

// File: BOPAlgo_ShellSplitter.cxx
// Created: Thu Jan 16 08:33:50 2014

#include <BOPAlgo_ShellSplitter.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BOPTools_CoupleOfShape.hxx>
#include <BOPTools_Parallel.hxx>
#include <BRep_Builder.hxx>
#include <IntTools_Context.hxx>
#include <NCollection_Vector.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfOrientedShape.hxx>
#include <TopTools_MapOfShape.hxx>

//
static void MakeShell(const ShapeList&, TopoShell&);
//
static void RefineShell(TopoShell&                                    theShell,
                        const TopTools_IndexedDataMapOfShapeListOfShape& theMEF,
                        ShapeList&                            aLShX);

//=================================================================================================

class BOPAlgo_CBK
{
public:
  BOPAlgo_CBK()
      : myPCB(NULL)
  {
  }

  //
  ~BOPAlgo_CBK() {}

  //
  void SetConnexityBlock(const BOPTools_ConnexityBlock& aCB)
  {
    myPCB = (BOPTools_ConnexityBlock*)&aCB;
  }

  //
  BOPTools_ConnexityBlock& ConnexityBlock() { return *myPCB; }

  //
  void SetProgressRange(const Message_ProgressRange& theRange) { myProgressRange = theRange; }

  //
  void Perform()
  {
    Message_ProgressScope aPS(myProgressRange, NULL, 1);
    if (!aPS.More())
    {
      return;
    }
    BOPAlgo_ShellSplitter::SplitBlock(*myPCB);
  }

protected:
  BOPTools_ConnexityBlock* myPCB;
  Message_ProgressRange    myProgressRange;
};

//=======================================================================
typedef NCollection_Vector<BOPAlgo_CBK> BOPAlgo_VectorOfCBK;

//=================================================================================================

BOPAlgo_ShellSplitter::BOPAlgo_ShellSplitter()
    : BOPAlgo_Algo(),
      myStartShapes(myAllocator),
      myShells(myAllocator),
      myLCB(myAllocator)
{
}

//=================================================================================================

BOPAlgo_ShellSplitter::BOPAlgo_ShellSplitter(const Handle(NCollection_BaseAllocator)& theAllocator)
    : BOPAlgo_Algo(theAllocator),
      myStartShapes(theAllocator),
      myShells(theAllocator),
      myLCB(myAllocator)
{
}

//=================================================================================================

BOPAlgo_ShellSplitter::~BOPAlgo_ShellSplitter() {}

//=================================================================================================

void BOPAlgo_ShellSplitter::AddStartElement(const TopoShape& aE)
{
  myStartShapes.Append(aE);
}

//=================================================================================================

const ShapeList& BOPAlgo_ShellSplitter::StartElements() const
{
  return myStartShapes;
}

//=================================================================================================

const ShapeList& BOPAlgo_ShellSplitter::Shells() const
{
  return myShells;
}

//=================================================================================================

void BOPAlgo_ShellSplitter::Perform(const Message_ProgressRange& theRange)
{
  GetReport()->Clear();
  Message_ProgressScope aPS(theRange, "Building shells", 1);
  //
  AlgoTools::MakeConnexityBlocks(myStartShapes, TopAbs_EDGE, TopAbs_FACE, myLCB);
  if (UserBreak(aPS))
  {
    return;
  }

  MakeShells(aPS.Next());
}

//=================================================================================================

void BOPAlgo_ShellSplitter::SplitBlock(BOPTools_ConnexityBlock& aCB)
{
  Standard_Integer                          aNbLF, aNbOff, aNbFP;
  Standard_Integer                          i;
  TopAbs_Orientation                        anOr;
  TopoEdge                               aEL;
  ShapeBuilder                              aBB;
  TopoDS_Iterator                           aItS;
  ShapeExplorer                           aExp;
  TopTools_ListIteratorOfListOfShape        aItF;
  BOPTools_CoupleOfShape                    aCSOff;
  TopTools_MapOfOrientedShape               AddedFacesMap;
  TopTools_IndexedDataMapOfShapeListOfShape aEFMap, aMEFP;
  Handle(IntTools_Context)                  aContext;
  //
  aContext = new IntTools_Context;
  //
  const ShapeList& myShapes = aCB.Shapes();
  //
  ShapeList& myLoops = aCB.ChangeLoops();
  myLoops.Clear();
  //
  // Copy faces into the map, for recursive search of free bounds
  TopTools_MapOfOrientedShape aMFaces;
  aItF.Initialize(myShapes);
  for (; aItF.More(); aItF.Next())
  {
    aMFaces.Add(aItF.Value());
  }
  //
  // remove the faces with free edges from processing
  for (;;)
  {
    // map the shapes
    aEFMap.Clear();
    aItF.Initialize(myShapes);
    for (; aItF.More(); aItF.Next())
    {
      const TopoShape& aF = aItF.Value();
      if (aMFaces.Contains(aF))
      {
        TopExp1::MapShapesAndAncestors(aF, TopAbs_EDGE, TopAbs_FACE, aEFMap);
      }
    }
    //
    Standard_Integer aNbBegin = aMFaces.Extent();
    // check the free edges
    Standard_Integer aNbE = aEFMap.Extent();
    for (i = 1; i <= aNbE; ++i)
    {
      const TopoEdge& aE = TopoDS::Edge(aEFMap.FindKey(i));
      if (!(BRepInspector::Degenerated(aE) || aE.Orientation() == TopAbs_INTERNAL))
      {
        const ShapeList& aLF = aEFMap(i);
        if (aLF.Extent() == 1)
        {
          // remove the face
          aMFaces.Remove(aLF.First());
        }
      }
    }
    //
    // check if any faces have been removed
    Standard_Integer aNbEnd = aMFaces.Extent();
    if ((aNbEnd == aNbBegin) || (aNbEnd == 0))
    {
      break;
    }
  }
  //
  if (aMFaces.IsEmpty())
  {
    return;
  }
  //
  // use only connected faces
  ShapeList aLFConnected;
  // Boundary faces
  TopTools_MapOfShape aBoundaryFaces;
  aItF.Initialize(myShapes);
  for (; aItF.More(); aItF.Next())
  {
    const TopoShape& aF = aItF.Value();
    if (aMFaces.Contains(aF))
    {
      aLFConnected.Append(aF);
      if (!aBoundaryFaces.Add(aF))
        aBoundaryFaces.Remove(aF);
    }
  }
  //
  const Standard_Integer aNbShapes      = aLFConnected.Extent();
  Standard_Boolean       bAllFacesTaken = Standard_False;
  //
  // Build the shells
  aItF.Initialize(aLFConnected);
  for (i = 1; aItF.More() && !bAllFacesTaken; aItF.Next(), ++i)
  {
    const TopoShape& aFF = aItF.Value();
    if (!AddedFacesMap.Add(aFF))
    {
      continue;
    }
    //
    // make a new shell
    TopoShell aShell;
    aBB.MakeShell(aShell);
    aBB.Add(aShell, aFF);

    aMEFP.Clear();
    TopExp1::MapShapesAndAncestors(aShell, TopAbs_EDGE, TopAbs_FACE, aMEFP);
    //
    // loop on faces added to Shell;
    // add their neighbor faces to Shell and so on
    aItS.Initialize(aShell);
    for (; aItS.More(); aItS.Next())
    {
      const TopoFace& aF         = (*(TopoFace*)(&aItS.Value()));
      Standard_Boolean   isBoundary = aBoundaryFaces.Contains(aF);
      //
      // loop on edges of aF; find a good neighbor face of aF by aE
      aExp.Init(aF, TopAbs_EDGE);
      for (; aExp.More(); aExp.Next())
      {
        const TopoEdge& aE = (*(TopoEdge*)(&aExp.Current()));
        //
        // proceed only free edges in this shell
        if (aMEFP.Contains(aE))
        {
          const ShapeList& aLFP = aMEFP.FindFromKey(aE);
          aNbFP                            = aLFP.Extent();
          if (aNbFP > 1)
          {
            continue;
          }
        }
        // avoid processing of internal edges
        anOr = aE.Orientation();
        if (anOr == TopAbs_INTERNAL)
        {
          continue;
        }
        // avoid processing of degenerated edges
        if (BRepInspector::Degenerated(aE))
        {
          continue;
        }
        //
        // candidate faces list
        const ShapeList& aLF = aEFMap.FindFromKey(aE);
        aNbLF                           = aLF.Extent();
        if (!aNbLF)
        {
          continue;
        }
        //
        // prepare for selecting the next face
        // take only not-processed faces as a candidates
        BOPTools_ListOfCoupleOfShape aLCSOff;
        //
        Standard_Integer                   aNbWaysInside = 0;
        TopoFace                        aSelF;
        TopTools_ListIteratorOfListOfShape aItLF(aLF);
        for (; aItLF.More(); aItLF.Next())
        {
          const TopoFace& aFL = (*(TopoFace*)(&aItLF.Value()));
          if (aF.IsSame(aFL) || AddedFacesMap.Contains(aFL))
          {
            continue;
          }
          //
          // find current edge in the face
          if (!AlgoTools::GetEdgeOff(aE, aFL, aEL))
          {
            continue;
          }
          //
          if (isBoundary && !aBoundaryFaces.Contains(aFL))
          {
            ++aNbWaysInside;
            aSelF = aFL;
          }
          aCSOff.SetShape1(aEL);
          aCSOff.SetShape2(aFL);
          aLCSOff.Append(aCSOff);
        } // for (; aItLF.More(); aItLF.Next()) {
        //
        aNbOff = aLCSOff.Extent();
        if (!aNbOff)
        {
          continue;
        }
        //
        // among all the adjacent faces chose one with the minimal
        // angle to the current one
        if (!isBoundary || aNbWaysInside != 1)
        {
          if (aNbOff == 1)
          {
            aSelF = (*(TopoFace*)(&aLCSOff.First().Shape2()));
          }
          else if (aNbOff > 1)
          {
            AlgoTools::GetFaceOff(aE, aF, aLCSOff, aSelF, aContext);
          }
        }
        //
        if (!aSelF.IsNull() && AddedFacesMap.Add(aSelF))
        {
          aBB.Add(aShell, aSelF);
          TopExp1::MapShapesAndAncestors(aSelF, TopAbs_EDGE, TopAbs_FACE, aMEFP);
        }
      } // for (; aExp.More(); aExp.Next()) {
    } // for (; aItS.More(); aItS.Next()) {
    //
    // split the shell on multi-connected edges
    ShapeList aLShSp;
    RefineShell(aShell, aMEFP, aLShSp);
    //
    // collect the not closed shells for further processing
    ShapeList aLShNC;
    //
    TopTools_ListIteratorOfListOfShape aItLShSp(aLShSp);
    for (; aItLShSp.More(); aItLShSp.Next())
    {
      TopoShell& aShSp = *((TopoShell*)&aItLShSp.Value());
      //
      if (BRepInspector::IsClosed(aShSp))
      {
        aShSp.Closed(Standard_True);
        myLoops.Append(aShSp);
      }
      else
      {
        aLShNC.Append(aShSp);
      }
    }
    //
    bAllFacesTaken = (AddedFacesMap.Extent() == aNbShapes);
    if (bAllFacesTaken)
    {
      break;
    }
    //
    if (aLShSp.Extent() == 1)
    {
      // not further processing of not closed shells is needed,
      // as it will not bring any new results
      continue;
    }
    //

    // remove th faces of not closed shells from the map of processed faces
    // and try to rebuild the shells using all not processed faces,
    // because faces of one shell might be needed for building the other
    TopTools_ListIteratorOfListOfShape aItLShNC(aLShNC);
    for (; aItLShNC.More(); aItLShNC.Next())
    {
      TopoDS_Iterator aItNC(aItLShNC.Value());
      for (; aItNC.More(); aItNC.Next())
      {
        AddedFacesMap.Remove(aItNC.Value());
      }
    }
  } // for (; aItF.More(); aItF.Next()) {
}

//=================================================================================================

TopoShape FindShape(const TopoShape& theShapeToFind, const TopoShape& theShape)
{
  TopoShape    aRes;
  ShapeExplorer anExp(theShape, theShapeToFind.ShapeType());
  for (; anExp.More(); anExp.Next())
  {
    const TopoShape& aShape = anExp.Current();
    if (aShape.IsSame(theShapeToFind))
    {
      aRes = aShape;
      break;
    }
  }
  return aRes;
}

//=================================================================================================

void RefineShell(TopoShell&                                    theShell,
                 const TopTools_IndexedDataMapOfShapeListOfShape& theMEF,
                 ShapeList&                            theLShSp)
{
  TopoDS_Iterator aIt(theShell);
  if (!aIt.More())
  {
    return;
  }
  //
  // Find edges with more than 2 adjacent faces - branch edges -
  // edges on which the input shell should be split
  TopTools_MapOfShape aMEStop;
  //
  Standard_Integer i, aNbMEF = theMEF.Extent();
  for (i = 1; i <= aNbMEF; ++i)
  {
    const TopoEdge&          aE  = TopoDS::Edge(theMEF.FindKey(i));
    const ShapeList& aLF = theMEF(i);
    if (aLF.Extent() > 2)
    {
      aMEStop.Add(aE);
      continue;
    }

    if (aLF.Extent() == 2)
    {
      const TopoFace& aF1 = TopoDS::Face(aLF.First());
      const TopoFace& aF2 = TopoDS::Face(aLF.Last());

      TopoShape aE1 = FindShape(aE, aF1);
      TopoShape aE2 = FindShape(aE, aF2);

      if (aE1.Orientation() == aE2.Orientation())
      {
        aMEStop.Add(aE);
        continue;
      }
    }
    //
    // check for internal edges - count faces, in which the edge
    // is internal, twice
    Standard_Integer                   aNbF = 0;
    TopTools_ListIteratorOfListOfShape aItLF(aLF);
    for (; aItLF.More() && aNbF <= 2; aItLF.Next())
    {
      const TopoFace& aF = TopoDS::Face(aItLF.Value());
      ++aNbF;
      ShapeExplorer aExp(aF, TopAbs_EDGE);
      for (; aExp.More(); aExp.Next())
      {
        const TopoShape& aEF = aExp.Current();
        if (aEF.IsSame(aE))
        {
          if (aEF.Orientation() == TopAbs_INTERNAL)
          {
            ++aNbF;
          }
          break;
        }
      }
    }
    //
    if (aNbF > 2)
    {
      aMEStop.Add(aE);
    }
  }
  //
  if (aMEStop.IsEmpty())
  {
    theLShSp.Append(theShell);
    return;
  }
  //
  TopoBuilder                     aBB;
  ShapeExplorer                    aExp;
  TopTools_IndexedMapOfShape         aMFB;
  TopTools_MapOfOrientedShape        aMFProcessed;
  ShapeList               aLFP, aLFP1;
  TopTools_ListIteratorOfListOfShape aItLF, aItLFP;
  //
  // The first Face
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aF1 = aIt.Value();
    if (!aMFProcessed.Add(aF1))
    {
      continue;
    }
    //
    aMFB.Clear();
    aLFP.Clear();
    //
    aMFB.Add(aF1);
    aLFP.Append(aF1);
    //
    // Trying to reach the branch point
    for (;;)
    {
      aItLFP.Initialize(aLFP);
      for (; aItLFP.More(); aItLFP.Next())
      {
        const TopoShape& aFP = aItLFP.Value();
        //
        aExp.Init(aFP, TopAbs_EDGE);
        for (; aExp.More(); aExp.Next())
        {
          const TopoEdge& aE = (*(TopoEdge*)(&aExp.Current()));
          if (aMEStop.Contains(aE))
          {
            continue;
          }
          //
          if (aE.Orientation() == TopAbs_INTERNAL)
          {
            continue;
          }
          //
          if (BRepInspector::Degenerated(aE))
          {
            continue;
          }
          //
          const ShapeList& aLF = theMEF.FindFromKey(aE);
          //
          aItLF.Initialize(aLF);
          for (; aItLF.More(); aItLF.Next())
          {
            const TopoShape& aFP1 = aItLF.Value();
            if (aFP1.IsSame(aFP))
            {
              continue;
            }
            if (aMFB.Contains(aFP1))
            {
              continue;
            }
            //
            if (aMFProcessed.Add(aFP1))
            {
              aMFB.Add(aFP1);
              aLFP1.Append(aFP1);
            }
          } // for (; aItLF.More(); aItLF.Next()) {
        } // for (; aExp.More(); aExp.Next()) {
      } // for (; aItLFP.More(); aItLFP.Next()) {
      //
      //
      if (aLFP1.IsEmpty())
      {
        break;
      }
      //
      aLFP.Clear();
      aLFP.Append(aLFP1);
    } // for (;;) {
    //
    Standard_Integer aNbMFB = aMFB.Extent();
    if (aNbMFB)
    {
      TopoShell aShSp;
      aBB.MakeShell(aShSp);
      //
      for (i = 1; i <= aNbMFB; ++i)
      {
        const TopoShape& aFB = aMFB(i);
        aBB.Add(aShSp, aFB);
      }
      theLShSp.Append(aShSp);
    }
  } // for (; aIt.More(); aIt.Next()) {
}

//=================================================================================================

void BOPAlgo_ShellSplitter::MakeShells(const Message_ProgressRange& theRange)
{
  Standard_Boolean                            bIsRegular;
  Standard_Integer                            aNbVCBK, k;
  BOPTools_ListIteratorOfListOfConnexityBlock aItCB;
  TopTools_ListIteratorOfListOfShape          aIt;
  BOPAlgo_VectorOfCBK                         aVCBK;
  //
  Message_ProgressScope aPSOuter(theRange, NULL, 1);
  myShells.Clear();
  //
  aItCB.Initialize(myLCB);
  for (; aItCB.More(); aItCB.Next())
  {
    if (UserBreak(aPSOuter))
    {
      return;
    }
    BOPTools_ConnexityBlock& aCB = aItCB.ChangeValue();
    bIsRegular                   = aCB.IsRegular();
    if (bIsRegular)
    {
      TopoShell aShell;
      //
      const ShapeList& aLF = aCB.Shapes();
      MakeShell(aLF, aShell);
      aShell.Closed(Standard_True);
      myShells.Append(aShell);
    }
    else
    {
      BOPAlgo_CBK& aCBK = aVCBK.Appended();
      aCBK.SetConnexityBlock(aCB);
    }
  }
  //
  aNbVCBK = aVCBK.Length();
  Message_ProgressScope aPSParallel(aPSOuter.Next(), NULL, aNbVCBK);
  for (Standard_Integer iS = 0; iS < aNbVCBK; ++iS)
  {
    aVCBK.ChangeValue(iS).SetProgressRange(aPSParallel.Next());
  }
  //===================================================
  BooleanParallelTools::Perform(myRunParallel, aVCBK);
  //===================================================
  for (k = 0; k < aNbVCBK; ++k)
  {
    BOPAlgo_CBK&                   aCBK = aVCBK(k);
    const BOPTools_ConnexityBlock& aCB  = aCBK.ConnexityBlock();
    const ShapeList&    aLS  = aCB.Loops();
    aIt.Initialize(aLS);
    for (; aIt.More(); aIt.Next())
    {
      TopoShape& aShell = aIt.ChangeValue();
      aShell.Closed(Standard_True);
      myShells.Append(aShell);
    }
  }
}

//=================================================================================================

void MakeShell(const ShapeList& aLS, TopoShell& aShell)
{
  ShapeBuilder                       aBB;
  TopTools_ListIteratorOfListOfShape aIt;
  //
  aBB.MakeShell(aShell);
  //
  aIt.Initialize(aLS);
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aF = aIt.Value();
    aBB.Add(aShell, aF);
  }
  //
  AlgoTools::OrientFacesOnShell(aShell);
}
