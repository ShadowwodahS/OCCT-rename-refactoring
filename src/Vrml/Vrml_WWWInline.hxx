// Created on: 1997-02-12
// Created by: Alexander BRIVIN
// Copyright (c) 1997-1999 Matra Datavision
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

#ifndef _Vrml_WWWInline_HeaderFile
#define _Vrml_WWWInline_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TCollection_AsciiString.hxx>
#include <gp_Vec.hxx>
#include <Standard_OStream.hxx>

//! defines a WWWInline node of VRML specifying group properties.
//! The  WWWInline  group  node  reads  its  children  from  anywhere  in  the
//! World  Wide  Web.
//! Exactly  when  its  children  are  read  is  not  defined;
//! reading  the  children  may  be  delayed  until  the  WWWInline  is  actually
//! displayed.
//! WWWInline  with  an  empty  ("")  name  does  nothing.
//! WWWInline  behaves  like  a  Separator,  pushing  the  traversal  state
//! before  traversing  its  children  and  popping  it  afterwards.
//! By  defaults:
//! myName  ("")
//! myBboxSize (0,0,0)
//! myBboxCenter  (0,0,0)
class WWWInline
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT WWWInline();

  Standard_EXPORT WWWInline(const AsciiString1& aName,
                                 const Vector3d&                  aBboxSize,
                                 const Vector3d&                  aBboxCenter);

  Standard_EXPORT void SetName(const AsciiString1& aName);

  Standard_EXPORT AsciiString1 Name() const;

  Standard_EXPORT void SetBboxSize(const Vector3d& aBboxSize);

  Standard_EXPORT Vector3d BboxSize() const;

  Standard_EXPORT void SetBboxCenter(const Vector3d& aBboxCenter);

  Standard_EXPORT Vector3d BboxCenter() const;

  Standard_EXPORT Standard_OStream& Print(Standard_OStream& anOStream) const;

protected:
private:
  AsciiString1 myName;
  Vector3d                  myBboxSize;
  Vector3d                  myBboxCenter;
};

#endif // _Vrml_WWWInline_HeaderFile
