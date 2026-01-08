// Created on: 1996-03-26
// Created by: Christian CAILLET
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

#ifndef _XSControl_Functions_HeaderFile
#define _XSControl_Functions_HeaderFile

//! Functions from XSControl1 gives access to actions which can be
//! commanded with the resources provided by XSControl1: especially
//! Controller and Transfer
//!
//! It works by adding functions by method Init
class Functions1
{
public:
  //! Defines and loads all functions for XSControl1 (as ActFunc)
  Standard_EXPORT static void Init();
};

#endif // _XSControl_Functions_HeaderFile
