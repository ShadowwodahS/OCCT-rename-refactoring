// Copyright (c) 2021 OPEN CASCADE SAS
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

#ifndef _Poly_ArrayOfNodes_HeaderFile
#define _Poly_ArrayOfNodes_HeaderFile

#include <NCollection_AliasedArray.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec3f.hxx>
#include <Standard_Macro.hxx>

//! Defines an array of 3D nodes of single/double precision configurable at construction time.
class NodeArray : public NCollection_AliasedArray<>
{
public:
  //! Empty constructor of double-precision array.
  NodeArray()
      : NCollection_AliasedArray((Standard_Integer)sizeof(Point3d))
  {
    //
  }

  //! Constructor of double-precision array.
  NodeArray(Standard_Integer theLength)
      : NCollection_AliasedArray((Standard_Integer)sizeof(Point3d), theLength)
  {
    //
  }

  //! Copy constructor
  Standard_EXPORT NodeArray(const NodeArray& theOther);

  //! Constructor wrapping pre-allocated C-array of values without copying them.
  NodeArray(const Point3d& theBegin, Standard_Integer theLength)
      : NCollection_AliasedArray(theBegin, theLength)
  {
    //
  }

  //! Constructor wrapping pre-allocated C-array of values without copying them.
  NodeArray(const gp_Vec3f& theBegin, Standard_Integer theLength)
      : NCollection_AliasedArray(theBegin, theLength)
  {
    //
  }

  //! Destructor.
  Standard_EXPORT ~NodeArray();

  //! Returns TRUE if array defines nodes with double precision.
  bool IsDoublePrecision() const { return myStride == (Standard_Integer)sizeof(Point3d); }

  //! Sets if array should define nodes with double or single precision.
  //! Raises exception if array was already allocated.
  void SetDoublePrecision(bool theIsDouble)
  {
    if (myData != NULL)
    {
      throw Standard_ProgramError(
        "NodeArray::SetDoublePrecision() should be called before allocation");
    }
    myStride = Standard_Integer(theIsDouble ? sizeof(Point3d) : sizeof(gp_Vec3f));
  }

  //! Copies data of theOther array to this.
  //! The arrays should have the same length,
  //! but may have different precision / number of components (data conversion will be applied in
  //! the latter case).
  Standard_EXPORT NodeArray& Assign(const NodeArray& theOther);

  //! Move assignment.
  NodeArray& Move(NodeArray& theOther)
  {
    NCollection_AliasedArray::Move(theOther);
    return *this;
  }

  //! Assignment operator; @sa Assign()
  NodeArray& operator=(const NodeArray& theOther) { return Assign(theOther); }

  //! Move constructor
  NodeArray(NodeArray&& theOther) Standard_Noexcept
      : NCollection_AliasedArray(std::move(theOther))
  {
    //
  }

  //! Move assignment operator; @sa Move()
  NodeArray& operator=(NodeArray&& theOther) Standard_Noexcept
  {
    return Move(theOther);
  }

public:
  //! A generalized accessor to point.
  inline Point3d Value(Standard_Integer theIndex) const;

  //! A generalized setter for point.
  inline void SetValue(Standard_Integer theIndex, const Point3d& theValue);

  //! operator[] - alias to Value
  Point3d operator[](Standard_Integer theIndex) const { return Value(theIndex); }
};

// =======================================================================
// function : Value
// purpose  :
// =======================================================================
inline Point3d NodeArray::Value(Standard_Integer theIndex) const
{
  if (myStride == (Standard_Integer)sizeof(Point3d))
  {
    return NCollection_AliasedArray::Value<Point3d>(theIndex);
  }
  else
  {
    const gp_Vec3f& aVec3 = NCollection_AliasedArray::Value<gp_Vec3f>(theIndex);
    return Point3d(aVec3.x(), aVec3.y(), aVec3.z());
  }
}

// =======================================================================
// function : SetValue
// purpose  :
// =======================================================================
inline void NodeArray::SetValue(Standard_Integer theIndex, const Point3d& theValue)
{
  if (myStride == (Standard_Integer)sizeof(Point3d))
  {
    NCollection_AliasedArray::ChangeValue<Point3d>(theIndex) = theValue;
  }
  else
  {
    gp_Vec3f& aVec3 = NCollection_AliasedArray::ChangeValue<gp_Vec3f>(theIndex);
    aVec3.SetValues((float)theValue.X(), (float)theValue.Y(), (float)theValue.Z());
  }
}

#endif // _Poly_ArrayOfNodes_HeaderFile
