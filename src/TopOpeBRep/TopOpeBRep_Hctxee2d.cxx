// Created on: 1998-10-29
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

#include <BRep_Tool.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <Standard_Type.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopOpeBRep_define.hxx>
#include <TopOpeBRep_Hctxee2d.hxx>
#include <TopOpeBRepTool_2d.hxx>
#include <TopOpeBRepTool_CurveTool.hxx>
#include <TopOpeBRepTool_GEOMETRY.hxx>
#include <TopOpeBRepTool_PROJECT.hxx>

IMPLEMENT_STANDARD_RTTIEXT(TopOpeBRep_Hctxee2d, RefObject)

#ifdef OCCT_DEBUG
  #include <GeomTools_SurfaceSet.hxx>
  #include <GeomTools_CurveSet.hxx>
  #include <GeomTools_Curve2dSet.hxx>
  #include <Geom_Curve.hxx>
Standard_EXPORT Standard_Boolean TopOpeBRep_GettracePROEDG();
#endif

//=================================================================================================

TopOpeBRep_Hctxee2d::TopOpeBRep_Hctxee2d() {}

//=================================================================================================

void TopOpeBRep_Hctxee2d::SetEdges(const TopoEdge&         E1,
                                   const TopoEdge&         E2,
                                   const BRepAdaptor_Surface& BAS1,
                                   const BRepAdaptor_Surface& BAS2)
{
  const TopoFace&  F1  = BAS1.Face();
  GeomAbs_SurfaceType ST1 = BAS1.GetType();
  const TopoFace&  F2  = BAS2.Face();

  myEdge1 = TopoDS::Edge(E1);
  myEdge2 = TopoDS::Edge(E2);

  Standard_Real first, last, tole, tolpc;
  gp_Pnt2d      pfirst, plast;

  Handle(GeomCurve2d) PC1;
  PC1 = FC2D_CurveOnSurface(myEdge1, F1, first, last, tolpc);
  if (PC1.IsNull())
    throw ExceptionBase("TopOpeBRep_Hctxee2d::SetEdges : no 2d curve");
  myCurve1.Load(PC1);
  BRepInspector::UVPoints(myEdge1, F1, pfirst, plast);
  tole = BRepInspector::Tolerance(myEdge1);
  myDomain1.SetValues(pfirst, first, tole, plast, last, tole);

#ifdef OCCT_DEBUG
  Standard_Boolean trc = Standard_False;
  if (trc)
  {
    std::cout << "ed1 on fa1 : {pfirst=(" << pfirst.X() << " " << pfirst.Y() << "),first=" << first
              << "\n";
    std::cout << "              plast =(" << plast.X() << " " << plast.Y() << "),last=" << last
              << "}" << std::endl;
  }
#endif

  Standard_Boolean           memesfaces  = F1.IsSame(F2);
  Standard_Boolean           memesupport = Standard_False;
  TopLoc_Location            L1, L2;
  const Handle(GeomSurface) S1 = BRepInspector::Surface(F1, L1);
  const Handle(GeomSurface) S2 = BRepInspector::Surface(F2, L2);
  if (S1 == S2 && L1 == L2)
    memesupport = Standard_True;

  if (ST1 == GeomAbs_Plane || memesfaces || memesupport)
  {
    Handle(GeomCurve2d) PC2 = FC2D_CurveOnSurface(myEdge2, F1, first, last, tolpc);
    myCurve2.Load(PC2);
    BRepInspector::UVPoints(myEdge2, F1, pfirst, plast);
    tole = BRepInspector::Tolerance(myEdge2);
    myDomain2.SetValues(pfirst, first, tole, plast, last, tole);

#ifdef OCCT_DEBUG
    if (trc)
    {
      std::cout << "ed2 on fa1 : {pfirst=(" << pfirst.X() << " " << pfirst.Y()
                << "),first=" << first << "\n";
      std::cout << "              plast =(" << plast.X() << " " << plast.Y() << "),last=" << last
                << "}" << std::endl;
    }
#endif
  }
  else
  {

    Handle(GeomCurve2d) PC2on1;
    Handle(GeomCurve3d)   NC;
    Standard_Boolean     dgE2 = BRepInspector::Degenerated(myEdge2);
    if (dgE2)
    { // xpu210998 : cto900Q3
      ShapeExplorer      exv(myEdge2, TopAbs_VERTEX);
      const TopoVertex& v2  = TopoDS::Vertex(exv.Current());
      Point3d               pt2 = BRepInspector::Pnt(v2);
      gp_Pnt2d             uv2;
      Standard_Real        d;
      Standard_Boolean     ok = FUN_tool_projPonF(pt2, F1, uv2, d);
      if (!ok)
        return; // nyiRaise
      Handle(GeomSurface) aS1  = BRepInspector::Surface(F1);
      Standard_Boolean     apex = FUN_tool_onapex(uv2, aS1);
      if (apex)
      {
        TopoVertex vf, vl;
        TopExp1::Vertices(myEdge1, vf, vl);
        Point3d                                    ptf  = BRepInspector::Pnt(vf);
        Standard_Real                             df   = pt2.Distance(ptf);
        Standard_Real                             tolf = BRepInspector::Tolerance(vf);
        Standard_Boolean                          onf  = (df < tolf);
        TopoVertex                             v1   = onf ? vf : vl;
        TopTools_IndexedDataMapOfShapeListOfShape mapVE;
        TopExp1::MapShapesAndAncestors(F1, TopAbs_VERTEX, TopAbs_EDGE, mapVE);
        const ShapeList&        Edsanc = mapVE.FindFromKey(v1);
        TopTools_ListIteratorOfListOfShape it(Edsanc);
        for (; it.More(); it.Next())
        {
          const TopoEdge& ee   = TopoDS::Edge(it.Value());
          Standard_Boolean   dgee = BRepInspector::Degenerated(ee);
          if (!dgee)
            continue;
          PC2on1 = BRepInspector::CurveOnSurface(ee, F1, first, last);
        }
      }
      else
      {
      } // NYIxpu210998
    } // dgE2
    else
    {
      // project curve of edge 2 on surface of face 1
      TopLoc_Location    loc;
      Handle(GeomCurve3d) C = BRepInspector::Curve(myEdge2, loc, first, last);
      NC                   = Handle(GeomCurve3d)::DownCast(C->Transformed(loc.Transformation()));
      Standard_Real tolreached2d;
      PC2on1 = TopOpeBRepTool_CurveTool::MakePCurveOnFace(F1, NC, tolreached2d);
    }

    if (!PC2on1.IsNull())
    {
      myCurve2.Load(PC2on1);
      tole = BRepInspector::Tolerance(myEdge2);
      PC2on1->D0(first, pfirst);
      PC2on1->D0(last, plast);
      myDomain2.SetValues(pfirst, first, tole, plast, last, tole);
#ifdef OCCT_DEBUG
      if (TopOpeBRep_GettracePROEDG())
      {
        std::cout << "------------ projection de curve" << std::endl;
        std::cout << "--- Curve : " << std::endl;
        GeomTools_CurveSet::PrintCurve(NC, std::cout);
        std::cout << "--- nouvelle PCurve : " << std::endl;
        GeomTools_Curve2dSet::PrintCurve2d(PC2on1, std::cout);
        Handle(GeomSurface) aS1 = BRepInspector::Surface(F1);
        std::cout << "--- sur surface : " << std::endl;
        GeomTools_SurfaceSet::PrintSurface(aS1, std::cout);
        std::cout << std::endl;
      }
#endif
    }
  }

} // SetEdges

//=================================================================================================

const TopoShape& TopOpeBRep_Hctxee2d::Edge(const Standard_Integer Index) const
{
  if (Index == 1)
    return myEdge1;
  else if (Index == 2)
    return myEdge2;
  else
    throw ExceptionBase("TopOpeBRep_Hctxee2d::Edge");
}

//=================================================================================================

const Geom2dAdaptor_Curve& TopOpeBRep_Hctxee2d::Curve(const Standard_Integer Index) const
{
  if (Index == 1)
    return myCurve1;
  else if (Index == 2)
    return myCurve2;
  else
    throw ExceptionBase("TopOpeBRep_Hctxee2d::Curve");
}

//=================================================================================================

const IntRes2d_Domain& TopOpeBRep_Hctxee2d::Domain(const Standard_Integer Index) const
{
  if (Index == 1)
    return myDomain1;
  else if (Index == 2)
    return myDomain2;
  else
    throw ExceptionBase("TopOpeBRep_Hctxee2d::Domain");
}
