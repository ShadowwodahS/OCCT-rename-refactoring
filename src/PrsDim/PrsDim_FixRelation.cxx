// Created on: 1996-12-05
// Created by: Flore Lantheaume/Odile Olivier
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

#include <PrsDim_FixRelation.hxx>

#include <PrsDim.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <DsgPrs_FixPresentation.hxx>
#include <ElCLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <gp_Ax1.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <Precision.hxx>
#include <Prs3d_Presentation.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <SelectMgr_Selection.hxx>
#include <TColStd_ListIteratorOfListOfTransient.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

IMPLEMENT_STANDARD_RTTIEXT(PrsDim_FixRelation, PrsDim_Relation)

static Standard_Boolean InDomain(const Standard_Real fpar,
                                 const Standard_Real lpar,
                                 const Standard_Real para)
{
  if (fpar >= 0.)
  {
    return ((para >= fpar) && (para <= lpar));
  }
  if (para >= (fpar + 2 * M_PI))
    return Standard_True;
  if (para <= lpar)
    return Standard_True;
  return Standard_False;
}

//=======================================================================
// function : Constructor
// purpose  : vertex Fix Relation
//=======================================================================

PrsDim_FixRelation::PrsDim_FixRelation(const TopoShape&       aShape,
                                       const Handle(GeomPlane)& aPlane,
                                       const TopoWire&        aWire)
    : PrsDim_Relation(),
      myWire(aWire)
{
  myFShape            = aShape;
  myPlane             = aPlane;
  myAutomaticPosition = Standard_True;
  myArrowSize         = 5.;
}

//=======================================================================
// function : Constructor
// purpose  : vertex Fix Relation
//=======================================================================

PrsDim_FixRelation::PrsDim_FixRelation(const TopoShape&       aShape,
                                       const Handle(GeomPlane)& aPlane,
                                       const TopoWire&        aWire,
                                       const Point3d&             aPosition,
                                       const Standard_Real       anArrowSize)
    : PrsDim_Relation(),
      myWire(aWire)
{
  myFShape   = aShape;
  myPlane    = aPlane;
  myPosition = aPosition;
  SetArrowSize(anArrowSize);
  myAutomaticPosition = Standard_False;
}

//=======================================================================
// function : Constructor
// purpose  : edge (line or circle) Fix Relation
//=======================================================================

PrsDim_FixRelation::PrsDim_FixRelation(const TopoShape& aShape, const Handle(GeomPlane)& aPlane)
{
  myFShape            = aShape;
  myPlane             = aPlane;
  myAutomaticPosition = Standard_True;
  myArrowSize         = 5.;
}

//=======================================================================
// function : Constructor
// purpose  : edge (line or circle) Fix Relation
//=======================================================================

PrsDim_FixRelation::PrsDim_FixRelation(const TopoShape&       aShape,
                                       const Handle(GeomPlane)& aPlane,
                                       const Point3d&             aPosition,
                                       const Standard_Real       anArrowSize)
{
  myFShape   = aShape;
  myPlane    = aPlane;
  myPosition = aPosition;
  SetArrowSize(anArrowSize);
  myAutomaticPosition = Standard_False;
}

//=================================================================================================

void PrsDim_FixRelation::Compute(const Handle(PrsMgr_PresentationManager)&,
                                 const Handle(Prs3d_Presentation)& aPresentation,
                                 const Standard_Integer)
{
  // Calculate position of the symbol and
  // point of attach of the segment on the shape
  Point3d curpos;
  if (myFShape.ShapeType() == TopAbs_VERTEX)
    ComputeVertex(TopoDS::Vertex(myFShape), curpos);
  else if (myFShape.ShapeType() == TopAbs_EDGE)
    ComputeEdge(TopoDS::Edge(myFShape), curpos);

  const Dir3d& nor = myPlane->Axis().Direction();

  // calculate presentation
  // definition of the symbol size
  if (!myArrowSizeIsDefined)
    myArrowSize = 5.;

  // creation of the presentation
  DsgPrs_FixPresentation::Add(aPresentation, myDrawer, myPntAttach, curpos, nor, myArrowSize);
}

//=================================================================================================

void PrsDim_FixRelation::ComputeSelection(const Handle(SelectionContainer)& aSelection,
                                          const Standard_Integer)
{
  Handle(SelectMgr_EntityOwner) own = new SelectMgr_EntityOwner(this, 7);

  // creation of segment sensible for the linked segment
  // of the shape fixed to symbol 'Fix'
  Handle(Select3D_SensitiveSegment) seg;
  seg = new Select3D_SensitiveSegment(own, myPntAttach, myPosition);
  aSelection->Add(seg);

  // Creation of the sensible zone of symbol 'Fix'
  Dir3d norm = myPlane->Axis().Direction();

  Vector3d dirac(myPntAttach, myPosition);
  dirac.Normalize();
  Vector3d norac = dirac.Crossed(Vector3d(norm));
  Axis3d ax(myPosition, norm);
  norac.Rotate(ax, M_PI / 8);

  norac *= (myArrowSize / 2);
  Point3d P1 = myPosition.Translated(norac);
  Point3d P2 = myPosition.Translated(-norac);
  seg       = new Select3D_SensitiveSegment(own, P1, P2);
  aSelection->Add(seg);

  norac *= 0.8;
  P1 = myPosition.Translated(norac);
  P2 = myPosition.Translated(-norac);
  dirac *= (myArrowSize / 2);
  Point3d PF(P1.XYZ());
  Point3d PL = PF.Translated(dirac);
  PL.Translate(norac);
  seg = new Select3D_SensitiveSegment(own, PF, PL);
  aSelection->Add(seg);

  PF.SetXYZ(P2.XYZ());
  PL = PF.Translated(dirac);
  PL.Translate(norac);
  seg = new Select3D_SensitiveSegment(own, PF, PL);
  aSelection->Add(seg);

  PF.SetXYZ((P1.XYZ() + P2.XYZ()) / 2);
  PL = PF.Translated(dirac);
  PL.Translate(norac);
  seg = new Select3D_SensitiveSegment(own, PF, PL);
}

//=======================================================================
// function : ComputeVertex
// purpose  : computes myPntAttach and the position of the presentation
//           when you fix a vertex
//=======================================================================

void PrsDim_FixRelation::ComputeVertex(const TopoVertex& /*FixVertex*/, Point3d& curpos)
{
  myPntAttach = BRepInspector::Pnt(TopoDS::Vertex(myFShape));
  curpos      = myPosition;
  if (myAutomaticPosition)
  {
    gp_Pln pln(myPlane->Pln());
    Dir3d dir(pln.XAxis().Direction());
    Vector3d transvec     = Vector3d(dir) * myArrowSize;
    curpos              = myPntAttach.Translated(transvec);
    myPosition          = curpos;
    myAutomaticPosition = Standard_True;
  }
}

//=================================================================================================

Point3d PrsDim_FixRelation::ComputePosition(const Handle(GeomCurve3d)& curv1,
                                           const Handle(GeomCurve3d)& curv2,
                                           const Point3d&             firstp1,
                                           const Point3d&             lastp1,
                                           const Point3d&             firstp2,
                                           const Point3d&             lastp2) const
{
  //---------------------------------------------------------
  // calculate the point of attach
  //---------------------------------------------------------
  Point3d curpos;

  if (curv1->IsInstance(STANDARD_TYPE(GeomCircle))
      || curv2->IsInstance(STANDARD_TYPE(GeomCircle)))
  {
    Handle(GeomCircle) gcirc = Handle(GeomCircle)::DownCast(curv1);
    if (gcirc.IsNull())
      gcirc = Handle(GeomCircle)::DownCast(curv2);
    Dir3d dir(gcirc->Location().XYZ() + myPntAttach.XYZ());
    Vector3d transvec = Vector3d(dir) * myArrowSize;
    curpos          = myPntAttach.Translated(transvec);
  }

  else
  {
    Vector3d vec1(firstp1, lastp1);
    Vector3d vec2(firstp2, lastp2);

    if (!vec1.IsParallel(vec2, Precision::Angular()))
    {
      Dir3d                  dir;
      constexpr Standard_Real conf = Precision::Confusion();
      if (lastp1.IsEqual(firstp2, conf) || firstp1.IsEqual(lastp2, conf))
        dir.SetXYZ(vec1.XYZ() - vec2.XYZ());
      else
        dir.SetXYZ(vec1.XYZ() + vec2.XYZ());
      Vector3d transvec = Vector3d(dir) * myArrowSize;
      curpos          = myPntAttach.Translated(transvec);
    }
    else
    {
      Vector3d crossvec = vec1.Crossed(vec2);
      vec1.Cross(crossvec);
      Dir3d dir(vec1);
      curpos = myPntAttach.Translated(Vector3d(dir) * myArrowSize);
    }
  }

  return curpos;
}

//=======================================================================
// function : ComputePosition
// purpose  : Computes the position of the "fix dimension" when the
//           fixed object is a vertex which is set at the intersection
//           of two curves.
//           The "dimension" is in the "middle" of the two edges.
//=======================================================================

Point3d PrsDim_FixRelation::ComputePosition(const Handle(GeomCurve3d)& curv,
                                           const Point3d&             firstp,
                                           const Point3d&             lastp) const
{
  //---------------------------------------------------------
  // calculate the point of attach
  //---------------------------------------------------------
  Point3d curpos;

  if (curv->IsKind(STANDARD_TYPE(GeomCircle)))
  {

    Handle(GeomCircle) gcirc = Handle(GeomCircle)::DownCast(curv);
    Dir3d              dir(gcirc->Location().XYZ() + myPntAttach.XYZ());
    Vector3d              transvec = Vector3d(dir) * myArrowSize;
    curpos                       = myPntAttach.Translated(transvec);

  } // if (curv->IsKind(STANDARD_TYPE(GeomCircle))

  else
  {
    //    gp_Pln pln(Component()->WorkingPlane()->Plane1()->GetValue()->Pln());
    gp_Pln pln(myPlane->Pln());
    Dir3d NormPln = pln.Axis().Direction();
    Vector3d vec(firstp, lastp);
    vec.Cross(Vector3d(NormPln));
    vec.Normalize();
    Vector3d transvec = vec * myArrowSize;
    curpos          = myPntAttach.Translated(transvec);
    Axis3d RotAx(myPntAttach, NormPln);
    curpos.Rotate(RotAx, M_PI / 10);
  }

  return curpos;
}

//=======================================================================
// function : ComputeEdge
// purpose  : computes myPntAttach and the position of the presentation
//           when you fix an edge
//=======================================================================

void PrsDim_FixRelation::ComputeEdge(const TopoEdge& FixEdge, Point3d& curpos)
{
  Handle(GeomCurve3d) curEdge;
  Point3d             ptbeg, ptend;
  if (!PrsDim::ComputeGeometry(FixEdge, curEdge, ptbeg, ptend))
    return;

  //---------------------------------------------------------
  // calcul du point de positionnement du symbole 'fix'
  //---------------------------------------------------------
  //--> In case of a straight line
  if (curEdge->IsKind(STANDARD_TYPE(GeomLine)))
  {
    gp_Lin        glin = Handle(GeomLine)::DownCast(curEdge)->Lin();
    Standard_Real pfirst(ElCLib1::Parameter(glin, ptbeg));
    Standard_Real plast(ElCLib1::Parameter(glin, ptend));
    ComputeLinePosition(glin, curpos, pfirst, plast);
  }

  //--> In case of a circle
  else if (curEdge->IsKind(STANDARD_TYPE(GeomCircle)))
  {
    gp_Circ           gcirc = Handle(GeomCircle)::DownCast(curEdge)->Circ();
    Standard_Real     pfirst, plast;
    BRepAdaptor_Curve cu(FixEdge);
    pfirst = cu.FirstParameter();
    plast  = cu.LastParameter();
    ComputeCirclePosition(gcirc, curpos, pfirst, plast);
  }

  else
    return;
}

//=======================================================================
// function : ComputeLinePosition
// purpose  : compute the values of myPntAttach and the position <pos> of
//           the symbol when the fixed edge has a geometric support equal
//           to a line.
//=======================================================================

void PrsDim_FixRelation::ComputeLinePosition(const gp_Lin&  glin,
                                             Point3d&        pos,
                                             Standard_Real& pfirst,
                                             Standard_Real& plast)
{
  if (myAutomaticPosition)
  {
    // point of attach is chosen as middle of the segment
    myPntAttach = ElCLib1::Value((pfirst + plast) / 2, glin);

    Dir3d norm = myPlane->Axis().Direction();

    norm.Cross(glin.Position().Direction());
    pos                 = myPntAttach.Translated(Vector3d(norm) * myArrowSize);
    myAutomaticPosition = Standard_True;
  } // if (myAutomaticPosition)

  else
  {
    pos                    = myPosition;
    Standard_Real linparam = ElCLib1::Parameter(glin, pos);

    // case if the projection of position is located between 2 vertices
    // de l'edge
    if ((linparam >= pfirst) && (linparam <= plast))
      myPntAttach = ElCLib1::Value(linparam, glin);

    // case if the projection of Position is outside of the limits
    // of the edge : the point closest to the projection is chosen
    // as the attach point
    else
    {
      Standard_Real pOnLin;
      if (linparam > plast)
        pOnLin = plast;
      else
        pOnLin = pfirst;
      myPntAttach = ElCLib1::Value(pOnLin, glin);
      Dir3d norm = myPlane->Axis().Direction();

      norm.Cross(glin.Position().Direction());
      gp_Lin        lsup(myPntAttach, norm);
      Standard_Real parpos = ElCLib1::Parameter(lsup, myPosition);
      pos                  = ElCLib1::Value(parpos, lsup);
    }
  }
  myPosition = pos;
}

//=======================================================================
// function : ComputeCirclePosition
// purpose  : compute the values of myPntAttach and the position <pos> of
//           the symbol when the fixed edge has a geometric support equal
//           to a circle.
//=======================================================================

void PrsDim_FixRelation::ComputeCirclePosition(const gp_Circ& gcirc,
                                               Point3d&        pos,
                                               Standard_Real& pfirst,
                                               Standard_Real& plast)
{
  // readjust parametres on the circle
  if (plast > 2 * M_PI)
  {
    Standard_Real nbtours = Floor(plast / (2 * M_PI));
    plast -= nbtours * 2 * M_PI;
    pfirst -= nbtours * 2 * M_PI;
  }

  if (myAutomaticPosition)
  {
    // the point attach is the "middle" of the segment (relatively
    // to the parametres of start and end vertices of the edge

    Standard_Real circparam = (pfirst + plast) / 2.;

    if (!InDomain(pfirst, plast, circparam))
    {
      Standard_Real otherpar = circparam + M_PI;
      if (otherpar > 2 * M_PI)
        otherpar -= 2 * M_PI;
      circparam = otherpar;
    }

    myPntAttach = ElCLib1::Value(circparam, gcirc);

    Vector3d dir(gcirc.Location().XYZ(), myPntAttach.XYZ());
    dir.Normalize();
    Vector3d transvec     = dir * myArrowSize;
    pos                 = myPntAttach.Translated(transvec);
    myPosition          = pos;
    myAutomaticPosition = Standard_True;
  } // if (myAutomaticPosition)

  else
  {
    // case if the projection of myPosition is outside of 2
    // vertices of the edge. In this case the parameter is readjusted
    // in the valid part of the circle
    pos = myPosition;

    Standard_Real circparam = ElCLib1::Parameter(gcirc, pos);

    if (!InDomain(pfirst, plast, circparam))
    {
      Standard_Real otherpar = circparam + M_PI;
      if (otherpar > 2 * M_PI)
        otherpar -= 2 * M_PI;
      circparam = otherpar;
    }

    myPntAttach = ElCLib1::Value(circparam, gcirc);
  }
}

//=================================================================================================

Standard_Boolean PrsDim_FixRelation::ConnectedEdges(const TopoWire&   WIRE,
                                                    const TopoVertex& V,
                                                    TopoEdge&         E1,
                                                    TopoEdge&         E2)
{
  TopTools_IndexedDataMapOfShapeListOfShape vertexMap;
  TopExp1::MapShapesAndAncestors(WIRE, TopAbs_VERTEX, TopAbs_EDGE, vertexMap);

  Standard_Boolean found(Standard_False);
  TopoVertex    theVertex;
  for (Standard_Integer i = 1; i <= vertexMap.Extent() && !found; i++)
  {
    if (vertexMap.FindKey(i).IsSame(V))
    {
      theVertex = TopoDS::Vertex(vertexMap.FindKey(i));
      found     = Standard_True;
    }
  }
  if (!found)
  {
    E1.Nullify();
    E2.Nullify();
    return Standard_False;
  }

  TopTools_ListIteratorOfListOfShape iterator(vertexMap.FindFromKey(theVertex));
  if (iterator.More())
  {
    E1 = TopoDS::Edge(iterator.Value());
    BRepAdaptor_Curve curv(E1);
    iterator.Next();
  }
  else
  {
    E1.Nullify();
    return Standard_False;
  }

  if (iterator.More())
  {
    E2 = TopoDS::Edge(iterator.Value());
    BRepAdaptor_Curve curv(E2);
    iterator.Next();
  }
  else
  {
    E2.Nullify();
    return Standard_False;
  }

  if (iterator.More())
  {
    E1.Nullify();
    E2.Nullify();
    return Standard_False;
  }
  return Standard_True;
}
