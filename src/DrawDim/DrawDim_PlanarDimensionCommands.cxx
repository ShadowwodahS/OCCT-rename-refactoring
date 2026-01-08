// Created on: 1996-06-03
// Created by: Denis PASCAL
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

#include <stdio.h>

#include <DrawDim.hxx>

#include <Draw_Interpretor.hxx>
#include <Draw_Appli.hxx>
#include <Draw.hxx>
#include <DBRep.hxx>
#include <DrawDim_PlanarDistance.hxx>
#include <DrawDim_PlanarRadius.hxx>
#include <DrawDim_PlanarAngle.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <Geom_Circle.hxx>
#include <BRep_Builder.hxx>
#include <gp_Pnt.hxx>
#include <gp_Circ.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopExp_Explorer.hxx>
#include <gp_Pln.hxx>
#include <DrawTrSurf.hxx>

//=================================================================================================

static Standard_Integer DrawDim_DISTANCE(DrawInterpreter& di,
                                         Standard_Integer  nb,
                                         const char**      arg)
{
  if (nb == 1)
  {
    di << "distance (name, profile[face],point1[vertex],point2[vertex])\n";
    di << "distance (name, profile[face],line1[edge],line2[edge])\n";
    di << "distance (name, profile[face],line[edge],point[vertex])\n";
  }
  else
  {
    Handle(DrawDim_PlanarDistance) DIST;
    if (nb == 5)
    {
      TopoShape aLocalShape = DBRep1::Get(arg[2], TopAbs_FACE);
      TopoFace  plan        = TopoDS::Face(aLocalShape);
      //      TopoFace plan =  TopoDS::Face(DBRep1::Get(arg[2],TopAbs_FACE));
      TopoShape geom1 = DBRep1::Get(arg[3]);
      TopoShape geom2 = DBRep1::Get(arg[4]);
      if (!plan.IsNull() && !geom1.IsNull() && !geom2.IsNull())
      {
        if (geom1.ShapeType() == TopAbs_VERTEX && geom2.ShapeType() == TopAbs_VERTEX)
        {
          DIST = new DrawDim_PlanarDistance(TopoDS::Vertex(geom1), TopoDS::Vertex(geom2));
        }
        else if (geom1.ShapeType() == TopAbs_VERTEX && geom2.ShapeType() == TopAbs_EDGE)
        {
          DIST = new DrawDim_PlanarDistance(TopoDS::Vertex(geom1), TopoDS::Edge(geom2));
        }
        else if (geom1.ShapeType() == TopAbs_EDGE && geom2.ShapeType() == TopAbs_EDGE)
        {
          DIST = new DrawDim_PlanarDistance(TopoDS::Edge(geom1), TopoDS::Edge(geom2));
        }
        if (!DIST.IsNull())
        {
          Draw1::Set(arg[1], DIST);
          return 0;
        }
      }
    }
  }
  di << "DrawDim_DISTANCE : error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer DrawDim_ANGLE(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb == 1)
  {
    di << "angle (name, profile[face],line1[edge],line2[edge])\n";
  }
  else
  {
    Handle(DrawDim_PlanarAngle) DIST;
    if (nb == 5)
    {
      TopoShape aLocalShape = DBRep1::Get(arg[2], TopAbs_FACE);
      TopoFace  plan        = TopoDS::Face(aLocalShape);
      aLocalShape              = DBRep1::Get(arg[3], TopAbs_EDGE);
      TopoEdge line1        = TopoDS::Edge(aLocalShape);
      aLocalShape              = DBRep1::Get(arg[4], TopAbs_EDGE);
      TopoEdge line2        = TopoDS::Edge(aLocalShape);
      //      TopoFace plan =  TopoDS::Face(DBRep1::Get(arg[2],TopAbs_FACE));
      //      TopoEdge line1 = TopoDS::Edge(DBRep1::Get(arg[3],TopAbs_EDGE));
      //      TopoEdge line2 = TopoDS::Edge(DBRep1::Get(arg[4],TopAbs_EDGE));
      if (!plan.IsNull() && !line1.IsNull() && !line2.IsNull())
      {
        DIST = new DrawDim_PlanarAngle(plan, line1, line2);
      }
      if (!DIST.IsNull())
      {
        Draw1::Set(arg[1], DIST);
        return 0;
      }
    }
  }
  di << "DrawDim_PlanarAngle : error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer DrawDim_RADIUS(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb == 1)
  {
    di << "radius (name, profile[face],cercle[edge])\n";
  }
  else
  {
    Handle(DrawDim_PlanarRadius) DIST;
    if (nb == 4)
    {
      TopoShape aLocalShape = DBRep1::Get(arg[2], TopAbs_FACE);
      TopoFace  plan        = TopoDS::Face(aLocalShape);
      aLocalShape              = DBRep1::Get(arg[3], TopAbs_EDGE);
      TopoEdge cercle       = TopoDS::Edge(aLocalShape);
      //      TopoFace plan =  TopoDS::Face(DBRep1::Get(arg[2],TopAbs_FACE));
      //      TopoEdge cercle = TopoDS::Edge(DBRep1::Get(arg[3],TopAbs_EDGE));
      if (!plan.IsNull() && !cercle.IsNull())
      {
        DIST = new DrawDim_PlanarRadius(cercle);
      }
    }
    if (!DIST.IsNull())
    {
      Draw1::Set(arg[1], DIST);
      return 0;
    }
  }
  di << "DrawDim_PlanarRadius : error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer DrawDim_CENTER(DrawInterpreter& di, Standard_Integer nb, const char** arg)
{
  if (nb == 3)
  {
    TopoShape aLocalShape = DBRep1::Get(arg[2], TopAbs_EDGE);
    TopoEdge  edge        = TopoDS::Edge(aLocalShape);
    //    TopoEdge edge = TopoDS::Edge(DBRep1::Get(arg[2],TopAbs_EDGE));
    Standard_Real      f, l;
    Handle(GeomCurve3d) curve = BRepInspector::Curve(edge, f, l);
    if (curve->IsKind(STANDARD_TYPE(GeomCircle)))
    {
      gp_Circ circle = Handle(GeomCircle)::DownCast(curve)->Circ();
      Point3d  center = circle.Location();
      //: abv: avoid dependence on TKTopAlgo
      TopoVertex vc;
      //      = BRepBuilderAPI_MakeVertex (center);
      ShapeBuilder B;
      B.MakeVertex(vc, center, Precision1::Confusion());
      DBRep1::Set(arg[1], vc);
      return 0;
    }
  }
  di << "DrawDim_CENTER : error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer DrawDim_VARIABLES(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n != 2)
    return 1;
  TopoShape aLocalShape = DBRep1::Get(a[1], TopAbs_FACE);
  TopoFace  F           = TopoDS::Face(aLocalShape);
  //  TopoFace F = TopoDS::Face(DBRep1::Get(a[1],TopAbs_FACE));
  if (F.IsNull())
    return 0;

  Standard_Integer    i = 0;
  TopoVertex       vf, vl;
  TopTools_MapOfShape M;
  M.Add(F);
  ShapeExplorer ex(F, TopAbs_EDGE);
  while (ex.More())
  {
    if (M.Add(ex.Current()))
    {
      TopExp1::Vertices(TopoDS::Edge(ex.Current()), vf, vl);
      if (M.Add(vf))
      {
        i++;
        char* p = (char*)malloc(100);
        Sprintf(p, "%s_%dv", a[1], i);
        DBRep1::Set(p, vf);
        di.AppendElement(p);
        DrawDim1::DrawShapeName(vf, p);
      }
      if (M.Add(vl))
      {
        i++;
        char* p = (char*)malloc(100);
        Sprintf(p, "%s_%dv", a[1], i);
        DBRep1::Set(p, vl);
        di.AppendElement(p);
        DrawDim1::DrawShapeName(vl, p);
      }
      i++;
      char* p = (char*)malloc(100);
      Sprintf(p, "%s_%de", a[1], i);
      DBRep1::Set(p, ex.Current());
      di.AppendElement(p);
      DrawDim1::DrawShapeName(ex.Current(), p);
    }
    ex.Next();
  }
  return 0;
}

//=================================================================================================

static Standard_Integer DrawDim_SPLACEMENT(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n == 4)
  {
    TopoShape shape       = DBRep1::Get(a[1]);
    TopoShape aLocalShape = DBRep1::Get(a[2], TopAbs_FACE);
    TopoFace  from        = TopoDS::Face(aLocalShape);
    aLocalShape              = DBRep1::Get(a[3], TopAbs_FACE);
    TopoFace to           = TopoDS::Face(aLocalShape);
    //    TopoFace from = TopoDS::Face(DBRep1::Get(a[2],TopAbs_FACE));
    //    TopoFace to = TopoDS::Face(DBRep1::Get(a[3],TopAbs_FACE));
    if (!shape.IsNull() && !from.IsNull() && !to.IsNull())
    {
      gp_Pln pfrom, pto;
      DrawDim1::Pln(from, pfrom);
      DrawDim1::Pln(to, pto);
      Ax3  axfrom(pfrom.Position1());
      Ax3  axto(pto.Position1());
      Transform3d trsf;
      trsf.SetDisplacement(axfrom, axto);
      TopLoc_Location move(trsf);
      shape.Move(move);
      DBRep1::Set(a[1], shape);
      return 0;
    }
  }
  di << "DrawDim_SPlacement : error\n";
  return 1;
}

//=================================================================================================

static Standard_Integer DrawDim_GPLACEMENT(DrawInterpreter& di, Standard_Integer n, const char** a)
{
  if (n == 4)
  {
    Handle(Geom_Geometry) geom        = DrawTrSurf1::Get(a[1]);
    TopoShape          aLocalShape = DBRep1::Get(a[2], TopAbs_FACE);
    TopoFace           from        = TopoDS::Face(aLocalShape);
    aLocalShape                       = DBRep1::Get(a[3], TopAbs_FACE);
    TopoFace to                    = TopoDS::Face(aLocalShape);
    //    TopoFace from = TopoDS::Face(DBRep1::Get(a[2],TopAbs_FACE));
    //    TopoFace to = TopoDS::Face(DBRep1::Get(a[3],TopAbs_FACE));
    if (!geom.IsNull() && !from.IsNull() && !to.IsNull())
    {
      gp_Pln pfrom, pto;
      DrawDim1::Pln(from, pfrom);
      DrawDim1::Pln(to, pto);
      Ax3  axfrom(pfrom.Position1());
      Ax3  axto(pto.Position1());
      Transform3d trsf;
      trsf.SetDisplacement(axfrom, axto);
      Handle(Geom_Geometry) newgeom = geom->Transformed(trsf);
      DrawTrSurf1::Set(a[1], newgeom);
      return 0;
    }
  }
  di << "DrawDim_Placement : error\n";
  return 1;
}

//=================================================================================================

void DrawDim1::PlanarDimensionCommands(DrawInterpreter& theCommands)
{
  // syntaxes
  theCommands.Add("distance", "distance,no args to get help", __FILE__, DrawDim_DISTANCE);

  theCommands.Add("radius", "radius, no args to get help", __FILE__, DrawDim_RADIUS);

  theCommands.Add("angle", "angle, no args to get help", __FILE__, DrawDim_ANGLE);

  theCommands.Add("center",
                  "to extract center of a circle : center ,name, circle",
                  __FILE__,
                  DrawDim_CENTER);

  theCommands.Add("variables", "to extract variables of a face", __FILE__, DrawDim_VARIABLES);

  theCommands.Add("splacement", "to move shape from face to face", __FILE__, DrawDim_SPLACEMENT);

  theCommands.Add("gplacement", "to move geometry from face to face", __FILE__, DrawDim_GPLACEMENT);
}
