// Created on: 1998-12-09
// Created by: Xuan PHAM PHU
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

#ifndef _TopOpeBRepTool_connexity_HeaderFile
#define _TopOpeBRepTool_connexity_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Shape.hxx>
#include <TopTools_Array1OfListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Standard_Boolean.hxx>

class TopOpeBRepTool_connexity
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT TopOpeBRepTool_connexity();

  Standard_EXPORT TopOpeBRepTool_connexity(const TopoShape& Key1);

  Standard_EXPORT void SetKey(const TopoShape& Key1);

  Standard_EXPORT const TopoShape& Key1() const;

  Standard_EXPORT Standard_Integer Item(const Standard_Integer OriKey,
                                        ShapeList&  Item) const;

  Standard_EXPORT Standard_Integer AllItems(ShapeList& Item) const;

  Standard_EXPORT void AddItem(const Standard_Integer OriKey, const ShapeList& Item);

  Standard_EXPORT void AddItem(const Standard_Integer OriKey, const TopoShape& Item);

  Standard_EXPORT Standard_Boolean RemoveItem(const Standard_Integer OriKey,
                                              const TopoShape&    Item);

  Standard_EXPORT Standard_Boolean RemoveItem(const TopoShape& Item);

  Standard_EXPORT ShapeList& ChangeItem(const Standard_Integer OriKey);

  Standard_EXPORT Standard_Boolean IsMultiple() const;

  Standard_EXPORT Standard_Boolean IsFaulty() const;

  Standard_EXPORT Standard_Integer IsInternal(ShapeList& Item) const;

protected:
private:
  TopoShape                 theKey;
  TopTools_Array1OfListOfShape theItems;
};

#endif // _TopOpeBRepTool_connexity_HeaderFile
