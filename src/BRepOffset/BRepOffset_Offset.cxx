// Created on: 1995-10-19
// Created by: Bruno DUMORTIER
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

#include <BRepOffset_Offset.hxx>

#include <Adaptor3d_CurveOnSurface.hxx>
#include <BRep_Builder.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeFace.hxx>
#include <BRepLib_MakeWire.hxx>
#include <BRepOffset.hxx>
#include <BRepOffset_Tool.hxx>
#include <BRepTools.hxx>
#include <ElSLib.hxx>
#include <gce_MakePln.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAPI.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomConvert_ApproxSurface.hxx>
#include <GeomFill_Pipe.hxx>
#include <GeomLib.hxx>
#include <GeomProjLib.hxx>
#include <gp.hxx>
#include <gp_Ax3.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Torus.hxx>
#include <GProp_GProps.hxx>
#include <Precision.hxx>
#include <ShapeFix_Shape.hxx>
#include <Standard_ConstructionError.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_SequenceOfShape.hxx>

#ifdef OCCT_DEBUG
static Standard_Boolean Affich   = Standard_False;
static Standard_Integer NbOFFSET = 0;
#endif
#ifdef DRAW
  #include <DrawTrSurf.hxx>
  #include <DBRep.hxx>
#endif
#include <stdio.h>
#include <Geom_BSplineSurface.hxx>

static Point3d GetFarestCorner(const TopoWire& aWire)
{
  TopTools_IndexedMapOfShape Vertices;
  TopExp1::MapShapes(aWire, TopAbs_VERTEX, Vertices);

  Standard_Real MaxDist = 0.;
  Point3d        thePoint;
  for (Standard_Integer i = 1; i <= Vertices.Extent(); i++)
    for (Standard_Integer j = 1; j <= Vertices.Extent(); j++)
    {
      const TopoVertex& V1    = TopoDS::Vertex(Vertices(i));
      const TopoVertex& V2    = TopoDS::Vertex(Vertices(j));
      Point3d               P1    = BRepInspector::Pnt(V1);
      Point3d               P2    = BRepInspector::Pnt(V2);
      Standard_Real        aDist = P1.SquareDistance(P2);
      if (aDist > MaxDist)
      {
        MaxDist  = aDist;
        thePoint = P1;
      }
    }

  return thePoint;
}

//=================================================================================================

static void UpdateEdge(const TopoEdge&        E,
                       const Handle(GeomCurve3d)& C,
                       const TopLoc_Location&    L,
                       const Standard_Real       Tol)
{
  // Cut curves to avoid copies in the extensions.
  ShapeBuilder              B;
  Handle(Geom_TrimmedCurve) BC = Handle(Geom_TrimmedCurve)::DownCast(C);
  if (!BC.IsNull())
  {
    B.UpdateEdge(E, BC->BasisCurve(), L, Tol);
  }
  else
  {
    B.UpdateEdge(E, C, L, Tol);
  }
}

//=================================================================================================

static void UpdateEdge(const TopoEdge&          E,
                       const Handle(GeomCurve2d)& C,
                       const TopoFace&          F,
                       const Standard_Real         Tol)
{
  // Cut curves to avoid copies in the extensions.
  ShapeBuilder                B;
  Handle(Geom2d_TrimmedCurve) BC = Handle(Geom2d_TrimmedCurve)::DownCast(C);
  if (!BC.IsNull())
  {
    B.UpdateEdge(E, BC->BasisCurve(), F, Tol);
  }
  else
  {
    B.UpdateEdge(E, C, F, Tol);
  }
}

//=================================================================================================

static void UpdateEdge(const TopoEdge&          E,
                       const Handle(GeomCurve2d)& C1,
                       const Handle(GeomCurve2d)& C2,
                       const TopoFace&          F,
                       const Standard_Real         Tol)
{
  // Cut curves to avoid copies in the extensions.
  ShapeBuilder                B;
  Handle(GeomCurve2d)        NC1, NC2;
  Handle(Geom2d_TrimmedCurve) BC1 = Handle(Geom2d_TrimmedCurve)::DownCast(C1);
  Handle(Geom2d_TrimmedCurve) BC2 = Handle(Geom2d_TrimmedCurve)::DownCast(C2);
  if (!BC1.IsNull())
    NC1 = BC1->BasisCurve();
  else
    NC1 = C1;
  if (!BC2.IsNull())
    NC2 = BC2->BasisCurve();
  else
    NC2 = C2;
  B.UpdateEdge(E, NC1, NC2, F, Tol);
}

//=======================================================================
// function : ComputeCurve3d
// purpose  : Particular case of Curve On Surface.
//=======================================================================

static void ComputeCurve3d(const TopoEdge&          Edge,
                           const Handle(GeomCurve2d)& Curve,
                           const Handle(GeomSurface)& Surf,
                           const TopLoc_Location&      Loc,
                           Standard_Real               Tol)
{
  // try to find the particular case
  // if not found call BRepLib::BuildCurve3d

  Standard_Boolean IsComputed = Standard_False;

  // Search only isos on analytic surfaces.
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
      if (D.IsParallel(gp1::DX2d(), Precision::Angular()))
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
            Ax3    Axis = Sph.Position();
            gp_Circ   Ci   = ElSLib1::SphereVIso(Axis, Sph.Radius(), P.Y());
            Dir3d    DRev = Axis.XDirection().Crossed(Axis.YDirection());
            Axis3d    AxeRev(Axis.Location(), DRev);
            Ci.Rotate(AxeRev, P.X());
            Handle(GeomCircle) Circle = new GeomCircle(Ci);
            if (D.IsOpposite(gp1::DX2d(), Precision::Angular()))
              Circle->Reverse();
            UpdateEdge(Edge, Circle, Loc, Tol);
          }
          IsComputed = Standard_True;
        }
        else if (STy == GeomAbs_Cylinder)
        {
          Cylinder1 Cyl  = S.Cylinder();
          gp_Pnt2d    P    = C.Line().Location();
          Ax3      Axis = Cyl.Position();
          gp_Circ     Ci   = ElSLib1::CylinderVIso(Axis, Cyl.Radius(), P.Y());
          Dir3d      DRev = Axis.XDirection().Crossed(Axis.YDirection());
          Axis3d      AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);
          if (D.IsOpposite(gp1::DX2d(), Precision::Angular()))
            Circle->Reverse();
          UpdateEdge(Edge, Circle, Loc, Tol);
          IsComputed = Standard_True;
        }
        else if (STy == GeomAbs_Cone)
        {
          Cone1  Cone = S.Cone();
          gp_Pnt2d P    = C.Line().Location();
          Ax3   Axis = Cone.Position();
          gp_Circ  Ci   = ElSLib1::ConeVIso(Axis, Cone.RefRadius(), Cone.SemiAngle(), P.Y());
          Dir3d   DRev = Axis.XDirection().Crossed(Axis.YDirection());
          Axis3d   AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);
          if (D.IsOpposite(gp1::DX2d(), Precision::Angular()))
            Circle->Reverse();
          UpdateEdge(Edge, Circle, Loc, Tol);
          IsComputed = Standard_True;
        }
        else if (STy == GeomAbs_Torus)
        {
          gp_Torus Tore = S.Torus();
          gp_Pnt2d P    = C.Line().Location();
          Ax3   Axis = Tore.Position();
          gp_Circ  Ci   = ElSLib1::TorusVIso(Axis, Tore.MajorRadius(), Tore.MinorRadius(), P.Y());
          Dir3d   DRev = Axis.XDirection().Crossed(Axis.YDirection());
          Axis3d   AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);
          if (D.IsOpposite(gp1::DX2d(), Precision::Angular()))
            Circle->Reverse();
          UpdateEdge(Edge, Circle, Loc, Tol);
          IsComputed = Standard_True;
        }
      }
      else if (D.IsParallel(gp1::DY2d(), Precision::Angular()))
      { // Iso U.
        if (STy == GeomAbs_Sphere)
        {
          gp_Sphere Sph  = S.Sphere();
          gp_Pnt2d  P    = C.Line().Location();
          Ax3    Axis = Sph.Position();
          // calculate iso 0.
          gp_Circ Ci = ElSLib1::SphereUIso(Axis, Sph.Radius(), 0.);

          // set to sameparameter (rotation of circle - offset of Y)
          Dir3d DRev = Axis.XDirection().Crossed(Axis.Direction());
          Axis3d AxeRev(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.Y());

          // transformation en iso U ( = P.X())
          DRev   = Axis.XDirection().Crossed(Axis.YDirection());
          AxeRev = Axis3d(Axis.Location(), DRev);
          Ci.Rotate(AxeRev, P.X());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);

          if (D.IsOpposite(gp1::DY2d(), Precision::Angular()))
            Circle->Reverse();
          UpdateEdge(Edge, Circle, Loc, Tol);
          IsComputed = Standard_True;
        }
        else if (STy == GeomAbs_Cylinder)
        {
          Cylinder1 Cyl = S.Cylinder();
          gp_Pnt2d    P   = C.Line().Location();
          gp_Lin      L   = ElSLib1::CylinderUIso(Cyl.Position(), Cyl.Radius(), P.X());
          Vector3d      Tr(L.Direction());
          Tr.Multiply(P.Y());
          L.Translate(Tr);
          Handle(GeomLine) Line = new GeomLine(L);
          if (D.IsOpposite(gp1::DY2d(), Precision::Angular()))
            Line->Reverse();
          UpdateEdge(Edge, Line, Loc, Tol);
          IsComputed = Standard_True;
        }
        else if (STy == GeomAbs_Cone)
        {
          Cone1  Cone = S.Cone();
          gp_Pnt2d P    = C.Line().Location();
          gp_Lin   L = ElSLib1::ConeUIso(Cone.Position(), Cone.RefRadius(), Cone.SemiAngle(), P.X());
          Vector3d   Tr(L.Direction());
          Tr.Multiply(P.Y());
          L.Translate(Tr);
          Handle(GeomLine) Line = new GeomLine(L);
          if (D.IsOpposite(gp1::DY2d(), Precision::Angular()))
            Line->Reverse();
          UpdateEdge(Edge, Line, Loc, Tol);
          IsComputed = Standard_True;
        }
        else if (STy == GeomAbs_Torus)
        {
          gp_Torus Tore = S.Torus();
          gp_Pnt2d P    = C.Line().Location();
          Ax3   Axis = Tore.Position();
          gp_Circ  Ci   = ElSLib1::TorusUIso(Axis, Tore.MajorRadius(), Tore.MinorRadius(), P.X());
          Ci.Rotate(Ci.Axis(), P.Y());
          Handle(GeomCircle) Circle = new GeomCircle(Ci);

          if (D.IsOpposite(gp1::DY2d(), Precision::Angular()))
            Circle->Reverse();
          UpdateEdge(Edge, Circle, Loc, Tol);
          IsComputed = Standard_True;
        }
      }
    }
  }
  else
  { // Cas Plan
    Handle(GeomCurve3d) C3d = GeomAPI1::To3d(Curve, S.Plane());
    UpdateEdge(Edge, C3d, Loc, Tol);
    IsComputed = Standard_True;
  }
  if (!IsComputed)
  {
    // BRepLib::BuildCurves3d(Edge,Tol);
    // Les Courbes 3d des edges dans le cas general ne sont calcules que si
    //  necessaire
    // ie dans les tuyaux et les bouchons ..
    //  dans la derniere etapes de MakeShells on reconstruira les courbes3d
    //  des edges du resultat qui n en ont pas.
  }
}

//=================================================================================================

BRepOffset_Offset::BRepOffset_Offset() {}

//=================================================================================================

BRepOffset_Offset::BRepOffset_Offset(const TopoFace&     Face,
                                     const Standard_Real    Offset,
                                     const Standard_Boolean OffsetOutside,
                                     const GeomAbs_JoinType JoinType)
{
  Init(Face, Offset, OffsetOutside, JoinType);
}

//=================================================================================================

BRepOffset_Offset::BRepOffset_Offset(const TopoFace&                  Face,
                                     const Standard_Real                 Offset,
                                     const TopTools_DataMapOfShapeShape& Created,
                                     const Standard_Boolean              OffsetOutside,
                                     const GeomAbs_JoinType              JoinType)
{
  Init(Face, Offset, Created, OffsetOutside, JoinType);
}

//=================================================================================================

BRepOffset_Offset::BRepOffset_Offset(const TopoEdge&     Path,
                                     const TopoEdge&     Edge1,
                                     const TopoEdge&     Edge2,
                                     const Standard_Real    Offset,
                                     const Standard_Boolean Polynomial,
                                     const Standard_Real    Tol,
                                     const GeomAbs_Shape    Conti)
{
  Init(Path, Edge1, Edge2, Offset, Polynomial, Tol, Conti);
}

//=================================================================================================

BRepOffset_Offset::BRepOffset_Offset(const TopoEdge&     Path,
                                     const TopoEdge&     Edge1,
                                     const TopoEdge&     Edge2,
                                     const Standard_Real    Offset,
                                     const TopoEdge&     FirstEdge,
                                     const TopoEdge&     LastEdge,
                                     const Standard_Boolean Polynomial,
                                     const Standard_Real    Tol,
                                     const GeomAbs_Shape    Conti)
{
  Init(Path, Edge1, Edge2, Offset, FirstEdge, LastEdge, Polynomial, Tol, Conti);
}

//=================================================================================================

BRepOffset_Offset::BRepOffset_Offset(const TopoVertex&        Vertex,
                                     const ShapeList& LEdge,
                                     const Standard_Real         Offset,
                                     const Standard_Boolean      Polynomial,
                                     const Standard_Real         Tol,
                                     const GeomAbs_Shape         Conti)
{
  Init(Vertex, LEdge, Offset, Polynomial, Tol, Conti);
}

//=================================================================================================

void BRepOffset_Offset::Init(const TopoFace&     Face,
                             const Standard_Real    Offset,
                             const Standard_Boolean OffsetOutside,
                             const GeomAbs_JoinType JoinType)
{
  TopTools_DataMapOfShapeShape Empty;
  Init(Face, Offset, Empty, OffsetOutside, JoinType);
}

//=================================================================================================

void BRepOffset_Offset::Init(const TopoFace&                  Face,
                             const Standard_Real                 Offset,
                             const TopTools_DataMapOfShapeShape& Created,
                             const Standard_Boolean              OffsetOutside,
                             const GeomAbs_JoinType              JoinType)
{
  myShape                = Face;
  Standard_Real myOffset = Offset;
  if (Face.Orientation() == TopAbs_REVERSED)
    myOffset *= -1.;

  TopLoc_Location      L;
  Handle(GeomSurface) S = BRepInspector::Surface(Face, L);
  // On detrime les surfaces, evite des recopies dans les extensions.
  Handle(Geom_RectangularTrimmedSurface) RT = Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
  if (!RT.IsNull())
    S = RT->BasisSurface();
  Standard_Boolean IsTransformed = Standard_False;
  if ((S->IsKind(STANDARD_TYPE(Geom_BSplineSurface))
       || S->IsKind(STANDARD_TYPE(Geom_SurfaceOfLinearExtrusion))
       || S->IsKind(STANDARD_TYPE(Geom_SurfaceOfRevolution))
       || S->IsKind(STANDARD_TYPE(Geom_OffsetSurface)))
      && !L.IsIdentity())
  {
    S = Handle(GeomSurface)::DownCast(S->Copy());
    S->Transform(L.Transformation());
    IsTransformed = Standard_True;
  }
  // particular case of cone
  Handle(Geom_ConicalSurface) Co;
  Co = Handle(Geom_ConicalSurface)::DownCast(S);
  if (!Co.IsNull())
  {
    Standard_Real Uc, Vc;
    Point3d        Apex = Co->Apex();
    ElSLib1::Parameters(Co->Cone(), Apex, Uc, Vc);
    Standard_Real UU1, UU2, VV1, VV2;
    BRepTools1::UVBounds(Face, UU1, UU2, VV1, VV2);
    if (VV2 < Vc && Co->SemiAngle() > 0)
      myOffset *= -1;
    else if (VV1 > Vc && Co->SemiAngle() < 0)
      myOffset *= -1;
    if (!Co->Position().Direct())
      myOffset *= -1;
  }

  Handle(GeomSurface) TheSurf = BRepOffset::Surface(S, myOffset, myStatus);

  // processing offsets of faces with possible degenerated edges
  Standard_Boolean UminDegen = Standard_False;
  Standard_Boolean UmaxDegen = Standard_False;
  Standard_Boolean VminDegen = Standard_False;
  Standard_Boolean VmaxDegen = Standard_False;
  Standard_Boolean UisoDegen = Standard_False;
  Point3d           MinApex, MaxApex;
  Standard_Boolean HasSingularity = Standard_False;
  Standard_Real    uf1, uf2, vf1, vf2, fpar, lpar;
  BRepTools1::UVBounds(Face, uf1, uf2, vf1, vf2);
  if (!(OffsetOutside && JoinType == GeomAbs_Arc)
      && (TheSurf->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface)
          || TheSurf->DynamicType() == STANDARD_TYPE(Geom_OffsetSurface)))
  {
    TopTools_SequenceOfShape DegEdges;
    ShapeExplorer          Explo(Face, TopAbs_EDGE);
    for (; Explo.More(); Explo.Next())
    {
      const TopoEdge& anEdge = TopoDS::Edge(Explo.Current());

      if (BRepInspector::Degenerated(anEdge))
      {
        Standard_Real        aF, aL;
        Handle(GeomCurve2d) c2d = BRepInspector::CurveOnSurface(anEdge, Face, aF, aL);

        gp_Pnt2d aFPnt2d = c2d->Value(aF), aLPnt2d = c2d->Value(aL);

        Point3d aFPnt = S->Value(aFPnt2d.X(), aFPnt2d.Y()),
               aLPnt = S->Value(aLPnt2d.X(), aLPnt2d.Y());

        //  aFPnt.SquareDistance(aLPnt) > Precision::SquareConfusion() -
        // is a sufficient condition of troubles: non-singular case, but edge is degenerated.
        // So, normal handling of degenerated edges is not applicable in case of non-singular point.
        if (aFPnt.SquareDistance(aLPnt) < Precision::SquareConfusion())
        {
          DegEdges.Append(anEdge);
        }
      }
    }
    if (!DegEdges.IsEmpty())
    {
      const Standard_Real TolApex = 1.e-5;
      // define the iso of singularity (u or v)
      TopoEdge          theDegEdge = TopoDS::Edge(DegEdges(1));
      Handle(GeomCurve2d) aCurve     = BRepInspector::CurveOnSurface(theDegEdge, Face, fpar, lpar);
      gp_Pnt2d             fp2d       = aCurve->Value(fpar);
      gp_Pnt2d             lp2d       = aCurve->Value(lpar);
      if (Abs(fp2d.X() - lp2d.X()) <= Precision::PConfusion())
        UisoDegen = Standard_True;

      if (DegEdges.Length() == 2)
      {
        if (UisoDegen)
        {
          UminDegen = Standard_True;
          UmaxDegen = Standard_True;
        }
        else
        {
          VminDegen = Standard_True;
          VmaxDegen = Standard_True;
        }
      }
      else // DegEdges.Length() == 1
      {
        if (UisoDegen)
        {
          if (Abs(fp2d.X() - uf1) <= Precision::Confusion())
            UminDegen = Standard_True;
          else
            UmaxDegen = Standard_True;
        }
        else
        {
          if (Abs(fp2d.Y() - vf1) <= Precision::Confusion())
            VminDegen = Standard_True;
          else
            VmaxDegen = Standard_True;
        }
      }
      if (TheSurf->DynamicType() == STANDARD_TYPE(Geom_ConicalSurface))
      {
        Cone1       theCone = Handle(Geom_ConicalSurface)::DownCast(TheSurf)->Cone();
        Point3d        apex    = theCone.Apex();
        Standard_Real Uapex, Vapex;
        ElSLib1::Parameters(theCone, apex, Uapex, Vapex);
        if (VminDegen)
        {
          TheSurf        = new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, Vapex, vf2);
          MinApex        = apex;
          HasSingularity = Standard_True;
        }
        else if (VmaxDegen)
        {
          TheSurf        = new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, Vapex);
          MaxApex        = apex;
          HasSingularity = Standard_True;
        }
      }
      else // TheSurf->DynamicType() == STANDARD_TYPE(Geom_OffsetSurface)
      {
        if (UminDegen)
        {
          Handle(GeomCurve3d) uiso = TheSurf->UIso(uf1);
          if (BRepOffset_Tool::Gabarit(uiso) > TolApex)
          {
            Handle(GeomSurface) BasisSurf =
              Handle(Geom_OffsetSurface)::DownCast(TheSurf)->BasisSurface();
            Point3d Papex, Pfirst, Pquart, Pmid;
            Papex                                   = BasisSurf->Value(uf1, vf1);
            Pfirst                                  = TheSurf->Value(uf1, vf1);
            Pquart                                  = TheSurf->Value(uf1, 0.75 * vf1 + 0.25 * vf2);
            Pmid                                    = TheSurf->Value(uf1, 0.5 * (vf1 + vf2));
            Vector3d                    DirApex       = Vector3d(Pfirst, Pquart) ^ Vector3d(Pfirst, Pmid);
            Handle(GeomLine)         LineApex      = new GeomLine(Papex, DirApex);
            Vector3d                    DirGeneratrix = BasisSurf->DN(uf1, vf1, 1, 0);
            Handle(GeomLine)         LineGeneratrix = new GeomLine(Pfirst, DirGeneratrix);
            GeomAPI_ExtremaCurveCurve theExtrema(LineGeneratrix, LineApex);
            Point3d                    Pint1, Pint2;
            theExtrema.NearestPoints(Pint1, Pint2);
            Standard_Real length = Pfirst.Distance(Pint1);
            if (OffsetOutside)
            {
              Handle(Geom_BoundedSurface) aSurf =
                new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, vf2);
              GeomLib1::ExtendSurfByLength(aSurf, length, 1, Standard_True, Standard_False);
              TheSurf = aSurf;
              Standard_Real u1, u2, v1, v2;
              TheSurf->Bounds(u1, u2, v1, v2);
              MinApex = TheSurf->Value(u1, vf1);
            }
            else
            {
              Handle(GeomCurve3d)          viso = TheSurf->VIso(vf1);
              GeomAPI_ProjectPointOnCurve Projector(Pint1, viso);
              Standard_Real               NewFirstU = Projector.LowerDistanceParameter();
              TheSurf = new Geom_RectangularTrimmedSurface(TheSurf, NewFirstU, uf2, vf1, vf2);
              MinApex = TheSurf->Value(NewFirstU, vf1);
            }
            HasSingularity = Standard_True;
          }
        } // end of if (UminDegen)
        if (UmaxDegen)
        {
          Handle(GeomCurve3d) uiso = TheSurf->UIso(uf2);
          if (BRepOffset_Tool::Gabarit(uiso) > TolApex)
          {
            Handle(GeomSurface) BasisSurf =
              Handle(Geom_OffsetSurface)::DownCast(TheSurf)->BasisSurface();
            Point3d Papex, Pfirst, Pquart, Pmid;
            Papex                                   = BasisSurf->Value(uf2, vf1);
            Pfirst                                  = TheSurf->Value(uf2, vf1);
            Pquart                                  = TheSurf->Value(uf2, 0.75 * vf1 + 0.25 * vf2);
            Pmid                                    = TheSurf->Value(uf2, 0.5 * (vf1 + vf2));
            Vector3d                    DirApex       = Vector3d(Pfirst, Pquart) ^ Vector3d(Pfirst, Pmid);
            Handle(GeomLine)         LineApex      = new GeomLine(Papex, DirApex);
            Vector3d                    DirGeneratrix = BasisSurf->DN(uf2, vf1, 1, 0);
            Handle(GeomLine)         LineGeneratrix = new GeomLine(Pfirst, DirGeneratrix);
            GeomAPI_ExtremaCurveCurve theExtrema(LineGeneratrix, LineApex);
            Point3d                    Pint1, Pint2;
            theExtrema.NearestPoints(Pint1, Pint2);
            Standard_Real length = Pfirst.Distance(Pint1);
            if (OffsetOutside)
            {
              Handle(Geom_BoundedSurface) aSurf =
                new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, vf2);
              GeomLib1::ExtendSurfByLength(aSurf, length, 1, Standard_True, Standard_True);
              TheSurf = aSurf;
              Standard_Real u1, u2, v1, v2;
              TheSurf->Bounds(u1, u2, v1, v2);
              MaxApex = TheSurf->Value(u2, vf1);
            }
            else
            {
              Handle(GeomCurve3d)          viso = TheSurf->VIso(vf1);
              GeomAPI_ProjectPointOnCurve Projector(Pint1, viso);
              Standard_Real               NewLastU = Projector.LowerDistanceParameter();
              TheSurf = new Geom_RectangularTrimmedSurface(TheSurf, uf1, NewLastU, vf1, vf2);
              MaxApex = TheSurf->Value(NewLastU, vf1);
            }
            HasSingularity = Standard_True;
          }
        } // end of if (UmaxDegen)
        if (VminDegen)
        {
          Handle(GeomCurve3d) viso = TheSurf->VIso(vf1);
          if (BRepOffset_Tool::Gabarit(viso) > TolApex)
          {
            Handle(GeomSurface) BasisSurf =
              Handle(Geom_OffsetSurface)::DownCast(TheSurf)->BasisSurface();
            Point3d Papex, Pfirst, Pquart, Pmid;
            Papex                                   = BasisSurf->Value(uf1, vf1);
            Pfirst                                  = TheSurf->Value(uf1, vf1);
            Pquart                                  = TheSurf->Value(0.75 * uf1 + 0.25 * uf2, vf1);
            Pmid                                    = TheSurf->Value(0.5 * (uf1 + uf2), vf1);
            Vector3d                    DirApex       = Vector3d(Pfirst, Pquart) ^ Vector3d(Pfirst, Pmid);
            Handle(GeomLine)         LineApex      = new GeomLine(Papex, DirApex);
            Vector3d                    DirGeneratrix = BasisSurf->DN(uf1, vf1, 0, 1);
            Handle(GeomLine)         LineGeneratrix = new GeomLine(Pfirst, DirGeneratrix);
            GeomAPI_ExtremaCurveCurve theExtrema(LineGeneratrix, LineApex);
            Point3d                    Pint1, Pint2;
            theExtrema.NearestPoints(Pint1, Pint2);
            Standard_Real length = Pfirst.Distance(Pint1);
            if (OffsetOutside)
            {
              Handle(Geom_BoundedSurface) aSurf =
                new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, vf2);
              GeomLib1::ExtendSurfByLength(aSurf, length, 1, Standard_False, Standard_False);
              TheSurf = aSurf;
              Standard_Real u1, u2, v1, v2;
              TheSurf->Bounds(u1, u2, v1, v2);
              MinApex = TheSurf->Value(uf1, v1);
            }
            else
            {
              Handle(GeomCurve3d)          uiso = TheSurf->UIso(uf1);
              GeomAPI_ProjectPointOnCurve Projector(Pint1, uiso);
              Standard_Real               NewFirstV = Projector.LowerDistanceParameter();
              TheSurf = new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, NewFirstV, vf2);
              MinApex = TheSurf->Value(uf1, NewFirstV);
              // TheSurf = new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1+length, vf2);
              // MinApex = TheSurf->Value( uf1, vf1+length );
            }
            HasSingularity = Standard_True;
          }
        } // end of if (VminDegen)
        if (VmaxDegen)
        {
          Handle(GeomCurve3d) viso = TheSurf->VIso(vf2);
          if (BRepOffset_Tool::Gabarit(viso) > TolApex)
          {
            Handle(GeomSurface) BasisSurf =
              Handle(Geom_OffsetSurface)::DownCast(TheSurf)->BasisSurface();
            Point3d Papex, Pfirst, Pquart, Pmid;
            Papex                                   = BasisSurf->Value(uf1, vf2);
            Pfirst                                  = TheSurf->Value(uf1, vf2);
            Pquart                                  = TheSurf->Value(0.75 * uf1 + 0.25 * uf2, vf2);
            Pmid                                    = TheSurf->Value(0.5 * (uf1 + uf2), vf2);
            Vector3d                    DirApex       = Vector3d(Pfirst, Pquart) ^ Vector3d(Pfirst, Pmid);
            Handle(GeomLine)         LineApex      = new GeomLine(Papex, DirApex);
            Vector3d                    DirGeneratrix = BasisSurf->DN(uf1, vf2, 0, 1);
            Handle(GeomLine)         LineGeneratrix = new GeomLine(Pfirst, DirGeneratrix);
            GeomAPI_ExtremaCurveCurve theExtrema(LineGeneratrix, LineApex);
            Point3d                    Pint1, Pint2;
            theExtrema.NearestPoints(Pint1, Pint2);
            Standard_Real length = Pfirst.Distance(Pint1);
            if (OffsetOutside)
            {
              Handle(Geom_BoundedSurface) aSurf =
                new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, vf2);
              GeomLib1::ExtendSurfByLength(aSurf, length, 1, Standard_False, Standard_True);
              TheSurf = aSurf;
              Standard_Real u1, u2, v1, v2;
              TheSurf->Bounds(u1, u2, v1, v2);
              MaxApex = TheSurf->Value(uf1, v2);
            }
            else
            {
              Handle(GeomCurve3d)          uiso = TheSurf->UIso(uf1);
              GeomAPI_ProjectPointOnCurve Projector(Pint1, uiso);
              Standard_Real               NewLastV = Projector.LowerDistanceParameter();
              TheSurf = new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, NewLastV);
              MaxApex = TheSurf->Value(uf1, NewLastV);
              // TheSurf = new Geom_RectangularTrimmedSurface(TheSurf, uf1, uf2, vf1, vf2-length);
              // MaxApex = TheSurf->Value( uf1, vf2-length );
            }
            HasSingularity = Standard_True;
          }
        } // end of if (VmaxDegen)
      } // end of else (case of Geom_OffsetSurface)
    } // end of if (!DegEdges.IsEmpty())
  } // end of processing offsets of faces with possible degenerated edges

  // find the PCurves of the edges of <Faces>

  ShapeBuilder myBuilder;
  myBuilder.MakeFace(myFace);
  if (!IsTransformed)
    myBuilder.UpdateFace(myFace, TheSurf, L, BRepInspector::Tolerance(Face));
  else
    myBuilder.UpdateFace(myFace, TheSurf, TopLoc_Location(), BRepInspector::Tolerance(Face));

  TopTools_DataMapOfShapeShape MapSS;

  // mise a jour de la map sur les vertex deja crees
  TopoShape aLocalShapeOriented = Face.Oriented(TopAbs_FORWARD);
  TopoFace  CurFace             = TopoDS::Face(aLocalShapeOriented);
  //  TopoFace CurFace = TopoDS::Face(Face.Oriented(TopAbs_FORWARD));

  TopTools_MapOfShape VonDegen;
  Standard_Real       u1, u2, v1, v2;
  TheSurf->Bounds(u1, u2, v1, v2);

  ShapeExplorer exp(CurFace, TopAbs_EDGE);
  for (; exp.More(); exp.Next())
  {
    const TopoEdge& E = TopoDS::Edge(exp.Current());
    TopoVertex      V1, V2, OV1, OV2;
    TopExp1::Vertices(E, V1, V2);
    if (HasSingularity && BRepInspector::Degenerated(E))
      VonDegen.Add(V1);
    if (Created.IsBound(E))
    {
      const TopoEdge& OE = TopoDS::Edge(Created(E));
      TopExp1::Vertices(OE, OV1, OV2);
      if (!MapSS.IsBound(V1))
        MapSS.Bind(V1, OV1);
      if (!MapSS.IsBound(V2))
        MapSS.Bind(V2, OV2);
    }
    if (Created.IsBound(V1))
    {
      if (!MapSS.IsBound(V1))
        MapSS.Bind(V1, Created(V1));
    }
    if (Created.IsBound(V2))
    {
      if (!MapSS.IsBound(V2))
        MapSS.Bind(V2, Created(V2));
    }
  }

  ShapeExplorer expw(CurFace, TopAbs_WIRE);
  for (; expw.More(); expw.Next())
  {
    const TopoWire& W = TopoDS::Wire(expw.Current());
    ShapeExplorer    expe(W.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
    TopoWire        OW;
    myBuilder.MakeWire(OW);
    for (; expe.More(); expe.Next())
    {
      const TopoEdge& E = TopoDS::Edge(expe.Current());
      TopoVertex      V1, V2;
      TopExp1::Vertices(E, V1, V2);
      gp_Pnt2d             P2d1, P2d2;
      Point3d               P1, P2;
      Standard_Real        vstart, vend;
      Standard_Real        f, l;
      Handle(GeomCurve2d) C2d = BRepInspector::CurveOnSurface(E, CurFace, f, l);
      TopoEdge          OE;
      if (MapSS.IsBound(E) && !VonDegen.Contains(V1) && !VonDegen.Contains(V2))
      { // c`est un edge de couture
        OE                               = TopoDS::Edge(MapSS(E));
        TopoShape         aLocalShape = E.Reversed();
        Handle(GeomCurve2d) C2d_1 =
          BRepInspector::CurveOnSurface(TopoDS::Edge(aLocalShape), CurFace, f, l);
        //	Handle(GeomCurve2d) C2d_1 =
        //	  BRepInspector::CurveOnSurface(TopoDS::Edge(E.Reversed()),CurFace,f,l);
        if (E.Orientation() == TopAbs_FORWARD)
          UpdateEdge(OE, C2d, C2d_1, myFace, BRepInspector::Tolerance(E));
        else
          UpdateEdge(OE, C2d_1, C2d, myFace, BRepInspector::Tolerance(E));
        myBuilder.Range(OE, f, l);
      }
      else
      {
        TopoShape aLocalShape = E.Oriented(TopAbs_FORWARD);
        TopoEdge  Eforward    = TopoDS::Edge(aLocalShape);
        P2d1                     = C2d->Value(BRepInspector::Parameter(V1, Eforward, CurFace));
        P2d2                     = C2d->Value(BRepInspector::Parameter(V2, Eforward, CurFace));
        if (VonDegen.Contains(V1))
        {
          if (Abs(P2d1.Y() - vf1) <= Precision::Confusion())
          {
            P1     = MinApex;
            vstart = v1;
          }
          else
          {
            P1     = MaxApex;
            vstart = v2;
          }
        }
        else
        {
          TheSurf->D0(P2d1.X(), P2d1.Y(), P1);
          if (!L.IsIdentity() && !IsTransformed)
            P1.Transform(L.Transformation());
          vstart = P2d1.Y();
        }
        if (VonDegen.Contains(V2))
        {
          if (Abs(P2d2.Y() - vf1) <= Precision::Confusion())
          {
            P2   = MinApex;
            vend = v1;
          }
          else
          {
            P2   = MaxApex;
            vend = v2;
          }
        }
        else
        {
          TheSurf->D0(P2d2.X(), P2d2.Y(), P2);
          if (!L.IsIdentity() && !IsTransformed)
            P2.Transform(L.Transformation());
          vend = P2d2.Y();
        }
        // E a-t-il ume image dans la Map des Created ?
        if (Created.IsBound(E))
        {
          OE = TopoDS::Edge(Created(E));
        }
        else if (MapSS.IsBound(E)) // seam edge
          OE = TopoDS::Edge(MapSS(E));
        else
        {
          myBuilder.MakeEdge(OE);
          TopoVertex OV1, OV2;
          if (MapSS.IsBound(V1))
          {
            OV1 = TopoDS::Vertex(MapSS(V1));
          }
          else
          {
            myBuilder.MakeVertex(OV1);
            myBuilder.UpdateVertex(OV1, P1, BRepInspector::Tolerance(V1));
            MapSS.Bind(V1, OV1);
          }
          if (MapSS.IsBound(V2))
          {
            OV2 = TopoDS::Vertex(MapSS(V2));
          }
          else
          {
            myBuilder.MakeVertex(OV2);
            myBuilder.UpdateVertex(OV2, P2, BRepInspector::Tolerance(V2));
            MapSS.Bind(V2, OV2);
          }
          myBuilder.Add(OE, OV1.Oriented(V1.Orientation()));
          myBuilder.Add(OE, OV2.Oriented(V2.Orientation()));
          if (BRepInspector::Degenerated(E))
          {
            myBuilder.Degenerated(OE, Standard_True);
            /*
    #ifdef OCCT_DEBUG
            Point3d        P1,P2;
            gp_Pnt2d      P2d;
            P2d = C2d->Value(f); TheSurf->D0(P2d.X(),P2d.Y(),P1);
            P2d = C2d->Value(l); TheSurf->D0(P2d.X(),P2d.Y(),P2);
            Standard_Real Tol = BRepInspector::Tolerance(V1);
            if (!P1.IsEqual(P2,Tol)) {
              std::cout <<"BRepOffset_Offset : E degenerated -> OE not degenerated"<<std::endl;
            }
    #endif
                */
          }
        }
        if (VonDegen.Contains(V1) || VonDegen.Contains(V2))
        {
          if (VonDegen.Contains(V1))
            P2d1.SetY(vstart);
          if (VonDegen.Contains(V2))
            P2d2.SetY(vend);
          C2d = new Geom2d_Line(P2d1, gp_Vec2d(P2d1, P2d2));
          f   = 0.;
          l   = P2d1.Distance(P2d2);
          if (MapSS.IsBound(E)) // seam edge
          {
            Handle(GeomCurve2d) C2d_1 = BRepInspector::CurveOnSurface(OE, myFace, f, l);
            if (E.Orientation() == TopAbs_FORWARD)
              UpdateEdge(OE, C2d, C2d_1, myFace, BRepInspector::Tolerance(E));
            else
              UpdateEdge(OE, C2d_1, C2d, myFace, BRepInspector::Tolerance(E));
          }
          else
            UpdateEdge(OE, C2d, myFace, BRepInspector::Tolerance(E));
          // myBuilder.Range(OE,f,l);
          myBuilder.Range(OE, myFace, f, l);
          if (!BRepInspector::Degenerated(E) && TheSurf->IsUClosed())
          {
            TopoShape         aLocalShapeReversedE = E.Reversed();
            Handle(GeomCurve2d) C2d_1 =
              BRepInspector::CurveOnSurface(TopoDS::Edge(aLocalShapeReversedE), CurFace, f, l);
            P2d1 = C2d_1->Value(BRepInspector::Parameter(V1, E, CurFace));
            P2d2 = C2d_1->Value(BRepInspector::Parameter(V2, E, CurFace));
            if (VonDegen.Contains(V1))
              P2d1.SetY(vstart);
            if (VonDegen.Contains(V2))
              P2d2.SetY(vend);
            C2d_1 = new Geom2d_Line(P2d1, gp_Vec2d(P2d1, P2d2));
            if (E.Orientation() == TopAbs_FORWARD)
              UpdateEdge(OE, C2d, C2d_1, myFace, BRepInspector::Tolerance(E));
            else
              UpdateEdge(OE, C2d_1, C2d, myFace, BRepInspector::Tolerance(E));
          }
          /*
          if (!BRepInspector::Degenerated(E))
            {
          Handle(GeomLine) theLine = new GeomLine( P1, Vector3d(P1, P2) );
          myBuilder.UpdateEdge( OE, theLine, BRepInspector::Tolerance(E) );
            }
          */
        }
        else
        {
          UpdateEdge(OE, C2d, myFace, BRepInspector::Tolerance(E));
          myBuilder.Range(OE, f, l);
          // ComputeCurve3d(OE,C2d,TheSurf,L,BRepInspector::Tolerance(E));
        }
        if (!BRepInspector::Degenerated(OE))
          ComputeCurve3d(OE, C2d, TheSurf, L, BRepInspector::Tolerance(E));
        MapSS.Bind(E, OE);
      }
      myBuilder.Add(OW, OE.Oriented(E.Orientation()));
    }
    myBuilder.Add(myFace, OW.Oriented(W.Orientation()));
  }

  myFace.Orientation(Face.Orientation());

  BRepTools1::Update(myFace);
}

//=================================================================================================

void BRepOffset_Offset::Init(const TopoEdge&     Path,
                             const TopoEdge&     Edge1,
                             const TopoEdge&     Edge2,
                             const Standard_Real    Offset,
                             const Standard_Boolean Polynomial,
                             const Standard_Real    Tol,
                             const GeomAbs_Shape    Conti)
{
  TopoEdge FirstEdge, LastEdge;
  Init(Path, Edge1, Edge2, Offset, FirstEdge, LastEdge, Polynomial, Tol, Conti);
}

//=================================================================================================

void BRepOffset_Offset::Init(const TopoEdge&     Path,
                             const TopoEdge&     Edge1,
                             const TopoEdge&     Edge2,
                             const Standard_Real    Offset,
                             const TopoEdge&     FirstEdge,
                             const TopoEdge&     LastEdge,
                             const Standard_Boolean Polynomial,
                             const Standard_Real    Tol,
                             const GeomAbs_Shape    Conti)
{
  Standard_Boolean C1Denerated = Standard_False;
  Standard_Boolean C2Denerated = Standard_False;
  myStatus                     = BRepOffset_Good;
  myShape                      = Path;

  TopLoc_Location Loc;
  Standard_Real   f[3], l[3];

  Handle(GeomCurve3d) CP = BRepInspector::Curve(Path, Loc, f[0], l[0]);
  CP                    = new Geom_TrimmedCurve(CP, f[0], l[0]);
  CP->Transform(Loc.Transformation());
  Handle(GeomAdaptor_Curve) HCP = new GeomAdaptor_Curve(CP);

  Handle(GeomCurve3d) C1 = BRepInspector::Curve(Edge1, Loc, f[1], l[1]);

  Handle(Adaptor3d_Curve) HEdge1;
  Standard_Boolean        C1is3D = Standard_True;
  if (C1.IsNull())
  {
    C1is3D = Standard_False;
    Handle(GeomCurve2d) C12d;
    Handle(GeomSurface) S1;
    BRepInspector::CurveOnSurface(Edge1, C12d, S1, Loc, f[1], l[1]);
    S1   = Handle(GeomSurface)::DownCast(S1->Transformed(Loc.Transformation()));
    C12d = new Geom2d_TrimmedCurve(C12d, f[1], l[1]);
    Handle(GeomAdaptor_Surface) HS1 = new GeomAdaptor_Surface(S1);
    Handle(Geom2dAdaptor_Curve) HC1 = new Geom2dAdaptor_Curve(C12d);
    Adaptor3d_CurveOnSurface    Cons(HC1, HS1);
    HEdge1 = new Adaptor3d_CurveOnSurface(Cons);
  }
  else
  {
    C1 = new Geom_TrimmedCurve(C1, f[1], l[1]);
    C1->Transform(Loc.Transformation());
    HEdge1 = new GeomAdaptor_Curve(C1);
    GeomAdaptor_Curve AC1(C1);
    if (AC1.GetType() == GeomAbs_Circle)
    {
      C1Denerated = (AC1.Circle().Radius() < Precision::Confusion());
    }
  }

  Handle(GeomCurve3d) C2 = BRepInspector::Curve(Edge2, Loc, f[2], l[2]);

  Handle(Adaptor3d_Curve) HEdge2;
  Standard_Boolean        C2is3D = Standard_True;
  if (C2.IsNull())
  {
    C2is3D = Standard_False;
    Handle(GeomCurve2d) C12d;
    Handle(GeomSurface) S1;
    BRepInspector::CurveOnSurface(Edge2, C12d, S1, Loc, f[2], l[2]);
    S1   = Handle(GeomSurface)::DownCast(S1->Transformed(Loc.Transformation()));
    C12d = new Geom2d_TrimmedCurve(C12d, f[2], l[2]);
    Handle(GeomAdaptor_Surface) HS1 = new GeomAdaptor_Surface(S1);
    Handle(Geom2dAdaptor_Curve) HC1 = new Geom2dAdaptor_Curve(C12d);
    Adaptor3d_CurveOnSurface    Cons(HC1, HS1);
    HEdge2 = new Adaptor3d_CurveOnSurface(Cons);
  }
  else
  {
    C2 = new Geom_TrimmedCurve(C2, f[2], l[2]);
    C2->Transform(Loc.Transformation());
    HEdge2 = new GeomAdaptor_Curve(C2);
    GeomAdaptor_Curve AC2(C2);
    if (AC2.GetType() == GeomAbs_Circle)
    {
      C2Denerated = (AC2.Circle().Radius() < Precision::Confusion());
    }
  }

  // Calcul du tuyau
  GeomFill_Pipe Pipe(HCP, HEdge1, HEdge2, Abs(Offset));
  Pipe.Perform(Tol, Polynomial, Conti);
  if (!Pipe.IsDone())
    throw Standard_ConstructionError("GeomFill_Pipe : Cannot make a surface");
  Standard_Real ErrorPipe = Pipe.ErrorOnSurf();

  Handle(GeomSurface) S      = Pipe.Surface();
  Standard_Boolean     ExchUV = Pipe.ExchangeUV();
  Standard_Real        f1, l1, f2, l2;
  S->Bounds(f1, l1, f2, l2);

  // Perform the face
  Standard_Real PathTol = BRepInspector::Tolerance(Path);
  Standard_Real TheTol;
  ShapeBuilder  myBuilder;
  myBuilder.MakeFace(myFace);
  TopLoc_Location Id;
  myBuilder.UpdateFace(myFace, S, Id, PathTol);

  // update de Edge1. (Rem : has already a 3d curve)
  Standard_Real        U, U1, U2;
  Handle(GeomCurve2d) PC;
  if (ExchUV)
  {
    PC = new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(1, 0));
    U1 = f1;
    U2 = l1;
    if (!C1is3D)
      C1 = S->VIso(f2);
  }
  else
  {
    PC = new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1));
    U1 = f2;
    U2 = l2;
    if (!C1is3D)
      C1 = S->UIso(f1);
  }

  Handle(GeomCurve3d) Dummy;
  if (!C1is3D)
    UpdateEdge(Edge1, C1, Id, BRepInspector::Tolerance(Edge1));
  else if (C1Denerated)
  {
    UpdateEdge(Edge1, Dummy, Id, BRepInspector::Tolerance(Edge1));
    myBuilder.Degenerated(Edge1, Standard_True);
  }

  TheTol = Max(PathTol, BRepInspector::Tolerance(Edge1) + ErrorPipe);
  UpdateEdge(Edge1, PC, myFace, TheTol);

  // mise a same range de la nouvelle pcurve.
  if (!C1is3D && !C1Denerated)
  {
    myBuilder.SameRange(Edge1, Standard_False);
    myBuilder.Range(Edge1, U1, U2, Standard_True);
  }
  myBuilder.Range(Edge1, myFace, U1, U2);
  BRepLib::SameRange(Edge1);

  // mise a sameparameter pour les KPart
  if (ErrorPipe == 0)
  {
    TheTol = Max(TheTol, Tol);
    myBuilder.SameParameter(Edge1, Standard_False);
    BRepLib::SameParameter(Edge1, TheTol);
  }

  // Update de edge2. (Rem : has already a 3d curve)
  if (ExchUV)
  {
    PC = new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(1, 0));
    U1 = f1;
    U2 = l1;
    if (!C2is3D)
      C2 = S->VIso(l2);
  }
  else
  {
    PC = new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1));
    U1 = f2;
    U2 = l2;
    if (!C2is3D)
      C2 = S->UIso(l1);
  }

  if (!C2is3D)
    UpdateEdge(Edge2, C2, Id, BRepInspector::Tolerance(Edge2));
  else if (C2Denerated)
  {
    UpdateEdge(Edge2, Dummy, Id, BRepInspector::Tolerance(Edge2));
    myBuilder.Degenerated(Edge2, Standard_True);
  }

  TheTol = Max(PathTol, BRepInspector::Tolerance(Edge2) + ErrorPipe);
  UpdateEdge(Edge2, PC, myFace, TheTol);

  // mise a same range de la nouvelle pcurve.
  myBuilder.SameRange(Edge2, Standard_False);
  if (!C2is3D && !C2Denerated)
    myBuilder.Range(Edge2, U1, U2, Standard_True);
  myBuilder.Range(Edge2, myFace, U1, U2);
  BRepLib::SameRange(Edge2);

  // mise a sameparameter pour les KPart
  if (ErrorPipe == 0)
  {
    TheTol = Max(TheTol, Tol);
    myBuilder.SameParameter(Edge2, Standard_False);
    BRepLib::SameParameter(Edge2, TheTol);
  }

  TopoEdge Edge3, Edge4;
  // eval edge3
  TopoVertex V1f, V1l, V2f, V2l;
  TopExp1::Vertices(Path, V1f, V1l);
  Standard_Boolean IsClosed = (V1f.IsSame(V1l));

  TopExp1::Vertices(Edge1, V1f, V1l);
  TopExp1::Vertices(Edge2, V2f, V2l);

  Standard_Boolean StartDegenerated = (V1f.IsSame(V2f));
  Standard_Boolean EndDegenerated   = (V1l.IsSame(V2l));

  Standard_Boolean E3rev = Standard_False;
  Standard_Boolean E4rev = Standard_False;

  TopoVertex VVf, VVl;
  if (FirstEdge.IsNull())
  {
    myBuilder.MakeEdge(Edge3);
    myBuilder.Add(Edge3, V1f.Oriented(TopAbs_FORWARD));
    myBuilder.Add(Edge3, V2f.Oriented(TopAbs_REVERSED));
  }
  else
  {
    TopoShape aLocalEdge = FirstEdge.Oriented(TopAbs_FORWARD);
    Edge3                   = TopoDS::Edge(aLocalEdge);
    //    Edge3 = TopoDS::Edge(FirstEdge.Oriented(TopAbs_FORWARD));
    TopExp1::Vertices(Edge3, VVf, VVl);
#ifdef OCCT_DEBUG
    // si firstedge n est pas nul, il faut que les vertex soient partages
    if (!VVf.IsSame(V1f) && !VVf.IsSame(V2f))
    {
      std::cout << "Attention Vertex non partages !!!!!!" << std::endl;
    }
#endif
    if (!VVf.IsSame(V1f) && !VVf.IsSame(V2f))
    {
      // On fait vraisemblablement des conneries !!
      // On cree un autre edge, on appelle le Sewing apres.
      myBuilder.MakeEdge(Edge3);
      myBuilder.Add(Edge3, V1f.Oriented(TopAbs_FORWARD));
      myBuilder.Add(Edge3, V2f.Oriented(TopAbs_REVERSED));
    }
    else if (!VVf.IsSame(V1f))
    {
      Edge3.Reverse();
      E3rev = Standard_True;
    }
  }

  if (IsClosed)
    Edge4 = Edge3;

  constexpr Standard_Real TolApp = Precision::Approximation();

  Handle(Geom2d_Line) L1, L2;
  if (IsClosed)
  {
    if (ExchUV)
    {
      // rem : si ExchUv, il faut  reverser le Wire.
      // donc l'edge Forward dans la face sera E4 : d'ou L1 et L2
      L2 = new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1));
      L1 = new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1));
      U1 = f2;
      U2 = l2;
    }
    else
    {
      L1 = new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(1, 0));
      L2 = new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(1, 0));
      U1 = f1;
      U2 = l1;
    }
    if (E3rev)
    {
      L1->Reverse();
      L2->Reverse();
      U  = -U1;
      U1 = -U2;
      U2 = U;
    }
    UpdateEdge(Edge3, L1, L2, myFace, PathTol);
    myBuilder.Range(Edge3, myFace, U1, U2);
    if (StartDegenerated)
      myBuilder.Degenerated(Edge3, Standard_True);
    else if (FirstEdge.IsNull()) // then the 3d curve has not been yet computed
      ComputeCurve3d(Edge3, L1, S, Id, TolApp);
  }
  else
  {
    if (LastEdge.IsNull())
    {
      myBuilder.MakeEdge(Edge4);
      myBuilder.Add(Edge4, V1l.Oriented(TopAbs_FORWARD));
      myBuilder.Add(Edge4, V2l.Oriented(TopAbs_REVERSED));
    }
    else
    {
      TopoShape aLocalEdge = LastEdge.Oriented(TopAbs_FORWARD);
      Edge4                   = TopoDS::Edge(aLocalEdge);
      //      Edge4 = TopoDS::Edge(LastEdge.Oriented(TopAbs_FORWARD));
      TopExp1::Vertices(Edge4, VVf, VVl);
#ifdef OCCT_DEBUG
      // si lastedge n est pas nul, il faut que les vertex soient partages
      if (!VVf.IsSame(V1l) && !VVf.IsSame(V2l))
      {
        std::cout << "Attention Vertex non partages !!!!!!" << std::endl;
      }
#endif
      if (!VVf.IsSame(V1l) && !VVf.IsSame(V2l))
      {
        // On fait vraisemblablement des conneries !!
        // On cree un autre edge, on appelle le Sewing apres.
        myBuilder.MakeEdge(Edge4);
        myBuilder.Add(Edge4, V1l.Oriented(TopAbs_FORWARD));
        myBuilder.Add(Edge4, V2l.Oriented(TopAbs_REVERSED));
      }
      else if (!VVf.IsSame(V1l))
      {
        Edge4.Reverse();
        E4rev = Standard_True;
      }
    }

    if (ExchUV)
    {
      L1 = new Geom2d_Line(gp_Pnt2d(f1, 0), gp_Dir2d(0, 1));
      U1 = f2;
      U2 = l2;
    }
    else
    {
      L1 = new Geom2d_Line(gp_Pnt2d(0, f2), gp_Dir2d(1, 0));
      U1 = f1;
      U2 = l1;
    }
    if (E3rev)
    {
      L1->Reverse();
      U  = -U1;
      U1 = -U2;
      U2 = U;
    }
    UpdateEdge(Edge3, L1, myFace, PathTol);
    myBuilder.Range(Edge3, myFace, U1, U2);
    if (StartDegenerated)
      myBuilder.Degenerated(Edge3, Standard_True);
    else if (FirstEdge.IsNull()) // then the 3d curve has not been yet computed
      ComputeCurve3d(Edge3, L1, S, Id, TolApp);

    if (ExchUV)
    {
      L2 = new Geom2d_Line(gp_Pnt2d(l1, 0), gp_Dir2d(0, 1));
      U1 = f2;
      U2 = l2;
    }
    else
    {
      L2 = new Geom2d_Line(gp_Pnt2d(0, l2), gp_Dir2d(1, 0));
      U1 = f1;
      U2 = l1;
    }
    if (E4rev)
    {
      L2->Reverse();
      U  = -U1;
      U1 = -U2;
      U2 = U;
    }
    UpdateEdge(Edge4, L2, myFace, PathTol);
    myBuilder.Range(Edge4, myFace, U1, U2);
    if (EndDegenerated)
      myBuilder.Degenerated(Edge4, Standard_True);
    else if (LastEdge.IsNull()) // then the 3d curve has not been yet computed
      ComputeCurve3d(Edge4, L2, S, Id, TolApp);
  }

  // SameParameter ??
  if (!FirstEdge.IsNull() && !StartDegenerated)
  {
    BRepLib::BuildCurve3d(Edge3, PathTol);
    myBuilder.SameRange(Edge3, Standard_False);
    myBuilder.SameParameter(Edge3, Standard_False);
    BRepLib::SameParameter(Edge3, Tol);
  }
  if (!LastEdge.IsNull() && !EndDegenerated)
  {
    BRepLib::BuildCurve3d(Edge4, PathTol);
    myBuilder.SameRange(Edge4, Standard_False);
    myBuilder.SameParameter(Edge4, Standard_False);
    BRepLib::SameParameter(Edge4, Tol);
  }

  TopoWire W;
  myBuilder.MakeWire(W);

  myBuilder.Add(W, Edge1.Oriented(TopAbs_REVERSED));
  myBuilder.Add(W, Edge2.Oriented(TopAbs_FORWARD));
  myBuilder.Add(W, Edge4.Reversed());
  myBuilder.Add(W, Edge3);

  if (ExchUV)
  {
    W.Reverse();
  }

  myBuilder.Add(myFace, W);
  if (ExchUV)
    myFace.Reverse();

  BRepTools1::Update(myFace);

  if (Edge1.Orientation() == TopAbs_REVERSED)
    myFace.Reverse();
}

//=================================================================================================

void BRepOffset_Offset::Init(const TopoVertex&        Vertex,
                             const ShapeList& LEdge,
                             const Standard_Real         Offset,
                             const Standard_Boolean      Polynomial,
                             const Standard_Real         TolApp,
                             const GeomAbs_Shape         Conti)
{
  myStatus = BRepOffset_Good;
  myShape  = Vertex;

  // evaluate the Ax3 of the Sphere
  // find 3 different vertices in LEdge
  TopTools_ListIteratorOfListOfShape it;
  Point3d                             P, P1, P2, P3;
  TopoVertex                      V1, V2, V3, V4;

#ifdef OCCT_DEBUG
  char* name = new char[100];
  if (Affich)
  {
    NbOFFSET++;

    sprintf(name, "VOnSph_%d", NbOFFSET);
  #ifdef DRAW
    DBRep1::Set(name, Vertex);
  #endif
    Standard_Integer NbEdges = 1;
    for (it.Initialize(LEdge); it.More(); it.Next())
    {
      sprintf(name, "EOnSph_%d_%d", NbOFFSET, NbEdges++);
  #ifdef DRAW
      const TopoShape& CurE = it.Value();
      DBRep1::Set(name, CurE);
  #endif
    }
  }
#endif

  Point3d Origin = BRepInspector::Pnt(Vertex);

  //// Find the axis of the sphere to exclude
  //// degenerated and seam edges from the face under construction
  BRepLib_MakeWire MW;
  MW.Add(LEdge);
  TopoWire theWire = MW.Wire();

  ShapeFix_Shape Fixer(theWire);
  Fixer.Perform();
  theWire = TopoDS::Wire(Fixer.Shape());

  GeometricProperties GlobalProps;
  BRepGProp::LinearProperties(theWire, GlobalProps);
  Point3d BaryCenter = GlobalProps.CentreOfMass();
  Vector3d Xdir(BaryCenter, Origin);

  Point3d FarestCorner = GetFarestCorner(theWire);
  gp_Pln thePlane     = gce_MakePln(Origin, BaryCenter, FarestCorner);
  Dir3d Vdir         = thePlane.Axis().Direction();

  Ax3 Axis(Origin, Vdir, Xdir);

  Handle(GeomSurface) S = new Geom_SphericalSurface(Axis, Abs(Offset));

  Standard_Real f, l, Tol = BRepInspector::Tolerance(Vertex);

  TopLoc_Location Loc;
  ShapeBuilder    myBuilder;
  myBuilder.MakeFace(myFace);
  Handle(GeomSurface) SS = S;

  // En polynomial, calcul de la surface par F(u,v).
  // Pas de changement de parametre, donc ProjLib1 sur la Sphere
  // reste OK.
  if (Polynomial)
  {
    GeomConvert_ApproxSurface Approx(S, TolApp, Conti, Conti, 10, 10, 10, 1);
    if (Approx.IsDone())
    {
      SS = Approx.Surface();
    }
  }

  myBuilder.UpdateFace(myFace, SS, Loc, Tol);

  TopoWire W;
  myBuilder.MakeWire(W);

#ifdef DRAW
  // POP pour NT
  //  char name[100];
  if (Affich)
  {
    sprintf(name, "SPHERE_%d", NbOFFSET);
    DrawTrSurf1::Set(name, S);
  }
  Standard_Integer CO = 1;
#endif

  for (it.Initialize(LEdge); it.More(); it.Next())
  {
    TopoEdge E = TopoDS::Edge(it.Value());

    Handle(GeomCurve3d) C = BRepInspector::Curve(E, Loc, f, l);
    if (C.IsNull())
    {
      BRepLib::BuildCurve3d(E, BRepInspector::Tolerance(E));
      C = BRepInspector::Curve(E, Loc, f, l);
    }
    C = new Geom_TrimmedCurve(C, f, l);
    C->Transform(Loc.Transformation());

#ifdef DRAW
    if (Affich)
    {
      sprintf(name, "CURVE_%d_%d", NbOFFSET, CO);
      DrawTrSurf1::Set(name, C);
      CO++;
    }
#endif

    Handle(GeomCurve2d) PCurve = GeomProjLib1::Curve2d(C, S);
    // check if the first point of PCurve in is the canonical boundaries
    // of the sphere. Else move it.
    // the transformation is : U` = U + PI + 2 k  PI
    //                         V` = +/- PI + 2 k` PI
    gp_Pnt2d         P2d        = PCurve->Value(f);
    Standard_Boolean IsToAdjust = Standard_False;
    if (P2d.Y() < -M_PI / 2.)
    {
      IsToAdjust = Standard_True;
      PCurve->Mirror(gp_Ax2d(gp_Pnt2d(0., -M_PI / 2.), gp1::DX2d()));
    }
    else if (P2d.Y() > M_PI / 2.)
    {
      IsToAdjust = Standard_True;
      PCurve->Mirror(gp_Ax2d(gp_Pnt2d(0., M_PI / 2.), gp1::DX2d()));
    }
    if (IsToAdjust)
    {
      // set the u firstpoint in [0,2*pi]
      gp_Vec2d Tr(M_PI, 0.);
      if (P2d.X() > M_PI)
        Tr.Reverse();
      PCurve->Translate(Tr);
    }

    UpdateEdge(E, PCurve, myFace, Tol);
    myBuilder.Range(E, myFace, f, l);
    myBuilder.Add(W, E);
  }
  if (Offset < 0.)
  {
    myBuilder.Add(myFace, W.Oriented(TopAbs_REVERSED));
    myFace.Reverse();
  }
  else
  {
    myBuilder.Add(myFace, W);
  }

  BRepTools1::Update(myFace);
}

//=================================================================================================

void BRepOffset_Offset::Init(const TopoEdge& Edge, const Standard_Real Offset)
{
  myShape                = Edge;
  Standard_Real myOffset = Abs(Offset);

  Standard_Real   f, l;
  TopLoc_Location Loc;

  Handle(GeomCurve3d) CP = BRepInspector::Curve(Edge, Loc, f, l);
  CP                    = new Geom_TrimmedCurve(CP, f, l);
  CP->Transform(Loc.Transformation());

  GeomFill_Pipe Pipe(CP, myOffset);
  Pipe.Perform();
  if (!Pipe.IsDone())
    throw Standard_ConstructionError("GeomFill_Pipe : Cannot make a surface");

  BRepLib_MakeFace MF(Pipe.Surface(), Precision::Confusion());
  myFace = MF.Face();

  if (Offset < 0.)
    myFace.Reverse();
}

//=================================================================================================

const TopoFace& BRepOffset_Offset::Face() const
{
  return myFace;
}

//=================================================================================================

TopoShape BRepOffset_Offset::Generated(const TopoShape& Shape) const
{
  TopoShape aShape;

  switch (myShape.ShapeType())
  {
    case TopAbs_FACE: {
      ShapeExplorer exp(myShape.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
      ShapeExplorer expo(myFace.Oriented(TopAbs_FORWARD), TopAbs_EDGE);
      for (; exp.More() && expo.More(); exp.Next(), expo.Next())
      {
        if (Shape.IsSame(exp.Current()))
        {
          if (myShape.Orientation() == TopAbs_REVERSED)
            aShape = expo.Current().Reversed();
          else
            aShape = expo.Current();
          break;
        }
      }
    }
    break;

    case TopAbs_EDGE:
      // have generate a pipe.
      {
        TopoVertex V1, V2;
        TopExp1::Vertices(TopoDS::Edge(myShape), V1, V2);

        ShapeExplorer expf(myFace.Oriented(TopAbs_FORWARD), TopAbs_WIRE);
        ShapeExplorer expo(expf.Current().Oriented(TopAbs_FORWARD), TopAbs_EDGE);
        expo.Next();
        expo.Next();

        if (V2.IsSame(Shape))
        {
          if (expf.Current().Orientation() == TopAbs_REVERSED)
            aShape = expo.Current().Reversed();
          else
            aShape = expo.Current();
        }
        else
        {
          expo.Next();
          if (expf.Current().Orientation() == TopAbs_REVERSED)
            aShape = expo.Current().Reversed();
          else
            aShape = expo.Current();
        }
        if (myFace.Orientation() == TopAbs_REVERSED)
          aShape.Reverse();
      }
      break;
    default:
      break;
  }

  return aShape;
}

//=================================================================================================

BRepOffset_Status BRepOffset_Offset::Status() const
{
  return myStatus;
}
