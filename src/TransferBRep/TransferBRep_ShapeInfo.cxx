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

#include <TopoDS_Shape.hxx>
#include <TopoDS_TShape.hxx>
#include <TransferBRep_ShapeInfo.hxx>

Handle(TypeInfo) ShapeInfo::Type(const TopoShape& /*ent*/)
{
  return STANDARD_TYPE(TopoShapeBase);
}

Standard_CString ShapeInfo::TypeName(const TopoShape& ent)
{
  if (ent.IsNull())
    return "TopoShape";
  switch (ent.ShapeType())
  {
    case TopAbs_VERTEX:
      return "TopoVertex";
    case TopAbs_EDGE:
      return "TopoEdge";
    case TopAbs_WIRE:
      return "TopoWire";
    case TopAbs_FACE:
      return "TopoFace";
    case TopAbs_SHELL:
      return "TopoShell";
    case TopAbs_SOLID:
      return "TopoSolid";
    case TopAbs_COMPSOLID:
      return "TopoDS_CompSolid";
    case TopAbs_COMPOUND:
      return "TopoCompound";
    default:
      break;
  }
  return "TopoShape";
}
