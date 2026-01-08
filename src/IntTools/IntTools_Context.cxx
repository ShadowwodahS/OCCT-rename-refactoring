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

#include <Bnd_Box.hxx>
#include <Bnd_OBB.hxx>
#include <BRep_Tool.hxx>
#include <BRepBndLib.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Extrema_LocateExtPC.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dHatch_Hatcher.hxx>
#include <Geom2dHatch_Intersector.hxx>
#include <Geom_BoundedCurve.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <IntTools_Context.hxx>
#include <IntTools_FClass2d.hxx>
#include <IntTools_SurfaceRangeLocalizeData.hxx>
#include <IntTools_Tools.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TopAbs_State.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(IntTools_Context, RefObject)

//
//=================================================================================================

IntTools_Context::IntTools_Context()
    : myAllocator(NCollection_BaseAllocator::CommonBaseAllocator()),
      myFClass2dMap(100, myAllocator),
      myProjPSMap(100, myAllocator),
      myProjPCMap(100, myAllocator),
      mySClassMap(100, myAllocator),
      myProjPTMap(100, myAllocator),
      myHatcherMap(100, myAllocator),
      myProjSDataMap(100, myAllocator),
      myBndBoxDataMap(100, myAllocator),
      mySurfAdaptorMap(100, myAllocator),
      myOBBMap(100, myAllocator),
      myCreateFlag(0),
      myPOnSTolerance(1.e-12)
{
}

//=================================================================================================

IntTools_Context::IntTools_Context(const Handle(NCollection_BaseAllocator)& theAllocator)
    : myAllocator(theAllocator),
      myFClass2dMap(100, myAllocator),
      myProjPSMap(100, myAllocator),
      myProjPCMap(100, myAllocator),
      mySClassMap(100, myAllocator),
      myProjPTMap(100, myAllocator),
      myHatcherMap(100, myAllocator),
      myProjSDataMap(100, myAllocator),
      myBndBoxDataMap(100, myAllocator),
      mySurfAdaptorMap(100, myAllocator),
      myOBBMap(100, myAllocator),
      myCreateFlag(1),
      myPOnSTolerance(1.e-12)
{
}

//=================================================================================================

IntTools_Context::~IntTools_Context()
{
  for (NCollection_DataMap<TopoShape, IntTools_FClass2d*, ShapeHasher>::Iterator
         anIt(myFClass2dMap);
       anIt.More();
       anIt.Next())
  {
    IntTools_FClass2d* pFClass2d = anIt.Value();
    ;
    (*pFClass2d).~IntTools_FClass2d();
    myAllocator->Free(pFClass2d);
  }
  myFClass2dMap.Clear();

  clearCachedPOnSProjectors();
  for (NCollection_DataMap<TopoShape, GeomAPI_ProjectPointOnCurve*, ShapeHasher>::
         Iterator anIt(myProjPCMap);
       anIt.More();
       anIt.Next())
  {
    GeomAPI_ProjectPointOnCurve* pProjPC = anIt.Value();
    (*pProjPC).~GeomAPI_ProjectPointOnCurve();
    myAllocator->Free(pProjPC);
  }
  myProjPCMap.Clear();

  for (NCollection_DataMap<TopoShape, BRepClass3d_SolidClassifier*, ShapeHasher>::
         Iterator anIt(mySClassMap);
       anIt.More();
       anIt.Next())
  {
    BRepClass3d_SolidClassifier* pSC = anIt.Value();
    (*pSC).~BRepClass3d_SolidClassifier();
    myAllocator->Free(pSC);
  }
  mySClassMap.Clear();

  for (NCollection_DataMap<Handle(GeomCurve3d), GeomAPI_ProjectPointOnCurve*>::Iterator anIt(
         myProjPTMap);
       anIt.More();
       anIt.Next())
  {
    GeomAPI_ProjectPointOnCurve* pProjPT = anIt.Value();
    (*pProjPT).~GeomAPI_ProjectPointOnCurve();
    myAllocator->Free(pProjPT);
  }
  myProjPTMap.Clear();

  for (NCollection_DataMap<TopoShape, Geom2dHatch_Hatcher*, ShapeHasher>::Iterator
         anIt(myHatcherMap);
       anIt.More();
       anIt.Next())
  {
    Geom2dHatch_Hatcher* pHatcher = anIt.Value();
    (*pHatcher).~Geom2dHatch_Hatcher();
    myAllocator->Free(pHatcher);
  }
  myHatcherMap.Clear();

  for (NCollection_DataMap<TopoShape,
                           IntTools_SurfaceRangeLocalizeData*,
                           ShapeHasher>::Iterator anIt(myProjSDataMap);
       anIt.More();
       anIt.Next())
  {
    IntTools_SurfaceRangeLocalizeData* pSData = anIt.Value();
    (*pSData).~IntTools_SurfaceRangeLocalizeData();
    myAllocator->Free(pSData);
  }
  myProjSDataMap.Clear();

  for (NCollection_DataMap<TopoShape, Box2*, ShapeHasher>::Iterator anIt(
         myBndBoxDataMap);
       anIt.More();
       anIt.Next())
  {
    Box2* pBox = anIt.Value();
    (*pBox).~Box2();
    myAllocator->Free(pBox);
  }
  myBndBoxDataMap.Clear();

  for (NCollection_DataMap<TopoShape, BRepAdaptor_Surface*, ShapeHasher>::Iterator
         anIt(mySurfAdaptorMap);
       anIt.More();
       anIt.Next())
  {
    BRepAdaptor_Surface* pSurfAdaptor = anIt.Value();
    (*pSurfAdaptor).~BRepAdaptor_Surface();
    myAllocator->Free(pSurfAdaptor);
  }
  mySurfAdaptorMap.Clear();

  for (NCollection_DataMap<TopoShape, OrientedBox*, ShapeHasher>::Iterator anIt(
         myOBBMap);
       anIt.More();
       anIt.Next())
  {
    OrientedBox* pOBB = anIt.Value();
    (*pOBB).~OrientedBox();
    myAllocator->Free(pOBB);
  }
  myOBBMap.Clear();
}

//=================================================================================================

Box2& IntTools_Context::BndBox(const TopoShape& aS)
{
  Box2* pBox = NULL;
  if (!myBndBoxDataMap.Find(aS, pBox))
  {
    //
    pBox = (Box2*)myAllocator->Allocate(sizeof(Box2));
    new (pBox) Box2();
    //
    Box2& aBox = *pBox;
    BRepBndLib1::Add(aS, aBox);
    //
    myBndBoxDataMap.Bind(aS, pBox);
  }
  return *pBox;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsInfiniteFace(const TopoFace& aFace)
{
  const Box2& aBox = BndBox(aFace);
  return aBox.IsOpenXmax() || aBox.IsOpenXmin() || aBox.IsOpenYmax() || aBox.IsOpenYmin()
         || aBox.IsOpenZmax() || aBox.IsOpenZmin();
}

//=================================================================================================

IntTools_FClass2d& IntTools_Context::FClass2d(const TopoFace& aF)
{
  IntTools_FClass2d* pFClass2d = NULL;
  if (!myFClass2dMap.Find(aF, pFClass2d))
  {
    Standard_Real aTolF;
    TopoFace   aFF;
    //
    aFF = aF;
    aFF.Orientation(TopAbs_FORWARD);
    aTolF = BRepInspector::Tolerance(aFF);
    //
    pFClass2d = (IntTools_FClass2d*)myAllocator->Allocate(sizeof(IntTools_FClass2d));
    new (pFClass2d) IntTools_FClass2d(aFF, aTolF);
    //
    myFClass2dMap.Bind(aFF, pFClass2d);
  }
  return *pFClass2d;
}

//=================================================================================================

PointOnSurfProjector& IntTools_Context::ProjPS(const TopoFace& aF)
{
  PointOnSurfProjector* pProjPS = NULL;
  if (!myProjPSMap.Find(aF, pProjPS))
  {
    Standard_Real Umin, Usup, Vmin, Vsup;
    UVBounds(aF, Umin, Usup, Vmin, Vsup);
    const Handle(GeomSurface)& aS = BRepInspector::Surface(aF);
    //
    pProjPS =
      (PointOnSurfProjector*)myAllocator->Allocate(sizeof(PointOnSurfProjector));
    new (pProjPS) PointOnSurfProjector();
    pProjPS->Init(aS, Umin, Usup, Vmin, Vsup, myPOnSTolerance);
    pProjPS->SetExtremaFlag(Extrema_ExtFlag_MIN); ///
    //
    myProjPSMap.Bind(aF, pProjPS);
  }
  return *pProjPS;
}

//=================================================================================================

GeomAPI_ProjectPointOnCurve& IntTools_Context::ProjPC(const TopoEdge& aE)
{
  GeomAPI_ProjectPointOnCurve* pProjPC = NULL;
  if (!myProjPCMap.Find(aE, pProjPC))
  {
    Standard_Real f, l;
    //
    Handle(GeomCurve3d) aC3D = BRepInspector::Curve(aE, f, l);
    //
    pProjPC =
      (GeomAPI_ProjectPointOnCurve*)myAllocator->Allocate(sizeof(GeomAPI_ProjectPointOnCurve));
    new (pProjPC) GeomAPI_ProjectPointOnCurve();
    pProjPC->Init(aC3D, f, l);
    //
    myProjPCMap.Bind(aE, pProjPC);
  }
  return *pProjPC;
}

//=================================================================================================

GeomAPI_ProjectPointOnCurve& IntTools_Context::ProjPT(const Handle(GeomCurve3d)& aC3D)

{
  GeomAPI_ProjectPointOnCurve* pProjPT = NULL;
  if (!myProjPTMap.Find(aC3D, pProjPT))
  {
    Standard_Real f, l;
    f = aC3D->FirstParameter();
    l = aC3D->LastParameter();
    //
    pProjPT =
      (GeomAPI_ProjectPointOnCurve*)myAllocator->Allocate(sizeof(GeomAPI_ProjectPointOnCurve));
    new (pProjPT) GeomAPI_ProjectPointOnCurve();
    pProjPT->Init(aC3D, f, l);
    //
    myProjPTMap.Bind(aC3D, pProjPT);
  }
  return *pProjPT;
}

//=================================================================================================

BRepClass3d_SolidClassifier& IntTools_Context::SolidClassifier(const TopoSolid& aSolid)
{
  BRepClass3d_SolidClassifier* pSC = NULL;
  if (!mySClassMap.Find(aSolid, pSC))
  {
    pSC = (BRepClass3d_SolidClassifier*)myAllocator->Allocate(sizeof(BRepClass3d_SolidClassifier));
    new (pSC) BRepClass3d_SolidClassifier(aSolid);
    //
    mySClassMap.Bind(aSolid, pSC);
  }
  return *pSC;
}

//=================================================================================================

BRepAdaptor_Surface& IntTools_Context::SurfaceAdaptor(const TopoFace& theFace)
{
  BRepAdaptor_Surface* pBAS = NULL;
  if (!mySurfAdaptorMap.Find(theFace, pBAS))
  {
    //
    pBAS = (BRepAdaptor_Surface*)myAllocator->Allocate(sizeof(BRepAdaptor_Surface));
    new (pBAS) BRepAdaptor_Surface(theFace, Standard_True);
    //
    mySurfAdaptorMap.Bind(theFace, pBAS);
  }
  return *pBAS;
}

//=================================================================================================

Geom2dHatch_Hatcher& IntTools_Context::Hatcher(const TopoFace& aF)
{
  Geom2dHatch_Hatcher* pHatcher = NULL;
  if (!myHatcherMap.Find(aF, pHatcher))
  {
    Standard_Real               aTolArcIntr, aTolTangfIntr, aTolHatch2D, aTolHatch3D;
    Standard_Real               aU1, aU2, aEpsT;
    TopAbs_Orientation          aOrE;
    Handle(GeomSurface)        aS;
    Handle(GeomCurve2d)        aC2D;
    Handle(Geom2d_TrimmedCurve) aCT2D;
    TopoFace                 aFF;
    ShapeExplorer             aExp;
    //
    aTolHatch2D   = 1.e-8;
    aTolHatch3D   = 1.e-8;
    aTolArcIntr   = 1.e-10;
    aTolTangfIntr = 1.e-10;
    aEpsT         = Precision::PConfusion();
    //
    Geom2dHatch_Intersector aIntr(aTolArcIntr, aTolTangfIntr);
    pHatcher = (Geom2dHatch_Hatcher*)myAllocator->Allocate(sizeof(Geom2dHatch_Hatcher));
    new (pHatcher)
      Geom2dHatch_Hatcher(aIntr, aTolHatch2D, aTolHatch3D, Standard_True, Standard_False);
    //
    aFF = aF;
    aFF.Orientation(TopAbs_FORWARD);
    aS = BRepInspector::Surface(aFF);

    aExp.Init(aFF, TopAbs_EDGE);
    for (; aExp.More(); aExp.Next())
    {
      const TopoEdge& aE = *((TopoEdge*)&aExp.Current());
      aOrE                  = aE.Orientation();
      //
      aC2D = BRepInspector::CurveOnSurface(aE, aFF, aU1, aU2);
      if (aC2D.IsNull())
      {
        continue;
      }
      if (fabs(aU1 - aU2) < aEpsT)
      {
        continue;
      }
      //
      aCT2D = new Geom2d_TrimmedCurve(aC2D, aU1, aU2);
      Geom2dAdaptor_Curve aGAC(aCT2D);
      pHatcher->AddElement(aGAC, aOrE);
    } // for (; aExp.More() ; aExp.Next()) {
    //
    myHatcherMap.Bind(aFF, pHatcher);
  }
  return *pHatcher;
}

//=================================================================================================

OrientedBox& IntTools_Context::OBB(const TopoShape& aS, const Standard_Real theGap)
{
  OrientedBox* pBox = NULL;
  if (!myOBBMap.Find(aS, pBox))
  {
    pBox = (OrientedBox*)myAllocator->Allocate(sizeof(OrientedBox));
    new (pBox) OrientedBox();
    //
    OrientedBox& aBox = *pBox;
    BRepBndLib1::AddOBB(aS, aBox);
    aBox.Enlarge(theGap);
    //
    myOBBMap.Bind(aS, pBox);
  }
  return *pBox;
}

//=================================================================================================

IntTools_SurfaceRangeLocalizeData& IntTools_Context::SurfaceData(const TopoFace& aF)
{
  IntTools_SurfaceRangeLocalizeData* pSData = NULL;
  if (!myProjSDataMap.Find(aF, pSData))
  {
    pSData = (IntTools_SurfaceRangeLocalizeData*)myAllocator->Allocate(
      sizeof(IntTools_SurfaceRangeLocalizeData));
    new (pSData) IntTools_SurfaceRangeLocalizeData(3,
                                                   3,
                                                   10. * Precision::PConfusion(),
                                                   10. * Precision::PConfusion());
    //
    myProjSDataMap.Bind(aF, pSData);
  }
  return *pSData;
}

//=================================================================================================

Standard_Integer IntTools_Context::ComputePE(const Point3d&       aP1,
                                             const Standard_Real aTolP1,
                                             const TopoEdge&  aE2,
                                             Standard_Real&      aT,
                                             Standard_Real&      aDist)
{
  if (!BRepInspector::IsGeometric(aE2))
  {
    return -2;
  }
  Standard_Real    aTolE2, aTolSum;
  Standard_Integer aNbProj;
  //
  GeomAPI_ProjectPointOnCurve& aProjector = ProjPC(aE2);
  aProjector.Perform(aP1);

  aNbProj = aProjector.NbPoints();
  if (aNbProj)
  {
    // point falls on the curve
    aDist = aProjector.LowerDistance();
    //
    aTolE2  = BRepInspector::Tolerance(aE2);
    aTolSum = aTolP1 + aTolE2 + Precision::Confusion();
    //
    aT = aProjector.LowerDistanceParameter();
    if (aDist > aTolSum)
    {
      return -4;
    }
  }
  else
  {
    // point falls out of the curve, check distance to vertices
    TopoEdge     aEFwd = TopoDS::Edge(aE2.Oriented(TopAbs_FORWARD));
    TopoDS_Iterator itV(aEFwd);
    aDist = RealLast();
    for (; itV.More(); itV.Next())
    {
      const TopoVertex& aV = TopoDS::Vertex(itV.Value());
      if (aV.Orientation() == TopAbs_FORWARD || aV.Orientation() == TopAbs_REVERSED)
      {
        Point3d aPV           = BRepInspector::Pnt(aV);
        aTolSum              = aTolP1 + BRepInspector::Tolerance(aV) + Precision::Confusion();
        Standard_Real aDist1 = aP1.Distance(aPV);
        if (aDist1 < aDist && aDist1 < aTolSum)
        {
          aDist = aDist1;
          aT    = BRepInspector::Parameter(aV, aEFwd);
        }
      }
    }
    if (Precision::IsInfinite(aDist))
    {
      return -3;
    }
  }
  return 0;
}

//=================================================================================================

Standard_Integer IntTools_Context::ComputeVE(const TopoVertex& theV,
                                             const TopoEdge&   theE,
                                             Standard_Real&       theT,
                                             Standard_Real&       theTol,
                                             const Standard_Real  theFuzz)
{
  if (BRepInspector::Degenerated(theE))
  {
    return -1;
  }
  if (!BRepInspector::IsGeometric(theE))
  {
    return -2;
  }
  Standard_Real    aDist, aTolV, aTolE, aTolSum;
  Standard_Integer aNbProj;
  Point3d           aP;
  //
  aP = BRepInspector::Pnt(theV);
  //
  GeomAPI_ProjectPointOnCurve& aProjector = ProjPC(theE);
  aProjector.Perform(aP);

  aNbProj = aProjector.NbPoints();
  if (!aNbProj)
  {
    return -3;
  }
  //
  aDist = aProjector.LowerDistance();
  //
  aTolV   = BRepInspector::Tolerance(theV);
  aTolE   = BRepInspector::Tolerance(theE);
  aTolSum = aTolV + aTolE + Max(theFuzz, Precision::Confusion());
  //
  theTol = aDist + aTolE;
  theT   = aProjector.LowerDistanceParameter();
  if (aDist > aTolSum)
  {
    return -4;
  }
  return 0;
}

//=================================================================================================

Standard_Integer IntTools_Context::ComputeVF(const TopoVertex& theVertex,
                                             const TopoFace&   theFace,
                                             Standard_Real&       theU,
                                             Standard_Real&       theV,
                                             Standard_Real&       theTol,
                                             const Standard_Real  theFuzz)
{
  Standard_Real aTolV, aTolF, aTolSum, aDist;
  Point3d        aP;

  aP = BRepInspector::Pnt(theVertex);
  //
  // 1. Check if the point is projectable on the surface
  PointOnSurfProjector& aProjector = ProjPS(theFace);
  aProjector.Perform(aP);
  //
  if (!aProjector.IsDone())
  { // the point is not  projectable on the surface
    return -1;
  }
  //
  // 2. Check the distance between the projection point and
  //    the original point
  aDist = aProjector.LowerDistance();
  //
  aTolV = BRepInspector::Tolerance(theVertex);
  aTolF = BRepInspector::Tolerance(theFace);
  //
  aTolSum = aTolV + aTolF + Max(theFuzz, Precision::Confusion());
  theTol  = aDist + aTolF;
  aProjector.LowerDistanceParameters(theU, theV);
  //
  if (aDist > aTolSum)
  {
    // the distance is too large
    return -2;
  }
  //
  gp_Pnt2d         aP2d(theU, theV);
  Standard_Boolean pri = IsPointInFace(theFace, aP2d);
  if (!pri)
  { //  the point lays on the surface but out of the face
    return -3;
  }
  return 0;
}

//=================================================================================================

TopAbs_State IntTools_Context::StatePointFace(const TopoFace& aF, const gp_Pnt2d& aP2d)
{
  TopAbs_State       aState;
  IntTools_FClass2d& aClass2d = FClass2d(aF);
  aState                      = aClass2d.Perform(aP2d);
  return aState;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsPointInFace(const TopoFace& aF, const gp_Pnt2d& aP2d)
{
  TopAbs_State aState = StatePointFace(aF, aP2d);
  if (aState == TopAbs_OUT || aState == TopAbs_ON)
  {
    return Standard_False;
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsPointInFace(const Point3d&       aP,
                                                 const TopoFace&  aF,
                                                 const Standard_Real aTol)
{
  Standard_Boolean bIn = Standard_False;
  Standard_Real    aDist;
  //
  PointOnSurfProjector& aProjector = ProjPS(aF);
  aProjector.Perform(aP);
  //
  Standard_Boolean bDone = aProjector.IsDone();
  if (bDone)
  {
    aDist = aProjector.LowerDistance();
    if (aDist < aTol)
    {
      Standard_Real U, V;
      //
      aProjector.LowerDistanceParameters(U, V);
      gp_Pnt2d aP2D(U, V);
      bIn = IsPointInFace(aF, aP2D);
    }
  }
  //
  return bIn;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsPointInOnFace(const TopoFace& aF, const gp_Pnt2d& aP2d)
{
  TopAbs_State aState = StatePointFace(aF, aP2d);
  if (aState == TopAbs_OUT)
  {
    return Standard_False;
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsValidPointForFace(const Point3d&       aP,
                                                       const TopoFace&  aF,
                                                       const Standard_Real aTol)
{
  Standard_Boolean bFlag;
  Standard_Real    Umin, U, V;

  PointOnSurfProjector& aProjector = ProjPS(aF);
  aProjector.Perform(aP);

  bFlag = aProjector.IsDone();
  if (bFlag)
  {

    Umin = aProjector.LowerDistance();
    // if (Umin > 1.e-3) { // it was
    if (Umin > aTol)
    {
      return !bFlag;
    }
    //
    aProjector.LowerDistanceParameters(U, V);
    gp_Pnt2d aP2D(U, V);
    bFlag = IsPointInOnFace(aF, aP2D);
  }
  return bFlag;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsValidPointForFaces(const Point3d&       aP,
                                                        const TopoFace&  aF1,
                                                        const TopoFace&  aF2,
                                                        const Standard_Real aTol)
{
  Standard_Boolean bFlag1, bFlag2;

  bFlag1 = IsValidPointForFace(aP, aF1, aTol);
  if (!bFlag1)
  {
    return bFlag1;
  }
  bFlag2 = IsValidPointForFace(aP, aF2, aTol);
  return bFlag2;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsValidBlockForFace(const Standard_Real   aT1,
                                                       const Standard_Real   aT2,
                                                       const IntTools_Curve& aC,
                                                       const TopoFace&    aF,
                                                       const Standard_Real   aTol)
{
  Standard_Boolean bFlag;
  Standard_Real    aTInterm;
  Point3d           aPInterm;

  aTInterm = Tools2::IntermediatePoint(aT1, aT2);

  const Handle(GeomCurve3d)& aC3D = aC.Curve();
  // point 3D
  aC3D->D0(aTInterm, aPInterm);
  //
  bFlag = IsValidPointForFace(aPInterm, aF, aTol);
  return bFlag;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsValidBlockForFaces(const Standard_Real   theT1,
                                                        const Standard_Real   theT2,
                                                        const IntTools_Curve& theC,
                                                        const TopoFace&    theF1,
                                                        const TopoFace&    theF2,
                                                        const Standard_Real   theTol)
{
  const Standard_Integer      aNbElem = 2;
  const Handle(GeomCurve2d)& aPC1    = theC.FirstCurve2d();
  const Handle(GeomCurve2d)& aPC2    = theC.SecondCurve2d();
  const Handle(GeomCurve3d)&   aC3D    = theC.Curve();

  const Handle(GeomCurve2d)* anArrPC[aNbElem] = {&aPC1, &aPC2};
  const TopoFace*          anArrF[aNbElem]  = {&theF1, &theF2};

  const Standard_Real aMidPar = Tools2::IntermediatePoint(theT1, theT2);
  const Point3d        aP(aC3D->Value(aMidPar));

  Standard_Boolean bFlag = Standard_True;
  gp_Pnt2d         aPnt2D;

  for (Standard_Integer i = 0; (i < 2) && bFlag; ++i)
  {
    const Handle(GeomCurve2d)& aPC = *anArrPC[i];
    const TopoFace&          aF  = *anArrF[i];

    if (!aPC.IsNull())
    {
      aPC->D0(aMidPar, aPnt2D);
      bFlag = IsPointInOnFace(aF, aPnt2D);
    }
    else
    {
      bFlag = IsValidPointForFace(aP, aF, theTol);
    }
  }

  return bFlag;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsVertexOnLine(const TopoVertex&  aV,
                                                  const IntTools_Curve& aC,
                                                  const Standard_Real   aTolC,
                                                  Standard_Real&        aT)
{
  Standard_Boolean bRet;
  Standard_Real    aTolV;
  //
  aTolV = BRepInspector::Tolerance(aV);
  bRet  = IntTools_Context::IsVertexOnLine(aV, aTolV, aC, aTolC, aT);
  //
  return bRet;
}

//=================================================================================================

Standard_Boolean IntTools_Context::IsVertexOnLine(const TopoVertex&  aV,
                                                  const Standard_Real   aTolV,
                                                  const IntTools_Curve& aC,
                                                  const Standard_Real   aTolC,
                                                  Standard_Real&        aT)
{
  Standard_Real    aFirst, aLast, aDist, aTolSum;
  Standard_Integer aNbProj;
  Point3d           aPv;

  aPv = BRepInspector::Pnt(aV);

  const Handle(GeomCurve3d)& aC3D = aC.Curve();

  aTolSum = aTolV + aTolC;
  //
  GeomAdaptor_Curve aGAC(aC3D);
  GeomAbs_CurveType aType = aGAC.GetType();
  if (aType == GeomAbs_BSplineCurve || aType == GeomAbs_BezierCurve)
  {
    aTolSum = 2. * aTolSum;
    if (aTolSum < 1.e-5)
    {
      aTolSum = 1.e-5;
    }
  }
  else
  {
    aTolSum = 2. * aTolSum; // xft
    if (aTolSum < 1.e-6)
      aTolSum = 1.e-6;
  }
  //
  aFirst = aC3D->FirstParameter();
  aLast  = aC3D->LastParameter();
  //
  // Checking extremities first
  // It is necessary to chose the closest bound to the point
  Standard_Boolean bFirstValid = Standard_False;
  Standard_Real    aFirstDist  = Precision::Infinite();
  //
  if (!Precision::IsInfinite(aFirst))
  {
    Point3d aPCFirst = aC3D->Value(aFirst);
    aFirstDist      = aPv.Distance(aPCFirst);
    if (aFirstDist < aTolSum)
    {
      bFirstValid = Standard_True;
      aT          = aFirst;
      //
      if (aFirstDist > aTolV)
      {
        Extrema_LocateExtPC anExt(aPv, aGAC, aFirst, 1.e-10);

        if (anExt.IsDone())
        {
          PointOnCurve1 aPOncurve = anExt.Point();
          aT                        = aPOncurve.Parameter();

          if ((aT > (aLast + aFirst) * 0.5) || (aPv.Distance(aPOncurve.Value()) > aTolSum)
              || (aPCFirst.Distance(aPOncurve.Value()) < Precision::Confusion()))
            aT = aFirst;
        }
        else
        {
          // Local search may fail. Try to use more precise algo.
          Extrema_ExtPC    anExt2(aPv, aGAC, 1.e-10);
          Standard_Real    aMinDist = RealLast();
          Standard_Integer aMinIdx  = -1;
          if (anExt2.IsDone())
          {
            for (Standard_Integer anIdx = 1; anIdx <= anExt2.NbExt(); anIdx++)
            {
              if (anExt2.IsMin(anIdx) && anExt2.SquareDistance(anIdx) < aMinDist)
              {
                aMinDist = anExt2.SquareDistance(anIdx);
                aMinIdx  = anIdx;
              }
            }
          }
          if (aMinIdx != -1)
          {
            const PointOnCurve1& aPOncurve = anExt2.Point(aMinIdx);
            aT                               = aPOncurve.Parameter();

            if ((aT > (aLast + aFirst) * 0.5) || (aPv.Distance(aPOncurve.Value()) > aTolSum)
                || (aPCFirst.Distance(aPOncurve.Value()) < Precision::Confusion()))
              aT = aFirst;
          }
        }
      }
    }
  }
  //
  if (!Precision::IsInfinite(aLast))
  {
    Point3d aPCLast = aC3D->Value(aLast);
    aDist          = aPv.Distance(aPCLast);
    if (bFirstValid && (aFirstDist < aDist))
    {
      return Standard_True;
    }
    //
    if (aDist < aTolSum)
    {
      aT = aLast;
      //
      if (aDist > aTolV)
      {
        Extrema_LocateExtPC anExt(aPv, aGAC, aLast, 1.e-10);

        if (anExt.IsDone())
        {
          PointOnCurve1 aPOncurve = anExt.Point();
          aT                        = aPOncurve.Parameter();

          if ((aT < (aLast + aFirst) * 0.5) || (aPv.Distance(aPOncurve.Value()) > aTolSum)
              || (aPCLast.Distance(aPOncurve.Value()) < Precision::Confusion()))
            aT = aLast;
        }
        else
        {
          // Local search may fail. Try to use more precise algo.
          Extrema_ExtPC    anExt2(aPv, aGAC, 1.e-10);
          Standard_Real    aMinDist = RealLast();
          Standard_Integer aMinIdx  = -1;
          if (anExt2.IsDone())
          {
            for (Standard_Integer anIdx = 1; anIdx <= anExt2.NbExt(); anIdx++)
            {
              if (anExt2.IsMin(anIdx) && anExt2.SquareDistance(anIdx) < aMinDist)
              {
                aMinDist = anExt2.SquareDistance(anIdx);
                aMinIdx  = anIdx;
              }
            }
          }
          if (aMinIdx != -1)
          {
            const PointOnCurve1& aPOncurve = anExt2.Point(aMinIdx);
            aT                               = aPOncurve.Parameter();

            if ((aT < (aLast + aFirst) * 0.5) || (aPv.Distance(aPOncurve.Value()) > aTolSum)
                || (aPCLast.Distance(aPOncurve.Value()) < Precision::Confusion()))
              aT = aLast;
          }
        }
      }
      //
      return Standard_True;
    }
  }
  else if (bFirstValid)
  {
    return Standard_True;
  }
  //
  GeomAPI_ProjectPointOnCurve& aProjector = ProjPT(aC3D);
  aProjector.Perform(aPv);

  aNbProj = aProjector.NbPoints();
  if (!aNbProj)
  {
    Handle(Geom_BoundedCurve) aBC = Handle(Geom_BoundedCurve)::DownCast(aC3D);
    if (!aBC.IsNull())
    {
      Point3d aPStart = aBC->StartPoint();
      Point3d aPEnd   = aBC->EndPoint();

      aDist = aPv.Distance(aPStart);
      if (aDist < aTolSum)
      {
        aT = aFirst;
        return Standard_True;
      }

      aDist = aPv.Distance(aPEnd);
      if (aDist < aTolSum)
      {
        aT = aLast;
        return Standard_True;
      }
    }

    return Standard_False;
  }

  aDist = aProjector.LowerDistance();

  if (aDist > aTolSum)
  {
    return Standard_False;
  }

  aT = aProjector.LowerDistanceParameter();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean IntTools_Context::ProjectPointOnEdge(const Point3d&      aP,
                                                      const TopoEdge& anEdge,
                                                      Standard_Real&     aT)
{
  Standard_Integer aNbPoints;

  GeomAPI_ProjectPointOnCurve& aProjector = ProjPC(anEdge);
  aProjector.Perform(aP);

  aNbPoints = aProjector.NbPoints();
  if (aNbPoints)
  {
    aT = aProjector.LowerDistanceParameter();
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

void IntTools_Context::SetPOnSProjectionTolerance(const Standard_Real theValue)
{
  myPOnSTolerance = theValue;
  clearCachedPOnSProjectors();
}

//=================================================================================================

void IntTools_Context::clearCachedPOnSProjectors()
{
  for (NCollection_DataMap<TopoShape, PointOnSurfProjector*, ShapeHasher>::
         Iterator aIt(myProjPSMap);
       aIt.More();
       aIt.Next())
  {
    PointOnSurfProjector* pProjPS = aIt.Value();
    (*pProjPS).~PointOnSurfProjector();
    myAllocator->Free(pProjPS);
  }
  myProjPSMap.Clear();
}

//=================================================================================================

void IntTools_Context::UVBounds(const TopoFace& theFace,
                                Standard_Real&     UMin,
                                Standard_Real&     UMax,
                                Standard_Real&     VMin,
                                Standard_Real&     VMax)
{
  const BRepAdaptor_Surface& aBAS = SurfaceAdaptor(theFace);
  UMin                            = aBAS.FirstUParameter();
  UMax                            = aBAS.LastUParameter();
  VMin                            = aBAS.FirstVParameter();
  VMax                            = aBAS.LastVParameter();
}
