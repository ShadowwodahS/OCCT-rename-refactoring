// Created on: 1998-02-11
// Created by: Philippe MANGIN
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

#ifndef _BRepFill_SectionPlacement_HeaderFile
#define _BRepFill_SectionPlacement_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>
#include <Standard_Integer.hxx>
class BRepFill_LocationLaw;

//! Place a shape in a local axis coordinate
class BRepFill_SectionPlacement
{
public:
  DEFINE_STANDARD_ALLOC

  //! Automatic placement
  Standard_EXPORT BRepFill_SectionPlacement(const Handle(BRepFill_LocationLaw)& Law1,
                                            const TopoShape&                 Section,
                                            const Standard_Boolean WithContact    = Standard_False,
                                            const Standard_Boolean WithCorrection = Standard_False);

  //! Placement on vertex
  Standard_EXPORT BRepFill_SectionPlacement(const Handle(BRepFill_LocationLaw)& Law1,
                                            const TopoShape&                 Section,
                                            const TopoShape&                 Vertex,
                                            const Standard_Boolean WithContact    = Standard_False,
                                            const Standard_Boolean WithCorrection = Standard_False);

  Standard_EXPORT const Transform3d& Transformation() const;

  Standard_EXPORT Standard_Real AbscissaOnPath();

protected:
private:
  Standard_EXPORT void Perform(const Standard_Boolean WithContact,
                               const Standard_Boolean WithCorrection,
                               const TopoShape&    Vertex);

  Handle(BRepFill_LocationLaw) myLaw;
  TopoShape                 mySection;
  Transform3d                      myTrsf;
  Standard_Real                myParam;
  Standard_Integer             myIndex;
};

#endif // _BRepFill_SectionPlacement_HeaderFile
