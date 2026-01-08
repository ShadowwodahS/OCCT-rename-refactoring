// Created on: 1994-08-25
// Created by: Jacques GOUSSARD
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

#include <BRep_Tool.hxx>
#include <BRepTools_TrsfModification.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomLib.hxx>
#include <GeomLib_Tool.hxx>
#include <gp_GTrsf2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_TrsfForm.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepTools_TrsfModification, BRepTools_Modification)

//=================================================================================================

BRepTools_TrsfModification::BRepTools_TrsfModification(const Transform3d& T)
    : myTrsf(T),
      myCopyMesh(Standard_False)
{
}

//=================================================================================================

Transform3d& BRepTools_TrsfModification::Trsf()
{
  return myTrsf;
}

//=================================================================================================

Standard_Boolean& BRepTools_TrsfModification::IsCopyMesh()
{
  return myCopyMesh;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewSurface(const TopoFace&    F,
                                                        Handle(GeomSurface)& S,
                                                        TopLoc_Location&      L,
                                                        Standard_Real&        Tol,
                                                        Standard_Boolean&     RevWires,
                                                        Standard_Boolean&     RevFace)
{
  S = BRepInspector::Surface(F, L);
  if (S.IsNull())
  {
    // processing cases when there is no geometry
    return Standard_False;
  }

  Tol = BRepInspector::Tolerance(F);
  Tol *= Abs(myTrsf.ScaleFactor());
  RevWires = Standard_False;
  RevFace  = myTrsf.IsNegative();

  Transform3d LT = L.Transformation();
  LT.Invert();
  LT.Multiply(myTrsf);
  LT.Multiply(L.Transformation());

  S = Handle(GeomSurface)::DownCast(S->Transformed(LT));

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewTriangulation(
  const TopoFace&          theFace,
  Handle(MeshTriangulation)& theTriangulation)
{
  if (!myCopyMesh)
  {
    return Standard_False;
  }

  TopLoc_Location aLoc;
  theTriangulation = BRepInspector::Triangulation(theFace, aLoc);

  if (theTriangulation.IsNull())
  {
    return Standard_False;
  }

  Transform3d aTrsf = myTrsf;
  if (!aLoc.IsIdentity())
  {
    aTrsf = aLoc.Transformation().Inverted() * aTrsf * aLoc.Transformation();
  }

  theTriangulation = theTriangulation->Copy();
  theTriangulation->SetCachedMinMax(Box2()); // clear bounding box
  theTriangulation->Deflection(theTriangulation->Deflection() * Abs(myTrsf.ScaleFactor()));
  // apply transformation to 3D nodes
  for (Standard_Integer anInd = 1; anInd <= theTriangulation->NbNodes(); ++anInd)
  {
    Point3d aP = theTriangulation->Node(anInd);
    aP.Transform(aTrsf);
    theTriangulation->SetNode(anInd, aP);
  }
  // modify 2D nodes
  Handle(GeomSurface) aSurf = BRepInspector::Surface(theFace, aLoc);
  if (theTriangulation->HasUVNodes() && !aSurf.IsNull())
  {
    for (Standard_Integer anInd = 1; anInd <= theTriangulation->NbNodes(); ++anInd)
    {
      gp_Pnt2d aP2d = theTriangulation->UVNode(anInd);
      aSurf->TransformParameters(aP2d.ChangeCoord().ChangeCoord(1),
                                 aP2d.ChangeCoord().ChangeCoord(2),
                                 myTrsf);
      theTriangulation->SetUVNode(anInd, aP2d);
    }
  }
  // modify triangles orientation in case of mirror transformation
  if (myTrsf.ScaleFactor() < 0.0)
  {
    for (Standard_Integer anInd = 1; anInd <= theTriangulation->NbTriangles(); ++anInd)
    {
      Triangle2    aTria = theTriangulation->Triangle1(anInd);
      Standard_Integer aN1, aN2, aN3;
      aTria.Get(aN1, aN2, aN3);
      aTria.Set(aN1, aN3, aN2);
      theTriangulation->SetTriangle(anInd, aTria);
    }
  }
  // modify normals
  if (theTriangulation->HasNormals())
  {
    for (Standard_Integer anInd = 1; anInd <= theTriangulation->NbNodes(); ++anInd)
    {
      Dir3d aNormal = theTriangulation->Normal(anInd);
      aNormal.Transform(aTrsf);
      theTriangulation->SetNormal(anInd, aNormal);
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewPolygon(const TopoEdge&      theE,
                                                        Handle(Poly_Polygon3D)& theP)
{
  if (!myCopyMesh)
  {
    return Standard_False;
  }

  TopLoc_Location aLoc;
  theP = BRepInspector::Polygon3D(theE, aLoc);
  if (theP.IsNull())
  {
    return Standard_False;
  }

  Transform3d aTrsf = myTrsf;
  if (!aLoc.IsIdentity())
  {
    aTrsf = aLoc.Transformation().Inverted() * aTrsf * aLoc.Transformation();
  }

  theP = theP->Copy();
  theP->Deflection(theP->Deflection() * Abs(myTrsf.ScaleFactor()));
  TColgp_Array1OfPnt& aNodesArray = theP->ChangeNodes();
  for (Standard_Integer anId = aNodesArray.Lower(); anId <= aNodesArray.Upper(); ++anId)
  {
    // Applying the transformation to each node of polygon
    aNodesArray.ChangeValue(anId).Transform(aTrsf);
  }
  // transform the parametrization
  if (theP->HasParameters())
  {
    TopLoc_Location    aCurveLoc;
    Standard_Real      aFirst, aLast;
    Handle(GeomCurve3d) aCurve = BRepInspector::Curve(theE, aCurveLoc, aFirst, aLast);
    if (!aCurve.IsNull())
    {
      Standard_Real aReparametrization = aCurve->ParametricTransformation(aTrsf);
      if (Abs(aReparametrization - 1.0) > Precision::PConfusion())
      {
        TColStd_Array1OfReal& aParams = theP->ChangeParameters();
        for (Standard_Integer anInd = aParams.Lower(); anInd <= aParams.Upper(); ++anInd)
        {
          aParams(anInd) *= aReparametrization;
        }
      }
    }
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewPolygonOnTriangulation(
  const TopoEdge&                   theE,
  const TopoFace&                   theF,
  Handle(Poly_PolygonOnTriangulation)& theP)
{
  if (!myCopyMesh)
  {
    return Standard_False;
  }

  TopLoc_Location            aLoc;
  Handle(MeshTriangulation) aT = BRepInspector::Triangulation(theF, aLoc);
  if (aT.IsNull())
  {
    theP = Handle(Poly_PolygonOnTriangulation)();
    return Standard_False;
  }

  theP = BRepInspector::PolygonOnTriangulation(theE, aT, aLoc);
  if (theP.IsNull())
  {
    return Standard_False;
  }
  theP = theP->Copy();
  theP->Deflection(theP->Deflection() * Abs(myTrsf.ScaleFactor()));

  // transform the parametrization
  Handle(GeomSurface) aSurf = BRepInspector::Surface(theF, aLoc);
  Standard_Real        aFirst, aLast;
  Handle(GeomCurve2d) aC2d = BRepInspector::CurveOnSurface(theE, theF, aFirst, aLast);
  if (!aSurf.IsNull() && !aC2d.IsNull()
      && Abs(Abs(myTrsf.ScaleFactor()) - 1.0) > TopLoc_Location::ScalePrec())
  {
    GeneralTransform2d aGTrsf = aSurf->ParametricTransformation(myTrsf);
    if (aGTrsf.Form() != gp_Identity)
    {
      Handle(GeomCurve2d) aNewC2d = GeomLib1::GTransform(aC2d, aGTrsf);
      for (Standard_Integer anInd = 1; anInd <= theP->NbNodes(); ++anInd)
      {
        Standard_Real aParam = theP->Parameter(anInd);
        gp_Pnt2d      aP2d   = aC2d->Value(aParam);
        aGTrsf.Transforms(aP2d.ChangeCoord());
        Tool2::Parameter(aNewC2d, aP2d, theP->Deflection(), aParam);
        theP->SetParameter(anInd, aParam);
      }
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewCurve(const TopoEdge&  E,
                                                      Handle(GeomCurve3d)& C,
                                                      TopLoc_Location&    L,
                                                      Standard_Real&      Tol)
{
  Standard_Real f, l;
  C = BRepInspector::Curve(E, L, f, l);

  Tol = BRepInspector::Tolerance(E);
  Tol *= Abs(myTrsf.ScaleFactor());

  Transform3d LT = L.Transformation();
  LT.Invert();
  LT.Multiply(myTrsf);
  LT.Multiply(L.Transformation());

  if (!C.IsNull())
  {
    C = Handle(GeomCurve3d)::DownCast(C->Transformed(LT));
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewPoint(const TopoVertex& V,
                                                      Point3d&              P,
                                                      Standard_Real&       Tol)
{
  P   = BRepInspector::Pnt(V);
  Tol = BRepInspector::Tolerance(V);
  Tol *= Abs(myTrsf.ScaleFactor());
  P.Transform(myTrsf);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewCurve2d(const TopoEdge& E,
                                                        const TopoFace& F,
                                                        const TopoEdge&,
                                                        const TopoFace&,
                                                        Handle(GeomCurve2d)& C,
                                                        Standard_Real&        Tol)
{
  TopLoc_Location loc;
  Tol                 = BRepInspector::Tolerance(E);
  Standard_Real scale = myTrsf.ScaleFactor();
  Tol *= Abs(scale);
  const Handle(GeomSurface)& S = BRepInspector::Surface(F, loc);

  if (S.IsNull())
  {
    // processing the case when the surface (geometry) is deleted
    return Standard_False;
  }
  GeomAdaptor_Surface GAsurf(S);
  if (GAsurf.GetType() == GeomAbs_Plane)
    return Standard_False;

  Standard_Real        f, l;
  Handle(GeomCurve2d) NewC = BRepInspector::CurveOnSurface(E, F, f, l);
  if (NewC.IsNull())
    return Standard_False;

  Standard_Real newf, newl;

  Handle(TypeInfo) TheType = NewC->DynamicType();

  if (TheType == STANDARD_TYPE(Geom2d_TrimmedCurve))
  {
    Handle(Geom2d_TrimmedCurve) TC = Handle(Geom2d_TrimmedCurve)::DownCast(NewC);
    NewC                           = TC->BasisCurve();
  }

  Standard_Real fc = NewC->FirstParameter(), lc = NewC->LastParameter();

  if (!NewC->IsPeriodic())
  {
    if (fc - f > Precision::PConfusion())
      f = fc;
    if (l - lc > Precision::PConfusion())
      l = lc;
    if (Abs(l - f) < Precision::PConfusion())
    {
      if (Abs(f - fc) < Precision::PConfusion() && !Precision::IsInfinite(lc))
      {
        l = lc;
      }
      else if (!Precision::IsInfinite(fc))
      {
        f = fc;
      }
    }
  }

  newf = f;
  newl = l;
  if (Abs(scale) != 1.)
  {

    NewC             = new Geom2d_TrimmedCurve(NewC, f, l);
    GeneralTransform2d gtrsf = S->ParametricTransformation(myTrsf);

    if (gtrsf.Form() != gp_Identity)
    {
      NewC = GeomLib1::GTransform(NewC, gtrsf);
      if (NewC.IsNull())
      {
        throw Standard_DomainError("TrsfModification:Error in NewCurve2d");
      }
      newf = NewC->FirstParameter();
      newl = NewC->LastParameter();
    }
  }
  // il faut parfois recadrer les ranges 3d / 2d
  TopoVertex V1, V2;
  TopExp1::Vertices(E, V1, V2);
  TopoShape  initEFOR = E.Oriented(TopAbs_FORWARD);                            // skl
  TopoEdge   EFOR     = TopoDS::Edge(initEFOR /*E.Oriented(TopAbs_FORWARD)*/); // skl
  Standard_Real aTolV;
  NewParameter(V1, EFOR, f, aTolV);
  NewParameter(V2, EFOR, l, aTolV);
  GeomLib1::SameRange(Precision::PConfusion(), NewC, newf, newl, f, l, C);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_TrsfModification::NewParameter(const TopoVertex& V,
                                                          const TopoEdge&   E,
                                                          Standard_Real&       P,
                                                          Standard_Real&       Tol)
{
  if (V.IsNull())
    return Standard_False; // infinite edge may have Null vertex

  TopLoc_Location loc;
  Tol = BRepInspector::Tolerance(V);
  Tol *= Abs(myTrsf.ScaleFactor());
  P = BRepInspector::Parameter(V, E);

  Standard_Real f, l;

  Handle(GeomCurve3d) C = BRepInspector::Curve(E, loc, f, l);
  if (!C.IsNull())
  {
    P = C->TransformedParameter(P, myTrsf);
  }

  return Standard_True;
}

//=================================================================================================

GeomAbs_Shape BRepTools_TrsfModification::Continuity(const TopoEdge& E,
                                                     const TopoFace& F1,
                                                     const TopoFace& F2,
                                                     const TopoEdge&,
                                                     const TopoFace&,
                                                     const TopoFace&)
{
  return BRepInspector::Continuity(E, F1, F2);
}
