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

#ifndef _Graphic3d_ShaderVariable_HeaderFile
#define _Graphic3d_ShaderVariable_HeaderFile

#include <Graphic3d_Vec.hxx>
#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>

//! Interface for generic variable value.
struct ValueInterface
{
  //! Releases memory resources of variable value.
  Standard_EXPORT virtual ~ValueInterface();

  //! Returns unique identifier of value type.
  virtual Standard_Size TypeID() const = 0;

  //! Returns variable value casted to specified type.
  template <class T>
  T& As();

  //! Returns variable value casted to specified type.
  template <class T>
  const T& As() const;
};

//! Generates unique type identifier for variable value.
template <class T>
struct UniformValueTypeID
{
  /* Not implemented */
};

template <>
struct UniformValueTypeID<Standard_Integer>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Standard_ShortReal>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Graphic3d_Vec2>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Graphic3d_Vec3>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Graphic3d_Vec4>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Graphic3d_Vec2i>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Graphic3d_Vec3i>
{
  static const Standard_Size ID = __LINE__;
};

template <>
struct UniformValueTypeID<Graphic3d_Vec4i>
{
  static const Standard_Size ID = __LINE__;
};

//! Describes specific value of custom uniform variable.
template <class T>
struct Graphic3d_UniformValue : public ValueInterface
{
  //! Creates new variable value.
  Graphic3d_UniformValue(const T& theValue)
      : Value(theValue)
  {
  }

  //! Returns unique identifier of value type.
  virtual Standard_Size TypeID() const;

  //! Value of custom uniform variable.
  T Value;
};

//! Integer1 uniform value.
typedef Graphic3d_UniformValue<Standard_Integer> Graphic3d_UniformInt;

//! Integer1 uniform 2D vector.
typedef Graphic3d_UniformValue<Graphic3d_Vec2i> Graphic3d_UniformVec2i;

//! Integer1 uniform 3D vector.
typedef Graphic3d_UniformValue<Graphic3d_Vec3i> Graphic3d_UniformVec3i;

//! Integer1 uniform 4D vector.
typedef Graphic3d_UniformValue<Graphic3d_Vec4i> Graphic3d_UniformVec4i;

//! Floating-point uniform value.
typedef Graphic3d_UniformValue<Standard_ShortReal> Graphic3d_UniformFloat;

//! Floating-point uniform 2D vector.
typedef Graphic3d_UniformValue<Graphic3d_Vec2> Graphic3d_UniformVec2;

//! Floating-point uniform 3D vector.
typedef Graphic3d_UniformValue<Graphic3d_Vec3> Graphic3d_UniformVec3;

//! Floating-point uniform 4D vector.
typedef Graphic3d_UniformValue<Graphic3d_Vec4> Graphic3d_UniformVec4;

//! Describes custom uniform shader variable.
class ShaderVariable1 : public RefObject
{
public:
  //! Releases resources of shader variable.
  Standard_EXPORT virtual ~ShaderVariable1();

  //! Returns name of shader variable.
  Standard_EXPORT const AsciiString1& Name() const;

  //! Checks if the shader variable is valid or not.
  Standard_EXPORT Standard_Boolean IsDone() const;

  //! Returns interface of shader variable value.
  Standard_EXPORT ValueInterface* Value();

  //! Creates new initialized shader variable.
  template <class T>
  static ShaderVariable1* Create(const AsciiString1& theName,
                                          const T&                       theValue);

public:
  DEFINE_STANDARD_RTTIEXT(ShaderVariable1, RefObject)

protected:
  //! Creates new uninitialized shader variable.
  Standard_EXPORT ShaderVariable1(const AsciiString1& theName);

protected:
  //! The name of uniform shader variable.
  AsciiString1 myName;

  //! The generic value of shader variable.
  ValueInterface* myValue;
};

DEFINE_STANDARD_HANDLE(ShaderVariable1, RefObject)

#include <Graphic3d_ShaderVariable.lxx>

#endif // _Graphic3d_ShaderVariable_HeaderFile
