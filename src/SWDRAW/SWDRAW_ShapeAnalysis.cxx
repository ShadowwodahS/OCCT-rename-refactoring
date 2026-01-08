// Created on: 1999-03-09
// Created by: data exchange team
// Copyright (c) 1999-1999 Matra Datavision
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

// sln 19.11.2001. Bug 2: Correction of output of 'statshape' draw function.

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepGProp.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <DBRep.hxx>
#include <Draw.hxx>
#include <DrawTrSurf.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <GProp_GProps.hxx>
#include <Precision.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <ShapeAnalysis_FreeBoundsProperties.hxx>
#include <ShapeAnalysis_ShapeContents.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_FreeBounds.hxx>
#include <SWDRAW.hxx>
#include <SWDRAW_ShapeAnalysis.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <BRepAdaptor_Curve2d.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <ShapeAnalysis_CanonicalRecognition.hxx>

#include <stdio.h>

static Standard_Integer tolerance(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << "myshape  : analyses a shape\n"
       << "myshape [mode] valmin : sub-shapes over valmin\n"
       << "myshape [mode] valmin valmax : between valmin and valmax\n"
       << "myshape [mode] 0 valmax : below valmax\n"
       << "  more : all shapes(D)  v vertices  e edges  f faces  c combined(faces)\n";
    return (argc < 2 ? 0 : 1 /* Error */);
  }
  Standard_CString arg1  = argv[1];
  TopoShape     Shape = DBRep1::Get(arg1);
  if (Shape.IsNull())
  {
    di << "Shape unknown : " << arg1 << "\n";
    return 1 /* Error */;
  }

  ShapeTolerance sat;
  if (argc == 2)
  {
    sat.InitTolerance();
    sat.AddTolerance(Shape);
    di << "Tolerance MAX=" << sat.GlobalTolerance(1) << " AVG=" << sat.GlobalTolerance(0)
       << " MIN=" << sat.GlobalTolerance(-1) << "\n";
    sat.InitTolerance();
    sat.AddTolerance(Shape, TopAbs_FACE);
    di << "FACE    : MAX=" << sat.GlobalTolerance(1) << " AVG=" << sat.GlobalTolerance(0)
       << " MIN=" << sat.GlobalTolerance(-1) << "\n";
    sat.InitTolerance();
    sat.AddTolerance(Shape, TopAbs_EDGE);
    di << "EDGE    : MAX=" << sat.GlobalTolerance(1) << " AVG=" << sat.GlobalTolerance(0)
       << " MIN=" << sat.GlobalTolerance(-1) << "\n";
    sat.InitTolerance();
    sat.AddTolerance(Shape, TopAbs_VERTEX);
    di << "VERTEX  : MAX=" << sat.GlobalTolerance(1) << " AVG=" << sat.GlobalTolerance(0)
       << " MIN=" << sat.GlobalTolerance(-1) << "\n";
  }
  else
  {
    //  Analyse plus fouillee
    Standard_Integer nextarg = 2;
    char             opt     = argv[2][0];
    TopAbs_ShapeEnum type    = TopAbs_SHAPE;
    if (opt == 'a')
      nextarg = 3;
    else if (opt == 'c')
    {
      nextarg = 3;
      type    = TopAbs_SHELL;
    }
    else if (opt == 'e')
    {
      nextarg = 3;
      type    = TopAbs_EDGE;
    }
    else if (opt == 'f')
    {
      nextarg = 3;
      type    = TopAbs_FACE;
    }
    else if (opt == 'v')
    {
      nextarg = 3;
      type    = TopAbs_VERTEX;
    }
    if (nextarg < 3)
      opt = 'a';

    Standard_Real tol1 = 0., tol2 = 0.;
    if (nextarg < argc)
      tol1 = Draw1::Atof(argv[nextarg]);
    if (nextarg < argc - 1)
      tol2 = Draw1::Atof(argv[nextarg + 1]);
    //    Standard_Real tol = Draw1::Atof (argv[2]);
    Handle(HSequenceOfShape) list = sat.InTolerance(Shape, tol1, tol2, type);
    Standard_Integer                  i, nb = list->Length();
    switch (type)
    {
      case TopAbs_VERTEX:
        di << "Analysing Vertices gives ";
        break;
      case TopAbs_EDGE:
        di << "Analysing Edges gives ";
        break;
      case TopAbs_FACE:
        di << "Analysing Faces gives ";
        break;
      case TopAbs_SHELL:
        di << "Analysing Shells,Faces+content gives ";
        break;
      default:
        di << "Analysing all sub-shapes gives ";
    }
    if (tol1 == 0)
      di << nb << " Shapes below tol=" << tol2;
    else if (tol2 == 0)
      di << nb << " Shapes over tol=" << tol1;
    else
      di << nb << " Shapes between tol1=" << tol1 << " and tol2=" << tol2;
    if (nb == 1)
      di << " , named tol_1";
    if (nb > 1)
      di << " , named tol_1 to tol_" << nb;
    di << "\n";
    char nomsh[30];
    for (i = 1; i <= nb; i++)
    {
      Sprintf(nomsh, "tol_%d", i);
      DBRep1::Set(nomsh, list->Value(i));
    }
  }
  return 0;
}

static Standard_Integer projface(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 4)
  {
    di << "Give FACE name and X Y [Z]\n";
    return 1;
  }
  Standard_CString arg1  = argv[1];
  TopoShape     Shape = DBRep1::Get(arg1);
  if (Shape.IsNull())
  {
    di << "Shape unknown : " << arg1 << "\n";
    return 1 /* Error */;
  }
  if (Shape.ShapeType() != TopAbs_FACE)
  {
    di << "Not a face\n";
    return 1 /* Error */;
  }
  TopoFace             F       = TopoDS::Face(Shape);
  Handle(GeomSurface)    thesurf = BRepInspector::Surface(F); // pas locface
  BRepTopAdaptor_FClass2d aClassifier(F, Precision1::Confusion());
  //  On y va
  Standard_Real X, Y, Z, U, V;
  X = U = Draw1::Atof(argv[2]);
  Y = V = Draw1::Atof(argv[3]);
  if (argc > 4)
  {
    Z = Draw1::Atof(argv[4]);
    Point3d P3D(X, Y, Z);
    di << " Point 3D X = " << X << "  Y = " << Y << "  Z = " << Z << "\n";
    Standard_Real uf, ul, vf, vl;
    thesurf->Bounds(uf, ul, vf, vl);

    if (Precision1::IsInfinite(uf))
      uf = -1000;
    if (Precision1::IsInfinite(ul))
      ul = 1000;
    if (Precision1::IsInfinite(vf))
      vf = -1000;
    if (Precision1::IsInfinite(vl))
      vl = 1000;
    Standard_Real du = Abs(ul - uf) / 10;
    Standard_Real dv = Abs(vl - vf) / 10;

    PointOnSurfProjector proj(P3D, thesurf, uf - du, ul + du, vf - dv, vl + dv);
    Standard_Integer           sol, nPSurf = proj.NbPoints();
    Standard_Integer           anIndSol = 0, anIndMin = 0;
    Standard_Real              aMinDist = RealLast();
    for (sol = 1; sol <= nPSurf; sol++)
    {

      proj.Parameters(sol, U, V);
      TopAbs_State aStatus = aClassifier.Perform(gp_Pnt2d(U, V));
      if (aStatus == TopAbs_OUT)
        continue;

      anIndSol++;
      Standard_Real aDist = proj.Distance(sol);
      di << "n0 " << anIndSol << " Distance " << aDist;
      di << "  U = " << U << "  V = " << V << "\n";

      if (aDist < aMinDist)
      {
        aMinDist = aDist;
        anIndMin = sol;
      }

      //  reprojection
      P3D = thesurf->Value(U, V);
      di << "  => reproj  X = " << P3D.X() << "  Y = " << P3D.Y() << "  Z = " << P3D.Z() << "\n";
    }
    di << " Found " << anIndSol << " Points\n";

    if (anIndMin != 0) // there is at least one suitable solution
    {
      di << "** Minimal distance to face = " << aMinDist << "\n";
      proj.Parameters(anIndMin, U, V);
      di << "**  Solution of minimal distance:  U = " << U << "  V = " << V << "\n";
      P3D = thesurf->Value(U, V);
      di << "  => reproj  X = " << P3D.X() << "  Y = " << P3D.Y() << "  Z = " << P3D.Z() << "\n";
    }
  }
  else // Check 2D point
  {
    di << " Point UV  U = " << U << "  V = " << V << "\n";
    TopAbs_State aStatus = aClassifier.Perform(gp_Pnt2d(U, V));
    if (aStatus == TopAbs_OUT)
      di << "does not belong to the face" << "\n";
    else
    {
      Point3d P3D = thesurf->Value(U, V);
      di << " => proj  X = " << P3D.X() << "  Y = " << P3D.Y() << "  Z = " << P3D.Z() << "\n";
    }
  }
  return 0;
}

static Standard_Integer projcurve(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  //  admet une EDGE ou une CURVE
  if (argc < 5)
  {
    di << "Give : EDGE X Y Z  or  CURVE3D X Y Z  or  CURVE3D first last X Y Z\n";
    return 1 /* Error */;
  }
  const char*      arg1  = argv[1];
  Standard_Integer i0    = 0;
  TopoShape     Shape = DBRep1::Get(arg1);

  Handle(GeomCurve3d) C;
  Standard_Real      cf, cl;
  if (!Shape.IsNull())
  {
    if (Shape.ShapeType() == TopAbs_EDGE)
    {
      TopoEdge E = TopoDS::Edge(Shape);
      C             = BRepInspector::Curve(E, cf, cl);
      di << "Edge " << arg1 << " Params from " << cf << " to " << cl << "\n";
    }
    else
    {
      di << "SHAPE " << arg1 << " Not an EDGE\n";
      return 1 /* Error */;
    }
  }
  else
  {
    const char* a1 = arg1;
    C              = DrawTrSurf1::GetCurve(a1);
    if (C.IsNull())
    {
      di << arg1 << " neither EDGE nor CURVE 3D\n";
      return 1 /* Error */;
    }
    cf = C->FirstParameter();
    cl = C->LastParameter();
    if (argc >= 7)
    {
      cf = Draw1::Atof(argv[2]);
      cl = Draw1::Atof(argv[3]);
      i0 = 2;
    }
    di << "Curve 3D " << arg1 << " Params from " << cf << " to " << cl << "\n";
  }

  Standard_Real X, Y, Z;
  X = Draw1::Atof(argv[2 + i0]);
  Y = Draw1::Atof(argv[3 + i0]);
  Z = Draw1::Atof(argv[4 + i0]);
  di << "Precision1 (BRepBuilderAPI1) : " << BRepBuilderAPI1::Precision1() << "  Projection : " << X
     << "  " << Y << "  " << Z << "\n";

  Point3d        P3D(X, Y, Z);
  Point3d        res;
  Standard_Real param, dist;

  dist = Curve2().Project(C, P3D, BRepBuilderAPI1::Precision1(), res, param, cf, cl);
  res.Coord(X, Y, Z);
  di << "Result : " << X << "  " << Y << "  " << Z << "\nParam = " << param << "  Gap = " << dist
     << "\n";
  return 0;
}

static Standard_Integer projpcurve(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  //  admet une EDGE ou une CURVE
  if (argc < 7)
  {
    di << "Give : projpcurve edge face tol X Y Z [start_param]\n";
    return 1;
  }

  TopoEdge aEdge = TopoDS::Edge(DBRep1::Get(argv[1]));
  if (aEdge.IsNull())
  {
    di << "SHAPE " << argv[1] << " is not an EDGE\n";
    return 1 /* Error */;
  }
  TopoFace aFace = TopoDS::Face(DBRep1::Get(argv[2]));
  if (aFace.IsNull())
  {
    di << "SHAPE " << argv[2] << " is not a FACE\n";
    return 1 /* Error */;
  }

  Standard_Real aTol = Draw1::Atof(argv[3]);

  Standard_Real X = Draw1::Atof(argv[4]);
  Standard_Real Y = Draw1::Atof(argv[5]);
  Standard_Real Z = Draw1::Atof(argv[6]);

  Point3d aP3D(X, Y, Z);

  Standard_Boolean IsStartPoint = Standard_False;
  Standard_Real    startpar     = 0.;
  if (argc > 7)
  {
    startpar     = Draw1::Atof(argv[7]);
    IsStartPoint = Standard_True;
  }

  Adaptor3d_CurveOnSurface aCOnS =
    Adaptor3d_CurveOnSurface(new BRepAdaptor_Curve2d(BRepAdaptor_Curve2d(aEdge, aFace)),
                             new BRepAdaptor_Surface(BRepAdaptor_Surface(aFace, Standard_False)));

  Point3d              aPnt;
  Standard_Real       aParam;
  Curve2 aTool;
  Standard_Real       aDist = RealLast();
  if (IsStartPoint)
  {
    aDist = aTool.NextProject(startpar, aCOnS, aP3D, aTol, aPnt, aParam);
  }
  else
  {
    aDist = aTool.Project(aCOnS, aP3D, aTol, aPnt, aParam, Standard_False);
  }

  di << "Point:" << "\n" << aPnt.X() << " " << aPnt.Y() << " " << aPnt.Z() << "\n";
  di << "Param: " << aParam << "\n";
  di << "Dist: " << aDist << "\n";
  return 0;
}

static Standard_Integer anaface(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << "donner un nom de face\n";
    return 1 /* Error */;
  }
  Standard_CString arg1  = argv[1];
  TopoShape     Shape = DBRep1::Get(arg1);
  if (Shape.IsNull())
  {
    di << arg1 << " inconnu\n";
    return 1 /* Error */;
  }
  //  if (Shape.ShapeType() != TopAbs_FACE) { di<<"Pas une FACE\n"; return 1 /* Error */; }

  //  On regarde les wires, sont-ils bien fermes
  Standard_Integer     nbw = 0, nbe = 0;
  Handle(GeomSurface) surface;
  TopoFace          Face;
  if (Shape.ShapeType() == TopAbs_FACE)
  {
    Face    = TopoDS::Face(Shape);
    surface = BRepInspector::Surface(Face); // pas locface
                                        //    TopLoc_Location locface;
    di << "Face, surface type = " << surface->DynamicType()->Name()
       << " Tol=" << BRepInspector::Tolerance(Face);
    if (Face.Orientation() == TopAbs_REVERSED)
    {
      di << " (REV)";
      //      Face.Orientation(TopAbs_FORWARD);
    }
    di << "\n";
  }
  else
  {
    di << "Analyse Wires, Ignore Faces\n";
  }
  Standard_Boolean iasurf = !surface.IsNull();
  //: sw  if (!Face.IsNull()) STW.SetFace (Face);
  //: sw  else STW.SetPrecision (BRepBuilderAPI1::Precision1());

  for (ShapeExplorer exp(Shape, TopAbs_WIRE); exp.More(); exp.Next())
  {
    TopoWire Wire = TopoDS::Wire(exp.Current());
    nbw++;
    nbe = 0;
    di << "WIRE " << nbw;
    if (Wire.Orientation() == TopAbs_FORWARD)
      di << " (FWD)\n";
    else
      di << " (REV)\n";
    Point3d           fin, debut;
    gp_Pnt2d         finuv, debuv;
    Coords2d            baseuv;
    TopoVertex    fv, lv;
    Standard_Real    maxp3d = 0., maxuv = 0., maxvtx = 0.;
    Standard_Real    df3d = 0., dl3d = 0., dvtx = 0., duv = 0.;
    Standard_Boolean iaw2d = iasurf;
    Standard_Real    u1 = 0., u2 = 0., v1 = 0., v2 = 0., umin = 0., umax = 0., vmin = 0., vmax = 0.,
                  totcross = 0.;

    //: sw    STW.ClearWire();  STW.Load (Wire);
    //    if (STW.Reorder()) di<<"Wire reordered to explore\n";
    //    for (ShapeExplorer exe(Wire.Oriented(TopAbs_FORWARD),TopAbs_EDGE); exe.More();
    //    exe.Next()) {
    //      TopoEdge Edge = TopoDS::Edge (exe.Current());  nbe ++;
    //: sw    for (nbe = 1; nbe <= STW.NbEdges(); nbe ++) {
    //: sw      TopoEdge Edge = STW.Edge(nbe);
    TopoDS_Iterator it(Wire); //: sw: ShapeTool_Wire -> TopoDS_Iterator (it is the same)
    for (; it.More(); it.Next())
    {
      TopoEdge Edge = TopoDS::Edge(it.Value());

      di << "Wire " << nbw << ", Edge " << nbe;
      if (Edge.Orientation() == TopAbs_FORWARD)
        di << " (FWD";
      else
        di << " (REV";
      di << " , Tol= " << BRepInspector::Tolerance(Edge) << " )\n";
      Standard_Real        f3d, l3d, f2d, l2d;
      Handle(GeomCurve3d)   curve3d = BRepInspector::Curve(Edge, f3d, l3d);
      Handle(GeomCurve2d) curve2d;
      if (iasurf)
        curve2d = BRepInspector::CurveOnSurface(Edge, Face, f2d, l2d);
      Standard_Boolean ia2d = !curve2d.IsNull();
      iaw2d |= ia2d;
      if (!ia2d)
        di << "--  No PCurve\n";
      if (curve3d.IsNull())
        di << "-- no Curve 3d\n";

      //      On va tacher de calculer les positions et les comparer
      gp_Pnt2d fuv, luv;
      if (Edge.Orientation() == TopAbs_FORWARD)
      {
        TopExp1::Vertices(Edge, fv, lv);
        if (ia2d)
          BRepInspector::UVPoints(Edge, Face, fuv, luv);
      }
      else
      {
        TopExp1::Vertices(Edge, lv, fv);
        if (ia2d)
          BRepInspector::UVPoints(Edge, Face, luv, fuv);
      }
      Point3d fp = BRepInspector::Pnt(fv);
      Point3d lp = BRepInspector::Pnt(lv);
      Point3d fxyz, lxyz;
      if (ia2d)
      {
        surface->D0(fuv.X(), fuv.Y(), fxyz);
        surface->D0(luv.X(), luv.Y(), lxyz);
        df3d   = fp.Distance(fxyz);
        maxp3d = Max(maxp3d, df3d);
        dl3d   = lp.Distance(lxyz);
        maxp3d = Max(maxp3d, dl3d);
        if (nbe > 1)
        {
          duv   = finuv.Distance(fuv);
          maxuv = Max(maxuv, duv);
        }
        // et les min-max
        u1 = Min(fuv.X(), luv.X());
        u2 = Max(fuv.X(), luv.X());
        v1 = Min(fuv.Y(), luv.Y());
        v2 = Max(fuv.Y(), luv.Y());
        if (nbe == 1)
        {
          umin = u1;
          umax = u2;
          vmin = v1;
          vmax = v2;
        }
        else
        {
          umin = Min(umin, u1);
          umax = Max(umax, u2);
          vmin = Min(vmin, v1);
          vmax = Max(vmax, v2);
        }
        // et la classification directe
        if (nbe == 1)
        {
          baseuv   = fuv.XY();
          totcross = 0;
        }
        else
        {
          Coords2d buv1 = fuv.XY() - baseuv;
          Coords2d buv2 = luv.XY() - baseuv;
          totcross += buv2.Crossed(buv1);
        }
      }

      //  Resultats ...
      if (nbe == 1)
      {
        debut = fxyz;
        debuv = fuv;
      }
      else
      {
        duv    = finuv.Distance(fuv);
        maxuv  = Max(maxuv, duv);
        dvtx   = fin.Distance(fxyz);
        maxvtx = Max(maxvtx, dvtx);
        di << "   Fin(" << nbe - 1 << ")-Debut(" << nbe << "): DISTANCE=" << dvtx;
        if (ia2d)
          di << " DeltaUV=" << duv;
        di << " Tol(Fin)=" << BRepInspector::Tolerance(lv) << "\n";
      }
      fin   = lxyz;
      finuv = luv;

      di << "-- Deb : VTX=" << fp.X() << " , " << fp.Y() << " , " << fp.Z()
         << "\n         XYZ=" << fxyz.X() << " , " << fxyz.Y() << " , " << fxyz.Z()
         << "\n         UV=" << fuv.X() << " , " << fuv.Y() << "  -- D.UV/3D=" << df3d << "\n";
      di << "-- Fin : VTX=" << lp.X() << " , " << lp.Y() << " , " << lp.Z()
         << "\n         XYZ=" << lxyz.X() << " , " << lxyz.Y() << " , " << lxyz.Z()
         << "\n         UV=" << luv.X() << " , " << luv.Y() << "  -- D.UV/3D=" << dl3d << "\n";
    }
    dvtx   = fin.Distance(debut);
    maxvtx = Max(maxvtx, dvtx);
    if (iaw2d)
    {
      duv   = finuv.Distance(debuv);
      maxuv = Max(maxuv, duv);
    }
    di << "   Fin(" << nbe << ")-Debut(1): DISTANCE=" << dvtx;
    if (iaw2d)
      di << " DeltaUV=" << duv;
    di << " Tol(Fin)=" << BRepInspector::Tolerance(lv) << "\n";

    di << "   Wire " << nbw << " Max :  Dist.Vertex=" << maxvtx;
    if (iaw2d)
      di << "  Ecart UV/3D=" << maxp3d << "  DeltaUV=" << maxuv;
    di << "\n";
    //  Min Max
    if (iaw2d)
    {
      di << "TotCross=" << totcross;
      di << "  UMin-Max:" << umin << " , " << umax << "  VMin-Max:" << vmin << " , " << vmax
         << "\n";
      Standard_Real difu = umax - umin, difv = vmax - vmin;
      GeometricProperties  G;
      BRepGProp1::SurfaceProperties(Face, G);
      if (G.Mass() > 0)
        di << "GProps:Mass Out\n";
      else
        di << "GProps:Mass In\n";
      ///  return (G.Mass() > 0);
      BRepTopAdaptor_FClass2d fcl(Face, BRepInspector::Tolerance(Face));
      if (fcl.PerformInfinitePoint() == TopAbs_OUT)
        di << "Classifier Infinite : Out\n";
      else
        di << "Classifier Infinite : In\n";
      gp_Pnt2d pcl;
      pcl.SetCoord(umin - difu, vmin - difv);
      if (fcl.Perform(pcl) == TopAbs_OUT)
        di << "Classifier UMin-VMin : Out\n";
      pcl.SetCoord(umin - difu, vmax + difv);
      if (fcl.Perform(pcl) == TopAbs_OUT)
        di << "Classifier UMin-VMax : Out\n";
      pcl.SetCoord(umax + difu, vmin - difv);
      if (fcl.Perform(pcl) == TopAbs_OUT)
        di << "Classifier UMax-VMin : Out\n";
      pcl.SetCoord(umax + difu, vmax + difv);
      if (fcl.Perform(pcl) == TopAbs_OUT)
        di << "Classifier UMax-VMax : Out\n";
    }
  }
  if (ShapeAnalysis1::IsOuterBound(Face))
    di << "ShapeAnalysis1: Outer Bound\n";
  else
    di << "ShapeAnalysis1: Not Outer Bound\n";
  di << " Total " << nbw << " Wire(s)\n";
  return 0;
}

static Standard_Integer XSHAPE_statshape(DrawInterpreter& di,
                                         Standard_Integer  argc,
                                         const char**      argv)
{
  if (argc < 2)
  {
    di << "ShapeName; + options : prefix for particular cases : what to get\n";
    return 1 /* Error */;
  }
  Standard_CString arg1 = argv[1];
  Standard_CString arg2 = NULL;
  Standard_CString arg3 = NULL;
  if (argc > 2)
    arg2 = argv[2];
  if (argc > 3)
    arg3 = argv[3];
  TopoShape Shape = DBRep1::Get(arg1);
  if (Shape.IsNull())
  {
    di << "Shape unknown : " << arg1 << "\n";
    return 1 /* Error */;
  }
  char             nompart[100];
  Standard_Integer nb;

  ShapeAnalysis_ShapeContents analyzer;
  if (argc > 3)
  {
    analyzer.ModifyBigSplineMode()     = (strstr("bigspl", arg3) != NULL);
    analyzer.ModifyIndirectMode()      = (strstr("indsur", arg3) != NULL);
    analyzer.ModifyOffsetSurfaceMode() = (strstr("ofsur", arg3) != NULL);
    analyzer.ModifyTrimmed3dMode()     = (strstr("trc3d", arg3) != NULL);
    analyzer.ModifyOffsetCurveMode()   = (strstr("ofcur", arg3) != NULL);
    analyzer.ModifyTrimmed2dMode()     = (strstr("trc2d", arg3) != NULL);
  }
  analyzer.Perform(Shape);

  di << "Count    Item\n-----    ----\n";
  nb = analyzer.NbEdges();
  if (nb > 0)
    di << nb << "	 EDGE (Oriented)\n";
  nb = analyzer.NbSharedEdges();
  if (nb > 0)
    di << nb << "	 EDGE (Shared)\n";
  nb = analyzer.NbFreeEdges();
  if (nb > 0)
    di << nb << "	 EDGE (Free)\n";
  nb = analyzer.NbFaces();
  if (nb > 0)
    di << nb << "	 FACE\n";
  nb = analyzer.NbFreeFaces();
  if (nb > 0)
    di << nb << "	 FACE (Free)\n";
  nb = analyzer.NbFreeWires();
  if (nb > 0)
    di << nb << "	 WIRE (Free)\n";
  nb = analyzer.NbShells();
  if (nb > 0)
    di << nb << "	 SHELL\n";
  nb = analyzer.NbSolids();
  if (nb > 0)
    di << nb << "	 SOLID\n";
  nb = analyzer.NbVertices();
  if (nb > 0)
    di << nb << "	 VERTEX (Oriented)\n";
  nb = analyzer.NbSharedVertices();
  if (nb > 0)
    di << nb << "	 VERTEX (Shared)\n";
  nb = analyzer.NbWires();
  if (nb > 0)
    di << nb << "	 WIRE\n";
  nb = analyzer.NbFaceWithSevWires();
  if (nb > 0)
    di << nb << "	Face with more than one wire\n";
  nb = analyzer.NbNoPCurve();
  if (nb > 0)
    di << nb << "	No pcurve\n";
  nb = analyzer.NbSolidsWithVoids();
  if (nb > 0)
    di << nb << "	SOLID with voids\n";
  nb = analyzer.NbWireWitnSeam();
  if (nb > 0)
    di << nb << "	Wire(s) with one seam edge\n";
  nb = analyzer.NbWireWithSevSeams();
  if (nb > 0)
    di << nb << "	Wire(s) with several seam edges\n";
  nb = analyzer.NbBigSplines();
  if (nb > 0)
    di << nb << "	bigspl : BSpline > 8192 poles\n";
  nb = analyzer.NbBezierSurf();
  if (nb > 0)
    di << nb << "	bezsur : BezierSurface\n";
  nb = analyzer.NbBSplibeSurf();
  if (nb > 0)
    di << nb << "	bspsur : BSplineSurface\n";
  nb = analyzer.NbC0Curves();
  if (nb > 0)
    di << nb << "	c0curv : Curve Only C0\n";
  nb = analyzer.NbC0Surfaces();
  if (nb > 0)
    di << nb << "	c0surf : Surface Only C0\n";
  nb = analyzer.NbIndirectSurf();
  if (nb > 0)
    di << nb << "	indsur : Indirect Surface\n";
  nb = analyzer.NbOffsetCurves();
  if (nb > 0)
    di << nb << "	ofcur  : Offset Curve(s)\n";
  nb = analyzer.NbOffsetSurf();
  if (nb > 0)
    di << nb << "	ofsur  : Offset Surface\n";
  nb = analyzer.NbTrimmedCurve2d();
  if (nb > 0)
    di << nb << "	trc2d  : Trimmed Curve2d\n";
  nb = analyzer.NbTrimmedCurve3d();
  if (nb > 0)
    di << nb << "	trc3d  : Trimmed Curve3d\n";
  nb = analyzer.NbTrimSurf();
  if (nb > 0)
    di << nb << "	trimsu : RectangularTrimmedSurface\n";

  if (arg3 == NULL)
    return 0;

  Handle(HSequenceOfShape) sec;
  if (analyzer.ModifyBigSplineMode())
  {
    sec = analyzer.BigSplineSec();
    for (Standard_Integer i = 1; i <= sec->Length(); i++)
    {
      Sprintf(nompart, "%s_bigspl_%d", arg2, i);
      DBRep1::Set(nompart, sec->Value(i));
    }
  }
  if (analyzer.ModifyIndirectMode())
  {
    sec = analyzer.IndirectSec();
    for (Standard_Integer i = 1; i <= sec->Length(); i++)
    {
      Sprintf(nompart, "%s_indsur_%d", arg2, i);
      DBRep1::Set(nompart, sec->Value(i));
    }
  }
  if (analyzer.ModifyOffsetSurfaceMode())
  {
    sec = analyzer.OffsetSurfaceSec();
    for (Standard_Integer i = 1; i <= sec->Length(); i++)
    {
      Sprintf(nompart, "%s_ofsur_%d", arg2, i);
      DBRep1::Set(nompart, sec->Value(i));
    }
  }
  if (analyzer.ModifyTrimmed3dMode())
  {
    sec = analyzer.Trimmed3dSec();
    for (Standard_Integer i = 1; i <= sec->Length(); i++)
    {
      Sprintf(nompart, "%s_trc3d_%d", arg2, i);
      DBRep1::Set(nompart, sec->Value(i));
    }
  }
  if (analyzer.ModifyOffsetCurveMode())
  {
    sec = analyzer.OffsetCurveSec();
    for (Standard_Integer i = 1; i <= sec->Length(); i++)
    {
      Sprintf(nompart, "%s_ofcur_%d", arg2, i);
      DBRep1::Set(nompart, sec->Value(i));
    }
  }
  if (analyzer.ModifyTrimmed2dMode())
  {
    sec = analyzer.Trimmed2dSec();
    for (Standard_Integer i = 1; i <= sec->Length(); i++)
    {
      Sprintf(nompart, "%s_trc2d_%d", arg2, i);
      DBRep1::Set(nompart, sec->Value(i));
    }
  }
  return 0;
}

static Standard_Integer XSHAPE_comptoledge(DrawInterpreter& di,
                                           Standard_Integer  argc,
                                           const char**      argv)
{
  if (argc < 2)
  {
    di << "Use:\n"
          "> comptol shape [nbpoints=371] [prefix]\n\n"
          "Computes real tolerance of edges in the shape \n"
          "as maximal deviation of 3d curve and pcurves.\n"
          "Deviation is computed by nbpoints sample points (default is 371).\n"
          "Gives the max, min and average value on all edges in the shape\n"
          "If prefix is defined, edges with maximal real tolerance and\n"
          "relation (and corresponding faces) will be saved with such names\n\n";
    return 0;
  }

  TopoShape shape = DBRep1::Get(argv[1]);
  if (shape.IsNull())
  {
    di << "Shape name \"" << argv[1] << "\" is invalid\n";
    return 1;
  }
  Standard_Integer nbpnts = 371;
  Standard_CString prefix = 0;
  if (argc > 2)
  {
    if (IsDigit(argv[2][0]))
    {
      nbpnts = Draw1::Atoi(argv[2]);
      if (nbpnts < 2)
        nbpnts = 2;
      if (argc > 3)
        prefix = argv[3];
    }
    else
      prefix = argv[2];
  }

  Standard_Integer   num = 0;
  Standard_Real      max = 0, min = 0, ave = 0.;
  Standard_Real      relmax = 0, relmin = 0, relave = 0.;
  Edge1 sae;
  TopoEdge        edmax, edmaxrel;
  for (ShapeExplorer exp(shape, TopAbs_EDGE); exp.More(); exp.Next())
  {
    Standard_Real tol;
    TopoEdge   edge = TopoDS::Edge(exp.Current());
    sae.CheckSameParameter(edge, tol, nbpnts);
    Standard_Real t   = BRepInspector::Tolerance(edge);
    Standard_Real rel = tol / (t > Precision1::Confusion() ? t : Precision1::Confusion());
    ave += tol;
    relave += rel;
    if (!num)
    {
      max = min = tol;
      relmax = relmin = rel;
      edmax = edmaxrel = edge;
    }
    else
    {
      if (max < tol)
      {
        max   = tol;
        edmax = edge;
      }
      if (min > tol)
        min = tol;
      if (relmax < rel)
      {
        relmax   = rel;
        edmaxrel = edge;
      }
      if (relmin > rel)
        relmin = rel;
    }
    num++;
  }
  if (!num)
  {
    di << "No edges found in the shape\n";
    return 1;
  }

  di << "Edges tolerance computed by " << nbpnts
     << " points: \n"
        "MAX="
     << max << " AVG=" << ave / num << " MIN=" << min << "\n";
  di << "Relation real tolerance / tolerance set in edge\n"
        "MAX="
     << relmax << " AVG=" << relave / num << " MIN=" << relmin << "\n";
  if (prefix && prefix[0])
  {
    char name[21];
    Sprintf(name, "%.10s_edge_tol", prefix);
    DBRep1::Set(name, edmax);
    di << "Edge with max tolerance saved to " << name;
    if (edmax.IsSame(edmaxrel))
      di << "\n";
    else
    {
      Sprintf(name, "%.10s_edge_rel", prefix);
      DBRep1::Set(name, edmaxrel);
      di << "; edge with max relation saved to " << name << "\n";
    }
    Standard_Integer num1 = 0;
    for (ShapeExplorer fac(shape, TopAbs_FACE); fac.More(); fac.Next())
    {
      TopoFace face = TopoDS::Face(fac.Current());
      for (ShapeExplorer ed(face, TopAbs_EDGE); ed.More(); ed.Next())
      {
        TopoEdge edge = TopoDS::Edge(ed.Current());
        if (edge.IsSame(edmax) || edge.IsSame(edmaxrel))
        {
          if (!num1)
            di << "Concerned faces saved to shapes ";
          Sprintf(name, "%.10s_%d", prefix, num1 + 1);
          DBRep1::Set(name, face);
          // std::cout << ( num1 ? ", " : "" ) << name;
          if (num1 == 0)
          {
            di << "" << name;
          }
          else
          {
            di << ", " << name;
          }
          num1++;
          break;
        }
      }
    }
    if (num1 > 0)
      di << "\n";
  }
  return 0;
}

//=================================================================================================

static Standard_Integer freebounds(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if ((n < 3) || (n > 5))
    return 1;
  TopoShape shape = DBRep1::Get(a[1]);
  if (shape.IsNull())
    return 1;
  Standard_Real    toler       = Draw1::Atof(a[2]);
  Standard_Boolean splitclosed = Standard_False, splitopen = Standard_False;
  if (n > 3)
    splitclosed = Draw1::Atoi(a[3]) != 0;
  if (n > 4)
    splitopen = Draw1::Atoi(a[4]) != 0;

  ShapeAnalysis_FreeBounds F;
  if (toler <= 0)
    F = ShapeAnalysis_FreeBounds(shape, splitclosed, splitopen);
  else
    F = ShapeAnalysis_FreeBounds(shape, toler, splitclosed, splitopen);

  char         name[100];
  TopoShape wires = F.GetClosedWires();
  Sprintf(name, "%s_c", a[1]);
  DBRep1::Set(name, wires);
  di << name << " - closed wires\n";
  wires = F.GetOpenWires();
  Sprintf(name, "%s_o", a[1]);
  DBRep1::Set(name, wires);
  di << name << " - open wires\n";

  return 0;
}

//=======================================================================
// function : PrintProps
// purpose  : auxiliary for FreeBoundsProps
//=======================================================================
static void PrintProps(Standard_Integer                           i,
                       const Handle(ShapeAnalysis_FreeBoundData)& fbd,
                       DrawInterpreter&                          di)
{
  char str[100];

  Standard_Real    area      = fbd->Area();
  Standard_Real    perimeter = fbd->Perimeter();
  Standard_Real    ratio     = fbd->Ratio();
  Standard_Real    width     = fbd->Width();
  Standard_Integer notch     = fbd->NbNotches();
  Sprintf(str, " %d\t%12.5f\t%12.5f\t%12.5f\t%12.5f\t%d", i, area, perimeter, ratio, width, notch);
  di << str << "\n";
}

//=================================================================================================

static Standard_Integer FreeBoundsProps(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if ((n < 2) || (n > 5))
  {
    di << "Usage : freeprops shapename [tolerance [splitclosed [splitopen]]]\n";
    return 1;
  }

  TopoShape source = DBRep1::Get(a[1]);
  if (source.IsNull())
  {
    di << "Error : unknown shape " << a[1] << "\n";
    return 1;
  }

  Standard_Real    toler       = 0.;
  Standard_Boolean splitclosed = Standard_False, splitopen = Standard_False;
  if (n > 2)
    toler = Draw1::Atof(a[2]);
  if (n > 3)
    splitclosed = Draw1::Atoi(a[3]) != 0;
  if (n > 4)
    splitopen = Draw1::Atoi(a[4]) != 0;
  ShapeAnalysis_FreeBoundsProperties analyzer;
  if (toler > 0)
    analyzer.Init(source, toler, splitclosed, splitopen);
  else
    analyzer.Init(source, splitclosed, splitopen);
  analyzer.Perform();
  TopoCompound closed, open;
  ShapeBuilder    B;

  Standard_Integer nb = analyzer.NbClosedFreeBounds();
  di << "\n";
  di << " \tArea mm2\tLength mm\tRatio L/W\tWidth mm\tNb notch\n";
  B.MakeCompound(closed);
  if (nb)
  {
    di << "Closed bounds properties\n";
    for (Standard_Integer i = 1; i <= nb; i++)
    {
      Handle(ShapeAnalysis_FreeBoundData) fbd = analyzer.ClosedFreeBound(i);
      PrintProps(i, fbd, di);
      B.Add(closed, fbd->FreeBound());
    }
  }

  nb = analyzer.NbOpenFreeBounds();
  B.MakeCompound(open);
  if (nb)
  {
    di << "Open bounds properties\n";
    for (Standard_Integer i = 1; i <= nb; i++)
    {
      Handle(ShapeAnalysis_FreeBoundData) fbd = analyzer.OpenFreeBound(i);
      PrintProps(i, fbd, di);
      B.Add(open, fbd->FreeBound());
    }
  }

  char name[100];
  Sprintf(name, "%s_c", a[1]);
  di << name << " - closed wires,  ";
  DBRep1::Set(name, closed);
  Sprintf(name, "%s_o", a[1]);
  di << name << " - closed wires \n";
  DBRep1::Set(name, open);
  return 0;
}

//=================================================================================================

static Standard_Integer closefreebounds(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if ((n < 4) || (n > 6))
    return 1;
  TopoShape shape = DBRep1::Get(a[1]);
  if (shape.IsNull())
    return 1;
  Standard_Real    sewtoler = Draw1::Atof(a[2]), closetoler = Draw1::Atof(a[3]);
  Standard_Boolean splitclosed = Standard_False, splitopen = Standard_False;
  if (n > 4)
    splitclosed = Draw1::Atoi(a[3]) != 0;
  if (n > 5)
    splitopen = Draw1::Atoi(a[4]) != 0;

  ShapeFix_FreeBounds F;
  if (sewtoler <= 0)
    F = ShapeFix_FreeBounds(shape, closetoler, splitclosed, splitopen);
  else
    F = ShapeFix_FreeBounds(shape, sewtoler, closetoler, splitclosed, splitopen);

  char         name[100];
  TopoShape wires = F.GetClosedWires();
  Sprintf(name, "%s_c", a[1]);
  DBRep1::Set(name, wires);
  di << name << " - closed wires\n";
  wires = F.GetOpenWires();
  Sprintf(name, "%s_o", a[1]);
  DBRep1::Set(name, wires);
  di << name << " - open wires\n";

  return 0;
}

static Standard_Integer getareacontour(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 2)
    return 1;
  TopoShape shape = DBRep1::Get(a[1]);
  if (shape.IsNull())
  {
    di << "Shape is not defined\n";
    return 1;
  }
  if (shape.ShapeType() != TopAbs_WIRE)
  {
    di << "invalid type of argument\n";
    return 1;
  }
  // Handle(ShapeExtend_WireData) asewd = new ShapeExtend_WireData(TopoDS::Wire(shape));
  Standard_Real anArea = ShapeAnalysis1::ContourArea(TopoDS::Wire(shape));
  di << "Area = " << anArea << "\n";
  return 0;
}

static Standard_Integer checkselfintersection(DrawInterpreter& di,
                                              Standard_Integer  argc,
                                              const char**      argv)
{
  if (argc < 2)
  {
    di << "Call please \"checkselfintersection wire [face]\"\n";
    return 1;
  }

  // Get wire.
  const char*  arg1 = argv[1];
  TopoShape wire = DBRep1::Get(arg1);
  if (wire.IsNull() || wire.ShapeType() != TopAbs_WIRE)
  {
    di << "A null shape or not a wire is used.\n";
    return 2;
  }

  // Get face if the user provided us with a face.
  TopoShape face;
  if (argc > 2)
  {
    const char* arg2 = argv[2];
    face             = DBRep1::Get(arg2);
    if (face.IsNull() || face.ShapeType() != TopAbs_FACE)
    {
      di << "A null shape or not a face is used.\n";
      return 3;
    }
  }

  // If the face is null, make a plane inside the wire.
  if (face.IsNull())
  {
    FaceMaker mkFace(TopoDS::Wire(wire), true);
    if (mkFace.IsDone())
      face = mkFace.Face();
    else
    {
      di << "Can't make a face for the wire. Provide please a face for analysis.\n";
      return 4;
    }
  }

  ShapeAnalysis_Wire analyser(TopoDS::Wire(wire), TopoDS::Face(face), Precision1::Confusion());
  Standard_Boolean   result = analyser.CheckSelfIntersection();

  if (result == Standard_True)
    di << "A self-intersecting wire.\n";
  else
    di << "Not self-intersecting wire.\n";
  return 0;
}

static Standard_Integer checkedge(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  if (argc < 2)
  {
    di << "Call please \"checkedge edge [face]\"\n";
    return 1;
  }

  // Get edge.
  const char*  arg1 = argv[1];
  TopoShape edge = DBRep1::Get(arg1);
  if (edge.IsNull() || edge.ShapeType() != TopAbs_EDGE)
  {
    di << "A null shape or not an edge is used.\n";
    return 2;
  }

  // Get face.
  TopoShape face;
  if (argc == 3)
  {
    const char* arg2 = argv[2];
    face             = DBRep1::Get(arg2);
    if (face.IsNull() || face.ShapeType() != TopAbs_FACE)
    {
      di << "A null shape or not a face is used.\n";
      return 3;
    }
  }

  // Analysis of the edge.
  Edge1 analyser;
  Standard_Boolean   isOk(Standard_True);

  // Curve 3D.
  if (analyser.HasCurve3d(TopoDS::Edge(edge)))
  {
    // Check vertices.
    if (analyser.CheckVerticesWithCurve3d(TopoDS::Edge(edge)))
    {
      isOk = Standard_False;
      di << "Vertices of the edge don't coincide with start/end points of 3d-curve (using "
            "tolerance of the vertices).\n";
    }
  }
  else
  {
    isOk = Standard_False;
    di << "Edge doesn't have a 3d-curve\n";
  }

  if (!face.IsNull())
  {
    // Curve 2D.
    if (analyser.HasPCurve(TopoDS::Edge(edge), TopoDS::Face(face)))
    {
      // Check vertices.
      if (analyser.CheckVerticesWithPCurve(TopoDS::Edge(edge), TopoDS::Face(face)))
      {
        isOk = Standard_False;
        di << "Vertices of the edge don't coincide with start/end points of 2d-curve (using "
              "tolerance of the vertices).\n";
      }
    }
    else
    {
      isOk = Standard_False;
      di << "Edge doesn't have a 2d-curve on this face\n";
    }
  }

  if (isOk)
    di << "Edge seems OK.\n";

  return 0;
}

//=======================================================================
// getanasurf
//=======================================================================
static Standard_Integer getanasurf(DrawInterpreter& di, Standard_Integer n, const char** a)

{
  if (n < 3)
  {
    di << "Usage: \n";
    di << "getanasurf res shape [target [tol [sample]]] \n";
    di << "target is required type of surface and can be: pln, cyl, con sph \n";
    di << "sample is surface of required type, which parameters are used as starting \n";
    di << "point for searching parameters of surface by Least Square method when input shape \n";
    di << "is edge or wire \n";
    return 1;
  }
  TopoShape sh = DBRep1::Get(a[2]);
  if (sh.IsNull())
    return 1;
  TopAbs_ShapeEnum aShType = sh.ShapeType();
  if (!(aShType == TopAbs_SHELL || aShType == TopAbs_FACE || aShType == TopAbs_EDGE
        || aShType == TopAbs_WIRE))
  {
    di << "Wrong shape type, shape can be shell or face or edge or wire\n";
    return 1;
  }

  GeomAbs_SurfaceType aTargets[] = {GeomAbs_Plane, GeomAbs_Cylinder, GeomAbs_Cone, GeomAbs_Sphere};
  Standard_Integer    isurf      = 0;
  if (n > 3)
  {
    if (strcmp(a[3], "pln") == 0)
      isurf = 0;
    else if (strcmp(a[3], "cyl") == 0)
      isurf = 1;
    else if (strcmp(a[3], "con") == 0)
      isurf = 2;
    else if (strcmp(a[3], "sph") == 0)
      isurf = 3;
  }

  Standard_Real tol = 1.e-7;
  if (n > 4)
    tol = Draw1::Atof(a[4]);

  // get sample target for edge and wire
  GeomAdaptor_Surface aSampleSurf;
  if (n > 5 && (sh.ShapeType() == TopAbs_EDGE || sh.ShapeType() == TopAbs_WIRE))
  {
    Handle(GeomSurface) aGSurf = DrawTrSurf1::GetSurface(a[5]);
    if (aGSurf.IsNull())
    {
      di << "Sample surface is null" << "\n";
      return 1;
    }
    aSampleSurf.Load(aGSurf);
    GeomAbs_SurfaceType aSType = aSampleSurf.GetType();
    if (aSType != aTargets[isurf])
    {
      di << "Sample surface has wrong type" << "\n";
      return 1;
    }
  }

  ShapeAnalysis_CanonicalRecognition aCanonRec(sh);
  Handle(GeomSurface)               aRes;
  switch (aTargets[isurf])
  {
    case GeomAbs_Plane: {
      gp_Pln aPln;
      if (aSampleSurf.GetType() == GeomAbs_Plane)
        aPln = aSampleSurf.Plane1();
      if (aCanonRec.IsPlane(tol, aPln))
        aRes = new GeomPlane(aPln);
      break;
    }
    case GeomAbs_Cylinder: {
      Cylinder1 aCyl;
      if (aSampleSurf.GetType() == GeomAbs_Cylinder)
        aCyl = aSampleSurf.Cylinder();
      if (aCanonRec.IsCylinder(tol, aCyl))
        aRes = new Geom_CylindricalSurface(aCyl);
      break;
    }
    case GeomAbs_Cone: {
      Cone1 aCon;
      if (aSampleSurf.GetType() == GeomAbs_Cone)
        aCon = aSampleSurf.Cone();
      if (aCanonRec.IsCone(tol, aCon))
        aRes = new Geom_ConicalSurface(aCon);
      break;
    }
    case GeomAbs_Sphere: {
      Sphere3 aSph;
      if (aSampleSurf.GetType() == GeomAbs_Sphere)
        aSph = aSampleSurf.Sphere();
      if (aCanonRec.IsSphere(tol, aSph))
        aRes = new Geom_SphericalSurface(aSph);
      break;
    }
    default:
      break;
  }

  if (!aRes.IsNull())
  {
    DrawTrSurf1::Set(a[1], aRes);
    di << "Gap = " << aCanonRec.GetGap() << "\n";
  }
  else
  {
    di << "Cannot get required surface" << "\n";
  }
  return 0;
}

//=================================================================================================

Standard_Integer getanacurve(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n < 3)
  {
    di << "Usage: \n";
    di << "getanacurve res shape [target [tol]] \n";
    di << "target is required type of curve and can be: lin, cir, ell  \n";
    return 1;
  }
  TopoShape sh = DBRep1::Get(a[2]);
  if (sh.IsNull())
    return 1;
  TopAbs_ShapeEnum aShType = sh.ShapeType();
  if (!(aShType == TopAbs_WIRE || aShType == TopAbs_EDGE))
  {
    di << "Wrong shape type, shape can be wire or an edge \n";
    return 1;
  }

  GeomAbs_CurveType aTargets[] = {GeomAbs_Line, GeomAbs_Circle, GeomAbs_Ellipse};
  Standard_Integer  icurv      = 0;
  if (n > 3)
  {
    if (strcmp(a[3], "lin") == 0)
      icurv = 0;
    else if (strcmp(a[3], "cir") == 0)
      icurv = 1;
    else if (strcmp(a[3], "ell") == 0)
      icurv = 2;
  }

  Standard_Real tol = Precision1::Confusion();
  if (n > 4)
    tol = Draw1::Atof(a[4]);

  ShapeAnalysis_CanonicalRecognition aCanonRec(sh);
  Handle(GeomCurve3d)                 aRes;
  switch (aTargets[icurv])
  {
    case GeomAbs_Line: {
      gp_Lin aLin;
      if (aCanonRec.IsLine(tol, aLin))
        aRes = new GeomLine(aLin);
      break;
    }
    case GeomAbs_Circle: {
      gp_Circ aCirc;
      if (aCanonRec.IsCircle(tol, aCirc))
        aRes = new GeomCircle(aCirc);
      break;
    }
    case GeomAbs_Ellipse: {
      gp_Elips anElips;
      if (aCanonRec.IsEllipse(tol, anElips))
        aRes = new Geom_Ellipse(anElips);
      break;
    }
    default:
      break;
  }

  if (!aRes.IsNull())
  {
    DrawTrSurf1::Set(a[1], aRes);
    di << "Gap = " << aCanonRec.GetGap() << "\n";
  }
  else
  {
    di << "Cannot get required curve" << "\n";
  }
  return 0;
}

//=================================================================================================

void ShapeAnalysis2::InitCommands(DrawInterpreter& theCommands)
{
  static Standard_Integer initactor = 0;
  if (initactor)
  {
    return;
  }
  initactor = 1;

  Standard_CString g = SWDRAW1::GroupName();

  theCommands.Add("tolerance", "shape [tolmin tolmax:real]", __FILE__, tolerance, g);
  theCommands.Add("projface",
                  "nom_face X Y [Z] - returns the closest orthogonal projection if exists",
                  __FILE__,
                  projface,
                  g);
  theCommands.Add("projcurve",
                  "nom_edge | curve3d | curve3d first last + X Y Z",
                  __FILE__,
                  projcurve,
                  g);
  theCommands.Add("projpcurve", "edge face tol x y z [start_param]", __FILE__, projpcurve, g);
  theCommands.Add("anaface", "nomface", __FILE__, anaface, g);
  theCommands.Add("statshape",
                  "shape [particul] : stats/particularites",
                  __FILE__,
                  XSHAPE_statshape,
                  g);
  theCommands.Add("comptol", "shape [nbpoints]", __FILE__, XSHAPE_comptoledge, g);
  theCommands.Add(
    "freebounds",
    "shp toler [splitclosed [splitopen]] - free bounds; toler <= 0 for shells (no sewing call)",
    __FILE__,
    freebounds,
    g);

  const char* groupold = "DE: old";
  theCommands.Add("fbprops",
                  "shp [toler [splitclosed [splitopen]]] - free bounds properties; toler <= 0 or "
                  "not specified - for shells (no sewing call)",
                  __FILE__,
                  FreeBoundsProps,
                  groupold);
  theCommands.Add("fbclose",
                  "shp sewtoler closetoler [splitclosed [splitopen]] - closes free bounds; use "
                  "sewtoler <= 0 for shells (no sewing call)",
                  __FILE__,
                  closefreebounds,
                  groupold);

  theCommands.Add("getareacontour", "wire ", __FILE__, getareacontour, groupold);
  theCommands.Add("checkselfintersection", "wire [face]", __FILE__, checkselfintersection, g);
  theCommands.Add("checkedge", "edge [face]", __FILE__, checkedge, g);
  theCommands.Add("getanasurf",
                  "getanasurf res shape [target [tol [sample]]] ",
                  __FILE__,
                  getanasurf,
                  g);
  theCommands.Add("getanacurve", "getanacurve res shape [target [tol]]", __FILE__, getanacurve, g);
}
