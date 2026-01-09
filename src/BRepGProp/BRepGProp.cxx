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

#include <BRepGProp.hxx>
#include <BRepGProp_Cinert.hxx>
#include <BRepGProp_Sinert.hxx>
#include <BRepGProp_Vinert.hxx>
#include <BRepGProp_MeshProps.hxx>
#include <BRepGProp_MeshCinert.hxx>
#include <BRepGProp_VinertGK.hxx>
#include <BRepGProp_Face.hxx>
#include <BRepGProp_Domain.hxx>
#include <TopoDS.hxx>
#include <BRepAdaptor_Curve.hxx>

#include <BRep_Tool.hxx>
#include <TopTools_MapOfShape.hxx>
#include <BRepCheck_Shell.hxx>

#ifdef OCCT_DEBUG
static Standard_Integer AffichEps = 0;
#endif

static Point3d roughBaryCenter(const TopoShape& S)
{
  Standard_Integer i;
  ShapeExplorer  ex;
  Coords3d           xyz(0, 0, 0);
  for (ex.Init(S, TopAbs_VERTEX), i = 0; ex.More(); ex.Next(), i++)
    xyz += BRepInspector::Pnt(TopoDS::Vertex(ex.Current())).XYZ();
  if (i > 0)
  {
    xyz /= i;
  }
  else
  {
    // Try using triangulation
    ex.Init(S, TopAbs_FACE);
    for (; ex.More(); ex.Next())
    {
      const TopoShape&               aF = ex.Current();
      TopLoc_Location                   aLocDummy;
      const Handle(MeshTriangulation)& aTri =
        BRepInspector::Triangulation(TopoDS::Face(aF), aLocDummy);
      if (!aTri.IsNull() && aTri->NbNodes() > 0)
      {
        xyz = aTri->Node(1).XYZ();
        if (!aLocDummy.IsIdentity())
        {
          aLocDummy.Transformation().Transforms(xyz);
        }
        break;
      }
    }
  }
  return Point3d(xyz);
}

void BRepGProp1::LinearProperties(const TopoShape&    S,
                                 GeometricProperties&          SProps,
                                 const Standard_Boolean SkipShared,
                                 const Standard_Boolean UseTriangulation)
{
  // find the origin
  Point3d P(0, 0, 0);
  P.Transform(S.Location());
  SProps = GeometricProperties(P);

  BRepAdaptor_Curve   BAC;
  TopTools_MapOfShape anEMap;
  ShapeExplorer     ex;
  for (ex.Init(S, TopAbs_EDGE); ex.More(); ex.Next())
  {
    const TopoEdge& aE = TopoDS::Edge(ex.Current());
    if (SkipShared && !anEMap.Add(aE))
    {
      continue;
    }

    Handle(PointArray1) theNodes;
    Standard_Boolean            IsGeom = BRepInspector::IsGeometric(aE);
    if (UseTriangulation || !IsGeom)
    {
      BRepGProp_MeshCinert::PreparePolygon(aE, theNodes);
    }
    if (!theNodes.IsNull())
    {
      BRepGProp_MeshCinert MG;
      MG.SetLocation(P);
      MG.Perform(theNodes->Array1());
      SProps.Add(MG);
    }
    else
    {
      if (IsGeom)
      {
        BAC.Initialize(aE);
        BRepGProp_Cinert CG(BAC, P);
        SProps.Add(CG);
      }
    }
  }
}

static Standard_Real surfaceProperties(const TopoShape&    S,
                                       GeometricProperties&          Props,
                                       const Standard_Real    Eps,
                                       const Standard_Boolean SkipShared,
                                       const Standard_Boolean UseTriangulation)
{
  Standard_Integer i;
#ifdef OCCT_DEBUG
  Standard_Integer iErrorMax = 0;
#endif
  Standard_Real    ErrorMax = 0.0, Error;
  ShapeExplorer  ex;
  Point3d           P(roughBaryCenter(S));
  BRepGProp_Sinert G;
  G.SetLocation(P);
  BRepGProp_MeshProps MG(BRepGProp_MeshProps::Sinert);
  MG.SetLocation(P);

  BRepGProp_Face      BF;
  BRepGProp_Domain    BD;
  TopTools_MapOfShape aFMap;
  TopLoc_Location     aLocDummy;

  for (ex.Init(S, TopAbs_FACE), i = 1; ex.More(); ex.Next(), i++)
  {
    const TopoFace& F = TopoDS::Face(ex.Current());
    if (SkipShared && !aFMap.Add(F))
    {
      continue;
    }

    Standard_Boolean NoSurf = Standard_False, NoTri = Standard_False;
    {
      const Handle(GeomSurface)& aSurf = BRepInspector::Surface(F, aLocDummy);
      if (aSurf.IsNull())
      {
        NoSurf = Standard_True;
      }
      const Handle(MeshTriangulation)& aTri = BRepInspector::Triangulation(F, aLocDummy);
      if (aTri.IsNull() || aTri->NbNodes() == 0 || aTri->NbTriangles() == 0)
      {
        NoTri = Standard_True;
      }
      if (NoTri && NoSurf)
      {
        continue;
      }
    }

    if ((UseTriangulation && !NoTri) || (NoSurf && !NoTri))
    {
      TopAbs_Orientation                anOri = F.Orientation();
      const Handle(MeshTriangulation)& aTri  = BRepInspector::Triangulation(F, aLocDummy);
      MG.Perform(aTri, aLocDummy, anOri);
      Props.Add(MG);
    }
    else
    {
      BF.Load(F);
      Standard_Boolean IsNatRestr = (F.NbChildren() == 0);
      if (!IsNatRestr)
        BD.Init(F);
      if (Eps < 1.0)
      {
        G.Perform(BF, BD, Eps);
        Error = G.GetEpsilon();
        if (ErrorMax < Error)
        {
          ErrorMax = Error;
#ifdef OCCT_DEBUG
          iErrorMax = i;
#endif
        }
      }
      else
      {
        if (IsNatRestr)
          G.Perform(BF);
        else
          G.Perform(BF, BD);
      }
      Props.Add(G);
#ifdef OCCT_DEBUG
      if (AffichEps)
        std::cout << "\n" << i << ":\tEpsArea = " << G.GetEpsilon();
#endif
    }
  }
#ifdef OCCT_DEBUG
  if (AffichEps)
    std::cout << "\n-----------------\n" << iErrorMax << ":\tMaxError = " << ErrorMax << "\n";
#endif
  return ErrorMax;
}

void BRepGProp1::SurfaceProperties(const TopoShape&    S,
                                  GeometricProperties&          Props,
                                  const Standard_Boolean SkipShared,
                                  const Standard_Boolean UseTriangulation)
{
  // find the origin
  Point3d P(0, 0, 0);
  P.Transform(S.Location());
  Props = GeometricProperties(P);
  surfaceProperties(S, Props, 1.0, SkipShared, UseTriangulation);
}

Standard_Real BRepGProp1::SurfaceProperties(const TopoShape&    S,
                                           GeometricProperties&          Props,
                                           const Standard_Real    Eps,
                                           const Standard_Boolean SkipShared)
{
  // find the origin
  Point3d P(0, 0, 0);
  P.Transform(S.Location());
  Props                  = GeometricProperties(P);
  Standard_Real ErrorMax = surfaceProperties(S, Props, Eps, SkipShared, Standard_False);
  return ErrorMax;
}

//=================================================================================================

static Standard_Real volumeProperties(const TopoShape&    S,
                                      GeometricProperties&          Props,
                                      const Standard_Real    Eps,
                                      const Standard_Boolean SkipShared,
                                      const Standard_Boolean UseTriangulation)
{
  Standard_Integer i;
#ifdef OCCT_DEBUG
  Standard_Integer iErrorMax = 0;
#endif
  Standard_Real    ErrorMax = 0.0, Error = 0.0;
  ShapeExplorer  ex;
  Point3d           P(roughBaryCenter(S));
  BRepGProp_Vinert G;
  G.SetLocation(P);
  BRepGProp_MeshProps MG(BRepGProp_MeshProps::Vinert);
  MG.SetLocation(P);

  BRepGProp_Face      BF;
  BRepGProp_Domain    BD;
  TopTools_MapOfShape aFwdFMap;
  TopTools_MapOfShape aRvsFMap;
  TopLoc_Location     aLocDummy;

  for (ex.Init(S, TopAbs_FACE), i = 1; ex.More(); ex.Next(), i++)
  {
    const TopoFace& F     = TopoDS::Face(ex.Current());
    TopAbs_Orientation anOri = F.Orientation();
    Standard_Boolean   isFwd = anOri == TopAbs_FORWARD;
    Standard_Boolean   isRvs = Standard_False;
    if (!isFwd)
    {
      isRvs = anOri == TopAbs_REVERSED;
    }
    if (SkipShared)
    {
      if ((isFwd && !aFwdFMap.Add(F)) || (isRvs && !aRvsFMap.Add(F)))
      {
        continue;
      }
    }
    Standard_Boolean NoSurf = Standard_False, NoTri = Standard_False;
    {
      const Handle(GeomSurface)& aSurf = BRepInspector::Surface(F, aLocDummy);
      if (aSurf.IsNull())
      {
        NoSurf = Standard_True;
      }
      const Handle(MeshTriangulation)& aTri = BRepInspector::Triangulation(F, aLocDummy);
      if (aTri.IsNull() || aTri->NbNodes() == 0 || aTri->NbTriangles() == 0)
      {
        NoTri = Standard_True;
      }
      if (NoTri && NoSurf)
      {
        continue;
      }
    }

    if (isFwd || isRvs)
    {
      if ((UseTriangulation && !NoTri) || (NoSurf && !NoTri))
      {
        const Handle(MeshTriangulation)& aTri = BRepInspector::Triangulation(F, aLocDummy);
        MG.Perform(aTri, aLocDummy, anOri);
        Props.Add(MG);
      }
      else
      {
        BF.Load(F);
        Standard_Boolean IsNatRestr = (F.NbChildren() == 0);
        if (!IsNatRestr)
          BD.Init(F);
        if (Eps < 1.0)
        {
          G.Perform(BF, BD, Eps);
          Error = G.GetEpsilon();
          if (ErrorMax < Error)
          {
            ErrorMax = Error;
#ifdef OCCT_DEBUG
            iErrorMax = i;
#endif
          }
        }
        else
        {
          if (IsNatRestr)
            G.Perform(BF);
          else
            G.Perform(BF, BD);
        }
        Props.Add(G);
#ifdef OCCT_DEBUG
        if (AffichEps)
          std::cout << "\n" << i << ":\tEpsVolume = " << G.GetEpsilon();
#endif
      }
    }
  }
#ifdef OCCT_DEBUG
  if (AffichEps)
    std::cout << "\n-----------------\n" << iErrorMax << ":\tMaxError = " << ErrorMax << "\n";
#endif
  return ErrorMax;
}

void BRepGProp1::VolumeProperties(const TopoShape&    S,
                                 GeometricProperties&          Props,
                                 const Standard_Boolean OnlyClosed,
                                 const Standard_Boolean SkipShared,
                                 const Standard_Boolean UseTriangulation)
{
  // find the origin
  Point3d P(0, 0, 0);
  P.Transform(S.Location());
  Props = GeometricProperties(P);
  if (OnlyClosed)
  {
    TopTools_MapOfShape aShMap;
    ShapeExplorer     ex(S, TopAbs_SHELL);
    for (; ex.More(); ex.Next())
    {
      const TopoShape& Sh = ex.Current();
      if (SkipShared && !aShMap.Add(Sh))
      {
        continue;
      }
      if (BRepInspector::IsClosed(Sh))
        volumeProperties(Sh, Props, 1.0, SkipShared, UseTriangulation);
    }
  }
  else
    volumeProperties(S, Props, 1.0, SkipShared, UseTriangulation);
}

//=================================================================================================

Standard_Real BRepGProp1::VolumeProperties(const TopoShape&    S,
                                          GeometricProperties&          Props,
                                          const Standard_Real    Eps,
                                          const Standard_Boolean OnlyClosed,
                                          const Standard_Boolean SkipShared)
{
  // find the origin
  Point3d P(0, 0, 0);
  P.Transform(S.Location());
  Props = GeometricProperties(P);
  Standard_Integer i;
#ifdef OCCT_DEBUG
  Standard_Integer iErrorMax = 0;
#endif
  Standard_Real ErrorMax = 0.0, Error = 0.0;
  if (OnlyClosed)
  {
    TopTools_MapOfShape aShMap;
    ShapeExplorer     ex(S, TopAbs_SHELL);
    for (i = 1; ex.More(); ex.Next(), i++)
    {
      const TopoShape& Sh = ex.Current();
      if (SkipShared && !aShMap.Add(Sh))
      {
        continue;
      }
      if (BRepInspector::IsClosed(Sh))
      {
        Error = volumeProperties(Sh, Props, Eps, SkipShared, Standard_False);
        if (ErrorMax < Error)
        {
          ErrorMax = Error;
#ifdef OCCT_DEBUG
          iErrorMax = i;
#endif
        }
      }
    }
  }
  else
    ErrorMax = volumeProperties(S, Props, Eps, SkipShared, Standard_False);
#ifdef OCCT_DEBUG
  if (AffichEps)
    std::cout << "\n\n===================" << iErrorMax << ":\tMaxEpsVolume = " << ErrorMax << "\n";
#endif
  return ErrorMax;
}

//===========================================================================================//
// Volume properties by Gauss-Kronrod integration
//===========================================================================================//
//=================================================================================================

static Standard_Real volumePropertiesGK(const TopoShape&    theShape,
                                        GeometricProperties&          theProps,
                                        const Standard_Real    theTol,
                                        const Standard_Boolean IsUseSpan,
                                        const Standard_Boolean CGFlag,
                                        const Standard_Boolean IFlag,
                                        const Standard_Boolean SkipShared)
{
  ShapeExplorer anExp;
  anExp.Init(theShape, TopAbs_FACE);

  Standard_Real aTol = theTol;

  // Compute properties.
  Point3d              aLoc(roughBaryCenter(theShape));
  BRepGProp_VinertGK  aVProps;
  BRepGProp_Face      aPropFace(IsUseSpan);
  BRepGProp_Domain    aPropDomain;
  Standard_Real       aLocalError;
  Standard_Real       anError = 0.;
  TopTools_MapOfShape aFwdFMap;
  TopTools_MapOfShape aRvsFMap;
  TopLoc_Location     aLocDummy;

  aVProps.SetLocation(aLoc);

  for (; anExp.More(); anExp.Next())
  {
    TopoFace        aFace = TopoDS::Face(anExp.Current());
    TopAbs_Orientation anOri = aFace.Orientation();
    Standard_Boolean   isFwd = anOri == TopAbs_FORWARD;
    Standard_Boolean   isRvs = Standard_False;
    if (!isFwd)
    {
      isRvs = anOri == TopAbs_REVERSED;
    }
    if (SkipShared)
    {
      if ((isFwd && !aFwdFMap.Add(aFace)) || (isRvs && !aRvsFMap.Add(aFace)))
      {
        continue;
      }
    }
    {
      const Handle(GeomSurface)& aSurf = BRepInspector::Surface(aFace, aLocDummy);
      if (aSurf.IsNull())
      {
        // skip faces without geometry
        continue;
      }
    }

    if (isFwd || isRvs)
    {
      aPropFace.Load(aFace);

      Standard_Boolean IsNatRestr = (aFace.NbChildren() == 0);
      if (IsNatRestr)
        aLocalError = aVProps.Perform(aPropFace, aTol, CGFlag, IFlag);
      else
      {
        aPropDomain.Init(aFace);
        aLocalError = aVProps.Perform(aPropFace, aPropDomain, aTol, CGFlag, IFlag);
      }

      if (aLocalError < 0.)
        return aLocalError;

      anError += aLocalError;
      theProps.Add(aVProps);
    }
  }

  return anError;
}

//=================================================================================================

Standard_Real BRepGProp1::VolumePropertiesGK(const TopoShape&    S,
                                            GeometricProperties&          Props,
                                            const Standard_Real    Eps,
                                            const Standard_Boolean OnlyClosed,
                                            const Standard_Boolean IsUseSpan,
                                            const Standard_Boolean CGFlag,
                                            const Standard_Boolean IFlag,
                                            const Standard_Boolean SkipShared)
{
  Point3d        P(0, 0, 0);
  Standard_Real anError = 0.;

  P.Transform(S.Location());
  Props = GeometricProperties(P);

  if (OnlyClosed)
  {
    // To select closed shells.
    ShapeExplorer      anExp;
    ShapeList aClosedShells;
    TopTools_MapOfShape  aShMap;

    anExp.Init(S, TopAbs_SHELL);

    for (; anExp.More(); anExp.Next())
    {
      const TopoShape& aShell = anExp.Current();
      if (SkipShared && !aShMap.Add(aShell))
      {
        continue;
      }

      BRepCheck_Shell  aChecker(TopoDS::Shell(aShell));
      BRepCheck_Status aStatus = aChecker.Closed(Standard_False);

      if (aStatus == BRepCheck_NoError)
        aClosedShells.Append(aShell);
    }

    if (aClosedShells.IsEmpty())
      return -1.;

    // Compute the properties for each closed shell.
    Standard_Real                      aTol = Eps;
    Standard_Real                      aLocalError;
    TopTools_ListIteratorOfListOfShape anIter(aClosedShells);

    for (; anIter.More(); anIter.Next())
    {
      const TopoShape& aShell = anIter.Value();

      aLocalError = volumePropertiesGK(aShell, Props, aTol, IsUseSpan, CGFlag, IFlag, SkipShared);

      if (aLocalError < 0)
        return aLocalError;

      anError += aLocalError;
    }
  }
  else
    anError = volumePropertiesGK(S, Props, Eps, IsUseSpan, CGFlag, IFlag, SkipShared);

  Standard_Real vol = Props.Mass();
  if (vol > Epsilon(1.))
    anError /= vol;
  return anError;
}

//=================================================================================================

static Standard_Real volumePropertiesGK(const TopoShape&    theShape,
                                        GeometricProperties&          theProps,
                                        const gp_Pln&          thePln,
                                        const Standard_Real    theTol,
                                        const Standard_Boolean IsUseSpan,
                                        const Standard_Boolean CGFlag,
                                        const Standard_Boolean IFlag,
                                        const Standard_Boolean SkipShared)
{
  ShapeExplorer anExp;
  anExp.Init(theShape, TopAbs_FACE);

  Standard_Real aTol = theTol;

  // Compute properties.
  Point3d              aLoc(roughBaryCenter(theShape));
  BRepGProp_VinertGK  aVProps;
  BRepGProp_Face      aPropFace(IsUseSpan);
  BRepGProp_Domain    aPropDomain;
  Standard_Real       aLocalError;
  Standard_Real       anError = 0.;
  TopTools_MapOfShape aFwdFMap;
  TopTools_MapOfShape aRvsFMap;
  TopLoc_Location     aLocDummy;

  aVProps.SetLocation(aLoc);

  for (; anExp.More(); anExp.Next())
  {
    TopoFace        aFace = TopoDS::Face(anExp.Current());
    TopAbs_Orientation anOri = aFace.Orientation();
    Standard_Boolean   isFwd = anOri == TopAbs_FORWARD;
    Standard_Boolean   isRvs = Standard_False;
    if (!isFwd)
    {
      isRvs = anOri == TopAbs_REVERSED;
    }
    if (SkipShared)
    {
      if ((isFwd && !aFwdFMap.Add(aFace)) || (isRvs && !aRvsFMap.Add(aFace)))
      {
        continue;
      }
    }
    {
      const Handle(GeomSurface)& aSurf = BRepInspector::Surface(aFace, aLocDummy);
      if (aSurf.IsNull())
      {
        // skip faces without geometry
        continue;
      }
    }

    if (isFwd || isRvs)
    {
      aPropFace.Load(aFace);

      Standard_Boolean IsNatRestr = (aFace.NbChildren() == 0);
      if (IsNatRestr)
        aLocalError = aVProps.Perform(aPropFace, thePln, aTol, CGFlag, IFlag);
      else
      {
        aPropDomain.Init(aFace);
        aLocalError = aVProps.Perform(aPropFace, aPropDomain, thePln, aTol, CGFlag, IFlag);
      }

      if (aLocalError < 0.)
        return aLocalError;

      anError += aLocalError;
      theProps.Add(aVProps);
    }
  }

  return anError;
}

//=================================================================================================

Standard_Real BRepGProp1::VolumePropertiesGK(const TopoShape&    S,
                                            GeometricProperties&          Props,
                                            const gp_Pln&          thePln,
                                            const Standard_Real    Eps,
                                            const Standard_Boolean OnlyClosed,
                                            const Standard_Boolean IsUseSpan,
                                            const Standard_Boolean CGFlag,
                                            const Standard_Boolean IFlag,
                                            const Standard_Boolean SkipShared)
{
  Point3d        P(0, 0, 0);
  Standard_Real anError = 0.;

  P.Transform(S.Location());
  Props = GeometricProperties(P);

  if (OnlyClosed)
  {
    // To select closed shells.
    ShapeExplorer      anExp;
    ShapeList aClosedShells;
    TopTools_MapOfShape  aShMap;

    anExp.Init(S, TopAbs_SHELL);

    for (; anExp.More(); anExp.Next())
    {
      const TopoShape& aShell = anExp.Current();
      if (SkipShared && !aShMap.Add(aShell))
      {
        continue;
      }

      BRepCheck_Shell  aChecker(TopoDS::Shell(aShell));
      BRepCheck_Status aStatus = aChecker.Closed(Standard_False);

      if (aStatus == BRepCheck_NoError)
        aClosedShells.Append(aShell);
    }

    if (aClosedShells.IsEmpty())
      return -1.;

    // Compute the properties for each closed shell.
    Standard_Real                      aTol = Eps;
    Standard_Real                      aLocalError;
    TopTools_ListIteratorOfListOfShape anIter(aClosedShells);

    for (; anIter.More(); anIter.Next())
    {
      const TopoShape& aShell = anIter.Value();

      aLocalError =
        volumePropertiesGK(aShell, Props, thePln, aTol, IsUseSpan, CGFlag, IFlag, SkipShared);

      if (aLocalError < 0)
        return aLocalError;

      anError += aLocalError;
    }
  }
  else
    anError = volumePropertiesGK(S, Props, thePln, Eps, IsUseSpan, CGFlag, IFlag, SkipShared);

  Standard_Real vol = Props.Mass();
  if (vol > Epsilon(1.))
    anError /= vol;

  return anError;
}
