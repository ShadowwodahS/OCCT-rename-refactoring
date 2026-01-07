// Created on: 2016-04-07
// Copyright (c) 2016 OPEN CASCADE SAS
// Created by: Oleg AGASHIN
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

#ifndef _IMeshTools_ShapeVisitor_HeaderFile
#define _IMeshTools_ShapeVisitor_HeaderFile

#include <Standard_Transient.hxx>
#include <Standard_Type.hxx>

class TopoFace;
class TopoEdge;

//! Interface class for shape visitor.
class IMeshTools_ShapeVisitor : public RefObject
{
public:
  //! Destructor.
  virtual ~IMeshTools_ShapeVisitor() {}

  //! Handles TopoFace object.
  Standard_EXPORT virtual void Visit(const TopoFace& theFace) = 0;

  //! Handles TopoEdge object.
  Standard_EXPORT virtual void Visit(const TopoEdge& theEdge) = 0;

  DEFINE_STANDARD_RTTIEXT(IMeshTools_ShapeVisitor, RefObject)

protected:
  //! Constructor.
  IMeshTools_ShapeVisitor() {}
};

#endif