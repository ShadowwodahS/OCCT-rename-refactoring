// Created on: 1999-11-03
// Created by: Denis PASCAL
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

#ifndef _TNaming_Scope_HeaderFile
#define _TNaming_Scope_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TDF_LabelMap.hxx>
class DataLabel;
class TopoShape;
class ShapeAttribute;

//! this class manage a scope of labels
//! ===================================
class NamingScope
{
public:
  DEFINE_STANDARD_ALLOC

  //! WithValid = FALSE
  Standard_EXPORT NamingScope();

  //! if <WithValid> the scope is defined by the map. If not
  //! on the whole framework.
  Standard_EXPORT NamingScope(const Standard_Boolean WithValid);

  //! create a scope with a map. WithValid = TRUE.
  Standard_EXPORT NamingScope(TDF_LabelMap& valid);

  Standard_EXPORT Standard_Boolean WithValid() const;

  Standard_EXPORT void WithValid(const Standard_Boolean mode);

  Standard_EXPORT void ClearValid();

  Standard_EXPORT void Valid(const DataLabel& L);

  Standard_EXPORT void ValidChildren(const DataLabel&       L,
                                     const Standard_Boolean withroot = Standard_True);

  Standard_EXPORT void Unvalid(const DataLabel& L);

  Standard_EXPORT void UnvalidChildren(const DataLabel&       L,
                                       const Standard_Boolean withroot = Standard_True);

  Standard_EXPORT Standard_Boolean IsValid(const DataLabel& L) const;

  Standard_EXPORT const TDF_LabelMap& GetValid() const;

  Standard_EXPORT TDF_LabelMap& ChangeValid();

  //! Returns  the current  value of  <NS> according to the
  //! Valid Scope.
  Standard_EXPORT TopoShape CurrentShape(const Handle(ShapeAttribute)& NS) const;

protected:
private:
  Standard_Boolean myWithValid;
  TDF_LabelMap     myValid;
};

#endif // _TNaming_Scope_HeaderFile
