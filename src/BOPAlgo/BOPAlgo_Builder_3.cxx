// Created by: Peter KURNEV
// Copyright (c) 2010-2014 OPEN CASCADE SAS
// Copyright (c) 2007-2010 CEA/DEN, EDF R&D, OPEN CASCADE
// Copyright (c) 2003-2007 OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN, CEDRAT,
//                         EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
//
#include <BOPAlgo_Builder.hxx>
//
//
#include <TopAbs_State.hxx>
//
#include <TopoDS.hxx>
#include <TopoDS_AlertWithShape.hxx>
#include <TopoDS_Shape.hxx>
//
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
//
#include <BRep_Builder.hxx>
//
#include <BOPAlgo_BuilderSolid.hxx>
//
#include <IntTools_Context.hxx>
//
#include <BOPDS_DS.hxx>
#include <BOPDS_ShapeInfo.hxx>
//
#include <BOPTools_AlgoTools.hxx>
#include <BOPTools_MapOfSet.hxx>
#include <BOPTools_Set.hxx>
#include <BOPTools_Parallel.hxx>
//
#include <BOPAlgo_Tools.hxx>
#include <NCollection_IncAllocator.hxx>
#include <NCollection_Vector.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_ListOfShape.hxx>

#include <algorithm>

static void OwnInternalShapes(const TopoShape&, TopTools_IndexedMapOfShape&);

//=================================================================================================

void BOPAlgo_Builder::FillImagesSolids(const Message_ProgressRange& theRange)
{
  Standard_Integer i = 0, aNbS = myDS->NbSourceShapes();
  for (i = 0; i < aNbS; ++i)
  {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    if (aSI.ShapeType() == TopAbs_SOLID)
    {
      break;
    }
  }
  if (i >= aNbS)
  {
    return;
  }

  Message_ProgressScope aPS(theRange, "Building splits of solids", 10);
  // Draft1 solids
  TopTools_DataMapOfShapeShape aDraftSolids;
  // Find all IN faces for all IN faces
  FillIn3DParts(aDraftSolids, aPS.Next(4));
  if (HasErrors())
  {
    return;
  }
  // Build split of the solids
  BuildSplitSolids(aDraftSolids, aPS.Next(5));
  if (HasErrors())
  {
    return;
  }
  // Fill solids with internal parts
  FillInternalShapes(aPS.Next());
}

//=================================================================================================

void BOPAlgo_Builder::FillIn3DParts(TopTools_DataMapOfShapeShape& theDraftSolids,
                                    const Message_ProgressRange&  theRange)
{
  Message_ProgressScope aPS(theRange, NULL, 2);

  Handle(NCollection_BaseAllocator) anAlloc = new NCollection_IncAllocator;

  // Find all faces that are IN solids

  // Store boxes of the shapes into a map
  TopTools_DataMapOfShapeBox aShapeBoxMap(1, anAlloc);

  // Fence map
  TopTools_MapOfShape aMFence(1, anAlloc);

  // Get all faces
  ShapeList aLFaces(anAlloc);

  Standard_Integer i, aNbS = myDS->NbSourceShapes();
  for (i = 0; i < aNbS; ++i)
  {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    if (aSI.ShapeType() != TopAbs_FACE)
      continue;

    if (UserBreak(aPS))
    {
      return;
    }

    const TopoShape&         aS    = aSI.Shape();
    const ShapeList* pLSIm = myImages.Seek(aS);

    if (pLSIm)
    {
      TopTools_ListIteratorOfListOfShape aItLSIm(*pLSIm);
      for (; aItLSIm.More(); aItLSIm.Next())
      {
        const TopoShape& aSIm = aItLSIm.Value();
        if (aMFence.Add(aSIm))
          aLFaces.Append(aSIm);
      }
    }
    else
    {
      aLFaces.Append(aS);
      aShapeBoxMap.Bind(aS, aSI.Box1());
    }
  }

  ShapeBuilder aBB;

  // Get all solids
  ShapeList aLSolids(anAlloc);
  // Keep INTERNAL faces of the solids
  TopTools_DataMapOfShapeListOfShape aSolidsIF(1, anAlloc);
  // Draft1 solids
  TopTools_IndexedDataMapOfShapeShape aDraftSolid(1, anAlloc);

  for (i = 0; i < aNbS; ++i)
  {
    BOPDS_ShapeInfo& aSI = myDS->ChangeShapeInfo(i);
    if (aSI.ShapeType() != TopAbs_SOLID)
    {
      continue;
    }
    if (UserBreak(aPS))
    {
      return;
    }
    const TopoShape& aS     = aSI.Shape();
    const TopoSolid& aSolid = (*(TopoSolid*)(&aS));
    //
    // Bounding box for the solid aS
    Box2& aBoxS = aSI.ChangeBox();
    if (aBoxS.IsVoid())
      myDS->BuildBndBoxSolid(i, aBoxS, myCheckInverted);

    // Build Draft1 Solid
    ShapeList aLIF;
    TopoSolid         aSD;
    aBB.MakeSolid(aSD);
    BuildDraftSolid(aSolid, aSD, aLIF);

    aLSolids.Append(aSD);
    aSolidsIF.Bind(aSD, aLIF);
    aShapeBoxMap.Bind(aSD, aBoxS);
    aDraftSolid.Add(aS, aSD);
  }

  // Perform classification of the faces
  TopTools_IndexedDataMapOfShapeListOfShape anInParts;

  BooleanTools::ClassifyFaces(aLFaces,
                               aLSolids,
                               myRunParallel,
                               myContext,
                               anInParts,
                               aShapeBoxMap,
                               aSolidsIF,
                               aPS.Next());

  // Analyze the results of classification
  Standard_Integer aNbSol = aDraftSolid.Extent();
  for (i = 1; i <= aNbSol; ++i)
  {
    if (UserBreak(aPS))
    {
      return;
    }
    const TopoSolid&         aSolid     = TopoDS::Solid(aDraftSolid.FindKey(i));
    const TopoSolid&         aSDraft    = TopoDS::Solid(aDraftSolid(i));
    const ShapeList& aLInFaces  = anInParts.FindFromKey(aSDraft);
    const ShapeList& aLInternal = aSolidsIF.Find(aSDraft);

    Standard_Integer aNbIN = aLInFaces.Extent();

    if (!aNbIN)
    {
      Standard_Boolean bHasImage = Standard_False;
      // Check if the shells of the solid have image
      for (TopoDS_Iterator it(aSolid); it.More() && !bHasImage; it.Next())
        bHasImage = myImages.IsBound(it.Value());

      if (!bHasImage)
        // no need to split the solid
        continue;
    }

    theDraftSolids.Bind(aSolid, aSDraft);

    Standard_Integer aNbInt = aLInternal.Extent();
    if (aNbInt || aNbIN)
    {
      // Combine the lists
      ShapeList* pLIN = myInParts.Bound(aSolid, ShapeList());

      TopTools_ListIteratorOfListOfShape aItLS(aLInFaces);
      for (; aItLS.More(); aItLS.Next())
        pLIN->Append(aItLS.Value());

      aItLS.Initialize(aLInternal);
      for (; aItLS.More(); aItLS.Next())
        pLIN->Append(aItLS.Value());
    }
  }
}

//=================================================================================================

void BOPAlgo_Builder::BuildDraftSolid(const TopoShape&   theSolid,
                                      TopoShape&         theDraftSolid,
                                      ShapeList& theLIF)
{
  Standard_Boolean                   bToReverse;
  Standard_Integer                   iFlag;
  TopAbs_Orientation                 aOrF, aOrSh, aOrSd;
  TopoDS_Iterator                    aIt1, aIt2;
  TopoShell                       aShD;
  TopoShape                       aFx;
  ShapeBuilder                       aBB;
  TopTools_ListIteratorOfListOfShape aItS;
  //
  aOrSd = theSolid.Orientation();
  theDraftSolid.Orientation(aOrSd);
  //
  aIt1.Initialize(theSolid);
  for (; aIt1.More(); aIt1.Next())
  {
    const TopoShape& aSh = aIt1.Value();
    if (aSh.ShapeType() != TopAbs_SHELL)
    {
      continue; // mb internal edges,vertices
    }
    //
    aOrSh = aSh.Orientation();
    aBB.MakeShell(aShD);
    aShD.Orientation(aOrSh);
    iFlag = 0;
    //
    aIt2.Initialize(aSh);
    for (; aIt2.More(); aIt2.Next())
    {
      const TopoShape& aF = aIt2.Value();
      aOrF                   = aF.Orientation();
      //
      if (myImages.IsBound(aF))
      {
        const ShapeList& aLSp = myImages.Find(aF);
        aItS.Initialize(aLSp);
        for (; aItS.More(); aItS.Next())
        {
          aFx = aItS.Value();
          //
          if (myShapesSD.IsBound(aFx))
          {
            //
            if (aOrF == TopAbs_INTERNAL)
            {
              aFx.Orientation(aOrF);
              theLIF.Append(aFx);
            }
            else
            {
              bToReverse =
                AlgoTools::IsSplitToReverseWithWarn(aFx, aF, myContext, myReport);
              if (bToReverse)
              {
                aFx.Reverse();
              }
              //
              iFlag = 1;
              aBB.Add(aShD, aFx);
            }
          } // if (myShapesSD.IsBound(aFx)) {
          else
          {
            aFx.Orientation(aOrF);
            if (aOrF == TopAbs_INTERNAL)
            {
              theLIF.Append(aFx);
            }
            else
            {
              iFlag = 1;
              aBB.Add(aShD, aFx);
            }
          }
        }
      } // if (myImages.IsBound(aF)) {
      //
      else
      {
        if (aOrF == TopAbs_INTERNAL)
        {
          theLIF.Append(aF);
        }
        else
        {
          iFlag = 1;
          aBB.Add(aShD, aF);
        }
      }
    } // for (; aIt2.More(); aIt2.Next()) {
    //
    if (iFlag)
    {
      aShD.Closed(BRepInspector::IsClosed(aShD));
      aBB.Add(theDraftSolid, aShD);
    }
  } // for (; aIt1.More(); aIt1.Next()) {
}

//=======================================================================

//=======================================================================
// class : BOPAlgo_SplitSolid
// purpose  : Auxiliary class to extend the BOPAlgo_BuilderSolid with the solid to split
//=======================================================================
class BOPAlgo_SplitSolid : public BOPAlgo_BuilderSolid
{
public:
  //! Sets the solid
  void SetSolid(const TopoSolid& theSolid) { mySolid = theSolid; }

  //! Returns the solid
  const TopoSolid& Solid() const { return mySolid; }

  //! Sets progress range
  void SetProgressRange(const Message_ProgressRange& theRange) { myRange = theRange; }

  // New perform method, using own progress range
  void Perform()
  {
    Message_ProgressScope aPS(myRange, NULL, 1);
    if (!aPS.More())
    {
      return;
    }
    BOPAlgo_BuilderSolid::Perform(aPS.Next());
  }

private:
  //! Disable the range enabled method
  virtual void Perform(const Message_ProgressRange& /* theRange*/) {}

private:
  TopoSolid          mySolid; //!< Solid to split
  Message_ProgressRange myRange;
};

// Vector of Solid Builders
typedef NCollection_Vector<BOPAlgo_SplitSolid> BOPAlgo_VectorOfBuilderSolid;

//=================================================================================================

void BOPAlgo_Builder::BuildSplitSolids(TopTools_DataMapOfShapeShape& theDraftSolids,
                                       const Message_ProgressRange&  theRange)
{
  Standard_Boolean                   bFlagSD;
  Standard_Integer                   i, aNbS;
  ShapeExplorer                    aExp;
  TopTools_ListIteratorOfListOfShape aIt;
  //
  Handle(NCollection_BaseAllocator) aAlr0;
  aAlr0 = NCollection_BaseAllocator::CommonBaseAllocator();
  //
  ShapeList         aSFS(aAlr0), aLSEmpty(aAlr0);
  TopTools_MapOfShape          aMFence(100, aAlr0);
  BOPTools_MapOfSet            aMST(100, aAlr0);
  BOPAlgo_VectorOfBuilderSolid aVBS;
  //
  Message_ProgressScope aPSOuter(theRange, NULL, 10);
  // 0. Find same domain solids for non-interfered solids
  aNbS = myDS->NbSourceShapes();
  for (i = 0; i < aNbS; ++i)
  {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    //
    if (aSI.ShapeType() != TopAbs_SOLID)
    {
      continue;
    }
    if (UserBreak(aPSOuter))
    {
      return;
    }
    //
    const TopoShape& aS = aSI.Shape();
    if (!aMFence.Add(aS))
    {
      continue;
    }
    if (theDraftSolids.IsBound(aS))
    {
      continue;
    }
    //
    BOPTools_Set aST;
    //
    aST.Add(aS, TopAbs_FACE);
    aMST.Add(aST);
    //
  } // for (i=1; i<=aNbS; ++i)
  //
  // Build temporary map of solids images to avoid rebuilding
  // of the solids without internal faces
  TopTools_IndexedDataMapOfShapeListOfShape aSolidsIm;
  // 1. Build solids for interfered source solids
  for (i = 0; i < aNbS; ++i)
  {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    if (aSI.ShapeType() != TopAbs_SOLID)
      continue;

    const TopoShape& aS     = aSI.Shape();
    const TopoSolid& aSolid = (*(TopoSolid*)(&aS));
    if (!theDraftSolids.IsBound(aS))
      continue;

    const TopoShape&         aSD   = theDraftSolids.Find(aS);
    const ShapeList* pLFIN = myInParts.Seek(aS);
    if (!pLFIN || pLFIN->IsEmpty())
    {
      aSolidsIm(aSolidsIm.Add(aS, ShapeList())).Append(aSD);
      continue;
    }

    aSFS.Clear();
    //
    // 1.1 Fill Shell Faces Set
    aExp.Init(aSD, TopAbs_FACE);
    for (; aExp.More(); aExp.Next())
    {
      const TopoShape& aF = aExp.Current();
      aSFS.Append(aF);
    }
    //
    // 1.2 Fill internal faces
    aIt.Initialize(*pLFIN);
    for (; aIt.More(); aIt.Next())
    {
      TopoShape aF = aIt.Value();
      //
      aF.Orientation(TopAbs_FORWARD);
      aSFS.Append(aF);
      aF.Orientation(TopAbs_REVERSED);
      aSFS.Append(aF);
    }
    //
    // 1.3 Build new solids
    BOPAlgo_SplitSolid& aBS = aVBS.Appended();
    aBS.SetSolid(aSolid);
    aBS.SetShapes(aSFS);
    aBS.SetRunParallel(myRunParallel);
  } // for (i=0; i<aNbS; ++i) {
  //
  Standard_Integer k, aNbBS;
  //
  aNbBS = aVBS.Length();
  // Set progress range for each task to be run in parallel
  Message_ProgressScope aPSParallel(aPSOuter.Next(9), "Splitting solids", aNbBS);
  for (Standard_Integer iS = 0; iS < aNbBS; iS++)
  {
    BOPAlgo_SplitSolid& aSplitSolid = aVBS.ChangeValue(iS);
    aSplitSolid.SetProgressRange(aPSParallel.Next());
  }
  //
  //===================================================
  BooleanParallelTools::Perform(myRunParallel, aVBS);
  //===================================================
  if (UserBreak(aPSOuter))
  {
    return;
  }
  //
  for (k = 0; k < aNbBS; ++k)
  {
    BOPAlgo_SplitSolid& aBS = aVBS(k);
    aSolidsIm.Add(aBS.Solid(), aBS.Areas());

    // Merge BuilderSolid's report into main report,
    // assigning the solid with the warnings/errors which
    // have been generated for it.
    // Convert all errors of BuilderSolid into warnings for main report.
    const Handle(Message_Report)& aBSReport       = aBS.GetReport();
    Message_Gravity               anAlertTypes[2] = {Message_Warning, Message_Fail};
    for (Standard_Integer iGravity = 0; iGravity < 2; iGravity++)
    {
      const Message_ListOfAlert& anLAlerts = aBSReport->GetAlerts(anAlertTypes[iGravity]);
      for (Message_ListOfAlert::Iterator itA(anLAlerts); itA.More(); itA.Next())
      {
        Handle(Message_Alert) anAlert = itA.Value();

        Handle(TopoDS_AlertWithShape) anAlertWithShape =
          Handle(TopoDS_AlertWithShape)::DownCast(itA.Value());
        if (!anAlertWithShape.IsNull())
        {
          TopoShape aWarnShape;
          ShapeBuilder().MakeCompound(TopoDS::Compound(aWarnShape));
          ShapeBuilder().Add(aWarnShape, aBS.Solid());
          ShapeBuilder().Add(aWarnShape, anAlertWithShape->GetShape());

          anAlertWithShape->SetShape(aWarnShape);
          AddWarning(anAlertWithShape);
        }
        else
          AddWarning(anAlert);
      }
    }
  }
  //
  // Add new solids to images map
  aNbBS = aSolidsIm.Extent();
  for (k = 1; k <= aNbBS; ++k)
  {
    const TopoShape&         aS   = aSolidsIm.FindKey(k);
    const ShapeList& aLSR = aSolidsIm(k);
    //
    if (!myImages.IsBound(aS))
    {
      ShapeList* pLSx = myImages.Bound(aS, ShapeList());
      //
      aIt.Initialize(aLSR);
      for (; aIt.More(); aIt.Next())
      {
        BOPTools_Set aST;
        //
        const TopoShape& aSR = aIt.Value();
        aST.Add(aSR, TopAbs_FACE);
        //
        bFlagSD = aMST.Contains(aST);
        //
        const BOPTools_Set& aSTx = aMST.Added(aST);
        const TopoShape& aSx  = aSTx.Shape();
        pLSx->Append(aSx);
        //
        ShapeList* pLOr = myOrigins.ChangeSeek(aSx);
        if (!pLOr)
        {
          pLOr = myOrigins.Bound(aSx, ShapeList());
        }
        pLOr->Append(aS);
        //
        if (bFlagSD)
        {
          myShapesSD.Bind(aSR, aSx);
        }
      }
    }
  }
}

//=================================================================================================

void BOPAlgo_Builder::FillInternalShapes(const Message_ProgressRange& theRange)
{
  Standard_Integer                   i, j, aNbS, aNbSI, aNbSx;
  TopAbs_ShapeEnum                   aType;
  TopAbs_State                       aState;
  TopoDS_Iterator                    aItS;
  ShapeBuilder                       aBB;
  TopTools_ListIteratorOfListOfShape aIt, aIt1;
  //
  Handle(NCollection_BaseAllocator) aAllocator;
  //-----------------------------------------------------scope f
  aAllocator = NCollection_BaseAllocator::CommonBaseAllocator();
  //
  TopTools_IndexedDataMapOfShapeListOfShape aMSx(100, aAllocator);
  TopTools_IndexedMapOfShape                aMx(100, aAllocator);
  TopTools_IndexedMapOfShape                aMSI(100, aAllocator);
  TopTools_MapOfShape                       aMFence(100, aAllocator);
  TopTools_MapOfShape                       aMSOr(100, aAllocator);
  ShapeList                      aLSd(aAllocator);
  ShapeList                      aLArgs(aAllocator);
  ShapeList                      aLSC(aAllocator);
  ShapeList                      aLSI(aAllocator);

  Message_ProgressScope aPS(theRange, NULL, 10);
  //
  // 1. Shapes to process
  //
  // 1.1 Shapes from pure arguments aMSI
  // 1.1.1 vertex, edge, wire
  //
  const ShapeList& aArguments = myDS->Arguments();
  aIt.Initialize(aArguments);
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aS = aIt.Value();
    AlgoTools::TreatCompound(aS, aLSC, &aMFence);
  }
  aIt.Initialize(aLSC);
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aS = aIt.Value();
    aType                  = aS.ShapeType();
    if (aType == TopAbs_WIRE)
    {
      aItS.Initialize(aS);
      for (; aItS.More(); aItS.Next())
      {
        const TopoShape& aE = aItS.Value();
        if (aMFence.Add(aE))
        {
          aLArgs.Append(aE);
        }
      }
    }
    else if (aType == TopAbs_VERTEX || aType == TopAbs_EDGE)
    {
      aLArgs.Append(aS);
    }
  }
  aMFence.Clear();
  //
  aIt.Initialize(aLArgs);
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aS = aIt.Value();
    aType                  = aS.ShapeType();
    if (aType == TopAbs_VERTEX || aType == TopAbs_EDGE || aType == TopAbs_WIRE)
    {
      if (aMFence.Add(aS))
      {
        if (myImages.IsBound(aS))
        {
          const ShapeList& aLSp = myImages.Find(aS);
          aIt1.Initialize(aLSp);
          for (; aIt1.More(); aIt1.Next())
          {
            const TopoShape& aSp = aIt1.Value();
            aMSI.Add(aSp);
          }
        }
        else
        {
          aMSI.Add(aS);
        }
      }
    }
  }
  if (UserBreak(aPS))
  {
    return;
  }

  aNbSI = aMSI.Extent();
  //
  // 2. Internal vertices, edges from source solids
  aMFence.Clear();
  aLSd.Clear();
  //
  aNbS = myDS->NbSourceShapes();
  for (i = 0; i < aNbS; ++i)
  {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    //
    if (aSI.ShapeType() != TopAbs_SOLID)
    {
      continue;
    }
    if (UserBreak(aPS))
    {
      return;
    }
    //
    const TopoShape& aS = aSI.Shape();
    //
    aMx.Clear();
    OwnInternalShapes(aS, aMx);
    //
    aNbSx = aMx.Extent();
    for (j = 1; j <= aNbSx; ++j)
    {
      const TopoShape& aSi = aMx(j);
      if (myImages.IsBound(aSi))
      {
        const ShapeList& aLSp = myImages.Find(aSi);
        aIt1.Initialize(aLSp);
        for (; aIt1.More(); aIt1.Next())
        {
          const TopoShape& aSp = aIt1.Value();
          aMSI.Add(aSp);
        }
      }
      else
      {
        aMSI.Add(aSi);
      }
    }
    //
    // build aux map from splits of solids
    if (myImages.IsBound(aS))
    {
      const ShapeList& aLSp = myImages.Find(aS);
      aIt.Initialize(aLSp);
      for (; aIt.More(); aIt.Next())
      {
        const TopoShape& aSp = aIt.Value();
        if (aMFence.Add(aSp))
        {
          TopExp1::MapShapesAndAncestors(aSp, TopAbs_VERTEX, TopAbs_EDGE, aMSx);
          TopExp1::MapShapesAndAncestors(aSp, TopAbs_VERTEX, TopAbs_FACE, aMSx);
          TopExp1::MapShapesAndAncestors(aSp, TopAbs_EDGE, TopAbs_FACE, aMSx);
          aLSd.Append(aSp);
        }
      }
    }
    else
    {
      if (aMFence.Add(aS))
      {
        TopExp1::MapShapesAndAncestors(aS, TopAbs_VERTEX, TopAbs_EDGE, aMSx);
        TopExp1::MapShapesAndAncestors(aS, TopAbs_VERTEX, TopAbs_FACE, aMSx);
        TopExp1::MapShapesAndAncestors(aS, TopAbs_EDGE, TopAbs_FACE, aMSx);
        aLSd.Append(aS);
        aMSOr.Add(aS);
      }
    }
  } // for (i=0; i<aNbS; ++i) {
  //
  // 3. Some shapes of aMSI can be already tied with faces of
  //    split solids
  aNbSI = aMSI.Extent();
  for (i = 1; i <= aNbSI; ++i)
  {
    const TopoShape& aSI = aMSI(i);
    if (aMSx.Contains(aSI))
    {
      const ShapeList& aLSx = aMSx.FindFromKey(aSI);
      aNbSx                            = aLSx.Extent();
      if (!aNbSx)
      {
        aLSI.Append(aSI);
      }
    }
    else
    {
      aLSI.Append(aSI);
    }
  }
  //
  // 4. Just check it
  aNbSI = aLSI.Extent();
  if (!aNbSI)
  {
    return;
  }

  aPS.Next();
  //
  // 5 Settle internal vertices and edges into solids
  aMx.Clear();

  Message_ProgressScope aPSLoop(aPS.Next(9), "Looking for internal shapes", aLSd.Size());

  aIt.Initialize(aLSd);
  for (; aIt.More(); aIt.Next(), aPSLoop.Next())
  {
    TopoSolid aSd = TopoDS::Solid(aIt.Value());
    //
    aIt1.Initialize(aLSI);
    for (; aIt1.More();)
    {
      TopoShape aSI = aIt1.Value();
      aSI.Orientation(TopAbs_INTERNAL);
      //
      aState = AlgoTools::ComputeStateByOnePoint(aSI, aSd, 1.e-11, myContext);
      //
      if (aState != TopAbs_IN)
      {
        aIt1.Next();
        continue;
      }
      //
      if (aMSOr.Contains(aSd))
      {
        // make new solid
        TopoSolid aSdx;
        //
        aBB.MakeSolid(aSdx);
        aItS.Initialize(aSd);
        for (; aItS.More(); aItS.Next())
        {
          const TopoShape& aSh = aItS.Value();
          aBB.Add(aSdx, aSh);
        }
        //
        aBB.Add(aSdx, aSI);
        //
        // no need to check for images of aSd as aMSOr contains only original solids
        ShapeList* pLS = myImages.Bound(aSd, ShapeList());
        pLS->Append(aSdx);
        //
        ShapeList* pLOr = myOrigins.Bound(aSdx, ShapeList());
        pLOr->Append(aSd);
        //
        aMSOr.Remove(aSd);
        aSd = aSdx;
      }
      else
      {
        aBB.Add(aSd, aSI);
      }
      //
      aLSI.Remove(aIt1);
    } // for (; aIt1.More();) {
  } // for (; aIt.More(); aIt.Next()) {
  //
  //-----------------------------------------------------scope t
  aLArgs.Clear();
  aLSd.Clear();
  aMSOr.Clear();
  aMFence.Clear();
  aMSI.Clear();
  aMx.Clear();
  aMSx.Clear();
}

//=================================================================================================

void OwnInternalShapes(const TopoShape& theS, TopTools_IndexedMapOfShape& theMx)
{
  TopoDS_Iterator aIt;
  //
  aIt.Initialize(theS);
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aSx = aIt.Value();
    if (aSx.ShapeType() != TopAbs_SHELL)
    {
      theMx.Add(aSx);
    }
  }
}
