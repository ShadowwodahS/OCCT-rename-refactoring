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

#ifndef _IMeshData_Shape_HeaderFile
#define _IMeshData_Shape_HeaderFile

#include <TopoDS_Shape.hxx>

//! Interface class representing model with associated TopoShape.
//! Intended for inheritance by structures and algorithms keeping
//! reference TopoShape.
class IMeshData_Shape : public RefObject
{
public:
  //! Destructor.
  virtual ~IMeshData_Shape() {}

  //! Assigns shape to discrete shape.
  void SetShape(const TopoShape& theShape) { myShape = theShape; }

  //! Returns shape assigned to discrete shape.
  const TopoShape& GetShape() const { return myShape; }

  DEFINE_STANDARD_RTTIEXT(IMeshData_Shape, RefObject)

protected:
  //! Constructor.
  IMeshData_Shape() {}

  //! Constructor.
  IMeshData_Shape(const TopoShape& theShape)
      : myShape(theShape)
  {
  }

private:
  TopoShape myShape;
};

#endif