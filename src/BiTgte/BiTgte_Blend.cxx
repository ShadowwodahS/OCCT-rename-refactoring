// Created on: 1996-12-16
// Created by: Bruno DUMORTIER
// Copyright (c) 1996-1999 Matra Datavision
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

#include <AppCont_Function.hxx>
#include <Approx_FitAndDivide.hxx>
#include <BiTgte_Blend.hxx>
#include <BiTgte_CurveOnEdge.hxx>
#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAlgo_AsDes.hxx>
#include <BRepAlgo_Loop.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepOffset_DataMapOfShapeOffset.hxx>
#include <BRepOffset_Inter2d.hxx>
#include <BRepOffset_Inter3d.hxx>
#include <BRepOffset_Interval.hxx>
#include <BRepOffset_ListOfInterval.hxx>
#include <BRepOffset_MakeLoops.hxx>
#include <BRepOffset_Offset.hxx>
#include <BRepOffset_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_Quilt.hxx>
#include <ChFi3d.hxx>
#include <Convert_CompBezierCurvesToBSplineCurve.hxx>
#include <ElSLib.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2dAPI_ProjectPointOnCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Lin.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Sphere.hxx>
#include <Precision.hxx>
#include <Standard_NotImplemented.hxx>
#include <StdFail_NotDone.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_DataMapOfShapeShape.hxx>
#include <TopTools_ListOfShape.hxx>

#ifdef OCCT_DEBUG
  #include <OSD_Chronometer.hxx>
#endif

#include <stdio.h>
// include - all hxx,
//         - all small static functions.
//======================== START STATIC FUNCTIONS ============
// variables for performance
Standard_Real t_mkcurve;
#ifdef OCCT_DEBUG
extern void ChFi3d_InitChron(OSD_Chronometer& ch);
extern void ChFi3d_ResultChron(OSD_Chronometer& ch, Standard_Real& time);
#endif
#ifdef DRAW
static Standard_Boolean Affich = Standard_False;
static char             name[100];
  #include <DBRep.hxx>
#endif

//=================================================================================================

static Standard_Boolean IsOnRestriction(const TopoVertex& V,
                                        const TopoEdge&   CurE,
                                        const TopoFace&   F,
                                        TopoEdge&         E)
{
  // find if Vertex V of CurE is on a restriction of F.
  // if yes, store this restriction in E.

  // dub - 03 01 97
  // Method somewhat brutal : possible to really optimize by a
  // direct call the SD of intersections -> See LBR

  Standard_Real        f, l;
  Handle(GeomCurve2d) CurC = BRepInspector::CurveOnSurface(CurE, F, f, l);
  Standard_Real        U    = BRepInspector::Parameter(V, CurE, F);
  gp_Pnt2d             P    = CurC->Value(U);

  Geom2dAPI_ProjectPointOnCurve Proj;

  // The tolerance is exaggerated : it is better to construct too many
  // tubes than to miss intersections.
  // Standard_Real Tol = 100 * BRepInspector::Tolerance(V);
  Standard_Real   Tol = BRepInspector::Tolerance(V);
  ShapeExplorer exp(F, TopAbs_EDGE);
  for (; exp.More(); exp.Next())
  {
    E                       = TopoDS::Edge(exp.Current());
    Handle(GeomCurve2d) PC = BRepInspector::CurveOnSurface(E, F, f, l);
    Proj.Init(P, PC, f, l);
    if (Proj.NbPoints() > 0)
    {
      if (Proj.LowerDistance() < Tol)
      {
        return Standard_True;
      }
    }
  }
  return Standard_False;
}

//=================================================================================================

static void Add(const TopoEdge&          E,
                TopTools_IndexedMapOfShape& Map,
                const TopoShape&         S,
                const BRepOffset_Offset&    OF,
                const BRepOffset_Analyse&   Analyse,
                const Standard_Boolean      WarningSurBordLibre)
// If WarningSurBordLibre = TRUE, no propagation if the edge is open.
{
  TopAbs_ShapeEnum Type = S.ShapeType();

  if (Type == TopAbs_FACE)
  {
    ShapeExplorer exp(S, TopAbs_EDGE);
    for (; exp.More(); exp.Next())
    {
      const TopoEdge& OriE        = TopoDS::Edge(exp.Current());
      TopoShape       aLocalShape = OF.Generated(OriE);
      const TopoEdge& IE          = TopoDS::Edge(aLocalShape);
      //      const TopoEdge& IE   = TopoDS::Edge(OF.Generated(OriE));
      if (E.IsEqual(IE))
      {
        if (WarningSurBordLibre)
        {
          // It is checked that the border is not free.
          const ShapeList& L = Analyse.Ancestors(OriE);
          if (L.Extent() == 1)
            break; // Nothing is done.
        }
        Map.Add(exp.Current());
        break;
      }
    }
  }
  else if (Type == TopAbs_EDGE)
  {
    ShapeExplorer exp(S, TopAbs_VERTEX);
    for (; exp.More(); exp.Next())
    {
      TopoShape       aLocalShape = OF.Generated(exp.Current());
      const TopoEdge& IE          = TopoDS::Edge(aLocalShape);
      //      const TopoEdge& IE = TopoDS::Edge(OF.Generated(exp.Current()));
      if (E.IsEqual(IE))
      {
        const ShapeList&        L = Analyse.Ancestors(exp.Current());
        TopTools_ListIteratorOfListOfShape it(L);
        for (; it.More(); it.Next())
        {
          Map.Add(it.Value());
        }
        break;
      }
    }
  }
}

//=================================================================================================

static Standard_Boolean IsInFace(const TopoEdge& E, const TopoFace& F)
{
  ShapeExplorer exp(F, TopAbs_EDGE);
  for (; exp.More(); exp.Next())
    if (E.IsSame(exp.Current()))
      return Standard_True;
  return Standard_False;
}

//=================================================================================================

static void KPartCurve3d(const TopoEdge&   Edge,
                         Handle(GeomCurve2d) Curve,
                         Handle(GeomSurface) Surf)
{
  // try to find the particular case
  // if not found call BRepLib::BuildCurve3d

  TopLoc_Location         Loc;
  constexpr Standard_Real Tol = Precision::Confusion();

  // Search only isos on analytical surfaces.
  Geom2dAdaptor_Curve C(Curve);
  GeomAdaptor_Surface S(Surf);
  GeomAbs_CurveType   CTy = C.GetType();
  GeomAbs_SurfaceType STy = S.GetType();
  ShapeBuilder        TheBuilder;

  if (STy != GeomAbs_Plane)
  { // if plane buildcurve3d manage KPart
    if (CTy == GeomAbs_Line)
    {
      gp_Dir2d D = C.Line().Direction();
      if (D.IsParallel(gp::DX2d(), Precision::Angular()))
      { // Iso V.
        if (STy == GeomAbs_Sphere)
        {
          gp_Pnt2d P = C.Line().Location();
          if (Abs(Abs(P.Y()) - M_PI / 2.) < Precision::PConfusion())
          {
            TheBuilder.Degenerated(Edge, Standard_True);
          }
          else
          {
            gp_Sphere Sph  = S.Sphere();
            gp_Ax3    Axis = Sph.Position();
            gp_Circ   Ci   = ElSLib1::SphereVIso(Axis, Sph.Radius(), P.Y());
            Dir3d    DRev = Axis.XDirection().Crossed(Axis.YDirection());
            Axis3d    AxeRev(Axis.Location(), DRev);
            Ci.Rotate(AxeRev, P.X());
            Handle(GeomCircle) Circle = new GeomCircle(Ci);
            if (D.IsOpposite(gp::DX2d(), Precision::Angular()))
              Circle->Reverse();
            TheBuilder.UpdateEdge(Edge, Circle, Loc, Tol);
          }
        }
        else if (STy == GeomAbs_Cylinder)
        {
          gp_Cylinder Cyl  = S.Cylinder();
          gp_Pnt2d    P    = C.Line().Location();
          gp_Ax3      Axis = Cyl.Position();
          gp_Circ     Ci   = ElSLib1::CylinderVIso(Axis, Cyl.Radius(), P.Y());
          Dir3d      DRev = Axis.XDirection().Crossed(Axis.YDirection());
          Axis3d      AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);
          if (D.IsOpposite(gp::DX2d(), Precision::Angular()))
            Circle->Reverse();
          TheBuilder.UpdateEdge(Edge, Circle, Loc, Tol);
        }
        else if (STy == GeomAbs_Cone)
        {
          gp_Cone  Cone = S.Cone();
          gp_Pnt2d P    = C.Line().Location();
          gp_Ax3   Axis = Cone.Position();
          gp_Circ  Ci   = ElSLib1::ConeVIso(Axis, Cone.RefRadius(), Cone.SemiAngle(), P.Y());
          Dir3d   DRev = Axis.XDirection().Crossed(Axis.YDirection());
          Axis3d   AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);
          if (D.IsOpposite(gp::DX2d(), Precision::Angular()))
            Circle->Reverse();
          TheBuilder.UpdateEdge(Edge, Circle, Loc, Tol);
        }
        else if (STy == GeomAbs_Torus)
        {
          gp_Torus Tore = S.Torus();
          gp_Pnt2d P    = C.Line().Location();
          gp_Ax3   Axis = Tore.Position();
          gp_Circ  Ci   = ElSLib1::TorusVIso(Axis, Tore.MajorRadius(), Tore.MinorRadius(), P.Y());
          Dir3d   DRev = Axis.XDirection().Crossed(Axis.YDirection());
          Axis3d   AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);
          if (D.IsOpposite(gp::DX2d(), Precision::Angular()))
            Circle->Reverse();
          TheBuilder.UpdateEdge(Edge, Circle, Loc, Tol);
        }
      }
      else if (D.IsParallel(gp::DY2d(), Precision::Angular()))
      { // Iso U.
        if (STy == GeomAbs_Sphere)
        {
          gp_Sphere Sph  = S.Sphere();
          gp_Pnt2d  P    = C.Line().Location();
          gp_Ax3    Axis = Sph.Position();
          // calculate iso 0.
          gp_Circ Ci = ElSLib1::SphereUIso(Axis, Sph.Radius(), 0.);

          // set to sameparameter (rotation of the circle - offset from Y)
          Dir3d DRev = Axis.XDirection().Crossed(Axis.Direction());
          Axis3d AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.Y());

          // transformation by iso U ( = P.X())
          DRev   = Axis.XDirection().Crossed(Axis.YDirection());
          AxeRev = Axis3d(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);

          if (D.IsOpposite(gp::DY2d(), Precision::Angular()))
            Circle->Reverse();
          TheBuilder.UpdateEdge(Edge, Circle, Loc, Tol);
        }
        else if (STy == GeomAbs_Cylinder)
        {
          gp_Cylinder Cyl = S.Cylinder();
          gp_Pnt2d    P   = C.Line().Location();
          gp_Lin      L   = ElSLib1::CylinderUIso(Cyl.Position(), Cyl.Radius(), P.X());
          Vector3d      Tr(L.Direction());
          Tr.Multiply(P.Y());
          L.Translate(Tr);
          Handle(GeomLine) Line = new GeomLine(L);
          if (D.IsOpposite(gp::DY2d(), Precision::Angular()))
            Line->Reverse();
          TheBuilder.UpdateEdge(Edge, Line, Loc, Tol);
        }
        else if (STy == GeomAbs_Cone)
        {
          gp_Cone  Cone = S.Cone();
          gp_Pnt2d P    = C.Line().Location();
          gp_Lin   L = ElSLib1::ConeUIso(Cone.Position(), Cone.RefRadius(), Cone.SemiAngle(), P.X());
          Vector3d   Tr(L.Direction());
          Tr.Multiply(P.Y());
          L.Translate(Tr);
          Handle(GeomLine) Line = new GeomLine(L);
          if (D.IsOpposite(gp::DY2d(), Precision::Angular()))
            Line->Reverse();
          TheBuilder.UpdateEdge(Edge, Line, Loc, Tol);
        }
        else if (STy == GeomAbs_Torus)
        {
        }
      }
    }
  }
  else
  { // Case Plane
    Handle(GeomCurve3d) C3d = GeomAPI1::To3d(Curve, S.Plane());
    TheBuilder.UpdateEdge(Edge, C3d, Loc, Tol);
  }
}

//=================================================================================================

class MakeCurve_Function : public ContinuityFunction
{
  BiTgte_CurveOnEdge myCurve;

public:
  MakeCurve_Function(const BiTgte_CurveOnEdge& C)
      : myCurve(C)
  {
    myNbPnt   = 1;
    myNbPnt2d = 0;
  }

  Standard_Real FirstParameter() const { return myCurve.FirstParameter(); }

  Standard_Real LastParameter() const { return myCurve.LastParameter(); }

  Standard_Boolean Value(const Standard_Real theT,
                         NCollection_Array1<gp_Pnt2d>& /*thePnt2d*/,
                         NCollection_Array1<Point3d>& thePnt) const
  {
    thePnt(1) = myCurve.Value(theT);
    return Standard_True;
  }

  Standard_Boolean D1(const Standard_Real /*theT*/,
                      NCollection_Array1<gp_Vec2d>& /*theVec2d*/,
                      NCollection_Array1<Vector3d>& /*theVec*/) const
  {
    return Standard_False;
  }
};

Handle(GeomCurve3d) MakeCurve(const BiTgte_CurveOnEdge& HC)
{
  Handle(GeomCurve3d) C;

#ifdef OCCT_DEBUG
  OSD_Chronometer ch;
  ChFi3d_InitChron(ch);
#endif

  if (HC.GetType() == GeomAbs_Circle)
  {
    C = new GeomCircle(HC.Circle());
    C = new Geom_TrimmedCurve(C, HC.FirstParameter(), HC.LastParameter());
  }
  else
  { // the approximation is done
    MakeCurve_Function F(HC);
    Standard_Integer   Deg1, Deg2;
    Deg1 = Deg2                 = 8;
    constexpr Standard_Real Tol = Precision::Approximation();
    Approx_FitAndDivide     Fit(F, Deg1, Deg2, Tol, Tol, Standard_True);
    Standard_Integer        i;
    Standard_Integer        NbCurves = Fit.NbMultiCurves();
    // it is attempted to make the curve at least C1
    BezierToBSpline Conv;

    for (i = 1; i <= NbCurves; i++)
    {
      AppParCurves_MultiCurve MC = Fit.Value(i);         // Load the Ith Curve
      TColgp_Array1OfPnt      Poles(1, MC.Degree() + 1); // Return poles
      MC.Curve(1, Poles);

      Conv.AddCurve(Poles);
    }

    Conv.Perform();

    Standard_Integer        NbPoles = Conv.NbPoles();
    Standard_Integer        NbKnots = Conv.NbKnots();
    TColgp_Array1OfPnt      NewPoles(1, NbPoles);
    TColStd_Array1OfReal    NewKnots(1, NbKnots);
    TColStd_Array1OfInteger NewMults(1, NbKnots);

    Conv.KnotsAndMults(NewKnots, NewMults);
    Conv.Poles(NewPoles);

    BSplCLib1::Reparametrize(HC.FirstParameter(), HC.LastParameter(), NewKnots);

    C = new BSplineCurve3d(NewPoles, NewKnots, NewMults, Conv.Degree());
  }

#ifdef OCCT_DEBUG
  ChFi3d_ResultChron(ch, t_mkcurve);
#endif

  return C;
}

//=======================================================================
// function : Touched
// purpose  : Only the faces connected with caps are given
//=======================================================================

static void Touched(const BRepOffset_Analyse&,
                    const TopTools_MapOfShape&,
                    const TopoShape&,
                    TopTools_MapOfShape&)
{
  // currently nothing is done !!
  /*if ( Standard_True) {
    return;
  }
  else {
    ShapeExplorer exp(Shape, TopAbs_EDGE);
    for ( ; exp.More(); exp.Next()) {
      const ShapeList& L = Analyse.Ancestors(exp.Current());
      if (StopFaces.Contains(L.First()))
        TouchedByCork.Add(L.Last());
      else if (StopFaces.Contains(L.Last()))
        TouchedByCork.Add(L.First());
    }
  }*/
  return;
}

//=================================================================================================

static TopoVertex FindVertex(const Point3d&              P,
                                const TopTools_MapOfShape& Map,
                                const Standard_Real        Tol)
{
  ShapeBuilder B;
  // Find in <Map> a vertex which represent the point <P>.
  Standard_Real                    Tol2, Dist;
  TopoVertex                    V, VV[2];
  Standard_Real                    TolCarre = Tol * Tol;
  TopTools_MapIteratorOfMapOfShape it(Map);
  for (; it.More(); it.Next())
  {
    const TopoEdge& E = TopoDS::Edge(it.Key());
    if (!E.IsNull())
    {
      TopExp1::Vertices(E, VV[0], VV[1]);

      for (Standard_Integer i = 0; i < 2; i++)
      {
        // if OK la Tolerance du Vertex
        Tol2 = BRepInspector::Tolerance(VV[i]);
        Tol2 *= Tol2;
        Point3d P1 = BRepInspector::Pnt(VV[i]);
        Dist      = P.SquareDistance(P1);
        if (Dist <= Tol2)
          return VV[i];
        // otherwise with the required tolerance.
        if (TolCarre > Tol2)
        {
          if (Dist <= TolCarre)
          {
            // so it is necessary to update the tolerance of Vertex.
            B.UpdateVertex(VV[i], Tol);
            return VV[i];
          }
        }
      }
    }
  }

  return V;
}

//=================================================================================================

static TopoEdge MakeDegeneratedEdge(const Handle(GeomCurve3d)& CC, const TopoVertex& VfOnE)
{
  ShapeBuilder            B;
  constexpr Standard_Real Tol = Precision::Confusion();
  // kill trimmed curves
  Handle(GeomCurve3d)        C  = CC;
  Handle(Geom_TrimmedCurve) CT = Handle(Geom_TrimmedCurve)::DownCast(C);
  while (!CT.IsNull())
  {
    C  = CT->BasisCurve();
    CT = Handle(Geom_TrimmedCurve)::DownCast(C);
  }

  TopoVertex V1, V2;
  if (VfOnE.IsNull())
  {
    Point3d P = C->Value(C->FirstParameter());
    B.MakeVertex(V1, P, Tol);
    V2 = V1;
  }
  else
  {
    V1 = V2 = VfOnE;
  }
  V1.Orientation(TopAbs_FORWARD);
  V2.Orientation(TopAbs_REVERSED);

  TopoEdge E;
  B.MakeEdge(E, C, Tol);
  B.Add(E, V1);
  B.Add(E, V2);
  //  B.UpdateVertex(V1,C->FirstParameter(),E,Tol);
  //  B.UpdateVertex(V2,C->LastParameter(),E,Tol);
  B.Range(E, CC->FirstParameter(), CC->LastParameter());
  return E;
}

//=================================================================================================

static TopAbs_Orientation Orientation(const TopoEdge&          E,
                                      const TopoFace&          F,
                                      const ShapeList& L)
{
  TopAbs_Orientation                 Orien = TopAbs_FORWARD;
  TopTools_ListIteratorOfListOfShape itld;
  for (itld.Initialize(L); itld.More(); itld.Next())
  {
    if (itld.Value().IsSame(E))
    {
      Orien = itld.Value().Orientation();
      break;
    }
  }
  if (F.Orientation() == TopAbs_REVERSED)
    Orien = TopAbs1::Reverse(Orien);

  return Orien;
}

//=================================================================================================

static TopoEdge FindCreatedEdge(const TopoVertex&                   V1,
                                   const TopoEdge&                     E,
                                   const BRepOffset_DataMapOfShapeOffset& MapSF,
                                   TopTools_MapOfShape&                   MapOnV,
                                   const BRepOffset_Analyse&              CenterAnalyse,
                                   Standard_Real                          Radius,
                                   Standard_Real                          Tol)
{
  TopoEdge E1;
  if (!CenterAnalyse.HasAncestor(V1))
    return E1; // return a Null Shape.

  ShapeList TangE;
  CenterAnalyse.TangentEdges(E, V1, TangE);

  TopTools_ListIteratorOfListOfShape itl(TangE);
  Standard_Boolean                   Find = Standard_False;
  for (; itl.More() && !Find; itl.Next())
  {
    const TopoEdge& ET = TopoDS::Edge(itl.Value());
    if (MapSF.IsBound(ET))
    {
      TopoShape aLocalShape = MapSF(ET).Generated(V1);
      E1                       = TopoDS::Edge(aLocalShape);
      //      E1 = TopoDS::Edge(MapSF(ET).Generated(V1));
      MapOnV.Add(E1);
      Find = Standard_True;
    }
    else
    {
      // Find the sharing of vertices in case of tangent consecutive 3 edges
      // the second of which is the edge that degenerates the tube.
      TopLoc_Location    CLoc;
      Standard_Real      ff, ll;
      Handle(GeomCurve3d) CET = BRepInspector::Curve(ET, CLoc, ff, ll);
      if (CET->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
      {
        CET = Handle(Geom_TrimmedCurve)::DownCast(CET)->BasisCurve();
      }
      Handle(GeomCircle) Circ = Handle(GeomCircle)::DownCast(CET);
      if (Circ.IsNull())
        continue;
      if (Abs(Circ->Radius() - Abs(Radius)) > Tol)
        continue;

      TopoVertex U1, U2;
      TopExp1::Vertices(ET, U1, U2);
      if (U1.IsSame(V1))
        U1 = U2;
      ShapeList Tang2;
      CenterAnalyse.TangentEdges(ET, U1, Tang2);
      TopTools_ListIteratorOfListOfShape it2(Tang2);
      for (; it2.More(); it2.Next())
      {
        const TopoEdge& ET2 = TopoDS::Edge(it2.Value());
        if (MapSF.IsBound(ET2))
        {
          TopoShape aLocalShape = MapSF(ET2).Generated(U1);
          MapOnV.Add(TopoDS::Edge(aLocalShape));
          //	  MapOnV.Add(TopoDS::Edge(MapSF(ET2).Generated(U1)));
        }
      }
    }
  }
  if (!Find)
  {
    TangE.Clear();
    //	CenterAnalyse.Edges(V1f, OT, TangE);
    if (CenterAnalyse.HasAncestor(V1))
    {
      TangE = CenterAnalyse.Ancestors(V1);
      itl.Initialize(TangE);
      for (; itl.More() && !Find; itl.Next())
      {
        if (MapSF.IsBound(itl.Value()))
        {
          MapOnV.Add(MapSF(itl.Value()).Generated(V1));
        }
      }
    }
  }

  return E1;
}

#ifdef DRAW
//=======================================================================
// function : Bubble
// purpose  : Sets in increasing order the sequence of vertices.
//=======================================================================

static void Bubble(const TopoEdge& E, TopTools_SequenceOfShape& Seq)
{
  Standard_Boolean Invert   = Standard_True;
  Standard_Integer NbPoints = Seq.Length();
  Standard_Real    U1, U2;
  TopoVertex    V1, V2;

  while (Invert)
  {
    Invert = Standard_False;
    for (Standard_Integer i = 1; i < NbPoints; i++)
    {
      TopoShape aLocalShape = Seq.Value(i).Oriented(TopAbs_INTERNAL);
      V1                       = TopoDS::Vertex(aLocalShape);
      aLocalShape              = Seq.Value(i + 1).Oriented(TopAbs_INTERNAL);
      V2                       = TopoDS::Vertex(aLocalShape);
      //      V1 = TopoDS::Vertex(Seq.Value(i)  .Oriented(TopAbs_INTERNAL));
      //      V2 = TopoDS::Vertex(Seq.Value(i+1).Oriented(TopAbs_INTERNAL));

      U1 = BRepInspector::Parameter(V1, E);
      U2 = BRepInspector::Parameter(V2, E);
      if (U2 < U1)
      {
        Seq.Exchange(i, i + 1);
        Invert = Standard_True;
      }
    }
  }
}

//=================================================================================================

static void CutEdge(const TopoEdge&          E,
                    const ShapeList& VOnE,
                    ShapeList&       NE)
{
  TopoShape aLocalShapeOrientedE = E.Oriented(TopAbs_FORWARD);
  TopoEdge  WE                   = TopoDS::Edge(aLocalShapeOrientedE);
  //  TopoEdge WE = TopoDS::Edge(E.Oriented(TopAbs_FORWARD));

  Standard_Real                      U1, U2;
  TopoVertex                      V1, V2;
  TopTools_SequenceOfShape           SV;
  TopTools_ListIteratorOfListOfShape it(VOnE);
  ShapeBuilder                       B;

  for (; it.More(); it.Next())
  {
    SV.Append(it.Value());
  }
  //--------------------------------
  // Parse vertices on the edge.
  //--------------------------------
  Bubble(WE, SV);

  Standard_Integer NbVer = SV.Length();
  //----------------------------------------------------------------
  // Construction of new edges.
  // The vertices at the extremities of edges are not
  // necessarily in the list of vertices
  //----------------------------------------------------------------
  if (SV.IsEmpty())
  {
    NE.Append(E);
    return;
  }
  TopoVertex VF, VL;
  Standard_Real f, l;
  BRepInspector::Range(WE, f, l);
  TopExp1::Vertices(WE, VF, VL);

  if (NbVer == 2)
  {
    if (SV(1).IsEqual(VF) && SV(2).IsEqual(VL))
    {
      NE.Append(E);
      return;
    }
  }
  //----------------------------------------------------
  // Processing of closed edges
  // If a vertex of intersection is on the common vertex,
  // it should appear at the beginning and the end of SV.
  //----------------------------------------------------
  TopoVertex VCEI;

  if (!VF.IsNull() && !VF.IsSame(SV.First()))
    SV.Prepend(VF);
  if (!VL.IsNull() && !VL.IsSame(SV.Last()))
    SV.Append(VL);

  V1 = TopoDS::Vertex(SV.First());
  SV.Remove(1);

  while (!SV.IsEmpty())
  {

    V2 = TopoDS::Vertex(SV.First());
    SV.Remove(1);

    if (V1.IsSame(V2))
    {
      std::cout << "Vertex Confondus dans CutEdges" << std::endl;
      continue;
    }
    //-------------------------------------------
    // Copy the edge and restriction by V1 V2.
    //-------------------------------------------
    TopoShape aLocalShape = WE.EmptyCopied();
    TopoEdge  NewEdge     = TopoDS::Edge(aLocalShape);
    //    TopoEdge NewEdge = TopoDS::Edge(WE.EmptyCopied());
    B.Add(NewEdge, V1.Oriented(TopAbs_FORWARD));
    B.Add(NewEdge, V2.Oriented(TopAbs_REVERSED));
    if (V1.IsSame(VF))
      U1 = f;
    else
    {
      aLocalShape = V1.Oriented(TopAbs_INTERNAL);
      U1          = BRepInspector::Parameter(TopoDS::Vertex(aLocalShape), WE);
      //      U1 = BRepInspector::Parameter
      //	(TopoDS::Vertex(V1.Oriented(TopAbs_INTERNAL)),WE);
    }
    if (V2.IsSame(VL))
      U2 = l;
    else
    {
      aLocalShape = V2.Oriented(TopAbs_INTERNAL);
      U2          = BRepInspector::Parameter(TopoDS::Vertex(aLocalShape), WE);
      //      U2 = BRepInspector::Parameter
      //	(TopoDS::Vertex(V2.Oriented(TopAbs_INTERNAL)),WE);
    }
    B.Range(NewEdge, U1, U2);
    NE.Append(NewEdge.Oriented(E.Orientation()));

    V1 = V2;
  }
}
#endif

//======================== END OF STATIC FUNCTIONS ============

//=================================================================================================

BiTgte_Blend::BiTgte_Blend()
{
  myAsDes      = new BRepAlgo_AsDes();
  myNbBranches = -1;
}

//=================================================================================================

BiTgte_Blend::BiTgte_Blend(const TopoShape&    S,
                           const Standard_Real    Radius,
                           const Standard_Real    Tol,
                           const Standard_Boolean NUBS)
{
  myAsDes = new BRepAlgo_AsDes();
  Init(S, Radius, Tol, NUBS);
}

//=================================================================================================

void BiTgte_Blend::Init(const TopoShape&    S,
                        const Standard_Real    Radius,
                        const Standard_Real    Tol,
                        const Standard_Boolean NUBS)
{
  Clear();
  myShape      = S;
  myTol        = Tol;
  myNubs       = NUBS;
  myRadius     = Radius;
  myNbBranches = -1;
  //  TopExp1::MapShapesAndAncestors(S,TopAbs_EDGE,TopAbs_FACE,myAncestors);
}

//=================================================================================================

void BiTgte_Blend::Clear()
{
  myInitOffsetFace.Clear();
  myImage.Clear();
  myImageOffset.Clear();
  myStopFaces.Clear();
  myAnalyse.Clear();
  myAsDes->Clear();
  myNbBranches = -1;
  myDone       = Standard_False;
}

//=================================================================================================

void BiTgte_Blend::SetStoppingFace(const TopoFace& Face)
{
  myStopFaces.Add(Face);
  //-------------
  // MAJ SD. -> To end loop, set faces of edges
  //-------------
  //  myInitOffsetFace.SetRoot(Face);
  //  myInitOffsetFace.Bind   (Face,Face);
  //  myImageOffset.SetRoot   (Face);
}

//=================================================================================================

void BiTgte_Blend::SetFaces(const TopoFace& F1, const TopoFace& F2)
{
  myFaces.Add(F1);
  myFaces.Add(F2);
}

//=================================================================================================

void BiTgte_Blend::SetEdge(const TopoEdge& Edge)
{
  myEdges.Add(Edge);
}

//=================================================================================================

void BiTgte_Blend::Perform(const Standard_Boolean BuildShape)
{
  myBuildShape = BuildShape;

  // Try cutting to avoid tubes on free borders
  // that are not actually free.
  Handle(BRepBuilderAPI_Sewing) Sew = new BRepBuilderAPI_Sewing(myTol);
  BRepLib::BuildCurves3d(myShape);
  ShapeExplorer expf(myShape, TopAbs_FACE);
  for (; expf.More(); expf.Next())
    Sew->Add(expf.Current());
  Sew->Perform();
  TopoShape SewedShape = Sew->SewedShape();
  if (SewedShape.IsNull())
    throw ExceptionBase("Sewing aux fraises");

  // Check if the sewing modified the orientation.
  expf.Init(myShape, TopAbs_FACE);
  TopoFace        FaceRef = TopoDS::Face(expf.Current());
  TopAbs_Orientation OriRef  = FaceRef.Orientation();
  if (Sew->IsModified(FaceRef))
    FaceRef = TopoDS::Face(Sew->Modified(FaceRef));
  expf.Init(SewedShape, TopAbs_FACE);
  for (; expf.More(); expf.Next())
  {
    const TopoFace& FF = TopoDS::Face(expf.Current());
    if (FaceRef.IsSame(FF) && (FF.Orientation() != OriRef))
    {
      SewedShape.Reverse();
      break;
    }
  }

  // Make SameParameter if Sew does not do it (Detect that edges
  // are not sameparameter but it does nothing.)
  expf.Init(SewedShape, TopAbs_EDGE);
  for (; expf.More(); expf.Next())
  {
    const TopoEdge& sec = TopoDS::Edge(expf.Current());
    BRepLib::SameParameter(sec, BRepInspector::Tolerance(sec));
  }

  TopExp1::MapShapesAndAncestors(SewedShape, TopAbs_EDGE, TopAbs_FACE, myAncestors);

  // Extend myFaces with the faces of the sewed shape.
  expf.Init(myShape, TopAbs_FACE);
  for (; expf.More(); expf.Next())
  {
    const TopoShape& F = expf.Current();
    if (myFaces.Contains(F) && Sew->IsModified(F))
    {
      myFaces.RemoveKey(F);
      myFaces.Add(Sew->Modified(F));
    }
  }

  myShape = SewedShape;
  // end Sewing for false free borders.

#ifdef OCCT_DEBUG
  OSD_Chronometer cl_total, ch;
  Standard_Real   t_total, t_center, t_surface, t_shape;

  t_total   = 0;
  t_center  = 0;
  t_surface = 0;
  t_mkcurve = 0;
  t_shape   = 0;
  ChFi3d_InitChron(cl_total);
#endif

  // ----------------------------------------------------------------
  // place faces with the proper orientation in the initial shape
  // ----------------------------------------------------------------
  ShapeExplorer exp(myShape, TopAbs_FACE);
  for (; exp.More(); exp.Next())
  {
    const TopoShape& F = exp.Current();
    if (myFaces.Contains(F))
    {
      myFaces.RemoveKey(F);
      myFaces.Add(F);
    }
    else if (myStopFaces.Contains(F))
    {
      myStopFaces.Remove(F);
      myStopFaces.Add(F);
    }
  }

  // ----------------------------------------------
  // Calculate lines of centers and of surfaces
  // ----------------------------------------------
#ifdef OCCT_DEBUG
  ChFi3d_InitChron(ch);
#endif

  ComputeCenters();

#ifdef OCCT_DEBUG
  ChFi3d_ResultChron(ch, t_center);
#endif

  // -----------------------------
  // Calculate connection Surfaces
  // -----------------------------
#ifdef OCCT_DEBUG
  ChFi3d_InitChron(ch);
#endif

  ComputeSurfaces();

#ifdef OCCT_DEBUG
  ChFi3d_ResultChron(ch, t_surface);
#endif

  // ----------------------------------
  // Calculate the generated shape if required
  // ----------------------------------
#ifdef OCCT_DEBUG
  ChFi3d_InitChron(ch);
#endif

  if (myBuildShape)
    ComputeShape();

#ifdef OCCT_DEBUG
  ChFi3d_ResultChron(ch, t_shape);
#endif

  // Finally construct curves 3d from edges to be transferred
  // since the partition is provided ( A Priori);
  BRepLib::BuildCurves3d(myResult, Precision::Confusion());

#ifdef OCCT_DEBUG
  ChFi3d_ResultChron(cl_total, t_total);
  std::cout << std::endl;
  std::cout << "Blend_PERFORM: temps total " << t_total << " s  dont :" << std::endl;
  std::cout << "- ComputeCenters  " << t_center << " s" << std::endl;
  std::cout << "- ComputeSurfaces " << t_surface << " s" << std::endl;
  std::cout << "----> MakeCurve   " << t_mkcurve << " s" << std::endl;
  if (myBuildShape)
    std::cout << "- ComputeShape " << t_shape << " s" << std::endl;
#endif

  myDone = Standard_True;
}

//=================================================================================================

Standard_Boolean BiTgte_Blend::IsDone() const
{
  return myDone;
}

//=================================================================================================

const TopoShape& BiTgte_Blend::Shape() const
{
  return myResult;
}

//=================================================================================================

Standard_Integer BiTgte_Blend::NbSurfaces() const
{
  return myCenters.Extent();
}

//=================================================================================================

Handle(GeomSurface) BiTgte_Blend::Surface(const Standard_Integer Index) const
{
  return Surface(myCenters(Index));
}

//=======================================================================
// function : TopoFace&
// purpose  :
//=======================================================================

const TopoFace& BiTgte_Blend::Face(const Standard_Integer Index) const
{
  return Face(myCenters(Index));
}

//=================================================================================================

void BiTgte_Blend::CenterLines(ShapeList& LC) const
{
  LC.Clear();
  Standard_Integer Nb = NbSurfaces();
  for (Standard_Integer i = 1; i <= Nb; i++)
    LC.Append(myCenters(i));
}

//=================================================================================================

Handle(GeomSurface) BiTgte_Blend::Surface(const TopoShape& CenterLine) const
{
  const TopoFace& F = myMapSF(CenterLine).Face();
  return BRepInspector::Surface(F);
}

//=======================================================================
// function : TopoFace&
// purpose  :
//=======================================================================

const TopoFace& BiTgte_Blend::Face(const TopoShape& CenterLine) const
{
  if (!myMapSF.IsBound(CenterLine))
  {
    throw Standard_DomainError("BiTgte_Blend::Face");
  }

  return myMapSF(CenterLine).Face();
}

//=================================================================================================

BiTgte_ContactType BiTgte_Blend::ContactType(const Standard_Integer Index) const
{
  const TopoShape& S1 = SupportShape1(Index);
  const TopoShape& S2 = SupportShape2(Index);

  TopAbs_ShapeEnum Type1 = S1.ShapeType();
  TopAbs_ShapeEnum Type2 = S2.ShapeType();

  if (Type2 < Type1)
  {
    TopAbs_ShapeEnum Dummy = Type1;
    Type1                  = Type2;
    Type2                  = Dummy;
  }
  BiTgte_ContactType Type = BiTgte_VertexVertex;

  switch (Type1)
  {
    case TopAbs_VERTEX:
      switch (Type2)
      {
        case TopAbs_VERTEX:
          Type = BiTgte_VertexVertex;
          break;
        case TopAbs_EDGE:
          Type = BiTgte_EdgeVertex;
          break;
        case TopAbs_FACE:
          Type = BiTgte_FaceVertex;
          break;
        default:
          break;
      }
      break;

    case TopAbs_EDGE:
      switch (Type2)
      {
        case TopAbs_EDGE:
          Type = BiTgte_EdgeEdge;
          break;
        case TopAbs_FACE:
          Type = BiTgte_FaceEdge;
          break;
        default:
          break;
      }
      break;

    case TopAbs_FACE:
      switch (Type2)
      {
        case TopAbs_FACE:
          Type = BiTgte_FaceEdge;
          break;
        default:
          break;
      }
      break;

    default:
      break;
  }

  return Type;
}

//=================================================================================================

const TopoShape& BiTgte_Blend::SupportShape1(const Standard_Integer Index) const
{
  const TopoEdge& CurE = TopoDS::Edge(myCenters(Index));

  const ShapeList& L = myAsDes->Ascendant(CurE);

  // --------------------------------------------------------------
  // F1 and F2 = 2 parallel faces intersecting at CurE.
  // --------------------------------------------------------------
  const TopoFace&  F1  = TopoDS::Face(L.First());
  const TopoShape& Or1 = myInitOffsetFace.ImageFrom(F1);
  return Or1;
}

//=================================================================================================

const TopoShape& BiTgte_Blend::SupportShape2(const Standard_Integer Index) const
{
  const TopoEdge& CurE = TopoDS::Edge(myCenters(Index));

  const ShapeList& L = myAsDes->Ascendant(CurE);

  // --------------------------------------------------------------
  // F1 and F2 = 2 parallel faces intersecting at CurE.
  // --------------------------------------------------------------
  const TopoFace&  F2  = TopoDS::Face(L.Last());
  const TopoShape& Or2 = myInitOffsetFace.ImageFrom(F2);
  return Or2;
}

//=================================================================================================

Handle(GeomCurve3d) BiTgte_Blend::CurveOnShape1(const Standard_Integer Index) const
{
  const TopoEdge&  CurE = TopoDS::Edge(myCenters(Index));
  const TopoShape& F    = myMapSF(CurE).Face();

  // somewhat brutal method based ONLY on the construction of the fillet:
  // the first edge of the tube is exactly the edge on Shape1.

  ShapeExplorer    exp(F, TopAbs_EDGE);
  const TopoEdge& E = TopoDS::Edge(exp.Current());
  Handle(GeomCurve3d) C;
  if (!BRepInspector::Degenerated(E))
  {
    Standard_Real f, l;
    C = BRepInspector::Curve(E, f, l);
    C = new Geom_TrimmedCurve(C, f, l);
  }
  return C;
}

//=================================================================================================

Handle(GeomCurve3d) BiTgte_Blend::CurveOnShape2(const Standard_Integer Index) const
{
  const TopoEdge&  CurE = TopoDS::Edge(myCenters(Index));
  const TopoShape& F    = myMapSF(CurE).Face();

  // somewhat brutal method based ONLY on the construction of the fillet:
  // the first edge of the tube is exactly the edge on Shape2.

  ShapeExplorer exp(F, TopAbs_EDGE);
  exp.Next();
  const TopoEdge& E = TopoDS::Edge(exp.Current());
  Handle(GeomCurve3d) C;
  if (!BRepInspector::Degenerated(E))
  {
    Standard_Real f, l;
    C = BRepInspector::Curve(E, f, l);
    C = new Geom_TrimmedCurve(C, f, l);
  }
  return C;
}

//=================================================================================================

Handle(GeomCurve2d) BiTgte_Blend::PCurveOnFace1(const Standard_Integer /*Index*/) const
{
  Handle(GeomCurve2d) C;
  return C;
}

//=================================================================================================

Handle(GeomCurve2d) BiTgte_Blend::PCurve1OnFillet(const Standard_Integer /*Index*/) const
{
  Handle(GeomCurve2d) C;
  return C;
}

//=================================================================================================

Handle(GeomCurve2d) BiTgte_Blend::PCurveOnFace2(const Standard_Integer /*Index*/) const
{
  Handle(GeomCurve2d) C;
  return C;
}

//=================================================================================================

Handle(GeomCurve2d) BiTgte_Blend::PCurve2OnFillet(const Standard_Integer /*Index*/) const
{
  Handle(GeomCurve2d) C;
  return C;
}

//=================================================================================================

Standard_Integer BiTgte_Blend::NbBranches()
{
  if (myNbBranches != -1)
    return myNbBranches;

  // else, compute the Branches.
  ShapeQuilt Glue;

  Standard_Integer NbFaces = myCenters.Extent();

  if (NbFaces == 0)
    return 0;

  Standard_Integer i;
  for (i = 1; i <= NbFaces; i++)
  {
    const TopoShape& CenterLine = myCenters(i);
    Glue.Add(myMapSF(CenterLine).Face());
  }

  const TopoShape Shells = Glue.Shells();

  // Reorder Map myCenters.
  // The method is brutal and unpolished,
  // it is possible to refine it.
  myNbBranches = 0;
  TopTools_IndexedMapOfShape tmpMap;

  ShapeExplorer exp(Shells, TopAbs_SHELL);
  for (; exp.More(); exp.Next())
  {
    myNbBranches++;
  }

  myIndices = new TColStd_HArray1OfInteger(1, myNbBranches + 1);

  myIndices->SetValue(1, 0);
  Standard_Integer Count = 0;
  Standard_Integer Index = 2;

  exp.Init(Shells, TopAbs_SHELL);
  for (; exp.More(); exp.Next())
  {
    // CurS = the current Shell.
    const TopoShape& CurS = exp.Current();

    ShapeExplorer exp2(CurS, TopAbs_FACE);
    for (; exp2.More(); exp2.Next())
    {
      // CurF = the current face of the current Shell.
      const TopoShape& CurF = exp2.Current();

      for (i = 1; i <= NbFaces; i++)
      {
        const TopoShape& Center = myCenters(i);
        const TopoShape& Rakk   = myMapSF(Center).Face();
        // Rakk = the ith generated connection face
        if (CurF.IsEqual(Rakk))
        {
          tmpMap.Add(Center);
          Count++;
          break;
        }
      }
    }
    myIndices->SetValue(Index, Count);
    Index++;
  }

  myCenters = tmpMap;
  return myNbBranches;
}

//=================================================================================================

void BiTgte_Blend::IndicesOfBranche(const Standard_Integer Index,
                                    Standard_Integer&      From,
                                    Standard_Integer&      To) const
{
  // Attention to the ranking in myIndices:
  // If the branches are  1-4 5-9 10-12, it is ranked in myIndices:
  //                      0 4   9    12
  From = myIndices->Value(Index) + 1;
  To   = myIndices->Value(Index + 1);
}

//=================================================================================================

void BiTgte_Blend::ComputeCenters()
{
  // ------------
  // Preanalyze.
  // ------------
  Standard_Real TolAngle = 2 * ASin(myTol / Abs(myRadius * 0.5));
  myAnalyse.Perform(myShape, TolAngle);

  // ------------------------------------------
  // calculate faces touched by caps
  // ------------------------------------------
  TopTools_MapOfShape TouchedByCork;
  Touched(myAnalyse, myStopFaces, myShape, TouchedByCork);

  // -----------------------
  // init of the intersector
  // -----------------------
  TopAbs_State Side = TopAbs_IN;
  if (myRadius < 0.)
    Side = TopAbs_OUT;
  BRepOffset_Inter3d Inter(myAsDes, Side, myTol);

  TopTools_DataMapOfShapeBox MapSBox;
  TopTools_MapOfShape        Done;
  // TopTools_MapIteratorOfMapOfShape it;

  ShapeBuilder    B;
  TopoCompound Co; // to only know on which edges the tubes are made
  B.MakeCompound(Co);

  // ----------------------------------------
  // Calculate Sections Face/Face + Propagation
  // ----------------------------------------
  Standard_Boolean JenRajoute = Standard_True;
  Standard_Integer i;

  while (JenRajoute)
  {
    JenRajoute = Standard_False;

    Standard_Boolean Fini = Standard_False;

    TopTools_DataMapOfShapeShape EdgeTgt;

    while (!Fini)
    {

      // -------------------------------------------------
      // locate in myFaces the Faces connected to myEdges.
      // -------------------------------------------------
      Fini = Standard_True;
      // for (it.Initialize(myEdges); it.More(); it.Next()) {
      for (i = 1; i <= myEdges.Extent(); i++)
      {
        const TopoEdge& E = TopoDS::Edge(myEdges(i));
        if (BRepInspector::Degenerated(E))
          continue;

        const ShapeList& L = myAncestors.FindFromKey(E);
        if (L.Extent() == 1)
        {
          // So this is a free border onwhich the ball should roll.
          myFaces.Add(E);

          // set in myStopFaces to not propagate the tube on free border.
          myStopFaces.Add(E);
        }
        else
        {
          TopTools_ListIteratorOfListOfShape itl;
          for (itl.Initialize(L); itl.More(); itl.Next())
          {
            const TopoShape& Sh = itl.Value();
            if (!myStopFaces.Contains(Sh))
              myFaces.Add(itl.Value());
          }
        }
      }
      myEdges.Clear();

      // --------------------------------------------
      // Construction of Offsets of all faces.
      // --------------------------------------------
      // for (it.Initialize(myFaces); it.More(); it.Next()) {
      for (i = 1; i <= myFaces.Extent(); i++)
      {
        const TopoShape& AS = myFaces(i);
        if (myMapSF.IsBound(AS))
          continue;

        BRepOffset_Offset OF1;
        TopoFace       BigF;

        if (AS.ShapeType() == TopAbs_FACE)
        {
          const TopoFace& F = TopoDS::Face(myFaces(i));
          if (TouchedByCork.Contains(F))
          {
            BRepOffset_Tool::EnLargeFace(F, BigF, Standard_True);
            OF1.Init(BigF, myRadius, EdgeTgt);
          }
          else
          {
            OF1.Init(F, myRadius, EdgeTgt);
          }
        }
        else
        { // So this is a Free Border edge on which the ball rolls.
          OF1.Init(TopoDS::Edge(AS), myRadius);
        }

        // ------------------------------------
        // Increment the map of created tangents
        // ------------------------------------
        ShapeList Let;
        if (AS.ShapeType() == TopAbs_FACE)
        {
          myAnalyse.Edges(TopoDS::Face(AS), ChFiDS_Tangential, Let);
        }
        TopTools_ListIteratorOfListOfShape itlet(Let);

        for (; itlet.More(); itlet.Next())
        {
          const TopoEdge& Cur = TopoDS::Edge(itlet.Value());
          if (!EdgeTgt.IsBound(Cur))
          {
            TopoShape       aLocalShape = OF1.Generated(Cur);
            const TopoEdge& OTE         = TopoDS::Edge(aLocalShape);
            //	    const TopoEdge& OTE = TopoDS::Edge(OF1.Generated(Cur));
            EdgeTgt.Bind(Cur, OF1.Generated(Cur));
            TopoVertex V1, V2, OV1, OV2;
            TopExp1::Vertices(Cur, V1, V2);
            TopExp1::Vertices(OTE, OV1, OV2);
            ShapeList LE;
            if (!EdgeTgt.IsBound(V1))
            {
              myAnalyse.Edges(V1, ChFiDS_Tangential, LE);
              const ShapeList& LA = myAnalyse.Ancestors(V1);
              if (LE.Extent() == LA.Extent())
                EdgeTgt.Bind(V1, OV1);
            }
            if (!EdgeTgt.IsBound(V2))
            {
              LE.Clear();
              myAnalyse.Edges(V2, ChFiDS_Tangential, LE);
              const ShapeList& LA = myAnalyse.Ancestors(V2);
              if (LE.Extent() == LA.Extent())
                EdgeTgt.Bind(V2, OV2);
            }
          }
        }
        // end of map created tangent

        if (OF1.Status() == BRepOffset_Reversed || OF1.Status() == BRepOffset_Degenerated)
          continue;

        const TopoFace& F1 = OF1.Face();

        // increment S D
        myInitOffsetFace.SetRoot(AS);
        myInitOffsetFace.Bind(AS, F1);

        Box2 Box1;
        BRepBndLib::Add(F1, Box1);
        MapSBox.Bind(F1, Box1);

        // ---------------------------------------------
        // intersection with all already created faces.
        // ---------------------------------------------
        Fini = !Intersect(AS, F1, MapSBox, OF1, Inter);

        if (AS.ShapeType() == TopAbs_FACE)
          B.Add(Co, AS);

        myMapSF.Bind(AS, OF1);
      }
    } // end of : while ( !Fini)

    //--------------------------------------------------------
    // so the offsets were created and intersected.
    // now the tubes are constructed.
    //--------------------------------------------------------
    // Construction of tubes on edge.
    //--------------------------------------------------------
    ChFiDS_TypeOfConcavity OT = ChFiDS_Convex;
    if (myRadius < 0.)
      OT = ChFiDS_Concave;

    TopTools_IndexedDataMapOfShapeListOfShape Map;
    TopExp1::MapShapesAndAncestors(Co, TopAbs_EDGE, TopAbs_FACE, Map);
    TopExp1::MapShapesAndAncestors(Co, TopAbs_VERTEX, TopAbs_EDGE, Map);

    ShapeExplorer exp(Co, TopAbs_EDGE);
    for (; exp.More(); exp.Next())
    {
      const TopoEdge& E = TopoDS::Edge(exp.Current());
      if (myMapSF.IsBound(E))
        continue;

      const ShapeList& Anc = Map.FindFromKey(E);
      if (Anc.Extent() == 2)
      {
        const BRepOffset_ListOfInterval& L = myAnalyse.Type(E);
        if (!L.IsEmpty() && L.First().Type() == OT)
        {
          TopoShape aLocalShapeGen = myMapSF(Anc.First()).Generated(E);
          TopoEdge  EOn1           = TopoDS::Edge(aLocalShapeGen);
          aLocalShapeGen              = myMapSF(Anc.Last()).Generated(E);
          TopoEdge EOn2            = TopoDS::Edge(aLocalShapeGen);
          //	  TopoEdge EOn1 = TopoDS::Edge(myMapSF(Anc.First()).Generated(E));
          //	  TopoEdge EOn2 = TopoDS::Edge(myMapSF(Anc.Last()) .Generated(E));
          // find if exits tangent edges in the original shape
          TopoEdge   E1f, E1l;
          TopoVertex V1f, V1l;
          TopExp1::Vertices(E, V1f, V1l);
          ShapeList TangE;
          myAnalyse.TangentEdges(E, V1f, TangE);
          // find if the pipe on the tangent edges are soon created.
          TopTools_ListIteratorOfListOfShape itl(TangE);
          Standard_Boolean                   Find = Standard_False;
          for (; itl.More() && !Find; itl.Next())
          {
            if (myMapSF.IsBound(itl.Value()))
            {
              TopoShape aLocalShape = myMapSF(itl.Value()).Generated(V1f);
              E1f                      = TopoDS::Edge(aLocalShape);
              //	      E1f  = TopoDS::Edge(myMapSF(itl.Value()).Generated(V1f));
              Find = Standard_True;
            }
          }
          TangE.Clear();
          myAnalyse.TangentEdges(E, V1l, TangE);
          // find if the pipe on the tangent edges are soon created.
          itl.Initialize(TangE);
          Find = Standard_False;
          for (; itl.More() && !Find; itl.Next())
          {
            if (myMapSF.IsBound(itl.Value()))
            {
              TopoShape aLocalShape = myMapSF(itl.Value()).Generated(V1l);
              E1l                      = TopoDS::Edge(aLocalShape);
              //	      E1l  = TopoDS::Edge(myMapSF(itl.Value()).Generated(V1l));
              Find = Standard_True;
            }
          }
          BRepOffset_Offset  OF1(E, EOn1, EOn2, myRadius, E1f, E1l);
          const TopoFace& F1 = OF1.Face();

          // maj S D
          myInitOffsetFace.SetRoot(E);
          myInitOffsetFace.Bind(E, F1);

          Box2 Box1;
          BRepBndLib::Add(F1, Box1);
          MapSBox.Bind(F1, Box1);

          // ---------------------------------------------
          // intersection with all already created faces.
          // ---------------------------------------------
          Standard_Boolean IsOnRest = Intersect(E, F1, MapSBox, OF1, Inter);
          JenRajoute                = JenRajoute || IsOnRest;

          myMapSF.Bind(E, OF1);
        }
      }
    }

  } // end while JenRajoute

  myEdges.Clear();
  myEdges = Inter.NewEdges();

  // -------------------------------------------------------------------
  // now it is necessary to limit edges on the neighbors (otherwise one
  // will go too far and will not be able to construct faces).
  // -------------------------------------------------------------------

  // Proceed with MakeLoops
  TopTools_IndexedDataMapOfShapeListOfShape aDMVV;
  ChFiDS_TypeOfConcavity                    OT = ChFiDS_Concave;
  if (myRadius < 0.)
    OT = ChFiDS_Convex;

  ShapeList LOF;
  // it.Initialize(myFaces);
  for (i = 1; i <= myFaces.Extent(); i++)
  {
    const TopoShape& CurS = myFaces(i);

    // tube on free border, it is undesirable.
    if (myStopFaces.Contains(CurS))
      continue;

    if (!myMapSF.IsBound(CurS))
      continue; // inverted or degenerated

    const TopoFace& CurOF = myMapSF(CurS).Face();
    LOF.Append(CurOF);

    if (CurS.ShapeType() == TopAbs_FACE)
    {
      const TopoFace& CurF = TopoDS::Face(CurS);
      ShapeExplorer    expe(CurF.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
      for (; expe.More(); expe.Next())
      {
        // --------------------------------------------------------------
        // set in myAsDes the edges generated by limitations of the
        // initial square if the type is correct (The edges that will
        // disappear are not set)
        // --------------------------------------------------------------
        const TopoEdge&               CurE = TopoDS::Edge(expe.Current());
        const BRepOffset_ListOfInterval& L    = myAnalyse.Type(CurE);
        if (!L.IsEmpty() && L.First().Type() != OT)
        {
          // a priori doe s not disappear, so it is set
          TopoShape       aLocalShape = myMapSF(CurF).Generated(CurE);
          const TopoEdge& CurOE       = TopoDS::Edge(aLocalShape);
          //	  const TopoEdge& CurOE =
          //	    TopoDS::Edge(myMapSF(CurF).Generated(CurE));
          myAsDes->Add(CurOF, CurOE.Oriented(CurE.Orientation()));
        }
        else
        {
          const ShapeList& Lanc = myAnalyse.Ancestors(CurE);
          if (!myFaces.Contains(Lanc.First()) || !myFaces.Contains(Lanc.Last())
              || myStopFaces.Contains(Lanc.First()) || myStopFaces.Contains(Lanc.Last()))
          {
            TopoShape       aLocalShape = myMapSF(CurF).Generated(CurE);
            const TopoEdge& CurOE       = TopoDS::Edge(aLocalShape);
            //	    const TopoEdge& CurOE =
            //	      TopoDS::Edge(myMapSF(CurF).Generated(CurE));
            myAsDes->Add(CurOF, CurOE.Oriented(CurE.Orientation()));
          }
        }
      }
      TopTools_DataMapOfShapeListOfShape anEmptyMap;
      BRepOffset_Inter2d::Compute(myAsDes,
                                  CurOF,
                                  myEdges,
                                  myTol,
                                  anEmptyMap,
                                  aDMVV,
                                  Message_ProgressRange());
    }
  }

  // ----------------------------------------------------------------
  // It is also required to make 2D intersections with generated tubes
  // (Useful for unwinding)
  // ----------------------------------------------------------------
  BRepOffset_DataMapIteratorOfDataMapOfShapeOffset It(myMapSF);
  for (; It.More(); It.Next())
  {
    const TopoShape& CurS = It.Key();
    if (CurS.ShapeType() == TopAbs_FACE)
      continue;

    const TopoFace& CurOF = It.Value().Face();

    // no unwinding by tubes on free border.
    if (myStopFaces.Contains(CurS))
      continue;

    LOF.Append(CurOF);

    // --------------------------------------------------------------
    // set in myAsDes the edge restrictions of the square
    // --------------------------------------------------------------
    ShapeExplorer expe(CurOF.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
    for (; expe.More(); expe.Next())
    {
      const TopoEdge& CurOE = TopoDS::Edge(expe.Current());
      myAsDes->Add(CurOF, CurOE);
    }

    TopTools_DataMapOfShapeListOfShape anEmptyMap;
    BRepOffset_Inter2d::Compute(myAsDes,
                                CurOF,
                                myEdges,
                                myTol,
                                anEmptyMap,
                                aDMVV,
                                Message_ProgressRange());
  }
  //
  // fuse vertices on edges stored in AsDes
  ShapeImage anEmptyImage;
  BRepOffset_Inter2d::FuseVertices(aDMVV, myAsDes, anEmptyImage);
  // ------------
  // unwinding
  // ------------
  BRepOffset_MakeLoops MakeLoops;
  MakeLoops.Build(LOF, myAsDes, myImageOffset, anEmptyImage, Message_ProgressRange());

  // ------------------------------------------------------------
  // It is possible to unwind edges at least one ancestor which of
  // is a face of the initial shape, so:
  // the edges generated by intersection tube-tube are missing
  // ------------------------------------------------------------

  // --------------------------------------------------------------
  // Currently set the unwinded surfaces in <myResult>
  // --------------------------------------------------------------
  B.MakeCompound(TopoDS::Compound(myResult));
  TopTools_ListIteratorOfListOfShape itLOF(LOF);
  for (; itLOF.More(); itLOF.Next())
  {
    const TopoShape& CurLOF = itLOF.Value();

    if (!myImageOffset.HasImage(CurLOF))
      continue;

    ShapeList Lim;
    myImageOffset.LastImage(CurLOF, Lim);
    TopTools_ListIteratorOfListOfShape itLim(Lim);
    for (; itLim.More(); itLim.Next())
    {
      // If a face is its own image, it is not set
      const TopoShape& CurLIM = itLim.Value();
      if (CurLIM.IsSame(CurLOF))
        break;

      B.Add(myResult, CurLIM);
    }
  }

#ifdef OCCT_DEBUG
  if (myResult.IsNull())
  {
    std::cout << " No Lines of Generated Centers" << std::endl;
  }
  #ifdef DRAW
  else
  {
    if (Affich)
      DBRep1::Set("Unwind", myResult);
  }
  #endif
#endif
}

//=================================================================================================

void BiTgte_Blend::ComputeSurfaces()
{
  // set in myFaces, the faces actually implied in the connection
  myFaces.Clear();

  // construct
  // 1 - Tubes (True Fillets)
  // 2 - Spheres.

#ifdef DRAW
  Standard_Integer nbc = 1;
#endif

  ShapeList               Empty;
  TopTools_DataMapOfShapeListOfShape EmptyMap;

  Handle(GeomSurface) GS1, GS2;
  Handle(GeomCurve3d)   GC1, GC2;

  Standard_Real      TolAngle = 2 * ASin(myTol / Abs(myRadius * 0.5));
  BRepOffset_Analyse CenterAnalyse(myResult, TolAngle);

  // -----------------------------------------------------
  // Construction of tubes in myResult
  // -----------------------------------------------------
  ShapeBuilder B;
  B.MakeCompound(TopoDS::Compound(myResult));

  // --------------------------------------------------------------------
  // Dummy: for construction of spheres:
  // Set in Co the center line, then it there are at least 3
  // center lines sharing the same vertex, Sphere on this vertex.
  // --------------------------------------------------------------------
  TopoCompound Co;
  B.MakeCompound(Co);

  // --------------------------------------------------------------------
  // Iteration on the edges lines of center
  // and their valid valid part is taken after cut and tube construction.
  // --------------------------------------------------------------------

  // TopTools_MapIteratorOfMapOfShape ic(myEdges);
  Standard_Integer i;
  for (i = 1; i <= myEdges.Extent(); i++)
  {
    const TopoEdge& CurE = TopoDS::Edge(myEdges(i));

    const ShapeList& L = myAsDes->Ascendant(CurE);
    if (L.Extent() != 2)
      continue;

    // --------------------------------------------------------------
    // F1 and F2 = 2 parallel faces intersecting in CurE.
    // --------------------------------------------------------------
    const TopoFace& F1 = TopoDS::Face(L.First());
    const TopoFace& F2 = TopoDS::Face(L.Last());

    // -----------------------------------------------------
    // find the orientation of edges of intersection
    // in the initial faces.
    // -----------------------------------------------------
    const ShapeList& LD1 = myAsDes->Descendant(F1);
    const ShapeList& LD2 = myAsDes->Descendant(F2);

    TopAbs_Orientation Orien1 = Orientation(CurE, F1, LD1);
    TopAbs_Orientation Orien2 = Orientation(CurE, F2, LD2);

    // ---------------------------------------------------------
    // Or1 and Or2 : the shapes generators of parallel faces
    // ---------------------------------------------------------
    const TopoShape& Or1 = myInitOffsetFace.ImageFrom(F1);
    const TopoShape& Or2 = myInitOffsetFace.ImageFrom(F2);

    myFaces.Add(Or1);
    myFaces.Add(Or2);

    TopoEdge     OE1, OE2;
    TopoFace     OF1, OF2;
    TopLoc_Location Loc;
    Standard_Real   f1, l1, f2, l2;

    Standard_Boolean OF1isEdge = Standard_False;

    if (Or1.ShapeType() == TopAbs_EDGE)
    {
      OF1isEdge = Standard_True;
      OE1       = TopoDS::Edge(Or1);
      GC1       = BRepInspector::Curve(OE1, Loc, f1, l1);
      GC1       = Handle(GeomCurve3d)::DownCast(GC1->Transformed(Loc.Transformation()));
    }
    else if (Or1.ShapeType() == TopAbs_FACE)
    {
      OF1 = TopoDS::Face(Or1);
      GS1 = BRepInspector::Surface(OF1);
    }

    // ----------------------------------------------------------------
    // If a vertex is used in contact, currently nothing is done
    // and the vertexes are not managed (Intersections with sphere);
    // ----------------------------------------------------------------
    if (OF1.IsNull() && OE1.IsNull())
      continue;

    Standard_Boolean OF2isEdge = Standard_False;

    if (Or2.ShapeType() == TopAbs_EDGE)
    {
      OF2isEdge = Standard_True;
      OE2       = TopoDS::Edge(Or2);
      GC2       = BRepInspector::Curve(OE2, Loc, f2, l2);
      GC2       = Handle(GeomCurve3d)::DownCast(GC2->Transformed(Loc.Transformation()));
    }
    else if (Or2.ShapeType() == TopAbs_FACE)
    {
      OF2 = TopoDS::Face(Or2);
      GS2 = BRepInspector::Surface(OF2);
    }
    // ----------------------------------------------------------------
    // If a vertex is used in contact, currently nothing is done
    // and the vertexes are not managed (Intersections with sphere);
    // ----------------------------------------------------------------
    if (OF2.IsNull() && OE2.IsNull())
      continue;

    ShapeList CurL;

    if (!myImageOffset.HasImage(CurE))
    { // the tubes are not unwinded
      if (OF1isEdge && OF2isEdge)
      {                    // if I don't have the image, possibly
        CurL.Append(CurE); // I'm on intersection tube-tube
      } // See comment on the call to
      else // MakeLoops
        continue;
    }
    else
    {
      myImageOffset.LastImage(CurE, CurL);
    }

    // ---------------------------------------------------------------
    // CurL = List of edges descending from CurE ( = Cuts of CurE)
    // ---------------------------------------------------------------
    TopTools_ListIteratorOfListOfShape itl(CurL);
    for (; itl.More(); itl.Next())
    {
      const TopoEdge& CurCutE = TopoDS::Edge(itl.Value());

      Handle(GeomCurve2d) PC1 = BRepInspector::CurveOnSurface(CurCutE, F1, f1, l1);
      Handle(GeomCurve2d) PC2 = BRepInspector::CurveOnSurface(CurCutE, F2, f2, l2);
      if (PC1.IsNull() || PC2.IsNull())
      {
#ifdef OCCT_DEBUG
        std::cout << "No PCurves on Intersections : No tubes constructed";
        std::cout << std::endl;
#endif
        continue;
      }

      TopoEdge          E1f, E1l;
      TopoVertex        V1f, V1l;
      TopoVertex        VfOnE1, VlOnE1, VfOnE2, VlOnE2;
      ShapeList TangE;
      TopTools_MapOfShape  MapOnV1f, MapOnV1l;

      TopExp1::Vertices(CurCutE, V1f, V1l);

      // find if the pipe on the tangent edges are soon created.
      // edges generated by V1f and V1l + Maj MapOnV1f/l
      E1f = FindCreatedEdge(V1f, CurCutE, myMapSF, MapOnV1f, CenterAnalyse, myRadius, myTol);

      E1l = FindCreatedEdge(V1l, CurCutE, myMapSF, MapOnV1l, CenterAnalyse, myRadius, myTol);

      TopoEdge E1, E2;
      if (OF1isEdge)
      {
        BiTgte_CurveOnEdge ConE(CurCutE, OE1);
        Handle(GeomCurve3d) C  = MakeCurve(ConE);
        Point3d             P1 = C->Value(C->FirstParameter());
        Point3d             P2 = C->Value(C->LastParameter());
        VfOnE1                = FindVertex(P1, MapOnV1f, myTol);
        if (VfOnE1.IsNull())
          VfOnE1 = FindVertex(P1, MapOnV1l, myTol);
        VlOnE1 = FindVertex(P2, MapOnV1l, myTol);
        if (VlOnE1.IsNull())
          VlOnE1 = FindVertex(P2, MapOnV1f, myTol);
        if (P1.SquareDistance(P2) < myTol * myTol)
        {
          // BRepOffset_Offset manages degenerated KPart
          // It is REQUIRED that C should be a circle with ZERO radius
          E1 = MakeDegeneratedEdge(C, VfOnE1);
        }
        else
        {
          E1 = BRepLib_MakeEdge(C, VfOnE1, VlOnE1);
        }
      }
      else
      {
        gp_Pnt2d P2d;
        P2d       = PC1->Value(f1);
        Point3d P1 = GS1->Value(P2d.X(), P2d.Y());
        P2d       = PC1->Value(l1);
        Point3d P2 = GS1->Value(P2d.X(), P2d.Y());
        VfOnE1    = FindVertex(P1, MapOnV1f, myTol);
        VlOnE1    = FindVertex(P2, MapOnV1l, myTol);
        BRepLib_MakeEdge MKE(PC1, GS1, VfOnE1, VlOnE1, f1, l1);
        if (MKE.IsDone())
          E1 = MKE.Edge();
        else
        {
          std::cout << "Edge Not Done" << std::endl;
          E1 = MKE.Edge();
        }

        KPartCurve3d(E1, PC1, GS1);
      }

      if (OF2isEdge)
      {
        BiTgte_CurveOnEdge ConE(CurCutE, OE2);
        Handle(GeomCurve3d) C  = MakeCurve(ConE);
        Point3d             P1 = C->Value(C->FirstParameter());
        Point3d             P2 = C->Value(C->LastParameter());
        VfOnE2                = FindVertex(P1, MapOnV1f, myTol);
        if (VfOnE2.IsNull())
          VfOnE2 = FindVertex(P1, MapOnV1l, myTol);
        VlOnE2 = FindVertex(P2, MapOnV1l, myTol);
        if (VlOnE2.IsNull())
          VlOnE2 = FindVertex(P2, MapOnV1f, myTol);
        if (P1.SquareDistance(P2) < myTol * myTol)
        {
          // BRepOffset_Offset manages degenerated KParts
          // It is REQUIRED that C should be a circle with ZERO radius
          E2 = MakeDegeneratedEdge(C, VfOnE2);
        }
        else
        {
          E2 = BRepLib_MakeEdge(C, VfOnE2, VlOnE2);
        }
      }
      else
      {
        gp_Pnt2d P2d;
        P2d       = PC2->Value(f2);
        Point3d P1 = GS2->Value(P2d.X(), P2d.Y());
        P2d       = PC2->Value(l2);
        Point3d P2 = GS2->Value(P2d.X(), P2d.Y());
        VfOnE2    = FindVertex(P1, MapOnV1f, myTol);
        VlOnE2    = FindVertex(P2, MapOnV1l, myTol);
        BRepLib_MakeEdge MKE(PC2, GS2, VfOnE2, VlOnE2, f2, l2);
        if (MKE.IsDone())
          E2 = MKE.Edge();
        else
        {
          std::cout << "edge not Done" << std::endl;
          E2 = MKE.Edge();
        }
        KPartCurve3d(E2, PC2, GS2);
      }
      // Increment of the Map of Created if reconstruction of the Shape is required
      if (myBuildShape)
      {
        myCreated.Bind(CurCutE, EmptyMap);

        myCreated(CurCutE).Bind(Or1, Empty);
        myCreated(CurCutE)(Or1).Append(E1);

        myCreated(CurCutE).Bind(Or2, Empty);
        myCreated(CurCutE)(Or2).Append(E2);
      }

      // ----------------------------------------------------------
      // try to init E1f, E1l, if not found with Analysis.
      // Should happen only if the THEORETICALLY tangent edges
      // are not actually tangent ( Cf: Approximation of lines
      // of intersection that add noise.)
      // ----------------------------------------------------------
      TopoVertex aVertex1, aVertex2;
      if (E1f.IsNull() && !VfOnE1.IsNull() && !VfOnE2.IsNull())
      {
        TopTools_MapIteratorOfMapOfShape it(MapOnV1f);
        for (; it.More(); it.Next())
        {
          const TopoEdge& E = TopoDS::Edge(it.Key());
          if (!E.IsNull())
          {
            TopExp1::Vertices(E, aVertex1, aVertex2);
            if ((aVertex1.IsSame(VfOnE1) && aVertex2.IsSame(VfOnE2))
                || (aVertex2.IsSame(VfOnE1) && aVertex1.IsSame(VfOnE2)))
            {
              E1f = E;
              break;
            }
          }
        }
      }
      if (E1l.IsNull() && !VlOnE1.IsNull() && !VlOnE2.IsNull())
      {
        TopTools_MapIteratorOfMapOfShape it(MapOnV1l);
        for (; it.More(); it.Next())
        {
          const TopoEdge& E = TopoDS::Edge(it.Key());
          if (!E.IsNull())
          {
            TopExp1::Vertices(E, aVertex1, aVertex2);
            if ((aVertex1.IsSame(VlOnE1) && aVertex2.IsSame(VlOnE2))
                || (aVertex2.IsSame(VlOnE1) && aVertex1.IsSame(VlOnE2)))
            {
              E1l = E;
              break;
            }
          }
        }
      }

      E1.Orientation(Orien1);
      E2.Orientation(Orien2);

      BRepOffset_Offset AnOffset(CurCutE, E1, E2, -myRadius, E1f, E1l, myNubs, myTol, GeomAbs_C2);
      myMapSF.Bind(CurCutE, AnOffset);
      myCenters.Add(CurCutE);
      B.Add(Co, CurCutE);

      const TopoFace& Tuyo = AnOffset.Face();
      B.Add(myResult, Tuyo);

      if (myBuildShape)
      {
        // method based ONLY on the construction of fillet:
        // the first edge of the tube is exactly on Shape1.
        GeomAPI_ProjectPointOnCurve Projector;
        ShapeExplorer             exp(Tuyo, TopAbs_EDGE);
        TopoVertex               V1, V2;
        if (OF1isEdge)
        { // Update CutEdges.
          const TopoEdge& EOnF1 = TopoDS::Edge(exp.Current());
          TopExp1::Vertices(EOnF1, V1, V2);

          Point3d P1 = BRepInspector::Pnt(V1);
          Projector.Init(P1, GC1);
          Standard_Real U1 = Projector.LowerDistanceParameter();

          Point3d P2 = BRepInspector::Pnt(V2);
          Projector.Init(P2, GC1);
          Standard_Real U2 = Projector.LowerDistanceParameter();

          TopoShape aLocalShape = V1.Oriented(TopAbs_INTERNAL);
          B.UpdateVertex(TopoDS::Vertex(aLocalShape), U1, TopoDS::Edge(Or1), myTol);
          aLocalShape = V2.Oriented(TopAbs_INTERNAL);
          B.UpdateVertex(TopoDS::Vertex(aLocalShape), U2, TopoDS::Edge(Or1), myTol);
          //	  B.UpdateVertex(TopoDS::Vertex(V1.Oriented(TopAbs_INTERNAL)),U1,
          //			 TopoDS::Edge(Or1),myTol);
          //	  B.UpdateVertex(TopoDS::Vertex(V2.Oriented(TopAbs_INTERNAL)),U2,
          //			 TopoDS::Edge(Or1),myTol);

          if (!myCutEdges.IsBound(Or1))
          {
            ShapeList Dummy;
            myCutEdges.Bind(Or1, Dummy);
          }
          ShapeList& L1 = myCutEdges(Or1);
          L1.Append(V1);
          L1.Append(V2);
        }
        if (OF2isEdge)
        { // Update CutEdges.
          exp.Next();
          const TopoEdge& EOnF2 = TopoDS::Edge(exp.Current());
          TopExp1::Vertices(EOnF2, V1, V2);

          Point3d P1 = BRepInspector::Pnt(V1);
          Projector.Init(P1, GC2);
          Standard_Real U1 = Projector.LowerDistanceParameter();

          Point3d P2 = BRepInspector::Pnt(V2);
          Projector.Init(P2, GC2);
          Standard_Real U2 = Projector.LowerDistanceParameter();

          TopoShape aLocalShape = V1.Oriented(TopAbs_INTERNAL);
          B.UpdateVertex(TopoDS::Vertex(aLocalShape), U1, TopoDS::Edge(Or2), myTol);
          aLocalShape = V2.Oriented(TopAbs_INTERNAL);
          B.UpdateVertex(TopoDS::Vertex(aLocalShape), U2, TopoDS::Edge(Or2), myTol);
          //	  B.UpdateVertex(TopoDS::Vertex(V1.Oriented(TopAbs_INTERNAL)),U1,
          //			 TopoDS::Edge(Or2),myTol);
          //	  B.UpdateVertex(TopoDS::Vertex(V2.Oriented(TopAbs_INTERNAL)),U2,
          //			 TopoDS::Edge(Or2),myTol);

          if (!myCutEdges.IsBound(Or2))
          {
            ShapeList Dummy;
            myCutEdges.Bind(Or2, Dummy);
          }
          ShapeList& L2 = myCutEdges(Or2);
          L2.Append(V1);
          L2.Append(V2);
        }
      }

#ifdef DRAW
      if (Affich)
      {
        sprintf(name, "%s_%d", "SURF", nbc);
        DBRep1::Set(name, AnOffset.Face());
        nbc++;
      }
#endif
    }
  }

  // ---------------------------------------------------
  // Construction of spheres,
  // if enough tubes arrive at the vertex
  // ---------------------------------------------------
  TopTools_IndexedDataMapOfShapeListOfShape Map;
  TopExp1::MapShapesAndAncestors(Co, TopAbs_VERTEX, TopAbs_EDGE, Map);

  for (Standard_Integer j = 1; j <= Map.Extent(); j++)
  {
    const TopoVertex& V = TopoDS::Vertex(Map.FindKey(j));
    if (Map(j).Extent() != 3)
      continue;

    ShapeList               LOE;
    TopTools_ListIteratorOfListOfShape it;

    for (it.Initialize(Map(j)); it.More(); it.Next())
    {
      Standard_Boolean Reverse = Standard_True;
      if (Reverse)
        LOE.Append(myMapSF(it.Value()).Generated(V).Reversed());
      else
        LOE.Append(myMapSF(it.Value()).Generated(V));
    }

    BRepOffset_Offset OFT(V, LOE, -myRadius, myNubs, myTol, GeomAbs_C2);
    myMapSF.Bind(V, OFT);
    myCenters.Add(V);

    B.Add(myResult, OFT.Face());

#ifdef DRAW
    if (Affich)
    {
      sprintf(name, "%s_%d", "SURF", nbc);
      DBRep1::Set(name, OFT.Face());
      nbc++;
    }
#endif
  }
}

//=================================================================================================

#include <TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx>

void BiTgte_Blend::ComputeShape()
{
  // Find in the initial Shapel:
  //  - untouched Faces
  //  - generated tubes
  //  - the faces neighbors of tubes that should be reconstructed preserving sharing.

  // For Debug : Visualize edges of the initial shape that should be reconstructed.
#ifdef DRAW
  if (Affich)
  {
    TopTools_DataMapIteratorOfDataMapOfShapeListOfShape itm(myCutEdges);
    Standard_Integer                                    NbEdges = 0;
    for (; itm.More(); itm.Next())
    {
      const TopoEdge&          E    = TopoDS::Edge(itm.Key());
      const ShapeList& VonE = itm.Value();
      ShapeList        NewE;

      CutEdge(E, VonE, NewE);
      for (TopTools_ListIteratorOfListOfShape it(NewE); it.More(); it.Next())
      {
        sprintf(name, "%s_%d", "CUTE", ++NbEdges);
        DBRep1::Set(name, it.Value());
      }
    }
  }
#endif
  // end debug

  TopTools_DataMapOfShapeShape Created;

  ShapeList               Empty;
  TopTools_DataMapOfShapeListOfShape EmptyMap;

  ShapeBuilder B;

  // Maj of the Map of created.
  // Update edges that do not change in the resulting shape
  // i.e. invariant edges in the unwinding.
  ShapeExplorer exp(myShape, TopAbs_FACE);
  // Standard_Integer nbe = 1;
  for (; exp.More(); exp.Next())
  {

    const TopoFace& CurF = TopoDS::Face(exp.Current());

    if (!myFaces.Contains(CurF))
      continue; // so the face is not touched

    // so the faces are unwinded
    if (!myMapSF.IsBound(CurF))
      continue; // inverted or degenerated

    const BRepOffset_Offset& Offset = myMapSF(CurF);
    const TopoFace&       CurOF  = myMapSF(CurF).Face();

    if (!myImageOffset.HasImage(CurOF)) // face disappears in unwinding
      continue;

    ShapeExplorer exp2(CurF, TopAbs_EDGE);
    for (; exp2.More(); exp2.Next())
    {
      const TopoEdge& CurE        = TopoDS::Edge(exp2.Current());
      TopoShape       aLocalShape = Offset.Generated(CurE);
      const TopoEdge& CurOE       = TopoDS::Edge(aLocalShape);
      //      const TopoEdge& CurOE = TopoDS::Edge(Offset.Generated(CurE));

      if (!myImageOffset.HasImage(CurOE))
        continue;
      // CurOE disappears

      const TopoEdge& ImE = TopoDS::Edge(myImageOffset.Image(CurOE).First());
      if (ImE.IsSame(CurOE))
      {
        myCreated.Bind(CurOE, EmptyMap);
        myCreated(CurOE).Bind(CurF, Empty);
        myCreated(CurOE)(CurF).Append(CurE);
      }
    }
  }

  // The connected faces are already in myResult.
  // So it is necessary to add faces:
  //    - non-touched (so not in myFaces)
  //    - issuing from the unwinding (non degenerated, non inverted, non disappeared)
  exp.Init(myShape, TopAbs_FACE);
  for (; exp.More(); exp.Next())
  {

    const TopoFace& CurF = TopoDS::Face(exp.Current());

    if (!myFaces.Contains(CurF))
    {
      // so the face is not touched
      B.Add(myResult, CurF);
    }
    else
    { // so the faces are unwindeds

      if (!myMapSF.IsBound(CurF))
        continue; // inverted or degenerated

      const TopoFace& CurOF = myMapSF(CurF).Face();

      if (!myImageOffset.HasImage(CurOF)) // face disappears in unwinding
        continue;

      // List of faces generated by a face in the unwinding
      ShapeList Lim;
      myImageOffset.LastImage(CurOF, Lim);
      TopTools_ListIteratorOfListOfShape itLim(Lim);
      for (; itLim.More(); itLim.Next())
      {
        // DeboucFace = offset Face unwinded in "Debouc".
        const TopoFace& DeboucFace = TopoDS::Face(itLim.Value());

        TopLoc_Location      L;
        Handle(GeomSurface) S = BRepInspector::Surface(CurF, L);

        TopoFace NewF;
        B.MakeFace(NewF);
        B.UpdateFace(NewF, S, L, BRepInspector::Tolerance(CurF));

        TopTools_DataMapOfShapeShape MapSS;

        TopoShape       aLocalShape = DeboucFace.Oriented(TopAbs_FORWARD);
        const TopoFace& Face        = TopoDS::Face(aLocalShape);
        //	const TopoFace& Face =
        //	  TopoDS::Face(DeboucFace.Oriented(TopAbs_FORWARD));
        ShapeExplorer exp2(Face, TopAbs_EDGE);
        for (; exp2.More(); exp2.Next())
        {
          const TopoEdge& E = TopoDS::Edge(exp2.Current());
          TopoVertex      V1, V2, OV1, OV2;
          TopExp1::Vertices(E, V1, V2);
          if (myCreated.IsBound(E))
          {
            if (myCreated(E).IsBound(CurF))
            {
              const TopoEdge& OE = TopoDS::Edge(myCreated(E)(CurF).First());
              TopExp1::Vertices(OE, OV1, OV2);
              if (!myCreated.IsBound(V1))
                myCreated.Bind(V1, EmptyMap);
              if (!myCreated.IsBound(V2))
                myCreated.Bind(V2, EmptyMap);
              if (!myCreated(V1).IsBound(CurF))
              {
                myCreated(V1).Bind(CurF, Empty);
                myCreated(V1)(CurF).Append(OV1);
              }
              if (!myCreated(V2).IsBound(CurF))
              {
                myCreated(V2).Bind(CurF, Empty);
                myCreated(V2)(CurF).Append(OV2);
              }
            }
          }
        }

        ShapeExplorer expw(Face, TopAbs_WIRE);
        for (; expw.More(); expw.Next())
        {
          const TopoWire& W = TopoDS::Wire(expw.Current());
          ShapeExplorer    expe(W.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
          TopoWire        OW;
          B.MakeWire(OW);

          for (; expe.More(); expe.Next())
          {
            const TopoEdge&   E = TopoDS::Edge(expe.Current());
            Standard_Real        f, l;
            Handle(GeomCurve2d) C2d = BRepInspector::CurveOnSurface(E, Face, f, l);
            TopoEdge          OE;
            if (MapSS.IsBound(E))
            { // this is an edge of cutting
              OE                                        = TopoDS::Edge(MapSS(E));
              TopoShape         aLocalShapeReversedE = E.Reversed();
              Handle(GeomCurve2d) C2d_1 =
                BRepInspector::CurveOnSurface(TopoDS::Edge(aLocalShapeReversedE), Face, f, l);
              //	      Handle(GeomCurve2d) C2d_1 =
              //		BRepInspector::CurveOnSurface(TopoDS::Edge(E.Reversed()),
              //					  Face,f,l);
              if (E.Orientation() == TopAbs_FORWARD)
                B.UpdateEdge(OE, C2d, C2d_1, NewF, BRepInspector::Tolerance(E));
              else
                B.UpdateEdge(OE, C2d_1, C2d, NewF, BRepInspector::Tolerance(E));
              B.Range(OE, f, l);
            }
            else
            {
              // Is there an image in the Map of Created ?
              if (myCreated.IsBound(E))
              {
                if (myCreated(E).IsBound(CurF))
                {
                  OE = TopoDS::Edge(myCreated(E)(CurF).First());
                }
              }
              else
              {
                B.MakeEdge(OE);
                TopoVertex V1, V2, OV1, OV2;
                TopExp1::Vertices(E, V1, V2);
                if (myCreated.IsBound(V1) && myCreated(V1).IsBound(CurF))
                {
                  OV1 = TopoDS::Vertex(myCreated(V1)(CurF).First());
                }
                else
                {
                  B.MakeVertex(OV1);
                  gp_Pnt2d P2d = C2d->Value(BRepInspector::Parameter(V1, E, Face));
                  Point3d   P;
                  S->D0(P2d.X(), P2d.Y(), P);
                  P.Transform(L.Transformation());
                  B.UpdateVertex(OV1, P, BRepInspector::Tolerance(V1));
                  myCreated.Bind(V1, EmptyMap);
                  myCreated(V1).Bind(CurF, Empty);
                  myCreated(V1)(CurF).Append(OV1);
                }
                if (myCreated.IsBound(V2) && myCreated(V2).IsBound(CurF))
                {
                  OV2 = TopoDS::Vertex(myCreated(V2)(CurF).First());
                }
                else
                {
                  B.MakeVertex(OV2);
                  gp_Pnt2d P2d = C2d->Value(BRepInspector::Parameter(V2, E, Face));
                  Point3d   P;
                  S->D0(P2d.X(), P2d.Y(), P);
                  P.Transform(L.Transformation());
                  B.UpdateVertex(OV2, P, BRepInspector::Tolerance(V2));
                  myCreated.Bind(V2, EmptyMap);
                  myCreated(V2).Bind(CurF, Empty);
                  myCreated(V2)(CurF).Append(OV2);
                }
                B.Add(OE, OV1.Oriented(V1.Orientation()));
                B.Add(OE, OV2.Oriented(V2.Orientation()));
              }
              B.UpdateEdge(OE, C2d, NewF, BRepInspector::Tolerance(E));
              B.Range(OE, f, l);
              //	      ComputeCurve3d(OE,C2d,TheSurf,L,BRepInspector::Tolerance(E));
              MapSS.Bind(E, OE);
            }
            B.Add(OW, OE.Oriented(E.Orientation()));
          }
          B.Add(NewF, OW.Oriented(W.Orientation()));
        }

        NewF.Orientation(DeboucFace.Orientation());

        BRepTools1::Update(NewF);
        B.Add(myResult, NewF);
      }
    }
  }

  // non-regarding the cause, there always remain greeb borders on this Shape, so it is sewn.
  Handle(BRepBuilderAPI_Sewing) Sew = new BRepBuilderAPI_Sewing(myTol);

  BRepLib::BuildCurves3d(myResult);

  exp.Init(myResult, TopAbs_FACE);
  for (; exp.More(); exp.Next())
    Sew->Add(exp.Current());

  Sew->Perform();

  // SameParameter is done in case Sew does not do it (Detect that the edges
  // are not sameparameter but does nothing.)

  const TopoShape& SewedShape = Sew->SewedShape();
  if (!SewedShape.IsNull())
  {
    exp.Init(Sew->SewedShape(), TopAbs_EDGE);
    for (; exp.More(); exp.Next())
    {
      const TopoEdge& sec = TopoDS::Edge(exp.Current());
      BRepLib::SameParameter(sec, BRepInspector::Tolerance(sec));
    }
    myResult = SewedShape;
  }
}

//=================================================================================================

Standard_Boolean BiTgte_Blend::Intersect(const TopoShape&               Init,
                                         const TopoFace&                Face,
                                         const TopTools_DataMapOfShapeBox& MapSBox,
                                         const BRepOffset_Offset&          OF1,
                                         BRepOffset_Inter3d&               Inter)
{
  Standard_Boolean JenRajoute = Standard_False;

  const Box2& Box1 = MapSBox(Face);

  // -----------------------------------------------
  // intersection with all already created faces.
  // -----------------------------------------------
  const TopoShape& InitShape1 = OF1.InitialShape();
  Standard_Boolean    F1surBordLibre =
    InitShape1.ShapeType() == TopAbs_EDGE && myStopFaces.Contains(InitShape1);

  TopTools_MapOfShape                              Done;
  BRepOffset_DataMapIteratorOfDataMapOfShapeOffset It(myMapSF);
  for (; It.More(); It.Next())
  {
    const BRepOffset_Offset& OF2 = It.Value();
    const TopoFace&       F2  = OF2.Face();

    if (Box1.IsOut(MapSBox(F2)))
      continue;

    if (Inter.IsDone(Face, F2))
      continue;

    // 2 tubes created on free border are not intersected.
    const TopoShape& InitShape2 = OF2.InitialShape();
    Standard_Boolean    F2surBordLibre =
      InitShape2.ShapeType() == TopAbs_EDGE && myStopFaces.Contains(InitShape2);

#ifdef OCCT_DEBUG
    if (F1surBordLibre && F2surBordLibre)
    {
      std::cout << "Rejection : 2 tubes on free border are not intersected";
      std::cout << std::endl;
    }
#endif

    if (F1surBordLibre && F2surBordLibre)
      continue;

    // -------------------------------------------------------
    // Tubes are not intersected with neighbor faces.
    // -------------------------------------------------------
    const TopoShape& ItKey = It.Key();

    if (Init.ShapeType() == TopAbs_EDGE)
    {
      if (ItKey.ShapeType() == TopAbs_FACE && IsInFace(TopoDS::Edge(Init), TopoDS::Face(ItKey)))
        continue;
    }

    Inter.FaceInter(Face, F2, myInitOffsetFace);

    // ------------------------------------------
    // an edge of F1 or F2 has been touched ?
    // if yes, add faces in myFaces
    //   ==> JenRajoute = True
    // ------------------------------------------
    ShapeList LInt;
    Done.Clear();
    if (myAsDes->HasCommonDescendant(Face, F2, LInt))
    {
      TopTools_ListIteratorOfListOfShape itl2;
      for (itl2.Initialize(LInt); itl2.More(); itl2.Next())
      {
        const TopoEdge& CurE = TopoDS::Edge(itl2.Value());
        TopoVertex      V1, V2;
        TopoEdge        E1, E2;
        TopExp1::Vertices(CurE, V1, V2);

        if (Done.Add(V1))
        {
          Standard_Boolean IsOnR1 = IsOnRestriction(V1, CurE, Face, E1);
          Standard_Boolean IsOnR2 = IsOnRestriction(V1, CurE, F2, E2);
#ifdef OCCT_DEBUG
          if (IsOnR1 && IsOnR2)
          {
            std::cout << "Leave in the same tps on 2 faces, ";
            std::cout << "propagation only on free border";
            std::cout << std::endl;
          }
#endif
          if (IsOnR1)
          {
            if (!myStopFaces.Contains(Init))
            {
              Add(E1, myEdges, Init, OF1, myAnalyse, IsOnR1 && IsOnR2);
              JenRajoute = Standard_True;
            }
          }
          if (IsOnR2)
          {
            if (!myStopFaces.Contains(ItKey))
            {
              Add(E2, myEdges, ItKey, OF2, myAnalyse, IsOnR1 && IsOnR2);
              JenRajoute = Standard_True;
            }
          }
        }

        if (Done.Add(V2))
        {
          Standard_Boolean IsOnR1 = IsOnRestriction(V2, CurE, Face, E1);
          Standard_Boolean IsOnR2 = IsOnRestriction(V2, CurE, F2, E2);

          // If IsOnR1 && IsOnR2,
          // Leave in the same tps on 2 faces, propagate only on
          // free borders.
          // A priori, only facet is closed.
#ifdef OCCT_DEBUG
          if (IsOnR1 && IsOnR2)
          {
            std::cout << "Leave with the same tps on 2 faces, ";
            std::cout << "propagate only if the border is free";
            std::cout << std::endl;
          }
#endif
          if (IsOnR1)
          {
            if (!myStopFaces.Contains(Init))
            {
              Add(E1, myEdges, Init, OF1, myAnalyse, IsOnR1 && IsOnR2);
              JenRajoute = Standard_True;
            }
          }
          if (IsOnR2)
          {
            if (!myStopFaces.Contains(ItKey))
            {
              Add(E2, myEdges, ItKey, OF2, myAnalyse, IsOnR1 && IsOnR2);
              JenRajoute = Standard_True;
            }
          }
        }
      }
    }
  }

  return JenRajoute;
}
