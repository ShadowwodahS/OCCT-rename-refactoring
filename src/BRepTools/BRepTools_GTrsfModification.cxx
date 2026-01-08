// Created on: 1996-12-30
// Created by: Stagiaire Mary FABIEN
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

#include <BRep_Tool.hxx>
#include <BRepTools_GTrsfModification.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Surface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomLib.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Pnt.hxx>
#include <gp_Quaternion.hxx>
#include <gp_XYZ.hxx>
#include <Standard_NoSuchObject.hxx>
#include <Standard_Type.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(BRepTools_GTrsfModification, BRepTools_Modification)

//=================================================================================================

BRepTools_GTrsfModification::BRepTools_GTrsfModification(const GeneralTransform& T)
    : myGTrsf(T)
{
  // on prend comme dilatation maximale pour la tolerance la norme sup
  Standard_Real loc1, loc2, loc3, loc4;

  loc1 = Max(Abs(T.Value(1, 1)), Abs(T.Value(1, 2)));
  loc2 = Max(Abs(T.Value(2, 1)), Abs(T.Value(2, 2)));
  loc3 = Max(Abs(T.Value(3, 1)), Abs(T.Value(3, 2)));
  loc4 = Max(Abs(T.Value(1, 3)), Abs(T.Value(2, 3)));

  loc1 = Max(loc1, loc2);
  loc2 = Max(loc3, loc4);

  loc1 = Max(loc1, loc2);

  myGScale = Max(loc1, Abs(T.Value(3, 3)));
}

//=================================================================================================

GeneralTransform& BRepTools_GTrsfModification::GTrsf()
{
  return myGTrsf;
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewSurface(const TopoFace&    F,
                                                         Handle(GeomSurface)& S,
                                                         TopLoc_Location&      L,
                                                         Standard_Real&        Tol,
                                                         Standard_Boolean&     RevWires,
                                                         Standard_Boolean&     RevFace)
{
  GeneralTransform gtrsf;
  gtrsf.SetVectorialPart(myGTrsf.VectorialPart());
  gtrsf.SetTranslationPart(myGTrsf.TranslationPart());
  S = BRepInspector::Surface(F, L);
  if (S.IsNull())
  {
    // processing the case when there is no geometry
    return Standard_False;
  }
  S = Handle(GeomSurface)::DownCast(S->Copy());

  Tol = BRepInspector::Tolerance(F);
  Tol *= myGScale;
  RevWires = Standard_False;
  RevFace  = myGTrsf.IsNegative();
  S        = Handle(GeomSurface)::DownCast(S->Transformed(L.Transformation()));

  Handle(TypeInfo) TheTypeS = S->DynamicType();
  if (TheTypeS == STANDARD_TYPE(Geom_BSplineSurface))
  {
    Handle(Geom_BSplineSurface) S2 = Handle(Geom_BSplineSurface)::DownCast(S);
    for (Standard_Integer i = 1; i <= S2->NbUPoles(); i++)
      for (Standard_Integer j = 1; j <= S2->NbVPoles(); j++)
      {
        Coords3d coor(S2->Pole(i, j).Coord());
        gtrsf.Transforms(coor);
        Point3d P(coor);
        S2->SetPole(i, j, P);
      }
  }
  else if (TheTypeS == STANDARD_TYPE(Geom_BezierSurface))
  {
    Handle(Geom_BezierSurface) S2 = Handle(Geom_BezierSurface)::DownCast(S);
    for (Standard_Integer i = 1; i <= S2->NbUPoles(); i++)
      for (Standard_Integer j = 1; j <= S2->NbVPoles(); j++)
      {
        Coords3d coor(S2->Pole(i, j).Coord());
        gtrsf.Transforms(coor);
        Point3d P(coor);
        S2->SetPole(i, j, P);
      }
  }
  else
  {
    throw Standard_NoSuchObject("BRepTools_GTrsfModification : Pb no BSpline/Bezier Type Surface");
  }

  L.Identity();
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewCurve(const TopoEdge&  E,
                                                       Handle(GeomCurve3d)& C,
                                                       TopLoc_Location&    L,
                                                       Standard_Real&      Tol)
{
  Standard_Real f, l;
  GeneralTransform      gtrsf;
  gtrsf.SetVectorialPart(myGTrsf.VectorialPart());
  gtrsf.SetTranslationPart(myGTrsf.TranslationPart());
  Tol = BRepInspector::Tolerance(E) * myGScale;
  C   = BRepInspector::Curve(E, L, f, l);

  if (!C.IsNull())
  {
    C = Handle(GeomCurve3d)::DownCast(C->Copy()->Transformed(L.Transformation()));
    Handle(TypeInfo) TheTypeC = C->DynamicType();
    if (TheTypeC == STANDARD_TYPE(BSplineCurve3d))
    {
      Handle(BSplineCurve3d) C2 = Handle(BSplineCurve3d)::DownCast(C);
      for (Standard_Integer i = 1; i <= C2->NbPoles(); i++)
      {
        Coords3d coor(C2->Pole(i).Coord());
        gtrsf.Transforms(coor);
        Point3d P(coor);
        C2->SetPole(i, P);
      }
    }
    else if (TheTypeC == STANDARD_TYPE(BezierCurve3d))
    {
      Handle(BezierCurve3d) C2 = Handle(BezierCurve3d)::DownCast(C);
      for (Standard_Integer i = 1; i <= C2->NbPoles(); i++)
      {
        Coords3d coor(C2->Pole(i).Coord());
        gtrsf.Transforms(coor);
        Point3d P(coor);
        C2->SetPole(i, P);
      }
    }
    else
    {
      throw Standard_NoSuchObject("BRepTools_GTrsfModification : Pb no BSpline/Bezier Type Curve");
    }
    C = new Geom_TrimmedCurve(C, f, l);
  }
  L.Identity();
  return !C.IsNull();
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewPoint(const TopoVertex& V,
                                                       Point3d&              P,
                                                       Standard_Real&       Tol)
{
  Point3d Pnt = BRepInspector::Pnt(V);
  Tol        = BRepInspector::Tolerance(V);
  Tol *= myGScale;
  Coords3d coor(Pnt.Coord());
  myGTrsf.Transforms(coor);
  P.SetXYZ(coor);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewCurve2d(const TopoEdge& E,
                                                         const TopoFace& F,
                                                         const TopoEdge&,
                                                         const TopoFace&,
                                                         Handle(GeomCurve2d)& C,
                                                         Standard_Real&        Tol)
{
  TopLoc_Location loc;
  Tol = BRepInspector::Tolerance(E);
  Tol *= myGScale;
  Standard_Real f, l;
  C = BRepInspector::CurveOnSurface(E, F, f, l);
  if (C.IsNull())
  {
    // processing the case when there is no geometry
    return Standard_False;
  }
  C = new Geom2d_TrimmedCurve(C, f, l);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewParameter(const TopoVertex& V,
                                                           const TopoEdge&   E,
                                                           Standard_Real&       P,
                                                           Standard_Real&       Tol)
{
  Tol = BRepInspector::Tolerance(V);
  Tol *= myGScale;
  P = BRepInspector::Parameter(V, E);
  return Standard_True;
}

//=================================================================================================

GeomAbs_Shape BRepTools_GTrsfModification::Continuity(const TopoEdge& E,
                                                      const TopoFace& F1,
                                                      const TopoFace& F2,
                                                      const TopoEdge&,
                                                      const TopoFace&,
                                                      const TopoFace&)
{
  return BRepInspector::Continuity(E, F1, F2);
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewTriangulation(
  const TopoFace&          theFace,
  Handle(MeshTriangulation)& theTriangulation)
{
  TopLoc_Location aLoc;
  theTriangulation = BRepInspector::Triangulation(theFace, aLoc);
  if (theTriangulation.IsNull())
  {
    return Standard_False;
  }

  GeneralTransform aGTrsf;
  aGTrsf.SetVectorialPart(myGTrsf.VectorialPart());
  aGTrsf.SetTranslationPart(myGTrsf.TranslationPart());
  aGTrsf.Multiply(aLoc.Transformation());

  theTriangulation = theTriangulation->Copy();
  theTriangulation->SetCachedMinMax(Box2()); // clear bounding box
  theTriangulation->Deflection(theTriangulation->Deflection() * Abs(myGScale));
  // apply transformation to 3D nodes
  for (Standard_Integer anInd = 1; anInd <= theTriangulation->NbNodes(); ++anInd)
  {
    Point3d aP = theTriangulation->Node(anInd);
    aGTrsf.Transforms(aP.ChangeCoord());
    theTriangulation->SetNode(anInd, aP);
  }
  // modify triangles orientation in case of mirror transformation
  if (myGScale < 0.0)
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
      gp_Mat aMat    = aGTrsf.VectorialPart();
      aMat.SetDiagonal(1., 1., 1.);
      Transform3d aTrsf;
      aTrsf.SetForm(gp_Rotation);
      (gp_Mat&)aTrsf.HVectorialPart() = aMat;
      aNormal.Transform(aTrsf);
      theTriangulation->SetNormal(anInd, aNormal);
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewPolygon(const TopoEdge&      theEdge,
                                                         Handle(Poly_Polygon3D)& thePoly)
{
  TopLoc_Location aLoc;
  thePoly = BRepInspector::Polygon3D(theEdge, aLoc);
  if (thePoly.IsNull())
  {
    return Standard_False;
  }

  GeneralTransform aGTrsf;
  aGTrsf.SetVectorialPart(myGTrsf.VectorialPart());
  aGTrsf.SetTranslationPart(myGTrsf.TranslationPart());
  aGTrsf.Multiply(aLoc.Transformation());

  thePoly = thePoly->Copy();
  thePoly->Deflection(thePoly->Deflection() * Abs(myGScale));
  // transform nodes
  TColgp_Array1OfPnt& aNodesArray = thePoly->ChangeNodes();
  for (Standard_Integer anId = aNodesArray.Lower(); anId <= aNodesArray.Upper(); ++anId)
  {
    Point3d& aP = aNodesArray.ChangeValue(anId);
    aGTrsf.Transforms(aP.ChangeCoord());
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepTools_GTrsfModification::NewPolygonOnTriangulation(
  const TopoEdge&                   theEdge,
  const TopoFace&                   theFace,
  Handle(Poly_PolygonOnTriangulation)& thePoly)
{
  TopLoc_Location            aLoc;
  Handle(MeshTriangulation) aT = BRepInspector::Triangulation(theFace, aLoc);
  if (aT.IsNull())
  {
    return Standard_False;
  }

  thePoly = BRepInspector::PolygonOnTriangulation(theEdge, aT, aLoc);
  if (!thePoly.IsNull())
    thePoly = thePoly->Copy();
  return Standard_True;
}
