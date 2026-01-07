// Created on: 1998-11-26
// Created by: Jean-Michel BOULCOURT
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

// Modif : Wed Jan 20 15:40:50 1999. In BuildListResultEdges, we
// in UpdatePcurve, problem with location of pcurve (mix between loc and locbid)
// Modif : Thu Jan 21 11:40:20 1999. Add trace context #if DEB
//           add test to avoid loop while in NextConnexEdge (in case of a closed connex wire)

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepLib.hxx>
#include <BRepLib_MakeEdge.hxx>
#include <BRepTools_Substitution.hxx>
#include <ElCLib.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dConvert_CompCurveToBSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BoundedCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomLib.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_NullObject.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopOpeBRepTool_FuseEdges.hxx>
#include <TopTools_DataMapOfIntegerListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>

#ifdef OCCT_DEBUG
extern Standard_Boolean TopOpeBRepBuild_GettraceFE();
#endif

//=================================================================================================

TopOpeBRepTool_FuseEdges::TopOpeBRepTool_FuseEdges(
  const TopoShape& theShape,
  //                                                   const Standard_Boolean PerformNow)
  const Standard_Boolean)
    : myShape(theShape),
      myShapeDone(Standard_False),
      myEdgesDone(Standard_False),
      myResultEdgesDone(Standard_False),
      myNbConnexEdge(0)
{
  //  if (theShape.ShapeType() != TopAbs_SHELL && theShape.ShapeType() != TopAbs_SOLID)
  //    throw Standard_ConstructionError("FuseEdges");
  Standard_NullObject_Raise_if(theShape.IsNull(), "FuseEdges");
  myMapFaces.Clear();
}

//=======================================================================
// function : AvoidEdges
// purpose  : set edges to avoid being fused
//=======================================================================

void TopOpeBRepTool_FuseEdges::AvoidEdges(const TopTools_IndexedMapOfShape& theMapEdg)
{
  myAvoidEdg = theMapEdg;
}

//=======================================================================
// function : Edges
// purpose  : returns  all the list of edges to be fused each list of the
//           map represent a set of connex edges that can be fused.
//=======================================================================

void TopOpeBRepTool_FuseEdges::Edges(TopTools_DataMapOfIntegerListOfShape& theMapLstEdg)
{

  if (!myEdgesDone)
  {
    BuildListEdges();
  }

  theMapLstEdg = myMapLstEdg;
}

//=======================================================================
// function : ResultEdges
// purpose  : returns  all the fused edges
//=======================================================================

void TopOpeBRepTool_FuseEdges::ResultEdges(TopTools_DataMapOfIntegerShape& theMapEdg)
{

  if (!myEdgesDone)
  {
    BuildListEdges();
  }

  if (!myResultEdgesDone)
  {
    BuildListResultEdges();
  }

  theMapEdg = myMapEdg;
}

//=======================================================================
// function : Faces
// purpose  : returns  all the faces that have been modified after perform
//=======================================================================

void TopOpeBRepTool_FuseEdges::Faces(TopTools_DataMapOfShapeShape& theMapFac)
{

  if (!myEdgesDone)
  {
    BuildListEdges();
  }

  if (!myResultEdgesDone)
  {
    BuildListResultEdges();
  }

  if (!myShapeDone)
  {
    Perform();
  }

  theMapFac = myMapFaces;
}

//=================================================================================================

Standard_Integer TopOpeBRepTool_FuseEdges::NbVertices()
{

  Standard_NullObject_Raise_if(myShape.IsNull(), "FuseEdges : No Shape");
  Standard_Integer nbedges, nbvertices = 0;

  if (!myEdgesDone)
  {
    BuildListEdges();
  }

  if ((nbedges = myMapLstEdg.Extent()) > 0)
  {

    TopTools_DataMapIteratorOfDataMapOfIntegerListOfShape itEdg;
    for (itEdg.Initialize(myMapLstEdg); itEdg.More(); itEdg.Next())
    {
      const Standard_Integer&     iLst    = itEdg.Key();
      const ShapeList& LmapEdg = myMapLstEdg.Find(iLst);
      nbvertices += LmapEdg.Extent() - 1;
    }
  }

  return nbvertices;
}

//=================================================================================================

TopoShape& TopOpeBRepTool_FuseEdges::Shape()
{
  Standard_NullObject_Raise_if(myShape.IsNull(), "FuseEdges : No Shape");

  if (!myEdgesDone)
  {
    BuildListEdges();
  }

  if (!myResultEdgesDone)
  {
    BuildListResultEdges();
  }

  if (!myShapeDone)
  {
    Perform();
  }

  return myShape;
}

//=======================================================================
// function : BuildListEdges
// purpose  : Build the all the lists of edges that are to be fused
//=======================================================================

void TopOpeBRepTool_FuseEdges::BuildListEdges()
{

#ifdef OCCT_DEBUG
  Standard_Boolean tFE = TopOpeBRepBuild_GettraceFE();
#endif

#ifdef OCCT_DEBUG
  if (tFE)
    std::cout << std::endl << "FuseEdges : BuildListEdges  " << std::endl;
#endif

  //--------------------------------------------------------
  // Step One : Build the map ancestors
  //--------------------------------------------------------

  // Clear the maps
  myMapLstEdg.Clear();
  myMapVerLstEdg.Clear();
  myMapEdgLstFac.Clear();

  TopExp1::MapShapesAndUniqueAncestors(myShape, TopAbs_VERTEX, TopAbs_EDGE, myMapVerLstEdg);
  TopExp1::MapShapesAndAncestors(myShape, TopAbs_EDGE, TopAbs_FACE, myMapEdgLstFac);

  Standard_Integer    iEdg;
  TopTools_MapOfShape mapUniqEdg;

  // for each edge of myMapEdgLstFac
  for (iEdg = 1; iEdg <= myMapEdgLstFac.Extent(); iEdg++)
  {
    const TopoShape&  edgecur = myMapEdgLstFac.FindKey(iEdg);
    ShapeList LstEdg;

    // if edge not already treated
    if (!mapUniqEdg.Contains(edgecur)
        && (edgecur.Orientation() == TopAbs_FORWARD || edgecur.Orientation() == TopAbs_REVERSED))
    {
      if (myAvoidEdg.Contains(edgecur))
        continue; // edge is not allowed to be fused
      BuildListConnexEdge(edgecur, mapUniqEdg, LstEdg);
      if (LstEdg.Extent() > 1)
      {
        myNbConnexEdge++;
        myMapLstEdg.Bind(myNbConnexEdge, LstEdg);
      }
    }
  }

  myEdgesDone       = Standard_True;
  myResultEdgesDone = Standard_False;
}

//=======================================================================
// function : BuildListResultEdges
// purpose  : Build the result fused edges
//=======================================================================

void TopOpeBRepTool_FuseEdges::BuildListResultEdges()
{

#ifdef OCCT_DEBUG
  Standard_Boolean tFE = TopOpeBRepBuild_GettraceFE();
#endif

#ifdef OCCT_DEBUG
  if (tFE)
    std::cout << std::endl << "FuseEdges : BuildListResultEdges  " << std::endl;
#endif

  // if we have edges to fuse
  if (myMapLstEdg.Extent() > 0)
  {
    TopTools_DataMapIteratorOfDataMapOfIntegerListOfShape itLstEdg;
    TopoVertex                                         VF, VL;
    Handle(GeomCurve3d)                                    C;
    TopLoc_Location                                       loc;
    Standard_Real                                         f, l;
    TopoEdge                                           NewEdge;

    myMapEdg.Clear();

    for (itLstEdg.Initialize(myMapLstEdg); itLstEdg.More(); itLstEdg.Next())
    {
      const Standard_Integer&     iLst    = itLstEdg.Key();
      const ShapeList& LmapEdg = myMapLstEdg.Find(iLst);
#ifdef OCCT_DEBUG
      TopTools_ListIteratorOfListOfShape itEdg;
#endif

      const TopoEdge& OldEdge = TopoDS::Edge(LmapEdg.First());

      // the first edge of the list will be replaced by the result fusion edge
      if (OldEdge.Orientation() == TopAbs_REVERSED)
      {
        VL = TopExp1::FirstVertex(TopoDS::Edge(LmapEdg.First()), Standard_True);
        VF = TopExp1::LastVertex(TopoDS::Edge(LmapEdg.Last()), Standard_True);
      }
      else
      {
        VF = TopExp1::FirstVertex(TopoDS::Edge(LmapEdg.First()), Standard_True);
        VL = TopExp1::LastVertex(TopoDS::Edge(LmapEdg.Last()), Standard_True);
      }
      C = BRepInspector::Curve(OldEdge, loc, f, l);

      if (!loc.IsIdentity())
      {
        C = Handle(GeomCurve3d)::DownCast(C->Transformed(loc.Transformation()));
      }
      // if the curve is trimmed we get the basis curve to fit the new vertices
      // otherwise the makeedge will fail.
      if (C->DynamicType() == STANDARD_TYPE(Geom_TrimmedCurve))
      {
        C = Handle(Geom_TrimmedCurve)::DownCast(C)->BasisCurve();
      }

#ifdef OCCT_DEBUG
      if (tFE)
        std::cout << std::endl << "FuseEdges : Creating New Edge " << std::endl;
#endif

      BRepLib_MakeEdge ME(C, VF, VL);

      if (!ME.IsDone())
      {
        // the MakeEdge has fails, one reason could be that the new Vertices are outside
        // the curve which is not infinite and limited to old vertices
        // we try to use ExtendCurveToPoint, then rebuild the NewEdge

#ifdef OCCT_DEBUG
        if (tFE)
          std::cout << std::endl
                    << "FuseEdges : MakeEdge failed. Trying to Extend Curve " << std::endl;
#endif
        Handle(Geom_BoundedCurve) ExtC = Handle(Geom_BoundedCurve)::DownCast(C->Copy());
        if (!ExtC.IsNull())
        {
          Point3d PF = BRepInspector::Pnt(VF);
          Point3d PL = BRepInspector::Pnt(VL);
          GeomLib::ExtendCurveToPoint(ExtC, PF, 1, 0);
          GeomLib::ExtendCurveToPoint(ExtC, PL, 1, 1);

          ME.Init(ExtC, VF, VL);
          if (!ME.IsDone())
            throw Standard_ConstructionError("FuseEdges : Fusion failed");
        }
        else
          throw Standard_ConstructionError("FuseEdges : Fusion failed");
      }

      NewEdge = ME.Edge();

#ifdef OCCT_DEBUG
      if (tFE)
        std::cout << std::endl << "FuseEdges : Updating pcurve " << std::endl;
#endif
      if (UpdatePCurve(OldEdge, NewEdge, LmapEdg))
        myMapEdg.Bind(iLst, NewEdge);
    }

    myResultEdgesDone = Standard_True;
  }
}

//=================================================================================================

void TopOpeBRepTool_FuseEdges::Perform()
{

#ifdef OCCT_DEBUG
  Standard_Boolean tFE = TopOpeBRepBuild_GettraceFE();
#endif

  if (!myResultEdgesDone)
  {
    BuildListResultEdges();
  }

#ifdef OCCT_DEBUG
  if (tFE)
    std::cout << std::endl << "FuseEdges : Perform  " << std::endl;
#endif

  // if we have fused edges
  if (myMapEdg.Extent() > 0)
  {
    TopTools_DataMapIteratorOfDataMapOfIntegerListOfShape itLstEdg;
    ShapeList                                  EmptyList, EdgeToSubs;
    ShapeSubstitution                                Bsub;

    for (itLstEdg.Initialize(myMapLstEdg); itLstEdg.More(); itLstEdg.Next())
    {
      const Standard_Integer& iLst = itLstEdg.Key();
      if (!myMapEdg.IsBound(iLst))
        continue;
      const ShapeList&        LmapEdg = myMapLstEdg.Find(iLst);
      TopTools_ListIteratorOfListOfShape itEdg;

      EdgeToSubs.Clear();
      const TopoEdge& OldEdge = TopoDS::Edge(LmapEdg.First());

      EdgeToSubs.Append(myMapEdg(iLst));
      Bsub.Substitute(OldEdge, EdgeToSubs);

      itEdg.Initialize(LmapEdg);

      // the other edges of the list will be removed
      while (itEdg.More())
      {
        if (!OldEdge.IsSame(TopoDS::Edge(itEdg.Value())))
        {
          Bsub.Substitute(itEdg.Value(), EmptyList);
        }
        itEdg.Next();
      }
    }

#ifdef OCCT_DEBUG
    if (tFE)
      std::cout << std::endl << "FuseEdges : Building New Shape  " << std::endl;
#endif

    // perform the effective substitution
    Bsub.Build(myShape);

    // before copying the resulting shape, map the modified faces into myMapFaces
    ShapeExplorer exp(myShape, TopAbs_FACE);

    for (; exp.More(); exp.Next())
    {
      const TopoShape& facecur = exp.Current();
      if (Bsub.IsCopied(facecur))
      {
        myMapFaces.Bind(facecur, (Bsub.Copy(facecur)).First());
      }
    }

    if (Bsub.IsCopied(myShape))
    {
      myShape = (Bsub.Copy(myShape)).First();
    }

#ifdef OCCT_DEBUG
    if (tFE)
      std::cout << std::endl << "FuseEdges : " << NbVertices() << " vertices removed" << std::endl;
#endif
  }

  myShapeDone = Standard_True;
}

//=======================================================================
// function : BuildListConnexEdge
// purpose  : giving one edge, build the list of connex edges which have
// vertices that have only two connex edges. All the edges that are added
// to the list must be added also to the mapUniq, in order for the caller
// to not treat again these edges.
// This list is always oriented in the "Forward" direction.
//=======================================================================

void TopOpeBRepTool_FuseEdges::BuildListConnexEdge(const TopoShape&   theEdge,
                                                   TopTools_MapOfShape&  theMapUniq,
                                                   ShapeList& theLstEdg)
{

  TopoVertex VF, VL;

  VL = TopExp1::LastVertex(TopoDS::Edge(theEdge), Standard_True);
  TopoShape edgeconnex;
  TopoShape edgecur = theEdge;
  theLstEdg.Clear();
  theLstEdg.Append(edgecur);
  theMapUniq.Add(edgecur);
  TopAbs_Orientation ori2;

  // we first build the list of edges connex to edgecur by looking from the last Vertex VL
  while (NextConnexEdge(VL, edgecur, edgeconnex))
  {
    if (theMapUniq.Contains(edgeconnex))
    {
      break;
    }
    theLstEdg.Append(edgeconnex);
    edgecur = edgeconnex;
    // here take care about internal or external edges. It is non-sense to build
    // the connex list with such edges.
    ori2 = edgecur.Orientation();
    if (ori2 == TopAbs_EXTERNAL || ori2 == TopAbs_INTERNAL)
    {
      break;
    }
    VL = TopExp1::LastVertex(TopoDS::Edge(edgecur), Standard_True);
    theMapUniq.Add(edgecur);
  }

  edgecur = theEdge;
  VF      = TopExp1::FirstVertex(TopoDS::Edge(theEdge), Standard_True);

  // then we build the list of edges connex to edgecur by looking from the first Vertex VF
  while (NextConnexEdge(VF, edgecur, edgeconnex))
  {
    if (theMapUniq.Contains(edgeconnex))
    {
      break;
    }
    theLstEdg.Prepend(edgeconnex);
    edgecur = edgeconnex;
    // here take care about internal or external edges. It is non-sense to build
    // the connex list with such edges.
    ori2 = edgecur.Orientation();
    if (ori2 == TopAbs_EXTERNAL || ori2 == TopAbs_INTERNAL)
    {
      break;
    }
    VF = TopExp1::FirstVertex(TopoDS::Edge(edgecur), Standard_True);
    theMapUniq.Add(edgecur);
  }
}

//=======================================================================
// function : NextConnexEdge
// purpose  : Look for an edge connex to theEdge at theVertex.
// the connex edge must satisfies the following criteria :
//   * theVertex must have exactly 2 connex edges.
//   * the 2 connex edges must have exactly the 2 same connex faces
//   * the 2 connex edges must lie on the same support.
//=======================================================================

Standard_Boolean TopOpeBRepTool_FuseEdges::NextConnexEdge(const TopoVertex& theVertex,
                                                          const TopoShape&  theEdge,
                                                          TopoShape&        theEdgeConnex) const
{

  const ShapeList&        LmapEdg   = myMapVerLstEdg.FindFromKey(theVertex);
  Standard_Boolean                   HasConnex = Standard_True;
  TopTools_ListIteratorOfListOfShape itEdg, itFac1, itFac2;

  // 1st condition
  if (LmapEdg.Extent() == 2)
  {
    itEdg.Initialize(LmapEdg);
    theEdgeConnex = itEdg.Value();
    if (theEdge.IsSame(theEdgeConnex))
    {
      itEdg.Next();
      theEdgeConnex = itEdg.Value();
    }

    if (myAvoidEdg.Contains(theEdgeConnex))
      HasConnex = Standard_False; // edge is not allowed to be fused

    // 2nd condition
    if (HasConnex)
    {
      const ShapeList& LmapFac1 = myMapEdgLstFac.FindFromKey(theEdge);
      const ShapeList& LmapFac2 = myMapEdgLstFac.FindFromKey(theEdgeConnex);

      if (LmapFac1.Extent() == LmapFac2.Extent() && LmapFac1.Extent() < 3)
      {
        itFac1.Initialize(LmapFac1);

        // for each face in LmapFac1 we look in LmapFac2 if it exists
        while (itFac1.More() && HasConnex)
        {
          const TopoShape& face1 = itFac1.Value();
          for (itFac2.Initialize(LmapFac2); itFac2.More(); itFac2.Next())
          {
            const TopoShape& face2 = itFac2.Value();
            HasConnex                 = Standard_False;
            if (face1.IsSame(face2))
            {
              HasConnex = Standard_True;
              break;
            }
          }
          itFac1.Next();
        }

        // 3rd condition : same support
        if (HasConnex)
        {
          HasConnex = SameSupport(TopoDS::Edge(theEdge), TopoDS::Edge(theEdgeConnex));
        }
      }
      else
        HasConnex = Standard_False;
    }
  }
  else
    HasConnex = Standard_False;

  return HasConnex;
}

//=======================================================================
// function : SameSupport
// purpose  : Edges SameSupport ou pas
//=======================================================================

Standard_Boolean TopOpeBRepTool_FuseEdges::SameSupport(const TopoEdge& E1,
                                                       const TopoEdge& E2) const
{

  if (E1.IsNull() || E2.IsNull())
  {
    return Standard_False;
  }

  Handle(GeomCurve3d)    C1, C2;
  TopLoc_Location       loc;
  Standard_Real         f1, l1, f2, l2;
  Handle(TypeInfo) typC1, typC2;

  C1 = BRepInspector::Curve(E1, loc, f1, l1);
  // modified by NIZNHY-PKV Mon Nov 15 16:24:10 1999
  // degenerated edges has no 3D curve
  if (C1.IsNull())
    return Standard_False;

  if (!loc.IsIdentity())
  {
    Handle(Geom_Geometry) GG1 = C1->Transformed(loc.Transformation());
    C1                        = Handle(GeomCurve3d)::DownCast(GG1);
  }
  C2 = BRepInspector::Curve(E2, loc, f2, l2);
  // modified by NIZNHY-PKV Mon Nov 15 16:24:38 1999
  // degenerated edges has no 3D curve
  if (C2.IsNull())
    return Standard_False;

  if (!loc.IsIdentity())
  {
    Handle(Geom_Geometry) GG2 = C2->Transformed(loc.Transformation());
    C2                        = Handle(GeomCurve3d)::DownCast(GG2);
  }

  typC1 = C1->DynamicType();
  typC2 = C2->DynamicType();

  if (typC1 == STANDARD_TYPE(Geom_TrimmedCurve))
  {
    C1    = Handle(Geom_TrimmedCurve)::DownCast(C1)->BasisCurve();
    typC1 = C1->DynamicType();
  }

  if (typC2 == STANDARD_TYPE(Geom_TrimmedCurve))
  {
    C2    = Handle(Geom_TrimmedCurve)::DownCast(C2)->BasisCurve();
    typC2 = C2->DynamicType();
  }

  if (typC1 != typC2)
  {
    return Standard_False;
  }

  if (typC1 != STANDARD_TYPE(GeomLine) && typC1 != STANDARD_TYPE(GeomCircle)
      && typC1 != STANDARD_TYPE(Geom_Ellipse) && typC1 != STANDARD_TYPE(BSplineCurve3d)
      && typC1 != STANDARD_TYPE(BezierCurve3d))
  {
#ifdef OCCT_DEBUG
    std::cout << " TopOpeBRepTool_FuseEdge : Type de Support non traite" << std::endl;
#endif
    return Standard_False;
  }

  // On a presomption de confusion
  const Standard_Real tollin = Precision::Confusion();
  const Standard_Real tolang = Precision::Angular();
  if (typC1 == STANDARD_TYPE(GeomLine))
  {
    gp_Lin li1(Handle(GeomLine)::DownCast(C1)->Lin());
    gp_Lin li2(Handle(GeomLine)::DownCast(C2)->Lin());
    Dir3d dir1(li1.Direction());
    Dir3d dir2(li2.Direction());

    if (dir1.IsParallel(dir2, tolang))
    {
      // on verifie que l'on n'a pas de cas degenere. Par exemple E1 et E2 connexes
      // mais bouclant l'un sur l'autre (cas tres rare)
      Point3d pf1 = BRepInspector::Pnt(TopExp1::FirstVertex(E1, Standard_True));
      Point3d pl1 = BRepInspector::Pnt(TopExp1::LastVertex(E1, Standard_True));
      Point3d pf2 = BRepInspector::Pnt(TopExp1::FirstVertex(E2, Standard_True));
      Point3d pl2 = BRepInspector::Pnt(TopExp1::LastVertex(E2, Standard_True));
      if (pl1.Distance(pf2) < tollin && pl2.Distance(pf1) < tollin)
        return Standard_False;
      else
        return Standard_True;
    }
    return Standard_False;
  }
  else if (typC1 == STANDARD_TYPE(GeomCircle))
  {
    gp_Circ ci1 = Handle(GeomCircle)::DownCast(C1)->Circ();
    gp_Circ ci2 = Handle(GeomCircle)::DownCast(C2)->Circ();
    if (Abs(ci1.Radius() - ci2.Radius()) <= tollin
        && ci1.Location().SquareDistance(ci2.Location()) <= tollin * tollin
        && ci1.Axis().IsParallel(ci2.Axis(), tolang))
    {
      // Point debut, calage dans periode, et detection meme sens
      return Standard_True;
    }
    return Standard_False;
  }
  else if (typC1 == STANDARD_TYPE(Geom_Ellipse))
  {
    gp_Elips ci1 = Handle(Geom_Ellipse)::DownCast(C1)->Elips();
    gp_Elips ci2 = Handle(Geom_Ellipse)::DownCast(C2)->Elips();

    if (Abs(ci1.MajorRadius() - ci2.MajorRadius()) <= tollin
        && Abs(ci1.MinorRadius() - ci2.MinorRadius()) <= tollin
        && ci1.Location().SquareDistance(ci2.Location()) <= tollin * tollin
        && ci1.Axis().IsParallel(ci2.Axis(), tolang))
    {
      // Point debut, calage dans periode, et detection meme sens
      return Standard_True;
    }
    return Standard_False;
  }
  else if (typC1 == STANDARD_TYPE(BSplineCurve3d))
  {

    // we must ensure that before fuse two bsplines, the end of one curve does not
    // corresponds to the beginning of the second.
    // we could add a special treatment for periodic bspline. This is not done for the moment.
    if (Abs(f2 - l1) > tollin && Abs(f1 - l2) > tollin)
    {
      return Standard_False;
    }

    Handle(BSplineCurve3d) B1 = Handle(BSplineCurve3d)::DownCast(C1);
    Handle(BSplineCurve3d) B2 = Handle(BSplineCurve3d)::DownCast(C2);

    Standard_Integer nbpoles = B1->NbPoles();
    if (nbpoles != B2->NbPoles())
    {
      return Standard_False;
    }

    Standard_Integer nbknots = B1->NbKnots();
    if (nbknots != B2->NbKnots())
    {
      return Standard_False;
    }

    TColgp_Array1OfPnt P1(1, nbpoles), P2(1, nbpoles);
    B1->Poles(P1);
    B2->Poles(P2);

    Standard_Real tol3d = BRepInspector::Tolerance(E1);
    for (Standard_Integer p = 1; p <= nbpoles; p++)
    {
      if ((P1(p)).Distance(P2(p)) > tol3d)
      {
        return Standard_False;
      }
    }

    TColStd_Array1OfReal K1(1, nbknots), K2(1, nbknots);
    B1->Knots(K1);
    B2->Knots(K2);

    TColStd_Array1OfInteger M1(1, nbknots), M2(1, nbknots);
    B1->Multiplicities(M1);
    B2->Multiplicities(M2);

    for (Standard_Integer k = 1; k <= nbknots; k++)
    {
      if ((K1(k) - K2(k)) > tollin)
      {
        return Standard_False;
      }
      if (Abs(M1(k) - M2(k)) > tollin)
      {
        return Standard_False;
      }
    }

    if (!B1->IsRational())
    {
      if (B2->IsRational())
      {
        return Standard_False;
      }
    }
    else
    {
      if (!B2->IsRational())
      {
        return Standard_False;
      }
    }

    if (B1->IsRational())
    {
      TColStd_Array1OfReal W1(1, nbpoles), W2(1, nbpoles);
      B1->Weights(W1);
      B2->Weights(W2);

      for (Standard_Integer w = 1; w <= nbpoles; w++)
      {
        if (Abs(W1(w) - W2(w)) > tollin)
        {
          return Standard_False;
        }
      }
    }
    return Standard_True;
  }
  else if (typC1 == STANDARD_TYPE(BezierCurve3d))
  {

    // we must ensure that before fuse two bezier, the end of one curve does not
    // corresponds to the beginning of the second.
    if (Abs(f2 - l1) > tollin && Abs(f1 - l2) > tollin)
    {
      return Standard_False;
    }

    Handle(BezierCurve3d) B1 = Handle(BezierCurve3d)::DownCast(C1);
    Handle(BezierCurve3d) B2 = Handle(BezierCurve3d)::DownCast(C2);

    Standard_Integer nbpoles = B1->NbPoles();
    if (nbpoles != B2->NbPoles())
    {
      return Standard_False;
    }

    TColgp_Array1OfPnt P1(1, nbpoles), P2(1, nbpoles);
    B1->Poles(P1);
    B2->Poles(P2);

    for (Standard_Integer p = 1; p <= nbpoles; p++)
    {
      if ((P1(p)).Distance(P2(p)) > tollin)
      {
        return Standard_False;
      }
    }

    if (!B1->IsRational())
    {
      if (B2->IsRational())
      {
        return Standard_False;
      }
    }
    else
    {
      if (!B2->IsRational())
      {
        return Standard_False;
      }
    }

    if (B1->IsRational())
    {
      TColStd_Array1OfReal W1(1, nbpoles), W2(1, nbpoles);
      B1->Weights(W1);
      B2->Weights(W2);

      for (Standard_Integer w = 1; w <= nbpoles; w++)
      {
        if (Abs(W1(w) - W2(w)) > tollin)
        {
          return Standard_False;
        }
      }
    }
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean TopOpeBRepTool_FuseEdges::UpdatePCurve(const TopoEdge&          theOldEdge,
                                                        TopoEdge&                theNewEdge,
                                                        const ShapeList& theLstEdg) const
{

  // get the pcurve of edge to substitute (theOldEdge)
  // using CurveOnSurface with Index syntax, so we can update the pcurve
  // on all the faces
  ShapeBuilder         B;
  Handle(GeomCurve2d) Curv2d;
  Handle(GeomSurface) Surf;
  TopLoc_Location      loc, locbid;
  Standard_Real        ef, el, cf, cl;
  Standard_Integer     iedg = 1;

  // take care that we want only Pcurve that maps on the surface where the 3D edges lies.
  const ShapeList& LmapFac = myMapEdgLstFac.FindFromKey(theOldEdge);

  BRepInspector::CurveOnSurface(theOldEdge, Curv2d, Surf, loc, cf, cl, iedg);

  Standard_Boolean pcurveRebuilt = Standard_False;

  while (!Curv2d.IsNull())
  {

    // we look for a face that contains the same surface as the one that cames
    // from CurveOnSurface
    Standard_Boolean                   SameSurf = Standard_False;
    TopTools_ListIteratorOfListOfShape itFac;

    for (itFac.Initialize(LmapFac); itFac.More(); itFac.Next())
    {
      const TopoShape&  face = itFac.Value();
      Handle(GeomSurface) S    = BRepInspector::Surface(TopoDS::Face(face), locbid);
      if (S == Surf)
      {
        SameSurf = Standard_True;
        break;
      }
    }

    if (SameSurf)
    {

      BRepInspector::Range(theNewEdge, ef, el);

      // modified by NIZNHY-PKV Mon Nov 15 14:59:48 1999 _from
      TopoEdge aFEdge = theOldEdge;
      aFEdge.Orientation(TopAbs_FORWARD);

      // take care if the edge is on the closing curve of a closed surface. In that case
      // we get the second pcurve by reversing the edge and calling again CurveOnSurface method

      BRepInspector::CurveOnSurface(aFEdge, Curv2d, Surf, loc, cf, cl, iedg);
      if (BRepInspector::IsClosed(theOldEdge, Surf, loc))
      {
        aFEdge.Reverse();
        TopoFace aFFace = TopoDS::Face(itFac.Value());
        aFFace.Orientation(TopAbs_FORWARD);
        Handle(GeomCurve2d) Curv2dR = BRepInspector::CurveOnSurface(aFEdge, aFFace, cf, cl);
        if (Curv2d->DynamicType() == STANDARD_TYPE(Geom2d_TrimmedCurve))
          Curv2d = Handle(Geom2d_TrimmedCurve)::DownCast(Curv2d)->BasisCurve();
        if (Curv2dR->DynamicType() == STANDARD_TYPE(Geom2d_TrimmedCurve))
          Curv2dR = Handle(Geom2d_TrimmedCurve)::DownCast(Curv2dR)->BasisCurve();

        B.UpdateEdge(theNewEdge, Curv2d, Curv2dR, Surf, loc, BRepInspector::Tolerance(theNewEdge));
      }
      else
      {
        // update the new edge
        if (Curv2d->DynamicType() == STANDARD_TYPE(Geom2d_TrimmedCurve))
          Curv2d = Handle(Geom2d_TrimmedCurve)::DownCast(Curv2d)->BasisCurve();
        Standard_Real f, l;
        f = Curv2d->FirstParameter();
        l = Curv2d->LastParameter();
        if (l - f + 2. * Epsilon(l - f) < el - ef)
        {
          Handle(Geom2d_BoundedCurve) bcurve = Handle(Geom2d_BoundedCurve)::DownCast(Curv2d);
          if (bcurve.IsNull())
            bcurve = new Geom2d_TrimmedCurve(Curv2d, cf, cl);
          Geom2dConvert_CompCurveToBSplineCurve Concat(bcurve);
          TopTools_ListIteratorOfListOfShape    iter(theLstEdg);
          iter.Next();
          for (; iter.More(); iter.Next())
          {
            const TopoEdge&          E = TopoDS::Edge(iter.Value());
            Standard_Real               first, last;
            Handle(GeomCurve2d)        C  = BRepInspector::CurveOnSurface(E, Surf, loc, first, last);
            Handle(Geom2d_BoundedCurve) BC = Handle(Geom2d_BoundedCurve)::DownCast(C);
            if (BC.IsNull())
              BC = new Geom2d_TrimmedCurve(C, first, last);
            if (!Concat.Add(BC, Precision::PConfusion()))
              // cannot merge pcurves
              return Standard_False;
          }
          Curv2d = Concat.BSplineCurve();

          // check that new curve 2d is same range
          Standard_Real first = Curv2d->FirstParameter();
          Standard_Real last  = Curv2d->LastParameter();
          if (Abs(first - ef) > Precision::PConfusion() || Abs(last - el) > Precision::PConfusion())
          {
            Handle(Geom2d_BSplineCurve) bc = Handle(Geom2d_BSplineCurve)::DownCast(Curv2d);
            TColStd_Array1OfReal        Knots(1, bc->NbKnots());
            bc->Knots(Knots);
            BSplCLib::Reparametrize(ef, el, Knots);
            bc->SetKnots(Knots);
          }
          pcurveRebuilt = Standard_True;
        }

        B.UpdateEdge(theNewEdge, Curv2d, Surf, loc, BRepInspector::Tolerance(theNewEdge));
      }

      // the old pcurve range is cf,cl. The new 3d edge range is ef,el. if we want
      // the pcurve to be samerange we must adapt the parameter of the edge. In general
      // cases cf=ef and cl=el expect for periodic curve if the new edge is going over
      // the value 0.
      if (theOldEdge.Orientation() == TopAbs_REVERSED)
      {
        B.Range(theNewEdge, cl - el + ef, cl);
      }
      else
      {
        B.Range(theNewEdge, cf, cf + el - ef);
      }
    }

    // get next pcurve
    iedg++;
    BRepInspector::CurveOnSurface(theOldEdge, Curv2d, Surf, loc, cf, cl, iedg);
  }

  if (pcurveRebuilt)
  {
    // force same parameter
    B.SameParameter(theNewEdge, Standard_False);
    BRepLib::SameParameter(theNewEdge, BRepInspector::Tolerance(theNewEdge));
  }

  return Standard_True;
}
