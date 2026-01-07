// Created on: 1997-12-24
// Created by: Prestataire Xuan PHAM PHU
// Copyright (c) 1997-1999 Matra Datavision
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

#include <BRep_Tool.hxx>
#include <gp_Vec.hxx>
#include <Standard_ProgramError.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRepDS_define.hxx>
#include <TopOpeBRepDS_Edge3dInterferenceTool.hxx>
#include <TopOpeBRepDS_EdgeVertexInterference.hxx>
#include <TopOpeBRepDS_Interference.hxx>
#include <TopOpeBRepTool_EXPORT.hxx>
#include <TopOpeBRepTool_TOOL.hxx>

#define M_FORWARD(st) (st == TopAbs_FORWARD)
#define M_REVERSED(st) (st == TopAbs_REVERSED)

static void FUN_Raise()
{
  throw Standard_ProgramError("Edge3dInterferenceTool");
}

#define POINT (0)
#define VERTEXonref (1)
#define VERTEXonOO (2)
#define VERTEXonOref (3)

// myIsVertex :
// ------------
// POINT :<Eref> interferes with <E> at a point
// <Eref> interferes with <E> at a vertex V,
//   VERTEXonref  : V is on shape of <Eref>
//   VERTEXonOO   : V is on shape of <E>
//   VERTEXonOref : V is on 2 shapes.

// myVonOO : only for VERTEXonOO || VERTEXonOref
// --------

// myP3d : only for POINT || VERTEXonref
// -------

static Standard_Boolean FUN_hasparam(const Handle(TopOpeBRepDS_Interference)& I,
                                     Standard_Real&                           paronE)
{
  // prequesitory : shapes <SIX> -> edge <E>
  // ? <paronE> = parameter of <G> on <E>
  TopOpeBRepDS_Kind GT     = I->GeometryType();
  Standard_Boolean  point  = (GT == TopOpeBRepDS_POINT);
  Standard_Boolean  vertex = (GT == TopOpeBRepDS_VERTEX);
  if (point)
  {
    Handle(TopOpeBRepDS_CurvePointInterference) CPI =
      Handle(TopOpeBRepDS_CurvePointInterference)::DownCast(I);
    if (CPI.IsNull())
      return Standard_False;
    paronE = CPI->Parameter();
  }
  if (vertex)
  {
    Handle(TopOpeBRepDS_EdgeVertexInterference) EVI =
      Handle(TopOpeBRepDS_EdgeVertexInterference)::DownCast(I);
    if (EVI.IsNull())
      return Standard_False;
    paronE = EVI->Parameter();
  }
  return Standard_True;
}

static Standard_Boolean FUN_paronOOE(const TopoEdge&     OOE,
                                     const Standard_Integer IsVertex,
                                     const TopoShape&    VonOO,
                                     const Point3d&          P3d,
                                     Standard_Real&         paronOOE)
{
  Standard_Boolean ok       = Standard_False;
  Standard_Boolean hasVonOO = (IsVertex > 1);
  if (hasVonOO)
    ok = FUN_tool_parVonE(TopoDS::Vertex(VonOO), OOE, paronOOE);
  else
  {
    Standard_Real dist;
    ok                 = FUN_tool_projPonE(P3d, OOE, paronOOE, dist);
    Standard_Real tol1 = BRepInspector::Tolerance(OOE);
    Standard_Real tol2 = tol1 * 1.e3;
    Standard_Real tol  = tol2;
    if (tol > 1.e-2)
      tol = 1.e-2;
    if (ok)
      ok = (dist <= tol);
  }
  return ok;
}

static Standard_Boolean FUN_keepIonF(const Vector3d&        tgref,
                                     const Standard_Real& parE,
                                     const TopoEdge&   E,
                                     const TopoFace&   F,
                                     const Standard_Real& tola)
// returns true if an interference I=(TonF,G=point/vertex,S=<E>)
// is to add to the Edge3dInterferenceTool resolving 3d complex transitions
// on edge E
{
  Vector3d           tmp;
  Standard_Boolean ok = TOOL1::TggeomE(parE, E, tmp);
  if (!ok)
    return Standard_False;
  Dir3d        tgE  = Dir3d(tmp);
  Standard_Real prod = Abs(tgref.Dot(tgE));
  if (Abs(1 - prod) < tola)
    return Standard_False; // <Eref> & <E> are tangent edges
  Vector3d dd;
  ok = FUN_tool_nggeomF(parE, E, F, dd);
  Dir3d ngF(dd);
  if (!ok)
    return Standard_False;
  prod = Abs((tgref ^ tgE).Dot(ngF));
  if (Abs(1 - prod) < tola)
    return Standard_False;
  return Standard_True;
}

// ----------------------------------------------------------------------
//             EDGE/FACE interferences reducing :
//
// ----------------------------------------------------------------------

//=================================================================================================

TopOpeBRepDS_Edge3dInterferenceTool::TopOpeBRepDS_Edge3dInterferenceTool()
    : myFaceOriented(0),
      myrefdef(Standard_False)
{
}

//=======================================================================
// function : InitPointVertex
// purpose  : Initializes reference data for edge/face complex transition
//=======================================================================
// I = (TonF, G=POINT/VERTEX, S=<E>) interference on <Eref>
// G has parameter <paronEref> on <Eref>, <paronE> on <E>

void TopOpeBRepDS_Edge3dInterferenceTool::InitPointVertex(const Standard_Integer IsVertex,
                                                          const TopoShape&    VonOO)
{
  myIsVertex = IsVertex;
  if (IsVertex > 1)
    myVonOO = VonOO;
}

//=======================================================================
// function : Init
// purpose  : Initializes reference data for edge/face complex transition
//=======================================================================
// I = (T on <F>, G=POINT/VERTEX, S=<E>) interference on <Eref>
// G has parameter <paronEref> on <Eref>, <paronE> on <E>
// -- <E> is edge of <F> --

void TopOpeBRepDS_Edge3dInterferenceTool::Init(const TopoShape&                      Eref,
                                               const TopoShape&                      E,
                                               const TopoShape&                      F,
                                               const Handle(TopOpeBRepDS_Interference)& I)
{
  const TopoEdge& EEref = TopoDS::Edge(Eref);
  const TopoEdge& EE    = TopoDS::Edge(E);
  const TopoFace& FF    = TopoDS::Face(F);
  myrefdef                 = Standard_False;

  myTole = Precision::Angular(); // NYI

  Standard_Real    pref = 0.0;
  Standard_Boolean ok   = ::FUN_hasparam(I, pref);
  if (!ok)
  {
    FUN_Raise();
    return;
  }
  // <myP3d> :
  {
    BRepAdaptor_Curve BC(EEref);
    myP3d = BC.Value(pref);
  }
  Vector3d tmp;
  ok = TOOL1::TggeomE(pref, EEref, tmp);
  if (!ok)
  {
    FUN_Raise();
    return;
  }
  Dir3d tgref(tmp);

  Standard_Real pOO;
  ok = ::FUN_paronOOE(EE, myIsVertex, myVonOO, myP3d, pOO);
  if (!ok)
  {
    FUN_Raise();
    return;
  }
  ok = TOOL1::TggeomE(pOO, EE, tmp);
  if (!ok)
  {
    FUN_Raise();
    return;
  }
  Dir3d tgOO(tmp);

  Standard_Real dot     = tgref.Dot(tgOO);
  dot                   = 1 - Abs(dot);
  Standard_Real    tola = Precision::Confusion();
  Standard_Boolean Esdm = (Abs(dot) < tola);
  if (Esdm)
    return;
  // NYI : il faut rejeter les interf I = (T,G,S=E) / E sdm with Eref

  // <myrefdef> :
  ok = ::FUN_keepIonF(tgref, pOO, EE, FF, myTole);
  if (!ok)
  {
    // <Eref> is tangent to <F>,
    // If the transition is FORWARD or REVERSED, it describes a 2d
    // transition (while crossing <E> on <F>), we do not keep it.
    const TopAbs_Orientation& O    = I->Transition().Orientation(TopAbs_IN);
    Standard_Boolean          is2d = M_FORWARD(O) || M_REVERSED(O);
    if (is2d)
      return;
  }
  myrefdef = Standard_True;

  // <myFaceOriented> :
  myFaceOriented = I->Transition().Index();

  // <myTgtref>
  myTgtref = tgref;

  Dir3d Norm = tgOO ^ tgref;
  myTool.Reset(tgOO, Norm);
}

//=================================================================================================

// I = (T on <F>, G=POINT/VERTEX, S=<E>) interference on <Eref>
void TopOpeBRepDS_Edge3dInterferenceTool::Add(const TopoShape&                      Eref,
                                              const TopoShape&                      E,
                                              const TopoShape&                      F,
                                              const Handle(TopOpeBRepDS_Interference)& I)
{
  if (!myrefdef)
  {
    Init(Eref, E, F, I);
    //    return;
  }

  if (!myrefdef)
    return;

  const TopoEdge& EE = TopoDS::Edge(E);
  const TopoFace& FF = TopoDS::Face(F);

  Standard_Real    pOO;
  Standard_Boolean ok = ::FUN_paronOOE(EE, myIsVertex, myVonOO, myP3d, pOO);
  if (!ok)
    return;
  gp_Pnt2d uv;
  {
    BRepAdaptor_Curve2d BC2d(EE, FF);
    uv = BC2d.Value(pOO);
  }

  ok = ::FUN_keepIonF(myTgtref, pOO, EE, FF, myTole);
  if (!ok)
  {
    const TopAbs_Orientation& O    = I->Transition().Orientation(TopAbs_IN);
    Standard_Boolean          is2d = M_FORWARD(O) || M_REVERSED(O);
    if (is2d)
      return;
  }

  TopAbs_Orientation oriloc = I->Transition().Orientation(TopAbs_IN);
  TopAbs_Orientation oritan;
  ok = FUN_tool_orientEinFFORWARD(EE, FF, oritan); // xpu : 30/12/97
  if (!ok)
    return;

  Dir3d Norm(FUN_tool_nggeomF(uv, FF));
  myTool.Compare(myTole, Norm, oriloc, oritan);
}

//=================================================================================================

void TopOpeBRepDS_Edge3dInterferenceTool::Transition(
  const Handle(TopOpeBRepDS_Interference)& I) const
{
  StateTransition& T = I->ChangeTransition();
  I->Support(myFaceOriented);

  TopAbs_State stb = myTool.StateBefore();
  TopAbs_State sta = myTool.StateAfter();
  T.Set(stb, sta);
}
