// Created on: 1997-11-25
// Created by: Jean Yves LEBEY
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _TopOpeBRepDS_connex_HeaderFile
#define _TopOpeBRepDS_connex_HeaderFile

#include <TopOpeBRepDS_define.hxx>

Standard_EXPORT const ShapeList& FDSCNX_EdgeConnexityShapeIndex(
  const TopoShape&                        E,
  const Handle(TopOpeBRepDS_HDataStructure)& HDS,
  const Standard_Integer                     SI);
Standard_EXPORT const ShapeList& FDSCNX_EdgeConnexitySameShape(
  const TopoShape&                        E,
  const Handle(TopOpeBRepDS_HDataStructure)& HDS);
Standard_EXPORT void FDSCNX_Prepare(const TopoShape&                        S1,
                                    const TopoShape&                        S2,
                                    const Handle(TopOpeBRepDS_HDataStructure)& HDS);
Standard_EXPORT Standard_Boolean
  FDSCNX_HasConnexFace(const TopoShape& S, const Handle(TopOpeBRepDS_HDataStructure)& HDS);
Standard_EXPORT void FDSCNX_FaceEdgeConnexFaces(const TopoShape&                        F,
                                                const TopoShape&                        E,
                                                const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                                ShapeList&                      LF);
Standard_EXPORT void FDSCNX_Dump(const Handle(TopOpeBRepDS_HDataStructure)& HDS);
Standard_EXPORT void FDSCNX_Dump(const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                 const Standard_Integer                     I);
Standard_EXPORT void FDSCNX_DumpIndex(const Handle(TopOpeBRepDS_HDataStructure)& HDS,
                                      const Standard_Integer                     I);
#endif
