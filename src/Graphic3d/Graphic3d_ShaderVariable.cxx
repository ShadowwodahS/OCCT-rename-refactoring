// Created on: 2013-09-25
// Created by: Denis BOGOLEPOV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <Graphic3d_ShaderVariable.hxx>

IMPLEMENT_STANDARD_RTTIEXT(ShaderVariable1, RefObject)

// Specific instantiations of struct templates to avoid compilation warnings
template struct Graphic3d_UniformValue<Standard_Integer>;
template struct Graphic3d_UniformValue<Standard_ShortReal>;
template struct Graphic3d_UniformValue<Graphic3d_Vec2>;
template struct Graphic3d_UniformValue<Graphic3d_Vec3>;
template struct Graphic3d_UniformValue<Graphic3d_Vec4>;
template struct Graphic3d_UniformValue<Graphic3d_Vec2i>;
template struct Graphic3d_UniformValue<Graphic3d_Vec3i>;
template struct Graphic3d_UniformValue<Graphic3d_Vec4i>;

// =======================================================================
// function : ~ValueInterface
// purpose  : Releases memory resources of variable value
// =======================================================================
ValueInterface::~ValueInterface()
{
  //
}

// =======================================================================
// function : ShaderVariable1
// purpose  : Creates new abstract shader variable
// =======================================================================
ShaderVariable1::ShaderVariable1(const AsciiString1& theName)
    : myName(theName),
      myValue(NULL)
{
  //
}

// =======================================================================
// function : ~Graphic3d_ShaderVariableBase
// purpose  : Releases resources of shader variable
// =======================================================================
ShaderVariable1::~ShaderVariable1()
{
  delete myValue;
}

// =======================================================================
// function : IsDone
// purpose  : Checks if the shader variable is valid or not
// =======================================================================
Standard_Boolean ShaderVariable1::IsDone() const
{
  return !myName.IsEmpty() && (myValue != NULL);
}

// =======================================================================
// function : Name
// purpose  : Returns name of shader variable
// =======================================================================
const AsciiString1& ShaderVariable1::Name() const
{
  return myName;
}

// =======================================================================
// function : Value
// purpose  : Returns interface of shader variable value
// =======================================================================
ValueInterface* ShaderVariable1::Value()
{
  return myValue;
}
