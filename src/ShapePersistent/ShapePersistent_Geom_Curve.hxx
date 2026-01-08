// Copyright (c) 2015 OPEN CASCADE SAS
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

#ifndef _ShapePersistent_Geom_Curve_HeaderFile
#define _ShapePersistent_Geom_Curve_HeaderFile

#include <StdObjMgt_TransientPersistentMap.hxx>

#include <ShapePersistent_Geom.hxx>
#include <ShapePersistent_HArray1.hxx>
#include <StdLPersistent_HArray1.hxx>

#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_OffsetCurve.hxx>

#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Parab.hxx>

class ShapePersistent_Geom_Curve : private ShapePersistent_Geom
{
  typedef Curve::PersistentBase pBase;

  typedef pBase pBounded;

  class pBezier1 : public pBounded
  {
    friend class ShapePersistent_Geom_Curve;

  public:
    pBezier1()
        : myRational(Standard_False)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myRational >> myPoles >> myWeights;
    }

    inline void Write(WriteData& theWriteData)
    {
      theWriteData << myRational << myPoles << myWeights;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myPoles);
      theChildren.Append(myWeights);
    }

    inline Standard_CString PName() const { return "PGeom_BezierCurve"; }

    virtual Handle(GeomCurve3d) Import() const;

  private:
    Standard_Boolean                     myRational;
    Handle(ShapePersistent_HArray1::Pnt) myPoles;
    Handle(HArray1::Real) myWeights;
  };

  class pBSpline : public pBounded
  {
    friend class ShapePersistent_Geom_Curve;

  public:
    pBSpline()
        : myRational(Standard_False),
          myPeriodic(Standard_False),
          mySpineDegree(0)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myRational >> myPeriodic >> mySpineDegree;
      theReadData >> myPoles >> myWeights >> myKnots >> myMultiplicities;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myRational << myPeriodic << mySpineDegree;
      theWriteData << myPoles << myWeights << myKnots << myMultiplicities;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myPoles);
      theChildren.Append(myWeights);
      theChildren.Append(myKnots);
      theChildren.Append(myMultiplicities);
    }

    inline Standard_CString PName() const { return "PGeom_BSplineCurve"; }

    virtual Handle(GeomCurve3d) Import() const;

  private:
    Standard_Boolean                        myRational;
    Standard_Boolean                        myPeriodic;
    Standard_Integer                        mySpineDegree;
    Handle(ShapePersistent_HArray1::Pnt)    myPoles;
    Handle(HArray1::Real)    myWeights;
    Handle(HArray1::Real)    myKnots;
    Handle(HArray1::Integer1) myMultiplicities;
  };

  class pTrimmed1 : public pBounded
  {
    friend class ShapePersistent_Geom_Curve;

  public:
    pTrimmed1()
        : myFirstU(0.0),
          myLastU(0.0)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myBasisCurve >> myFirstU >> myLastU;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myBasisCurve << myFirstU << myLastU;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myBasisCurve);
    }

    inline Standard_CString PName() const { return "PGeom_TrimmedCurve"; }

    virtual Handle(GeomCurve3d) Import() const;

  private:
    Handle(Curve) myBasisCurve;
    Standard_Real myFirstU;
    Standard_Real myLastU;
  };

  class pOffset1 : public pBase
  {
    friend class ShapePersistent_Geom_Curve;

  public:
    pOffset1()
        : myOffsetValue(0.0)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myBasisCurve >> myOffsetDirection >> myOffsetValue;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myBasisCurve << myOffsetDirection << myOffsetValue;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myBasisCurve);
    }

    inline Standard_CString PName() const { return "PGeom_OffsetCurve"; }

    virtual Handle(GeomCurve3d) Import() const;

  private:
    Handle(Curve) myBasisCurve;
    Dir3d        myOffsetDirection;
    Standard_Real myOffsetValue;
  };

public:
  typedef instance<Curve, GeomLine, Axis3d> Line;

  typedef subBase_gp<Curve, Frame3d>                Conic;
  typedef instance<Conic, GeomCircle, gp_Circ>    Circle;
  typedef instance<Conic, Geom_Ellipse, gp_Elips>  Ellipse;
  typedef instance<Conic, Geom_Hyperbola, gp_Hypr> Hyperbola;
  typedef instance<Conic, Geom_Parabola, gp_Parab> Parabola;

  typedef subBase_empty<Curve>       Bounded;
  typedef Delayed<Bounded, pBezier1>  Bezier;
  typedef Delayed<Bounded, pBSpline> BSpline;
  typedef Delayed<Bounded, pTrimmed1> Trimmed;

  typedef Delayed<Curve, pOffset1> Offset;

public:
  //! Create a persistent object for a line
  Standard_EXPORT static Handle(Curve) Translate(const Handle(GeomLine)&          theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a circle
  Standard_EXPORT static Handle(Curve) Translate(const Handle(GeomCircle)&        theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a ellipse
  Standard_EXPORT static Handle(Curve) Translate(const Handle(Geom_Ellipse)&       theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a hyperbola
  Standard_EXPORT static Handle(Curve) Translate(const Handle(Geom_Hyperbola)&     theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a parabola
  Standard_EXPORT static Handle(Curve) Translate(const Handle(Geom_Parabola)&      theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a Bezier curve
  Standard_EXPORT static Handle(Curve) Translate(const Handle(BezierCurve3d)&   theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a BSpline curve
  Standard_EXPORT static Handle(Curve) Translate(const Handle(BSplineCurve3d)&  theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a trimmed curve
  Standard_EXPORT static Handle(Curve) Translate(const Handle(Geom_TrimmedCurve)&  theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for an offset curve
  Standard_EXPORT static Handle(Curve) Translate(const Handle(Geom_OffsetCurve)&   theCurve,
                                                 StdObjMgt_TransientPersistentMap& theMap);
};

//=======================================================================
// Line
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::instance<ShapePersistent_Geom::Curve, GeomLine, Axis3d>::
  PName() const;

template <>
void ShapePersistent_Geom::instance<ShapePersistent_Geom::Curve, GeomLine, Axis3d>::Write(
  WriteData& theWriteData) const;

//=======================================================================
// Conic
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Curve, Frame3d>::PName()
  const;

//=======================================================================
// Circle
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::
  instance<ShapePersistent_Geom_Curve::Conic, GeomCircle, gp_Circ>::PName() const;

template <>
void ShapePersistent_Geom::instance<ShapePersistent_Geom_Curve::Conic, GeomCircle, gp_Circ>::Write(
  WriteData& theWriteData) const;

//=======================================================================
// Ellipse
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::
  instance<ShapePersistent_Geom_Curve::Conic, Geom_Ellipse, gp_Elips>::PName() const;

template <>
void ShapePersistent_Geom::instance<ShapePersistent_Geom_Curve::Conic, Geom_Ellipse, gp_Elips>::
  Write(WriteData& theWriteData) const;

//=======================================================================
// Hyperbola
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::
  instance<ShapePersistent_Geom_Curve::Conic, Geom_Hyperbola, gp_Hypr>::PName() const;

template <>
void ShapePersistent_Geom::instance<ShapePersistent_Geom_Curve::Conic, Geom_Hyperbola, gp_Hypr>::
  Write(WriteData& theWriteData) const;

//=======================================================================
// Parabola
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::
  instance<ShapePersistent_Geom_Curve::Conic, Geom_Parabola, gp_Parab>::PName() const;

template <>
void ShapePersistent_Geom::instance<ShapePersistent_Geom_Curve::Conic, Geom_Parabola, gp_Parab>::
  Write(WriteData& theWriteData) const;

#endif
