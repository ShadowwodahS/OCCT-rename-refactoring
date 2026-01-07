// Created on: 1993-02-04
// Created by: Laurent BOURESCHE
// Copyright (c) 1993-1999 Matra Datavision
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

#include <BRepSweep_Translation.hxx>

#include <GeomAdaptor_SurfaceOfLinearExtrusion.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepTools.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Surface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Lin.hxx>
#include <gp_Lin2d.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Sweep_NumShape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

static void SetThePCurve(const ShapeBuilder&         B,
                         TopoEdge&                E,
                         const TopoFace&          F,
                         const TopAbs_Orientation    O,
                         const Handle(GeomCurve2d)& C)
{
  // check if there is already a pcurve on non planar faces
  Standard_Real        f, l;
  Handle(GeomCurve2d) OC;
  TopLoc_Location      SL;
  Handle(GeomPlane)   GP = Handle(GeomPlane)::DownCast(BRepInspector::Surface(F, SL));
  if (GP.IsNull())
    OC = BRepInspector::CurveOnSurface(E, F, f, l);
  if (OC.IsNull())
    B.UpdateEdge(E, C, F, Precision::Confusion());
  else
  {
    if (O == TopAbs_REVERSED)
      B.UpdateEdge(E, OC, C, F, Precision::Confusion());
    else
      B.UpdateEdge(E, C, OC, F, Precision::Confusion());
  }
}

//=================================================================================================

BRepSweep_Translation::BRepSweep_Translation(const TopoShape&    S,
                                             const Sweep_NumShape&  N,
                                             const TopLoc_Location& L,
                                             const Vector3d&          V,
                                             const Standard_Boolean C,
                                             const Standard_Boolean Canonize)
    : BRepSweep_Trsf(ShapeBuilder(), S, N, L, C),
      myVec(V),
      myCanonize(Canonize)
{

  Standard_ConstructionError_Raise_if(V.Magnitude() < Precision::Confusion(),
                                      "BRepSweep_Translation::Constructor");
  Init();
}

//=================================================================================================

TopoShape BRepSweep_Translation::MakeEmptyVertex(const TopoShape&   aGenV,
                                                    const Sweep_NumShape& aDirV)
{
  // Only called when the option of construction is with copy.
  Standard_ConstructionError_Raise_if(!myCopy, "BRepSweep_Translation::MakeEmptyVertex");
  Point3d P = BRepInspector::Pnt(TopoDS::Vertex(aGenV));
  if (aDirV.Index() == 2)
    P.Transform(myLocation.Transformation());
  TopoVertex V;
  ////// modified by jgv, 5.10.01, for buc61008 //////
  // myBuilder.Builder().MakeVertex(V,P,Precision::Confusion());
  myBuilder.Builder().MakeVertex(V, P, BRepInspector::Tolerance(TopoDS::Vertex(aGenV)));
  ////////////////////////////////////////////////////
  return V;
}

//=================================================================================================

TopoShape BRepSweep_Translation::MakeEmptyDirectingEdge(const TopoShape& aGenV,
                                                           const Sweep_NumShape&)
{
  Point3d            P = BRepInspector::Pnt(TopoDS::Vertex(aGenV));
  gp_Lin            L(P, myVec);
  Handle(GeomLine) GL = new GeomLine(L);
  TopoEdge       E;
  myBuilder.Builder().MakeEdge(E, GL, BRepInspector::Tolerance(TopoDS::Vertex(aGenV)));
  return E;
}

//=================================================================================================

TopoShape BRepSweep_Translation::MakeEmptyGeneratingEdge(const TopoShape&   aGenE,
                                                            const Sweep_NumShape& aDirV)
{
  // Call only in case of construction with copy.
  Standard_ConstructionError_Raise_if(!myCopy, "BRepSweep_Translation::MakeEmptyVertex");
  TopoEdge newE;
  if (BRepInspector::Degenerated(TopoDS::Edge(aGenE)))
  {
    myBuilder.Builder().MakeEdge(newE);
    myBuilder.Builder().UpdateEdge(newE, BRepInspector::Tolerance(TopoDS::Edge(aGenE)));
    myBuilder.Builder().Degenerated(newE, Standard_True);
  }
  else
  {
    TopLoc_Location    L;
    Standard_Real      First, Last;
    Handle(GeomCurve3d) C = BRepInspector::Curve(TopoDS::Edge(aGenE), L, First, Last);
    if (!C.IsNull())
    {
      C = Handle(GeomCurve3d)::DownCast(C->Copy());
      C->Transform(L.Transformation());
      if (aDirV.Index() == 2)
        C->Transform(myLocation.Transformation());
    }
    myBuilder.Builder().MakeEdge(newE, C, BRepInspector::Tolerance(TopoDS::Edge(aGenE)));
  }
  return newE;
}

//=================================================================================================

void BRepSweep_Translation::SetParameters(const TopoShape& aNewFace,
                                          TopoShape&       aNewVertex,
                                          const TopoShape& aGenF,
                                          const TopoShape& aGenV,
                                          const Sweep_NumShape&)
{
  // Glue the parameter of vertices directly included in cap faces.
  gp_Pnt2d pnt2d = BRepInspector::Parameters(TopoDS::Vertex(aGenV), TopoDS::Face(aGenF));
  myBuilder.Builder().UpdateVertex(TopoDS::Vertex(aNewVertex),
                                   pnt2d.X(),
                                   pnt2d.Y(),
                                   TopoDS::Face(aNewFace),
                                   Precision::PConfusion());
}

//=================================================================================================

void BRepSweep_Translation::SetDirectingParameter(const TopoShape& aNewEdge,
                                                  TopoShape&       aNewVertex,
                                                  const TopoShape&,
                                                  const Sweep_NumShape&,
                                                  const Sweep_NumShape& aDirV)
{
  Standard_Real param = 0;
  if (aDirV.Index() == 2)
    param = myVec.Magnitude();
  myBuilder.Builder().UpdateVertex(TopoDS::Vertex(aNewVertex),
                                   param,
                                   TopoDS::Edge(aNewEdge),
                                   Precision::PConfusion());
}

//=================================================================================================

void BRepSweep_Translation::SetGeneratingParameter(const TopoShape& aNewEdge,
                                                   TopoShape&       aNewVertex,
                                                   const TopoShape& aGenE,
                                                   const TopoShape& aGenV,
                                                   const Sweep_NumShape&)
{
  TopoVertex vbid = TopoDS::Vertex(aNewVertex);
  vbid.Orientation(aGenV.Orientation());
  myBuilder.Builder().UpdateVertex(vbid,
                                   BRepInspector::Parameter(TopoDS::Vertex(aGenV), TopoDS::Edge(aGenE)),
                                   TopoDS::Edge(aNewEdge),
                                   Precision::PConfusion());
}

//=================================================================================================

TopoShape BRepSweep_Translation::MakeEmptyFace(const TopoShape&   aGenS,
                                                  const Sweep_NumShape& aDirS)
{
  Standard_Real        toler;
  TopoFace          F;
  Handle(GeomSurface) S;
  if (myDirShapeTool.Type(aDirS) == TopAbs_EDGE)
  {
    TopLoc_Location    L;
    Standard_Real      First, Last;
    Handle(GeomCurve3d) C = BRepInspector::Curve(TopoDS::Edge(aGenS), L, First, Last);
    toler                = BRepInspector::Tolerance(TopoDS::Edge(aGenS));
    Transform3d Tr           = L.Transformation();
    C                    = Handle(GeomCurve3d)::DownCast(C->Copy());
    // extruded surfaces are inverted correspondingly to the topology, so reverse.
    C->Transform(Tr);
    Dir3d D(myVec);
    D.Reverse();

    if (myCanonize)
    {
      Handle(GeomAdaptor_Curve)            HC = new GeomAdaptor_Curve(C, First, Last);
      GeomAdaptor_SurfaceOfLinearExtrusion AS(HC, D);
      switch (AS.GetType())
      {

        case GeomAbs_Plane:
          S = new GeomPlane(AS.Plane());
          break;
        case GeomAbs_Cylinder:
          S = new Geom_CylindricalSurface(AS.Cylinder());
          break;
        default:
          S = new Geom_SurfaceOfLinearExtrusion(C, D);
          break;
      }
    }
    else
    {
      S = new Geom_SurfaceOfLinearExtrusion(C, D);
    }
  }
  else
  {
    TopLoc_Location L;
    S          = BRepInspector::Surface(TopoDS::Face(aGenS), L);
    toler      = BRepInspector::Tolerance(TopoDS::Face(aGenS));
    Transform3d Tr = L.Transformation();
    S          = Handle(GeomSurface)::DownCast(S->Copy());
    S->Transform(Tr);
    if (aDirS.Index() == 2)
      S->Translate(myVec);
  }
  myBuilder.Builder().MakeFace(F, S, toler);
  return F;
}

//=================================================================================================

void BRepSweep_Translation::SetPCurve(const TopoShape& aNewFace,
                                      TopoShape&       aNewEdge,
                                      const TopoShape& aGenF,
                                      const TopoShape& aGenE,
                                      const Sweep_NumShape&,
                                      const TopAbs_Orientation)
{
  // Set on edges of cap faces the same pcurves as
  // edges of the generating face.
  Standard_Boolean isclosed = BRepInspector::IsClosed(TopoDS::Edge(aGenE), TopoDS::Face(aGenF));
  if (isclosed)
  {
    Standard_Real        First, Last;
    TopoEdge          anE = TopoDS::Edge(aGenE.Oriented(TopAbs_FORWARD));
    Handle(GeomCurve2d) aC1 = BRepInspector::CurveOnSurface(anE, TopoDS::Face(aGenF), First, Last);
    anE.Reverse();
    Handle(GeomCurve2d) aC2 = BRepInspector::CurveOnSurface(anE, TopoDS::Face(aGenF), First, Last);
    myBuilder.Builder().UpdateEdge(TopoDS::Edge(aNewEdge),
                                   aC1,
                                   aC2,
                                   TopoDS::Face(aNewFace),
                                   Precision::PConfusion());
  }
  else
  {
    Standard_Real First, Last;
    myBuilder.Builder().UpdateEdge(
      TopoDS::Edge(aNewEdge),
      BRepInspector::CurveOnSurface(TopoDS::Edge(aGenE), TopoDS::Face(aGenF), First, Last),
      TopoDS::Face(aNewFace),
      Precision::PConfusion());
  }
}

//=================================================================================================

void BRepSweep_Translation::SetGeneratingPCurve(const TopoShape& aNewFace,
                                                TopoShape&       aNewEdge,
                                                const TopoShape&,
                                                const Sweep_NumShape&,
                                                const Sweep_NumShape&    aDirV,
                                                const TopAbs_Orientation orien)
{
  TopLoc_Location     Loc;
  GeomAdaptor_Surface AS(BRepInspector::Surface(TopoDS::Face(aNewFace), Loc));
  //  Standard_Real First,Last;
  gp_Lin2d    L;
  TopoEdge aNewOrientedEdge = TopoDS::Edge(aNewEdge);
  aNewOrientedEdge.Orientation(orien);

  if (AS.GetType() == GeomAbs_Plane)
  {
    /* nothing is done JAG
        gp_Pln pln = AS.Plane();
        gp_Ax3 ax3 = pln.Position();

    // JYL : the following produces bugs on an edge constructed from a trimmed 3D curve :
    //
    //    Handle(GeomLine)
    //      GL = Handle(GeomLine)::DownCast(BRepInspector::Curve(TopoDS::Edge(aGenE),
    //							Loc,First,Last));
    //    gp_Lin gl = GL->Lin();
    //    gl.Transform(Loc.Transformation());
    //
    // correction :
        const TopoEdge& EE = TopoDS::Edge(aGenE);
        BRepAdaptor_Curve BRAC(EE);
        gp_Lin gl = BRAC.Line();

        if(aDirV.Index()==2) gl.Translate(myVec);
        Point3d pnt = gl.Location();
        Dir3d dir = gl.Direction();
        Standard_Real u,v;
        ElSLib1::PlaneParameters(ax3,pnt,u,v);
        gp_Pnt2d pnt2d(u,v);
        gp_Dir2d dir2d(dir.Dot(ax3.XDirection()),dir.Dot(ax3.YDirection()));
        L.SetLocation(pnt2d);
        L.SetDirection(dir2d);
    */
  }
  else
  {
    Standard_Real v = 0;
    if (aDirV.Index() == 2)
      v = -myVec.Magnitude();
    L.SetLocation(gp_Pnt2d(0, v));
    L.SetDirection(gp_Dir2d(1, 0));
    //  }
    Handle(Geom2d_Line) GL = new Geom2d_Line(L);
    SetThePCurve(myBuilder.Builder(), TopoDS::Edge(aNewEdge), TopoDS::Face(aNewFace), orien, GL);
  }
}

//=================================================================================================

void BRepSweep_Translation::SetDirectingPCurve(const TopoShape& aNewFace,
                                               TopoShape&       aNewEdge,
                                               const TopoShape& aGenE,
                                               const TopoShape& aGenV,
                                               const Sweep_NumShape&,
                                               const TopAbs_Orientation orien)
{
  TopLoc_Location     Loc;
  GeomAdaptor_Surface AS(BRepInspector::Surface(TopoDS::Face(aNewFace), Loc));
  gp_Lin2d            L;
  if (AS.GetType() != GeomAbs_Plane)
  {
    L.SetLocation(gp_Pnt2d(BRepInspector::Parameter(TopoDS::Vertex(aGenV), TopoDS::Edge(aGenE)), 0));
    L.SetDirection(gp_Dir2d(0, -1));
    /* JAG
      }
      else{

        gp_Pln pln = AS.Plane();
        gp_Ax3 ax3 = pln.Position();
        Point3d pv = BRepInspector::Pnt(TopoDS::Vertex(aGenV));
        Dir3d dir(myVec);
        Standard_Real u,v;
        ElSLib1::PlaneParameters(ax3,pv,u,v);
        gp_Pnt2d pnt2d(u,v);
        gp_Dir2d dir2d(dir.Dot(ax3.XDirection()),dir.Dot(ax3.YDirection()));
        L.SetLocation(pnt2d);
        L.SetDirection(dir2d);

      }
    */
    Handle(Geom2d_Line) GL = new Geom2d_Line(L);
    SetThePCurve(myBuilder.Builder(), TopoDS::Edge(aNewEdge), TopoDS::Face(aNewFace), orien, GL);
  }
}

//=================================================================================================

TopAbs_Orientation BRepSweep_Translation::DirectSolid(const TopoShape& aGenS,
                                                      const Sweep_NumShape&)
{
  // compare the face normal and the direction
  BRepAdaptor_Surface surf(TopoDS::Face(aGenS));
  Point3d              P;
  Vector3d              du, dv;
  surf.D1((surf.FirstUParameter() + surf.LastUParameter()) / 2.,
          (surf.FirstVParameter() + surf.LastVParameter()) / 2.,
          P,
          du,
          dv);

  Standard_Real      x      = myVec.DotCross(du, dv);
  TopAbs_Orientation orient = (x > 0) ? TopAbs_REVERSED : TopAbs_FORWARD;
  return orient;
}

//=================================================================================================

Standard_Boolean BRepSweep_Translation::GGDShapeIsToAdd(const TopoShape&,
                                                        const TopoShape&,
                                                        const TopoShape&,
                                                        const TopoShape&,
                                                        const Sweep_NumShape&) const
{
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepSweep_Translation::GDDShapeIsToAdd(const TopoShape&,
                                                        const TopoShape&,
                                                        const TopoShape&,
                                                        const Sweep_NumShape&,
                                                        const Sweep_NumShape&) const
{
  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepSweep_Translation::SeparatedWires(const TopoShape&,
                                                       const TopoShape&,
                                                       const TopoShape&,
                                                       const TopoShape&,
                                                       const Sweep_NumShape&) const
{
  return Standard_False;
}

//=================================================================================================

Standard_Boolean BRepSweep_Translation::HasShape(const TopoShape&   aGenS,
                                                 const Sweep_NumShape& aDirS) const
{
  if (myDirShapeTool.Type(aDirS) == TopAbs_EDGE)
  {

    if (myGenShapeTool.Type(aGenS) == TopAbs_EDGE)
    {
      TopoEdge E = TopoDS::Edge(aGenS);

      // check if the edge is degenerated
      if (BRepInspector::Degenerated(E))
      {
        return Standard_False;
      }
      // check if the edge is a sewing edge

      //  modified by NIZHNY-EAP Fri Dec 24 11:13:09 1999 ___BEGIN___
      //      const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*) &E.TShape());

      //      BRep_ListOfCurveRepresentation& lcr = TE->ChangeCurves();
      //      BRep_ListIteratorOfListOfCurveRepresentation itcr(lcr);

      //      while (itcr.More()) {
      //	const Handle(BRep_CurveRepresentation)& cr = itcr.Value();
      //	if (cr->IsCurveOnSurface() &&
      //	    cr->IsCurveOnClosedSurface() ) 	{
      //	  std::cout<<"sewing edge"<<std::endl;
      //	  return Standard_False;
      //	}
      //	itcr.Next();
      //      }
      ShapeExplorer FaceExp(myGenShape, TopAbs_FACE);
      for (; FaceExp.More(); FaceExp.Next())
      {
        TopoFace F = TopoDS::Face(FaceExp.Current());
        if (BRepTools1::IsReallyClosed(E, F))
          return Standard_False;
      }
      //  modified by NIZHNY-EAP Fri Dec 24 11:13:21 1999 ___END___
    }
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean BRepSweep_Translation::IsInvariant(const TopoShape&) const
{
  return Standard_False;
}

//=================================================================================================

Vector3d BRepSweep_Translation::Vec() const
{
  return myVec;
}
