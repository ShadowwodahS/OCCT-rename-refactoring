// Copyright (c) 1995-1999 Matra Datavision
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

#include <DrawTrSurf_BezierCurve.hxx>

#include <Draw_Color.hxx>
#include <Draw_Display.hxx>
#include <DrawTrSurf.hxx>
#include <DrawTrSurf_Params.hxx>
#include <Geom_BezierCurve.hxx>
#include <GeomTools_CurveSet.hxx>
#include <gp_Pnt2d.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DrawTrSurf_BezierCurve, DrawTrSurf_Curve)

DrawTrSurf_BezierCurve::DrawTrSurf_BezierCurve(const Handle(BezierCurve3d)& C)
    : DrawTrSurf_Curve(C, Draw_vert, 16, 0.05, 1)
{
  drawPoles = Standard_True;
  polesLook = Draw_rouge;
}

DrawTrSurf_BezierCurve::DrawTrSurf_BezierCurve(const Handle(BezierCurve3d)& C,
                                               const DrawColor&               CurvColor,
                                               const DrawColor&               PolesColor,
                                               const Standard_Boolean          ShowPoles,
                                               const Standard_Integer          Discret,
                                               const Standard_Real             Deflection,
                                               const Standard_Integer          DrawMode)
    : DrawTrSurf_Curve(C, CurvColor, Discret, Deflection, DrawMode)
{
  drawPoles = ShowPoles;
  polesLook = PolesColor;
}

void DrawTrSurf_BezierCurve::DrawOn(DrawDisplay& dis) const
{
  Handle(BezierCurve3d) C = Handle(BezierCurve3d)::DownCast(curv);
  if (drawPoles)
  {
    Standard_Integer NbPoles = C->NbPoles();
    dis.SetColor(polesLook);
    TColgp_Array1OfPnt CPoles(1, NbPoles);
    C->Poles(CPoles);
    dis.MoveTo(CPoles(1));
    for (Standard_Integer i = 2; i <= NbPoles; i++)
    {
      dis.DrawTo(CPoles(i));
    }
  }

  DrawTrSurf_Curve::DrawOn(dis);
}

void DrawTrSurf_BezierCurve::FindPole(const Standard_Real X,
                                      const Standard_Real Y,
                                      const DrawDisplay& D,
                                      const Standard_Real XPrec,
                                      Standard_Integer&   Index) const
{
  Handle(BezierCurve3d) bz = Handle(BezierCurve3d)::DownCast(curv);
  gp_Pnt2d                 p1(X / D.Zoom(), Y / D.Zoom());
  Standard_Real            Prec = XPrec / D.Zoom();
  Index++;
  Standard_Integer NbPoles = bz->NbPoles();
  while (Index <= NbPoles)
  {
    if (D.Project(bz->Pole(Index)).Distance(p1) <= Prec)
    {
      return;
    }
    Index++;
  }
  Index = 0;
}

//=================================================================================================

Handle(Draw_Drawable3D) DrawTrSurf_BezierCurve::Copy() const
{
  Handle(DrawTrSurf_BezierCurve) DC =
    new DrawTrSurf_BezierCurve(Handle(BezierCurve3d)::DownCast(curv->Copy()),
                               look,
                               polesLook,
                               drawPoles,
                               GetDiscretisation(),
                               GetDeflection(),
                               GetDrawMode());
  return DC;
}

//=================================================================================================

Handle(Draw_Drawable3D) DrawTrSurf_BezierCurve::Restore(Standard_IStream& theStream)
{
  const DrawTrSurf_Params& aParams = DrawTrSurf1::Parameters();
  Handle(BezierCurve3d) aGeomCurve =
    Handle(BezierCurve3d)::DownCast(GeomTools_CurveSet::ReadCurve(theStream));
  Handle(DrawTrSurf_BezierCurve) aDrawCurve = new DrawTrSurf_BezierCurve(aGeomCurve,
                                                                         aParams.CurvColor,
                                                                         aParams.PolesColor,
                                                                         aParams.IsShowPoles,
                                                                         aParams.Discret,
                                                                         aParams.Deflection,
                                                                         aParams.DrawMode);
  return aDrawCurve;
}
