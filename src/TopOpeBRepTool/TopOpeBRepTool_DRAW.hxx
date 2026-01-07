// Created on: 1994-10-24
// Created by: Jean Yves LEBEY
// Copyright (c) 1994-1999 Matra Datavision
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

#ifndef _TopOpeBRepTool_DRAW_HeaderFile
#define _TopOpeBRepTool_DRAW_HeaderFile
#ifdef DRAW

  #include <gp_Pnt.hxx>
  #include <gp_Pnt2d.hxx>
  #include <gp_Lin.hxx>
  #include <Draw_ColorKind.hxx>
  #include <Draw_MarkerShape.hxx>
  #include <TopOpeBRepTool_define.hxx>
  #include <Geom_Curve.hxx>
  #include <Geom2d_Curve.hxx>
  #include <TopOpeBRepTool_define.hxx>

Standard_EXPORT void           TopOpeBRepTool_DrawPoint(const Point3d&          P,
                                                        const Draw_MarkerShape T,
                                                        const Draw_ColorKind   C);
Standard_EXPORT void           TopOpeBRepTool_DrawSegment(const Point3d&        P1,
                                                          const Point3d&        P2,
                                                          const Draw_ColorKind C);
Standard_EXPORT void           TopOpeBRepTool_DrawSegment(const Point3d& P1, const Point3d& P2);
Standard_EXPORT void           TopOpeBRepTool_DrawSegment(const Point3d& P,
                                                          const gp_Lin&,
                                                          const Standard_Real  Par,
                                                          const Draw_ColorKind C);
Standard_EXPORT Draw_ColorKind TopOpeBRepTool_ColorOnState(const TopAbs_State S);
Standard_EXPORT void           TopOpeBRepTool_DrawSegment(const Point3d&       P,
                                                          const gp_Lin&       L,
                                                          const Standard_Real Par,
                                                          const TopAbs_State  S);
Standard_EXPORT void           FDRAW_DINS(const AsciiString1 pref,
                                          const TopoShape&           SS,
                                          const AsciiString1 Snam,
                                          const AsciiString1 suff = "");
Standard_EXPORT void           FDRAW_DINE(const AsciiString1 pref,
                                          const TopoEdge&            EE,
                                          const AsciiString1 Enam,
                                          const AsciiString1 suff = "");
Standard_EXPORT void           FDRAW_DINLOE(const AsciiString1 pref,
                                            const ShapeList&   LOE,
                                            const AsciiString1 str1,
                                            const AsciiString1 str2);
Standard_EXPORT void           FUN_tool_draw(const AsciiString1& aa,
                                             const Point3d&                  p,
                                             const Dir3d&                  d);
Standard_EXPORT void           FUN_tool_draw(const AsciiString1& aa,
                                             const gp_Pnt2d&                p,
                                             const gp_Dir2d&                d,
                                             const Standard_Integer&        i);
Standard_EXPORT void           FUN_tool_draw(const AsciiString1 aa, const gp_Pnt2d& p2d);
Standard_EXPORT void           FUN_tool_draw(const AsciiString1 aa, const Point3d& p);
Standard_EXPORT void           FUN_tool_draw(const AsciiString1 aa,
                                             const Handle(GeomCurve2d)    c2d);
Standard_EXPORT void           FUN_tool_draw(const AsciiString1 aa,
                                             const Handle(GeomCurve2d)    c2d,
                                             const Standard_Real           f,
                                             const Standard_Real           l);
Standard_EXPORT void FUN_tool_draw(const AsciiString1& aa, const Handle(GeomCurve3d)& C);
Standard_EXPORT void FUN_tool_draw(const AsciiString1 aa,
                                   const Handle(GeomCurve3d)      c,
                                   const Standard_Real           f,
                                   const Standard_Real           l);
Standard_EXPORT void FUN_tool_draw(const AsciiString1 aa, const TopoShape& s);
Standard_EXPORT void FUN_tool_draw(const AsciiString1 aa,
                                   const TopoShape&           S,
                                   const Standard_Integer        is);
Standard_EXPORT void FUN_tool_draw(AsciiString1 aa,
                                   const TopoEdge&      E,
                                   const TopoFace&      F,
                                   const Standard_Integer  ie);
Standard_EXPORT void FUN_tool_draw(AsciiString1 aa,
                                   const TopoEdge&      E,
                                   const TopoFace&      F,
                                   const Standard_Integer  ie);
Standard_EXPORT const AsciiString1& FUN_tool_PRODINS();
Standard_EXPORT const AsciiString1& FUN_tool_PRODINP();
#endif
#endif
