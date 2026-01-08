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

#include <BOPAlgo_Tools.hxx>

#include <BOPAlgo_Builder.hxx>
#include <BOPAlgo_BuilderFace.hxx>
#include <BOPDS_DataMapOfPaveBlockListOfPaveBlock.hxx>
#include <BOPDS_DS.hxx>
#include <BOPDS_PaveBlock.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BOPTools_BoxTree.hxx>
#include <BOPTools_Parallel.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepLib.hxx>
#include <Bnd_Tools.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Parab.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <IntTools_Context.hxx>
#include <NCollection_IncAllocator.hxx>
#include <NCollection_Vector.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <TColStd_IndexedMapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#include <algorithm>

typedef NCollection_IndexedDataMap<TopoShape, Dir3d, ShapeHasher>
  BOPAlgo_IndexedDataMapOfShapeDir;
typedef NCollection_IndexedDataMap<TopoShape, gp_Pln, ShapeHasher>
  BOPAlgo_IndexedDataMapOfShapePln;

static void MakeWires(const TopTools_IndexedMapOfShape& theEdges,
                      TopoCompound&                  theWires,
                      const Standard_Boolean            theCheckUniquePlane,
                      BOPAlgo_IndexedDataMapOfShapeDir& theDMEdgeTgt,
                      TopTools_MapOfShape&              theMEdgesNoUniquePlane);

static Standard_Boolean FindPlane(const BRepAdaptor_Curve& theCurve, gp_Pln& thePlane);

static Standard_Boolean FindPlane(const TopoShape&               theWire,
                                  gp_Pln&                           thePlane,
                                  BOPAlgo_IndexedDataMapOfShapeDir& theDMEdgeTgt,
                                  TopTools_MapOfShape&              theMEdgesNoUniquePlane);

static Standard_Boolean FindEdgeTangent(const TopoEdge&                theEdge,
                                        BOPAlgo_IndexedDataMapOfShapeDir& theDMEdgeTgt,
                                        Dir3d&                           theTgt);

static Standard_Boolean FindEdgeTangent(const BRepAdaptor_Curve& theCurve, Vector3d& theTangent);

//=================================================================================================

void BooleanTools::FillMap(const Handle(BOPDS_PaveBlock)&                aPB,
                            const Standard_Integer                        nF,
                            BOPDS_IndexedDataMapOfPaveBlockListOfInteger& aMPBLI,
                            const Handle(NCollection_BaseAllocator)&      aAllocator)
{
  TColStd_ListOfInteger* pLI = aMPBLI.ChangeSeek(aPB);
  if (!pLI)
  {
    pLI = &aMPBLI(aMPBLI.Add(aPB, TColStd_ListOfInteger(aAllocator)));
  }
  pLI->Append(nF);
}

//=================================================================================================

void BooleanTools::PerformCommonBlocks(BOPDS_IndexedDataMapOfPaveBlockListOfPaveBlock& aMPBLPB,
                                        const Handle(NCollection_BaseAllocator)&        aAllocator,
                                        BOPDS_PDS&                                      pDS,
                                        const Handle(IntTools_Context)&                 theContext)
{
  Standard_Integer aNbCB;
  //
  aNbCB = aMPBLPB.Extent();
  if (!aNbCB)
  {
    return;
  }
  // Make Blocks of the pave blocks
  NCollection_List<BOPDS_ListOfPaveBlock> aMBlocks(aAllocator);
  BooleanTools::MakeBlocks(aMPBLPB, aMBlocks, aAllocator);

  // Use temporary allocator for the local fence map
  Handle(NCollection_IncAllocator) anAllocTmp = new NCollection_IncAllocator;

  NCollection_List<BOPDS_ListOfPaveBlock>::Iterator aItB(aMBlocks);
  for (; aItB.More(); aItB.Next())
  {
    const BOPDS_ListOfPaveBlock& aLPB  = aItB.Value();
    Standard_Integer             aNbPB = aLPB.Extent();
    if (aNbPB < 2)
      continue;

    // Reset the allocator
    anAllocTmp->Reset(false);
    // New common block
    Handle(BOPDS_CommonBlock) aCB;
    // Faces of the common block
    TColStd_ListOfInteger aLFaces;
    // Fence map to avoid duplicates in the list of faces of the common block
    TColStd_MapOfInteger aMFaces(1, anAllocTmp);

    BOPDS_ListIteratorOfListOfPaveBlock aItLPB(aLPB);
    for (; aItLPB.More(); aItLPB.Next())
    {
      const Handle(BOPDS_PaveBlock)& aPB = aItLPB.Value();
      if (pDS->IsCommonBlock(aPB))
      {
        const Handle(BOPDS_CommonBlock)& aCBx = pDS->CommonBlock(aPB);
        // Move all faces to the new common block
        TColStd_ListIteratorOfListOfInteger aItLF(aCBx->Faces());
        for (; aItLF.More(); aItLF.Next())
        {
          const Standard_Integer nF = aItLF.Value();
          // Append to common list avoiding duplicates
          if (aMFaces.Add(nF))
            aLFaces.Append(nF);
        }
        if (aCB.IsNull())
          aCB = aCBx;
      }
    }

    if (aCB.IsNull())
      aCB = new BOPDS_CommonBlock;

    aCB->SetPaveBlocks(aLPB);
    aCB->SetFaces(aLFaces);
    for (aItLPB.Initialize(aLPB); aItLPB.More(); aItLPB.Next())
      pDS->SetCommonBlock(aItLPB.Value(), aCB);

    // Compute tolerance for Common Block1
    Standard_Real aTolCB = BooleanTools::ComputeToleranceOfCB(aCB, pDS, theContext);
    aCB->SetTolerance(aTolCB);
  }
}

//=================================================================================================

void BooleanTools::PerformCommonBlocks(const BOPDS_IndexedDataMapOfPaveBlockListOfInteger& aMPBLI,
                                        const Handle(NCollection_BaseAllocator)&, // aAllocator
                                        BOPDS_PDS&                      pDS,
                                        const Handle(IntTools_Context)& theContext)
{
  Standard_Integer                    nF, i, aNb;
  TColStd_ListIteratorOfListOfInteger aItLI;
  Handle(BOPDS_PaveBlock)             aPB;
  Handle(BOPDS_CommonBlock)           aCB;
  //
  aNb = aMPBLI.Extent();
  for (i = 1; i <= aNb; ++i)
  {
    aPB = aMPBLI.FindKey(i);
    if (pDS->IsCommonBlock(aPB))
    {
      aCB = pDS->CommonBlock(aPB);
    }
    else
    {
      aCB = new BOPDS_CommonBlock;
      aCB->AddPaveBlock(aPB);
    }
    //
    const TColStd_ListOfInteger& aLI = aMPBLI.FindFromKey(aPB);
    TColStd_ListOfInteger        aNewFaces;
    const TColStd_ListOfInteger& anOldFaces = aCB->Faces();
    aItLI.Initialize(aLI);
    for (; aItLI.More(); aItLI.Next())
    {
      nF = aItLI.Value();
      // the both lists aLI and anOldFaces are expected to be short,
      // so we can allow to run nested loop here
      TColStd_ListIteratorOfListOfInteger it(anOldFaces);
      for (; it.More(); it.Next())
      {
        if (it.Value() == nF)
          break;
      }
      if (!it.More())
      {
        aNewFaces.Append(nF);
      }
    }
    aCB->AppendFaces(aNewFaces);
    pDS->SetCommonBlock(aPB, aCB);
    // Compute tolerance for Common Block1
    Standard_Real aTolCB = BooleanTools::ComputeToleranceOfCB(aCB, pDS, theContext);
    aCB->SetTolerance(aTolCB);
  }
}

//=================================================================================================

Standard_Real BooleanTools::ComputeToleranceOfCB(const Handle(BOPDS_CommonBlock)& theCB,
                                                  const BOPDS_PDS                  theDS,
                                                  const Handle(IntTools_Context)&  theContext)
{
  Standard_Real aTolMax = 0.;
  if (theCB.IsNull())
  {
    return aTolMax;
  }
  //
  const Handle(BOPDS_PaveBlock)& aPBR = theCB->PaveBlock1();
  Standard_Integer               nE   = aPBR->OriginalEdge();
  const TopoEdge&             aEOr = *(TopoEdge*)&theDS->Shape(nE);
  aTolMax                             = BRepInspector::Tolerance(aEOr);
  //
  const BOPDS_ListOfPaveBlock& aLPB = theCB->PaveBlocks();
  const TColStd_ListOfInteger& aLFI = theCB->Faces();
  //
  if ((aLPB.Extent() < 2) && aLFI.IsEmpty())
  {
    return aTolMax;
  }
  //
  const Standard_Integer aNbPnt = 11;
  Standard_Real          aTol, aT, aT1, aT2, aDt;
  Point3d                 aP;
  //
  const Handle(GeomCurve3d)& aC3D = BRepInspector::Curve(aEOr, aT1, aT2);
  //
  aPBR->Range(aT1, aT2);
  aDt = (aT2 - aT1) / (aNbPnt + 1);
  //
  Handle(IntTools_Context) aCtx = theContext;
  if (aCtx.IsNull())
    aCtx = new IntTools_Context();

  // compute max tolerance for common blocks on edges
  if (aLPB.Extent() > 1)
  {
    // compute max distance between edges
    BOPDS_ListIteratorOfListOfPaveBlock aItPB;
    GeomAPI_ProjectPointOnCurve         aProjPC;
    //
    aItPB.Initialize(aLPB);
    for (; aItPB.More(); aItPB.Next())
    {
      const Handle(BOPDS_PaveBlock)& aPB = aItPB.Value();
      if (aPB == aPBR)
      {
        continue;
      }
      //
      nE                    = aPB->OriginalEdge();
      const TopoEdge& aE = *(TopoEdge*)&theDS->Shape(nE);
      aTol                  = BRepInspector::Tolerance(aE);
      //
      aProjPC = aCtx->ProjPC(aE);
      //
      aT = aT1;
      for (Standard_Integer i = 1; i <= aNbPnt; i++)
      {
        aT += aDt;
        aC3D->D0(aT, aP);
        aProjPC.Perform(aP);
        if (aProjPC.NbPoints())
        {
          Standard_Real aTolNew = aTol + aProjPC.LowerDistance();
          if (aTolNew > aTolMax)
          {
            aTolMax = aTolNew;
          }
        }
      }
    }
  }
  //
  // compute max tolerance for common blocks on faces
  if (aLFI.Extent())
  {
    for (TColStd_ListIteratorOfListOfInteger aItLI(aLFI); aItLI.More(); aItLI.Next())
    {
      const Standard_Integer nF = aItLI.Value();
      const TopoFace&     aF = *(TopoFace*)&theDS->Shape(nF);
      aTol                      = BRepInspector::Tolerance(aF);
      //
      PointOnSurfProjector& aProjPS = aCtx->ProjPS(aF);
      //
      aT = aT1;
      for (Standard_Integer i = 1; i <= aNbPnt; i++)
      {
        aT += aDt;
        aC3D->D0(aT, aP);
        aProjPS.Perform(aP);
        if (aProjPS.NbPoints())
        {
          Standard_Real aTolNew = aTol + aProjPS.LowerDistance();
          if (aTolNew > aTolMax)
          {
            aTolMax = aTolNew;
          }
        }
      }
    }
  }
  //
  return aTolMax;
}

//=================================================================================================

Standard_Integer BooleanTools::EdgesToWires(const TopoShape&    theEdges,
                                             TopoShape&          theWires,
                                             const Standard_Boolean theShared,
                                             const Standard_Real    theAngTol)
{
  Standard_Integer iErr = 0;
  //
  // 1. Check the input edges
  //
  // List of edges to process
  ShapeList aLE;
  //
  ShapeExplorer aExp(theEdges, TopAbs_EDGE);
  for (; aExp.More(); aExp.Next())
  {
    const TopoEdge& aE = TopoDS::Edge(aExp.Current());
    if (!BRepInspector::Degenerated(aE) && BRepInspector::IsGeometric(aE))
    {
      aLE.Append(aExp.Current());
    }
  }
  //
  if (aLE.IsEmpty())
  {
    // no edges to process
    iErr = 1;
    return iErr;
  }
  //
  ShapeBuilder    aBB;
  TopoCompound aRWires;
  aBB.MakeCompound(aRWires);
  //
  if (aLE.Extent() == 1)
  {
    TopoWire aWire;
    aBB.MakeWire(aWire);
    aBB.Add(aWire, aLE.First());
    aBB.Add(aRWires, aWire);
    theWires = aRWires;
    return iErr;
  }
  //
  // 2. Make compound of shared edges
  TopoShape aSEdges;
  //
  if (!theShared)
  {
    // intersect the edges if necessary
    BOPAlgo_Builder aGF;
    aGF.SetArguments(aLE);
    aGF.Perform();
    if (aGF.HasErrors())
    {
      // unable to share the edges
      iErr = 2;
      return iErr;
    }
    //
    aSEdges = aGF.Shape();
  }
  else
  {
    aBB.MakeCompound(TopoDS::Compound(aSEdges));
    TopTools_ListIteratorOfListOfShape aItLE(aLE);
    for (; aItLE.More(); aItLE.Next())
    {
      aBB.Add(aSEdges, aItLE.Value());
    }
  }
  //
  // 3. Find edges located in the same planes and make wires from them.
  // If the plane cannot be found for a single edge, then it is necessary
  // to find all pairs of connected edges with the same cross product.

  // Try to compute the plane in which the edge is located
  BOPAlgo_IndexedDataMapOfShapePln aDMEdgePln;
  // Compute the tangent direction for the edges for which the plane is not defined
  BOPAlgo_IndexedDataMapOfShapeDir aDMEdgeTgt;
  //
  // edges for which the plane is not found
  TopTools_MapOfShape aMEdgesNoUniquePlane;
  //
  // edges for which the plane cannot be found on a single edge
  TopoCompound aLEdges;
  aBB.MakeCompound(aLEdges);
  //
  aExp.Init(aSEdges, TopAbs_EDGE);
  for (; aExp.More(); aExp.Next())
  {
    const TopoEdge& aE = TopoDS::Edge(aExp.Current());
    BRepAdaptor_Curve  aBAC(aE);
    //
    gp_Pln aPln;
    if (FindPlane(aBAC, aPln))
    {
      aDMEdgePln.Add(aE, aPln);
    }
    else
    {
      Vector3d aVT;
      if (FindEdgeTangent(aBAC, aVT))
      {
        aDMEdgeTgt.Add(aE, Dir3d(aVT));
        aBB.Add(aLEdges, aE);
        aMEdgesNoUniquePlane.Add(aE);
      }
    }
  }
  //
  typedef NCollection_List<Dir3d> BOPAlgo_ListOfDir;
  //
  // to avoid processing of the same edges in the same plane store
  // the processed planes into a list and use it as a fence map
  BOPAlgo_ListOfDir aLPFence;
  //
  // used edges
  TopTools_MapOfShape aMEFence;
  //
  // look for a planes on the single edges
  Standard_Integer i, j, aNbPlanes = aDMEdgePln.Extent(), aNbEdges = aDMEdgeTgt.Extent();
  for (i = 1; i <= aNbPlanes; ++i)
  {
    const TopoShape& aEI = aDMEdgePln.FindKey(i);
    if (!aMEFence.Add(aEI))
    {
      continue;
    }
    //
    const gp_Pln& aPlnI = aDMEdgePln(i);
    const Dir3d& aDI   = aPlnI.Position1().Direction();
    //
    aLPFence.Append(aDI);
    //
    TopTools_IndexedMapOfShape aMEPln;
    aMEPln.Add(aEI);
    //
    TopTools_IndexedMapOfShape aMV;
    TopExp1::MapShapes(aEI, TopAbs_VERTEX, aMV);
    //
    // look for other edges with the plane parallel to current one
    for (j = i + 1; j <= aNbPlanes; ++j)
    {
      const Dir3d& aDJ = aDMEdgePln(j).Position1().Direction();
      if (aDI.IsParallel(aDJ, theAngTol))
      {
        const TopoShape& aEJ = aDMEdgePln.FindKey(j);
        aMEPln.Add(aEJ);
        aMEFence.Add(aEJ);
        TopExp1::MapShapes(aEJ, TopAbs_VERTEX, aMV);
      }
    }
    //
    // look for all other edges located in the plane parallel to current one
    TopoCompound aCEPln;
    aBB.MakeCompound(aCEPln);
    //
    for (j = 1; j <= aNbEdges; ++j)
    {
      const Dir3d& aDJ = aDMEdgeTgt(j);
      if (aDI.IsNormal(aDJ, theAngTol))
      {
        aBB.Add(aCEPln, aDMEdgeTgt.FindKey(j));
      }
    }
    //
    // make blocks of these edges and check blocks to be connected
    // to any of the already added edges or forming a wire themselves
    ShapeList aLCBE;
    AlgoTools::MakeConnexityBlocks(aCEPln, TopAbs_VERTEX, TopAbs_EDGE, aLCBE);
    //
    // make wire from each block
    TopTools_ListIteratorOfListOfShape aItLCB(aLCBE);
    for (; aItLCB.More(); aItLCB.Next())
    {
      const TopoShape& aCBE = aItLCB.Value();
      //
      // check connectivity
      ShapeExplorer aExpV(aCBE, TopAbs_VERTEX);
      for (; aExpV.More(); aExpV.Next())
      {
        if (aMV.Contains(aExpV.Current()))
        {
          break;
        }
      }
      //
      Standard_Boolean bAddBlock = aExpV.More();
      if (!bAddBlock)
      {
        // check if the edges are forming a wire
        gp_Pln aPln;
        bAddBlock = FindPlane(aCBE, aPln, aDMEdgeTgt, aMEdgesNoUniquePlane);
      }
      //
      if (bAddBlock)
      {
        // add edges
        for (TopoDS_Iterator aItE(aCBE); aItE.More(); aItE.Next())
        {
          aMEPln.Add(aItE.Value());
        }
      }
    }
    //
    MakeWires(aMEPln, aRWires, Standard_False, aDMEdgeTgt, aMEdgesNoUniquePlane);
  }
  //
  // make connection map from vertices to edges to find the connected pairs
  TopTools_IndexedDataMapOfShapeListOfShape aDMVE;
  TopExp1::MapShapesAndAncestors(aLEdges, TopAbs_VERTEX, TopAbs_EDGE, aDMVE);
  //
  // find planes for connected edges
  Standard_Integer aNbV = aDMVE.Extent();
  for (i = 1; i <= aNbV; ++i)
  {
    const ShapeList& aLEI = aDMVE(i);
    if (aLEI.Extent() < 2)
    {
      continue;
    }
    //
    TopTools_ListIteratorOfListOfShape aItLEI1(aLEI);
    for (; aItLEI1.More(); aItLEI1.Next())
    {
      const TopoShape& aEI1 = aItLEI1.Value();
      const Dir3d&       aDI1 = aDMEdgeTgt.FindFromKey(aEI1);
      //
      TopTools_ListIteratorOfListOfShape aItLEI2(aLEI);
      for (; aItLEI2.More(); aItLEI2.Next())
      {
        const TopoShape& aEI2 = aItLEI2.Value();
        if (aEI2.IsSame(aEI1))
        {
          continue;
        }
        //
        const Dir3d& aDI2 = aDMEdgeTgt.FindFromKey(aEI2);
        //
        if (aDI1.IsParallel(aDI2, theAngTol))
        {
          continue;
        }
        //
        Dir3d aDNI = aDI1 ^ aDI2;
        //
        // check if this normal direction has not been checked yet
        BOPAlgo_ListOfDir::Iterator aItLPln(aLPFence);
        for (; aItLPln.More(); aItLPln.Next())
        {
          if (aDNI.IsParallel(aItLPln.Value(), theAngTol))
          {
            break;
          }
        }
        if (aItLPln.More())
        {
          continue;
        }
        //
        aLPFence.Append(aDNI);
        //
        // find all other edges in the plane parallel to current one
        TopTools_IndexedMapOfShape aMEPln;
        aMEPln.Add(aEI1);
        aMEPln.Add(aEI2);
        //
        // iterate on all other edges to find all edges lying in the plane parallel to current one
        for (j = 1; j <= aNbEdges; ++j)
        {
          const Dir3d& aDJ = aDMEdgeTgt(j);
          if (aDNI.IsNormal(aDJ, theAngTol))
          {
            aMEPln.Add(aDMEdgeTgt.FindKey(j));
          }
        }
        //
        MakeWires(aMEPln, aRWires, Standard_True, aDMEdgeTgt, aMEdgesNoUniquePlane);
      } // for (; aItLEI2.More(); aItLEI2.Next()) {
    } // for (; aItLEI1.More(); aItLEI1.Next()) {
  } // for (i = 1; i < aNb; ++i) {
  //
  // 4. Find unused edges and make wires from them
  TopTools_IndexedMapOfShape aMEAlone, aMEUsed;
  TopExp1::MapShapes(aRWires, TopAbs_EDGE, aMEUsed);
  //
  for (i = 1; i <= aNbEdges; ++i)
  {
    const TopoShape& aE = aDMEdgeTgt.FindKey(i);
    if (!aMEUsed.Contains(aE))
    {
      aMEAlone.Add(aE);
    }
  }
  //
  MakeWires(aMEAlone, aRWires, Standard_False, aDMEdgeTgt, aMEdgesNoUniquePlane);
  //
  theWires = aRWires;
  //
  return iErr;
}

//=================================================================================================

Standard_Boolean BooleanTools::WiresToFaces(const TopoShape& theWires,
                                             TopoShape&       theFaces,
                                             const Standard_Real theAngTol)
{
  ShapeBuilder        aBB;
  TopTools_MapOfShape aMFence;
  TopoCompound     aRFaces;
  aBB.MakeCompound(aRFaces);
  //
  const Standard_Real aMax = 1.e+8;
  //
  // map to store the tangent vectors for the edges
  BOPAlgo_IndexedDataMapOfShapeDir aDMEdgeTgt;
  // maps to store the planes found for the wires
  BOPAlgo_IndexedDataMapOfShapePln aDMWirePln;
  // map to store the tolerance for the wire
  NCollection_DataMap<TopoShape, Standard_Real, ShapeHasher> aDMWireTol;
  // edges for which the plane is not found
  TopTools_MapOfShape aMEdgesNoUniquePlane;
  //
  // Find planes for the wires
  ShapeExplorer aExpW(theWires, TopAbs_WIRE);
  for (; aExpW.More(); aExpW.Next())
  {
    const TopoWire& aWire = TopoDS::Wire(aExpW.Current());
    gp_Pln             aPlane;
    if (FindPlane(aWire, aPlane, aDMEdgeTgt, aMEdgesNoUniquePlane))
    {
      aDMWirePln.Add(aWire, aPlane);
      // find tolerance for the wire - max tolerance of its edges
      aDMWireTol.Bind(aWire, BRepInspector::MaxTolerance(aWire, TopAbs_EDGE));
    }
  }
  //
  Standard_Integer i, j, aNb = aDMWirePln.Extent();
  for (i = 1; i <= aNb; ++i)
  {
    const TopoShape& aWireI = aDMWirePln.FindKey(i);
    if (aMFence.Contains(aWireI))
    {
      continue;
    }
    //
    const gp_Pln& aPlnI = aDMWirePln(i);
    //
    ShapeList aLW;
    aLW.Append(aWireI);
    aMFence.Add(aWireI);
    //
    Standard_Real aTolI = aDMWireTol.Find(aWireI);
    //
    // Find other wires in the same plane
    for (j = i + 1; j <= aNb; ++j)
    {
      const TopoShape& aWireJ = aDMWirePln.FindKey(j);
      if (aMFence.Contains(aWireJ))
      {
        continue;
      }
      //
      // check if the planes are the same
      const gp_Pln& aPlnJ = aDMWirePln(j);
      // check direction of the planes
      if (!aPlnI.Position1().Direction().IsParallel(aPlnJ.Position1().Direction(), theAngTol))
      {
        continue;
      }
      // check distance between the planes
      Standard_Real aDist = aPlnI.Distance(aPlnJ.Location());
      Standard_Real aTolJ = aDMWireTol.Find(aWireJ);
      if (aDist > (aTolI + aTolJ))
      {
        continue;
      }
      //
      aLW.Append(aWireJ);
      aMFence.Add(aWireJ);
    }
    //
    // Take the edges to build the face
    ShapeList               aLE;
    TopTools_ListIteratorOfListOfShape aItLW(aLW);
    for (; aItLW.More(); aItLW.Next())
    {
      TopoDS_Iterator aItE(aItLW.Value());
      for (; aItE.More(); aItE.Next())
      {
        aLE.Append(aItE.Value().Oriented(TopAbs_FORWARD));
        aLE.Append(aItE.Value().Oriented(TopAbs_REVERSED));
      }
    }
    //
    // build planar face
    TopoFace aFF = FaceMaker(aPlnI, -aMax, aMax, -aMax, aMax).Face();
    aFF.Orientation(TopAbs_FORWARD);
    //
    try
    {
      OCC_CATCH_SIGNALS
      //
      // build pcurves for edges on this face
      BRepLib1::BuildPCurveForEdgesOnPlane(aLE, aFF);
      //
      // split the face with the edges
      BOPAlgo_BuilderFace aBF;
      aBF.SetShapes(aLE);
      aBF.SetFace(aFF);
      aBF.Perform();
      if (aBF.HasErrors())
      {
        continue;
      }
      //
      const ShapeList&        aLFSp = aBF.Areas();
      TopTools_ListIteratorOfListOfShape aItLF(aLFSp);
      for (; aItLF.More(); aItLF.Next())
      {
        const TopoShape& aFSp = aItLF.ChangeValue();
        aBB.Add(aRFaces, aFSp);
      }
    }
    catch (ExceptionBase const&)
    {
      continue;
    }
  }
  //
  // fix tolerances of the resulting faces
  TopTools_IndexedMapOfShape aMEmpty;
  AlgoTools::CorrectTolerances(aRFaces, aMEmpty, 0.05, Standard_False);
  AlgoTools::CorrectShapeTolerances(aRFaces, aMEmpty, Standard_False);
  //
  theFaces = aRFaces;
  return theFaces.NbChildren() > 0;
}

//=======================================================================
// function : MakeWires
// purpose  : Makes wires from the separate blocks of the given edges
//=======================================================================
void MakeWires(const TopTools_IndexedMapOfShape& theEdges,
               TopoCompound&                  theWires,
               const Standard_Boolean            theCheckUniquePlane,
               BOPAlgo_IndexedDataMapOfShapeDir& theDMEdgeTgt,
               TopTools_MapOfShape&              theMEdgesNoUniquePlane)
{
  TopoCompound aCE;
  ShapeBuilder().MakeCompound(aCE);
  Standard_Integer i, aNbE = theEdges.Extent();
  for (i = 1; i <= aNbE; ++i)
  {
    ShapeBuilder().Add(aCE, theEdges(i));
  }
  //
  ShapeList aLCBE;
  AlgoTools::MakeConnexityBlocks(aCE, TopAbs_VERTEX, TopAbs_EDGE, aLCBE);
  //
  // make wire from each block
  TopTools_ListIteratorOfListOfShape aItLCB(aLCBE);
  for (; aItLCB.More(); aItLCB.Next())
  {
    const TopoShape& aCBE = aItLCB.Value();
    //
    if (theCheckUniquePlane)
    {
      gp_Pln aPln;
      if (!FindPlane(aCBE, aPln, theDMEdgeTgt, theMEdgesNoUniquePlane))
      {
        continue;
      }
    }
    //
    TopoWire aWire;
    ShapeBuilder().MakeWire(aWire);
    for (TopoDS_Iterator aItE(aCBE); aItE.More(); aItE.Next())
    {
      ShapeBuilder().Add(aWire, aItE.Value());
    }
    //
    ShapeBuilder().Add(theWires, aWire);
  }
}

//=======================================================================
// function : FindEdgeTangent
// purpose  : Finds the tangent for the edge using the map
//=======================================================================
Standard_Boolean FindEdgeTangent(const TopoEdge&                theEdge,
                                 BOPAlgo_IndexedDataMapOfShapeDir& theDMEdgeTgt,
                                 Dir3d&                           theTgt)
{
  Dir3d* pDTE = theDMEdgeTgt.ChangeSeek(theEdge);
  if (!pDTE)
  {
    Vector3d            aVTE;
    BRepAdaptor_Curve aBAC(theEdge);
    if (!FindEdgeTangent(aBAC, aVTE))
    {
      return Standard_False;
    }
    pDTE = &theDMEdgeTgt(theDMEdgeTgt.Add(theEdge, Dir3d(aVTE)));
  }
  theTgt = *pDTE;
  return Standard_True;
}

//=======================================================================
// function : FindEdgeTangent
// purpose  : Finds the tangent for the edge
//=======================================================================
Standard_Boolean FindEdgeTangent(const BRepAdaptor_Curve& theCurve, Vector3d& theTangent)
{
  if (!theCurve.Is3DCurve())
  {
    return Standard_False;
  }
  // for the line the tangent is defined by the direction
  if (theCurve.GetType() == GeomAbs_Line)
  {
    theTangent = theCurve.Line().Position1().Direction();
    return Standard_True;
  }
  //
  // for other curves take D1 and check for its length
  Standard_Real          aT, aT1(theCurve.FirstParameter()), aT2(theCurve.LastParameter());
  const Standard_Integer aNbP = 11;
  const Standard_Real    aDt  = (aT2 - aT1) / aNbP;
  //
  for (aT = aT1 + aDt; aT <= aT2; aT += aDt)
  {
    Point3d aP;
    theCurve.D1(aT, aP, theTangent);
    if (theTangent.Magnitude() > Precision1::Confusion())
    {
      return Standard_True;
    }
  }
  //
  return Standard_False;
}

//=======================================================================
// function : FindPlane
// purpose  : Finds the plane in which the edge is located
//=======================================================================
Standard_Boolean FindPlane(const BRepAdaptor_Curve& theCurve, gp_Pln& thePlane)
{
  if (!theCurve.Is3DCurve())
  {
    return Standard_False;
  }
  //
  Standard_Boolean bFound = Standard_True;
  Vector3d           aVN;
  switch (theCurve.GetType())
  {
    case GeomAbs_Line:
      return Standard_False;
    case GeomAbs_Circle:
      aVN = theCurve.Circle().Position1().Direction();
      break;
    case GeomAbs_Ellipse:
      aVN = theCurve.Ellipse().Position1().Direction();
      break;
    case GeomAbs_Hyperbola:
      aVN = theCurve.Hyperbola().Position1().Direction();
      break;
    case GeomAbs_Parabola:
      aVN = theCurve.Parabola().Position1().Direction();
      break;
    default: {
      // for all other types of curve compute two tangent vectors
      // on the curve and cross them
      bFound = Standard_False;
      Standard_Real          aT, aT1(theCurve.FirstParameter()), aT2(theCurve.LastParameter());
      const Standard_Integer aNbP = 11;
      const Standard_Real    aDt  = (aT2 - aT1) / aNbP;
      //
      aT = aT1;
      Point3d aP1;
      Vector3d aV1;
      theCurve.D1(aT, aP1, aV1);
      //
      for (aT = aT1 + aDt; aT <= aT2; aT += aDt)
      {
        Point3d aP2;
        Vector3d aV2;
        theCurve.D1(aT, aP2, aV2);
        //
        aVN = aV1 ^ aV2;
        if (aVN.Magnitude() > Precision1::Confusion())
        {
          bFound = Standard_True;
          break;
        }
      }
      break;
    }
  }
  //
  if (bFound)
  {
    thePlane = gp_Pln(theCurve.Value(theCurve.FirstParameter()), Dir3d(aVN));
  }
  return bFound;
}

//=======================================================================
// function : FindPlane
// purpose  : Finds the plane in which the wire is located
//=======================================================================
Standard_Boolean FindPlane(const TopoShape&               theWire,
                           gp_Pln&                           thePlane,
                           BOPAlgo_IndexedDataMapOfShapeDir& theDMEdgeTgt,
                           TopTools_MapOfShape&              theMEdgesNoUniquePlane)
{
  ShapeExplorer aExpE1(theWire, TopAbs_EDGE);
  if (!aExpE1.More())
  {
    return Standard_False;
  }
  //
  // try to find two not parallel edges in wire to get normal of the plane
  for (; aExpE1.More(); aExpE1.Next())
  {
    // get the first edge in the wire
    const TopoEdge& aE1 = TopoDS::Edge(aExpE1.Current());
    //
    // find tangent for the first edge
    Dir3d aDTE1;
    if (!FindEdgeTangent(aE1, theDMEdgeTgt, aDTE1))
    {
      continue;
    }
    //
    // find the other edge not parallel to the first one
    ShapeExplorer aExpE2(theWire, TopAbs_EDGE);
    for (; aExpE2.More(); aExpE2.Next())
    {
      const TopoEdge& aE2 = TopoDS::Edge(aExpE2.Current());
      if (aE1.IsSame(aE2))
      {
        continue;
      }
      //
      // find tangent for the second edge
      Dir3d aDTE2;
      if (!FindEdgeTangent(aE2, theDMEdgeTgt, aDTE2))
      {
        continue;
      }
      //
      if (aDTE1.IsParallel(aDTE2, Precision1::Angular()))
      {
        continue;
      }
      //
      Dir3d aDN = aDTE1 ^ aDTE2;
      //
      TopoDS_Iterator aItV(aE1);
      thePlane = gp_Pln(BRepInspector::Pnt(TopoDS::Vertex(aItV.Value())), aDN);
      return Standard_True;
    }
  }
  //
  // try to compute normal on the single edge
  aExpE1.Init(theWire, TopAbs_EDGE);
  for (; aExpE1.More(); aExpE1.Next())
  {
    const TopoEdge& aE = TopoDS::Edge(aExpE1.Current());
    if (theMEdgesNoUniquePlane.Contains(aE))
    {
      continue;
    }
    BRepAdaptor_Curve aBAC(aE);
    if (FindPlane(aBAC, thePlane))
    {
      return Standard_True;
    }
    theMEdgesNoUniquePlane.Add(aE);
  }
  return Standard_False;
}

/////////////////////////////////////////////////////////////////////////
//=================================================================================================

class BOPAlgo_PairVerticesSelector : public BOPTools_BoxPairSelector
{
public:
  BOPAlgo_PairVerticesSelector()
      : myVertices(NULL),
        myFuzzyValue(Precision1::Confusion())
  {
  }

  //! Sets the map of vertices with tolerances
  void SetMapOfVerticesTolerances(const TopTools_IndexedDataMapOfShapeReal& theVertices)
  {
    myVertices = &theVertices;
  }

  //! Sets the fuzzy value
  void SetFuzzyValue(const Standard_Real theFuzzyValue) { myFuzzyValue = theFuzzyValue; }

  //! Checks and accepts the pair of elements.
  virtual Standard_Boolean Accept(const Standard_Integer theID1,
                                  const Standard_Integer theID2) Standard_OVERRIDE
  {
    if (!RejectElement(theID1, theID2))
    {
      const Standard_Integer anID1  = this->myBVHSet1->Element(theID1);
      const TopoVertex&   aV1    = TopoDS::Vertex(myVertices->FindKey(anID1));
      Standard_Real          aTolV1 = BRepInspector::Tolerance(aV1);
      if (aTolV1 < myVertices->FindFromIndex(anID1))
        aTolV1 = myVertices->FindFromIndex(anID1);
      Point3d aP1 = BRepInspector::Pnt(aV1);

      const Standard_Integer anID2  = this->myBVHSet1->Element(theID2);
      const TopoVertex&   aV2    = TopoDS::Vertex(myVertices->FindKey(anID2));
      Standard_Real          aTolV2 = BRepInspector::Tolerance(aV2);
      if (aTolV2 < myVertices->FindFromIndex(anID2))
        aTolV2 = myVertices->FindFromIndex(anID2);
      Point3d aP2 = BRepInspector::Pnt(aV2);

      Standard_Real aTolSum2 = aTolV1 + aTolV2 + myFuzzyValue;
      aTolSum2 *= aTolSum2;

      Standard_Real aD2 = aP1.SquareDistance(aP2);
      if (aD2 < aTolSum2)
      {
        myPairs.push_back(PairIDs(anID1, anID2));
        return Standard_True;
      }
    }
    return Standard_False;
  }

protected:
  const TopTools_IndexedDataMapOfShapeReal* myVertices;
  Standard_Real                             myFuzzyValue;
};

//
/////////////////////////////////////////////////////////////////////////

//=======================================================================
// function : IntersectVertices
// purpose  : Builds the chains of intersecting vertices
//=======================================================================
void BooleanTools::IntersectVertices(const TopTools_IndexedDataMapOfShapeReal& theVertices,
                                      const Standard_Real                       theFuzzyValue,
                                      TopTools_ListOfListOfShape&               theChains)
{
  Standard_Integer aNbV = theVertices.Extent();
  if (aNbV <= 1)
  {
    if (aNbV == 1)
    {
      theChains.Append(ShapeList()).Append(theVertices.FindKey(1));
    }
    return;
  }

  // Additional tolerance for intersection
  Standard_Real aTolAdd = theFuzzyValue / 2.;

  // Use BVH Tree for sorting the vertices
  BOPTools_BoxTree aBBTree;
  aBBTree.SetSize(aNbV);

  for (Standard_Integer i = 1; i <= aNbV; ++i)
  {
    const TopoVertex& aV   = TopoDS::Vertex(theVertices.FindKey(i));
    Standard_Real        aTol = BRepInspector::Tolerance(aV);
    if (aTol < theVertices(i))
      aTol = theVertices(i);

    // Build bnd box for vertex
    Box2 aBox;
    aBox.Add(BRepInspector::Pnt(aV));
    aBox.SetGap(aTol + aTolAdd);
    aBBTree.Add(i, Tools5::Bnd2BVH(aBox));
  }

  aBBTree.Build();

  // Perform selection of the interfering vertices
  BOPAlgo_PairVerticesSelector aPairSelector;
  aPairSelector.SetBVHSets(&aBBTree, &aBBTree);
  aPairSelector.SetSame(Standard_True);
  aPairSelector.SetMapOfVerticesTolerances(theVertices);
  aPairSelector.SetFuzzyValue(theFuzzyValue);
  aPairSelector.Select();

  // Treat the selected pairs
  const std::vector<BOPTools_BoxPairSelector::PairIDs>& aPairs = aPairSelector.Pairs();
  const Standard_Integer aNbPairs = static_cast<Standard_Integer>(aPairs.size());

  // Collect interfering pairs
  Handle(NCollection_IncAllocator) anAlloc = new NCollection_IncAllocator;
  NCollection_IndexedDataMap<Standard_Integer, TColStd_ListOfInteger> aMILI(1, anAlloc);

  for (Standard_Integer iPair = 0; iPair < aNbPairs; ++iPair)
  {
    const BOPTools_BoxPairSelector::PairIDs& aPair = aPairs[iPair];
    BooleanTools::FillMap(aPair.ID1, aPair.ID2, aMILI, anAlloc);
  }

  NCollection_List<TColStd_ListOfInteger> aBlocks(anAlloc);
  BooleanTools::MakeBlocks(aMILI, aBlocks, anAlloc);

  NCollection_List<TColStd_ListOfInteger>::Iterator itLI(aBlocks);
  for (; itLI.More(); itLI.Next())
  {
    const TColStd_ListOfInteger& aLI    = itLI.Value();
    ShapeList&        aChain = theChains.Append(ShapeList());

    for (TColStd_ListOfInteger::Iterator itI(aLI); itI.More(); itI.Next())
      aChain.Append(theVertices.FindKey(itI.Value()));
  }

  // Add not interfered vertices as a chain of 1 element
  for (Standard_Integer i = 1; i <= aNbV; ++i)
  {
    if (!aMILI.Contains(i))
    {
      ShapeList& aChain = theChains.Append(ShapeList());
      aChain.Append(theVertices.FindKey(i));
    }
  }
}

//=======================================================================
// Classification of the faces relatively solids
//=======================================================================

//=======================================================================
// class     : BOPAlgo_ShapeBox
// purpose   : Auxiliary class defining ShapeBox structure
//=======================================================================
class BOPAlgo_ShapeBox
{
public:
  //! Empty constructor
  BOPAlgo_ShapeBox() {};

  //! Sets the shape
  void SetShape(const TopoShape& theS) { myShape = theS; };

  //! Returns the shape
  const TopoShape& Shape() const { return myShape; };

  //! Sets the bounding box
  void SetBox(const Box2& theBox) { myBox = theBox; };

  //! Returns the bounding box
  const Box2& Box1() const { return myBox; };

private:
  TopoShape myShape;
  Box2      myBox;
};

// Vector of ShapeBox
typedef NCollection_Vector<BOPAlgo_ShapeBox> BOPAlgo_VectorOfShapeBox;

//=======================================================================
// class : BOPAlgo_FillIn3DParts
// purpose : Auxiliary class for faces classification in parallel mode
//=======================================================================
class BOPAlgo_FillIn3DParts : public BOPAlgo_ParallelAlgo
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructor
  BOPAlgo_FillIn3DParts()
  {
    myBBTree    = NULL;
    myVShapeBox = NULL;
  };

  //! Destructor
  virtual ~BOPAlgo_FillIn3DParts() {};

  //! Sets the solid
  void SetSolid(const TopoSolid& theSolid) { mySolid = theSolid; };

  //! Returns the solid
  const TopoSolid& Solid() const { return mySolid; };

  //! Sets the box for the solid
  void SetBoxS(const Box2& theBox) { myBoxS = theBox; };

  //! Returns the solid's box
  const Box2& BoxS() const { return myBoxS; };

  //! Sets own INTERNAL faces of the solid
  void SetOwnIF(const ShapeList& theLIF) { myOwnIF = theLIF; };

  //! Returns own INTERNAL faces of the solid
  const ShapeList& OwnIF() const { return myOwnIF; };

  //! Sets the Bounding Box1 tree
  void SetBBTree(BOPTools_BoxTree* theBBTree) { myBBTree = theBBTree; };

  //! Sets the ShapeBox structure
  void SetShapeBoxVector(const BOPAlgo_VectorOfShapeBox& theShapeBox)
  {
    myVShapeBox = (BOPAlgo_VectorOfShapeBox*)&theShapeBox;
  };

  //! Sets the context
  void SetContext(const Handle(IntTools_Context)& theContext) { myContext = theContext; }

  //! Returns the context
  const Handle(IntTools_Context)& Context() const { return myContext; }

  //! Performs the classification
  virtual void Perform();

  //! Returns the faces classified as IN for solid
  const ShapeList& InFaces() const { return myInFaces; };

private:
  //! Prepares Edge-Face connection map of the given shape
  void MapEdgesAndFaces(const TopoShape&                        theF,
                        TopTools_IndexedDataMapOfShapeListOfShape& theEFMap,
                        const Handle(NCollection_BaseAllocator)&   theAlloc);

  //! Makes the connexity block of faces using the connection map
  void MakeConnexityBlock(const TopoFace&                               theF,
                          const TopTools_IndexedMapOfShape&                theMEToAvoid,
                          const TopTools_IndexedDataMapOfShapeListOfShape& theEFMap,
                          TopTools_MapOfShape&                             theMFDone,
                          ShapeList&                            theLCB,
                          TopoFace&                                     theFaceToClassify);

  TopoSolid         mySolid;   //! Solid
  Box2              myBoxS;    // Bounding box of the solid
  ShapeList myOwnIF;   //! Own INTERNAL faces of the solid
  ShapeList myInFaces; //! Faces classified as IN

  BOPTools_BoxTree*         myBBTree;    //! BVH tree of bounding boxes
  BOPAlgo_VectorOfShapeBox* myVShapeBox; //! ShapeBoxMap

  TopoDS_Iterator myItF; //! Iterators
  TopoDS_Iterator myItW;

  Handle(IntTools_Context) myContext; //! Context
};

//=================================================================================================

void BOPAlgo_FillIn3DParts::Perform()
{
  Message_ProgressScope aPSOuter(myProgressRange, NULL, 2);
  if (UserBreak(aPSOuter))
  {
    return;
  }

  myInFaces.Clear();

  // 1. Select boxes of faces that are not out of aBoxS
  BOPTools_BoxTreeSelector aSelector;
  aSelector.SetBox(Tools5::Bnd2BVH(myBoxS));
  aSelector.SetBVHSet(myBBTree);
  //
  if (!aSelector.Select())
    return;

  const TColStd_ListOfInteger& aLIFP = aSelector.Indices();

  // 2. Fill maps of edges and faces of the solid

  Handle(NCollection_BaseAllocator) anAlloc = new NCollection_IncAllocator;

  BOPAlgo_VectorOfShapeBox& aVShapeBox = *myVShapeBox;

  TopTools_IndexedMapOfShape aMSE(1, anAlloc), aMSF(1, anAlloc);
  TopExp1::MapShapes(mySolid, TopAbs_EDGE, aMSE);
  TopExp1::MapShapes(mySolid, TopAbs_FACE, aMSF);

  // Check if the Solid contains any faces
  Standard_Boolean bIsEmpty = aMSF.IsEmpty();

  // Add own internal faces of the solid into aMSF
  TopTools_ListIteratorOfListOfShape aItLS(myOwnIF);
  for (; aItLS.More(); aItLS.Next())
    aMSF.Add(aItLS.Value());

  // 3. aIVec - faces to process.
  //    Filter the selected faces with faces of the solid.
  NCollection_Vector<Standard_Integer> aIVec(256, anAlloc);

  TColStd_ListIteratorOfListOfInteger aItLI(aLIFP);
  for (; aItLI.More(); aItLI.Next())
  {
    Standard_Integer    nFP = aItLI.Value();
    const TopoShape& aFP = aVShapeBox(nFP).Shape();
    if (!aMSF.Contains(aFP))
      aIVec.Appended() = nFP;
  }

  // 4. Classify faces relatively solid.
  //    Store faces that are IN mySolid into <myInFaces>

  Standard_Integer k, aNbFP = aIVec.Length();
  // Sort indices if necessary
  if (aNbFP > 1)
    std::sort(aIVec.begin(), aIVec.end());

  if (bIsEmpty)
  {
    // The solid is empty as it does not contain any faces.
    // It could happen when the input solid consists of INTERNAL faces only.
    // Classification of any point relatively empty solid would always give IN status.
    // Thus, we consider all selected faces as IN without real classification.
    for (k = 0; k < aNbFP; ++k)
      myInFaces.Append(aVShapeBox(aIVec(k)).Shape());

    return;
  }

  // Prepare EF map of faces to process for building connexity blocks
  TopTools_IndexedDataMapOfShapeListOfShape aMEFP(1, anAlloc);
  if (aNbFP > 1)
  {
    for (k = 0; k < aNbFP; ++k)
      MapEdgesAndFaces(aVShapeBox(aIVec(k)).Shape(), aMEFP, anAlloc);
  }

  aPSOuter.Next();

  // Map of Edge-Face connection, necessary for solid classification.
  // It will be filled when first classification is performed.
  TopTools_IndexedDataMapOfShapeListOfShape aMEFDS(1, anAlloc);

  // Fence map to avoid processing of the same faces twice
  TopTools_MapOfShape aMFDone(1, anAlloc);

  Message_ProgressScope aPSLoop(aPSOuter.Next(), NULL, aNbFP);
  for (k = 0; k < aNbFP; ++k, aPSLoop.Next())
  {
    if (UserBreak(aPSLoop))
    {
      return;
    }
    Standard_Integer   nFP = aIVec(k);
    const TopoFace& aFP = (*(TopoFace*)&aVShapeBox(nFP).Shape());
    if (!aMFDone.Add(aFP))
      continue;

    // Make connexity blocks of faces, avoiding passing through the
    // borders of the solid. It helps to reduce significantly the
    // number of classified faces.
    ShapeList aLCBF(anAlloc);
    // The most appropriate face for classification
    TopoFace aFaceToClassify;
    MakeConnexityBlock(aFP, aMSE, aMEFP, aMFDone, aLCBF, aFaceToClassify);

    if (!myBoxS.IsWhole())
    {
      // First, try fast classification of the whole block by additional
      // check on bounding boxes - check that bounding boxes of all vertices
      // of the block interfere with the box of the solid.
      // If not, the faces are out.
      Standard_Boolean bOut = Standard_False;
      aItLS.Initialize(aLCBF);
      for (; aItLS.More() && !bOut; aItLS.Next())
      {
        ShapeExplorer anExpV(aItLS.Value(), TopAbs_VERTEX);
        for (; anExpV.More() && !bOut; anExpV.Next())
        {
          const TopoVertex& aV = TopoDS::Vertex(anExpV.Current());
          Box2              aBBV;
          aBBV.Add(BRepInspector::Pnt(aV));
          aBBV.SetGap(BRepInspector::Tolerance(aV));
          bOut = myBoxS.IsOut(aBBV);
        }
      }
      if (bOut)
        continue;
    }

    if (aFaceToClassify.IsNull())
      aFaceToClassify = aFP;

    if (aMEFDS.IsEmpty())
      // Fill EF map for Solid
      TopExp1::MapShapesAndAncestors(mySolid, TopAbs_EDGE, TopAbs_FACE, aMEFDS);

    // All vertices are interfere with the solids box, run classification.
    Standard_Boolean bIsIN = AlgoTools::IsInternalFace(aFaceToClassify,
                                                                mySolid,
                                                                aMEFDS,
                                                                Precision1::Confusion(),
                                                                myContext);
    if (bIsIN)
    {
      aItLS.Initialize(aLCBF);
      for (; aItLS.More(); aItLS.Next())
        myInFaces.Append(aItLS.Value());
    }
  }
}

//=================================================================================================

void BOPAlgo_FillIn3DParts::MapEdgesAndFaces(const TopoShape&                        theF,
                                             TopTools_IndexedDataMapOfShapeListOfShape& theEFMap,
                                             const Handle(NCollection_BaseAllocator)& theAllocator)
{
  myItF.Initialize(theF);
  for (; myItF.More(); myItF.Next())
  {
    const TopoShape& aW = myItF.Value();
    if (aW.ShapeType() != TopAbs_WIRE)
      continue;

    myItW.Initialize(aW);
    for (; myItW.More(); myItW.Next())
    {
      const TopoShape& aE = myItW.Value();

      ShapeList* pLF = theEFMap.ChangeSeek(aE);
      if (!pLF)
        pLF = &theEFMap(theEFMap.Add(aE, ShapeList(theAllocator)));
      pLF->Append(theF);
    }
  }
}

//=================================================================================================

void BOPAlgo_FillIn3DParts::MakeConnexityBlock(
  const TopoFace&                               theFStart,
  const TopTools_IndexedMapOfShape&                theMEAvoid,
  const TopTools_IndexedDataMapOfShapeListOfShape& theEFMap,
  TopTools_MapOfShape&                             theMFDone,
  ShapeList&                            theLCB,
  TopoFace&                                     theFaceToClassify)
{
  // Add start element
  theLCB.Append(theFStart);
  if (theEFMap.IsEmpty())
    return;

  TopTools_ListIteratorOfListOfShape aItCB(theLCB);
  for (; aItCB.More(); aItCB.Next())
  {
    const TopoShape& aF = aItCB.Value();
    myItF.Initialize(aF);
    for (; myItF.More(); myItF.Next())
    {
      const TopoShape& aW = myItF.Value();
      if (aW.ShapeType() != TopAbs_WIRE)
        continue;

      myItW.Initialize(aW);
      for (; myItW.More(); myItW.Next())
      {
        const TopoEdge& aE = TopoDS::Edge(myItW.Value());
        if (theMEAvoid.Contains(aE) || BRepInspector::Degenerated(aE))
        {
          if (theFaceToClassify.IsNull())
            theFaceToClassify = TopoDS::Face(aF);
          continue;
        }

        const ShapeList* pLF = theEFMap.Seek(aE);
        if (!pLF)
          continue;
        TopTools_ListIteratorOfListOfShape aItLF(*pLF);
        for (; aItLF.More(); aItLF.Next())
        {
          const TopoShape& aFToAdd = aItLF.Value();
          if (theMFDone.Add(aFToAdd))
            theLCB.Append(aFToAdd);
        }
      }
    }
  }
}

// Vector of solid classifiers
typedef NCollection_Vector<BOPAlgo_FillIn3DParts> BOPAlgo_VectorOfFillIn3DParts;

//=================================================================================================

void BooleanTools::ClassifyFaces(const ShapeList&                theFaces,
                                  const ShapeList&                theSolids,
                                  const Standard_Boolean                     theRunParallel,
                                  Handle(IntTools_Context)&                  theContext,
                                  TopTools_IndexedDataMapOfShapeListOfShape& theInParts,
                                  const TopTools_DataMapOfShapeBox&          theShapeBoxMap,
                                  const TopTools_DataMapOfShapeListOfShape&  theSolidsIF,
                                  const Message_ProgressRange&               theRange)
{
  Handle(NCollection_BaseAllocator) anAlloc = new NCollection_IncAllocator;

  Message_ProgressScope aPSOuter(theRange, NULL, 10);

  // Fill the vector of shape box with faces and its bounding boxes
  BOPAlgo_VectorOfShapeBox aVSB(256, anAlloc);

  TopTools_ListIteratorOfListOfShape aItLF(theFaces);
  for (; aItLF.More(); aItLF.Next())
  {
    if (!aPSOuter.More())
    {
      return;
    }

    const TopoShape& aF = aItLF.Value();
    // Append face to the vector of shape box
    BOPAlgo_ShapeBox& aSB = aVSB.Appended();
    aSB.SetShape(aF);

    // Get bounding box for the face
    const Box2* pBox = theShapeBoxMap.Seek(aF);
    if (pBox)
      aSB.SetBox(*pBox);
    else
    {
      // Build the bounding box
      Box2 aBox;
      BRepBndLib1::Add(aF, aBox);
      aSB.SetBox(aBox);
    }
  }

  // Prepare BVH tree of bounding boxes of the faces to classify
  // taking the bounding boxes from the just prepared vector
  BOPTools_BoxTree aBBTree;
  Standard_Integer aNbF = aVSB.Length();
  aBBTree.SetSize(aNbF);
  for (Standard_Integer i = 0; i < aNbF; ++i)
  {
    aBBTree.Add(i, Tools5::Bnd2BVH(aVSB(i).Box1()));
  }

  // Shake tree filler
  aBBTree.Build();

  // Prepare vector of solids to classify
  BOPAlgo_VectorOfFillIn3DParts aVFIP;

  TopTools_ListIteratorOfListOfShape aItLS(theSolids);
  for (; aItLS.More(); aItLS.Next())
  {
    const TopoSolid& aSolid = TopoDS::Solid(aItLS.Value());
    // Append solid to the vector
    BOPAlgo_FillIn3DParts& aFIP = aVFIP.Appended();
    aFIP.SetSolid(aSolid);

    // Get bounding box for the solid
    const Box2* pBox = theShapeBoxMap.Seek(aSolid);
    if (pBox)
      aFIP.SetBoxS(*pBox);
    else
    {
      // Build the bounding box
      Box2 aBox;
      BRepBndLib1::Add(aSolid, aBox);
      if (!aBox.IsWhole())
      {
        if (AlgoTools::IsInvertedSolid(aSolid))
          aBox.SetWhole();
      }
      aFIP.SetBoxS(aBox);
    }

    const ShapeList* pLIF = theSolidsIF.Seek(aSolid);
    if (pLIF)
      aFIP.SetOwnIF(*pLIF);

    aFIP.SetBBTree(&aBBTree);
    aFIP.SetShapeBoxVector(aVSB);
  }

  // Close preparation task
  aPSOuter.Next();
  // Set progress range for each task to be run in parallel
  Standard_Integer      aNbS = aVFIP.Length();
  Message_ProgressScope aPSParallel(aPSOuter.Next(9),
                                    "Classification of faces relatively solids",
                                    aNbS);
  for (Standard_Integer iFS = 0; iFS < aNbS; ++iFS)
  {
    aVFIP.ChangeValue(iFS).SetProgressRange(aPSParallel.Next());
  }
  // Perform classification
  //================================================================
  BooleanParallelTools::Perform(theRunParallel, aVFIP, theContext);
  //================================================================
  // Analyze the results and fill the resulting map
  for (Standard_Integer i = 0; i < aNbS; ++i)
  {
    BOPAlgo_FillIn3DParts&      aFIP  = aVFIP(i);
    const TopoShape&         aS    = aFIP.Solid();
    const ShapeList& aLFIn = aFIP.InFaces();
    theInParts.Add(aS, aLFIn);
  }
}

//=================================================================================================

void BooleanTools::FillInternals(const ShapeList&               theSolids,
                                  const ShapeList&               theParts,
                                  const TopTools_DataMapOfShapeListOfShape& theImages,
                                  const Handle(IntTools_Context)&           theContext)
{
  if (theSolids.IsEmpty() || theParts.IsEmpty())
    return;

  // Map the solids to avoid classification of the own shapes of the solids
  TopTools_IndexedMapOfShape     aMSSolids;
  ShapeList::Iterator itLS(theSolids);
  for (; itLS.More(); itLS.Next())
  {
    const TopoShape& aSolid = itLS.Value();
    if (aSolid.ShapeType() == TopAbs_SOLID)
    {
      TopExp1::MapShapes(aSolid, TopAbs_VERTEX, aMSSolids);
      TopExp1::MapShapes(aSolid, TopAbs_EDGE, aMSSolids);
      TopExp1::MapShapes(aSolid, TopAbs_FACE, aMSSolids);
    }
  }

  // Extract BRep elements from the given parts and
  // check them for possible splits
  ShapeList           aLPartsInput = theParts, aLParts;
  ShapeList::Iterator itLP(aLPartsInput);
  for (; itLP.More(); itLP.Next())
  {
    const TopoShape& aPart = itLP.Value();
    switch (aPart.ShapeType())
    {
      case TopAbs_VERTEX:
      case TopAbs_EDGE:
      case TopAbs_FACE: {
        const ShapeList* pIm = theImages.Seek(aPart);
        if (pIm)
        {
          ShapeList::Iterator itIm(*pIm);
          for (; itIm.More(); itIm.Next())
          {
            const TopoShape& aPartIm = itIm.Value();
            if (!aMSSolids.Contains(aPartIm))
              aLParts.Append(aPartIm);
          }
        }
        else if (!aMSSolids.Contains(aPart))
          aLParts.Append(aPart);

        break;
      }
      default: {
        for (TopoDS_Iterator it(aPart); it.More(); it.Next())
          aLPartsInput.Append(it.Value());
        break;
      }
    }
  }

  // Classify the given parts relatively solids.
  // Add edges and vertices classified as IN into solids instantly,
  // and collect faces classified as IN into a list for further shell creation

  TopTools_DataMapOfShapeListOfShape anINFaces;
  itLS.Initialize(theSolids);
  for (; itLS.More(); itLS.Next())
  {
    const TopoShape& aSolid = itLS.Value();
    if (aSolid.ShapeType() != TopAbs_SOLID)
      continue;

    TopoSolid aSd = *(TopoSolid*)&aSolid;

    itLP.Initialize(aLParts);
    for (; itLP.More();)
    {
      TopoShape aPart = itLP.Value();
      TopAbs_State aState =
        AlgoTools::ComputeStateByOnePoint(aPart, aSd, Precision1::Confusion(), theContext);
      if (aState == TopAbs_IN)
      {
        if (aPart.ShapeType() == TopAbs_FACE)
        {
          ShapeList* pFaces = anINFaces.ChangeSeek(aSd);
          if (!pFaces)
            pFaces = anINFaces.Bound(aSd, ShapeList());
          pFaces->Append(aPart);
        }
        else
        {
          aPart.Orientation(TopAbs_INTERNAL);
          ShapeBuilder().Add(aSd, aPart);
        }
        aLParts.Remove(itLP);
      }
      else
        itLP.Next();
    }
  }

  // Make shells from faces and put them into solids
  TopTools_DataMapOfShapeListOfShape::Iterator itM(anINFaces);
  for (; itM.More(); itM.Next())
  {
    TopoSolid                aSd    = *(TopoSolid*)&itM.Key1();
    const ShapeList& aFaces = itM.Value();

    TopoCompound aCF;
    ShapeBuilder().MakeCompound(aCF);

    ShapeList::Iterator itLF(aFaces);
    for (; itLF.More(); itLF.Next())
      ShapeBuilder().Add(aCF, itLF.Value());

    // Build blocks from the faces
    ShapeList aLCB;
    AlgoTools::MakeConnexityBlocks(aCF, TopAbs_EDGE, TopAbs_FACE, aLCB);

    // Build shell from each block
    ShapeList::Iterator itCB(aLCB);
    for (; itCB.More(); itCB.Next())
    {
      const TopoShape& aCB = itCB.Value();

      TopoShell aShell;
      ShapeBuilder().MakeShell(aShell);
      // Add faces of the block to the shell
      ShapeExplorer expF(aCB, TopAbs_FACE);
      for (; expF.More(); expF.Next())
      {
        TopoFace aFInt = TopoDS::Face(expF.Current());
        aFInt.Orientation(TopAbs_INTERNAL);
        ShapeBuilder().Add(aShell, aFInt);
      }

      ShapeBuilder().Add(aSd, aShell);
    }
  }
}

//=================================================================================================

Standard_Boolean BooleanTools::TrsfToPoint(const Box2&      theBox1,
                                            const Box2&      theBox2,
                                            Transform3d&            theTrsf,
                                            const Point3d&       thePoint,
                                            const Standard_Real theCriteria)
{
  // Unify two boxes
  Box2 aBox = theBox1;
  aBox.Add(theBox2);

  Coords3d        aBCenter = (aBox.CornerMin().XYZ() + aBox.CornerMax().XYZ()) / 2.;
  Standard_Real aPBDist  = (thePoint.XYZ() - aBCenter).Modulus();
  if (aPBDist < theCriteria)
    return Standard_False;

  Standard_Real aBSize = Sqrt(aBox.SquareExtent());
  if ((aBSize / aPBDist) > (1. / theCriteria))
    return Standard_False;

  theTrsf.SetTranslation(Vector3d(aBox.CornerMin(), thePoint));
  return Standard_True;
}
