// Created on: 1996-12-17
// Created by: Remi Lequette
// Copyright (c) 1996-1999 Matra Datavision
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

#ifndef _TNaming_NewShapeIterator_HeaderFile
#define _TNaming_NewShapeIterator_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TNaming_PtrNode.hxx>
#include <Standard_Integer.hxx>
class Standard_NoMoreObject;
class Standard_NoSuchObject;
class Tool11;
class TNaming_Name;
class TNaming_Naming;
class TopoShape;
class TNaming_UsedShapes;
class DataLabel;
class Iterator1;
class ShapeAttribute;

//! Iterates on all the descendants of a shape
class NewShapeIterator
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT NewShapeIterator(const TopoShape&    aShape,
                                           const Standard_Integer Transaction,
                                           const DataLabel&       access);

  Standard_EXPORT NewShapeIterator(const TopoShape& aShape, const DataLabel& access);

  //! Iterates from the current Shape in <anIterator>
  Standard_EXPORT NewShapeIterator(const NewShapeIterator& anIterator);

  //! Iterates from the current Shape in <anIterator>
  Standard_EXPORT NewShapeIterator(const Iterator1& anIterator);

  Standard_Boolean More() const;

  Standard_EXPORT void Next();

  Standard_EXPORT DataLabel Label() const;

  Standard_EXPORT Handle(ShapeAttribute) NamedShape1() const;

  //! Warning! Can be a Null Shape if a descendant is deleted.
  Standard_EXPORT const TopoShape& Shape() const;

  //! True if the new  shape is a modification  (split,
  //! fuse,etc...) of the old shape.
  Standard_EXPORT Standard_Boolean IsModification() const;

  friend class Tool11;
  friend class TNaming_Name;
  friend class TNaming_Naming;

protected:
private:
  Standard_EXPORT NewShapeIterator(const TopoShape&               aShape,
                                           const Standard_Integer            Transaction,
                                           const Handle(TNaming_UsedShapes)& Shapes);

  Standard_EXPORT NewShapeIterator(const TopoShape&               aShape,
                                           const Handle(TNaming_UsedShapes)& Shapes);

  TNaming_PtrNode  myNode;
  Standard_Integer myTrans;
};

#include <TNaming_NewShapeIterator.lxx>

#endif // _TNaming_NewShapeIterator_HeaderFile
