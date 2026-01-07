// Created on: 1998-11-24
// Created by: Prestataire Xuan PHAM PHU
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

#ifndef _TopOpeBRepTool_PURGE_HeaderFile
#define _TopOpeBRepTool_PURGE_HeaderFile
#include <TopOpeBRepTool_define.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <gp_Pnt2d.hxx>

#ifdef OCCT_DEBUG
extern void FUN_REINIT();
// extern Standard_Integer FUN_addepc(const TopoShape& ed,const TopoShape& f);
#endif

// ----------------------------------------------------------------------
// TopOpeBRepTool_faulty.cxx
// ----------------------------------------------------------------------
// Standard_IMPORT gp_Pnt2d FUN_GetVParonF(const TopoEdge& E, const TopoFace& F, const
// Standard_Integer Index); Standard_IMPORT Standard_Real FUN_toluv(const GeomAdaptor_Surface& GAS,
// const Standard_Real& tol3d); Standard_IMPORT void FUN_tool_Vertices(const TopoEdge& E,
// TopTools_Array1OfShape& vertices); Standard_IMPORT void FUN_mapVloe(const TopoShape& F,
// TopTools_IndexedDataMapOfShapeListOfShape& mapVloe);

// Standard_IMPORT Standard_Boolean FUN_DetectEdgeswithfaultyUV(const TopoFace& Fin, const
// TopoShape& fF, const ShapeList& ISOEds, 						const
// Standard_Boolean has2fybounds,
// ShapeList& lfyE, Standard_Integer& Ivfaulty, 						const
// Standard_Boolean& stopatfirst=Standard_False); Standard_IMPORT Standard_Boolean
// FUN_DetectEdgewithfaultyUV(const
// TopoFace& Fin, const TopoShape& fF, const ShapeList& ISOEds, const
// Standard_Boolean has2fybounds, TopoShape& fyE, Standard_Integer& Ivfaulty); Standard_IMPORT
// Standard_Boolean FUN_DetectFaultyClosingEdge(const TopoFace& Fin,const ShapeList&
// Eds,const ShapeList& cEds,ShapeList& fyE); Standard_IMPORT Standard_Boolean
// FUN_isUVClosed(const TopoFace& F, const TopoFace& fF);

// ----------------------------------------------------------------------
// TopOpeBRepTool_PURGE.cxx
// ----------------------------------------------------------------------
// Standard_IMPORT void FUN_tool_ttranslate(const gp_Vec2d& tvector, const TopoFace& fF,
// TopoEdge& fyE);
#endif
