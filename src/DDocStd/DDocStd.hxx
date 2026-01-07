// Created on: 2000-03-01
// Created by: Denis PASCAL
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#ifndef _DDocStd_HeaderFile
#define _DDocStd_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Boolean.hxx>
#include <Draw_Interpretor.hxx>
class AppManager;
class AppDocument;
class DataLabel;
class Standard_GUID;
class TDF_Attribute;

//! This package   provides Draw1 services to test  CAF
//! standard documents (see TDocStd package)
//!
//! It provides :
//!
//! * Modification registration and Update management.
//!
//! * External references mechanism
//!
//! * UNDO/REDO
//!
//! * Document Creation, Save and Restore
class DDocStd1
{
public:
  DEFINE_STANDARD_ALLOC

  //! Returns the global instance of application.
  Standard_EXPORT static const Handle(AppManager)& GetApplication();

  Standard_EXPORT static Standard_Boolean GetDocument(
    Standard_CString&         Name,
    Handle(AppDocument)& Doc,
    const Standard_Boolean    Complain = Standard_True);

  Standard_EXPORT static Standard_Boolean Find(const Handle(AppDocument)& Document,
                                               const Standard_CString          Entry,
                                               DataLabel&                      Label,
                                               const Standard_Boolean Complain = Standard_True);

  Standard_EXPORT static Standard_Boolean Find(const Handle(AppDocument)& Document,
                                               const Standard_CString          Entry,
                                               const Standard_GUID&            ID,
                                               Handle(TDF_Attribute)&          A,
                                               const Standard_Boolean Complain = Standard_True);

  //! Safe variant for arbitrary type of argument
  template <class T>
  static Standard_Boolean Find(const Handle(AppDocument)& Document,
                               const Standard_CString          Entry,
                               const Standard_GUID&            ID,
                               Handle(T)&                      A,
                               const Standard_Boolean          Complain = Standard_True)
  {
    Handle(TDF_Attribute) anAttr = A;
    return Find(Document, Entry, ID, anAttr, Complain)
           && !(A = Handle(T)::DownCast(anAttr)).IsNull();
  }

  Standard_EXPORT static DrawInterpreter& ReturnLabel(DrawInterpreter& theCommands,
                                                       const DataLabel&  L);

  Standard_EXPORT static void AllCommands(DrawInterpreter& theCommands);

  //! NewDocument, Open, SaveAs, Save
  Standard_EXPORT static void ApplicationCommands(DrawInterpreter& theCommands);

  //! Undo, Redo, SetModified, Propagate
  Standard_EXPORT static void DocumentCommands(DrawInterpreter& theCommands);

  //! Modified, Update
  Standard_EXPORT static void ToolsCommands(DrawInterpreter& theCommands);

  //! Create, Add, Remove, Open, Commit, Undo, Redo, SetNestedMode
  Standard_EXPORT static void MTMCommands(DrawInterpreter& theCommands);

  //! ShapeSchema_Read
  Standard_EXPORT static void ShapeSchemaCommands(DrawInterpreter& theCommands);
};

#endif // _DDocStd_HeaderFile
