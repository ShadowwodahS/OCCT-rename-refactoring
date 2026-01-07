// Created on: 1990-12-11
// Created by: Remi Lequette
// Copyright (c) 1990-1999 Matra Datavision
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

#ifndef _TopoDS_HeaderFile
#define _TopoDS_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_TypeMismatch.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_Shape.hxx>

class TopoVertex;
class TopoShape;
class TopoEdge;
class TopoWire;
class TopoFace;
class TopoShell;
class TopoSolid;
class TopoDS_CompSolid;
class TopoCompound;
class TopoDS_HShape;
class TopoDS_TShape;
class TopoDS_TVertex;
class TopoDS_TEdge;
class TopoDS_TWire;
class TopoDS_TFace;
class TopoDS_TShell;
class TopoDS_TSolid;
class TopoDS_TCompSolid;
class TopoDS_TCompound;
class TopoBuilder;
class TopoDS_Iterator;

//! Provides methods to cast objects of class TopoShape to more specialized
//! sub-classes. The types are not verified before casting. If the type does
//! not match, a Standard_TypeMismatch exception is thrown. Below are examples
//! of correct and incorrect casts:
//!
//! Correct:
//! @code
//! TopoShape aShape = ...; // aShape->ShapeType() == TopAbs_VERTEX
//! const TopoVertex& aVertex = TopoDS::Vertex(aShape);
//! @endcode
//!
//! Incorrect (will throw a Standard_TypeMismatch exception):
//! @code
//! TopoShape aShape = ...; // aShape->ShapeType() == TopAbs_VERTEX
//! const TopoFace& face = TopoDS::Edge(aShape);
//! @endcode
namespace TopoDS
{
//! Casts shape theShape to the more specialized return type, Vertex.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoVertex
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoVertex& Vertex(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_VERTEX,
                                 "TopoDS::Vertex");
  return *(TopoVertex*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Vertex.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoVertex
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoVertex& Vertex(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_VERTEX,
                                 "TopoDS::Vertex");
  return *(TopoVertex*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Edge.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoEdge
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoEdge& Edge(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_EDGE,
                                 "TopoDS::Edge");
  return *(TopoEdge*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Edge.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoEdge
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoEdge& Edge(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_EDGE,
                                 "TopoDS::Edge");
  return *(TopoEdge*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Wire.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoWire
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoWire& Wire(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_WIRE,
                                 "TopoDS::Wire");
  return *(TopoWire*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Wire.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoWire
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoWire& Wire(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_WIRE,
                                 "TopoDS::Wire");
  return *(TopoWire*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Face.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoFace
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoFace& Face(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_FACE,
                                 "TopoDS::Face");
  return *(TopoFace*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Face.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoFace
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoFace& Face(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_FACE,
                                 "TopoDS::Face");
  return *(TopoFace*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Shell.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoShell
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoShell& Shell(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_SHELL,
                                 "TopoDS::Shell");
  return *(TopoShell*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Shell.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoShell
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoShell& Shell(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_SHELL,
                                 "TopoDS::Shell");
  return *(TopoShell*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Solid.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoSolid
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoSolid& Solid(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_SOLID,
                                 "TopoDS::Solid");
  return *(TopoSolid*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Solid.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoSolid
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoSolid& Solid(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_SOLID,
                                 "TopoDS::Solid");
  return *(TopoSolid*)&theShape;
}

//! Casts shape theShape to the more specialized return type, CompSolid.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoDS_CompSolid
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoDS_CompSolid& CompSolid(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_COMPSOLID,
                                 "TopoDS::CompSolid");
  return *(TopoDS_CompSolid*)&theShape;
}

//! Casts shape theShape to the more specialized return type, CompSolid.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoDS_CompSolid
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoDS_CompSolid& CompSolid(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_COMPSOLID,
                                 "TopoDS::CompSolid");
  return *(TopoDS_CompSolid*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Compound.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoCompound
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline const TopoCompound& Compound(const TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_COMPOUND,
                                 "TopoDS::Compound");
  return *(TopoCompound*)&theShape;
}

//! Casts shape theShape to the more specialized return type, Compound.
//! @param theShape the shape to be cast
//! @return the casted shape as TopoCompound
//! @throws Standard_TypeMismatch if theShape cannot be cast to this return type.
inline TopoCompound& Compound(TopoShape& theShape)
{
  Standard_TypeMismatch_Raise_if(theShape.IsNull() ? Standard_False
                                                   : theShape.ShapeType() != TopAbs_COMPOUND,
                                 "TopoDS::Compound");
  return *(TopoCompound*)&theShape;
}
} // namespace TopoDS

#endif // _TopoDS_HeaderFile
