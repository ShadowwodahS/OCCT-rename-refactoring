// Created on: 1993-07-01
// Created by: Remi LEQUETTE
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

#ifndef _TopTools_ShapeSet_HeaderFile
#define _TopTools_ShapeSet_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_LocationSet.hxx>
#include <Standard_Integer.hxx>
#include <Standard_OStream.hxx>
#include <Standard_IStream.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopTools_FormatVersion.hxx>

class TopoShape;
class AsciiString1;

//! A ShapeSets    contains  a  Shape    and all   its
//! sub-shapes and locations.  It  can be dump,  write
//! and read.
//!
//! Methods to handle the geometry can be redefined.
class TopTools_ShapeSet
{
public:
  DEFINE_STANDARD_ALLOC

  //! Builds an empty ShapeSet.
  Standard_EXPORT TopTools_ShapeSet();

  Standard_EXPORT virtual ~TopTools_ShapeSet();

  //! Sets the TopTools_FormatVersion
  Standard_EXPORT void SetFormatNb(const Standard_Integer theFormatNb);

  //! Returns the TopTools_FormatVersion
  Standard_EXPORT Standard_Integer FormatNb() const;

  //! Clears the content of the set.  This method can be
  //! redefined.
  Standard_EXPORT virtual void Clear();

  //! Stores <S> and its sub-shape. Returns the index of <S>.
  //! The method AddGeometry is called on each sub-shape.
  Standard_EXPORT Standard_Integer Add(const TopoShape& S);

  //! Returns the sub-shape of index <I>.
  Standard_EXPORT const TopoShape& Shape(const Standard_Integer I) const;

  //! Returns the index of <S>.
  Standard_EXPORT Standard_Integer Index(const TopoShape& S) const;

  Standard_EXPORT const LocationSet1& Locations() const;

  Standard_EXPORT LocationSet1& ChangeLocations();

  //! Dumps the number of objects in me on the stream <OS>.
  //! (Number of shapes of each type)
  Standard_EXPORT Standard_OStream& DumpExtent(Standard_OStream& OS) const;

  //! Dumps the number of objects in me in the string S
  //! (Number of shapes of each type)
  Standard_EXPORT void DumpExtent(AsciiString1& S) const;

  //! Dumps the content of me on the stream <OS>.
  //!
  //! Dumps the shapes from first to last.
  //! For each Shape
  //! Dump the type, the flags, the subshapes
  //! calls DumpGeometry(S)
  //!
  //! Dumps the geometry calling DumpGeometry.
  //!
  //! Dumps the locations.
  Standard_EXPORT virtual void Dump(Standard_OStream& OS) const;

  //! Writes the content of  me  on the stream <OS> in a
  //! format that can be read back by Read.
  //!
  //! Writes the locations.
  //!
  //! Writes the geometry calling WriteGeometry.
  //!
  //! Dumps the shapes from last to first.
  //! For each shape  :
  //! Write the type.
  //! calls WriteGeometry(S).
  //! Write the flags, the subshapes.
  Standard_EXPORT virtual void Write(
    Standard_OStream&            OS,
    const Message_ProgressRange& theProgress = Message_ProgressRange());

  //! Reads the content of me from the  stream  <IS>. me
  //! is first cleared.
  //!
  //! Reads the locations.
  //!
  //! Reads the geometry calling ReadGeometry.
  //!
  //! Reads the shapes.
  //! For each shape
  //! Reads the type.
  //! calls ReadGeometry(T,S).
  //! Reads the flag, the subshapes.
  Standard_EXPORT virtual void Read(
    Standard_IStream&            IS,
    const Message_ProgressRange& theProgress = Message_ProgressRange());

  //! Dumps   on  <OS>    the  shape  <S>.   Dumps   the
  //! orientation, the index of the TShape and the index
  //! of the Location.
  Standard_EXPORT void Dump(const TopoShape& S, Standard_OStream& OS) const;

  //! Writes   on  <OS>   the shape   <S>.    Writes the
  //! orientation, the index of the TShape and the index
  //! of the Location.
  Standard_EXPORT void Write(const TopoShape& S, Standard_OStream& OS) const;

  //! Reads from <IS> a shape and returns it in S.
  Standard_EXPORT void Read(TopoShape& S, Standard_IStream& IS) const;

  //! Stores the geometry of <S>.
  Standard_EXPORT virtual void AddGeometry(const TopoShape& S);

  //! Dumps the geometry of me on the stream <OS>.
  Standard_EXPORT virtual void DumpGeometry(Standard_OStream& OS) const;

  //! Writes the geometry of  me  on the stream <OS> in a
  //! format that can be read back by Read.
  Standard_EXPORT virtual void WriteGeometry(
    Standard_OStream&            OS,
    const Message_ProgressRange& theProgress = Message_ProgressRange());

  //! Reads the geometry of me from the  stream  <IS>.
  Standard_EXPORT virtual void ReadGeometry(
    Standard_IStream&            IS,
    const Message_ProgressRange& theProgress = Message_ProgressRange());

  //! Dumps the geometry of <S> on the stream <OS>.
  Standard_EXPORT virtual void DumpGeometry(const TopoShape& S, Standard_OStream& OS) const;

  //! Writes the geometry of <S>  on the stream <OS> in a
  //! format that can be read back by Read.
  Standard_EXPORT virtual void WriteGeometry(const TopoShape& S, Standard_OStream& OS) const;

  //! Reads the geometry of a shape of type <T> from the
  //! stream <IS> and returns it in <S>.
  Standard_EXPORT virtual void ReadGeometry(const TopAbs_ShapeEnum T,
                                            Standard_IStream&      IS,
                                            TopoShape&          S);

  //! Inserts  the shape <S2> in  the  shape <S1>.  This
  //! method must be   redefined  to  use   the  correct
  //! builder.
  Standard_EXPORT virtual void AddShapes(TopoShape& S1, const TopoShape& S2);

  //! This method is   called after  each  new  completed
  //! shape. <T> is the  type. <S> is  the shape. In this
  //! class it does nothing, but it gives the opportunity
  //! in derived  classes to perform  extra  treatment on
  //! shapes.
  Standard_EXPORT virtual void Check(const TopAbs_ShapeEnum T, TopoShape& S);

  //! Returns number of shapes read from file.
  Standard_EXPORT Standard_Integer NbShapes1() const;

public:
  static const Standard_CString THE_ASCII_VERSIONS[TopTools_FormatVersion_VERSION_3 + 1];

private:
  //! Reads  from <IS>  a shape  and  returns  it in  S.
  //! <NbShapes1> is the number of tshapes in the set.
  Standard_EXPORT void Read(TopoShape&          S,
                            Standard_IStream&      IS,
                            const Standard_Integer NbShapes1) const;

  TopTools_IndexedMapOfShape myShapes;
  LocationSet1       myLocations;
  Standard_Integer           myFormatNb;
};

#endif // _TopTools_ShapeSet_HeaderFile
