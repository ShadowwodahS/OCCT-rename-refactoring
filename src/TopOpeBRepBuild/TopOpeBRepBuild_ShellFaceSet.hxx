// Created on: 1993-06-16
// Created by: Jean Yves LEBEY
// Copyright (c) 1993-1999 Matra Datavision
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

#ifndef _TopOpeBRepBuild_ShellFaceSet_HeaderFile
#define _TopOpeBRepBuild_ShellFaceSet_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopoDS_Solid.hxx>
#include <TopOpeBRepBuild_ShapeSet.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopTools_ListOfShape.hxx>
class TopoShape;
class AsciiString1;

//! a bound is a shell, a boundelement is a face.
//! The ShapeSet stores :
//! - a list of shell (bounds),
//! - a list of face (boundelements) to start reconstructions,
//! - a map of edge giving the list of face incident to an edge.
class TopOpeBRepBuild_ShellFaceSet : public TopOpeBRepBuild_ShapeSet
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates a ShellFaceSet to build blocks of faces
  //! connected by edges.
  Standard_EXPORT TopOpeBRepBuild_ShellFaceSet();

  //! Creates a ShellFaceSet to build blocks of faces
  //! connected by edges.
  Standard_EXPORT TopOpeBRepBuild_ShellFaceSet(const TopoShape&    S,
                                               const Standard_Address Addr = NULL);

  Standard_EXPORT const TopoSolid& Solid() const;

  Standard_EXPORT virtual void AddShape(const TopoShape& S) Standard_OVERRIDE;

  Standard_EXPORT virtual void AddStartElement(const TopoShape& S) Standard_OVERRIDE;

  Standard_EXPORT virtual void AddElement(const TopoShape& S) Standard_OVERRIDE;

  Standard_EXPORT virtual void DumpSS() Standard_OVERRIDE;

  Standard_EXPORT virtual AsciiString1 SName(
    const TopoShape&            S,
    const AsciiString1& sb = "",
    const AsciiString1& sa = "") const Standard_OVERRIDE;

  Standard_EXPORT virtual AsciiString1 SName(
    const ShapeList&    S,
    const AsciiString1& sb = "",
    const AsciiString1& sa = "") const Standard_OVERRIDE;

  Standard_EXPORT virtual AsciiString1 SNameori(
    const TopoShape&            S,
    const AsciiString1& sb = "",
    const AsciiString1& sa = "") const Standard_OVERRIDE;

  Standard_EXPORT virtual AsciiString1 SNameori(
    const ShapeList&    S,
    const AsciiString1& sb = "",
    const AsciiString1& sa = "") const Standard_OVERRIDE;

protected:
private:
  TopoSolid mySolid;
};

#endif // _TopOpeBRepBuild_ShellFaceSet_HeaderFile
