// Created on: 2020-01-25
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2020 OPEN CASCADE SAS
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

#ifndef Convert_TransientShape_H
#define Convert_TransientShape_H

#include <Standard.hxx>
#include <Standard_Handle.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Type.hxx>
#include <Standard_Transient.hxx>

#include <TopoDS_Shape.hxx>

//! \class Convert_TransientShape
//! \brief An interface to convert a shape into a transient object to be used in arguments
class Convert_TransientShape : public RefObject
{
public:
  //! Constructor
  Convert_TransientShape(const TopoShape& theShape) { SetShape(theShape); }

  //! Destructor
  virtual ~Convert_TransientShape() {}

  //! Returns current shape
  const TopoShape Shape() const { return myShape; }

  //! Fills current shape
  void SetShape(const TopoShape& theShape) { myShape = theShape; }

  DEFINE_STANDARD_RTTI_INLINE(Convert_TransientShape, RefObject)

private:
  TopoShape myShape; //!< the shape
};

#endif
