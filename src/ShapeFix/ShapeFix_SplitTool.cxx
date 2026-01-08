// Created on: 2004-07-14
// Created by: Sergey KUUL
// Copyright (c) 2004-2014 OPEN CASCADE SAS
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

#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <gp_Pnt.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeAnalysis_Edge.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <ShapeAnalysis_TransferParametersProj.hxx>
#include <ShapeBuild_Edge.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeExtend_WireData.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_SplitTool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

//=================================================================================================

SplitTool::SplitTool() {}

//=================================================================================================

Standard_Boolean SplitTool::SplitEdge(const TopoEdge&   edge,
                                               const Standard_Real  param,
                                               const TopoVertex& vert,
                                               const TopoFace&   face,
                                               TopoEdge&         newE1,
                                               TopoEdge&         newE2,
                                               const Standard_Real  tol3d,
                                               const Standard_Real  tol2d) const
{
  Standard_Real        a, b;
  Edge1   sae;
  Handle(GeomCurve2d) c2d;
  sae.PCurve(edge, face, c2d, a, b, Standard_True);
  if (Abs(a - param) < tol2d || Abs(b - param) < tol2d)
    return Standard_False;
  // check distance between edge and new vertex
  Point3d          P1;
  TopLoc_Location L;
  if (BRepInspector::SameParameter(edge))
  {
    Standard_Real            f, l;
    const Handle(GeomCurve3d) c3d = BRepInspector::Curve(edge, L, f, l);
    if (c3d.IsNull())
      return Standard_False;
    P1 = c3d->Value(param);
    if (!L.IsIdentity())
      P1 = P1.Transformed(L.Transformation());
  }
  else
  {
    Handle(GeomSurface)          surf = BRepInspector::Surface(face, L);
    Handle(ShapeAnalysis_Surface) sas  = new ShapeAnalysis_Surface(surf);
    P1                                 = sas->Value(c2d->Value(param));
    if (!L.IsIdentity())
      P1 = P1.Transformed(L.Transformation());
  }
  Point3d P2 = BRepInspector::Pnt(vert);
  if (P1.Distance(P2) > tol3d)
  {
    // return Standard_False;
    ShapeBuilder B;
    B.UpdateVertex(vert, P1.Distance(P2));
  }

  Handle(ShapeAnalysis_TransferParametersProj) transferParameters =
    new ShapeAnalysis_TransferParametersProj;
  transferParameters->SetMaxTolerance(tol3d);
  transferParameters->Init(edge, face);
  Standard_Real first, last;
  if (a < b)
  {
    first = a;
    last  = b;
  }
  else
  {
    first = b;
    last  = a;
  }

  Edge2       sbe;
  Handle(ShapeFix_Edge) sfe    = new ShapeFix_Edge;
  TopAbs_Orientation    orient = edge.Orientation();
  ShapeBuilder          B;
  TopoEdge           wE = edge;
  wE.Orientation(TopAbs_FORWARD);
  TopoShape aTmpShape = vert.Oriented(TopAbs_REVERSED); // for porting
  newE1 = sbe.CopyReplaceVertices(wE, sae.FirstVertex(wE), TopoDS::Vertex(aTmpShape));
  sbe.CopyPCurves(newE1, wE);
  transferParameters->TransferRange(newE1, first, param, Standard_True);
  B.SameRange(newE1, Standard_False);
  sfe->FixSameParameter(newE1);
  // B.SameParameter(newE1,Standard_False);
  aTmpShape = vert.Oriented(TopAbs_FORWARD);
  newE2     = sbe.CopyReplaceVertices(wE, TopoDS::Vertex(aTmpShape), sae.LastVertex(wE));
  sbe.CopyPCurves(newE2, wE);
  transferParameters->TransferRange(newE2, param, last, Standard_True);
  B.SameRange(newE2, Standard_False);
  sfe->FixSameParameter(newE2);
  // B.SameParameter(newE2,Standard_False);

  newE1.Orientation(orient);
  newE2.Orientation(orient);
  if (orient == TopAbs_REVERSED)
  {
    TopoEdge tmp = newE2;
    newE2           = newE1;
    newE1           = tmp;
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean SplitTool::SplitEdge(const TopoEdge&   edge,
                                               const Standard_Real  param1,
                                               const Standard_Real  param2,
                                               const TopoVertex& vert,
                                               const TopoFace&   face,
                                               TopoEdge&         newE1,
                                               TopoEdge&         newE2,
                                               const Standard_Real  tol3d,
                                               const Standard_Real  tol2d) const
{
  Standard_Real param = (param1 + param2) / 2;
  if (SplitEdge(edge, param, vert, face, newE1, newE2, tol3d, tol2d))
  {
    // cut new edges by param1 and param2
    Standard_Boolean     IsCutLine;
    Handle(GeomCurve2d) Crv1, Crv2;
    Standard_Real        fp1, lp1, fp2, lp2;
    Edge1   sae;
    if (sae.PCurve(newE1, face, Crv1, fp1, lp1, Standard_False))
    {
      if (sae.PCurve(newE2, face, Crv2, fp2, lp2, Standard_False))
      {
        if (lp1 == param)
        {
          if ((lp1 - fp1) * (lp1 - param1) > 0)
          {
            CutEdge(newE1, fp1, param1, face, IsCutLine);
            CutEdge(newE2, lp2, param2, face, IsCutLine);
          }
          else
          {
            CutEdge(newE1, fp1, param2, face, IsCutLine);
            CutEdge(newE2, lp2, param1, face, IsCutLine);
          }
        }
        else
        {
          if ((fp1 - lp1) * (fp1 - param1) > 0)
          {
            CutEdge(newE1, lp1, param1, face, IsCutLine);
            CutEdge(newE2, fp2, param2, face, IsCutLine);
          }
          else
          {
            CutEdge(newE1, lp1, param2, face, IsCutLine);
            CutEdge(newE2, fp2, param1, face, IsCutLine);
          }
        }
      }
    }
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean SplitTool::CutEdge(const TopoEdge&  edge,
                                             const Standard_Real pend,
                                             const Standard_Real cut,
                                             const TopoFace&  face,
                                             Standard_Boolean&   iscutline) const
{
  if (Abs(cut - pend) < 10. * Precision1::PConfusion())
    return Standard_False;
  Standard_Real aRange = Abs(cut - pend);
  Standard_Real a, b;
  BRepInspector::Range(edge, a, b);
  iscutline = Standard_False;
  if (aRange < 10. * Precision1::PConfusion())
    return Standard_False;

  // case pcurve is trimm of line
  if (!BRepInspector::SameParameter(edge))
  {
    Edge1   sae;
    Handle(GeomCurve2d) Crv;
    Standard_Real        fp, lp;
    if (sae.PCurve(edge, face, Crv, fp, lp, Standard_False))
    {
      if (Crv->IsKind(STANDARD_TYPE(Geom2d_TrimmedCurve)))
      {
        Handle(Geom2d_TrimmedCurve) tc = Handle(Geom2d_TrimmedCurve)::DownCast(Crv);
        if (tc->BasisCurve()->IsKind(STANDARD_TYPE(Geom2d_Line)))
        {
          ShapeBuilder B;
          B.Range(edge, Min(pend, cut), Max(pend, cut));
          if (Abs(pend - lp) < Precision1::PConfusion())
          { // cut from the beginning
            Standard_Real cut3d = (cut - fp) * (b - a) / (lp - fp);
            if (cut3d <= Precision1::PConfusion())
              return Standard_False;
            B.Range(edge, a + cut3d, b, Standard_True);
            iscutline = Standard_True;
          }
          else if (Abs(pend - fp) < Precision1::PConfusion())
          { // cut from the end
            Standard_Real cut3d = (lp - cut) * (b - a) / (lp - fp);
            if (cut3d <= Precision1::PConfusion())
              return Standard_False;
            B.Range(edge, a, b - cut3d, Standard_True);
            iscutline = Standard_True;
          }
        }
      }
    }
    return Standard_True;
  }

  // det-study on 03/12/01 checking the old and new ranges
  if (Abs(Abs(a - b) - aRange) < Precision1::PConfusion())
    return Standard_False;
  if (aRange < 10. * Precision1::PConfusion())
    return Standard_False;

  Handle(GeomCurve3d)  c = BRepInspector::Curve(edge, a, b);
  Curve2 sac;
  a                = Min(pend, cut);
  b                = Max(pend, cut);
  Standard_Real na = a, nb = b;

  ShapeBuilder B;
  if (!BRepInspector::Degenerated(edge) && !c.IsNull()
      && sac.ValidateRange(c, na, nb, Precision1::PConfusion()) && (na != a || nb != b))
  {
    B.Range(edge, na, nb, Standard_True);
    Edge1 sae;
    if (sae.HasPCurve(edge, face))
    {
      B.SameRange(edge, Standard_False);
    }

    ShapeFix_Edge sfe;
    sfe.FixSameParameter(edge);
  }
  else
  {
    B.Range(edge, a, b, Standard_False);
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean SplitTool::SplitEdge(const TopoEdge&                edge,
                                               const Standard_Real               fp,
                                               const TopoVertex&              V1,
                                               const Standard_Real               lp,
                                               const TopoVertex&              V2,
                                               const TopoFace&                face,
                                               TopTools_SequenceOfShape&         SeqE,
                                               Standard_Integer&                 aNum,
                                               const Handle(ShapeBuild_ReShape)& context,
                                               const Standard_Real               tol3d,
                                               const Standard_Real               tol2d) const
{
  if (fabs(lp - fp) < tol2d)
    return Standard_False;
  aNum = 0;
  SeqE.Clear();
  ShapeBuilder         B;
  Standard_Real        a, b;
  Edge1   sae;
  Handle(GeomCurve2d) c2d;
  sae.PCurve(edge, face, c2d, a, b, Standard_True);
  TopoVertex VF    = sae.FirstVertex(edge);
  TopoVertex VL    = sae.LastVertex(edge);
  Standard_Real tolVF = BRepInspector::Tolerance(VF);
  Standard_Real tolVL = BRepInspector::Tolerance(VL);
  Standard_Real tolV1 = BRepInspector::Tolerance(V1);
  Standard_Real tolV2 = BRepInspector::Tolerance(V2);
  Point3d        PVF   = BRepInspector::Pnt(VF);
  Point3d        PVL   = BRepInspector::Pnt(VL);
  Point3d        PV1   = BRepInspector::Pnt(V1);
  Point3d        PV2   = BRepInspector::Pnt(V2);

  Standard_Real    par1, par2;
  Standard_Boolean IsReverse = Standard_False;
  if ((b - a) * (lp - fp) > 0)
  {
    par1 = fp;
    par2 = lp;
  }
  else
  {
    par1      = lp;
    par2      = fp;
    IsReverse = Standard_True;
  }

  if (fabs(a - par1) <= tol2d && fabs(b - par2) <= tol2d)
  {
    if (IsReverse)
    {
      Standard_Real newtol = tolVF + PVF.Distance(PV2);
      if (tolV2 < newtol)
        B.UpdateVertex(V2, newtol);
      if (VF.Orientation() == V2.Orientation())
      {
        context->Replace(VF, V2);
        VF = V2;
      }
      else
      {
        context->Replace(VF, V2.Reversed());
        VF = TopoDS::Vertex(V2.Reversed());
      }
      newtol = tolVL + PVL.Distance(PV1);
      if (tolV1 < newtol)
        B.UpdateVertex(V1, newtol);
      if (VL.Orientation() == V1.Orientation())
      {
        context->Replace(VL, V1);
        VL = V1;
      }
      else
      {
        context->Replace(VL, V1.Reversed());
        VL = TopoDS::Vertex(V1.Reversed());
      }
    }
    else
    {
      Standard_Real newtol = tolVF + PVF.Distance(PV1);
      if (tolV1 < newtol)
        B.UpdateVertex(V1, newtol);
      if (VF.Orientation() == V1.Orientation())
      {
        context->Replace(VF, V1);
        VF = V1;
      }
      else
      {
        context->Replace(VF, V1.Reversed());
        VF = TopoDS::Vertex(V1.Reversed());
      }
      newtol = tolVL + PVL.Distance(PV2);
      if (tolV2 < newtol)
        B.UpdateVertex(V2, newtol);
      if (VL.Orientation() == V2.Orientation())
      {
        context->Replace(VL, V2);
        VL = V2;
      }
      else
      {
        context->Replace(VL, V2.Reversed());
        VL = TopoDS::Vertex(V2.Reversed());
      }
    }
    SeqE.Append(edge);
    aNum = 1;
  }

  if (fabs(a - par1) <= tol2d && fabs(b - par2) > tol2d)
  {
    TopoEdge newE1, newE2;
    if (IsReverse)
    {
      if (!SplitEdge(edge, par2, V1, face, newE1, newE2, tol3d, tol2d))
        return Standard_False;
      Standard_Real newtol = tolVF + PVF.Distance(PV2);
      if (tolV2 < newtol)
        B.UpdateVertex(V2, newtol);
      if (VF.Orientation() == V2.Orientation())
      {
        context->Replace(VF, V2);
        VF = V2;
      }
      else
      {
        context->Replace(VF, V2.Reversed());
        VF = TopoDS::Vertex(V2.Reversed());
      }
    }
    else
    {
      if (!SplitEdge(edge, par2, V2, face, newE1, newE2, tol3d, tol2d))
        return Standard_False;
      Standard_Real newtol = tolVF + PVF.Distance(PV1);
      if (tolV1 < newtol)
        B.UpdateVertex(V1, newtol);
      if (VF.Orientation() == V1.Orientation())
      {
        context->Replace(VF, V1);
        VF = V1;
      }
      else
      {
        context->Replace(VF, V1.Reversed());
        VF = TopoDS::Vertex(V1.Reversed());
      }
    }
    SeqE.Append(newE1);
    SeqE.Append(newE2);
    aNum = 1;
  }

  if (fabs(a - par1) > tol2d && fabs(b - par2) <= tol2d)
  {
    TopoEdge newE1, newE2;
    if (IsReverse)
    {
      if (!SplitEdge(edge, par1, V2, face, newE1, newE2, tol3d, tol2d))
        return Standard_False;
      Standard_Real newtol = tolVL + PVL.Distance(PV1);
      if (tolV1 < newtol)
        B.UpdateVertex(V1, newtol);
      if (VL.Orientation() == V1.Orientation())
      {
        context->Replace(VL, V1);
        VL = V1;
      }
      else
      {
        context->Replace(VL, V1.Reversed());
        VL = TopoDS::Vertex(V1.Reversed());
      }
    }
    else
    {
      if (!SplitEdge(edge, par1, V1, face, newE1, newE2, tol3d, tol2d))
        return Standard_False;
      Standard_Real newtol = tolVL + PVL.Distance(PV2);
      if (tolV2 < newtol)
        B.UpdateVertex(V2, newtol);
      if (VL.Orientation() == V2.Orientation())
      {
        context->Replace(VL, V2);
        VL = V2;
      }
      else
      {
        context->Replace(VL, V2.Reversed());
        VL = TopoDS::Vertex(V2.Reversed());
      }
    }
    SeqE.Append(newE1);
    SeqE.Append(newE2);
    aNum = 2;
  }

  if (fabs(a - par1) > tol2d && fabs(b - par2) > tol2d)
  {
    TopoEdge newE1, newE2, newE3, newE4;
    if (IsReverse)
    {
      if (!SplitEdge(edge, par1, V2, face, newE1, newE2, tol3d, tol2d))
        return Standard_False;
      if (!SplitEdge(newE2, par2, V1, face, newE3, newE4, tol3d, tol2d))
        return Standard_False;
    }
    else
    {
      if (!SplitEdge(edge, par1, V1, face, newE1, newE2, tol3d, tol2d))
        return Standard_False;
      if (!SplitEdge(newE2, par2, V2, face, newE3, newE4, tol3d, tol2d))
        return Standard_False;
    }
    SeqE.Append(newE1);
    SeqE.Append(newE3);
    SeqE.Append(newE4);
    aNum = 2;
  }

  if (aNum == 0)
    return Standard_False;

  Handle(ShapeExtend_WireData) sewd = new ShapeExtend_WireData;
  for (Standard_Integer i = 1; i <= SeqE.Length(); i++)
  {
    sewd->Add(SeqE.Value(i));
  }
  context->Replace(edge, sewd->Wire());
  for (ShapeExplorer exp(sewd->Wire(), TopAbs_EDGE); exp.More(); exp.Next())
  {
    TopoEdge E = TopoDS::Edge(exp.Current());
    BRepTools1::Update(E);
  }

  return Standard_True;
}
