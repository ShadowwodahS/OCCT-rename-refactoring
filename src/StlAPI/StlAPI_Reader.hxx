// Created on: 2000-06-23
// Created by: Sergey MOZOKHIN
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

#ifndef _StlAPI_Reader_HeaderFile
#define _StlAPI_Reader_HeaderFile

#include <Standard_Handle.hxx>

class TopoShape;

//! Reading from stereolithography format.
//! Reads STL file and creates a shape composed of triangular faces, one per facet.
//! IMPORTANT: This approach is very inefficient, especially for large files.
//! IMPORTANT: Consider reading STL file to MeshTriangulation object instead (see class RWStl1).
class StlReader
{
public:
  //! Reads STL file to the TopoShape (each triangle is converted to the face).
  //! @return True if reading is successful
  Standard_EXPORT Standard_Boolean Read(TopoShape& theShape, const Standard_CString theFileName);
};

#endif // _StlAPI_Reader_HeaderFile
