// Copyright (c) 1991-1999 Matra Datavision
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

#ifndef _gp_XY_HeaderFile
#define _gp_XY_HeaderFile

#include <gp.hxx>
#include <gp_Mat2d.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_OutOfRange.hxx>

//! This class describes a cartesian coordinate entity in 2D
//! space {X,Y}. This class is non persistent. This entity used
//! for algebraic calculation. An XY can be transformed with a
//! Trsf2d or a  GTrsf2d from package gp1.
//! It is used in vectorial computations or for holding this type
//! of information in data structures.
class Coords2d
{
public:
  DEFINE_STANDARD_ALLOC

  //! Creates XY object with zero coordinates (0,0).
  Coords2d()
      : x(0.),
        y(0.)
  {
  }

  //! a number pair defined by the XY coordinates
  Coords2d(const Standard_Real theX, const Standard_Real theY)
      : x(theX),
        y(theY)
  {
  }

  //! modifies the coordinate of range theIndex
  //! theIndex = 1 => X is modified
  //! theIndex = 2 => Y is modified
  //! Raises OutOfRange if theIndex != {1, 2}.
  inline void SetCoord(const Standard_Integer theIndex, const Standard_Real theXi)
  {
    Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 2, NULL);
    (&x)[theIndex - 1] = theXi;
  }

  //! For this number pair, assigns
  //! the values theX and theY to its coordinates
  inline void SetCoord(const Standard_Real theX, const Standard_Real theY)
  {
    x = theX;
    y = theY;
  }

  //! Assigns the given value to the X coordinate of this number pair.
  void SetX(const Standard_Real theX) { x = theX; }

  //! Assigns the given value to the Y  coordinate of this number pair.
  void SetY(const Standard_Real theY) { y = theY; }

  //! returns the coordinate of range theIndex :
  //! theIndex = 1 => X is returned
  //! theIndex = 2 => Y is returned
  //! Raises OutOfRange if theIndex != {1, 2}.
  inline Standard_Real Coord(const Standard_Integer theIndex) const
  {
    Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 2, NULL);
    return (&x)[theIndex - 1];
  }

  inline Standard_Real& ChangeCoord(const Standard_Integer theIndex)
  {
    Standard_OutOfRange_Raise_if(theIndex < 1 || theIndex > 2, NULL);
    return (&x)[theIndex - 1];
  }

  //! For this number pair, returns its coordinates X and Y.
  inline void Coord(Standard_Real& theX, Standard_Real& theY) const
  {
    theX = x;
    theY = y;
  }

  //! Returns the X coordinate of this number pair.
  Standard_Real X() const { return x; }

  //! Returns the Y coordinate of this number pair.
  Standard_Real Y() const { return y; }

  //! Computes Sqrt (X*X + Y*Y) where X and Y are the two coordinates of this number pair.
  Standard_Real Modulus() const { return sqrt(x * x + y * y); }

  //! Computes X*X + Y*Y where X and Y are the two coordinates of this number pair.
  Standard_Real SquareModulus() const { return x * x + y * y; }

  //! Returns true if the coordinates of this number pair are
  //! equal to the respective coordinates of the number pair
  //! theOther, within the specified tolerance theTolerance. I.e.:
  //! abs(<me>.X() - theOther.X()) <= theTolerance and
  //! abs(<me>.Y() - theOther.Y()) <= theTolerance and
  //! computations
  Standard_EXPORT Standard_Boolean IsEqual(const Coords2d&        theOther,
                                           const Standard_Real theTolerance) const;

  //! Computes the sum of this number pair and number pair theOther
  //! @code
  //! <me>.X() = <me>.X() + theOther.X()
  //! <me>.Y() = <me>.Y() + theOther.Y()
  //! @endcode
  inline void Add(const Coords2d& theOther)
  {
    x += theOther.x;
    y += theOther.y;
  }

  void operator+=(const Coords2d& theOther) { Add(theOther); }

  //! Computes the sum of this number pair and number pair theOther
  //! @code
  //! new.X() = <me>.X() + theOther.X()
  //! new.Y() = <me>.Y() + theOther.Y()
  //! @endcode
  Standard_NODISCARD Coords2d Added(const Coords2d& theOther) const
  {
    return Coords2d(x + theOther.X(), y + theOther.Y());
  }

  Standard_NODISCARD Coords2d operator+(const Coords2d& theOther) const { return Added(theOther); }

  //! @code
  //! double D = <me>.X() * theOther.Y() - <me>.Y() * theOther.X()
  //! @endcode
  Standard_NODISCARD Standard_Real Crossed(const Coords2d& theOther) const
  {
    return x * theOther.y - y * theOther.x;
  }

  Standard_NODISCARD Standard_Real operator^(const Coords2d& theOther) const
  {
    return Crossed(theOther);
  }

  //! computes the magnitude of the cross product between <me> and
  //! theRight. Returns || <me> ^ theRight ||
  inline Standard_Real CrossMagnitude(const Coords2d& theRight) const
  {
    Standard_Real aVal = x * theRight.y - y * theRight.x;
    return aVal < 0 ? -aVal : aVal;
  }

  //! computes the square magnitude of the cross product between <me> and
  //! theRight. Returns || <me> ^ theRight ||**2
  inline Standard_Real CrossSquareMagnitude(const Coords2d& theRight) const
  {
    Standard_Real aZresult = x * theRight.y - y * theRight.x;
    return aZresult * aZresult;
  }

  //! divides <me> by a real.
  void Divide(const Standard_Real theScalar)
  {
    x /= theScalar;
    y /= theScalar;
  }

  void operator/=(const Standard_Real theScalar) { Divide(theScalar); }

  //! Divides <me> by a real.
  Standard_NODISCARD Coords2d Divided(const Standard_Real theScalar) const
  {
    return Coords2d(x / theScalar, y / theScalar);
  }

  Standard_NODISCARD Coords2d operator/(const Standard_Real theScalar) const
  {
    return Divided(theScalar);
  }

  //! Computes the scalar product between <me> and theOther
  Standard_Real Dot(const Coords2d& theOther) const { return x * theOther.x + y * theOther.y; }

  Standard_Real operator*(const Coords2d& theOther) const { return Dot(theOther); }

  //! @code
  //! <me>.X() = <me>.X() * theScalar;
  //! <me>.Y() = <me>.Y() * theScalar;
  //! @endcode
  void Multiply(const Standard_Real theScalar)
  {
    x *= theScalar;
    y *= theScalar;
  }

  void operator*=(const Standard_Real theScalar) { Multiply(theScalar); }

  //! @code
  //! <me>.X() = <me>.X() * theOther.X();
  //! <me>.Y() = <me>.Y() * theOther.Y();
  //! @endcode
  void Multiply(const Coords2d& theOther)
  {
    x *= theOther.x;
    y *= theOther.y;
  }

  void operator*=(const Coords2d& theOther) { Multiply(theOther); }

  //! <me> = theMatrix * <me>
  void Multiply(const Matrix2d& theMatrix);

  void operator*=(const Matrix2d& theMatrix) { Multiply(theMatrix); }

  //! @code
  //! New.X() = <me>.X() * theScalar;
  //! New.Y() = <me>.Y() * theScalar;
  //! @endcode
  Standard_NODISCARD Coords2d Multiplied(const Standard_Real theScalar) const
  {
    return Coords2d(x * theScalar, y * theScalar);
  }

  Standard_NODISCARD Coords2d operator*(const Standard_Real theScalar) const
  {
    return Multiplied(theScalar);
  }

  //! @code
  //! new.X() = <me>.X() * theOther.X();
  //! new.Y() = <me>.Y() * theOther.Y();
  //! @endcode
  Standard_NODISCARD Coords2d Multiplied(const Coords2d& theOther) const
  {
    return Coords2d(x * theOther.X(), y * theOther.Y());
  }

  //! New = theMatrix * <me>
  Standard_NODISCARD Coords2d Multiplied(const Matrix2d& theMatrix) const
  {
    return Coords2d(theMatrix.Value(1, 1) * x + theMatrix.Value(1, 2) * y,
                 theMatrix.Value(2, 1) * x + theMatrix.Value(2, 2) * y);
  }

  Standard_NODISCARD Coords2d operator*(const Matrix2d& theMatrix) const
  {
    return Multiplied(theMatrix);
  }

  //! @code
  //! <me>.X() = <me>.X()/ <me>.Modulus()
  //! <me>.Y() = <me>.Y()/ <me>.Modulus()
  //! @endcode
  //! Raises ConstructionError if <me>.Modulus() <= Resolution from gp1
  void Normalize();

  //! @code
  //! New.X() = <me>.X()/ <me>.Modulus()
  //! New.Y() = <me>.Y()/ <me>.Modulus()
  //! @endcode
  //! Raises ConstructionError if <me>.Modulus() <= Resolution from gp1
  Standard_NODISCARD Coords2d Normalized() const
  {
    Standard_Real aD = Modulus();
    Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                        "Coords2d::Normalized() - vector has zero norm");
    return Coords2d(x / aD, y / aD);
  }

  //! @code
  //! <me>.X() = -<me>.X()
  //! <me>.Y() = -<me>.Y()
  inline void Reverse()
  {
    x = -x;
    y = -y;
  }

  //! @code
  //! New.X() = -<me>.X()
  //! New.Y() = -<me>.Y()
  //! @endcode
  Standard_NODISCARD Coords2d Reversed() const
  {
    Coords2d aCoord2D = *this;
    aCoord2D.Reverse();
    return aCoord2D;
  }

  Standard_NODISCARD Coords2d operator-() const { return Reversed(); }

  //! Computes  the following linear combination and
  //! assigns the result to this number pair:
  //! @code
  //! theA1 * theXY1 + theA2 * theXY2
  //! @endcode
  inline void SetLinearForm(const Standard_Real theA1,
                            const Coords2d&        theXY1,
                            const Standard_Real theA2,
                            const Coords2d&        theXY2)
  {
    x = theA1 * theXY1.x + theA2 * theXY2.x;
    y = theA1 * theXY1.y + theA2 * theXY2.y;
  }

  //! --  Computes  the following linear combination and
  //! assigns the result to this number pair:
  //! @code
  //! theA1 * theXY1 + theA2 * theXY2 + theXY3
  //! @endcode
  inline void SetLinearForm(const Standard_Real theA1,
                            const Coords2d&        theXY1,
                            const Standard_Real theA2,
                            const Coords2d&        theXY2,
                            const Coords2d&        theXY3)
  {
    x = theA1 * theXY1.x + theA2 * theXY2.x + theXY3.x;
    y = theA1 * theXY1.y + theA2 * theXY2.y + theXY3.y;
  }

  //! Computes  the following linear combination and
  //! assigns the result to this number pair:
  //! @code
  //! theA1 * theXY1 + theXY2
  //! @endcode
  inline void SetLinearForm(const Standard_Real theA1, const Coords2d& theXY1, const Coords2d& theXY2)
  {
    x = theA1 * theXY1.x + theXY2.x;
    y = theA1 * theXY1.y + theXY2.y;
  }

  //! Computes  the following linear combination and
  //! assigns the result to this number pair:
  //! @code
  //! theXY1 + theXY2
  //! @endcode
  inline void SetLinearForm(const Coords2d& theXY1, const Coords2d& theXY2)
  {
    x = theXY1.x + theXY2.x;
    y = theXY1.y + theXY2.y;
  }

  //! @code
  //! <me>.X() = <me>.X() - theOther.X()
  //! <me>.Y() = <me>.Y() - theOther.Y()
  //! @endcode
  inline void Subtract(const Coords2d& theOther)
  {
    x -= theOther.x;
    y -= theOther.y;
  }

  void operator-=(const Coords2d& theOther) { Subtract(theOther); }

  //! @code
  //! new.X() = <me>.X() - theOther.X()
  //! new.Y() = <me>.Y() - theOther.Y()
  //! @endcode
  Standard_NODISCARD Coords2d Subtracted(const Coords2d& theOther) const
  {
    Coords2d aCoord2D = *this;
    aCoord2D.Subtract(theOther);
    return aCoord2D;
  }

  Standard_NODISCARD Coords2d operator-(const Coords2d& theOther) const { return Subtracted(theOther); }

private:
  Standard_Real x;
  Standard_Real y;
};

//=======================================================================
// function :  Multiply
// purpose :
//=======================================================================
inline void Coords2d::Multiply(const Matrix2d& theMatrix)
{
  Standard_Real aXresult = theMatrix.Value(1, 1) * x + theMatrix.Value(1, 2) * y;
  y                      = theMatrix.Value(2, 1) * x + theMatrix.Value(2, 2) * y;
  x                      = aXresult;
}

//=======================================================================
// function :  Normalize
// purpose :
//=======================================================================
inline void Coords2d::Normalize()
{
  Standard_Real aD = Modulus();
  Standard_ConstructionError_Raise_if(aD <= gp1::Resolution(),
                                      "Coords2d::Normalize() - vector has zero norm");
  x = x / aD;
  y = y / aD;
}

//=======================================================================
// function :  operator*
// purpose :
//=======================================================================
inline Coords2d operator*(const Matrix2d& theMatrix, const Coords2d& theCoord1)
{
  return theCoord1.Multiplied(theMatrix);
}

//=======================================================================
// function :  operator*
// purpose :
//=======================================================================
inline Coords2d operator*(const Standard_Real theScalar, const Coords2d& theCoord1)
{
  return theCoord1.Multiplied(theScalar);
}

#endif // _gp_XY_HeaderFile
