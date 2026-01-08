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

// 25.12.98 pdn: adding empty constructor

#include <BRep_TEdge.hxx>
#include <BRep_TFace.hxx>
#include <BRep_Tool.hxx>
#include <BRep_TVertex.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>

//=================================================================================================

ShapeTolerance1::ShapeTolerance1() {}

//=================================================================================================

Standard_Boolean ShapeTolerance1::LimitTolerance(const TopoShape&    shape,
                                                         const Standard_Real    tmin,
                                                         const Standard_Real    tmax,
                                                         const TopAbs_ShapeEnum styp) const
{
  if (shape.IsNull() || tmin < 0)
    return Standard_False;
  Standard_Boolean iamax = (tmax >= tmin);
  Standard_Real    prec;
  Standard_Boolean fait = Standard_False;
  if (styp == TopAbs_VERTEX || styp == TopAbs_EDGE || styp == TopAbs_FACE)
  {
    for (ShapeExplorer ex(shape, styp); ex.More(); ex.Next())
    {
      TopoShape sh     = ex.Current();
      int          newtol = 0;
      if (styp == TopAbs_VERTEX)
      {
        TopoVertex V = TopoDS::Vertex(sh);
        prec            = BRepInspector::Tolerance(V);
        if (iamax && prec > tmax)
          newtol = 1;
        else if (prec < tmin)
          newtol = -1;
        if (newtol)
        {
          const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V.TShape());
          TV->Tolerance((newtol > 0 ? tmax : tmin));
          fait = Standard_True;
        }
      }
      else if (styp == TopAbs_EDGE)
      {
        TopoEdge E = TopoDS::Edge(sh);
        prec          = BRepInspector::Tolerance(E);
        if (iamax && prec > tmax)
          newtol = 1;
        else if (prec < tmin)
          newtol = -1;
        if (newtol)
        {
          const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
          TE->Tolerance((newtol > 0 ? tmax : tmin));
          fait = Standard_True;
        }
      }
      else if (styp == TopAbs_FACE)
      {
        TopoFace F = TopoDS::Face(sh);
        prec          = BRepInspector::Tolerance(F);
        if (iamax && prec > tmax)
          newtol = 1;
        else if (prec < tmin)
          newtol = -1;
        if (newtol)
        {
          const Handle(BRep_TFace)& TF = *((Handle(BRep_TFace)*)&F.TShape());
          TF->Tolerance((newtol > 0 ? tmax : tmin));
          fait = Standard_True;
        }
      }
    }
  }
  else if (styp == TopAbs_WIRE)
  {
    for (ShapeExplorer ex(shape, TopAbs_EDGE); ex.More(); ex.Next())
    {
      TopoShape sh = ex.Current();
      TopoEdge  E  = TopoDS::Edge(sh);
      LimitTolerance(E, tmin, tmax, TopAbs_EDGE);
      TopoVertex V1, V2;
      TopExp1::Vertices(E, V1, V2);
      if (!V1.IsNull())
        fait |= LimitTolerance(V1, tmin, tmax, TopAbs_VERTEX);
      if (!V2.IsNull())
        fait |= LimitTolerance(V2, tmin, tmax, TopAbs_VERTEX);
    }
  }
  else
  {
    fait |= LimitTolerance(shape, tmin, tmax, TopAbs_VERTEX);
    fait |= LimitTolerance(shape, tmin, tmax, TopAbs_EDGE);
    fait |= LimitTolerance(shape, tmin, tmax, TopAbs_FACE);
  }
  return fait;
}

//=================================================================================================

void ShapeTolerance1::SetTolerance(const TopoShape&    shape,
                                           const Standard_Real    preci,
                                           const TopAbs_ShapeEnum styp) const
{
  //   VERTEX ou EDGE ou FACE : ces types seulement
  //   WIRE : EDGE + VERTEX
  //   Autres : TOUT (donc == WIRE + FACE)
  if (shape.IsNull() || preci <= 0)
    return;
  if (styp == TopAbs_VERTEX || styp == TopAbs_EDGE || styp == TopAbs_FACE)
  {
    for (ShapeExplorer ex(shape, styp); ex.More(); ex.Next())
    {
      TopoShape sh = ex.Current();
      if (styp == TopAbs_VERTEX)
      {
        TopoVertex V = TopoDS::Vertex(sh);
        //	B.UpdateVertex (V,preci);
        const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V.TShape());
        TV->Tolerance(preci);
      }
      else if (styp == TopAbs_EDGE)
      {
        TopoEdge E = TopoDS::Edge(sh);
        //	B.UpdateEdge   (E,preci);
        const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
        TE->Tolerance(preci);
      }
      else if (styp == TopAbs_FACE)
      {
        TopoFace F = TopoDS::Face(sh);
        //	B.UpdateFace   (F,preci);
        const Handle(BRep_TFace)& TF = *((Handle(BRep_TFace)*)&F.TShape());
        TF->Tolerance(preci);
      }
    }
  }
  else if (styp == TopAbs_WIRE)
  {
    for (ShapeExplorer ex(shape, TopAbs_EDGE); ex.More(); ex.Next())
    {
      TopoShape sh = ex.Current();
      TopoEdge  E  = TopoDS::Edge(sh);
      //      B.UpdateEdge   (E,preci);
      const Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());
      TE->Tolerance(preci);
      TopoVertex V1, V2;
      TopExp1::Vertices(E, V1, V2);
      if (!V1.IsNull())
      {
        //	B.UpdateVertex (V1,preci);
        const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V1.TShape());
        TV->Tolerance(preci);
      }
      if (!V2.IsNull())
      {
        //	B.UpdateVertex (V2,preci);
        const Handle(BRep_TVertex)& TV = *((Handle(BRep_TVertex)*)&V2.TShape());
        TV->Tolerance(preci);
      }
    }
  }
  else
  {
    SetTolerance(shape, preci, TopAbs_VERTEX);
    SetTolerance(shape, preci, TopAbs_EDGE);
    SetTolerance(shape, preci, TopAbs_FACE);
  }
}
