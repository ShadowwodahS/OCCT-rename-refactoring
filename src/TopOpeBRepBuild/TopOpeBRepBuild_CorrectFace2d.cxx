// Created on: 2000-01-26
// Created by: Peter KURNEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <Bnd_Box2d.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Surface.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopOpeBRepBuild_CorrectFace2d.hxx>
#include <TopOpeBRepBuild_Tools2d.hxx>
#include <TopTools_IndexedDataMapOfShapeShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

//=================================================================================================

TopOpeBRepBuild_CorrectFace2d::TopOpeBRepBuild_CorrectFace2d()
{
  myIsDone      = Standard_False;
  myErrorStatus = 1;
}

//=================================================================================================

TopOpeBRepBuild_CorrectFace2d::TopOpeBRepBuild_CorrectFace2d(
  const TopoFace&                        aFace,
  const TopTools_IndexedMapOfOrientedShape& anAvoidMap,
  TopTools_IndexedDataMapOfShapeShape&      aMap)
{
  myFace        = aFace;
  myAvoidMap    = anAvoidMap;
  myIsDone      = Standard_False;
  myErrorStatus = 1;
  myMap         = (Standard_Address)&aMap;
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::SetMapOfTrans2dInfo(TopTools_IndexedDataMapOfShapeShape& aMap)
{
  myMap = (Standard_Address)&aMap;
}

//=================================================================================================

TopTools_IndexedDataMapOfShapeShape& TopOpeBRepBuild_CorrectFace2d::MapOfTrans2dInfo()
{
  return *(TopTools_IndexedDataMapOfShapeShape*)myMap;
}

//=================================================================================================

const TopoFace& TopOpeBRepBuild_CorrectFace2d::Face() const
{
  return myFace;
}

//=================================================================================================

Standard_Boolean TopOpeBRepBuild_CorrectFace2d::IsDone() const
{
  return myIsDone;
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_CorrectFace2d::ErrorStatus() const
{
  return myErrorStatus;
}

//=================================================================================================

const TopoFace& TopOpeBRepBuild_CorrectFace2d::CorrectedFace() const
{
  return myCorrectedFace;
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::BuildCopyData(
  const TopoFace&                        aFace,
  const TopTools_IndexedMapOfOrientedShape& anAvoidMap,
  TopoFace&                              aCopyFace,
  TopTools_IndexedMapOfOrientedShape&       aCopyAvoidMap,
  const Standard_Boolean                    aNeedToUsePMap)
{
  TopTools_IndexedDataMapOfShapeShape EdMap;

  ShapeBuilder BB;

  TopTools_IndexedDataMapOfShapeShape* pMap = (TopTools_IndexedDataMapOfShapeShape*)myMap;
  //
  // 1. Copy myFace =>> myCopyFace
  TopoShape aLocalShape = aFace.EmptyCopied();
  aCopyFace                = TopoDS::Face(aLocalShape);
  //  aCopyFace=TopoDS::Face(aFace.EmptyCopied());

  ShapeExplorer anExpWires(aFace, TopAbs_WIRE);
  for (; anExpWires.More(); anExpWires.Next())
  {
    const TopoWire& W = TopoDS::Wire(anExpWires.Current());

    aLocalShape       = W.EmptyCopied();
    TopoWire aWire = TopoDS::Wire(aLocalShape);
    //    TopoWire aWire = TopoDS::Wire(W.EmptyCopied());

    ShapeExplorer anExpEdges(W, TopAbs_EDGE);
    for (; anExpEdges.More(); anExpEdges.Next())
    {
      const TopoEdge& E = TopoDS::Edge(anExpEdges.Current());

      if (EdMap.Contains(E))
      {
        TopoShape anEdge = EdMap.ChangeFromKey(E);
        anEdge.Orientation(E.Orientation());
        BB.Add(aWire, anEdge);
        continue;
      }

      if (aNeedToUsePMap)
      {
        if (pMap->Contains(E))
        {
          TopoShape& anEdge = pMap->ChangeFromKey(E);
          anEdge.Orientation(E.Orientation());
          EdMap.Add(E, anEdge);
          BB.Add(aWire, anEdge);
          continue;
        }
      }

      // add edges
      aLocalShape         = E.EmptyCopied();
      TopoShape anEdge = TopoDS::Edge(aLocalShape);
      //      TopoShape anEdge = TopoDS::Edge(E.EmptyCopied());

      EdMap.Add(E, anEdge);

      ShapeExplorer anExpVertices(E, TopAbs_VERTEX);
      for (; anExpVertices.More(); anExpVertices.Next())
      {
        const TopoShape& aV = anExpVertices.Current();
        BB.Add(anEdge, aV);
      }

      anEdge.Orientation(E.Orientation());
      BB.Add(aWire, anEdge);
    }
    // Add wires
    aWire.Orientation(W.Orientation());
    EdMap.Add(W, aWire);

    BB.Add(aCopyFace, aWire);
  }

  //
  // 2. Copy myAvoidMap =>> myCopyAvoidMap
  Standard_Integer i, aNb;
  aNb = anAvoidMap.Extent();

  for (i = 1; i <= aNb; i++)
  {
    const TopoShape& aSh = anAvoidMap(i);

    if (EdMap.Contains(aSh))
    {
      TopoShape& aCopyShape = EdMap.ChangeFromKey(aSh);
      aCopyShape.Orientation(aSh.Orientation());
      aCopyAvoidMap.Add(aCopyShape);
    }
  }

  //
  // 3. Inversed EdMap
  if (aNeedToUsePMap)
  {
    aNb = EdMap.Extent();
    myEdMapInversed.Clear();
    for (i = 1; i <= aNb; i++)
    {
      const TopoShape& aSh     = EdMap.FindKey(i);
      const TopoShape& aShCopy = EdMap.FindFromIndex(i);
      myEdMapInversed.Add(aShCopy, aSh);
    }
  }
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::CheckFace()
{
  //
  // I. Is the surface periodic
  TopLoc_Location      aLocF;
  Handle(GeomSurface) Surf = BRepInspector::Surface(myCopyFace, aLocF);
  if (!(Surf->IsUPeriodic() || Surf->IsVPeriodic()))
  {
    myIsDone      = Standard_True;
    myErrorStatus = 4;
    return;
  }

  // modified by NIZHNY-MZV  Mon Apr 24 11:29:56 2000
  // don't treat torus surfaces
  if (Surf->IsUPeriodic() && Surf->IsVPeriodic())
  {
    myIsDone      = Standard_True;
    myErrorStatus = 4;
    return;
  }

  //
  // II. Has each wire at least one "licit" edge
  TopTools_IndexedMapOfOrientedShape aVoidWires;
  Standard_Integer                   i, aNbVoidWires, anEdgeExists = 0, priz;
  ShapeExplorer                    aFExp, aWExp;
  aFExp.Init(myCopyFace, TopAbs_WIRE);
  for (; aFExp.More(); aFExp.Next())
  {
    const TopoShape& aWire = aFExp.Current();
    if (myCopyAvoidMap.Contains(aWire))
    {
      anEdgeExists = 1;
      continue;
    }

    anEdgeExists = 0;
    aWExp.Init(aWire, TopAbs_EDGE);
    for (; aWExp.More(); aWExp.Next())
    {
      const TopoShape& anEdge = aWExp.Current();
      if (myCopyAvoidMap.Contains(anEdge))
      {
        anEdgeExists = 1;
        break;
      }
    }

    if (!anEdgeExists)
    {
      // This is the wire without any "Right" information
      aVoidWires.Add(aWire);
    }
  }

  // For Void Wires .
  // We assume the first edge from the wire is non-movable edge
  // PKV 24-Feb-2000
  aNbVoidWires = aVoidWires.Extent();
  for (i = 1; i <= aNbVoidWires; i++)
  {
    TopTools_IndexedMapOfShape aEM;
    TopExp1::MapShapes(aVoidWires(i), TopAbs_EDGE, aEM);
    if (aEM.Extent())
      myCopyAvoidMap.Add(aEM(1));
  }

  // III. Check all wires to know  whether they are closed or not
  aFExp.Init(myCopyFace, TopAbs_WIRE);
  for (; aFExp.More(); aFExp.Next())
  {
    myCurrentWire = TopoDS::Wire(aFExp.Current());
    priz          = MakeRightWire();
    if (priz)
    {
      // This myFace contains a wire (myCurrentWire) that is not closed.
      myIsDone      = Standard_True;
      myErrorStatus = 3;
      return;
    }
  }

  //
  // VI. Check connectability wires in 2d
  TopoFace                        aCopyFace;
  TopTools_IndexedMapOfOrientedShape aCopyAvoidMap;

  // Coping data
  BuildCopyData(myCopyFace, myCopyAvoidMap, aCopyFace, aCopyAvoidMap, Standard_False);

  aFExp.Init(aCopyFace, TopAbs_WIRE);
  for (; aFExp.More(); aFExp.Next())
  {
    myCurrentWire = TopoDS::Wire(aFExp.Current());
    if (!aCopyAvoidMap.Contains(myCurrentWire))
    {
      priz = ConnectWire(aCopyFace, aCopyAvoidMap, Standard_True);
      if (priz)
      {
        myIsDone      = Standard_True;
        myErrorStatus = 6;
        return;
      }
    }
  }

  // Face seems to be OK
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_CorrectFace2d::ConnectWire(
  TopoFace&                              aCopyFace,
  const TopTools_IndexedMapOfOrientedShape& aCopyAvoidMap,
  const Standard_Boolean                    aTryFlag)
{
  ShapeBuilder BB;
  //
  // 1.Make right order
  Standard_Integer priz;
  priz = MakeRightWire();
  if (priz)
    return priz;
  //
  // 2. Define the First Edge on the Wire from aCopyAvoidMap
  Standard_Integer i, aNbEdges = 0, aNbAvoidEdgesOnWire;
  Standard_Real    aDMax = 0, aR;

  ShapeExplorer                    aWExp;
  TopoEdge                        aFEdge;
  TopoShape                       aFirstEdge;
  TopTools_IndexedMapOfOrientedShape anAvoidMap;

  aWExp.Init(myCurrentWire, TopAbs_EDGE);
  for (; aWExp.More(); aWExp.Next(), aNbEdges++)
  {
    const TopoShape& anEdge = aWExp.Current();
    if (aCopyAvoidMap.Contains(anEdge))
      anAvoidMap.Add(anEdge);
  }

  if (aNbEdges == 1) // nothing to do with this wire.
    return 0;

  aNbAvoidEdgesOnWire = anAvoidMap.Extent();
  if (aNbAvoidEdgesOnWire == aNbEdges) // nothing to do. all edges on wire are good.
    return 0;

  // if  at least one  non-degenerated edge exists
  // among anAvoidMap set it as aFirstEdge
  priz = 0;
  for (i = 1; i <= aNbAvoidEdgesOnWire; i++)
  {
    const TopoEdge& anEdge = TopoDS::Edge(anAvoidMap(i));
    if (!BRepInspector::Degenerated(anEdge))
    {
      aFirstEdge = anEdge;
      priz       = 1;
      break;
    }
  }

  if (!priz)
  {
    // All of anAvoidMap edges are degenerated
    // So take the edge with max dist. between
    // First and Last 2d points as the First edge
    // if(aNbAvoidEdgesOnWire != 1)
    // return 1; //in case of several degenerated edges we cannot connect wire by right way
    for (i = 1; i <= aNbAvoidEdgesOnWire; i++)
    {
      gp_Pnt2d           PF, PL;
      const TopoEdge& aDegenEdge = TopoDS::Edge(anAvoidMap(i));
      GetP2dFL(aCopyFace, aDegenEdge, PF, PL);
      aR = PF.Distance(PL);
      if ((aR - aDMax) > 1e-7)
      {
        aDMax      = aR;
        aFirstEdge = anAvoidMap(i);
      }
    }
  }

  //
  // 3. Build HeadList (from myOrderedWireList), where aFirstEdge will be the first
  ShapeList HeadList;
  MakeHeadList(aFirstEdge, HeadList);
  //
  // 4. Check HeadList to provide toward point-arrow direction
  // modified by NIZNHY-PKV Mon Apr 24 14:43:20 2000 f
  // CheckList (HeadList);
  // modified by NIZNHY-PKV Tue Apr 25 12:08:29 2000CheckList (aCopyFace, HeadList);
  // modified by NIZNHY-PKV Mon Apr 24 14:43:24 2000t
  //
  //  5. Connect Edges
  TopTools_IndexedMapOfShape anEdMap;
  gp_Pnt2d                   PF, PL, PA, PB, PA1, PB1;
  Handle(GeomCurve2d)       aTrCF, aTrCR, aTrC;
  Standard_Real              aD, aDTolerance = Precision::Confusion();

  TopTools_SequenceOfShape aSeqEdges;

  TopTools_ListIteratorOfListOfShape anIt;
  anIt.Initialize(HeadList);
  for (; anIt.More(); anIt.Next())
    aSeqEdges.Append(anIt.Value());
  aNbEdges = aSeqEdges.Length();

  //
  // First Edge
  TopoEdge aFstEdge = TopoDS::Edge(aSeqEdges(1));
  GetP2dFL(aCopyFace, aFstEdge, PA, PB);
  PA1 = PA;
  PB1 = PB;

  for (i = 2; i <= aNbEdges; i++)
  {
    TopoEdge anEdge = TopoDS::Edge(aSeqEdges(i));
    GetP2dFL(aCopyFace, anEdge, PF, PL);

    aD = PF.Distance(PB);
    if (aD < aDTolerance)
    {
      PA = PF;
      PB = PL;
      continue; // nothing to do with the edge cose it already connected
    }

    // tralslation's  vector
    gp_Vec2d aTrV(PF, PB);

    // Begin modified by NIZHNY-MZV  Mon Mar 27 16:04:04 2000
    // translation vector must be equal to 2PI*n or it is wrong wire
    Standard_Real U = aTrV.X();
    Standard_Real V = aTrV.Y();

    BRepAdaptor_Surface BAS(aCopyFace);
    Standard_Boolean    UP = BAS.IsUPeriodic();
    Standard_Boolean    VP = BAS.IsVPeriodic();

    Standard_Boolean nonPU = (fabs(U) < 1e-7) ? Standard_True : Standard_False;
    Standard_Boolean nonPV = (fabs(V) < 1e-7) ? Standard_True : Standard_False;

    if (!nonPU && UP)
    {
      Standard_Real dU = fmod(fabs(U), 2 * M_PI);
      nonPU            = (dU > 1e-7 && (2 * M_PI - dU > 1e-7)) ? Standard_True : Standard_False;
    }

    if (!nonPV && VP)
    {
      Standard_Real dV = fmod(fabs(V), 2 * M_PI);
      nonPV            = (dV > 1e-7 && (2 * M_PI - dV > 1e-7)) ? Standard_True : Standard_False;
    }

    //    printf("(fmod(fabs(U), 2*M_PI) =%lf\n", (fmod(fabs(U), 2*M_PI)));
    //    printf(" (fmod(fabs(V), 2*M_PI) > 1e-7)=%lf\n", (fmod(fabs(V), 2*M_PI)));

    if (nonPU && nonPV && !BRepInspector::Degenerated(anEdge))
      return 1;
    // End modified by NIZHNY-MZV  Mon Mar 27 16:04:11 2000

    if (BRepInspector::IsClosed(anEdge, aCopyFace))
    {
      // a. Closed edge <--->
      if (anEdMap.Contains(anEdge))
        continue;
      anEdMap.Add(anEdge);

      TopAbs_Orientation anOri = anEdge.Orientation();

      TopoEdge anEF, anER;
      if (anOri == TopAbs_FORWARD)
      {
        anEF                     = anEdge;
        TopoShape aLocalShape = anEdge.Reversed();
        anER                     = TopoDS::Edge(aLocalShape);
        //	anER=TopoDS::Edge(anEdge.Reversed());
      }

      else
      {
        anER                     = anEdge;
        TopoShape aLocalShape = anEdge.Reversed();
        anEF                     = TopoDS::Edge(aLocalShape);
        //	anEF=TopoDS::Edge(anEdge.Reversed());
      }

      TranslateCurve2d(anEF, aCopyFace, aTrV, aTrCF);
      TranslateCurve2d(anER, aCopyFace, aTrV, aTrCR);

      if (aTryFlag) // Use Builder in a trying case
        BB.UpdateEdge(anEdge, aTrCF, aTrCR, aCopyFace, myFaceTolerance);

      else // Use "False-Builder" otherwise
        UpdateEdge(anEdge, aTrCF, aTrCR, aCopyFace, myFaceTolerance);
    }

    else
    {
      // b. Usual Edge
      TranslateCurve2d(anEdge, aCopyFace, aTrV, aTrC);

      if (aTryFlag)
        BB.UpdateEdge(anEdge, aTrC, aCopyFace, myFaceTolerance);
      else
        UpdateEdge(anEdge, aTrC, aCopyFace, myFaceTolerance);
    }

    GetP2dFL(aCopyFace, anEdge, PF, PL);

    PA = PF;
    PB = PL;

    ////////////////////////////////////////////
    // In case of a trying we check the first
    // and last 2d point of the contour
    if (aTryFlag)
    {
      if (i == aNbEdges)
      {
        aD = PA1.Distance(PB);
        if (aD > aDTolerance)
          return 1;
      }
    }
    ////////////////////////////////////////////

  } // end of for (i=2; i<=aNbEdges; i++)

  return 0;
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::Perform()
{
  /////////////
  // 0.
  myCopyAvoidMap.Clear();
  BuildCopyData(myFace, myAvoidMap, myCopyFace, myCopyAvoidMap, Standard_True);
  /////////////

  myCorrectedFace = myCopyFace;
  myCorrectedFace.Orientation(myCopyFace.Orientation());
  myFaceTolerance = BRepInspector::Tolerance(myCopyFace);
  //
  // 1. Check the input face first
  CheckFace();
  if (myIsDone)
    return;

  //
  // 2. Make all wires connected
  Standard_Integer priz;
  ShapeExplorer  aFExp;

  aFExp.Init(myCopyFace, TopAbs_WIRE);
  for (; aFExp.More(); aFExp.Next())
  {
    myCurrentWire = TopoDS::Wire(aFExp.Current());
    if (!myCopyAvoidMap.Contains(myCurrentWire))
    {
      priz = ConnectWire(myCopyFace, myCopyAvoidMap, Standard_False);
      if (priz)
      {
        // This myFace contains a wire (myCurrentWire) that is not closed.
        myIsDone      = Standard_False;
        myErrorStatus = 3;
        return;
      }
    }
  }
  //
  // 3. Define Outer Wire
  TopoWire anOuterWire;
  priz = OuterWire(anOuterWire);
  if (priz)
  {
    myIsDone      = Standard_False;
    myErrorStatus = 5; // can't find outer wire
    return;
  }
  //
  // 4. Moving the anOuterWire and other wires in 2d space
  MoveWires2d(anOuterWire);

  myIsDone      = Standard_True;
  myErrorStatus = 0;
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_CorrectFace2d::MakeRightWire()
{
  Standard_Integer aNbEdgesReally = 0;
  myOrderedWireList.Clear();

  // 1. Real amount of the edges on aWire
  ShapeExplorer aWExp;
  aWExp.Init(myCurrentWire, TopAbs_EDGE);
  for (; aWExp.More(); aWExp.Next())
    aNbEdgesReally++;

  // 2. We'll use Tools3::Path
  ShapeList aL;
  BooleanBuildTools2D::Path(myCurrentWire, aL);
  if (aL.Extent() != aNbEdgesReally)
  {
    myErrorStatus = 4;
    return 1;
  }

  // Begin modified by NIZNHY-PKV Tue Apr 25 12:04:45 2000
  // from path we obtained list in reverse order, so to have right wire
  // we need to reverse it
  ShapeList               aFL;
  TopTools_ListIteratorOfListOfShape lit(aL);
  for (; lit.More(); lit.Next())
    aFL.Prepend(lit.Value());

  myOrderedWireList = aFL;
  // End modified by NIZNHY-PKV Tue Apr 25 12:06:45 2000
  return 0;
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::GetP2dFL(const TopoFace& aF,
                                             const TopoEdge& anEdge,
                                             gp_Pnt2d&          P2dF,
                                             gp_Pnt2d&          P2dL)
{
  Standard_Real              aFirst, aLast;
  const Handle(GeomCurve2d) C2d = BRepInspector::CurveOnSurface(anEdge, aF, aFirst, aLast);
  C2d->D0(aFirst, P2dF);
  C2d->D0(aLast, P2dL);
  if (anEdge.Orientation() == TopAbs_REVERSED)
  {
    gp_Pnt2d P2dTmp;
    P2dTmp = P2dF;
    P2dF   = P2dL;
    P2dL   = P2dTmp;
  }
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::MakeHeadList(const TopoShape&   aFirstEdge,
                                                 ShapeList& HeadList) const
{
  TopoShape                       aFE = aFirstEdge;
  ShapeList               aTailList;
  TopTools_ListIteratorOfListOfShape anIt;
  Standard_Integer                   aFlag = 0;

  anIt.Initialize(myOrderedWireList);
  for (; anIt.More(); anIt.Next())
  {
    const TopoShape& anEdge = anIt.Value();
    // modified by NIZHNY-MZV  Mon Mar 27 11:40:00 2000
    if (aFE.IsNull() && !BRepInspector::Degenerated(TopoDS::Edge(anEdge)))
      aFE = anEdge;
    if (anEdge == aFE)
      aFlag = 1; // turn the switch ON
    if (aFlag)
      HeadList.Append(anEdge);
  }

  anIt.Initialize(myOrderedWireList);
  for (; anIt.More(); anIt.Next())
  {
    const TopoShape& anEdge = anIt.Value();
    if (anEdge == aFE)
      break;
    aTailList.Append(anEdge);
  }
  HeadList.Append(aTailList);
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::CheckList(const TopoFace&    aFace,
                                              ShapeList& HeadList)
{
  TopAbs_Orientation                 r1;
  Standard_Real                      aDTolerance = Precision::Confusion();
  TopTools_SequenceOfShape           aSeq;
  TopTools_ListIteratorOfListOfShape anIt(HeadList);
  for (; anIt.More(); anIt.Next())
  {
    aSeq.Append(anIt.Value());
  }

  r1 = aSeq(1).Orientation();
  Standard_Integer i, aNb = aSeq.Length();
  // modified by NIZNHY-PKV Mon Apr 24 14:43:57 2000f
  Standard_Boolean aFirstCheck = Standard_False;
  if (r1 == TopAbs_REVERSED)
  {
    // when the orientation of the first edge is Revesed,
    // and when everything in 2d is Ok :
    // if at least one Forward edge  exists in the List
    // we consider that no necessity to reverse
    // the list (aFirstCheck=Standard_True) .

    Standard_Real aD;
    gp_Pnt2d      PF, PL, PA, PB;

    TopoEdge aFstEdge = TopoDS::Edge(aSeq(1));
    GetP2dFL(aFace, aFstEdge, PA, PB);
    for (i = 2; i <= aNb; i++)
    {
      TopoEdge anEdge = TopoDS::Edge(aSeq(i));
      GetP2dFL(aFace, anEdge, PF, PL);

      aD = PF.Distance(PB);
      if (aD < aDTolerance)
      {
        PA = PF;
        PB = PL;
        if (anEdge.Orientation() == TopAbs_FORWARD)
        {
          aFirstCheck = Standard_True;
          break;
        }
      }
    }
  }
  // modified by NIZNHY-PKV Mon Apr 24 14:43:59 2000t

  r1 = aSeq(1).Orientation();

  TopoVertex aV1R, aV2F;

  TopoEdge     aFirstEdge = TopoDS::Edge(aSeq(1));
  ShapeExplorer anExp;
  anExp.Init(aFirstEdge, TopAbs_VERTEX);
  for (; anExp.More(); anExp.Next())
  {
    const TopoShape& aV1 = anExp.Current();
    if (aV1.Orientation() == TopAbs_REVERSED)
    {
      aV1R = TopoDS::Vertex(aV1);
      break;
    }
  }

  TopoEdge aSecondEdge = TopoDS::Edge(aSeq(2));
  anExp.Init(aSecondEdge, TopAbs_VERTEX);
  for (; anExp.More(); anExp.Next())
  {
    const TopoShape& aV1 = anExp.Current();
    if (aV1.Orientation() == TopAbs_FORWARD)
    {
      aV2F = TopoDS::Vertex(aV1);
      break;
    }
  }

  Point3d P1, P2;
  P1 = BRepInspector::Pnt(aV1R);
  P2 = BRepInspector::Pnt(aV2F);

  // modified by NIZNHY-PKV Mon Apr 24 13:24:06 2000 f
  Standard_Real dist = P1.Distance(P2);
  if ((!(dist < aDTolerance) && r1 == TopAbs_FORWARD) ||
      //(r1==TopAbs_REVERSED)) {
      (!aFirstCheck && r1 == TopAbs_REVERSED))
  {
    // modified by NIZNHY-PKV Mon Apr 24 14:28:06 2000 t
    //  We have to reverse the order in list
    aSeq.Append(aFirstEdge);
    HeadList.Clear();
    aNb = aSeq.Length();
    for (i = aNb; i > 1; i--)
    {
      HeadList.Append(aSeq(i));
    }
  }
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::TranslateCurve2d(const TopoEdge&    anE,
                                                     const TopoFace&    aFace,
                                                     const gp_Vec2d&       aTrV,
                                                     Handle(GeomCurve2d)& aC2DOut)
{
  Standard_Real        aFirst, aLast;
  Handle(GeomCurve2d) C2d = BRepInspector::CurveOnSurface(anE, aFace, aFirst, aLast);
  Handle(GeomCurve2d) aTrC;
  aTrC                               = Handle(GeomCurve2d)::DownCast(C2d->Copy());
  Handle(Geom2d_TrimmedCurve) newC2D = new Geom2d_TrimmedCurve(aTrC, aFirst, aLast);
  newC2D->Translate(aTrV);
  aC2DOut = newC2D;
}

//=================================================================================================

Standard_Integer TopOpeBRepBuild_CorrectFace2d::OuterWire(TopoWire& anOuterWire) const
{
  TopLoc_Location      Loc;
  Handle(GeomSurface) Surf = BRepInspector::Surface(TopoDS::Face(myCorrectedFace), Loc);

  ShapeExplorer ex(myCorrectedFace, TopAbs_WIRE);
  for (; ex.More(); ex.Next())
  {
    const TopoWire& aWire = TopoDS::Wire(ex.Current());

    TopoFace  newFace;
    ShapeBuilder BB;
    BB.MakeFace(newFace, Surf, Loc, myFaceTolerance);
    BB.Add(newFace, aWire);

    BRepTopAdaptor_FClass2d aClass2d(newFace, myFaceTolerance);
    TopAbs_State            aState = aClass2d.PerformInfinitePoint();
    if (aState == TopAbs_OUT)
    {
      anOuterWire = aWire;
      return 0;
    }
  }
  return 1; // such wire is not found
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::BndBoxWire(const TopoWire& aWire, Bnd_Box2d& B2d) const
{
  Bnd_Box2d       aB2d;
  ShapeExplorer aWEx(aWire, TopAbs_EDGE);
  for (; aWEx.More(); aWEx.Next())
  {
    const TopoEdge&  anEdge = TopoDS::Edge(aWEx.Current());
    Standard_Real       aTolE  = BRepInspector::Tolerance(anEdge);
    BRepAdaptor_Curve2d aBAC2d(anEdge, myCorrectedFace);
    BndLib_Add2dCurve::Add(aBAC2d, aTolE, aB2d);
  }
  B2d = aB2d;
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::MoveWire2d(TopoWire& aWire, const gp_Vec2d& aTrV)
{
  if (aTrV.Magnitude() < Precision::Confusion())
    return;

  Standard_Integer           i, aNbEdges;
  TopTools_SequenceOfShape   aSeqEdges;
  TopTools_IndexedMapOfShape anEdMap;
  Handle(GeomCurve2d)       aTrCF, aTrCR, aTrC;

  ShapeExplorer aWExp;

  aWExp.Init(aWire, TopAbs_EDGE);
  for (; aWExp.More(); aWExp.Next())
  {
    aSeqEdges.Append(aWExp.Current());
  }
  aNbEdges = aSeqEdges.Length();

  //
  // First Edge
  for (i = 1; i <= aNbEdges; i++)
  {
    TopoEdge anEdge = TopoDS::Edge(aSeqEdges(i));
    if (BRepInspector::IsClosed(anEdge, myCorrectedFace))
    {
      // a. Closed edge <--->
      if (anEdMap.Contains(anEdge))
        continue;
      anEdMap.Add(anEdge);

      TopAbs_Orientation anOri = anEdge.Orientation();

      TopoEdge anEF, anER;

      if (anOri == TopAbs_FORWARD)
      {
        anEF                     = anEdge;
        TopoShape aLocalShape = anEdge.Reversed();
        anER                     = TopoDS::Edge(aLocalShape);
        //	anER=TopoDS::Edge(anEdge.Reversed());
      }

      else
      {
        anER                     = anEdge;
        TopoShape aLocalShape = anEdge.Reversed();
        anEF                     = TopoDS::Edge(aLocalShape);
        //	anEF=TopoDS::Edge(anEdge.Reversed());
      }

      TranslateCurve2d(anEF, myCorrectedFace, aTrV, aTrCF);
      TranslateCurve2d(anER, myCorrectedFace, aTrV, aTrCR);
      UpdateEdge(anEdge, aTrCF, aTrCR, myCorrectedFace, myFaceTolerance);
    }

    else
    {
      // b. Usual Edge
      TranslateCurve2d(anEdge, myCorrectedFace, aTrV, aTrC);
      UpdateEdge(anEdge, aTrC, myCorrectedFace, myFaceTolerance);
    }
  } // end of for (i=1; i<=aNbEdges; i++)
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::MoveWires2d(TopoWire& anOuterWire)
{
  Bnd_Box2d        B2d, B2dOuterWire;
  Standard_Real    OuterU1, OuterV1, OuterU2, OuterV2, x, a, TwoPI;
  Standard_Integer b, k;
  gp_Vec2d         aTrV;

  TwoPI = 2. * M_PI;

  BndBoxWire(anOuterWire, B2dOuterWire);
  B2dOuterWire.Get(OuterU1, OuterV1, OuterU2, OuterV2);

  a = .5 * (OuterU1 + OuterU2);
  b = Standard_Integer(-a / TwoPI);
  k = (a < 0.) ? 1 : 0;
  x = (b + k) * TwoPI;
  //
  // 1. Move the Outer Wire to [0, 2*PI]
  aTrV.SetCoord(x, 0.);
  MoveWire2d(anOuterWire, aTrV);

  BndBoxWire(anOuterWire, B2dOuterWire);
  B2dOuterWire.Get(OuterU1, OuterV1, OuterU2, OuterV2);
  //
  // 2. Move all other wires into bounding box of the Outer Wire
  ShapeExplorer aFExp;
  aFExp.Init(myCorrectedFace, TopAbs_WIRE);
  for (; aFExp.More(); aFExp.Next())
  {
    myCurrentWire = TopoDS::Wire(aFExp.Current());
    if (myCurrentWire != anOuterWire)
    {
      BndBoxWire(myCurrentWire, B2d);
      Standard_Real u1, v1, u2, v2;
      B2d.Get(u1, v1, u2, v2);

      if (B2d.IsOut(B2dOuterWire))
      {
        // printf(" Need to Move\n" );
        a = u1 - OuterU1;
        b = Standard_Integer(-a / TwoPI);
        k = (a < 0.) ? 1 : 0;
        x = (b + k) * TwoPI;
        aTrV.SetCoord(x, 0.);
        MoveWire2d(myCurrentWire, aTrV);
      }
    }
  }
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::UpdateEdge(const TopoEdge&          ECopy,
                                               const Handle(GeomCurve2d)& C,
                                               const TopoFace&          F,
                                               const Standard_Real         Tol)
{
  ShapeBuilder BB;

  TopTools_IndexedDataMapOfShapeShape* pMap = (TopTools_IndexedDataMapOfShapeShape*)myMap;

  // E is the Original Edge from Original Face.
  if (myEdMapInversed.Contains(ECopy))
  {
    const TopoShape& E = myEdMapInversed.FindFromKey(ECopy);

    if (!pMap->Contains(E))
    {
      ShapeExplorer anExp;

      TopoShape anEdge = E.EmptyCopied();

      anExp.Init(E, TopAbs_VERTEX);
      for (; anExp.More(); anExp.Next())
      {
        const TopoShape& aV = anExp.Current();
        BB.Add(anEdge, aV);
      }

      BB.UpdateEdge(TopoDS::Edge(anEdge), C, F, Tol);
      pMap->Add(E, anEdge);
      // printf("pMap->Extent()=%d\n", pMap->Extent());
    }

    else
    {
      TopoShape& anEdge = pMap->ChangeFromKey(E);
      BB.UpdateEdge(TopoDS::Edge(anEdge), C, F, Tol);
    }
  }

  BB.UpdateEdge(ECopy, C, F, Tol);
}

//=================================================================================================

void TopOpeBRepBuild_CorrectFace2d::UpdateEdge(const TopoEdge&          ECopy,
                                               const Handle(GeomCurve2d)& C1,
                                               const Handle(GeomCurve2d)& C2,
                                               const TopoFace&          F,
                                               const Standard_Real         Tol)
{
  ShapeBuilder BB;

  TopTools_IndexedDataMapOfShapeShape* pMap = (TopTools_IndexedDataMapOfShapeShape*)myMap;

  // E is the Original Edge from Original Face.
  if (myEdMapInversed.Contains(ECopy))
  {
    const TopoShape& E = myEdMapInversed.FindFromKey(ECopy);

    if (!pMap->Contains(E))
    {
      TopoShape    anEdge = E.EmptyCopied();
      ShapeExplorer anExp;
      anExp.Init(E, TopAbs_VERTEX);
      for (; anExp.More(); anExp.Next())
      {
        const TopoShape& aV = anExp.Current();
        BB.Add(anEdge, aV);
      }
      BB.UpdateEdge(TopoDS::Edge(anEdge), C1, C2, F, Tol);
      pMap->Add(E, anEdge);
      // printf("pMap->Extent()=%d\n", pMap->Extent());
    }

    else
    {
      TopoShape& anEdge = pMap->ChangeFromKey(E);
      BB.UpdateEdge(TopoDS::Edge(anEdge), C1, C2, F, Tol);
    }
  }

  BB.UpdateEdge(ECopy, C1, C2, F, Tol);
}

//
// Description for ErrorStatus
// ErrorStatus=0 : All is done;
// ErrorStatus=1 : Nothing is done;
// ErrorStatus=2 : The wire doesn't contain any licit edges and the wire is not closed;
// ErrorStatus=3 : The face contains a wire that is not connectable
// ErrorStatus=4 : The base surface is not periodic;
// ErrorStatus=5 : an outer wir can not be found;
// ErrorStatus=6 : Wrong  connectability of a wire in 2d
