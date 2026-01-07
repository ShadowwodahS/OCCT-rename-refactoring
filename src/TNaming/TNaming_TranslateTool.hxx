// Created on: 1999-06-24
// Created by: Sergey ZARITCHNY
// Copyright (c) 1999 Matra Datavision
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

#ifndef _TNaming_TranslateTool_HeaderFile
#define _TNaming_TranslateTool_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Standard_Transient.hxx>
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
class TopoShape;

class TNaming_TranslateTool;
DEFINE_STANDARD_HANDLE(TNaming_TranslateTool, RefObject)

//! tool to copy underlying TShape of a Shape.
//! The TranslateTool class is provided to support the
//! translation of topological data structures  Transient
//! to  Transient.
class TNaming_TranslateTool : public RefObject
{

public:
  Standard_EXPORT void Add(TopoShape& S1, const TopoShape& S2) const;

  Standard_EXPORT void MakeVertex(TopoShape& S) const;

  Standard_EXPORT void MakeEdge(TopoShape& S) const;

  Standard_EXPORT void MakeWire(TopoShape& S) const;

  Standard_EXPORT void MakeFace(TopoShape& S) const;

  Standard_EXPORT void MakeShell(TopoShape& S) const;

  Standard_EXPORT void MakeSolid(TopoShape& S) const;

  Standard_EXPORT void MakeCompSolid(TopoShape& S) const;

  Standard_EXPORT void MakeCompound(TopoShape& S) const;

  Standard_EXPORT void UpdateVertex(const TopoShape&                         S1,
                                    TopoShape&                               S2,
                                    TColStd_IndexedDataMapOfTransientTransient& M) const;

  Standard_EXPORT void UpdateEdge(const TopoShape&                         S1,
                                  TopoShape&                               S2,
                                  TColStd_IndexedDataMapOfTransientTransient& M) const;

  Standard_EXPORT void UpdateFace(const TopoShape&                         S1,
                                  TopoShape&                               S2,
                                  TColStd_IndexedDataMapOfTransientTransient& M) const;

  Standard_EXPORT void UpdateShape(const TopoShape& S1, TopoShape& S2) const;

  DEFINE_STANDARD_RTTIEXT(TNaming_TranslateTool, RefObject)

protected:
private:
};

#endif // _TNaming_TranslateTool_HeaderFile
