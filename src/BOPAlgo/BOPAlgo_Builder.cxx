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

#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_Alerts.hxx>
#include <BOPAlgo_BuilderSolid.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BOPDS_DS.hxx>
#include <BOPDS_ShapeInfo.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BRep_Builder.hxx>
#include <IntTools_Context.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Solid.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfOrientedShape.hxx>

//=================================================================================================

BOPAlgo_Builder::BOPAlgo_Builder()
    : BOPAlgo_BuilderShape(),
      myArguments(myAllocator),
      myMapFence(100, myAllocator),
      myPaveFiller(NULL),
      myDS(NULL),
      myEntryPoint(0),
      myImages(100, myAllocator),
      myShapesSD(100, myAllocator),
      myOrigins(100, myAllocator),
      myInParts(100, myAllocator),
      myNonDestructive(Standard_False),
      myGlue(BOPAlgo_GlueOff),
      myCheckInverted(Standard_True)
{
}

//=================================================================================================

BOPAlgo_Builder::BOPAlgo_Builder(const Handle(NCollection_BaseAllocator)& theAllocator)
    : BOPAlgo_BuilderShape(theAllocator),
      myArguments(myAllocator),
      myMapFence(100, myAllocator),
      myPaveFiller(NULL),
      myDS(NULL),
      myEntryPoint(0),
      myImages(100, myAllocator),
      myShapesSD(100, myAllocator),
      myOrigins(100, myAllocator),
      myInParts(100, myAllocator),
      myNonDestructive(Standard_False),
      myGlue(BOPAlgo_GlueOff),
      myCheckInverted(Standard_True)
{
}

//=================================================================================================

BOPAlgo_Builder::~BOPAlgo_Builder()
{
  if (myEntryPoint == 1)
  {
    if (myPaveFiller)
    {
      delete myPaveFiller;
      myPaveFiller = NULL;
    }
  }
}

//=================================================================================================

void BOPAlgo_Builder::Clear()
{
  BOPAlgo_BuilderShape::Clear();
  myArguments.Clear();
  myMapFence.Clear();
  myImages.Clear();
  myShapesSD.Clear();
  myOrigins.Clear();
  myInParts.Clear();
}

//=================================================================================================

void BOPAlgo_Builder::AddArgument(const TopoShape& theShape)
{
  if (myMapFence.Add(theShape))
  {
    myArguments.Append(theShape);
  }
}

//=================================================================================================

void BOPAlgo_Builder::SetArguments(const ShapeList& theShapes)
{
  TopTools_ListIteratorOfListOfShape aIt;
  //
  myArguments.Clear();
  //
  aIt.Initialize(theShapes);
  for (; aIt.More(); aIt.Next())
  {
    const TopoShape& aS = aIt.Value();
    AddArgument(aS);
  }
}

//=================================================================================================

void BOPAlgo_Builder::CheckData()
{
  Standard_Integer aNb = myArguments.Extent();
  if (aNb < 2)
  {
    AddError(new BOPAlgo_AlertTooFewArguments); // too few arguments to process
    return;
  }
  //
  CheckFiller();
}

//=================================================================================================

void BOPAlgo_Builder::CheckFiller()
{
  if (!myPaveFiller)
  {
    AddError(new BOPAlgo_AlertNoFiller);
    return;
  }
  GetReport()->Merge(myPaveFiller->GetReport());
}

//=================================================================================================

void BOPAlgo_Builder::Prepare()
{
  ShapeBuilder    aBB;
  TopoCompound aC;
  //
  // 1. myShape is empty compound
  aBB.MakeCompound(aC);
  myShape = aC;
}

//=================================================================================================

void BOPAlgo_Builder::Perform(const Message_ProgressRange& theRange)
{
  GetReport()->Clear();
  //
  if (myEntryPoint == 1)
  {
    if (myPaveFiller)
    {
      delete myPaveFiller;
      myPaveFiller = NULL;
    }
  }
  //
  Handle(NCollection_BaseAllocator) aAllocator = NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BooleanPaveFiller* pPF = new BooleanPaveFiller(aAllocator);
  //
  pPF->SetArguments(myArguments);
  pPF->SetRunParallel(myRunParallel);
  Message_ProgressScope aPS(theRange, "Performing General Fuse operation", 10);
  pPF->SetFuzzyValue(myFuzzyValue);
  pPF->SetNonDestructive(myNonDestructive);
  pPF->SetGlue(myGlue);
  pPF->SetUseOBB(myUseOBB);
  //
  pPF->Perform(aPS.Next(9));
  //
  myEntryPoint = 1;
  PerformInternal(*pPF, aPS.Next(1));
}

//=================================================================================================

void BOPAlgo_Builder::PerformWithFiller(const BooleanPaveFiller&    theFiller,
                                        const Message_ProgressRange& theRange)
{
  GetReport()->Clear();
  myEntryPoint     = 0;
  myNonDestructive = theFiller.NonDestructive();
  myFuzzyValue     = theFiller.FuzzyValue();
  myGlue           = theFiller.Glue();
  myUseOBB         = theFiller.UseOBB();
  PerformInternal(theFiller, theRange);
}

//=================================================================================================

void BOPAlgo_Builder::PerformInternal(const BooleanPaveFiller&    theFiller,
                                      const Message_ProgressRange& theRange)
{
  GetReport()->Clear();
  //
  try
  {
    OCC_CATCH_SIGNALS
    PerformInternal1(theFiller, theRange);
  }
  //
  catch (ExceptionBase const&)
  {
    AddError(new BOPAlgo_AlertBuilderFailed);
  }
}

//=================================================================================================

BOPAlgo_Builder::NbShapes1 BOPAlgo_Builder::getNbShapes() const
{
  NbShapes1 aCounter;
  aCounter.NbVertices() = myDS->ShapesSD().Size();
  for (Standard_Integer i = 0; i < myDS->NbSourceShapes(); ++i)
  {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    switch (aSI.ShapeType())
    {
      case TopAbs_EDGE: {
        if (myDS->HasPaveBlocks(i))
        {
          aCounter.NbEdges()++;
        }
        break;
      }
      case TopAbs_WIRE:
        aCounter.NbWires()++;
        break;
      case TopAbs_FACE: {
        if (myDS->HasFaceInfo(i))
        {
          aCounter.NbFaces()++;
        }
        break;
      }
      case TopAbs_SHELL:
        aCounter.NbShells()++;
        break;
      case TopAbs_SOLID:
        aCounter.NbSolids()++;
        break;
      case TopAbs_COMPSOLID:
        aCounter.NbCompsolids()++;
        break;
      case TopAbs_COMPOUND:
        aCounter.NbCompounds()++;
        break;
      default:
        break;
    }
  }
  return aCounter;
}

//=================================================================================================

void BOPAlgo_Builder::fillPIConstants(const Standard_Real theWhole, PISteps& theSteps) const
{
  // Fill in the constants:
  if (myFillHistory)
  {
    // for FillHistroty, which takes about 5% of the whole operation
    theSteps.SetStep(PIOperation_FillHistory, 0.05 * theWhole);
  }

  // and for PostTreat, which takes about 3% of the whole operation
  theSteps.SetStep(PIOperation_PostTreat, 0.03 * theWhole);
}

//=================================================================================================

void BOPAlgo_Builder::fillPISteps(PISteps& theSteps) const
{
  // Compute the rest of the operations - all depend on the number of sub-shapes of certain type
  NbShapes1 aNbShapes = getNbShapes();

  theSteps.SetStep(PIOperation_TreatVertices, aNbShapes.NbVertices());
  theSteps.SetStep(PIOperation_TreatEdges, aNbShapes.NbEdges());
  theSteps.SetStep(PIOperation_TreatWires, aNbShapes.NbWires());
  theSteps.SetStep(PIOperation_TreatFaces, 20 * aNbShapes.NbFaces());
  theSteps.SetStep(PIOperation_TreatShells, aNbShapes.NbShells());
  theSteps.SetStep(PIOperation_TreatSolids, 50 * aNbShapes.NbSolids());
  theSteps.SetStep(PIOperation_TreatCompsolids, aNbShapes.NbCompsolids());
  theSteps.SetStep(PIOperation_TreatCompounds, aNbShapes.NbCompounds());
}

//=================================================================================================

void BOPAlgo_Builder::PerformInternal1(const BooleanPaveFiller&    theFiller,
                                       const Message_ProgressRange& theRange)
{
  myPaveFiller     = (BooleanPaveFiller*)&theFiller;
  myDS             = myPaveFiller->PDS();
  myContext        = myPaveFiller->Context();
  myFuzzyValue     = myPaveFiller->FuzzyValue();
  myNonDestructive = myPaveFiller->NonDestructive();
  //
  Message_ProgressScope aPS(theRange, "Building the result of General Fuse operation", 100);
  // 1. CheckData
  CheckData();
  if (HasErrors())
  {
    return;
  }
  //
  // 2. Prepare
  Prepare();
  if (HasErrors())
  {
    return;
  }
  //
  PISteps aSteps(PIOperation_Last);
  analyzeProgress(100., aSteps);
  // 3. Fill Images
  // 3.1 Vertice
  FillImagesVertices(aPS.Next(aSteps.GetStep(PIOperation_TreatVertices)));
  if (HasErrors())
  {
    return;
  }
  //
  BuildResult(TopAbs_VERTEX);
  if (HasErrors())
  {
    return;
  }
  // 3.2 Edges
  FillImagesEdges(aPS.Next(aSteps.GetStep(PIOperation_TreatEdges)));
  if (HasErrors())
  {
    return;
  }
  //
  BuildResult(TopAbs_EDGE);
  if (HasErrors())
  {
    return;
  }
  //
  // 3.3 Wires
  FillImagesContainers(TopAbs_WIRE, aPS.Next(aSteps.GetStep(PIOperation_TreatWires)));
  if (HasErrors())
  {
    return;
  }
  //
  BuildResult(TopAbs_WIRE);
  if (HasErrors())
  {
    return;
  }

  // 3.4 Faces
  FillImagesFaces(aPS.Next(aSteps.GetStep(PIOperation_TreatFaces)));
  if (HasErrors())
  {
    return;
  }
  //
  BuildResult(TopAbs_FACE);
  if (HasErrors())
  {
    return;
  }
  // 3.5 Shells
  FillImagesContainers(TopAbs_SHELL, aPS.Next(aSteps.GetStep(PIOperation_TreatShells)));
  if (HasErrors())
  {
    return;
  }

  BuildResult(TopAbs_SHELL);
  if (HasErrors())
  {
    return;
  }
  // 3.6 Solids
  FillImagesSolids(aPS.Next(aSteps.GetStep(PIOperation_TreatSolids)));
  if (HasErrors())
  {
    return;
  }

  BuildResult(TopAbs_SOLID);
  if (HasErrors())
  {
    return;
  }
  // 3.7 CompSolids
  FillImagesContainers(TopAbs_COMPSOLID, aPS.Next(aSteps.GetStep(PIOperation_TreatCompsolids)));
  if (HasErrors())
  {
    return;
  }

  BuildResult(TopAbs_COMPSOLID);
  if (HasErrors())
  {
    return;
  }

  // 3.8 Compounds
  FillImagesCompounds(aPS.Next(aSteps.GetStep(PIOperation_TreatCompounds)));
  if (HasErrors())
  {
    return;
  }

  BuildResult(TopAbs_COMPOUND);
  if (HasErrors())
  {
    return;
  }
  //
  // 4 History
  PrepareHistory(aPS.Next(aSteps.GetStep(PIOperation_FillHistory)));
  if (HasErrors())
  {
    return;
  }
  //
  // 5 Post-treatment
  PostTreat(aPS.Next(aSteps.GetStep(PIOperation_PostTreat)));
}

//=================================================================================================

void BOPAlgo_Builder::PostTreat(const Message_ProgressRange& theRange)
{
  Standard_Integer           i, aNbS;
  TopAbs_ShapeEnum           aType;
  TopTools_IndexedMapOfShape aMA;
  if (myPaveFiller->NonDestructive())
  {
    // MapToAvoid
    aNbS = myDS->NbSourceShapes();
    for (i = 0; i < aNbS; ++i)
    {
      const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
      aType                      = aSI.ShapeType();
      if (aType == TopAbs_VERTEX || aType == TopAbs_EDGE || aType == TopAbs_FACE)
      {
        const TopoShape& aS = aSI.Shape();
        aMA.Add(aS);
      }
    }
  }
  //
  Message_ProgressScope aPS(theRange, "Post treatment of result shape", 2);
  AlgoTools::CorrectTolerances(myShape, aMA, 0.05, myRunParallel);
  aPS.Next();
  AlgoTools::CorrectShapeTolerances(myShape, aMA, myRunParallel);
}

//=================================================================================================

void BOPAlgo_Builder::BuildBOP(const ShapeList&  theObjects,
                               const TopAbs_State           theObjState,
                               const ShapeList&  theTools,
                               const TopAbs_State           theToolsState,
                               const Message_ProgressRange& theRange,
                               Handle(Message_Report)       theReport)
{
  if (HasErrors())
    return;

  // Report for the method
  Handle(Message_Report) aReport = theReport.IsNull() ? myReport : theReport;

  if (myArguments.IsEmpty() || myShape.IsNull())
  {
    aReport->AddAlert(Message_Fail, new BOPAlgo_AlertBuilderFailed());
    return;
  }
  // Check the input data
  if ((theObjState != TopAbs_IN && theObjState != TopAbs_OUT)
      || (theToolsState != TopAbs_IN && theToolsState != TopAbs_OUT))
  {
    aReport->AddAlert(Message_Fail, new BOPAlgo_AlertBOPNotSet());
    return;
  }

  // Check input shapes
  Standard_Boolean hasObjects = !theObjects.IsEmpty();
  Standard_Boolean hasTools   = !theTools.IsEmpty();
  if (!hasObjects && !hasTools)
  {
    aReport->AddAlert(Message_Fail, new BOPAlgo_AlertTooFewArguments());
    return;
  }

  // Check that all input solids are from the arguments
  for (Standard_Integer i = 0; i < 2; ++i)
  {
    const ShapeList&    aList = !i ? theObjects : theTools;
    ShapeList::Iterator itLS(aList);
    for (; itLS.More(); itLS.Next())
    {
      const TopoShape& aS = itLS.Value();
      // Check if the shape belongs to the arguments of operation
      if (myDS->Index(aS) < 0)
      {
        aReport->AddAlert(Message_Fail, new BOPAlgo_AlertUnknownShape(aS));
        return;
      }

      // Check if the shape is a solid or collection of them
      if (aS.ShapeType() != TopAbs_SOLID)
      {
        ShapeList aLS;
        TopTools_MapOfShape  aMFence;
        AlgoTools::TreatCompound(aS, aLS, &aMFence);

        ShapeList::Iterator it(aLS);
        for (; it.More(); it.Next())
        {
          const TopoShape& aSx = it.Value();
          if (aSx.ShapeType() != TopAbs_SOLID && aSx.ShapeType() != TopAbs_COMPSOLID)
          {
            aReport->AddAlert(Message_Fail, new BOPAlgo_AlertUnsupportedType(aS));
            return;
          }
        }
      }
    }
  }

  // Classification of the faces relatively solids has been made
  // on the stage of Solids splitting. All results are saved into
  // myInParts map, which connects the solids with its IN faces from
  // other arguments. All faces not contained in the list of IN faces
  // will be considered as OUT.

  // Prepare the maps of splits of solids faces with orientations
  TopTools_IndexedMapOfOrientedShape aMObjFacesOri, aMToolFacesOri;
  // Prepare the maps of splits of solids faces
  TopTools_IndexedMapOfShape aMObjFaces, aMToolFaces;
  // Copy the list of IN faces of the solids into map
  TopTools_MapOfShape anINObjects, anINTools;

  for (Standard_Integer i = 0; i < 2; ++i)
  {
    const ShapeList&         aList   = !i ? theObjects : theTools;
    TopTools_IndexedMapOfOrientedShape& aMapOri = !i ? aMObjFacesOri : aMToolFacesOri;
    TopTools_IndexedMapOfShape&         aMap    = !i ? aMObjFaces : aMToolFaces;
    ShapeList::Iterator      itLS(aList);
    for (; itLS.More(); itLS.Next())
    {
      const TopoShape& aShape = itLS.Value();
      ShapeExplorer     expS(aShape, TopAbs_SOLID);
      for (; expS.More(); expS.Next())
      {
        const TopoShape& aS = expS.Current();
        ShapeExplorer     expF(aS, TopAbs_FACE);
        for (; expF.More(); expF.Next())
        {
          const TopoShape& aF = expF.Current();
          if (aF.Orientation() != TopAbs_FORWARD && aF.Orientation() != TopAbs_REVERSED)
            continue;
          const ShapeList* pLFIm = myImages.Seek(aF);
          if (pLFIm)
          {
            ShapeList::Iterator itLFIm(*pLFIm);
            for (; itLFIm.More(); itLFIm.Next())
            {
              TopoFace aFIm = TopoDS::Face(itLFIm.Value());
              if (AlgoTools::IsSplitToReverse(aFIm, aF, myContext))
                aFIm.Reverse();
              aMapOri.Add(aFIm);
              aMap.Add(aFIm);
            }
          }
          else
          {
            aMapOri.Add(aF);
            aMap.Add(aF);
          }
        }

        // Copy the list of IN faces into a map
        const ShapeList* pLFIN = myInParts.Seek(aS);
        if (pLFIN)
        {
          TopTools_MapOfShape&           anINMap = !i ? anINObjects : anINTools;
          ShapeList::Iterator itLFIn(*pLFIN);
          for (; itLFIn.More(); itLFIn.Next())
            anINMap.Add(itLFIn.Value());
        }
      }
    }
  }

  // Now we need to select all faces which will participate in
  // building of the resulting solids. The final set of faces
  // depends on the given states for the groups.
  Standard_Boolean isObjectsIN = (theObjState == TopAbs_IN),
                   isToolsIN   = (theToolsState == TopAbs_IN);

  // Shortcuts
  Standard_Boolean bAvoidIN = (!isObjectsIN && !isToolsIN), // avoid all in faces
    bAvoidINforBoth         = (isObjectsIN != isToolsIN);   // avoid faces IN for both groups

  // Choose which SD faces are needed to be taken - equally or differently oriented faces
  Standard_Boolean isSameOriNeeded = (theObjState == theToolsState);
  // Resulting faces
  TopTools_IndexedMapOfOrientedShape aMResFacesOri;
  TopTools_MapOfShape                aMResFacesFence;
  // Fence map
  TopTools_MapOfShape aMFence, aMFToAvoid;
  // Oriented fence map
  TopTools_MapOfOrientedShape aMFenceOri;

  for (Standard_Integer i = 0; i < 2; ++i)
  {
    const TopTools_IndexedMapOfOrientedShape& aMap            = !i ? aMObjFacesOri : aMToolFacesOri;
    const TopTools_IndexedMapOfShape&         anOppositeMap   = !i ? aMToolFaces : aMObjFaces;
    const TopTools_MapOfShape&                anINMap         = !i ? anINObjects : anINTools;
    const TopTools_MapOfShape&                anOppositeINMap = !i ? anINTools : anINObjects;
    const Standard_Boolean                    bTakeIN         = !i ? isObjectsIN : isToolsIN;

    const Standard_Integer aNbF = aMap.Extent();
    for (Standard_Integer j = 1; j <= aNbF; ++j)
    {
      TopoShape aFIm = aMap(j);

      Standard_Boolean isIN         = anINMap.Contains(aFIm);
      Standard_Boolean isINOpposite = anOppositeINMap.Contains(aFIm);

      // Filtering for FUSE - avoid any IN faces
      if (bAvoidIN && (isIN || isINOpposite))
        continue;

      // Filtering for CUT - avoid faces IN for both groups
      if (bAvoidINforBoth && isIN && isINOpposite)
        continue;

      // Treatment of SD faces
      if (!aMFence.Add(aFIm))
      {
        if (!anOppositeMap.Contains(aFIm))
        {
          // The face belongs to only one group
          if (bTakeIN != isSameOriNeeded)
            aMFToAvoid.Add(aFIm);
        }
        else
        {
          // The face belongs to both groups.
          // Using its orientation decide if it is needed in the result or not.
          Standard_Boolean isSameOri = !aMFenceOri.Add(aFIm);
          if (isSameOriNeeded == isSameOri)
          {
            // Take the shape without classification
            if (aMResFacesFence.Add(aFIm))
              aMResFacesOri.Add(aFIm);
          }
          else
            // Remove the face
            aMFToAvoid.Add(aFIm);

          continue;
        }
      }
      if (!aMFenceOri.Add(aFIm))
        continue;

      if (bTakeIN == isINOpposite)
      {
        if (isIN)
        {
          aMResFacesOri.Add(aFIm);
          aMResFacesOri.Add(aFIm.Reversed());
        }
        else if (bTakeIN && !isSameOriNeeded)
          aMResFacesOri.Add(aFIm.Reversed());
        else
          aMResFacesOri.Add(aFIm);
        aMResFacesFence.Add(aFIm);
      }
    }
  }

  // Remove the faces which has to be avoided
  ShapeList   aResFaces;
  const Standard_Integer aNbRF = aMResFacesOri.Extent();
  for (Standard_Integer i = 1; i <= aNbRF; ++i)
  {
    const TopoShape& aRF = aMResFacesOri(i);
    if (!aMFToAvoid.Contains(aRF))
      aResFaces.Append(aRF);
  }
  Message_ProgressScope aPS(theRange, NULL, 2);
  ShapeBuilder          aBB;

  // Try to build closed solids from the faces
  BOPAlgo_BuilderSolid aBS;
  aBS.SetShapes(aResFaces);
  aBS.SetRunParallel(myRunParallel);
  aBS.SetContext(myContext);
  aBS.SetFuzzyValue(myFuzzyValue);
  aBS.Perform(aPS.Next());

  // Resulting solids
  ShapeList aResSolids;

  aMFence.Clear();
  if (!aBS.HasErrors())
  {
    // If any, add solids into resulting compound
    TopTools_ListIteratorOfListOfShape itA(aBS.Areas());
    for (; itA.More(); itA.Next())
    {
      const TopoShape& aSolid = itA.Value();
      // The solid must contain at least one face
      // from either of objects or tools
      ShapeExplorer expF(aSolid, TopAbs_FACE);
      for (; expF.More(); expF.Next())
      {
        const TopoShape& aF = expF.Current();
        if (aMObjFacesOri.Contains(aF) || aMToolFacesOri.Contains(aF))
          break;
      }
      if (expF.More())
      {
        aResSolids.Append(aSolid);
        TopExp1::MapShapes(aSolid, aMFence);
      }
    }
  }
  else
  {
    return;
  }

  // Collect unused faces
  TopoCompound anUnUsedFaces;
  aBB.MakeCompound(anUnUsedFaces);

  ShapeList::Iterator itLF(aResFaces);
  for (; itLF.More(); itLF.Next())
  {
    if (aMFence.Add(itLF.Value()))
      aBB.Add(anUnUsedFaces, itLF.Value());
  }

  // Build blocks from the unused faces
  ShapeList aLCB;
  AlgoTools::MakeConnexityBlocks(anUnUsedFaces, TopAbs_EDGE, TopAbs_FACE, aLCB);

  // Build solid from each block
  TopTools_ListIteratorOfListOfShape itCB(aLCB);
  for (; itCB.More(); itCB.Next())
  {
    const TopoShape& aCB = itCB.Value();
    TopoShell        aShell;
    aBB.MakeShell(aShell);
    // Add faces of the block to the shell
    ShapeExplorer anExpF(aCB, TopAbs_FACE);
    for (; anExpF.More(); anExpF.Next())
      aBB.Add(aShell, TopoDS::Face(anExpF.Current()));

    AlgoTools::OrientFacesOnShell(aShell);
    // Make solid out of the shell
    TopoSolid aSolid;
    aBB.MakeSolid(aSolid);
    aBB.Add(aSolid, aShell);
    // Add new solid to result
    aResSolids.Append(aSolid);
  }

  if (!bAvoidIN)
  {
    // Fill solids with internal parts coming with the solids
    ShapeList anInParts;
    for (Standard_Integer i = 0; i < 2; ++i)
    {
      const ShapeList&    aList = !i ? theObjects : theTools;
      ShapeList::Iterator itLS(aList);
      for (; itLS.More(); itLS.Next())
      {
        ShapeExplorer expS(itLS.Value(), TopAbs_SOLID);
        for (; expS.More(); expS.Next())
        {
          const TopoShape& aS = expS.Current(); // Solid
          for (TopoDS_Iterator it(aS); it.More(); it.Next())
          {
            const TopoShape& aSInt = it.Value();
            if (aSInt.Orientation() == TopAbs_INTERNAL)
              anInParts.Append(aSInt); // vertex or edge
            else
            {
              // shell treatment
              TopoDS_Iterator itInt(aSInt);
              if (itInt.More() && itInt.Value().Orientation() == TopAbs_INTERNAL)
                anInParts.Append(aSInt);
            }
          }
        }
      }
    }

    BooleanTools::FillInternals(aResSolids, anInParts, myImages, myContext);
  }

  // Combine solids into compound
  TopoShape aResult;
  aBB.MakeCompound(TopoDS::Compound(aResult));

  ShapeList::Iterator itLS(aResSolids);
  for (; itLS.More(); itLS.Next())
    aBB.Add(aResult, itLS.Value());

  myShape = aResult;
  PrepareHistory(aPS.Next());
}
