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

#include <BRep_Builder.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Circle.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_Curve.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_ElementarySurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_Surface.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <gp_Pnt.hxx>
#include <Message_Msg.hxx>
#include <ShapeCustom_ConvertToRevolution.hxx>
#include <Standard_Type.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShapeCustom_ConvertToRevolution, ShapeCustom_Modification)

//=================================================================================================

ShapeCustom_ConvertToRevolution::ShapeCustom_ConvertToRevolution() {}

// Analyze surface: is it to be converted?
static Standard_Boolean IsToConvert(const Handle(GeomSurface)&     S,
                                    Handle(Geom_ElementarySurface)& ES)
{
  ES = Handle(Geom_ElementarySurface)::DownCast(S);
  if (ES.IsNull())
  {
    if (S->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
    {
      Handle(Geom_RectangularTrimmedSurface) RTS =
        Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
      ES = Handle(Geom_ElementarySurface)::DownCast(RTS->BasisSurface());
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_OffsetSurface)))
    {
      Handle(Geom_OffsetSurface) OS = Handle(Geom_OffsetSurface)::DownCast(S);
      ES                            = Handle(Geom_ElementarySurface)::DownCast(OS->BasisSurface());
    }
    if (ES.IsNull())
      return Standard_False;
  }

  return ES->IsKind(STANDARD_TYPE(Geom_SphericalSurface))
         || ES->IsKind(STANDARD_TYPE(Geom_ToroidalSurface))
         || ES->IsKind(STANDARD_TYPE(Geom_CylindricalSurface))
         || ES->IsKind(STANDARD_TYPE(Geom_ConicalSurface));
}

//=================================================================================================

Standard_Boolean ShapeCustom_ConvertToRevolution::NewSurface(const TopoFace&    F,
                                                             Handle(GeomSurface)& S,
                                                             TopLoc_Location&      L,
                                                             Standard_Real&        Tol,
                                                             Standard_Boolean&     RevWires,
                                                             Standard_Boolean&     RevFace)
{
  S = BRepInspector::Surface(F, L);

  Handle(Geom_ElementarySurface) ES;
  if (!IsToConvert(S, ES))
    return Standard_False;

  // remove location if it contains inversion
  /*
    Transform3d t = L.Transformation();
    gp_Mat m = t.VectorialPart();
    Standard_Boolean neg = t.IsNegative();
    Standard_Boolean det = ( m.Determinant() <0 ? Standard_True : Standard_False );
    if ( neg != det ) {
      ES = Handle(Geom_ElementarySurface)::DownCast ( ES->Transformed(t) );
      L.Identity();
    }
  */

  Ax3 Ax3 = ES->Position1();
  Point3d pos = Ax3.Location();
  Dir3d dir = Ax3.Direction();
  Dir3d X   = Ax3.XDirection();

  // create basis line to rotate
  Handle(GeomCurve3d) BasisCurve;
  if (ES->IsKind(STANDARD_TYPE(Geom_SphericalSurface)))
  {
    Handle(Geom_SphericalSurface) SS = Handle(Geom_SphericalSurface)::DownCast(ES);
    Frame3d                        Ax2(pos, X ^ dir, X);
    Handle(GeomCircle)           Circ = new GeomCircle(Ax2, SS->Radius());
    BasisCurve                         = new Geom_TrimmedCurve(Circ, -M_PI / 2., M_PI / 2.);
  }
  else if (ES->IsKind(STANDARD_TYPE(Geom_ToroidalSurface)))
  {
    Handle(Geom_ToroidalSurface) TS = Handle(Geom_ToroidalSurface)::DownCast(ES);
    Frame3d                       Ax2(pos.XYZ() + X.XYZ() * TS->MajorRadius(), X ^ dir, X);
    BasisCurve = new GeomCircle(Ax2, TS->MinorRadius());
  }
  else if (ES->IsKind(STANDARD_TYPE(Geom_CylindricalSurface)))
  {
    Handle(Geom_CylindricalSurface) CS = Handle(Geom_CylindricalSurface)::DownCast(ES);
    Axis3d                          Ax1(pos.XYZ() + X.XYZ() * CS->Radius(), dir);
    BasisCurve = new GeomLine(Ax1);
  }
  else if (ES->IsKind(STANDARD_TYPE(Geom_ConicalSurface)))
  {
    Handle(Geom_ConicalSurface) CS = Handle(Geom_ConicalSurface)::DownCast(ES);
    Dir3d                      N  = dir.XYZ() + X.XYZ() * Tan(CS->SemiAngle());
    Axis3d                      Ax1(pos.XYZ() + X.XYZ() * CS->RefRadius(), N);
    BasisCurve = new GeomLine(Ax1);
  }

  // create revolution with proper U parametrization
  Axis3d Axis = Ax3.Axis();

  // if the surface is indirect (taking into account locations), reverse dir

  /*
    Transform3d t = L.Transformation();
    gp_Mat m = t.VectorialPart();
    Standard_Boolean neg = t.IsNegative();
    Standard_Boolean det = ( m.Determinant() <0 ? Standard_True : Standard_False );
    Standard_Boolean isdir = Ax3.Direct();
    if ( ( neg != det ) == isdir ) Axis.Reverse();
  */
  if (!Ax3.Direct())
    Axis.Reverse();

  Handle(Geom_SurfaceOfRevolution) Rev = new Geom_SurfaceOfRevolution(BasisCurve, Axis);

  // set resulting surface and restore trimming or offsetting if necessary
  if (ES == S)
    S = Rev;
  else
  {
    if (S->IsKind(STANDARD_TYPE(Geom_RectangularTrimmedSurface)))
    {
      Handle(Geom_RectangularTrimmedSurface) RTS =
        Handle(Geom_RectangularTrimmedSurface)::DownCast(S);
      Standard_Real U1, U2, V1, V2;
      RTS->Bounds(U1, U2, V1, V2);
      S = new Geom_RectangularTrimmedSurface(Rev, U1, U2, V1, V2);
    }
    else if (S->IsKind(STANDARD_TYPE(Geom_OffsetSurface)))
    {
      Handle(Geom_OffsetSurface) OS = Handle(Geom_OffsetSurface)::DownCast(S);
      S                             = new Geom_OffsetSurface(Rev, OS->Offset());
    }
    else
      S = Rev;
  }
  SendMsg(F, Message_Msg("ConvertToRevolution.NewSurface.MSG0"));

  Tol      = BRepInspector::Tolerance(F);
  RevWires = Standard_False;
  RevFace  = Standard_False;
  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeCustom_ConvertToRevolution::NewCurve(const TopoEdge&  E,
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
    Handle(GeomSurface)           S = GC->Surface();
    Handle(Geom_ElementarySurface) ES;
    if (!IsToConvert(S, ES))
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

Standard_Boolean ShapeCustom_ConvertToRevolution::NewPoint(const TopoVertex& /*V*/,
                                                           Point3d& /*P*/,
                                                           Standard_Real& /*Tol*/)
{
  // 3d points are never modified
  return Standard_False;
}

//=================================================================================================

Standard_Boolean ShapeCustom_ConvertToRevolution::NewCurve2d(const TopoEdge& E,
                                                             const TopoFace& F,
                                                             const TopoEdge& NewE,
                                                             const TopoFace& /*NewF*/,
                                                             Handle(GeomCurve2d)& C,
                                                             Standard_Real&        Tol)
{
  TopLoc_Location                L;
  Handle(GeomSurface)           S = BRepInspector::Surface(F, L);
  Handle(Geom_ElementarySurface) ES;

  // just copy pcurve if either its surface is changing or edge was copied
  if (!IsToConvert(S, ES) && E.IsSame(NewE))
    return Standard_False;

  Standard_Real f, l;
  C = BRepInspector::CurveOnSurface(E, F, f, l);
  if (!C.IsNull())
  {
    C = Handle(GeomCurve2d)::DownCast(C->Copy());

    // for spherical surface, surface of revolution since based on TrimmedCurve
    // has V parametrisation shifted on 2PI; translate pcurve accordingly
    if (!ES.IsNull() && ES->IsKind(STANDARD_TYPE(Geom_SphericalSurface)))
    {
      gp_Vec2d shift(0., 2 * M_PI);
      C->Translate(shift);
    }
  }

  Tol = BRepInspector::Tolerance(E);
  return Standard_True;
}

//=================================================================================================

Standard_Boolean ShapeCustom_ConvertToRevolution::NewParameter(const TopoVertex& /*V*/,
                                                               const TopoEdge& /*E*/,
                                                               Standard_Real& /*P*/,
                                                               Standard_Real& /*Tol*/)
{
  return Standard_False;
}

//=================================================================================================

GeomAbs_Shape ShapeCustom_ConvertToRevolution::Continuity(const TopoEdge& E,
                                                          const TopoFace& F1,
                                                          const TopoFace& F2,
                                                          const TopoEdge& /*NewE*/,
                                                          const TopoFace& /*NewF1*/,
                                                          const TopoFace& /*NewF2*/)
{
  return BRepInspector::Continuity(E, F1, F2);
}
