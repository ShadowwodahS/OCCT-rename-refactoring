// Created on: 2001-03-28
// Created by: Peter KURNEV
// Copyright (c) 2001-2014 OPEN CASCADE SAS
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

#include <BOPTest.hxx>
#include <BOPTools_AlgoTools2D.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <DBRep.hxx>
#include <Draw.hxx>
#include <DrawTrSurf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <IntTools_FClass2d.hxx>
#include <TopAbs_State.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>

#include <stdio.h>
static void                 PrintState(DrawInterpreter& aDI, const TopAbs_State& aState);
static Handle(GeomCurve2d) CurveOnSurface(const TopoEdge& E,
                                           const TopoFace& F,
                                           Standard_Real&     First,
                                           Standard_Real&     Last);
static Handle(GeomCurve2d) CurveOnSurface(const TopoEdge&          E,
                                           const Handle(GeomSurface)& S,
                                           const TopLoc_Location&      L,
                                           Standard_Real&              First,
                                           Standard_Real&              Last);

static Standard_Integer bclassify(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer b2dclassify(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer b2dclassifx(DrawInterpreter&, Standard_Integer, const char**);
static Standard_Integer bhaspc(DrawInterpreter&, Standard_Integer, const char**);

//=================================================================================================

void BOPTest1::LowCommands(DrawInterpreter& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done)
    return;
  done = Standard_True;
  // Chapter's name
  const char* g = "BOPTest1 commands";
  theCommands.Add("bclassify",
                  "use bclassify Solid Point [Tolerance=1.e-7]",
                  __FILE__,
                  bclassify,
                  g);
  theCommands.Add(
    "b2dclassify",
    "use b2dclassify Face Point2d [Tol] [UseBox] [GapCheckTol]\n"
    "Classify  the Point  Point2d  with  Tolerance <Tol> on the face described by <Face>.\n"
    "<UseBox> == 1/0 (default <UseBox> = 0): switch on/off the use Box2 in the classification.\n"
    "<GapCheckTol> (default <GapCheckTol> = 0.1): this is for additional verification of\n"
    "the vertex with a tolerance >= <GapCheckTol>.",
    __FILE__,
    b2dclassify,
    g);
  theCommands.Add("b2dclassifx", "use b2dclassifx Face Point2d [Tol] ", __FILE__, b2dclassifx, g);
  theCommands.Add("bhaspc", "use bhaspc Edge Face [do]", __FILE__, bhaspc, g);
}

// lj cd
//=================================================================================================

Standard_Integer b2dclassifx(DrawInterpreter& theDI,
                             Standard_Integer  theArgNb,
                             const char**      theArgVec)
{
  if (theArgNb < 3)
  {
    theDI << " use b2dclassifx Face Point2d [Tol]\n";
    return 1;
  }

  TopoShape aS = DBRep1::Get(theArgVec[1]);
  if (aS.IsNull())
  {
    theDI << " Null Shape is not allowed here\n";
    return 1;
  }
  else if (aS.ShapeType() != TopAbs_FACE)
  {
    theDI << " Shape type must be FACE\n";
    return 1;
  }
  TopAbs_State aState;
  gp_Pnt2d     aP(8., 9.);
  //
  DrawTrSurf1::GetPoint2d(theArgVec[2], aP);
  const TopoFace&  aF   = TopoDS::Face(aS);
  const Standard_Real aTol = (theArgNb == 4) ? Draw1::Atof(theArgVec[3]) : BRepInspector::Tolerance(aF);
  //
  IntTools_FClass2d aClassifier(aF, aTol);
  aState = aClassifier.Perform(aP);
  PrintState(theDI, aState);
  //
  return 0;
}

//
//=================================================================================================

Standard_Integer b2dclassify(DrawInterpreter& theDI,
                             Standard_Integer  theArgNb,
                             const char**      theArgVec)
{
  if (theArgNb < 3)
  {
    theDI << " use b2dclassify Face Point2d [Tol] [UseBox] [GapCheckTol]\n";
    return 1;
  }

  TopoShape aS = DBRep1::Get(theArgVec[1]);
  if (aS.IsNull())
  {
    theDI << " Null Shape is not allowed here\n";
    return 1;
  }
  else if (aS.ShapeType() != TopAbs_FACE)
  {
    theDI << " Shape type must be FACE\n";
    return 1;
  }
  //
  gp_Pnt2d aP(8., 9.);
  //
  DrawTrSurf1::GetPoint2d(theArgVec[2], aP);
  const TopoFace&  aF   = TopoDS::Face(aS);
  const Standard_Real aTol = (theArgNb >= 4) ? Draw1::Atof(theArgVec[3]) : BRepInspector::Tolerance(aF);
  const Standard_Boolean anUseBox =
    (theArgNb >= 5 && Draw1::Atof(theArgVec[4]) == 1) ? Standard_True : Standard_False;
  const Standard_Real      aGapCheckTol = (theArgNb == 6) ? Draw1::Atof(theArgVec[5]) : 0.1;
  BRepClass_FaceClassifier aClassifier;
  aClassifier.Perform(aF, aP, aTol, anUseBox, aGapCheckTol);
  PrintState(theDI, aClassifier.State());
  //
  return 0;
}

//=================================================================================================

Standard_Integer bclassify(DrawInterpreter& theDI,
                           Standard_Integer  theArgNb,
                           const char**      theArgVec)
{
  if (theArgNb < 3)
  {
    theDI << " use bclassify Solid Point [Tolerance=1.e-7]\n";
    return 1;
  }

  TopoShape aS = DBRep1::Get(theArgVec[1]);
  if (aS.IsNull())
  {
    theDI << " Null Shape is not allowed\n";
    return 1;
  }
  else if (aS.ShapeType() != TopAbs_SOLID)
  {
    theDI << " Shape type must be SOLID\n";
    return 1;
  }

  Point3d aP(8., 9., 10.);
  DrawTrSurf1::GetPoint(theArgVec[2], aP);
  const Standard_Real aTol = (theArgNb == 4) ? Draw1::Atof(theArgVec[3]) : 1.e-7;

  BRepClass3d_SolidClassifier aSC(aS);
  aSC.Perform(aP, aTol);

  PrintState(theDI, aSC.State());
  return 0;
}

//=================================================================================================

Standard_Integer bhaspc(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
  {
    di << " use bhaspc Edge Face [do]\n";
    return 1;
  }

  TopoShape S1 = DBRep1::Get(a[1]);
  TopoShape S2 = DBRep1::Get(a[2]);

  if (S1.IsNull() || S2.IsNull())
  {
    di << " Null shapes are not allowed \n";
    return 1;
  }
  if (S1.ShapeType() != TopAbs_EDGE || S2.ShapeType() != TopAbs_FACE)
  {
    di << " Type mismatch\n";
    return 1;
  }
  //
  const TopoEdge& aE = TopoDS::Edge(S1);
  const TopoFace& aF = TopoDS::Face(S2);
  Standard_Real      f2D, l2D;

  Handle(GeomCurve2d) C2D = CurveOnSurface(aE, aF, f2D, l2D);

  if (C2D.IsNull())
  {
    di << " No 2D Curves detected\n";
  }
  else
  {
    di << " Ok Edge has P-Curve on this Face\n";
  }

  if (n == 4)
  {
    if (!strcmp(a[3], "do"))
    {
      AlgoTools2D::BuildPCurveForEdgeOnFace(aE, aF);
    }
  }

  return 0;
}

//=================================================================================================

void PrintState(DrawInterpreter& theDI, const TopAbs_State& theState)
{
  switch (theState)
  {
    case TopAbs_IN:
      theDI << "The point is IN shape\n";
      break;
    case TopAbs_OUT:
      theDI << "The point is OUT of shape\n";
      break;
    case TopAbs_ON:
      theDI << "The point is ON shape\n";
      break;
    case TopAbs_UNKNOWN:
    default:
      theDI << "The point is UNKNOWN shape\n";
      break;
  }
}

//=================================================================================================

Handle(GeomCurve2d) CurveOnSurface(const TopoEdge& E,
                                    const TopoFace& F,
                                    Standard_Real&     First,
                                    Standard_Real&     Last)
{
  TopLoc_Location             l;
  const Handle(GeomSurface)& S          = BRepInspector::Surface(F, l);
  TopoEdge                 aLocalEdge = E;
  if (F.Orientation() == TopAbs_REVERSED)
  {
    aLocalEdge.Reverse();
  }
  return CurveOnSurface(aLocalEdge, S, l, First, Last);
}

static Handle(GeomCurve2d) nullPCurve;

//=================================================================================================

Handle(GeomCurve2d) CurveOnSurface(const TopoEdge&          E,
                                    const Handle(GeomSurface)& S,
                                    const TopLoc_Location&      L,
                                    Standard_Real&              First,
                                    Standard_Real&              Last)
{
  TopLoc_Location  l           = L.Predivided(E.Location());
  Standard_Boolean Eisreversed = (E.Orientation() == TopAbs_REVERSED);

  // find the representation
  BRep_ListIteratorOfListOfCurveRepresentation itcr(
    (*((Handle(BRep_TEdge)*)&E.TShape()))->ChangeCurves());

  while (itcr.More())
  {
    const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
    if (cr->IsCurveOnSurface(S, l))
    {
      Handle(BRep_GCurve) GC(Handle(BRep_GCurve)::DownCast(cr));
      GC->Range(First, Last);
      if (GC->IsCurveOnClosedSurface() && Eisreversed)
        return GC->PCurve2();
      else
        return GC->PCurve();
    }
    itcr.Next();
  }
  return nullPCurve;
}
