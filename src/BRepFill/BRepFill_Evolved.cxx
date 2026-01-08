// Created on: 1994-10-03
// Created by: Bruno DUMORTIER
// Copyright (c) 1994-1999 Matra Datavision
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

#include <Bisector_Bisec.hxx>
#include <Bisector_BisecAna.hxx>
#include <Bnd_Box2d.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepAlgo_Loop.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepFill_DataMapOfNodeDataMapOfShapeShape.hxx>
#include <BRepFill_DataMapOfShapeSequenceOfPnt.hxx>
#include <BRepFill_DataMapOfShapeSequenceOfReal.hxx>
#include <BRepFill_Evolved.hxx>
#include <BRepFill_OffsetAncestors.hxx>
#include <BRepFill_OffsetWire.hxx>
#include <BRepFill_Pipe.hxx>
#include <BRepFill_TrimSurfaceTool.hxx>
#include <BRepLib.hxx>
#include <BRepLib_FindSurface.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepLProp.hxx>
#include <BRepMAT2d_BisectingLocus.hxx>
#include <BRepMAT2d_Explorer.hxx>
#include <BRepMAT2d_LinkTopoBilo.hxx>
#include <BRepSweep_Prism.hxx>
#include <BRepSweep_Revol.hxx>
#include <BRepTools.hxx>
#include <BRepTools_Modifier.hxx>
#include <BRepTools_Quilt.hxx>
#include <BRepTools_TrsfModification.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAPI_ExtremaCurveCurve.hxx>
#include <Geom2dInt_GInter.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomProjLib.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>
#include <IntRes2d_IntersectionPoint.hxx>
#include <MAT2d_CutCurve.hxx>
#include <MAT_Arc.hxx>
#include <MAT_BasicElt.hxx>
#include <MAT_Graph.hxx>
#include <MAT_Node.hxx>
#include <MAT_Side.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_NotImplemented.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <TColStd_SequenceOfReal.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeSequenceOfShape.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

// #define DRAW
#ifdef DRAW
  #include <DBRep.hxx>
  #include <DrawTrSurf.hxx>
  #include <stdio.h>
static Standard_Boolean AffichGeom  = Standard_False;
static Standard_Boolean AffichEdge  = Standard_False;
static Standard_Integer NbFACES     = 0;
static Standard_Integer NbTRIMFACES = 0;
static Standard_Integer NbVEVOS     = 0;
static Standard_Integer NbPROFILS   = 0;
static Standard_Integer NbEDGES     = 0;
#endif

static Standard_Real BRepFill_Confusion()
{
  Standard_Real Tol = 1.e-6;
  return Tol;
}

static const TopoWire PutProfilAt(const TopoWire&     ProfRef,
                                     const Ax3&          AxeRef,
                                     const TopoEdge&     E,
                                     const TopoFace&     F,
                                     const Standard_Boolean AtStart);

static void TrimFace(const TopoFace&        Face,
                     TopTools_SequenceOfShape& TheEdges,
                     TopTools_SequenceOfShape& S);

static void TrimEdge(const TopoEdge&              Edge,
                     const TopTools_SequenceOfShape& TheEdgesControle,
                     TopTools_SequenceOfShape&       TheVer,
                     TColStd_SequenceOfReal&         ThePar,
                     TopTools_SequenceOfShape&       S);

static Standard_Integer PosOnFace(Standard_Real d1, Standard_Real d2, Standard_Real d3);

static void ComputeIntervals(const TopTools_SequenceOfShape& VonF,
                             const TopTools_SequenceOfShape& VOnL,
                             const TColgp_SequenceOfPnt&     ParOnF,
                             const TColgp_SequenceOfPnt&     ParOnL,
                             const BRepFill_TrimSurfaceTool& Trim,
                             const Handle(GeomCurve2d)&     Bis,
                             const TopoVertex&            VS,
                             const TopoVertex&            VE,
                             TColStd_SequenceOfReal&         FirstPar,
                             TColStd_SequenceOfReal&         LastPar,
                             TopTools_SequenceOfShape&       FirstV,
                             TopTools_SequenceOfShape&       LastV);

static Standard_Real DistanceToOZ(const TopoVertex& V);

static Standard_Real Altitud(const TopoVertex& V);

static Standard_Boolean DoubleOrNotInFace(const TopTools_SequenceOfShape& EC,
                                          const TopoVertex&            V);

static void SimpleExpression(const Bisector_Bisec& B, Handle(GeomCurve2d)& Bis);

static TopAbs_Orientation Relative(const TopoWire&   W1,
                                   const TopoWire&   W2,
                                   const TopoVertex& V,
                                   Standard_Boolean&    Commun);

static void CutEdge(const TopoEdge& E, const TopoFace& F, ShapeList& Cuts);

static void CutEdgeProf(const TopoEdge&            E,
                        const Handle(GeomPlane)&     Plane,
                        const Handle(Geom2d_Line)&    Line,
                        ShapeList&         Cuts,
                        TopTools_DataMapOfShapeShape& MapVerRefMoved);

static Standard_Integer VertexFromNode(const Handle(MAT_Node)&                    aNode,
                                       const TopoEdge&                         E,
                                       const TopoVertex&                       VF,
                                       const TopoVertex&                       VL,
                                       BRepFill_DataMapOfNodeDataMapOfShapeShape& MapNodeVertex,
                                       TopoVertex&                             VS);

//=================================================================================================

static void EdgeVertices(const TopoEdge& E, TopoVertex& V1, TopoVertex& V2)
{
  if (E.Orientation() == TopAbs_REVERSED)
  {
    TopExp1::Vertices(E, V2, V1);
  }
  else
  {
    TopExp1::Vertices(E, V1, V2);
  }
}

//=================================================================================================

BRepFill_Evolved::BRepFill_Evolved()
    : myIsDone(Standard_False),
      mySpineType(Standard_True)
{
}

//=================================================================================================

BRepFill_Evolved::BRepFill_Evolved(const TopoWire&     Spine,
                                   const TopoWire&     Profile,
                                   const Ax3&          AxeProf,
                                   const GeomAbs_JoinType Join,
                                   const Standard_Boolean Solid)

    : myIsDone(Standard_False)
{
  Perform(Spine, Profile, AxeProf, Join, Solid);
}

//=================================================================================================

BRepFill_Evolved::BRepFill_Evolved(const TopoFace&     Spine,
                                   const TopoWire&     Profile,
                                   const Ax3&          AxeProf,
                                   const GeomAbs_JoinType Join,
                                   const Standard_Boolean Solid)
    : myIsDone(Standard_False)
{
  Perform(Spine, Profile, AxeProf, Join, Solid);
}

//=================================================================================================

static Standard_Boolean IsVertical(const TopoEdge& E)
{
  TopoVertex V1, V2;
  TopExp1::Vertices(E, V1, V2);
  Point3d P1 = BRepInspector::Pnt(V1);
  Point3d P2 = BRepInspector::Pnt(V2);

  if (Abs(P1.Y() - P2.Y()) < BRepFill_Confusion())
  {
    // It is a Line ?
    TopLoc_Location    Loc;
    Standard_Real      f, l;
    Handle(GeomCurve3d) GC = BRepInspector::Curve(E, Loc, f, l);
    if (GC->DynamicType() == STANDARD_TYPE(GeomLine))
      return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

static Standard_Boolean IsPlanar(const TopoEdge& E)
{
  TopoVertex V1, V2;
  TopExp1::Vertices(E, V1, V2);
  Point3d P1 = BRepInspector::Pnt(V1);
  Point3d P2 = BRepInspector::Pnt(V2);

  if (Abs(P1.Z() - P2.Z()) < BRepFill_Confusion())
  {
    // It is a Line ?
    TopLoc_Location    Loc;
    Standard_Real      f, l;
    Handle(GeomCurve3d) GC = BRepInspector::Curve(E, Loc, f, l);
    if (GC->DynamicType() == STANDARD_TYPE(GeomLine))
      return Standard_True;
  }
  return Standard_False;
}

//=======================================================================
// function : Side
// purpose  : determine the position of the profil correspondingly to plane XOZ.
//           Return 1 : MAT_Left.
//           Return 2 : MAT_Left and Planar.
//           Return 3 : MAT_Left and Vertical.
//           Return 4 : MAT_Right.
//           Return 5 : MAT_Right and Planar.
//           Return 6 : MAT_Right and Vertical.
//=======================================================================

static Standard_Integer Side(const TopoWire& Profil, const Standard_Real Tol)
{
  TopoVertex V1, V2;
  // Rem : it is enough to test the first edge of the Wire.
  //       ( Correctly cut in PrepareProfil)
  ShapeExplorer Explo(Profil, TopAbs_EDGE);

  Standard_Integer   TheSide;
  const TopoEdge& E = TopoDS::Edge(Explo.Current());

  TopExp1::Vertices(E, V1, V2);
  Point3d P1 = BRepInspector::Pnt(V1);
  Point3d P2 = BRepInspector::Pnt(V2);

  if (P1.Y() < -Tol || P2.Y() < -Tol)
    TheSide = 4;
  else
    TheSide = 1;
  if (IsVertical(E))
    TheSide += 2;
  else if (IsPlanar(E))
    TheSide++;
  return TheSide;
}

//=================================================================================================

void BRepFill_Evolved::Perform(const TopoWire&     Spine,
                               const TopoWire&     Profile,
                               const Ax3&          AxeProf,
                               const GeomAbs_JoinType Join,
                               const Standard_Boolean Solid)
{
  mySpineType       = Standard_False;
  TopoFace aFace = BRepLib_MakeFace(Spine, Standard_True);
  PrivatePerform(aFace, Profile, AxeProf, Join, Solid);
}

//=================================================================================================

void BRepFill_Evolved::Perform(const TopoFace&     Spine,
                               const TopoWire&     Profile,
                               const Ax3&          AxeProf,
                               const GeomAbs_JoinType Join,
                               const Standard_Boolean Solid)
{
  mySpineType = Standard_True;
  PrivatePerform(Spine, Profile, AxeProf, Join, Solid);
}

//=================================================================================================

void BRepFill_Evolved::PrivatePerform(const TopoFace&     Spine,
                                      const TopoWire&     Profile,
                                      const Ax3&          AxeProf,
                                      const GeomAbs_JoinType Join,
                                      const Standard_Boolean Solid)
{
  TopoShape aLocalShape = Spine.Oriented(TopAbs_FORWARD);
  mySpine                  = TopoDS::Face(aLocalShape);
  //  mySpine    = TopoDS::Face(Spine.Oriented(TopAbs_FORWARD));
  aLocalShape = Profile.Oriented(TopAbs_FORWARD);
  myProfile   = TopoDS::Wire(aLocalShape);
  //  myProfile  = TopoDS::Wire(Profile.Oriented(TopAbs_FORWARD));
  myJoinType = Join;
  myMap.Clear();

  if (myJoinType > GeomAbs_Arc)
  {
    throw Standard_NotImplemented();
  }

  ShapeList               WorkProf;
  TopoFace                        WorkSpine;
  TopTools_ListIteratorOfListOfShape WPIte;

  //-------------------------------------------------------------------
  // Positioning of mySpine and myProfil in the workspace.
  //-------------------------------------------------------------------
  TopLoc_Location LSpine = FindLocation(mySpine);
  Transform3d         T;
  T.SetTransformation(AxeProf);
  TopLoc_Location LProfile(T);
  TopLoc_Location InitLS = mySpine.Location();
  TopLoc_Location InitLP = myProfile.Location();
  TransformInitWork(LSpine, LProfile);

  //------------------------------------------------------------------
  // projection of the profile and cut of the spine.
  //------------------------------------------------------------------
  TopTools_DataMapOfShapeShape MapProf, MapSpine;

  PrepareProfile(WorkProf, MapProf);
  PrepareSpine(WorkSpine, MapSpine);

  Standard_Real    Tol     = BRepFill_Confusion();
  Standard_Boolean YaLeft  = Standard_False;
  Standard_Boolean YaRight = Standard_False;
  TopoWire      SP;

  for (WPIte.Initialize(WorkProf); WPIte.More(); WPIte.Next())
  {
    SP = TopoDS::Wire(WPIte.Value());
    if (Side(SP, Tol) < 4)
      YaLeft = Standard_True;
    else
      YaRight = Standard_True;
    if (YaLeft && YaRight)
      break;
  }

  TopoFace              Face;
  BRepMAT2d_BisectingLocus Locus;

  //----------------------------------------------------------
  // Initialisation of cut volevo.
  // For each part of the profile create a volevo added to CutVevo
  //----------------------------------------------------------
  BRepFill_Evolved       CutVevo;
  TopoWire            WP;
  ShapeBuilder           BB;
  BRepTools_WireExplorer WExp;

  BB.MakeWire(WP);

  for (WPIte.Initialize(WorkProf); WPIte.More(); WPIte.Next())
  {
    for (WExp.Init(TopoDS::Wire(WPIte.Value())); WExp.More(); WExp.Next())
    {
      BB.Add(WP, WExp.Current());
    }
  }
  CutVevo.SetWork(WorkSpine, WP);

  ShapeQuilt  Glue;
  Standard_Integer CSide;

  //---------------------------------
  // Construction of vevos to the left.
  //---------------------------------
  if (YaLeft)
  {
    //-----------------------------------------------------
    // Calculate the map of bisector locations at the left.
    // and links Topology -> base elements of the map.
    //-----------------------------------------------------
    BRepMAT2d_Explorer Exp(WorkSpine);
    Locus.Compute(Exp, 1, MAT_Left);
    BRepMAT2d_LinkTopoBilo Link1(Exp, Locus);

    for (WPIte.Initialize(WorkProf); WPIte.More(); WPIte.Next())
    {
      SP    = TopoDS::Wire(WPIte.Value());
      CSide = Side(SP, Tol);
      //-----------------------------------------------
      // Construction and adding of elementary volevo.
      //-----------------------------------------------
      BRepFill_Evolved Vevo;
      if (CSide == 1)
      {
        Vevo.ElementaryPerform(WorkSpine, SP, Locus, Link1, Join);
      }
      else if (CSide == 2)
      {
        Vevo.PlanarPerform(WorkSpine, SP, Locus, Link1, Join);
      }
      else if (CSide == 3)
      {
        Vevo.VerticalPerform(WorkSpine, SP, Locus, Link1, Join);
      }
      CutVevo.Add(Vevo, SP, Glue);
    }
  }

  //---------------------------------
  // Construction of vevos to the right.
  //---------------------------------
  if (YaRight)
  {
    //-----------------------------------
    // Decomposition of the face into wires.
    //-----------------------------------
    ShapeExplorer SpineExp(WorkSpine, TopAbs_WIRE);
    for (; SpineExp.More(); SpineExp.Next())
    {
      //----------------------------------------------
      // Calculate the map to the right of the current wire.
      //----------------------------------------------
      BRepLib_MakeFace B(gp_Pln(0., 0., 1., 0.));
      TopoShape     aLocalShapeRev = SpineExp.Current().Reversed();
      B.Add(TopoDS::Wire(aLocalShapeRev));
      //      B.Add(TopoDS::Wire(SpineExp.Current().Reversed()));
      Face = B.Face();
      BRepMAT2d_Explorer Exp(Face);
      Locus.Compute(Exp, 1, MAT_Left);
      BRepMAT2d_LinkTopoBilo Link1(Exp, Locus);

      for (WPIte.Initialize(WorkProf); WPIte.More(); WPIte.Next())
      {
        SP    = TopoDS::Wire(WPIte.Value());
        CSide = Side(SP, Tol);
        //-----------------------------------------------
        // Construction and adding of an elementary volevo
        //-----------------------------------------------
        BRepFill_Evolved Vevo;
        if (CSide == 4)
        {
          Vevo.ElementaryPerform(Face, SP, Locus, Link1, Join);
        }
        else if (CSide == 5)
        {
          Vevo.PlanarPerform(Face, SP, Locus, Link1, Join);
        }
        else if (CSide == 6)
        {
          Vevo.VerticalPerform(Face, SP, Locus, Link1, Join);
        }
        CutVevo.Add(Vevo, SP, Glue);
      }
    }
  }

  if (Solid)
    CutVevo.AddTopAndBottom(Glue);

  //-------------------------------------------------------------------------
  // Gluing of regularites on parallel edges generate4d by vertices of the
  // cut of the profile.
  //-------------------------------------------------------------------------
  CutVevo.ContinuityOnOffsetEdge(WorkProf);

  //-----------------------------------------------------------------
  // construction of the shape via the quilt, ie:
  // - sharing of topologies of elementary added volevos.
  // - Orientation of faces correspondingly to each other.
  //-----------------------------------------------------------------
  TopoShape& SCV = CutVevo.ChangeShape();
  SCV               = Glue.Shells();
  //------------------------------------------------------------------------
  // Transfer of the map of generated elements and of the shape of Cutvevo
  // in myMap and repositioning in the initial space.
  //------------------------------------------------------------------------
  Transfert(CutVevo, MapProf, MapSpine, LSpine.Inverted(), InitLS, InitLP);

  // Orientation of the solid.
  if (Solid)
    MakeSolid();

  //  modified by NIZHNY-EAP Mon Jan 24 11:26:48 2000 ___BEGIN___
  BRepLib::UpdateTolerances(myShape, Standard_False);
  //  modified by NIZHNY-EAP Mon Jan 24 11:26:50 2000 ___END___
  myIsDone = Standard_True;
}

//=================================================================================================

static void IsInversed(const TopoShape& S,
                       const TopoEdge&  E1,
                       const TopoEdge&  E2,
                       Standard_Boolean*   Inverse)
{

  Inverse[0] = Inverse[1] = 0;
  if (S.ShapeType() != TopAbs_EDGE)
    return;

  Point3d            P;
  Vector3d            DS, DC1, DC2;
  BRepAdaptor_Curve CS(TopoDS::Edge(S));
  if (S.Orientation() == TopAbs_FORWARD)
  {
    CS.D1(CS.FirstParameter(), P, DS);
  }
  else
  {
    CS.D1(CS.LastParameter(), P, DS);
    DS.Reverse();
  }

  if (!BRepInspector::Degenerated(E1))
  {
    BRepAdaptor_Curve C1(TopoDS::Edge(E1));
    if (E1.Orientation() == TopAbs_FORWARD)
    {
      C1.D1(C1.FirstParameter(), P, DC1);
    }
    else
    {
      C1.D1(C1.LastParameter(), P, DC1);
      DC1.Reverse();
    }
    Inverse[0] = (DS.Dot(DC1) < 0.);
  }
  else
    Inverse[0] = 1;

  if (!BRepInspector::Degenerated(E2))
  {
    BRepAdaptor_Curve C2(TopoDS::Edge(E2));
    if (E2.Orientation() == TopAbs_FORWARD)
    {
      C2.D1(C2.FirstParameter(), P, DC2);
    }
    else
    {
      C2.D1(C2.LastParameter(), P, DC2);
      DC2.Reverse();
    }
    Inverse[1] = (DS.Dot(DC2) < 0.);
  }
  else
    Inverse[1] = 1;
}

//=================================================================================================

void BRepFill_Evolved::SetWork(const TopoFace& Sp, const TopoWire& Pr)
{
  mySpine   = Sp;
  myProfile = Pr;
}

//=======================================================================
// function : ConcaveSide
// purpose  : Determine if the pipes were at the side of the
//           concavity. In this case they can be closed.
//           WARNING: Not finished. Done only for circles.
//=======================================================================

static Standard_Boolean ConcaveSide(const TopoShape& S, const TopoFace& F)
{

  if (S.ShapeType() == TopAbs_VERTEX)
    return Standard_False;

  if (S.ShapeType() == TopAbs_EDGE)
  {
    Standard_Real        f, l;
    Handle(GeomCurve2d) G2d = BRepInspector::CurveOnSurface(TopoDS::Edge(S), F, f, l);
    Handle(GeomCurve2d) G2dOC;

    Geom2dAdaptor_Curve AC(G2d, f, l);
    if (AC.GetType() == GeomAbs_Circle)
    {
      Standard_Boolean Direct = AC.Circle().IsDirect();
      if (S.Orientation() == TopAbs_REVERSED)
        Direct = (!Direct);
      return Direct;
    }
  }
  return Standard_False;
}

//=================================================================================================

void BRepFill_Evolved::ElementaryPerform(const TopoFace&              Sp,
                                         const TopoWire&              Pr,
                                         const BRepMAT2d_BisectingLocus& Locus,
                                         BRepMAT2d_LinkTopoBilo&         Link1,
                                         const GeomAbs_JoinType /*Join*/)
{

#ifdef DRAW
  if (AffichEdge)
  {
    char name[100];
    sprintf(name, "PROFIL_%d", ++NbPROFILS);
    DBRep1::Set(name, Pr);
  }
#endif
  TopoShape aLocalShape = Sp.Oriented(TopAbs_FORWARD);
  mySpine                  = TopoDS::Face(aLocalShape);
  //  mySpine   = TopoDS::Face(Sp.Oriented(TopAbs_FORWARD));
  myProfile = Pr;
  myMap.Clear();

  ShapeBuilder myBuilder;
  myBuilder.MakeCompound(TopoDS::Compound(myShape));

  //---------------------------------------------------------------------
  // MapNodeVertex : associate to each node of the map (key1) and
  //                 to each element of the profile (key2) a vertex (item).
  // MapBis        : a set of edges or vertexes (item) generated by
  //                 a bisectrice on a face or an edge (key) of
  //                 tubes or revolutions.
  // MapVerPar     : Map of parameters of vertices on parallel edges
  //                 the list contained in MapVerPar (E) corresponds
  //                 to parameters on E of vertices contained in  MapBis(E);
  // MapBS         : links BasicElt of the map => Topology of the spine.
  //---------------------------------------------------------------------

  BRepFill_DataMapOfNodeDataMapOfShapeShape MapNodeVertex;
  TopTools_DataMapOfShapeSequenceOfShape    MapBis;
  BRepFill_DataMapOfShapeSequenceOfReal     MapVerPar;

  TopTools_DataMapOfShapeShape EmptyMap;
  TopTools_SequenceOfShape     EmptySeq;
  ShapeList         EmptyList;
  TColStd_SequenceOfReal       EmptySeqOfReal;

  // mark of the profile.
  Ax3 AxeRef(Point3d(0., 0., 0.), Dir3d(0., 0., 1.), Dir3d(1., 0., 0.));

  //---------------------------------------------------------------
  // Construction of revolutions and tubes.
  //---------------------------------------------------------------
  BRepTools_WireExplorer ProfExp;
  ShapeExplorer        FaceExp;
  BRepTools_WireExplorer WireExp;

  for (FaceExp.Init(mySpine, TopAbs_WIRE); FaceExp.More(); FaceExp.Next())
  {

    for (WireExp.Init(TopoDS::Wire(FaceExp.Current())); WireExp.More(); WireExp.Next())
    {

      const TopoEdge& CurrentEdge = WireExp.Current();
      TopoVertex      VFirst, VLast;
      EdgeVertices(CurrentEdge, VFirst, VLast);

      for (Link1.Init(VLast); Link1.More(); Link1.Next())
      {
        //----------------------------.
        // Construction of a Revolution
        //----------------------------.
        MakeRevol(CurrentEdge, VLast, AxeRef);
      }

      for (Link1.Init(CurrentEdge); Link1.More(); Link1.Next())
      {
        //------------------------.
        // Construction of a Tube
        //-------------------------
        MakePipe(CurrentEdge, AxeRef);
      }
    }
  }

#ifdef DRAW
  if (AffichEdge)
  {
    std::cout << " End Construction of geometric primitives" << std::endl;
  }
#endif

  //---------------------------------------------------
  // Construction of edges associated to bissectrices.
  //---------------------------------------------------
  Handle(MAT_Arc)      CurrentArc;
  Handle(GeomCurve2d) Bis, PCurve1, PCurve2;
  Handle(GeomCurve3d)   CBis;
  Standard_Boolean     Reverse;
  TopoEdge          CurrentEdge;
  TopoShape         S[2];
  TopoFace          F[2];
  TopoEdge          E[4];
  TopLoc_Location      L;
  Standard_Integer     k;

  for (Standard_Integer i = 1; i <= Locus.Graph()->NumberOfArcs(); i++)
  {
    CurrentArc = Locus.Graph()->Arc(i);
    SimpleExpression(Locus.GeomBis(CurrentArc, Reverse), Bis);

    //------------------------------------------------------------------
    // Return elements of the spine corresponding to separate basicElts.
    //------------------------------------------------------------------
    S[0] = Link1.GeneratingShape(CurrentArc->FirstElement());
    S[1] = Link1.GeneratingShape(CurrentArc->SecondElement());

    Standard_Boolean Concave0 = ConcaveSide(S[0], mySpine);
    Standard_Boolean Concave1 = ConcaveSide(S[1], mySpine);

    TopTools_SequenceOfShape VOnF, VOnL;
    TColgp_SequenceOfPnt     ParOnF, ParOnL;

    TopTools_DataMapOfShapeSequenceOfShape MapSeqVer;
    BRepFill_DataMapOfShapeSequenceOfPnt   MapSeqPar;

    for (ProfExp.Init(myProfile); ProfExp.More(); ProfExp.Next())
    {
      //-----------------------------------------------
      // Return two faces separated by the bissectrice.
      //-----------------------------------------------
      F[0] = TopoDS::Face(myMap(S[0])(ProfExp.Current()).First());
      F[1] = TopoDS::Face(myMap(S[1])(ProfExp.Current()).First());

      //------------------------------------
      // Return parallel edges on each face.
      //------------------------------------
      TopoVertex VF, VL;

      EdgeVertices(ProfExp.Current(), VF, VL);

      E[0] = TopoDS::Edge(myMap(S[0])(VF).First());
      E[1] = TopoDS::Edge(myMap(S[0])(VL).First());
      E[2] = TopoDS::Edge(myMap(S[1])(VF).First());
      E[3] = TopoDS::Edge(myMap(S[1])(VL).First());

      Standard_Boolean Inv0[2];
      Standard_Boolean Inv1[2];

      Inv0[0] = Inv0[1] = Inv1[0] = Inv1[1] = 0;
      if (Concave0)
        IsInversed(S[0], E[0], E[1], Inv0);
      if (Concave1)
        IsInversed(S[1], E[2], E[3], Inv1);

      //---------------------------------------------
      // Construction of geometries.
      //---------------------------------------------
      BRepFill_TrimSurfaceTool Trim(Bis, F[0], F[1], E[0], E[2], Inv0[0], Inv1[0]);
      //-----------------------------------------------------------
      // Construction of vertices corresponding to the node of the map
      //-----------------------------------------------------------
      TopoVertex    VS, VE;
      Handle(MAT_Node) Node1, Node2;

      if (Reverse)
      {
        Node1 = CurrentArc->SecondNode();
        Node2 = CurrentArc->FirstNode();
      }
      else
      {
        Node1 = CurrentArc->FirstNode();
        Node2 = CurrentArc->SecondNode();
      }
      //--------------------------------------------------------
      // Particular case when the node is on a vertex of the spine.
      //--------------------------------------------------------
      if (Node1->OnBasicElt())
      {
        if (S[0].ShapeType() == TopAbs_VERTEX)
        {
          Node1 = CurrentArc->FirstElement()->StartArc()->FirstNode();
        }
        else if (S[1].ShapeType() == TopAbs_VERTEX)
        {
          Node1 = CurrentArc->SecondElement()->StartArc()->FirstNode();
        }
      }
      // End of particular case.

      Standard_Integer StartOnF =
        VertexFromNode(Node1, TopoDS::Edge(ProfExp.Current()), VF, VL, MapNodeVertex, VS);

      Standard_Integer EndOnF =
        VertexFromNode(Node2, TopoDS::Edge(ProfExp.Current()), VF, VL, MapNodeVertex, VE);

      //-----------------------------------------------------------
      // Construction of vertices on edges parallel to the spine.
      //-----------------------------------------------------------
      if (!MapSeqVer.IsBound(VF))
      {
        if (Inv0[0] || Inv1[0])
        {
          ParOnF.Clear();
          VOnF.Clear();
        }
        else
        {
          Trim.IntersectWith(E[0], E[2], ParOnF);
          VOnF.Clear();
          for (Standard_Integer s = 1; s <= ParOnF.Length(); s++)
          {
            TopoVertex VC;
            myBuilder.MakeVertex(VC);
            VOnF.Append(VC);
          }
          if (StartOnF == 1)
          {
            VOnF.SetValue(1, VS);
          }
          if (EndOnF == 1)
          {
            VOnF.SetValue(ParOnF.Length(), VE);
          }
        }
      }
      else
      {
        ParOnF = MapSeqPar(VF);
        VOnF   = MapSeqVer(VF);
      }

      if (!MapSeqVer.IsBound(VL))
      {
        if (Inv0[1] || Inv1[1])
        {
          ParOnL.Clear();
          VOnL.Clear();
        }
        else
        {
          Trim.IntersectWith(E[1], E[3], ParOnL);
          VOnL.Clear();
          for (Standard_Integer s = 1; s <= ParOnL.Length(); s++)
          {
            TopoVertex VC;
            myBuilder.MakeVertex(VC);
            VOnL.Append(VC);
          }
          if (StartOnF == 3)
          {
            VOnL.SetValue(1, VS);
          }
          if (EndOnF == 3)
          {
            VOnL.SetValue(ParOnL.Length(), VE);
          }
        }
      }
      else
      {
        ParOnL = MapSeqPar(VL);
        VOnL   = MapSeqVer(VL);
      }

      //------------------------------------------------------
      // Test if the Bissectrice is not projected on the face
      //------------------------------------------------------
      if ((StartOnF == 0) && (EndOnF == 0) && VOnL.IsEmpty() && VOnF.IsEmpty())
        // No trace of the bisectrice on the face.
        continue;

      if ((StartOnF == 0) && (EndOnF == 0) && (VOnL.Length() + VOnF.Length() == 1))
        // the first or last node of the arc is on the edge
        // but the arc is not on the face.
        continue;

      //---------------------------------------------------------
      // determine the intervals of the bissectrice that are
      // projected on F[0] and F[1].
      //---------------------------------------------------------
      TColStd_SequenceOfReal   LastPar, FirstPar;
      TopTools_SequenceOfShape FirstV, LastV;

      ComputeIntervals(VOnF,
                       VOnL,
                       ParOnF,
                       ParOnL,
                       Trim,
                       Bis,
                       VS,
                       VE,
                       FirstPar,
                       LastPar,
                       FirstV,
                       LastV);

      for (Standard_Integer Ti = 1; Ti <= FirstPar.Length(); Ti++)
      {
        TopoVertex V1 = TopoDS::Vertex(FirstV.Value(Ti));
        TopoVertex V2 = TopoDS::Vertex(LastV.Value(Ti));

        GeomAbs_Shape Continuity;

        Trim.Project(FirstPar.Value(Ti), LastPar.Value(Ti), CBis, PCurve1, PCurve2, Continuity);

        //-------------------------------------
        // Coding of the edge.
        //-------------------------------------
        myBuilder.MakeEdge(CurrentEdge, CBis, BRepFill_Confusion());

        myBuilder.UpdateVertex(V1, CBis->Value(CBis->FirstParameter()), BRepFill_Confusion());
        myBuilder.UpdateVertex(V2, CBis->Value(CBis->LastParameter()), BRepFill_Confusion());

        myBuilder.Add(CurrentEdge, V1.Oriented(TopAbs_FORWARD));
        myBuilder.Add(CurrentEdge, V2.Oriented(TopAbs_REVERSED));

        myBuilder.Range(CurrentEdge, CBis->FirstParameter(), CBis->LastParameter());
        myBuilder.UpdateEdge(CurrentEdge, PCurve1, F[0], BRepFill_Confusion());
        myBuilder.UpdateEdge(CurrentEdge, PCurve2, F[1], BRepFill_Confusion());

        myBuilder.Continuity(CurrentEdge, F[0], F[1], Continuity);

#ifdef DRAW
        if (AffichEdge)
        {
          char name[100];
          sprintf(name, "ARCEDGE_%d_%d_%d", i, vv, Ti);
          DBRep1::Set(name, CurrentEdge);
        }
#endif
        //-------------------------------------------
        // Storage of the edge for each of faces.
        //-------------------------------------------
        for (k = 0; k <= 1; k++)
        {
          if (!MapBis.IsBound(F[k]))
          {
            MapBis.Bind(F[k], EmptySeq);
          }
        }
        //---------------------------------------------------------------
        // orientation of the edge depends on the direction of the skin.
        // skin => same orientation E[0] , inverted orientation E[2]
        // if contreskin it is inverted.
        //--------------------------------------------------------------
        E[0].Orientation(BRepTools1::OriEdgeInFace(E[0], F[0]));
        E[2].Orientation(BRepTools1::OriEdgeInFace(E[2], F[1]));

        if (DistanceToOZ(VF) < DistanceToOZ(VL))
        {
          // Skin
          MapBis(F[0]).Append(CurrentEdge.Oriented(E[0].Orientation()));
          CurrentEdge.Orientation(TopAbs1::Complement(E[2].Orientation()));
          MapBis(F[1]).Append(CurrentEdge);
        }
        else
        {
          // Contreskin
          MapBis(F[1]).Append(CurrentEdge.Oriented(E[2].Orientation()));
          CurrentEdge.Orientation(TopAbs1::Complement(E[0].Orientation()));
          MapBis(F[0]).Append(CurrentEdge);
        }
      }

      //----------------------------------------------
      // Storage of vertices on parallel edges.
      // fill MapBis and MapVerPar.
      // VOnF for E[0] and E[2].
      // VOnL for E[1] and E[3].
      //----------------------------------------------
      for (k = 0; k <= 2; k = k + 2)
      {
        if (!MapSeqVer.IsBound(VF))
        {
          if (!VOnF.IsEmpty())
          {
            if (!MapBis.IsBound(E[k]))
            {
              MapBis.Bind(E[k], EmptySeq);
              MapVerPar.Bind(E[k], EmptySeqOfReal);
            }
            for (Standard_Integer ii = 1; ii <= VOnF.Length(); ii++)
            {
              MapBis(E[k]).Append(VOnF.Value(ii));
              if (k == 0)
                MapVerPar(E[k]).Append(ParOnF.Value(ii).Y());
              else
                MapVerPar(E[k]).Append(ParOnF.Value(ii).Z());
            }
          }
        }
      }

      for (k = 1; k <= 3; k = k + 2)
      {
        if (!MapSeqVer.IsBound(VL))
        {
          if (!VOnL.IsEmpty())
          {
            if (!MapBis.IsBound(E[k]))
            {
              MapBis.Bind(E[k], EmptySeq);
              MapVerPar.Bind(E[k], EmptySeqOfReal);
            }
            for (Standard_Integer ii = 1; ii <= VOnL.Length(); ii++)
            {
              MapBis(E[k]).Append(VOnL.Value(ii));
              if (k == 1)
                MapVerPar(E[k]).Append(ParOnL.Value(ii).Y());
              else
                MapVerPar(E[k]).Append(ParOnL.Value(ii).Z());
            }
          }
        }
      }

      //----------------------------------------------------------------
      // Edge [1] of the current face will be Edge [0] of the next face.
      // => copy of VonL in VonF. To avoid creating the same vertices twice.
      //-----------------------------------------------------------------

      MapSeqPar.Bind(VF, ParOnF);
      MapSeqVer.Bind(VF, VOnF);
      MapSeqPar.Bind(VL, ParOnL);
      MapSeqVer.Bind(VL, VOnL);
    }
  }

#ifdef DRAW
  if (AffichEdge)
  {
    std::cout << " End of Construction of edges and vertices on bissectrices" << std::endl;
  }
#endif

  //----------------------------------
  // Construction of parallel edges.
  //----------------------------------
  BRepFill_DataMapIteratorOfDataMapOfShapeDataMapOfShapeListOfShape ite1;
  TopoShape                                                      CurrentProf, PrecProf;
  TopoFace                                                       CurrentFace;
  TopoShape                                                      CurrentSpine;
  TopoVertex                                                     VCF, VCL;

  for (ite1.Initialize(myMap); ite1.More(); ite1.Next())
  {
    CurrentSpine = ite1.Key();

    for (ProfExp.Init(myProfile); ProfExp.More(); ProfExp.Next())
    {
      CurrentProf = ProfExp.Current();
      EdgeVertices(TopoDS::Edge(CurrentProf), VCF, VCL);
      CurrentEdge = TopoDS::Edge(myMap(CurrentSpine)(VCF).First());

      //-------------------------------------------------------------
      // RQ : Current Edge is oriented relatively to the face (oriented forward)
      //     generated by edge CurrentProf .
      //-------------------------------------------------------------
      if (MapBis.IsBound(CurrentEdge))
      {

        //--------------------------------------------------------
        // Find if one of two faces connected to the edge
        // belongs to volevo. The edges on this face serve
        // to eliminate certain vertices that can appear twice
        // on the parallel edge. These Vertices correspond to the
        // nodes of the map.
        //---------------------------------------------------------
        TopoShape     FaceControle;
        Standard_Boolean YaFace = Standard_True;

        FaceControle = myMap(CurrentSpine)(CurrentProf).First();
        if (!MapBis.IsBound(FaceControle))
        {
          YaFace = Standard_False;
          if (!PrecProf.IsNull())
          {
            FaceControle = myMap(CurrentSpine)(PrecProf).First();
            if (MapBis.IsBound(FaceControle))
            {
              YaFace = Standard_True;
            }
          }
        }

        if (YaFace)
        {
          //------------------------------------------------------------
          // No connected face in the volevo => no parallel edge.
          //------------------------------------------------------------
          TopTools_SequenceOfShape aSeqOfShape;
          TrimEdge(CurrentEdge,
                   MapBis(FaceControle),
                   MapBis(CurrentEdge),
                   MapVerPar(CurrentEdge),
                   aSeqOfShape);

          for (k = 1; k <= aSeqOfShape.Length(); k++)
          {
            myMap(CurrentSpine)(VCF).Append(aSeqOfShape.Value(k));

#ifdef DRAW
            if (AffichEdge)
            {
              char name[100];
              sprintf(name, "PAREDGE_%d_%d", ++NbEDGES, k);
              DBRep1::Set(name, aSeqOfShape.Value(k));
            }
#endif
          }
        }
      }
      PrecProf = CurrentProf;
    }

    //------------------------------------------------------------
    // Construction of the parallel edge from the last vertex of myProfile.
    //------------------------------------------------------------
    CurrentEdge = TopoDS::Edge(myMap(CurrentSpine)(VCL).First());

    if (MapBis.IsBound(CurrentEdge))
    {
      Standard_Boolean YaFace = Standard_True;
      TopoShape     FaceControle;

      FaceControle = myMap(CurrentSpine)(CurrentProf).First();
      if (!MapBis.IsBound(FaceControle))
      {
        YaFace = Standard_False;
      }
      // the number of element of the list allows to know
      // if the edges have already been done (closed profile) .
      if (YaFace && myMap(CurrentSpine)(VCL).Extent() <= 1)
      {
        TopTools_SequenceOfShape aSeqOfShape;
        TrimEdge(CurrentEdge,
                 MapBis(FaceControle),
                 MapBis(CurrentEdge),
                 MapVerPar(CurrentEdge),
                 aSeqOfShape);

        for (k = 1; k <= aSeqOfShape.Length(); k++)
        {
          myMap(CurrentSpine)(VCL).Append(aSeqOfShape.Value(k));

#ifdef DRAW
          if (AffichEdge)
          {
            char name[100];
            sprintf(name, "PAREDGE_%d_%d", ++NbEDGES, k);
            DBRep1::Set(name, aSeqOfShape.Value(k));
          }
#endif
        }
      }
    }
  }

#ifdef DRAW
  if (AffichEdge)
  {
    std::cout << " End Construction of parallel edges " << std::endl;
  }
#endif

  //-------------------------------------------------------------------
  // Cut faces by edges.
  //-------------------------------------------------------------------
  for (ite1.Initialize(myMap); ite1.More(); ite1.Next())
  {
    CurrentSpine = ite1.Key();

    for (ProfExp.Init(myProfile); ProfExp.More(); ProfExp.Next())
    {
      CurrentProf = ProfExp.Current();
      CurrentFace = TopoDS::Face(myMap(CurrentSpine)(CurrentProf).First());
      myMap(CurrentSpine)(CurrentProf).Clear();

      if (MapBis.IsBound(CurrentFace))
      {
        //----------------------------------------------------------
        // If the face does not contain edges that can limit it
        // it does not appear in volevo.
        // cut of face by edges can generate many faces.
        //
        // Add edges generated on the edges parallel to the set
        // of edges that limit the face.
        //
        //------------------------------------------------------------
        EdgeVertices(TopoDS::Edge(CurrentProf), VCF, VCL);

        TopTools_ListIteratorOfListOfShape itl;
        const ShapeList&        LF = myMap(CurrentSpine)(VCF);

        TopAbs_Orientation Ori = BRepTools1::OriEdgeInFace(TopoDS::Edge(LF.First()), CurrentFace);
        for (itl.Initialize(LF), itl.Next(); itl.More(); itl.Next())
        {
          TopoEdge RE = TopoDS::Edge(itl.Value());
          MapBis(CurrentFace).Append(RE.Oriented(Ori));
        }
        const ShapeList& LL = myMap(CurrentSpine)(VCL);
        Ori = BRepTools1::OriEdgeInFace(TopoDS::Edge(LL.First()), CurrentFace);
        for (itl.Initialize(LL), itl.Next(); itl.More(); itl.Next())
        {
          TopoEdge RE = TopoDS::Edge(itl.Value());
          MapBis(CurrentFace).Append(RE.Oriented(Ori));
        }

        // Cut of the face.
        TopTools_SequenceOfShape aSeqOfShape;

        TrimFace(CurrentFace, MapBis(CurrentFace), aSeqOfShape);

        for (Standard_Integer ii = 1; ii <= aSeqOfShape.Length(); ii++)
        {
          myBuilder.Add(myShape, aSeqOfShape.Value(ii));
          myMap(CurrentSpine)(CurrentProf).Append(aSeqOfShape.Value(ii));
        }
      }
    }
    //-----------------------------------------------------------------
    // Removal of first edge (edge of origin) from lists of myMap
    // corresponding to vertices of the profile.
    //-----------------------------------------------------------------
    ShapeExplorer     Explo(myProfile, TopAbs_VERTEX);
    TopTools_MapOfShape vmap;

    for (; Explo.More(); Explo.Next())
    {
      if (vmap.Add(Explo.Current()))
      {
        myMap(CurrentSpine)(Explo.Current()).RemoveFirst();
      }
    }
  }
  myIsDone = Standard_True;

#ifdef DRAW
  if (AffichEdge)
  {
    std::cout << " End of construction of an elementary volevo." << std::endl;
    char name[100];
    sprintf(name, "VEVO_%d", ++NbVEVOS);
    DBRep1::Set(name, myShape);
  }
#endif
}

//=================================================================================================

void BRepFill_Evolved::PlanarPerform(const TopoFace&              Sp,
                                     const TopoWire&              Pr,
                                     const BRepMAT2d_BisectingLocus& Locus,
                                     BRepMAT2d_LinkTopoBilo&         Link1,
                                     const GeomAbs_JoinType          Join)
{
  TopoShape aLocalShapeOriented = Sp.Oriented(TopAbs_FORWARD);
  mySpine                          = TopoDS::Face(aLocalShapeOriented);
  //  mySpine   = TopoDS::Face(Sp.Oriented(TopAbs_FORWARD));
  myProfile = Pr;
  myMap.Clear();

  ShapeBuilder B;
  B.MakeCompound(TopoDS::Compound(myShape));

  BRepTools_WireExplorer             ProfExp;
  ShapeExplorer                    Exp, exp1, exp2;
  TopTools_DataMapOfShapeListOfShape EmptyMap;
  ShapeList               EmptyList;
  TopTools_DataMapOfShapeShape       MapVP;
  BRepFill_OffsetWire                Paral;

  for (ProfExp.Init(myProfile); ProfExp.More(); ProfExp.Next())
  {
    const TopoEdge&       E = ProfExp.Current();
    BRepAlgo_FaceRestrictor  FR;
    OffsetAncestors OffAnc;

    TopoVertex V[2];
    EdgeVertices(E, V[0], V[1]);
    Standard_Real Alt = Altitud(V[0]);
    Standard_Real Offset[2];
    Offset[0]                = DistanceToOZ(V[0]);
    Offset[1]                = DistanceToOZ(V[1]);
    Standard_Boolean IsMinV1 = (Offset[0] < Offset[1]);

    for (Standard_Integer i = 0; i <= 1; i++)
    {
      if (!MapVP.IsBound(V[i]))
      {
        //------------------------------------------------
        // Calculate parallel lines corresponding to vertices.
        //------------------------------------------------
        Paral.PerformWithBiLo(mySpine, Offset[i], Locus, Link1, Join, Alt);
        OffAnc.Perform(Paral);
        MapVP.Bind(V[i], Paral.Shape());

        //-----------------------------
        // Update myMap (.)(V[i])
        //-----------------------------
        for (Exp.Init(Paral.Shape(), TopAbs_EDGE); Exp.More(); Exp.Next())
        {
          const TopoEdge&  WC = TopoDS::Edge(Exp.Current());
          const TopoShape& GS = OffAnc.Ancestor(WC);
          if (!myMap.IsBound(GS))
            myMap.Bind(GS, EmptyMap);
          if (!myMap(GS).IsBound(V[i]))
            myMap(GS).Bind(V[i], Paral.GeneratedShapes(GS));
        }
      }
      TopoShape Rest = MapVP(V[i]);

      Standard_Boolean ToReverse = Standard_False;
      if ((IsMinV1 && (i == 1)) || (!IsMinV1 && (i == 0)))
        ToReverse = Standard_True;

      if (!Rest.IsNull())
      {
        if (Rest.ShapeType() == TopAbs_WIRE)
        {
          if (ToReverse)
          {
            TopoShape aLocalShape = Rest.Reversed();
            TopoWire  aWire       = TopoDS::Wire(aLocalShape);
            FR.Add(aWire);
          }
          else
            FR.Add(TopoDS::Wire(Rest));
        }
        else
        {
          for (Exp.Init(Rest, TopAbs_WIRE); Exp.More(); Exp.Next())
          {
            TopoWire WCop = TopoDS::Wire(Exp.Current());
            if (ToReverse)
            {
              TopoShape aLocalShape = WCop.Reversed();
              TopoWire  bWire       = TopoDS::Wire(aLocalShape);
              //	      TopoWire bWire =   TopoDS::Wire(WCop.Reversed());
              FR.Add(bWire);
            }
            else
              FR.Add(WCop);
          }
        }
      }
    }
#ifdef DRAW
    if (AffichEdge)
    {
      TopTools_DataMapIteratorOfDataMapOfShapeShape it(MapVP);
      Standard_Integer                              k = 0;
      for (; it.More(); it.Next())
      {
        char name[100];
        sprintf(name, "PARALI_%d", ++k);
        DBRep1::Set(name, it.Value());
      }
    }
#endif

    //----------------------------------------------------
    // Construction of faces limited by parallels.
    // - set to the height of the support face.
    //----------------------------------------------------
    Transform3d T;
    T.SetTranslation(Vector3d(0, 0, Alt));
    TopLoc_Location LT(T);
    TopoShape    aLocalShape = mySpine.Moved(LT);
    FR.Init(TopoDS::Face(aLocalShape));
    //    FR.Init(TopoDS::Face(mySpine.Moved(LT)));
    FR.Perform();

    for (; FR.More(); FR.Next())
    {
      const TopoFace& F = FR.Current();
      B.Add(myShape, F);
      //---------------------------------------
      // Update myMap(.)(E)
      //---------------------------------------
      for (Exp.Init(F, TopAbs_EDGE); Exp.More(); Exp.Next())
      {
        const TopoEdge& CE = TopoDS::Edge(Exp.Current());
        if (OffAnc.HasAncestor(CE))
        {
          const TopoShape& InitE = OffAnc.Ancestor(CE);
          if (!myMap.IsBound(InitE))
            myMap.Bind(InitE, EmptyMap);
          if (!myMap(InitE).IsBound(E))
            myMap(InitE).Bind(E, EmptyList);
          myMap(InitE)(E).Append(F);
        }
      }
    }
  } // End loop on profile.
}

//=================================================================================================

void BRepFill_Evolved::VerticalPerform(const TopoFace&              Sp,
                                       const TopoWire&              Pr,
                                       const BRepMAT2d_BisectingLocus& Locus,
                                       BRepMAT2d_LinkTopoBilo&         Link1,
                                       const GeomAbs_JoinType          Join)
{
  TopoShape aLocalShape = Sp.Oriented(TopAbs_FORWARD);
  mySpine                  = TopoDS::Face(aLocalShape);
  //  mySpine   = TopoDS::Face(Sp.Oriented(TopAbs_FORWARD));
  myProfile = Pr;
  myMap.Clear();

  ShapeBuilder B;
  B.MakeCompound(TopoDS::Compound(myShape));

  BRepTools_WireExplorer   ProfExp;
  ShapeExplorer          Exp;
  BRepFill_OffsetWire      Paral;
  OffsetAncestors OffAnc;
  TopoVertex            V1, V2;

  Standard_Boolean                   First = Standard_True;
  TopoShape                       Base;
  TopTools_DataMapOfShapeListOfShape EmptyMap;

  for (ProfExp.Init(myProfile); ProfExp.More(); ProfExp.Next())
  {
    const TopoEdge& E = ProfExp.Current();
    EdgeVertices(E, V1, V2);
    Standard_Real Alt1 = Altitud(V1);
    Standard_Real Alt2 = Altitud(V2);

    if (First)
    {
      Standard_Real Offset = DistanceToOZ(V1);
      if (Abs(Offset) < BRepFill_Confusion())
      {
        Offset = 0.;
      }
      Paral.PerformWithBiLo(mySpine, Offset, Locus, Link1, Join, Alt1);
      OffAnc.Perform(Paral);
      Base = Paral.Shape();

      // MAJ myMap
      for (Exp.Init(Base, TopAbs_EDGE); Exp.More(); Exp.Next())
      {
        const TopoEdge&  anEdge = TopoDS::Edge(Exp.Current());
        const TopoShape& AE     = OffAnc.Ancestor(anEdge);
        if (!myMap.IsBound(AE))
        {
          myMap.Bind(AE, EmptyMap);
        }
        if (!myMap(AE).IsBound(V1))
        {
          ShapeList L;
          myMap(AE).Bind(V1, L);
        }
        myMap(AE)(V1).Append(anEdge);
      }
      First = Standard_False;
    }

#ifdef DRAW
    if (AffichEdge)
    {
      char name[100];
      sprintf(name, "PARALI_%d", ++NbVEVOS);
      DBRep1::Set(name, Base);
    }
#endif

    BRepSweep_Prism PS(Base, Vector3d(0, 0, Alt2 - Alt1), Standard_False);
#ifdef DRAW
    if (AffichEdge)
    {
      char name[100];
      sprintf(name, "PRISM_%d", NbVEVOS);
      DBRep1::Set(name, PS.Shape());
    }
#endif

    Base = PS.LastShape();

    for (Exp.Init(PS.Shape(), TopAbs_FACE); Exp.More(); Exp.Next())
    {
      B.Add(myShape, Exp.Current());
    }

    // MAJ myMap
    BRepFill_DataMapIteratorOfDataMapOfShapeDataMapOfShapeListOfShape it(myMap);

    for (; it.More(); it.Next())
    {
      const ShapeList&        LOF = it.Value()(V1);
      TopTools_ListIteratorOfListOfShape itLOF(LOF);
      if (!myMap(it.Key()).IsBound(V2))
      {
        ShapeList L;
        myMap(it.Key()).Bind(V2, L);
      }

      if (!myMap(it.Key()).IsBound(E))
      {
        ShapeList L;
        myMap(it.Key()).Bind(E, L);
      }

      for (; itLOF.More(); itLOF.Next())
      {
        const TopoShape& OS = itLOF.Value();
        myMap(it.Key())(V2).Append(PS.LastShape(OS));
        myMap(it.Key())(E).Append(PS.Shape(OS));
      }
    }
  }
}

//=======================================================================
// function : Bubble
// purpose  : Order the sequence of points by growing x.
//=======================================================================

static void Bubble(TColStd_SequenceOfReal& Seq)
{
  Standard_Boolean Invert   = Standard_True;
  Standard_Integer NbPoints = Seq.Length();

  while (Invert)
  {
    Invert = Standard_False;
    for (Standard_Integer i = 1; i < NbPoints; i++)
    {
      if (Seq.Value(i + 1) < Seq.Value(i))
      {
        Seq.Exchange(i, i + 1);
        Invert = Standard_True;
      }
    }
  }
}

//=======================================================================
// function : PrepareProfile
// purpose  : - Projection of the profile on the working plane.
//           - Cut of the profile at the extrema of distance from profile to axis Oz.
//           - Isolate vertical and horizontal parts.
//           - Reconstruction of wires starting from cut edges.
//           New wires stored in <WorkProf> are always at the same
//           side of axis OZ or mixed with it.
//=======================================================================

void BRepFill_Evolved::PrepareProfile(ShapeList&         WorkProf,
                                      TopTools_DataMapOfShapeShape& MapProf) const
{
  // Supposedly the profile is located so that the only transformation
  // to be carried out is a projection on plane yOz.

  // initialise the projection Plane and the Line to evaluate the extrema.
  Handle(GeomPlane)  Plane = new GeomPlane(Ax3(gp1::YOZ()));
  Handle(Geom2d_Line) Line  = new Geom2d_Line(gp1::OY2d());

  // Map initial vertex -> projected vertex.
  TopTools_DataMapOfShapeShape MapVerRefMoved;

  TopoVertex        V1, V2, VRef1, VRef2;
  TopoWire          W;
  ShapeBuilder         B;
  ShapeList WP;
  B.MakeWire(W);
  WP.Append(W);

  BRepTools_WireExplorer Exp(myProfile);

  while (Exp.More())
  {
    ShapeList Cuts;
    Standard_Boolean     NewWire = Standard_False;
    const TopoEdge&   E       = TopoDS::Edge(Exp.Current());

    // Cut of the edge.
    CutEdgeProf(E, Plane, Line, Cuts, MapVerRefMoved);

    EdgeVertices(E, VRef1, VRef2);

    if (Cuts.IsEmpty())
    {
      // Neither extrema nor intersections nor vertices on the axis.
      B.Add(W, E);
      MapProf.Bind(E, E);
    }
    else
    {
      while (!Cuts.IsEmpty())
      {
        const TopoEdge& NE = TopoDS::Edge(Cuts.First());
        MapProf.Bind(NE, E);
        EdgeVertices(NE, V1, V2);
        if (!MapProf.IsBound(V1))
          MapProf.Bind(V1, E);
        if (!MapProf.IsBound(V2))
          MapProf.Bind(V2, E);

        B.Add(W, NE);
        Cuts.RemoveFirst();

        if (DistanceToOZ(V2) < BRepFill_Confusion() && DistanceToOZ(V1) > BRepFill_Confusion())
        {
          // NE ends on axis OZ => new wire
          if (Cuts.IsEmpty())
          {
            // last part of the current edge
            // If it is not the last edge of myProfile
            // create a new wire.
            NewWire = Standard_True;
          }
          else
          {
            // New wire.
            B.MakeWire(W);
            WP.Append(W);
          }
        }
      }
    }
    Exp.Next();
    if (Exp.More() && NewWire)
    {
      B.MakeWire(W);
      WP.Append(W);
    }
  }

  // In the list of Wires, find edges generating plane or vertical vevo.
  TopTools_ListIteratorOfListOfShape ite;
  TopoWire                        CurW, NW;
  ShapeExplorer                    EW;

  for (ite.Initialize(WP); ite.More(); ite.Next())
  {
    CurW                     = TopoDS::Wire(ite.Value());
    Standard_Boolean YaModif = Standard_False;
    for (EW.Init(CurW, TopAbs_EDGE); EW.More(); EW.Next())
    {
      const TopoEdge& EE = TopoDS::Edge(EW.Current());
      if (IsVertical(EE) || IsPlanar(EE))
      {
        YaModif = Standard_True;
        break;
      }
    }

    if (YaModif)
    {
      // Status = 0 for the beginning
      //          3 vertical
      //          2 horizontal
      //          1 other
      Standard_Integer Status = 0;

      for (EW.Init(CurW, TopAbs_EDGE); EW.More(); EW.Next())
      {
        const TopoEdge& EE = TopoDS::Edge(EW.Current());
        if (IsVertical(EE))
        {
          if (Status != 3)
          {
            B.MakeWire(NW);
            WorkProf.Append(NW);
            Status = 3;
          }
        }
        else if (IsPlanar(EE))
        {
          if (Status != 2)
          {
            B.MakeWire(NW);
            WorkProf.Append(NW);
            Status = 2;
          }
        }
        else if (Status != 1)
        {
          B.MakeWire(NW);
          WorkProf.Append(NW);
          Status = 1;
        }
        B.Add(NW, EE);
      }
    }
    else
    {
      WorkProf.Append(CurW);
    }
  }

  // connect vertices modified in MapProf;
  TopTools_DataMapIteratorOfDataMapOfShapeShape gilbert(MapVerRefMoved);
  for (; gilbert.More(); gilbert.Next())
  {
    MapProf.Bind(gilbert.Value(), gilbert.Key());
  }
}

//=================================================================================================

void BRepFill_Evolved::PrepareSpine(TopoFace&                  WorkSpine,
                                    TopTools_DataMapOfShapeShape& MapSpine) const
{
  ShapeBuilder                       B;
  ShapeList               Cuts;
  TopTools_ListIteratorOfListOfShape IteCuts;
  TopoVertex                      V1, V2;

  TopLoc_Location             L;
  const Handle(GeomSurface)& S    = BRepInspector::Surface(mySpine, L);
  Standard_Real               TolF = BRepInspector::Tolerance(mySpine);
  B.MakeFace(WorkSpine, S, L, TolF);

  for (TopoDS_Iterator IteF(mySpine); IteF.More(); IteF.Next())
  {

    TopoWire NW;
    B.MakeWire(NW);
    Standard_Boolean IsClosed = IteF.Value().Closed();

    for (TopoDS_Iterator IteW(IteF.Value()); IteW.More(); IteW.Next())
    {

      TopoEdge E = TopoDS::Edge(IteW.Value());
      EdgeVertices(E, V1, V2);
      MapSpine.Bind(V1, V1);
      MapSpine.Bind(V2, V2);
      Cuts.Clear();

      // Cut
      CutEdge(E, mySpine, Cuts);

      if (Cuts.IsEmpty())
      {
        B.Add(NW, E);
        MapSpine.Bind(E, E);
      }
      else
      {
        for (IteCuts.Initialize(Cuts); IteCuts.More(); IteCuts.Next())
        {
          const TopoEdge& NE = TopoDS::Edge(IteCuts.Value());
          B.Add(NW, NE);
          MapSpine.Bind(NE, E);
          EdgeVertices(NE, V1, V2);
          if (!MapSpine.IsBound(V1))
            MapSpine.Bind(V1, E);
          if (!MapSpine.IsBound(V2))
            MapSpine.Bind(V2, E);
        }
      }
    }
    NW.Closed(IsClosed);
    B.Add(WorkSpine, NW);
  }

  // Construct curves 3D of the spine
  BRepLib::BuildCurves3d(WorkSpine);

#ifdef DRAW
  if (AffichEdge)
  {
    char name[100];
    sprintf(name, "workspine");
    DBRep1::Set(name, WorkSpine);
  }
#endif
}

//=================================================================================================

const TopoShape& BRepFill_Evolved::Top() const
{
  return myTop;
}

//=================================================================================================

const TopoShape& BRepFill_Evolved::Bottom() const
{
  return myBottom;
}

//=================================================================================================

const ShapeList& BRepFill_Evolved::GeneratedShapes(const TopoShape& SpineShape,
                                                              const TopoShape& ProfShape) const
{
  if (myMap.IsBound(SpineShape) && myMap(SpineShape).IsBound(ProfShape))
  {
    return myMap(SpineShape)(ProfShape);
  }
  else
  {
    static ShapeList Empty;
    return Empty;
  }
}

//=================================================================================================

BRepFill_DataMapOfShapeDataMapOfShapeListOfShape& BRepFill_Evolved::Generated()
{
  return myMap;
}

//=================================================================================================

static TopAbs_Orientation Compare(const TopoEdge& E1, const TopoEdge& E2)
{
  TopAbs_Orientation OO = TopAbs_FORWARD;
  TopoVertex      V1[2], V2[2];
  TopExp1::Vertices(E1, V1[0], V1[1]);
  TopExp1::Vertices(E2, V2[0], V2[1]);
  Point3d P1 = BRepInspector::Pnt(V1[0]);
  Point3d P2 = BRepInspector::Pnt(V2[0]);
  Point3d P3 = BRepInspector::Pnt(V2[1]);
  if (P1.Distance(P3) < P1.Distance(P2))
    OO = TopAbs_REVERSED;

  return OO;
}

//=================================================================================================

void BRepFill_Evolved::Add(BRepFill_Evolved& Vevo, const TopoWire& Prof, ShapeQuilt& Glue)

{
  BRepFill_DataMapOfShapeDataMapOfShapeListOfShape&                 MapVevo = Vevo.Generated();
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape               iteP;
  BRepFill_DataMapIteratorOfDataMapOfShapeDataMapOfShapeListOfShape iteS;
  TopoShape                                                      CurrentSpine, CurrentProf;

  if (Vevo.Shape().IsNull())
    return;

  //-------------------------------------------------
  // Find wires common to <me> and <Vevo>.
  //-------------------------------------------------

  ShapeExplorer ExProf;
  for (ExProf.Init(Prof, TopAbs_VERTEX); ExProf.More(); ExProf.Next())
  {
    const TopoShape& VV = ExProf.Current();
    //---------------------------------------------------------------
    // Parse edges generated by VV in myMap if they existent
    // and Bind in Glue
    //---------------------------------------------------------------

    //------------------------------------------------- -------------
    // Note: the curves of of reinforced edges are in the same direction
    //          if one remains on the same edge.
    //          if one passes from left to the right they are inverted.
    //------------------------------------------------- -------------
    Standard_Boolean Commun = Standard_False;
    Relative(myProfile, Prof, TopoDS::Vertex(VV), Commun);

    if (Commun)
    {
      for (iteS.Initialize(myMap); iteS.More(); iteS.Next())
      {
        const TopoShape& SP = iteS.Key();
        if (iteS.Value().IsBound(VV) && MapVevo.IsBound(SP) && MapVevo(SP).IsBound(VV))
        {

          const ShapeList&        MyList   = myMap(SP)(VV);
          const ShapeList&        VevoList = Vevo.GeneratedShapes(SP, VV);
          TopTools_ListIteratorOfListOfShape MyIte(MyList);
          TopTools_ListIteratorOfListOfShape VevoIte(VevoList);
          for (; MyIte.More(); MyIte.Next(), VevoIte.Next())
          {
            const TopoEdge& ME           = TopoDS::Edge(MyIte.Value());
            const TopoEdge& VE           = TopoDS::Edge(VevoIte.Value());
            TopAbs_Orientation OG           = Compare(ME, VE);
            TopoShape       aLocalShape  = VE.Oriented(TopAbs_FORWARD);
            TopoShape       aLocalShape2 = ME.Oriented(OG);
            Glue.Bind(TopoDS::Edge(aLocalShape), TopoDS::Edge(aLocalShape2));
            //	    Glue.Bind(TopoDS::Edge(VE.Oriented (TopAbs_FORWARD)),
            //		      TopoDS::Edge(ME.Oriented (OG)));
          }
        }
      }
    }
  }
  Glue.Add(Vevo.Shape());

  //----------------------------------------------------------
  // Add map of elements generate in Vevo in myMap.
  //----------------------------------------------------------
  TopTools_DataMapOfShapeListOfShape EmptyMap;
  ShapeList               EmptyList;

  for (iteS.Initialize(MapVevo); iteS.More(); iteS.Next())
  {
    CurrentSpine = iteS.Key();
    for (iteP.Initialize(MapVevo(CurrentSpine)); iteP.More(); iteP.Next())
    {
      CurrentProf = iteP.Key();
      if (!myMap.IsBound(CurrentSpine))
      {
        //------------------------------------------------
        // The element of spine is not yet present .
        // => previous profile not on the border.
        //-------------------------------------------------
        myMap.Bind(CurrentSpine, EmptyMap);
      }
      if (!myMap(CurrentSpine).IsBound(CurrentProf))
      {
        myMap(CurrentSpine).Bind(CurrentProf, EmptyList);
        const ShapeList&        GenShapes = MapVevo(CurrentSpine)(CurrentProf);
        TopTools_ListIteratorOfListOfShape itl(GenShapes);
        for (; itl.More(); itl.Next())
        {
          // during Glue.Add the shared shapes are recreated.
          if (Glue.IsCopied(itl.Value()))
            myMap(CurrentSpine)(CurrentProf).Append(Glue.Copy(itl.Value()));
          else
            myMap(CurrentSpine)(CurrentProf).Append(itl.Value());
        }
      }
    }
  }
}

//=================================================================================================

TopoShape& BRepFill_Evolved::ChangeShape()
{
  return myShape;
}

//=================================================================================================

void BRepFill_Evolved::Transfert(BRepFill_Evolved&                   Vevo,
                                 const TopTools_DataMapOfShapeShape& MapProf,
                                 const TopTools_DataMapOfShapeShape& MapSpine,
                                 const TopLoc_Location&              LS,
                                 const TopLoc_Location&              InitLS,
                                 const TopLoc_Location&              InitLP)
{
  //----------------------------------------------------------------
  // Transfer the shape from Vevo in myShape and Reposition shapes.
  //----------------------------------------------------------------
  myShape = Vevo.Shape();
  mySpine.Location(InitLS);
  myProfile.Location(InitLP);
  myShape.Move(LS);

  //
  // Expecting for better, the Same Parameter is forced here
  //  ( Pb Sameparameter between YaPlanar and Tuyaux
  //
  ShapeBuilder    B;
  ShapeExplorer ex(myShape, TopAbs_EDGE);
  while (ex.More())
  {
    B.SameRange(TopoDS::Edge(ex.Current()), Standard_False);
    B.SameParameter(TopoDS::Edge(ex.Current()), Standard_False);
    BRepLib::SameParameter(TopoDS::Edge(ex.Current()));
    ex.Next();
  }

  //--------------------------------------------------------------
  // Transfer of myMap of Vevo into myMap.
  //--------------------------------------------------------------
  BRepFill_DataMapIteratorOfDataMapOfShapeDataMapOfShapeListOfShape iteS;
  TopTools_DataMapIteratorOfDataMapOfShapeListOfShape               iteP;
  TopTools_DataMapOfShapeListOfShape                                EmptyMap;
  ShapeList                                              EmptyList;
  TopoShape                                                      InitialSpine, InitialProf;

  BRepFill_DataMapOfShapeDataMapOfShapeListOfShape& MapVevo = Vevo.Generated();

  for (iteS.Initialize(MapVevo); iteS.More(); iteS.Next())
  {
    InitialSpine = MapSpine(iteS.Key());
    InitialSpine.Move(LS);

    for (iteP.Initialize(MapVevo(iteS.Key())); iteP.More(); iteP.Next())
    {
      InitialProf = MapProf(iteP.Key());
      InitialProf.Location(InitLP);

      ShapeList& GenShapes = MapVevo.ChangeFind(iteS.Key()).ChangeFind(iteP.Key());

      TopTools_ListIteratorOfListOfShape itl;
      for (itl.Initialize(GenShapes); itl.More(); itl.Next())
      {
        itl.ChangeValue().Move(LS);
      }

      if (!myMap.IsBound(InitialSpine))
      {
        myMap.Bind(InitialSpine, EmptyMap);
      }

      if (!myMap(InitialSpine).IsBound(InitialProf))
      {
        myMap(InitialSpine).Bind(InitialProf, EmptyList);
      }
      myMap(InitialSpine)(InitialProf).Append(GenShapes);
    }
  }
  //--------------------------------------------------------------
  // Transfer of Top and Bottom of Vevo in myTop and myBottom.
  //--------------------------------------------------------------
  myTop = Vevo.Top();
  myTop.Move(LS);
  myBottom = Vevo.Bottom();
  myBottom.Move(LS);
}

//=================================================================================================

Standard_Boolean BRepFill_Evolved::IsDone() const
{
  return myIsDone;
}

//=================================================================================================

const TopoShape& BRepFill_Evolved::Shape() const
{
  return myShape;
}

//=================================================================================================

GeomAbs_JoinType BRepFill_Evolved::JoinType() const
{
  return myJoinType;
}

//=================================================================================================

void BRepFill_Evolved::AddTopAndBottom(ShapeQuilt& Glue)
{
  //  return first and last vertex of the profile.
  TopoVertex V[2];
  TopExp1::Vertices(myProfile, V[0], V[1]);
  if (V[0].IsSame(V[1]))
    return;

  TopTools_ListIteratorOfListOfShape itL;
  Standard_Boolean                   ToReverse = Standard_False;
  for (Standard_Integer i = 0; i <= 1; i++)
  {

    BRepAlgo_Loop Loop;
    // Construction of supports.
    gp_Pln      S(0., 0., 1., -Altitud(V[i]));
    TopoFace F = BRepLib_MakeFace(S);
    Loop.Init(F);

    ShapeExplorer     ExpSpine(mySpine, TopAbs_EDGE);
    TopTools_MapOfShape View;

    for (; ExpSpine.More(); ExpSpine.Next())
    {
      const TopoEdge&          ES                 = TopoDS::Edge(ExpSpine.Current());
      const ShapeList& L                  = GeneratedShapes(ES, V[i]);
      Standard_Boolean            ComputeOrientation = 0;

      for (itL.Initialize(L); itL.More(); itL.Next())
      {
        const TopoEdge& E = TopoDS::Edge(itL.Value());

        if (!ComputeOrientation)
        {
          BRepAdaptor_Curve C1(ES);
          BRepAdaptor_Curve C2(E);
          Standard_Real     f, l, fs, ls;
          BRepInspector::Range(E, f, l);
          BRepInspector::Range(ES, fs, ls);
          Standard_Real u  = 0.3 * f + 0.7 * l;
          Standard_Real us = 0.3 * fs + 0.7 * ls;
          Point3d        P;
          Vector3d        V1, V2;
          C1.D1(us, P, V1);
          C2.D1(u, P, V2);
          ToReverse          = (V1.Dot(V2) < 0.);
          ComputeOrientation = 1;
        }

        TopAbs_Orientation Or = ES.Orientation();
        if (ToReverse)
          Or = TopAbs1::Reverse(Or);
        TopoShape aLocalShape = E.Oriented(Or);
        Loop.AddConstEdge(TopoDS::Edge(aLocalShape));
        //	Loop.AddConstEdge(TopoDS::Edge(E.Oriented(Or)));
      }
    }

    Point3d           PV    = BRepInspector::Pnt(V[i]);
    Standard_Boolean IsOut = PV.Y() < 0;

    for (ExpSpine.Init(mySpine, TopAbs_VERTEX); ExpSpine.More(); ExpSpine.Next())
    {
      const TopoVertex& ES = TopoDS::Vertex(ExpSpine.Current());
      if (View.Add(ES))
      {
        const ShapeList& L = GeneratedShapes(ES, V[i]);
        for (itL.Initialize(L); itL.More(); itL.Next())
        {
          const TopoEdge& E = TopoDS::Edge(itL.Value());
          if (!BRepInspector::Degenerated(E))
          {
            // the center of circle (ie vertex) is IN the cap if vertex IsOut
            //                                    OUT                   !IsOut
            BRepAdaptor_Curve C(E);
            Standard_Real     f, l;
            BRepInspector::Range(E, f, l);
            Standard_Real u = 0.3 * f + 0.7 * l;
            Point3d        P = BRepInspector::Pnt(ES);
            Point3d        PC;
            Vector3d        VC;
            C.D1(u, PC, VC);
            Vector3d aPPC(P, PC);
            Vector3d Prod = aPPC.Crossed(VC);
            if (IsOut)
            {
              ToReverse = Prod.Z() < 0.;
            }
            else
            {
              ToReverse = Prod.Z() > 0.;
            }
            TopAbs_Orientation Or = TopAbs_FORWARD;
            if (ToReverse)
              Or = TopAbs_REVERSED;
            TopoShape aLocalShape = E.Oriented(Or);
            Loop.AddConstEdge(TopoDS::Edge(aLocalShape));
            //	    Loop.AddConstEdge(TopoDS::Edge(E.Oriented(Or)));
          }
        }
      }
    }

    Loop.Perform();
    Loop.WiresToFaces();
    const ShapeList&        L = Loop.NewFaces();
    TopTools_ListIteratorOfListOfShape anIterL(L);

    // Maj of myTop and myBottom for the history
    // and addition of constructed faces.
    TopoCompound Bouchon;
    ShapeBuilder    B;
    B.MakeCompound(Bouchon);
    Standard_Integer j = 0;

    for (anIterL.Initialize(L); anIterL.More(); anIterL.Next())
    {
      j++;
      Glue.Add(anIterL.Value());
      if (j == 1 && i == 0)
        myTop = anIterL.Value();
      if (j == 1 && i == 1)
        myBottom = anIterL.Value();
      B.Add(Bouchon, anIterL.Value());
    }
    if (i == 0 && j > 1)
      myTop = Bouchon;
    if (i == 1 && j > 1)
      myBottom = Bouchon;
  }
}

//================================================================== =====
// function : MakePipe
// purpose  :
//=======================================================================

void BRepFill_Evolved::MakeSolid()
{

  ShapeExplorer  exp(myShape, TopAbs_SHELL);
  Standard_Integer ish = 0;
  TopoCompound  Res;
  TopoSolid     Sol;
  ShapeBuilder     B;
  B.MakeCompound(Res);

  for (; exp.More(); exp.Next())
  {
    const TopoShape& Sh = exp.Current();
    B.MakeSolid(Sol);
    B.Add(Sol, Sh);
    BRepClass3d_SolidClassifier SC(Sol);
    SC.PerformInfinitePoint(BRepFill_Confusion());
    if (SC.State() == TopAbs_IN)
    {
      B.MakeSolid(Sol);
      B.Add(Sol, Sh.Reversed());
    }
    B.Add(Res, Sol);
    ish++;
  }
  if (ish == 1)
  {
    myShape = Sol;
  }
  else
  {
    myShape = Res;
  }
}

//=================================================================================================

void BRepFill_Evolved::MakePipe(const TopoEdge& SE, const Ax3& AxeRef)
{
  BRepTools_WireExplorer ProfExp;
  ShapeExplorer        FaceExp;

  Transform3d trsf;
  if (Side(myProfile, BRepFill_Confusion()) > 3)
  { // side right
    trsf.SetRotation(gp1::OZ(), M_PI);
  }
  TopLoc_Location DumLoc(trsf);
  TopoShape    aLocalShape = myProfile.Moved(DumLoc);
  TopoWire     DummyProf =
    PutProfilAt(TopoDS::Wire(aLocalShape), AxeRef, SE, mySpine, Standard_True);
  //  TopoWire DummyProf =
  //    PutProfilAt (TopoDS::Wire(myProfile.Moved(DumLoc)),
  //		 AxeRef,SE,
  //		 mySpine,Standard_True);

  // Copy of the profile to avoid the accumulation of
  // locations on the Edges of myProfile!

  Handle(BRepTools_TrsfModification) TrsfMod = new BRepTools_TrsfModification(Transform3d());
  ShapeModifier                 Modif(DummyProf, TrsfMod);

  TopoWire GenProf = TopoDS::Wire(Modif.ModifiedShape(DummyProf));

#ifdef DRAW
  if (AffichGeom)
  {
    char name[100];
    sprintf(name, "EVOLBASE_%d", ++NbFACES);
    DBRep1::Set(name, SE);
    sprintf(name, "EVOLPROF_%d", NbFACES);
    DBRep1::Set(name, GenProf);
  }
#endif

  BRepFill_Pipe Pipe(BRepLib_MakeWire(SE), GenProf);
  // BRepFill_Pipe Pipe = BRepFill_Pipe(BRepLib_MakeWire(SE),GenProf);

#ifdef DRAW
  if (AffichGeom)
  {
    char name[100];
    sprintf(name, "EVOL_%d", ++NbFACES);
    DBRep1::Set(name, Pipe.Shape());
  }
#endif
  //---------------------------------------------
  // Arrangement of Tubes in myMap.
  //---------------------------------------------

  BRepTools_WireExplorer             GenProfExp;
  ShapeList               L;
  TopoVertex                      VF, VL, VFG, VLG;
  Standard_Boolean                   FirstVertex = Standard_True;
  TopTools_DataMapOfShapeListOfShape P;

  myMap.Bind(SE, P);

  for (ProfExp.Init(myProfile), GenProfExp.Init(GenProf); ProfExp.More();
       ProfExp.Next(), GenProfExp.Next())
  {

    EdgeVertices(ProfExp.Current(), VF, VL);
    EdgeVertices(GenProfExp.Current(), VFG, VLG);

    if (FirstVertex)
    {
      myMap(SE).Bind(VF, L);
      myMap(SE)(VF).Append(Pipe.Edge(SE, VFG));
      FirstVertex = Standard_False;
    }
    myMap(SE).Bind(VL, L);
    myMap(SE)(VL).Append(Pipe.Edge(SE, VLG));
    myMap(SE).Bind(ProfExp.Current(), L);
    myMap(SE)(ProfExp.Current()).Append(Pipe.Face(SE, GenProfExp.Current()));
  }
}

//=================================================================================================

void BRepFill_Evolved::MakeRevol(const TopoEdge&   SE,
                                 const TopoVertex& VLast,
                                 const Ax3&        AxeRef)
{
  BRepTools_WireExplorer ProfExp;
  ShapeExplorer        FaceExp;

  Transform3d trsf;
  if (Side(myProfile, BRepFill_Confusion()) > 3)
  { // side right
    trsf.SetRotation(gp1::OZ(), M_PI);
  }
  TopLoc_Location DumLoc(trsf);
  TopoShape    aLocalShape = myProfile.Moved(DumLoc);
  TopoWire GenProf = PutProfilAt(TopoDS::Wire(aLocalShape), AxeRef, SE, mySpine, Standard_False);
  //  TopoWire GenProf =
  //    PutProfilAt (TopoDS::Wire(myProfile.Moved(DumLoc)),
  //		 AxeRef,SE,
  //		 mySpine,Standard_False);

  Axis3d AxeRev(BRepInspector::Pnt(VLast), -gp1::DZ());

  // Position of the sewing on the edge of the spine
  // so that the bissectrices didn't cross the sewings.
  Transform3d dummy;
  dummy.SetRotation(AxeRev, 1.5 * M_PI);
  TopLoc_Location DummyLoc(dummy);
  GenProf.Move(DummyLoc);

  BRepSweep_Revol Rev(GenProf, AxeRev, Standard_True);

#ifdef DRAW
  if (AffichGeom)
  {
    char name[100];
    sprintf(name, "EVOLBASE_%d", ++NbFACES);
    DrawTrSurf1::Set(name, new GeomLine(AxeRev));
    //    DrawTrSurf1::Set(name,new GeomLine(AxeRev));
    sprintf(name, "EVOLPROF_%d", NbFACES);
    DBRep1::Set(name, GenProf);

    sprintf(name, "EVOL_%d", NbFACES);
    DBRep1::Set(name, Rev.Shape());
  }
#endif
  //--------------------------------------------
  // Arrangement of revolutions in myMap.
  //---------------------------------------------
  BRepTools_WireExplorer             GenProfExp;
  ShapeList               L;
  TopoVertex                      VF, VL, VFG, VLG;
  Standard_Boolean                   FirstVertex = Standard_True;
  TopTools_DataMapOfShapeListOfShape R;

  myMap.Bind(VLast, R);

  for (ProfExp.Init(myProfile), GenProfExp.Init(GenProf); ProfExp.More();
       ProfExp.Next(), GenProfExp.Next())
  {

    EdgeVertices(ProfExp.Current(), VF, VL);
    EdgeVertices(GenProfExp.Current(), VFG, VLG);

    TopAbs_Orientation Or = GenProfExp.Current().Orientation();

    if (FirstVertex)
    {
      myMap(VLast).Bind(VF, L);
      const TopoShape& RV = Rev.Shape(VFG);
      //      TopAbs_Orientation OO = TopAbs1::Compose(RV.Orientation(),Or);
      TopAbs_Orientation OO = RV.Orientation();
      myMap(VLast)(VF).Append(RV.Oriented(OO));
      FirstVertex = Standard_False;
    }
    myMap(VLast).Bind(ProfExp.Current(), L);
    const TopoShape& RF = Rev.Shape(GenProfExp.Current());
    TopAbs_Orientation  OO = TopAbs1::Compose(RF.Orientation(), Or);

    myMap(VLast)(ProfExp.Current()).Append(RF.Oriented(OO));
    myMap(VLast).Bind(VL, L);
    const TopoShape& RV = Rev.Shape(VLG);
    //    OO = TopAbs1::Compose(RV.Orientation(),Or);
    OO = RV.Orientation();
    myMap(VLast)(VL).Append(RV.Oriented(OO));
  }
}

//=================================================================================================

TopLoc_Location BRepFill_Evolved::FindLocation(const TopoFace& Face) const
{
  TopLoc_Location      L;
  Handle(GeomSurface) S;
  S = BRepInspector::Surface(Face, L);

  if (!S->IsKind(STANDARD_TYPE(GeomPlane)))
  {
    BRepLib_FindSurface FS(Face, -1, Standard_True);
    if (FS.Found())
    {
      S = FS.Surface();
      L = FS.Location();
    }
    else
      throw Standard_NoSuchObject("BRepFill_Evolved : The Face is not planar");
  }

  if (!L.IsIdentity())
    S = Handle(GeomSurface)::DownCast(S->Transformed(L.Transformation()));

  Handle(GeomPlane) P    = Handle(GeomPlane)::DownCast(S);
  Ax3             Axis = P->Position();

  Transform3d T;
  Ax3  AxeRef(Point3d(0., 0., 0.), Dir3d(0., 0., 1.), Dir3d(1., 0., 0.));
  T.SetTransformation(AxeRef, Axis);

  return TopLoc_Location(T);
}

//=================================================================================================

void BRepFill_Evolved::TransformInitWork(const TopLoc_Location& LS, const TopLoc_Location& LP)
{
  mySpine.Move(LS);
  myProfile.Move(LP);

#ifdef DRAW
  if (AffichEdge)
  {
    char name[100];
    sprintf(name, "movedspine");
    TopoFace SL = mySpine;
    DBRep1::Set(name, SL);
    sprintf(name, "movedprofile");
    TopoWire PL = myProfile;
    DBRep1::Set(name, PL);
  }
#endif
}

//=======================================================================
// function : ContinuityOnOffsetEdge
// purpose  : Coding of regularities on edges parallel to CutVevo
//           common to left and right parts of volevo.
//=======================================================================
void BRepFill_Evolved::ContinuityOnOffsetEdge(const ShapeList&)
{
  BRepTools_WireExplorer                                            WExp;
  BRepFill_DataMapIteratorOfDataMapOfShapeDataMapOfShapeListOfShape iteS;
  TopoVertex                                                     VF, VL, V;
  TopoEdge                                                       PrecE, CurE, FirstE;
  ShapeBuilder                                                      B;

  WExp.Init(myProfile);
  FirstE = WExp.Current();
  PrecE  = FirstE;
  EdgeVertices(FirstE, VF, V);
  if (WExp.More())
    WExp.Next();

  for (; WExp.More(); WExp.Next())
  {
    CurE = WExp.Current();
    V    = WExp.CurrentVertex();

    if (DistanceToOZ(V) <= BRepFill_Confusion())
    {
      // the regularities are already coded on the edges of elementary volevos
      Standard_Real     U1 = BRepInspector::Parameter(V, CurE);
      Standard_Real     U2 = BRepInspector::Parameter(V, PrecE);
      BRepAdaptor_Curve Curve1(CurE);
      BRepAdaptor_Curve Curve2(PrecE);
      GeomAbs_Shape     Continuity = BRepLProp1::Continuity(Curve1, Curve2, U1, U2);

      if (Continuity >= 1)
      {
        //-----------------------------------------------------
        // Code continuity for all edges generated by V.
        //-----------------------------------------------------
        for (iteS.Initialize(myMap); iteS.More(); iteS.Next())
        {
          const TopoShape& SP = iteS.Key();
          if (myMap(SP).IsBound(V) && myMap(SP).IsBound(CurE) && myMap(SP).IsBound(PrecE))
          {
            if (!myMap(SP)(V).IsEmpty() && !myMap(SP)(CurE).IsEmpty()
                && !myMap(SP)(PrecE).IsEmpty())
              B.Continuity(TopoDS::Edge(myMap(SP)(V).First()),
                           TopoDS::Face(myMap(SP)(CurE).First()),
                           TopoDS::Face(myMap(SP)(PrecE).First()),
                           Continuity);
          }
        }
      }
    }
    PrecE = CurE;
  }

  EdgeVertices(PrecE, V, VL);

  if (VF.IsSame(VL))
  {
    // Closed profile.
    Standard_Real     U1 = BRepInspector::Parameter(VF, CurE);
    Standard_Real     U2 = BRepInspector::Parameter(VF, FirstE);
    BRepAdaptor_Curve Curve1(CurE);
    BRepAdaptor_Curve Curve2(FirstE);
    GeomAbs_Shape     Continuity = BRepLProp1::Continuity(Curve1, Curve2, U1, U2);

    if (Continuity >= 1)
    {
      //---------------------------------------------
      // Code continuity for all edges generated by V.
      //---------------------------------------------
      for (iteS.Initialize(myMap); iteS.More(); iteS.Next())
      {
        const TopoShape& SP = iteS.Key();
        if (myMap(SP).IsBound(VF) && myMap(SP).IsBound(CurE) && myMap(SP).IsBound(FirstE))
        {
          if (!myMap(SP)(VF).IsEmpty() && !myMap(SP)(CurE).IsEmpty()
              && !myMap(SP)(FirstE).IsEmpty())
            B.Continuity(TopoDS::Edge(myMap(SP)(VF).First()),
                         TopoDS::Face(myMap(SP)(CurE).First()),
                         TopoDS::Face(myMap(SP)(FirstE).First()),
                         Continuity);
        }
      }
    }
  }
}

//=======================================================================
// function : AddDegeneratedEdge
// purpose  : degenerated edges can be missing in some face
//           the missing degenerated edges have vertices corresponding
//           to node of the map.
//           Now it is enough to compare points UV of vertices
//           on edges with a certain tolerance.
//=======================================================================

static void AddDegeneratedEdge(TopoFace& F, TopoWire& W)
{
  TopLoc_Location      L;
  Handle(GeomSurface) S = BRepInspector::Surface(F, L);
  if (S->DynamicType() == STANDARD_TYPE(Geom_RectangularTrimmedSurface))
  {
    Handle(GeomSurface) SB = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
    if (SB->DynamicType() == STANDARD_TYPE(GeomPlane))
    {
      return;
    }
  }

  if (S->DynamicType() == STANDARD_TYPE(GeomPlane))
  {
    return;
  }

  ShapeBuilder  B;
  Standard_Real TolConf = 1.e-4;

  Standard_Boolean Change = Standard_True;

  while (Change)
  {
    Change = Standard_False;
    BRepTools_WireExplorer WE(W, F);
    gp_Pnt2d               PF, PrevP, P1, P2;
    TopoVertex          VF, V1, V2;

    for (; WE.More(); WE.Next())
    {
      const TopoEdge& CE = WE.Current();
      EdgeVertices(CE, V1, V2);
      if (CE.Orientation() == TopAbs_REVERSED)
        BRepInspector::UVPoints(CE, F, P2, P1);
      else
        BRepInspector::UVPoints(CE, F, P1, P2);
      if (VF.IsNull())
      {
        VF = V1;
        PF = P1;
      }
      else
      {
        if (!P1.IsEqual(PrevP, TolConf))
        {
          // degenerated edge to be inserted.
          Change = Standard_True;
          gp_Vec2d                    V(PrevP, P1);
          Handle(Geom2d_Line)         C2d = new Geom2d_Line(PrevP, gp_Dir2d(V));
          Standard_Real               f = 0, l = PrevP.Distance(P1);
          Handle(Geom2d_TrimmedCurve) CT = new Geom2d_TrimmedCurve(C2d, f, l);
          TopoEdge                 NE = BRepLib_MakeEdge(C2d, S);
          B.Degenerated(NE, Standard_True);
          B.Add(NE, V1.Oriented(TopAbs_FORWARD));
          B.Add(NE, V1.Oriented(TopAbs_REVERSED));
          B.Range(NE, f, l);
          B.Add(W, NE);
          break;
        }
      }
      PrevP = P2;
    }
    if (!Change && VF.IsSame(V2))
    { // closed
      if (!PF.IsEqual(P2, TolConf))
      {
        // Degenerated edge to be inserted.
        Change = Standard_True;
        gp_Vec2d                    V(P2, PF);
        Handle(Geom2d_Line)         C2d = new Geom2d_Line(P2, gp_Dir2d(V));
        Standard_Real               f = 0, l = P2.Distance(PF);
        Handle(Geom2d_TrimmedCurve) CT = new Geom2d_TrimmedCurve(C2d, f, l);
        TopoEdge                 NE = BRepLib_MakeEdge(C2d, S);
        B.Degenerated(NE, Standard_True);
        B.Add(NE, VF.Oriented(TopAbs_FORWARD));
        B.Add(NE, VF.Oriented(TopAbs_REVERSED));
        B.Range(NE, f, l);
        B.Add(W, NE);
      }
    }
  }
}

//=================================================================================================

void TrimFace(const TopoFace&        Face,
              TopTools_SequenceOfShape& TheEdges,
              TopTools_SequenceOfShape& S)
{

#ifdef DRAW
  Standard_Integer NB = TheEdges.Length();
  if (AffichEdge)
  {
    char name[100];
    std::cout << " TrimFace " << ++NbTRIMFACES;
    std::cout << " : " << NB << " edges within the restriction" << std::endl;
    for (Standard_Integer j = 1; j <= NB; j++)
    {
      sprintf(name, "TRIMEDGE_%d_%d", NbTRIMFACES, j);
      DBRep1::Set(name, TopoDS::Edge(TheEdges.Value(j)));
    }
  }
#endif

  //--------------------------------------
  // Creation of wires limiting faces.
  //--------------------------------------
  ShapeBuilder TheBuilder;

  Standard_Integer NbEdges;
  Standard_Boolean NewWire = Standard_True;
  Standard_Boolean AddEdge = Standard_False;
  TopoWire      GoodWire;

  while (!TheEdges.IsEmpty())
  {

    BRepLib_MakeWire MWire(TopoDS::Edge(TheEdges.First()));
    GoodWire = MWire.Wire();
    TheEdges.Remove(1);
    NbEdges = TheEdges.Length();
    NewWire = Standard_False;

    while (!NewWire)
    {
      AddEdge = Standard_False;

      for (Standard_Integer i = 1; i <= NbEdges && !AddEdge; i++)
      {
        const TopoEdge& E = TopoDS::Edge(TheEdges.Value(i));
        if (BRepInspector::Degenerated(E))
        {
          TheEdges.Remove(i);
          AddEdge  = Standard_True;
          NbEdges  = TheEdges.Length();
          GoodWire = MWire.Wire();
        }
        else
        {
          MWire.Add(E);
          if (MWire.Error() == BRepLib_WireDone)
          {
            // the connection is successful
            // it is removed from the sequence and one restarts from the beginning.
            TheEdges.Remove(i);
            AddEdge  = Standard_True;
            NbEdges  = TheEdges.Length();
            GoodWire = MWire.Wire();
          }
        }
      }
      NewWire = (!AddEdge);
    }
    TopoShape aLocalShape = Face.EmptyCopied();
    TopoFace  FaceCut     = TopoDS::Face(aLocalShape);
    //    TopoFace FaceCut = TopoDS::Face(Face.EmptyCopied());
    FaceCut.Orientation(TopAbs_FORWARD);
    BRepTools1::Update(FaceCut);
    AddDegeneratedEdge(FaceCut, GoodWire);
    TheBuilder.Add(FaceCut, GoodWire);
    FaceCut.Orientation(Face.Orientation());
    S.Append(FaceCut);
  }
}

//=================================================================================================

const TopoWire PutProfilAt(const TopoWire&     ProfRef,
                              const Ax3&          AxeRef,
                              const TopoEdge&     E,
                              const TopoFace&     F,
                              const Standard_Boolean AtStart)
{
  gp_Vec2d             D1;
  gp_Pnt2d             P;
  TopoWire          Prof;
  Handle(GeomCurve2d) C2d;
  Standard_Real        First, Last;

  C2d = BRepInspector::CurveOnSurface(E, F, First, Last);
  if (C2d.IsNull())
  {
    throw Standard_ConstructionError("ConstructionError in PutProfilAt");
  }

  if (E.Orientation() == TopAbs_REVERSED)
  {
    if (!AtStart)
      C2d->D1(First, P, D1);
    else
      C2d->D1(Last, P, D1);
    D1.Reverse();
  }
  else
  {
    if (!AtStart)
      C2d->D1(Last, P, D1);
    else
      C2d->D1(First, P, D1);
  }
  Point3d P3d(P.X(), P.Y(), 0.);
  Vector3d V3d(D1.X(), D1.Y(), 0.);

  Ax3  Ax(P3d, gp1::DZ(), V3d);
  Transform3d Trans;
  Trans.SetTransformation(Ax, AxeRef);
  TopoShape aLocalShape = ProfRef.Moved(TopLoc_Location(Trans));
  Prof                     = TopoDS::Wire(aLocalShape);
  //  Prof = TopoDS::Wire(ProfRef.Moved(TopLoc_Location(Trans)));
  return Prof;
}

//=================================================================================================

void TrimEdge(const TopoEdge&              Edge,
              const TopTools_SequenceOfShape& TheEdgesControle,
              TopTools_SequenceOfShape&       TheVer,
              TColStd_SequenceOfReal&         ThePar,
              TopTools_SequenceOfShape&       S)
{
  Standard_Boolean Change = Standard_True;
  ShapeBuilder     TheBuilder;
  S.Clear();
  //------------------------------------------------------------
  // Parse two sequences depending on the parameter on the edge.
  //------------------------------------------------------------
  while (Change)
  {
    Change = Standard_False;
    for (Standard_Integer i = 1; i < ThePar.Length(); i++)
    {
      if (ThePar.Value(i) > ThePar.Value(i + 1))
      {
        ThePar.Exchange(i, i + 1);
        TheVer.Exchange(i, i + 1);
        Change = Standard_True;
      }
    }
  }

  //----------------------------------------------------------
  // If a vertex is not in the proofing point, it is removed.
  //----------------------------------------------------------
  if (!BRepInspector::Degenerated(Edge))
  {
    for (Standard_Integer k = 1; k <= TheVer.Length(); k++)
    {
      if (DoubleOrNotInFace(TheEdgesControle, TopoDS::Vertex(TheVer.Value(k))))
      {
        TheVer.Remove(k);
        ThePar.Remove(k);
        k--;
      }
    }
  }

  //-------------------------------------------------------------------
  // Processing of double vertices for non-degenerated edges.
  // If a vertex_double appears twice in the edges of control,
  // the vertex is eliminated .
  // otherwise its only representation is preserved.
  //-------------------------------------------------------------------
  if (!BRepInspector::Degenerated(Edge))
  {
    for (Standard_Integer k = 1; k < TheVer.Length(); k++)
    {
      if (TheVer.Value(k).IsSame(TheVer.Value(k + 1)))
      {
        TheVer.Remove(k + 1);
        ThePar.Remove(k + 1);
        if (DoubleOrNotInFace(TheEdgesControle, TopoDS::Vertex(TheVer.Value(k))))
        {
          TheVer.Remove(k);
          ThePar.Remove(k);
          //	  k--;
        }
        k--;
      }
    }
  }

  //-----------------------------------------------------------
  // Creation of edges.
  // the number of vertices should be even. The edges to be created leave
  // from a vertex with uneven index i to vertex i+1;
  //-----------------------------------------------------------
  for (Standard_Integer k = 1; k < TheVer.Length(); k = k + 2)
  {
    TopoShape aLocalShape = Edge.EmptyCopied();
    TopoEdge  NewEdge     = TopoDS::Edge(aLocalShape);
    //    TopoEdge NewEdge = TopoDS::Edge(Edge.EmptyCopied());

    if (NewEdge.Orientation() == TopAbs_REVERSED)
    {
      TheBuilder.Add(NewEdge, TheVer.Value(k).Oriented(TopAbs_REVERSED));
      TheBuilder.Add(NewEdge, TheVer.Value(k + 1).Oriented(TopAbs_FORWARD));
    }
    else
    {
      TheBuilder.Add(NewEdge, TheVer.Value(k).Oriented(TopAbs_FORWARD));
      TheBuilder.Add(NewEdge, TheVer.Value(k + 1).Oriented(TopAbs_REVERSED));
    }
    TheBuilder.Range(NewEdge, ThePar.Value(k), ThePar.Value(k + 1));
    //  modified by NIZHNY-EAP Wed Dec 22 12:09:48 1999 ___BEGIN___
    BRepLib::UpdateTolerances(NewEdge, Standard_False);
    //  modified by NIZHNY-EAP Wed Dec 22 13:34:19 1999 ___END___
    S.Append(NewEdge);
  }
}

//=================================================================================================

void ComputeIntervals(const TopTools_SequenceOfShape& VOnF,
                      const TopTools_SequenceOfShape& VOnL,
                      const TColgp_SequenceOfPnt&     ParOnF,
                      const TColgp_SequenceOfPnt&     ParOnL,
                      const BRepFill_TrimSurfaceTool& Trim,
                      const Handle(GeomCurve2d)&     Bis,
                      const TopoVertex&            VS,
                      const TopoVertex&            VE,
                      TColStd_SequenceOfReal&         FirstPar,
                      TColStd_SequenceOfReal&         LastPar,
                      TopTools_SequenceOfShape&       FirstV,
                      TopTools_SequenceOfShape&       LastV)
{
  Standard_Integer IOnF = 1, IOnL = 1;
  Standard_Real    U1 = 0., U2;
  TopoShape     V1, V2;

  if (!VS.IsNull())
  {
    U1 = Bis->FirstParameter();
    V1 = VS;
  }
  while (IOnF <= VOnF.Length() || IOnL <= VOnL.Length())
  {
    //---------------------------------------------------------
    // Return the smallest parameter on the bissectrice
    // corresponding to the current positions IOnF,IOnL.
    //---------------------------------------------------------
    if (IOnL > VOnL.Length()
        || (IOnF <= VOnF.Length() && ParOnF.Value(IOnF).X() < ParOnL.Value(IOnL).X()))
    {

      U2 = ParOnF.Value(IOnF).X();
      V2 = VOnF.Value(IOnF);
      IOnF++;
    }
    else
    {
      U2 = ParOnL.Value(IOnL).X();
      V2 = VOnL.Value(IOnL);
      IOnL++;
    }
    //---------------------------------------------------------------------
    // When V2 and V1 are different the medium point P of the
    // interval is tested compared to the face. If P is in the face the interval
    // is valid.
    //---------------------------------------------------------------------
    if (!V1.IsNull() && !V2.IsSame(V1))
    {
      gp_Pnt2d P = Bis->Value((U2 + U1) * 0.5);
      if (Trim.IsOnFace(P))
      {
        FirstPar.Append(U1);
        LastPar.Append(U2);
        FirstV.Append(V1);
        LastV.Append(V2);
      }
    }
    U1 = U2;
    V1 = V2;
  }

  if (!VE.IsNull())
  {
    U2 = Bis->LastParameter();
    V2 = VE;
    if (!V2.IsSame(V1))
    {
      gp_Pnt2d P = Bis->Value((U2 + U1) * 0.5);
      if (Trim.IsOnFace(P))
      {
        FirstPar.Append(U1);
        LastPar.Append(U2);
        FirstV.Append(V1);
        LastV.Append(V2);
      }
    }
  }
}

//=======================================================================
// function : Relative
// purpose  : Commun is true if two wires have V in common
//           return FORWARD if the wires near the vertex are at
//           the same side. otherwise REVERSED.
//=======================================================================
static TopAbs_Orientation Relative(const TopoWire&   W1,
                                   const TopoWire&   W2,
                                   const TopoVertex& V,
                                   Standard_Boolean&    Commun)
{
  ShapeExplorer Exp;
  TopoEdge     E1, E2;
  TopoVertex   V1, V2;

  for (Exp.Init(W1, TopAbs_EDGE); Exp.More(); Exp.Next())
  {
    const TopoEdge& E = TopoDS::Edge(Exp.Current());
    TopExp1::Vertices(E, V1, V2);
    if (V1.IsSame(V) || V2.IsSame(V))
    {
      E1 = E;
      break;
    }
  }
  for (Exp.Init(W2, TopAbs_EDGE); Exp.More(); Exp.Next())
  {
    const TopoEdge& E = TopoDS::Edge(Exp.Current());
    TopExp1::Vertices(E, V1, V2);
    if (V1.IsSame(V) || V2.IsSame(V))
    {
      E2 = E;
      break;
    }
  }

  if (E1.IsNull() || E2.IsNull())
  {
    Commun = Standard_False;
    return TopAbs_FORWARD;
  }
  Commun = Standard_True;

  TopoWire   WW1 = BRepLib_MakeWire(E1);
  TopoWire   WW2 = BRepLib_MakeWire(E2);
  Standard_Real Tol = BRepFill_Confusion();
  if (Side(WW1, Tol) < 4 && Side(WW2, Tol) < 4) // two to the left
    return TopAbs_FORWARD;
  if (Side(WW1, Tol) > 4 && Side(WW2, Tol) > 4) // two to the right
    return TopAbs_FORWARD;

  return TopAbs_REVERSED;
}

//=======================================================================
// function : IsOnFace
// purpose  : Return the position of the point defined by d1
//           in the face defined by d2 d3.
//
//           0 : the point is out of the face.
//           1 : the point is on edge corresponding to d2.
//           2 : the point is inside the face.
//           3 : the point is on edge corresponding to d3.
//=======================================================================

Standard_Integer PosOnFace(Standard_Real d1, Standard_Real d2, Standard_Real d3)
{
  if (Abs(d1 - d2) <= BRepFill_Confusion())
    return 1;
  if (Abs(d1 - d3) <= BRepFill_Confusion())
    return 3;

  if (d2 < d3)
  {
    if (d1 > (d2 + BRepFill_Confusion()) && d1 < (d3 - BRepFill_Confusion()))
      return 2;
  }
  else
  {
    if (d1 > (d3 + BRepFill_Confusion()) && d1 < (d2 - BRepFill_Confusion()))
      return 2;
  }
  return 0;
}

//=======================================================================
// function : DoubleOrNotInFace
// purpose  : Return True if V appears zero or two times in the sequence
//           of edges EC
//=======================================================================

Standard_Boolean DoubleOrNotInFace(const TopTools_SequenceOfShape& EC, const TopoVertex& V)
{
  Standard_Boolean Vu = Standard_False;

  for (Standard_Integer i = 1; i <= EC.Length(); i++)
  {
    TopoVertex V1, V2;
    TopExp1::Vertices(TopoDS::Edge(EC.Value(i)), V1, V2);
    if (V1.IsSame(V))
    {
      if (Vu)
        return Standard_True;
      else
        Vu = Standard_True;
    }
    if (V2.IsSame(V))
    {
      if (Vu)
        return Standard_True;
      else
        Vu = Standard_True;
    }
  }
  if (Vu)
    return Standard_False;
  else
    return Standard_True;
}

//=================================================================================================

Standard_Real DistanceToOZ(const TopoVertex& V)
{
  Point3d PV3d = BRepInspector::Pnt(V);
  return Abs(PV3d.Y());
}

//=================================================================================================

Standard_Real Altitud(const TopoVertex& V)
{
  Point3d PV3d = BRepInspector::Pnt(V);
  return PV3d.Z();
}

//=================================================================================================

void SimpleExpression(const Bisector_Bisec& B, Handle(GeomCurve2d)& Bis)
{
  Bis = B.Value();

  Handle(TypeInfo) BT = Bis->DynamicType();
  if (BT == STANDARD_TYPE(Geom2d_TrimmedCurve))
  {
    Handle(Geom2d_TrimmedCurve) TrBis  = Handle(Geom2d_TrimmedCurve)::DownCast(Bis);
    Handle(GeomCurve2d)        BasBis = TrBis->BasisCurve();
    BT                                 = BasBis->DynamicType();
    if (BT == STANDARD_TYPE(Bisector_BisecAna))
    {
      Bis = Handle(Bisector_BisecAna)::DownCast(BasBis)->Geom2dCurve();
      Bis = new Geom2d_TrimmedCurve(Bis, TrBis->FirstParameter(), TrBis->LastParameter());
    }
  }
}

//=======================================================================
// function : CutEdgeProf
// purpose  : Projection and Cut of an edge at extrema of distance to axis OZ.
//=======================================================================

void CutEdgeProf(const TopoEdge&            E,
                 const Handle(GeomPlane)&     Plane,
                 const Handle(Geom2d_Line)&    Line,
                 ShapeList&         Cuts,
                 TopTools_DataMapOfShapeShape& MapVerRefMoved)
{
  Cuts.Clear();

  Standard_Real             f, l;
  Handle(GeomCurve3d)        C;
  Handle(Geom_TrimmedCurve) CT;
  Handle(GeomCurve2d)      C2d;
  TopLoc_Location           L;

  // Return the curve associated to each Edge
  C  = BRepInspector::Curve(E, L, f, l);
  CT = new Geom_TrimmedCurve(C, f, l);
  CT->Transform(L.Transformation());

  // project it in the plane and return the associated PCurve
  Dir3d Normal = Plane->Pln().Axis().Direction();
  C             = GeomProjLib1::ProjectOnPlane(CT, Plane, Normal, Standard_False);
  C2d           = GeomProjLib1::Curve2d(C, Plane);

  // Calculate the extrema with the straight line
  TColStd_SequenceOfReal Seq;

  Standard_Real U1 = -Precision::Infinite();
  Standard_Real U2 = Precision::Infinite();
  f                = C2d->FirstParameter();
  l                = C2d->LastParameter();

  Bnd_Box2d           B;
  Geom2dAdaptor_Curve AC2d(C2d);
  Add2dCurve::Add(AC2d, BRepFill_Confusion(), B);
  Standard_Real xmin, xmax;
  B.Get(xmin, U1, xmax, U2);

  //  modified by NIZHNY-EAP Wed Feb  2 16:32:37 2000 ___BEGIN___
  // no sense if C2 is normal to Line or really is a point
  if (U1 != U2)
  {
    Geom2dAPI_ExtremaCurveCurve Extrema(Line, C2d, U1 - 1., U2 + 1., f, l);

    Standard_Integer i, Nb = Extrema.NbExtrema();
    for (i = 1; i <= Nb; i++)
    {
      Extrema.Parameters(i, U1, U2);
      Seq.Append(U2);
    }
  }
  //  modified by NIZHNY-EAP Wed Feb  2 16:33:05 2000 ___END___

  // On calcule les intersection avec Oy.
  Geom2dAdaptor_Curve     ALine(Line);
  constexpr Standard_Real Tol  = Precision::Intersection();
  Standard_Real           TolC = 0.;

  Geom2dInt_GInter Intersector(ALine, AC2d, TolC, Tol);
  Standard_Integer i, Nb = Intersector.NbPoints();

  for (i = 1; i <= Nb; i++)
  {
    Seq.Append(Intersector.Point(i).ParamOnSecond());
  }

  // Compute the new edges.
  ShapeBuilder  Builder;
  TopoVertex VV, Vf, Vl, VRf, VRl;
  TopExp1::Vertices(E, VRf, VRl);

  if (!MapVerRefMoved.IsBound(VRf))
  {
    Builder.MakeVertex(Vf, C->Value(f), BRepInspector::Tolerance(VRf));
    MapVerRefMoved.Bind(VRf, Vf);
  }
  else
  {
    Vf = TopoDS::Vertex(MapVerRefMoved(VRf));
  }

  if (!MapVerRefMoved.IsBound(VRl))
  {
    Builder.MakeVertex(Vl, C->Value(l), BRepInspector::Tolerance(VRl));
    MapVerRefMoved.Bind(VRl, Vl);
  }
  else
  {
    Vl = TopoDS::Vertex(MapVerRefMoved(VRl));
  }

  if (!Seq.IsEmpty())
  {

    Bubble(Seq);

    Standard_Boolean Empty = Standard_False;

    Standard_Real CurParam = f;
    Standard_Real Param;

    while (!Empty)
    {
      Param = Seq.First();
      Seq.Remove(1);
      Empty = Seq.IsEmpty();
      if (Abs(Param - CurParam) > BRepFill_Confusion() && Abs(Param - l) > BRepFill_Confusion())
      {

        VV = BRepLib_MakeVertex(C->Value(Param));

        TopoEdge EE = BRepLib_MakeEdge(C, Vf, VV);
        EE.Orientation(E.Orientation());
        if (EE.Orientation() == TopAbs_FORWARD)
          Cuts.Append(EE);
        else
          Cuts.Prepend(EE);

        // reinitialize
        CurParam = Param;
        Vf       = VV;
      }
    }
  }

  TopoEdge EE = BRepLib_MakeEdge(C, Vf, Vl);
  EE.Orientation(E.Orientation());
  if (EE.Orientation() == TopAbs_FORWARD)
    Cuts.Append(EE);
  else
    Cuts.Prepend(EE);
}

//=======================================================================
// function : CutEdge
// purpose  : Cut an edge at the extrema of curves and at points of inflexion.
//           Closed circles are also cut in two.
//           If <Cuts> are empty the edge is not modified.
//           The first and the last vertex of the original edge
//           belong to the first and last parts respectively.
//=======================================================================
void CutEdge(const TopoEdge& E, const TopoFace& F, ShapeList& Cuts)
{
  Cuts.Clear();
  MAT2d_CutCurve              Cuter;
  Standard_Real               f, l;
  Handle(GeomCurve2d)        C2d;
  Handle(Geom2d_TrimmedCurve) CT2d;

  TopoVertex V1, V2, VF, VL;
  TopExp1::Vertices(E, V1, V2);
  ShapeBuilder B;

  C2d  = BRepInspector::CurveOnSurface(E, F, f, l);
  CT2d = new Geom2d_TrimmedCurve(C2d, f, l);

  if (CT2d->BasisCurve()->IsKind(STANDARD_TYPE(Geom2d_Circle)) && BRepInspector::IsClosed(E))
  {
    //---------------------------
    // Cut closed circle.
    //---------------------------
    Standard_Real m1 = (2 * f + l) / 3.;
    Standard_Real m2 = (f + 2 * l) / 3.;
    gp_Pnt2d      P1 = CT2d->Value(m1);
    gp_Pnt2d      P2 = CT2d->Value(m2);

    TopoVertex VL1          = BRepLib_MakeVertex(Point3d(P1.X(), P1.Y(), 0.));
    TopoVertex VL2          = BRepLib_MakeVertex(Point3d(P2.X(), P2.Y(), 0.));
    TopoShape  aLocalShape1 = E.EmptyCopied();
    TopoShape  aLocalShape2 = E.EmptyCopied();
    TopoShape  aLocalShape3 = E.EmptyCopied();
    TopoEdge   FE           = TopoDS::Edge(aLocalShape1);
    TopoEdge   ME           = TopoDS::Edge(aLocalShape2);
    TopoEdge   LE           = TopoDS::Edge(aLocalShape3);
    //    TopoEdge FE = TopoDS::Edge(E.EmptyCopied());
    //   TopoEdge ME = TopoDS::Edge(E.EmptyCopied());
    //    TopoEdge LE = TopoDS::Edge(E.EmptyCopied());

    FE.Orientation(TopAbs_FORWARD);
    ME.Orientation(TopAbs_FORWARD);
    LE.Orientation(TopAbs_FORWARD);

    B.Add(FE, V1);
    B.Add(FE, VL1.Oriented(TopAbs_REVERSED));
    B.Range(FE, f, m1);

    B.Add(ME, VL1.Oriented(TopAbs_FORWARD));
    B.Add(ME, VL2.Oriented(TopAbs_REVERSED));
    B.Range(ME, m1, m2);

    B.Add(LE, VL2.Oriented(TopAbs_FORWARD));
    B.Add(LE, V2);
    B.Range(LE, m2, l);

    Cuts.Append(FE.Oriented(E.Orientation()));
    Cuts.Append(ME.Oriented(E.Orientation()));
    Cuts.Append(LE.Oriented(E.Orientation()));
    //--------
    // Return.
    //--------
    return;
  }

  //-------------------------
  // Cut of the curve.
  //-------------------------
  Cuter.Perform(CT2d);

  if (Cuter.UnModified())
  {
    //-----------------------------
    // edge not modified => return.
    //-----------------------------
    return;
  }
  else
  {
    //------------------------
    // Creation of cut edges.
    //------------------------
    VF = V1;

    for (Standard_Integer k = 1; k <= Cuter.NbCurves(); k++)
    {
      Handle(Geom2d_TrimmedCurve) CC = Cuter.Value(k);
      if (k == Cuter.NbCurves())
      {
        VL = V2;
      }
      else
      {
        gp_Pnt2d P = CC->Value(CC->LastParameter());
        VL         = BRepLib_MakeVertex(Point3d(P.X(), P.Y(), 0.));
      }
      TopoShape aLocalShape = E.EmptyCopied();
      TopoEdge  NE          = TopoDS::Edge(aLocalShape);
      //      TopoEdge NE = TopoDS::Edge(E.EmptyCopied());
      NE.Orientation(TopAbs_FORWARD);
      B.Add(NE, VF.Oriented(TopAbs_FORWARD));
      B.Add(NE, VL.Oriented(TopAbs_REVERSED));
      B.Range(NE, CC->FirstParameter(), CC->LastParameter());
      Cuts.Append(NE.Oriented(E.Orientation()));
      VF = VL;
    }
  }
}

//=======================================================================
// function : VertexFromNode
// purpose  : Test if the position of aNode correspondingly to the distance to OZ
//           of vertices VF and VL. returns Status.
//           if Status is different from 0 Returned
//           the vertex corresponding to aNode is created.
//=======================================================================

Standard_Integer VertexFromNode(const Handle(MAT_Node)&                    aNode,
                                const TopoEdge&                         E,
                                const TopoVertex&                       VF,
                                const TopoVertex&                       VL,
                                BRepFill_DataMapOfNodeDataMapOfShapeShape& MapNodeVertex,
                                TopoVertex&                             VN)
{
  TopoShape                 ShapeOnNode;
  TopTools_DataMapOfShapeShape EmptyMap;
  Standard_Integer             Status = 0;
  ShapeBuilder                 B;

  if (!aNode->Infinite())
  {
    Status = PosOnFace(aNode->Distance(), DistanceToOZ(VF), DistanceToOZ(VL));
  }
  if (Status == 2)
    ShapeOnNode = E;
  else if (Status == 1)
    ShapeOnNode = VF;
  else if (Status == 3)
    ShapeOnNode = VL;

  if (!ShapeOnNode.IsNull())
  {
    //-------------------------------------------------
    // the vertex will correspond to a node of the map
    //-------------------------------------------------
    if (MapNodeVertex.IsBound(aNode) && MapNodeVertex(aNode).IsBound(ShapeOnNode))
    {
      VN = TopoDS::Vertex(MapNodeVertex(aNode)(ShapeOnNode));
    }
    else
    {
      B.MakeVertex(VN);
      if (!MapNodeVertex.IsBound(aNode))
      {
        MapNodeVertex.Bind(aNode, EmptyMap);
      }
      MapNodeVertex(aNode).Bind(ShapeOnNode, VN);
    }
  }
  return Status;
}
