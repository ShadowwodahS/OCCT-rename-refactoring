// Created on: 1998-03-23
// Created by: Jean Yves LEBEY
// Copyright (c) 1998-1999 Matra Datavision
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

#include <gp_Vec2d.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopOpeBRepTool_2d.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <ProjLib_ProjectedCurve.hxx>
#include <Geom_Surface.hxx>
#include <TopOpeBRepTool_CurveTool.hxx>
#include <TopOpeBRepTool_DataMapOfShapeListOfC2DF.hxx>
#include <TopOpeBRepTool_C2DF.hxx>
#include <TopOpeBRepTool_ListOfC2DF.hxx>
#include <TopOpeBRepTool_tol.hxx>
#include <TopOpeBRepTool_EXPORT.hxx>
#include <TopOpeBRepTool_TOOL.hxx>

#ifdef DRAW
  #include <TopOpeBRepTool_DRAW.hxx>
  #include <Geom2d_Curve.hxx>
#endif

#ifdef OCCT_DEBUG
void debc2dnull(void) {}

Standard_EXPORT Standard_Boolean TopOpeBRepTool_GettraceC2D();
#endif

// structure e -> C2D/F
static TopOpeBRepTool_DataMapOfShapeListOfC2DF* GLOBAL_pmosloc2df = NULL;
static Standard_Integer                         GLOBAL_C2D_i      = 0; // DEB

// structure ancetre
static TopTools_IndexedDataMapOfShapeListOfShape* GLOBAL_pidmoslosc2df = NULL;
static TopoFace*                               GLOBAL_pFc2df        = NULL;
static TopoShape*                              GLOBAL_pS1c2df       = NULL;
static TopoShape*                              GLOBAL_pS2c2df       = NULL;

Standard_EXPORT Handle(GeomCurve2d) MakePCurve(const ProjLib_ProjectedCurve& PC);

// ------------------------------------------------------------------------------------
static const TopoFace& FC2D_FancestorE(const TopoEdge& E)
{
  if (GLOBAL_pmosloc2df == NULL)
    GLOBAL_pmosloc2df = new TopOpeBRepTool_DataMapOfShapeListOfC2DF();
  Standard_Integer ancemp = (*GLOBAL_pidmoslosc2df).Extent();
  if (ancemp == 0)
  {
    TopExp1::MapShapesAndAncestors(*GLOBAL_pS1c2df,
                                  TopAbs_EDGE,
                                  TopAbs_FACE,
                                  (*GLOBAL_pidmoslosc2df));
    TopExp1::MapShapesAndAncestors(*GLOBAL_pS2c2df,
                                  TopAbs_EDGE,
                                  TopAbs_FACE,
                                  (*GLOBAL_pidmoslosc2df));
  }
  Standard_Boolean Eb = (*GLOBAL_pidmoslosc2df).Contains(E);
  if (!Eb)
    return *GLOBAL_pFc2df;
  const ShapeList& lf = (*GLOBAL_pidmoslosc2df).FindFromKey(E);
  if (lf.IsEmpty())
    return *GLOBAL_pFc2df;
  const TopoFace& F = TopoDS::Face(lf.First());
  return F;
}

// ------------------------------------------------------------------------------------
Standard_EXPORT int FC2D_Prepare(const TopoShape& S1, const TopoShape& S2)
{
  if (GLOBAL_pmosloc2df == NULL)
    GLOBAL_pmosloc2df = new TopOpeBRepTool_DataMapOfShapeListOfC2DF();
  GLOBAL_pmosloc2df->Clear();
  GLOBAL_C2D_i = 0;

  if (GLOBAL_pidmoslosc2df == NULL)
    GLOBAL_pidmoslosc2df = new TopTools_IndexedDataMapOfShapeListOfShape();
  GLOBAL_pidmoslosc2df->Clear();

  if (GLOBAL_pFc2df == NULL)
    GLOBAL_pFc2df = new TopoFace();
  GLOBAL_pFc2df->Nullify();

  if (GLOBAL_pS1c2df == NULL)
    GLOBAL_pS1c2df = new TopoShape();
  *GLOBAL_pS1c2df = S1;

  if (GLOBAL_pS2c2df == NULL)
    GLOBAL_pS2c2df = new TopoShape();
  *GLOBAL_pS2c2df = S2;

  return 0;
}

// ------------------------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FC2D_HasC3D(const TopoEdge& E)
{
  TopLoc_Location    loc;
  Standard_Real      f3d, l3d;
  Handle(GeomCurve3d) C3D = BRepInspector::Curve(E, loc, f3d, l3d);
  Standard_Boolean   b   = (!C3D.IsNull());
  return b;
}

// ------------------------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FC2D_HasCurveOnSurface(const TopoEdge& E, const TopoFace& F)
{
  Handle(GeomCurve2d) C2D;
  Standard_Boolean     hasold = FC2D_HasOldCurveOnSurface(E, F, C2D);
  Standard_Boolean     hasnew = FC2D_HasNewCurveOnSurface(E, F, C2D);
  Standard_Boolean     b      = hasold || hasnew;
  return b;
}

// ------------------------------------------------------------------------------------
Standard_EXPORT Standard_Boolean FC2D_HasOldCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D,
                                                           Standard_Real&        f2d,
                                                           Standard_Real&        l2d,
                                                           Standard_Real&        tol)
{
  Standard_Boolean hasold = Standard_False;
  tol                     = BRepInspector::Tolerance(E);
  C2D                     = BRepInspector::CurveOnSurface(E, F, f2d, l2d);
  hasold                  = (!C2D.IsNull());
  return hasold;
}

Standard_EXPORT Standard_Boolean FC2D_HasOldCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D)
{
  Standard_Real    f2d, l2d, tol;
  Standard_Boolean hasold = FC2D_HasOldCurveOnSurface(E, F, C2D, f2d, l2d, tol);
  return hasold;
}

// ------------------------------------------------------------------------------------
static TopOpeBRepTool_C2DF* FC2D_PNewCurveOnSurface(const TopoEdge& E, const TopoFace& F)
{
  TopOpeBRepTool_C2DF* pc2df = NULL;
  if (GLOBAL_pmosloc2df == NULL)
    return NULL;
  Standard_Boolean Eisb = GLOBAL_pmosloc2df->IsBound(E);
  if (!Eisb)
    return NULL;
  TopOpeBRepTool_ListIteratorOfListOfC2DF it(GLOBAL_pmosloc2df->Find(E));
  for (; it.More(); it.Next())
  {
    const TopOpeBRepTool_C2DF& c2df = it.Value();
    Standard_Boolean           isf  = c2df.IsFace(F);
    if (isf)
    {
      pc2df = (TopOpeBRepTool_C2DF*)&c2df;
      break;
    }
  }
  return pc2df;
}

Standard_EXPORT Standard_Boolean FC2D_HasNewCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D,
                                                           Standard_Real&        f2d,
                                                           Standard_Real&        l2d,
                                                           Standard_Real&        tol)
{
  const TopOpeBRepTool_C2DF* pc2df  = FC2D_PNewCurveOnSurface(E, F);
  Standard_Boolean           hasnew = (pc2df != NULL);
  if (hasnew)
    C2D = pc2df->PC(f2d, l2d, tol);
  return hasnew;
}

Standard_EXPORT Standard_Boolean FC2D_HasNewCurveOnSurface(const TopoEdge&    E,
                                                           const TopoFace&    F,
                                                           Handle(GeomCurve2d)& C2D)
{
  Standard_Real    f2d, l2d, tol;
  Standard_Boolean b = FC2D_HasNewCurveOnSurface(E, F, C2D, f2d, l2d, tol);
  return b;
}

// ------------------------------------------------------------------------------------
Standard_Integer FC2D_AddNewCurveOnSurface(Handle(GeomCurve2d) C2D,
                                           const TopoEdge&   E,
                                           const TopoFace&   F,
                                           const Standard_Real& f2d,
                                           const Standard_Real& l2d,
                                           const Standard_Real& tol)
{
  if (C2D.IsNull())
    return 1;
  TopOpeBRepTool_C2DF c2df(C2D, f2d, l2d, tol, F);
  if (GLOBAL_pmosloc2df == NULL)
    return 1;
  TopOpeBRepTool_ListOfC2DF thelist;
  GLOBAL_pmosloc2df->Bind(E, thelist);
  TopOpeBRepTool_ListOfC2DF& lc2df = GLOBAL_pmosloc2df->ChangeFind(E);
  lc2df.Append(c2df);
  return 0;
}

// ------------------------------------------------------------------------------------
static Handle(GeomCurve2d) FC2D_make2d(const TopoEdge&     E,
                                        const TopoFace&     F,
                                        Standard_Real&         f2d,
                                        Standard_Real&         l2d,
                                        Standard_Real&         tol,
                                        const Standard_Boolean trim3d = Standard_False);

static Handle(GeomCurve2d) FC2D_make2d(const TopoEdge&     E,
                                        const TopoFace&     F,
                                        Standard_Real&         f2d,
                                        Standard_Real&         l2d,
                                        Standard_Real&         tol,
                                        const Standard_Boolean trim3d)
{
  Handle(GeomCurve2d) C2D = BRepInspector::CurveOnSurface(E, F, f2d, l2d);
  if (!C2D.IsNull())
    return C2D;

  // pas de 2D
  Standard_Real      f3d, l3d;
  TopLoc_Location    eloc;
  Handle(GeomCurve3d) C1     = BRepInspector::Curve(E, eloc, f3d, l3d);
  Standard_Boolean   hasC3D = (!C1.IsNull());

  if (hasC3D)
  {
    Standard_Boolean   elocid = eloc.IsIdentity();
    Handle(GeomCurve3d) C2;
    if (elocid)
      C2 = C1;
    else
      C2 = Handle(GeomCurve3d)::DownCast(C1->Transformed(eloc.Transformation()));
    Standard_Real f = 0., l = 0.;
    if (trim3d)
    {
      f = f3d;
      l = l3d;
    }
    C2D = CurveTool6::MakePCurveOnFace(F, C2, tol, f, l);
    f2d = f3d;
    l2d = l3d;
    return C2D;
  }
  else
  {
    // E sans courbe 2d sur F, E sans courbe 3d
    // une face accedant a E : FE
    const TopoFace& FE = FC2D_FancestorE(E);
    if (FE.IsNull())
      return C2D;
    Standard_Boolean            compminmaxUV = Standard_False;
    BRepAdaptor_Surface         BAS(F, compminmaxUV);
    Handle(BRepAdaptor_Surface) BAHS = new BRepAdaptor_Surface(BAS);
    BRepAdaptor_Curve           AC(E, FE);
    Handle(BRepAdaptor_Curve)   AHC = new BRepAdaptor_Curve(AC);
    Standard_Real               tolin;
    FTOL_FaceTolerances3d(F, FE, tolin);
    ProjLib_ProjectedCurve projcurv(BAHS, AHC, tolin);
    C2D = MakePCurve(projcurv);
    Standard_Real f, l;
    BRepInspector::Range(E, f, l);
    f2d = f;
    l2d = l;
  }

#ifdef OCCT_DEBUG
  if (TopOpeBRepTool_GettraceC2D() && C2D.IsNull())
  {
    std::cout << "#FC2D_make2d1 --> PCurve IsNull" << std::endl;
  }
#endif

  return C2D;
} // make2d1

// ------------------------------------------------------------------------------------
// modified by NIZHNY-MZV  Mon Oct  4 10:37:36 1999
Standard_EXPORT Handle(GeomCurve2d) FC2D_MakeCurveOnSurface(const TopoEdge&     E,
                                                             const TopoFace&     F,
                                                             Standard_Real&         f,
                                                             Standard_Real&         l,
                                                             Standard_Real&         tol,
                                                             const Standard_Boolean trim3d)
{
#ifdef DRAW
  if (TopOpeBRepTool_GettraceC2D())
  {
    std::cout << "\n#FC2D_MakeCurveOnSurface : " << std::endl;
    AsciiString1 fnam("c2df");
    GLOBAL_C2D_i++;
    fnam = fnam + GLOBAL_C2D_i;
    FDRAW_DINS("", F, fnam, "");
    AsciiString1 enam("c2de");
    GLOBAL_C2D_i++;
    enam = enam + GLOBAL_C2D_i;
    FDRAW_DINE(" ", E, enam, "\n");
    std::cout.flush();
    debc2dnull();
  }
#endif

  Handle(GeomCurve2d) C2D = FC2D_make2d(E, F, f, l, tol, trim3d);
  FC2D_AddNewCurveOnSurface(C2D, E, F, f, l, tol);
  return C2D;
}

// ------------------------------------------------------------------------------------
Standard_EXPORT Handle(GeomCurve2d) FC2D_CurveOnSurface(const TopoEdge&     E,
                                                         const TopoFace&     F,
                                                         Standard_Real&         f,
                                                         Standard_Real&         l,
                                                         Standard_Real&         tol,
                                                         const Standard_Boolean trim3d)
{
  Handle(GeomCurve2d) C2D;
  Standard_Boolean     hasold = FC2D_HasOldCurveOnSurface(E, F, C2D, f, l, tol);
  if (hasold)
  {
    return C2D;
  }
  Standard_Boolean hasnew = FC2D_HasNewCurveOnSurface(E, F, C2D, f, l, tol);
  if (hasnew)
  {
    return C2D;
  }
  C2D = FC2D_MakeCurveOnSurface(E, F, f, l, tol, trim3d);
  return C2D;
}

// ------------------------------------------------------------------------------------
Standard_EXPORT Handle(GeomCurve2d) FC2D_EditableCurveOnSurface(const TopoEdge&     E,
                                                                 const TopoFace&     F,
                                                                 Standard_Real&         f,
                                                                 Standard_Real&         l,
                                                                 Standard_Real&         tol,
                                                                 const Standard_Boolean trim3d)
{
  Standard_Boolean hasold = Standard_False;
  {
    Handle(GeomCurve2d) C2D;
    hasold = FC2D_HasOldCurveOnSurface(E, F, C2D, f, l, tol);
    if (hasold)
    {
      Handle(GeomCurve2d) copC2D = Handle(GeomCurve2d)::DownCast(C2D->Copy());
      return copC2D;
    }
  }
  Standard_Boolean hasnew = Standard_False;
  {
    Handle(GeomCurve2d) newC2D;
    hasnew = FC2D_HasNewCurveOnSurface(E, F, newC2D, f, l, tol);
    if (hasnew)
    {
      return newC2D;
    }
  }
  Handle(GeomCurve2d) makC2D = FC2D_MakeCurveOnSurface(E, F, f, l, tol, trim3d);
  return makC2D;
}

// ------------------------------------------------------------------------------------
static void FC2D_translate(Handle(GeomCurve2d) C2D,
                           //                           const TopoEdge& E,
                           const TopoEdge&,
                           const TopoFace& F,
                           const TopoEdge& EF)
{
  TopLoc_Location             sloc;
  const Handle(GeomSurface)& S1      = BRepInspector::Surface(F, sloc);
  Standard_Boolean            isperio = S1->IsUPeriodic() || S1->IsVPeriodic();
  gp_Dir2d                    d2d;
  gp_Pnt2d                    O2d;
  Standard_Boolean            isuiso, isviso;
  Standard_Boolean            uviso  = TOOL1::UVISO(C2D, isuiso, isviso, d2d, O2d);
  Standard_Boolean            EFnull = EF.IsNull();

  if (isperio && uviso && !EFnull)
  {
    // C2D prend comme origine dans F l'origine de la pcurve de EF dans F
    TopoFace FFOR = F;
    FFOR.Orientation(TopAbs_FORWARD);
    gp_Pnt2d p1, p2;
    BRepInspector::UVPoints(EF, FFOR, p1, p2);
    Standard_Real    pEF    = isuiso ? p1.X() : p1.Y();
    Standard_Real    pC2D   = isuiso ? O2d.X() : O2d.Y();
    Standard_Real    factor = pEF - pC2D;
    Standard_Boolean b      = (Abs(factor) > 1.e-6);
    if (b)
    {
      gp_Vec2d transl(1., 0.);
      if (isviso)
        transl = gp_Vec2d(0., 1.);
      transl.Multiply(factor);
      C2D->Translate(transl);
    }
  }
}

// ------------------------------------------------------------------------------------
static Handle(GeomCurve2d) FC2D_make2d(const TopoEdge&     E,
                                        const TopoFace&     F,
                                        const TopoEdge&     EF,
                                        Standard_Real&         f2d,
                                        Standard_Real&         l2d,
                                        Standard_Real&         tol,
                                        const Standard_Boolean trim3d = Standard_False);

static Handle(GeomCurve2d) FC2D_make2d(const TopoEdge&     E,
                                        const TopoFace&     F,
                                        const TopoEdge&     EF,
                                        Standard_Real&         f2d,
                                        Standard_Real&         l2d,
                                        Standard_Real&         tol,
                                        const Standard_Boolean trim3d)
{
  Handle(GeomCurve2d) C2D = BRepInspector::CurveOnSurface(E, F, f2d, l2d);
  if (!C2D.IsNull())
    return C2D;

  // pas de 2D
  Standard_Real      f3d, l3d;
  TopLoc_Location    eloc;
  Handle(GeomCurve3d) C1     = BRepInspector::Curve(E, eloc, f3d, l3d);
  Standard_Boolean   hasC3D = (!C1.IsNull());

  if (hasC3D)
  {
    Standard_Boolean   elocid = eloc.IsIdentity();
    Handle(GeomCurve3d) C2;
    if (elocid)
      C2 = C1;
    else
      C2 = Handle(GeomCurve3d)::DownCast(C1->Transformed(eloc.Transformation()));
    Standard_Real f = 0., l = 0.;
    if (trim3d)
    {
      f = f3d;
      l = l3d;
    }
    C2D = CurveTool6::MakePCurveOnFace(F, C2, tol, f, l);
    f2d = f3d;
    l2d = l3d;
    FC2D_translate(C2D, E, F, EF);
    return C2D;
  }
  else
  {
    // E sans courbe 2d sur F, E sans courbe 3d
    // une face accedant a E : FE
    const TopoFace& FE = FC2D_FancestorE(E);
    if (FE.IsNull())
      return C2D;
    Standard_Boolean            compminmaxUV = Standard_False;
    BRepAdaptor_Surface         BAS(F, compminmaxUV);
    Handle(BRepAdaptor_Surface) BAHS = new BRepAdaptor_Surface(BAS);
    BRepAdaptor_Curve           AC(E, FE);
    Handle(BRepAdaptor_Curve)   AHC = new BRepAdaptor_Curve(AC);
    Standard_Real               tolin;
    FTOL_FaceTolerances3d(F, FE, tolin);
    ProjLib_ProjectedCurve projcurv(BAHS, AHC, tolin);
    C2D = MakePCurve(projcurv);
    Standard_Real f, l;
    BRepInspector::Range(E, f, l);
    f2d = f;
    l2d = l;
    FC2D_translate(C2D, E, F, EF);
  }

#ifdef OCCT_DEBUG
  if (TopOpeBRepTool_GettraceC2D() && C2D.IsNull())
  {
    std::cout << "#FC2D_make2d2 --> PCurve IsNull" << std::endl;
  }
#endif

  return C2D;
} // make2d2

// ------------------------------------------------------------------------------------
Standard_EXPORT Handle(GeomCurve2d) FC2D_CurveOnSurface(const TopoEdge&     E,
                                                         const TopoFace&     F,
                                                         const TopoEdge&     EF,
                                                         Standard_Real&         f2d,
                                                         Standard_Real&         l2d,
                                                         Standard_Real&         tol,
                                                         const Standard_Boolean trim3d)
{
  Handle(GeomCurve2d) C2D;

  Standard_Boolean hasold = FC2D_HasOldCurveOnSurface(E, F, C2D, f2d, l2d, tol);
  if (hasold)
    return C2D;

  TopOpeBRepTool_C2DF* pc2df = FC2D_PNewCurveOnSurface(E, F);
  if (pc2df != NULL)
  {
    C2D = pc2df->PC(f2d, l2d, tol);
    FC2D_translate(C2D, E, F, EF);
    pc2df->SetPC(C2D, f2d, l2d, tol);
    return C2D;
  }

#ifdef DRAW
  if (TopOpeBRepTool_GettraceC2D())
  {
    std::cout << "\n#FC2D_CurveOnSurface : " << std::endl;
    AsciiString1 fnam("c2df");
    GLOBAL_C2D_i++;
    fnam = fnam + GLOBAL_C2D_i;
    FDRAW_DINS("", F, fnam, "");
    AsciiString1 enam("c2de");
    GLOBAL_C2D_i++;
    enam = enam + GLOBAL_C2D_i;
    FDRAW_DINE(" ", E, enam, "\n");
    std::cout.flush();
    debc2dnull();
  }
#endif

  C2D = FC2D_make2d(E, F, EF, f2d, l2d, tol, trim3d);
  FC2D_AddNewCurveOnSurface(C2D, E, F, f2d, l2d, tol);
  return C2D;
}
