// Created on: 1995-07-07
// Created by: Joelle CHAUVET
// Copyright (c) 1995-1999 Matra Datavision
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

// Modified : 08/07/97 : JPI : traitement des edges degeneres comme pour les fillets 2D
// Modified:	Fri Sep 25 09:38:04 1998
//              status = ChFi2d_NotAuthorized si les aretes ne sont pas
//              des droites ou des cercles; fonction IsLineOrCircle
//              (BUC60288)

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeFace.hxx>
#include <ChFi2d.hxx>
#include <ChFi2d_Builder.hxx>
#include <ElCLib.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dInt_GInter.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAPI.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <Precision.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

Point3d ComputePoint(const TopoVertex& V,
                    const TopoEdge&   E,
                    const Standard_Real  D1,
                    Standard_Real&       Param1);
Point3d ComputePoint(const TopoFace&       F,
                    const Handle(GeomLine)& L,
                    const TopoEdge&       E,
                    Standard_Real&           Param);
void   OrientChamfer(TopoEdge& chamfer, const TopoEdge& E, const TopoVertex& V);

static Standard_Boolean IsLineOrCircle(const TopoEdge& E, const TopoFace& F);

//=================================================================================================

TopoEdge ChFi2d_Builder::AddChamfer(const TopoEdge&  E1,
                                       const TopoEdge&  E2,
                                       const Standard_Real D1,
                                       const Standard_Real D2)
{
  TopoVertex commonVertex;
  TopoEdge   basisEdge1, basisEdge2;
  TopoEdge   E1Mod, E2Mod, chamfer;

  Standard_Boolean hasConnection = ChFi2d1::CommonVertex(E1, E2, commonVertex);
  if (!hasConnection)
    return chamfer;

  if (IsAFillet(E1) || IsAChamfer(E1) || IsAFillet(E2) || IsAChamfer(E2))
  {
    status = ChFi2d_NotAuthorized;
    return chamfer;
  } //  if (IsAChamfer ...

  if (!IsLineOrCircle(E1, newFace) || !IsLineOrCircle(E2, newFace))
  {
    status = ChFi2d_NotAuthorized;
    return chamfer;
  } //  if (!IsLineOrCircle ...

  // EE1 and EE2 are copies of E1 and E2 with the good orientation
  // on <newFace>
  TopoEdge EE1, EE2;
  status = ChFi2d1::FindConnectedEdges(newFace, commonVertex, EE1, EE2);
  if (EE1.IsSame(E2))
  {
    TopAbs_Orientation orient = EE1.Orientation();
    EE1                       = EE2;
    EE2                       = E2;
    EE2.Orientation(orient);
  }

  ComputeChamfer(commonVertex, EE1, EE2, D1, D2, E1Mod, E2Mod, chamfer);
  if (status == ChFi2d_IsDone || status == ChFi2d_FirstEdgeDegenerated
      || status == ChFi2d_LastEdgeDegenerated || status == ChFi2d_BothEdgesDegenerated)
  {
    //  if (status == ChFi2d_IsDone) {
    BuildNewWire(EE1, EE2, E1Mod, chamfer, E2Mod);
    basisEdge1 = BasisEdge(EE1);
    basisEdge2 = BasisEdge(EE2);
    UpDateHistory(basisEdge1, basisEdge2, E1Mod, E2Mod, chamfer, 2);
    status = ChFi2d_IsDone;
    return TopoDS::Edge(chamfers.Value(chamfers.Length()));
  }
  return chamfer;
} // AddChamfer

//=================================================================================================

TopoEdge ChFi2d_Builder::AddChamfer(const TopoEdge&   E,
                                       const TopoVertex& V,
                                       const Standard_Real  D,
                                       const Standard_Real  Ang)
{
  TopoEdge aChamfer, adjEdge1, adjEdge2;
  status = ChFi2d1::FindConnectedEdges(newFace, V, adjEdge1, adjEdge2);
  if (status == ChFi2d_ConnexionError)
    return aChamfer;

  // adjEdge1 is a copy of E  with the good orientation
  // on <newFace>
  if (adjEdge2.IsSame(E))
  {
    TopAbs_Orientation orient = adjEdge2.Orientation();
    adjEdge2                  = adjEdge1;
    adjEdge1                  = E;
    adjEdge1.Orientation(orient);
  }

  if (IsAFillet(adjEdge1) || IsAChamfer(adjEdge1) || IsAFillet(adjEdge2) || IsAChamfer(adjEdge2))
  {
    status = ChFi2d_NotAuthorized;
    return aChamfer;
  } //  if (IsAChamfer ...

  if (!IsLineOrCircle(adjEdge1, newFace) || !IsLineOrCircle(adjEdge2, newFace))
  {
    status = ChFi2d_NotAuthorized;
    return aChamfer;
  } //  if (!IsLineOrCircle ...

  TopoEdge E1, E2;
  ComputeChamfer(V, adjEdge1, D, Ang, adjEdge2, E1, E2, aChamfer);
  TopoEdge basisEdge1, basisEdge2;
  if (status == ChFi2d_IsDone || status == ChFi2d_FirstEdgeDegenerated
      || status == ChFi2d_LastEdgeDegenerated || status == ChFi2d_BothEdgesDegenerated)
  {
    //  if (status == ChFi2d_IsDone) {
    BuildNewWire(adjEdge1, adjEdge2, E1, aChamfer, E2);
    basisEdge1 = BasisEdge(adjEdge1);
    basisEdge2 = BasisEdge(adjEdge2);
    UpDateHistory(basisEdge1, basisEdge2, E1, E2, aChamfer, 2);
    status = ChFi2d_IsDone;
    return TopoDS::Edge(chamfers.Value(chamfers.Length()));
  }
  return aChamfer;
}

//=================================================================================================

void ChFi2d_Builder::ComputeChamfer(const TopoVertex& V,
                                    const TopoEdge&   E1,
                                    const TopoEdge&   E2,
                                    const Standard_Real  D1,
                                    const Standard_Real  D2,
                                    TopoEdge&         TrimE1,
                                    TopoEdge&         TrimE2,
                                    TopoEdge&         Chamfer)
{
  TopoVertex    newExtr1, newExtr2;
  Standard_Boolean Degen1, Degen2;
  Chamfer = BuildChamferEdge(V, E1, E2, D1, D2, newExtr1, newExtr2);
  if (status != ChFi2d_IsDone)
    return;
  TrimE1 = BuildNewEdge(E1, V, newExtr1, Degen1);
  TrimE2 = BuildNewEdge(E2, V, newExtr2, Degen2);
  if (Degen1 && Degen2)
    status = ChFi2d_BothEdgesDegenerated;
  if (Degen1 && !Degen2)
    status = ChFi2d_FirstEdgeDegenerated;
  if (!Degen1 && Degen2)
    status = ChFi2d_LastEdgeDegenerated;
  //   TrimE1 = BuildNewEdge(E1, V, newExtr1);
  //  TrimE2 = BuildNewEdge(E2, V, newExtr2);
} // ComputeChamfer

//=================================================================================================

void ChFi2d_Builder::ComputeChamfer(const TopoVertex& V,
                                    const TopoEdge&   E1,
                                    const Standard_Real  D,
                                    const Standard_Real  Ang,
                                    const TopoEdge&   E2,
                                    TopoEdge&         TrimE1,
                                    TopoEdge&         TrimE2,
                                    TopoEdge&         Chamfer)
{
  TopoVertex    newExtr1, newExtr2;
  Standard_Boolean Degen1, Degen2;
  Chamfer = BuildChamferEdge(V, E1, D, Ang, E2, newExtr1, newExtr2);
  if (status != ChFi2d_IsDone)
    return;
  TrimE1 = BuildNewEdge(E1, V, newExtr1, Degen1);
  TrimE2 = BuildNewEdge(E2, V, newExtr2, Degen2);
  if (Degen1 && Degen2)
    status = ChFi2d_BothEdgesDegenerated;
  if (Degen1 && !Degen2)
    status = ChFi2d_FirstEdgeDegenerated;
  if (!Degen1 && Degen2)
    status = ChFi2d_LastEdgeDegenerated;
  //   TrimE1 = BuildNewEdge(E1, V, newExtr1);
  //   TrimE2 = BuildNewEdge(E2, V, newExtr2);
} // ComputeChamfer

//=================================================================================================

TopoEdge ChFi2d_Builder::ModifyChamfer(const TopoEdge& Chamfer,
                                          const TopoEdge& /*E1*/,
                                          const TopoEdge&  E2,
                                          const Standard_Real D1,
                                          const Standard_Real D2)
{
  TopoVertex aVertex = RemoveChamfer(Chamfer);
  TopoEdge   adjEdge1, adjEdge2;
  status = ChFi2d1::FindConnectedEdges(newFace, aVertex, adjEdge1, adjEdge2);
  TopoEdge aChamfer;
  if (status == ChFi2d_ConnexionError)
    return aChamfer;

  // adjEdge1 and adjEdge2 are copies of E1 and E2 with the good orientation
  // on <newFace>
  if (adjEdge1.IsSame(E2))
  {
    TopAbs_Orientation orient = adjEdge1.Orientation();
    adjEdge1                  = adjEdge2;
    adjEdge2                  = E2;
    adjEdge2.Orientation(orient);
  }

  aChamfer = AddChamfer(adjEdge1, adjEdge2, D1, D2);
  return aChamfer;
} // ModifyChamfer

//=================================================================================================

TopoEdge ChFi2d_Builder::ModifyChamfer(const TopoEdge&  Chamfer,
                                          const TopoEdge&  E,
                                          const Standard_Real D,
                                          const Standard_Real Ang)
{
  TopoVertex aVertex = RemoveChamfer(Chamfer);
  TopoEdge   adjEdge1, adjEdge2;
  status = ChFi2d1::FindConnectedEdges(newFace, aVertex, adjEdge1, adjEdge2);
  TopoEdge aChamfer;
  if (status == ChFi2d_ConnexionError)
    return aChamfer;

  if (adjEdge1.IsSame(E))
    aChamfer = AddChamfer(adjEdge1, aVertex, D, Ang);
  else
    aChamfer = AddChamfer(adjEdge2, aVertex, D, Ang);
  return aChamfer;
} // ModifyChamfer

//=================================================================================================

TopoVertex ChFi2d_Builder::RemoveChamfer(const TopoEdge& Chamfer)
{
  TopoVertex commonVertex;

  Standard_Integer i      = 1;
  Standard_Integer IsFind = Standard_False;
  while (i <= chamfers.Length())
  {
    const TopoEdge& aChamfer = TopoDS::Edge(chamfers.Value(i));
    if (aChamfer.IsSame(Chamfer))
    {
      chamfers.Remove(i);
      IsFind = Standard_True;
      break;
    }
    i++;
  }
  if (!IsFind)
    return commonVertex;

  TopoVertex firstVertex, lastVertex;
  TopExp1::Vertices(Chamfer, firstVertex, lastVertex);

  TopoEdge adjEdge1, adjEdge2;
  status = ChFi2d1::FindConnectedEdges(newFace, firstVertex, adjEdge1, adjEdge2);
  if (status == ChFi2d_ConnexionError)
    return commonVertex;

  TopoEdge basisEdge1, basisEdge2, E1, E2;
  // E1 and E2 are the adjacentes edges to Chamfer

  if (adjEdge1.IsSame(Chamfer))
    E1 = adjEdge2;
  else
    E1 = adjEdge1;
  basisEdge1 = BasisEdge(E1);
  status     = ChFi2d1::FindConnectedEdges(newFace, lastVertex, adjEdge1, adjEdge2);
  if (status == ChFi2d_ConnexionError)
    return commonVertex;
  if (adjEdge1.IsSame(Chamfer))
    E2 = adjEdge2;
  else
    E2 = adjEdge1;
  basisEdge2 = BasisEdge(E2);
  TopoVertex    connectionE1Chamfer, connectionE2Chamfer;
  Standard_Boolean hasConnection = ChFi2d1::CommonVertex(basisEdge1, basisEdge2, commonVertex);
  if (!hasConnection)
  {
    status = ChFi2d_ConnexionError;
    return commonVertex;
  }
  hasConnection = ChFi2d1::CommonVertex(E1, Chamfer, connectionE1Chamfer);
  if (!hasConnection)
  {
    status = ChFi2d_ConnexionError;
    return commonVertex;
  }
  hasConnection = ChFi2d1::CommonVertex(E2, Chamfer, connectionE2Chamfer);
  if (!hasConnection)
  {
    status = ChFi2d_ConnexionError;
    return commonVertex;
  }

  // rebuild edges on wire
  TopoEdge      newEdge1, newEdge2;
  TopoVertex    v, v1, v2;
  BRepLib_MakeEdge makeEdge;
  TopLoc_Location  loc;
  Standard_Real    first, last;

  TopExp1::Vertices(E1, firstVertex, lastVertex);
  TopExp1::Vertices(basisEdge1, v1, v2);
  if (v1.IsSame(commonVertex))
    v = v2;
  else
    v = v1;

  if (firstVertex.IsSame(v) || lastVertex.IsSame(v))
    // It means the edge support only one fillet. In this case
    // the new edge must be the basis edge.
    newEdge1 = basisEdge1;
  else
  {
    // It means the edge support one fillet on each end.
    if (firstVertex.IsSame(connectionE1Chamfer))
    {
      //  syntaxe invalide sur NT
      //      const Handle(GeomCurve3d)& curve =
      //	BRepInspector::Curve(E1, loc, first, last);
      Handle(GeomCurve3d) curve = BRepInspector::Curve(E1, loc, first, last);
      makeEdge.Init(curve, commonVertex, lastVertex);
      newEdge1 = makeEdge.Edge();
      newEdge1.Orientation(basisEdge1.Orientation());
      newEdge1.Location(basisEdge1.Location());
    } // if (firstVertex ...
    else if (lastVertex.IsSame(connectionE1Chamfer))
    {
      //  syntaxe invalide sur NT
      //      const Handle(GeomCurve3d)& curve =
      //	BRepInspector::Curve(E1, loc, first, last);
      Handle(GeomCurve3d) curve = BRepInspector::Curve(E1, loc, first, last);
      makeEdge.Init(curve, firstVertex, commonVertex);
      newEdge1 = makeEdge.Edge();
      newEdge1.Orientation(basisEdge1.Orientation());
      newEdge1.Location(basisEdge1.Location());
    } // else if (lastVertex ...
  } // else ...

  TopExp1::Vertices(basisEdge2, v1, v2);
  if (v1.IsSame(commonVertex))
    v = v2;
  else
    v = v1;

  TopExp1::Vertices(E2, firstVertex, lastVertex);
  if (firstVertex.IsSame(v) || lastVertex.IsSame(v))
    // It means the edge support only one fillet. In this case
    // the new edge must be the basis edge.
    newEdge2 = basisEdge2;
  else
  {
    // It means the edge support one fillet on each end.
    if (firstVertex.IsSame(connectionE2Chamfer))
    {
      //  syntaxe invalide sur NT
      //      const Handle(GeomCurve3d)& curve =
      //	BRepInspector::Curve(E2, loc, first, last);
      Handle(GeomCurve3d) curve = BRepInspector::Curve(E2, loc, first, last);
      makeEdge.Init(curve, commonVertex, lastVertex);
      newEdge2 = makeEdge.Edge();
      newEdge2.Orientation(basisEdge2.Orientation());
      newEdge2.Location(basisEdge2.Location());
    } // if (firstVertex ...
    else if (lastVertex.IsSame(connectionE2Chamfer))
    {
      //  syntaxe invalide sur NT
      //      const Handle(GeomCurve3d)& curve =
      //	BRepInspector::Curve(E2, loc, first, last);
      Handle(GeomCurve3d) curve = BRepInspector::Curve(E2, loc, first, last);
      makeEdge.Init(curve, firstVertex, commonVertex);
      newEdge2 = makeEdge.Edge();
      newEdge2.Orientation(basisEdge2.Orientation());
      newEdge2.Location(basisEdge2.Location());
    } // else if (lastVertex ...
  } // else ...

  // rebuild the newFace
  ShapeExplorer Ex(newFace, TopAbs_EDGE);
  TopoWire     newWire;

  ShapeBuilder B;
  B.MakeWire(newWire);

  while (Ex.More())
  {
    const TopoEdge& theEdge = TopoDS::Edge(Ex.Current());
    if (!theEdge.IsSame(E1) && !theEdge.IsSame(E2) && !theEdge.IsSame(Chamfer))
      B.Add(newWire, theEdge);
    else
    {
      if (theEdge == E1)
        B.Add(newWire, newEdge1);
      else if (theEdge == E2)
        B.Add(newWire, newEdge2);
    } // else
    Ex.Next();
  } // while ...

  BRepAdaptor_Surface Adaptor3dSurface(refFace);
  BRepLib_MakeFace    mFace(Adaptor3dSurface.Plane1(), newWire);
  newFace.Nullify();
  newFace = mFace;

  UpDateHistory(basisEdge1, basisEdge2, newEdge1, newEdge2);

  return commonVertex;
} // RemoveChamfer

//=================================================================================================

TopoEdge ChFi2d_Builder::BuildChamferEdge(const TopoVertex& V,
                                             const TopoEdge&   AdjEdge1,
                                             const TopoEdge&   AdjEdge2,
                                             const Standard_Real  D1,
                                             const Standard_Real  D2,
                                             TopoVertex&       NewExtr1,
                                             TopoVertex&       NewExtr2)
{
  TopoEdge chamfer;
  if (D1 <= 0 || D2 <= 0)
  {
    status = ChFi2d_ParametersError;
    return chamfer;
  } // if ( D1 <=0 ...

  Standard_Real param1, param2;
  Point3d        p1 = ComputePoint(V, AdjEdge1, D1, param1);
  Point3d        p2 = ComputePoint(V, AdjEdge2, D2, param2);

  Standard_Real tol = Precision1::Confusion();
  ShapeBuilder  B;
  B.MakeVertex(NewExtr1, p1, tol);
  B.MakeVertex(NewExtr2, p2, tol);
  NewExtr1.Orientation(TopAbs_FORWARD);
  NewExtr2.Orientation(TopAbs_REVERSED);

  // chamfer edge construction
  TopLoc_Location            loc;
  const Handle(GeomSurface) refSurf = BRepInspector::Surface(refFace, loc);
  Vector3d                     myVec(p1, p2);
  Dir3d                     myDir(myVec);
  Handle(GeomLine)          newLine = new GeomLine(p1, myDir);
  Standard_Real              param   = ElCLib1::Parameter(newLine->Lin(), p2);
  B.MakeEdge(chamfer, newLine, tol);
  B.Range(chamfer, 0., param);
  B.Add(chamfer, NewExtr1);
  B.UpdateVertex(NewExtr1, 0., chamfer, tol);
  B.Add(chamfer, NewExtr2);
  B.UpdateVertex(NewExtr2, param, chamfer, tol);
  OrientChamfer(chamfer, AdjEdge1, V);

  // set the orientation of NewExtr1 and NewExtr2 for the adjacent edges
  TopoVertex V1 = TopExp1::FirstVertex(AdjEdge1);
  TopoVertex V2 = TopExp1::LastVertex(AdjEdge1);
  if (V1.IsSame(V))
    NewExtr1.Orientation(V1.Orientation());
  else
    NewExtr1.Orientation(V2.Orientation());

  V1 = TopExp1::FirstVertex(AdjEdge2);
  V2 = TopExp1::LastVertex(AdjEdge2);
  if (V1.IsSame(V))
    NewExtr2.Orientation(V1.Orientation());
  else
    NewExtr2.Orientation(V2.Orientation());
  B.UpdateVertex(NewExtr1, param1, AdjEdge1, tol);
  B.UpdateVertex(NewExtr2, param2, AdjEdge2, tol);

  status = ChFi2d_IsDone;
  return chamfer;
} // BuildChamferEdge

//=================================================================================================

TopoEdge ChFi2d_Builder::BuildChamferEdge(const TopoVertex& V,
                                             const TopoEdge&   AdjEdge1,
                                             const Standard_Real  D,
                                             const Standard_Real  Ang,
                                             const TopoEdge&   AdjEdge2,
                                             TopoVertex&       NewExtr1,
                                             TopoVertex&       NewExtr2)
{
  TopoEdge chamfer;
  if (D <= 0 || Ang <= 0)
  {
    status = ChFi2d_ParametersError;
    return chamfer;
  } // if ( D <= 0 ...

  Standard_Real param1, param2;
  Point3d        p1 = ComputePoint(V, AdjEdge1, D, param1);
  Point3d        p  = BRepInspector::Pnt(V);
  Vector3d        myVec(p1, p);

  // compute the tangent vector on AdjEdge2 at the vertex V.
  BRepAdaptor_Curve c(AdjEdge2, refFace);
  Standard_Real     first, last;
  first = c.FirstParameter();
  last  = c.LastParameter();

  Point3d aPoint;
  Vector3d tan;
  c.D1(first, aPoint, tan);
  if (aPoint.Distance(p) > Precision1::Confusion())
  {
    c.D1(last, aPoint, tan);
  }
  // tangent orientation
  TopoVertex v1, v2;
  TopExp1::Vertices(AdjEdge2, v1, v2);
  TopAbs_Orientation orient;
  if (v1.IsSame(V))
    orient = v1.Orientation();
  else
    orient = v2.Orientation();
  if (orient == TopAbs_REVERSED)
    tan *= -1;

  // compute the chamfer geometric support
  Axis3d            RotAxe(p1, tan ^ myVec);
  Vector3d            vecLin = myVec.Rotated(RotAxe, -Ang);
  Dir3d            myDir(vecLin);
  Handle(GeomLine) newLine = new GeomLine(p1, myDir);
  ShapeBuilder      B1;
  B1.MakeEdge(chamfer, newLine, Precision1::Confusion());
  Point3d p2 = ComputePoint(refFace, newLine, AdjEdge2, param2);

  Standard_Real tol = Precision1::Confusion();
  ShapeBuilder  B;
  B.MakeVertex(NewExtr1, p1, tol);
  B.MakeVertex(NewExtr2, p2, tol);
  NewExtr1.Orientation(TopAbs_FORWARD);
  NewExtr2.Orientation(TopAbs_REVERSED);

  // chamfer edge construction
  Standard_Real param = ElCLib1::Parameter(newLine->Lin(), p2);
  B.MakeEdge(chamfer, newLine, tol);
  B.Range(chamfer, 0., param);
  B.Add(chamfer, NewExtr1);
  B.UpdateVertex(NewExtr1, 0., chamfer, tol);
  B.Add(chamfer, NewExtr2);
  B.UpdateVertex(NewExtr2, param, chamfer, tol);
  OrientChamfer(chamfer, AdjEdge1, V);

  // set the orientation of NewExtr1 and NewExtr2 for the adjacent edges
  TopoVertex V1 = TopExp1::FirstVertex(AdjEdge1);
  TopoVertex V2 = TopExp1::LastVertex(AdjEdge1);
  if (V1.IsSame(V))
    NewExtr1.Orientation(V1.Orientation());
  else
    NewExtr1.Orientation(V2.Orientation());

  V1 = TopExp1::FirstVertex(AdjEdge2);
  V2 = TopExp1::LastVertex(AdjEdge2);
  if (V1.IsSame(V))
    NewExtr2.Orientation(V1.Orientation());
  else
    NewExtr2.Orientation(V2.Orientation());
  B.UpdateVertex(NewExtr1, param1, AdjEdge1, tol);
  B.UpdateVertex(NewExtr2, param2, AdjEdge2, tol);

  status = ChFi2d_IsDone;
  return chamfer;
}

//=================================================================================================

Point3d ComputePoint(const TopoVertex& V,
                    const TopoEdge&   E,
                    const Standard_Real  D,
                    Standard_Real&       Param)
{
  // geometric support
  BRepAdaptor_Curve c(E);
  Standard_Real     first, last;
  first = c.FirstParameter();
  last  = c.LastParameter();

  Point3d thePoint;
  if (c.GetType() == GeomAbs_Line)
  {
    Point3d        p1, p2;
    TopoVertex v1, v2;
    TopExp1::Vertices(E, v1, v2);
    p1 = BRepInspector::Pnt(v1);
    p2 = BRepInspector::Pnt(v2);
    Vector3d myVec(p1, p2);
    myVec.Normalize();
    myVec *= D;
    if (v2.IsSame(V))
    {
      myVec *= -1; // change the sense of myVec
      thePoint = p2.Translated(myVec);
    } // if (v2 ...
    else
      thePoint = p1.Translated(myVec);

    Param = ElCLib1::Parameter(c.Line(), thePoint);
    // szv:OCC20823-begin
    c.D0(Param, thePoint);
    // szv:OCC20823-end
    return thePoint;
  } // if (C->IsKind(TYPE ...

  if (c.GetType() == GeomAbs_Circle)
  {
    gp_Circ       cir    = c.Circle();
    Standard_Real radius = cir.Radius();
    TopoVertex v1, v2;
    TopExp1::Vertices(E, v1, v2);
    Standard_Real param1, param2;
    if (V.IsSame(v1))
    {
      param1 = BRepInspector::Parameter(v1, E);
      param2 = BRepInspector::Parameter(v2, E);
    }
    else
    {
      param1 = BRepInspector::Parameter(v2, E);
      param2 = BRepInspector::Parameter(v1, E);
    }
    Standard_Real deltaAlpha = D / radius;
    if (param1 > param2)
      Param = param1 - deltaAlpha;
    else
      Param = param1 + deltaAlpha;
    c.D0(Param, thePoint);
    return thePoint;
  } // if (C->IsKind(TYPE ...

  else
  {
    // in all other case than lines and circles.
    Point3d        p;
    TopoVertex v1, v2;
    TopExp1::Vertices(E, v1, v2);
    if (V.IsSame(v1))
    {
      p = BRepInspector::Pnt(v1);
    }
    else
    {
      p = BRepInspector::Pnt(v2);
    }

    const GeomAdaptor_Curve& cc = c.Curve();
    if (p.Distance(c.Value(first)) <= Precision1::Confusion())
    {
      GCPnts_AbscissaPoint computePoint(cc, D, first);
      Param = computePoint.Parameter();
    }
    else
    {
      GCPnts_AbscissaPoint computePoint(cc, D, last);
      Param = computePoint.Parameter();
    }
    thePoint = cc.Value(Param);
    return thePoint;
  } // else ...
} // ComputePoint

//=================================================================================================

Point3d ComputePoint(const TopoFace&       F,
                    const Handle(GeomLine)& L,
                    const TopoEdge&       E,
                    Standard_Real&           Param)
{
  BRepAdaptor_Surface  Adaptor3dSurface(F);
  Handle(GeomPlane)   refSurf = new GeomPlane(Adaptor3dSurface.Plane1());
  Handle(GeomCurve2d) lin2d   = GeomAPI1::To2d(L, refSurf->Pln());
  Handle(GeomCurve2d) c2d;
  Standard_Real        first, last;
  c2d = BRepInspector::CurveOnSurface(E, F, first, last);
  Geom2dAdaptor_Curve adaptorL(lin2d);
  Geom2dAdaptor_Curve adaptorC(c2d);
  Geom2dInt_GInter    Intersection(adaptorL,
                                adaptorC,
                                Precision1::PIntersection(),
                                Precision1::PIntersection());
  Standard_Real       paramOnLine = 1E300;
  gp_Pnt2d            p2d;
  if (Intersection.IsDone())
  {
    Standard_Integer i = 1;
    while (i <= Intersection.NbPoints())
    {
      IntRes2d_IntersectionPoint iP = Intersection.Point(i);
      if (iP.ParamOnFirst() < paramOnLine)
      {
        p2d         = iP.Value();
        paramOnLine = iP.ParamOnFirst();
        Param       = iP.ParamOnSecond();
      } // if (iP.ParamOnFirst ...
      i++;
    } // while ( i <= ...
  } // if (Intersection.IsDone ...

  Point3d thePoint = Adaptor3dSurface.Value(p2d.X(), p2d.Y());
  return thePoint;
} // ComputePoint

//=================================================================================================

void OrientChamfer(TopoEdge& chamfer, const TopoEdge& E, const TopoVertex& V)
{
  TopAbs_Orientation vOrient, orient = E.Orientation();
  TopoVertex      v1, v2;
  TopExp1::Vertices(E, v1, v2);
  if (v1.IsSame(V))
    vOrient = v2.Orientation();
  else
    vOrient = v1.Orientation();

  if ((orient == TopAbs_FORWARD && vOrient == TopAbs_FORWARD)
      || (orient == TopAbs_REVERSED && vOrient == TopAbs_REVERSED))
    chamfer.Orientation(TopAbs_FORWARD);
  else
    chamfer.Orientation(TopAbs_REVERSED);
} // OrientChamfer

//=================================================================================================

Standard_Boolean IsLineOrCircle(const TopoEdge& E, const TopoFace& F)
{
  Standard_Real   first, last;
  TopLoc_Location loc;
  //  syntaxe invalide sur NT
  //      const Handle(GeomCurve2d)& C =
  //	BRepInspector::CurveOnSurface(E,F,first,last);
  Handle(GeomCurve2d)        C = BRepInspector::CurveOnSurface(E, F, first, last);
  Handle(GeomCurve2d)        basisC;
  Handle(Geom2d_TrimmedCurve) TC = Handle(Geom2d_TrimmedCurve)::DownCast(C);
  if (!TC.IsNull())
    basisC = TC->BasisCurve();
  else
    basisC = C;

  if (basisC->DynamicType() == STANDARD_TYPE(Geom2d_Circle)
      || basisC->DynamicType() == STANDARD_TYPE(Geom2d_Line))
  {
    return Standard_True;
  }
  else
  {
    return Standard_False;
  } // else ...
} // IsLineOrCircle
