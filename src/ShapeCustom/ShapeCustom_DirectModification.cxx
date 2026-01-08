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

//: l8 abv 15.01.99: CTS22022: writing correct pcurves for indirect tori
//: p5 abv 26.02.99: PRO18207: force copying of edge if any its pcurve is to be replaced
// szv#4 S4163
// S4181 pdn 20.04.99 Modification of indirect rectangular trimming surfaces and taking
//  locations into account
// szv 03.01.01 PositiveCones merged in

#include <BRep_Builder.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_ElementarySurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_Surface.hxx>
#include <gp_Mat.hxx>
#include <gp_Pnt.hxx>
#include <Message_Msg.hxx>
#include <ShapeCustom_DirectModification.hxx>
#include <Standard_Type.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShapeCustom_DirectModification, ShapeCustom_Modification)

//=================================================================================================

ShapeCustom_DirectModification::ShapeCustom_DirectModification() {}

// S4181 returns 0 - none, 1 - indirect, 2 - negative cone, 3 - indirect negative cone
static Standard_Integer IsIndirectSurface(Handle(GeomSurface)& S, TopLoc_Location& L)
{
  Standard_Integer result = 0;

  Handle(GeomSurface) TS = S;
  while (TS->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
    TS = Handle(Geom_RectangularTrimmedSurface)::DownCast(TS)->BasisSurface();

  Handle(Geom_ElementarySurface) ES = Handle(Geom_ElementarySurface)::DownCast(TS);
  if (!ES.IsNull())
  {
    // is the surface indirect ?
    Transform3d          t   = L.Transformation();
    Standard_Boolean neg = t.IsNegative();
    Standard_Boolean det = (t.VectorialPart().Determinant() < 0.0);
    Standard_Boolean dir = ES->Position().Direct();
    if ((neg != det) == dir)
      result = 1;
    Handle(Geom_ConicalSurface) CS = Handle(Geom_ConicalSurface)::DownCast(ES);
    if (!CS.IsNull())
    {
      // does the cone have negative semiangle ?
      if (CS->SemiAngle() < 0.0)
        result += 2;
    }
    if (result)
      S = TS;
  }

  return result;
}

//=================================================================================================

Standard_Boolean ShapeCustom_DirectModification::NewSurface(const TopoFace&    F,
                                                            Handle(GeomSurface)& S,
                                                            TopLoc_Location&      L,
                                                            Standard_Real&        Tol,
                                                            Standard_Boolean&     RevWires,
                                                            Standard_Boolean&     RevFace)
{
  S = BRepInspector::Surface(F, L);

  switch (IsIndirectSurface(S, L))
  {
    case 1: { // Indirect surface
      // UReverse a copy of S
      S        = S->UReversed();
      RevWires = Standard_True;
      RevFace  = Standard_True;
      break;
    }
    case 2: { // Negative cone
      // U- and VReverse a copy of S
      S = S->VReversed();
      S->UReverse();
      RevWires = Standard_False;
      RevFace  = Standard_False;
      break;
    }
    case 3: { // Indirect negative cone
      // VReverse a copy of S
      S        = S->VReversed();
      RevWires = Standard_True;
      RevFace  = Standard_True;
      break;
    }
    default:
      return Standard_False;
  }

  SendMsg(F, Message_Msg("DirectModification.NewSurface.MSG0"));

  Tol = BRepInspector::Tolerance(F);

  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeCustom_DirectModification::NewCurve(const TopoEdge&  E,
                                                          Handle(GeomCurve3d)& C,
                                                          TopLoc_Location&    L,
                                                          Standard_Real&      Tol)
{
  //: p5 abv 26 Feb 99: force copying of edge if any its pcurve will be replaced
  Handle(BRep_TEdge)& TE = *((Handle(BRep_TEdge)*)&E.TShape());

  // iterate on pcurves
  BRep_ListIteratorOfListOfCurveRepresentation itcr(TE->Curves());
  for (; itcr.More(); itcr.Next())
  {
    Handle(BRep_GCurve) GC = Handle(BRep_GCurve)::DownCast(itcr.Value());
    if (GC.IsNull() || !GC->IsCurveOnSurface())
      continue;
    Handle(GeomSurface) S   = GC->Surface();
    TopLoc_Location      Loc = GC->Location();
    if (!IsIndirectSurface(S, Loc))
      continue;
    Standard_Real f, l;
    C = BRepInspector::Curve(E, L, f, l);
    if (!C.IsNull())
      C = Handle(GeomCurve3d)::DownCast(C->Copy());
    Tol = BRepInspector::Tolerance(E);
    return Standard_True;
  }
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ShapeCustom_DirectModification::NewPoint(const TopoVertex& /*V*/,
                                                          Point3d& /*P*/,
                                                          Standard_Real& /*Tol*/)
{
  // 3d points are never modified
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ShapeCustom_DirectModification::NewCurve2d(const TopoEdge&    E,
                                                            const TopoFace&    F,
                                                            const TopoEdge&    NewE,
                                                            const TopoFace&    NewF,
                                                            Handle(GeomCurve2d)& C,
                                                            Standard_Real&        Tol)
{
  TopLoc_Location      L;
  Handle(GeomSurface) S = BRepInspector::Surface(F, L);

  Standard_Integer result = IsIndirectSurface(S, L);
  if (!result && E.IsSame(NewE))
    return Standard_False;

  Standard_Real f, l;
  C   = BRepInspector::CurveOnSurface(E, F, f, l);
  Tol = BRepInspector::Tolerance(E);

  if (result)
  {

    Transform2d T;

    switch (result)
    {
      case 1: { // Indirect surface
        // mirror the PCurve about the V axis
        T.SetMirror(gp1::OY2d());
        C = Handle(GeomCurve2d)::DownCast(C->Transformed(T));
        break;
      }
      case 2: { // Negative cone
        // mirror the PCurve about the U and V axis
        T.SetMirror(gp1::OX2d());
        C = Handle(GeomCurve2d)::DownCast(C->Transformed(T));
        T.SetMirror(gp1::OY2d());
        C->Transform(T);
        break;
      }
      case 3: { // Indirect negative cone
        // mirror the PCurve about the U axis
        T.SetMirror(gp1::OX2d());
        C = Handle(GeomCurve2d)::DownCast(C->Transformed(T));
        break;
      }
    }

    // #26 rln When seam edge contains triangulations trimming is lost by ShapeBuilder::UpdateEdge
    if (BRepTools1::IsReallyClosed(E, F))
    {
      // szv#4:S4163:12Mar99 SGI warning
      TopoShape         sh  = NewE.Reversed();
      Handle(GeomCurve2d) tmp = BRepInspector::CurveOnSurface(TopoDS::Edge(sh), NewF, f, l);
      if (tmp.IsNull())
      {
        tmp = BRepInspector::CurveOnSurface(E, F, f, l);
        ShapeBuilder B;
        B.UpdateEdge(NewE, tmp, C, NewF, Tol);
        B.Range(NewE, NewF, f, l);
        // anyway, tmp will be removed later by ShapeModifier
      }
    }
  }
  else
  {
    //: p5 abv 26 Feb 99: force copying of pcurves if edge was copied
    if (!C.IsNull())
      C = Handle(GeomCurve2d)::DownCast(C->Copy());
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeCustom_DirectModification::NewParameter(const TopoVertex& /*V*/,
                                                              const TopoEdge& /*E*/,
                                                              Standard_Real& /*P*/,
                                                              Standard_Real& /*Tol*/)
{
  return Standard_False;
}

//=================================================================================================

GeomAbs_Shape ShapeCustom_DirectModification::Continuity(const TopoEdge& E,
                                                         const TopoFace& F1,
                                                         const TopoFace& F2,
                                                         const TopoEdge& /*NewE*/,
                                                         const TopoFace& /*NewF1*/,
                                                         const TopoFace& /*NewF2*/)
{
  return BRepInspector::Continuity(E, F1, F2);
}
