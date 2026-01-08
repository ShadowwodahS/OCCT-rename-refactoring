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

#ifndef _ShapePersistent_Geom_Surface_HeaderFile
#define _ShapePersistent_Geom_Surface_HeaderFile

#include <ShapePersistent_Geom.hxx>
#include <ShapePersistent_HArray2.hxx>
#include <StdLPersistent_HArray1.hxx>
#include <StdLPersistent_HArray2.hxx>

#include <Geom_Plane.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_OffsetSurface.hxx>

#include <gp_Ax3.hxx>
#include <gp_Cone.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>

class ShapePersistent_Geom_Surface : private ShapePersistent_Geom
{
  typedef Surface::PersistentBase pBase;

  class pSweptData1
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    inline void Read(ReadData& theReadData)
    {
      theReadData >> myBasisCurve >> myDirection;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myBasisCurve << myDirection;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myBasisCurve);
    }

  protected:
    Handle(Curve) myBasisCurve;
    Dir3d        myDirection;
  };

  struct pSwept : pBase, pSweptData1
  {
    inline Standard_CString PName() const { return "PGeom_SweptSurface"; }
  };

  class pLinearExtrusion : public pSwept
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    virtual Handle(GeomSurface) Import() const;

    inline Standard_CString PName() const { return "PGeom_SurfaceOfLinearExtrusion"; }
  };

  class pRevolution : public pSwept
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    inline void Read(ReadData& theReadData)
    {
      pSwept::Read(theReadData);
      theReadData >> myLocation;
    }

    inline void Write(WriteData& theWriteData) const
    {
      pSwept::Write(theWriteData);
      theWriteData << myLocation;
    }

    inline Standard_CString PName() const { return "PGeom_SurfaceOfRevolution"; }

    virtual Handle(GeomSurface) Import() const;

  private:
    Point3d myLocation;
  };

  typedef pBase pBounded;

  class pBezier1 : public pBounded
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    pBezier1()
        : myURational(Standard_False),
          myVRational(Standard_False)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myURational >> myVRational >> myPoles >> myWeights;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myURational << myVRational << myPoles << myWeights;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myPoles);
      theChildren.Append(myWeights);
    }

    inline Standard_CString PName() const { return "PGeom_BezierSurface"; }

    virtual Handle(GeomSurface) Import() const;

  private:
    Standard_Boolean                     myURational;
    Standard_Boolean                     myVRational;
    Handle(ShapePersistent_HArray2::Pnt) myPoles;
    Handle(HArray2::Real) myWeights;
  };

  class pBSpline : public pBounded
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    pBSpline()
        : myURational(Standard_False),
          myVRational(Standard_False),
          myUPeriodic(Standard_False),
          myVPeriodic(Standard_False),
          myUSpineDegree(0),
          myVSpineDegree(0)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myURational >> myVRational;
      theReadData >> myUPeriodic >> myVPeriodic;
      theReadData >> myUSpineDegree >> myVSpineDegree;
      theReadData >> myPoles;
      theReadData >> myWeights;
      theReadData >> myUKnots >> myVKnots;
      theReadData >> myUMultiplicities >> myVMultiplicities;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myURational << myVRational;
      theWriteData << myUPeriodic << myVPeriodic;
      theWriteData << myUSpineDegree << myVSpineDegree;
      theWriteData << myPoles;
      theWriteData << myWeights;
      theWriteData << myUKnots << myVKnots;
      theWriteData << myUMultiplicities << myVMultiplicities;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myPoles);
      theChildren.Append(myWeights);
      theChildren.Append(myUKnots);
      theChildren.Append(myVKnots);
      theChildren.Append(myUMultiplicities);
      theChildren.Append(myVMultiplicities);
    }

    inline Standard_CString PName() const { return "PGeom_BSplineSurface"; }

    virtual Handle(GeomSurface) Import() const;

  private:
    Standard_Boolean                        myURational;
    Standard_Boolean                        myVRational;
    Standard_Boolean                        myUPeriodic;
    Standard_Boolean                        myVPeriodic;
    Standard_Integer                        myUSpineDegree;
    Standard_Integer                        myVSpineDegree;
    Handle(ShapePersistent_HArray2::Pnt)    myPoles;
    Handle(HArray2::Real)    myWeights;
    Handle(HArray1::Real)    myUKnots;
    Handle(HArray1::Real)    myVKnots;
    Handle(HArray1::Integer1) myUMultiplicities;
    Handle(HArray1::Integer1) myVMultiplicities;
  };

  class pRectangularTrimmed1 : public pBounded
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    pRectangularTrimmed1()
        : myFirstU(0.0),
          myLastU(0.0),
          myFirstV(0.0),
          myLastV(0.0)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myBasisSurface;
      theReadData >> myFirstU >> myLastU >> myFirstV >> myLastV;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myBasisSurface;
      theWriteData << myFirstU << myLastU << myFirstV << myLastV;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myBasisSurface);
    }

    inline Standard_CString PName() const { return "PGeom_RectangularTrimmedSurface"; }

    virtual Handle(GeomSurface) Import() const;

  private:
    Handle(Surface) myBasisSurface;
    Standard_Real   myFirstU;
    Standard_Real   myLastU;
    Standard_Real   myFirstV;
    Standard_Real   myLastV;
  };

  class pOffset1 : public pBase
  {
    friend class ShapePersistent_Geom_Surface;

  public:
    pOffset1()
        : myOffsetValue(0.0)
    {
    }

    inline void Read(ReadData& theReadData)
    {
      theReadData >> myBasisSurface >> myOffsetValue;
    }

    inline void Write(WriteData& theWriteData) const
    {
      theWriteData << myBasisSurface << myOffsetValue;
    }

    inline void PChildren(StdObjMgt_Persistent::SequenceOfPersistent& theChildren) const
    {
      theChildren.Append(myBasisSurface);
    }

    inline Standard_CString PName() const { return "PGeom_OffsetSurface"; }

    virtual Handle(GeomSurface) Import() const;

  private:
    Handle(Surface) myBasisSurface;
    Standard_Real   myOffsetValue;
  };

public:
  typedef subBase_gp<Surface, Ax3>                                Elementary;
  typedef instance<Elementary, GeomPlane, Ax3>                   Plane1;
  typedef instance<Elementary, Geom_ConicalSurface, Cone1>         Conical;
  typedef instance<Elementary, Geom_CylindricalSurface, Cylinder1> Cylindrical;
  typedef instance<Elementary, Geom_SphericalSurface, Sphere3>     Spherical;
  typedef instance<Elementary, Geom_ToroidalSurface, gp_Torus>       Toroidal;

  typedef subBase<Surface, pSweptData1>     Swept;
  typedef Delayed<Swept, pLinearExtrusion> LinearExtrusion;
  typedef Delayed<Swept, pRevolution>      Revolution;

  typedef subBase_empty<Surface>                Bounded;
  typedef Delayed<Bounded, pBezier1>             Bezier;
  typedef Delayed<Bounded, pBSpline>            BSpline;
  typedef Delayed<Bounded, pRectangularTrimmed1> RectangularTrimmed;

  typedef Delayed<Surface, pOffset1> Offset;

public:
  //! Create a persistent object for a plane
  Standard_EXPORT static Handle(Surface) Translate(const Handle(GeomPlane)&         theSurf,
                                                   StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a cylinder
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_CylindricalSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap&      theMap);
  //! Create a persistent object for a cone
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_ConicalSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap&  theMap);
  //! Create a persistent object for a sphere
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_SphericalSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap&    theMap);
  //! Create a persistent object for a torus
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_ToroidalSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap&   theMap);
  //! Create a persistent object for a surface of linear extrusion
  Standard_EXPORT static Handle(Surface) Translate(
    const Handle(Geom_SurfaceOfLinearExtrusion)& theSurf,
    StdObjMgt_TransientPersistentMap&            theMap);
  //! Create a persistent object for a surface of evolution
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_SurfaceOfRevolution)& theSurf,
                                                   StdObjMgt_TransientPersistentMap&       theMap);
  //! Create a persistent object for a Bezier surface
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_BezierSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap& theMap);
  //! Create a persistent object for a BSpline surface
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_BSplineSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap&  theMap);
  //! Create a persistent object for a rectangylar trimmed surface
  Standard_EXPORT static Handle(Surface) Translate(
    const Handle(Geom_RectangularTrimmedSurface)& theSurf,
    StdObjMgt_TransientPersistentMap&             theMap);
  //! Create a persistent object for an offset surface
  Standard_EXPORT static Handle(Surface) Translate(const Handle(Geom_OffsetSurface)& theSurf,
                                                   StdObjMgt_TransientPersistentMap& theMap);
};

//=======================================================================
// Elementary
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>::PName()
  const;

//=======================================================================
// Plane1
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  GeomPlane,
  Ax3>::PName() const;

template <>
void ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  GeomPlane,
  Ax3>::Write(WriteData& theWriteData) const;

//=======================================================================
// Conical
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_ConicalSurface,
  Cone1>::PName() const;

template <>
void ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_ConicalSurface,
  Cone1>::Write(WriteData& theWriteData) const;

//=======================================================================
// Cylindrical
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_CylindricalSurface,
  Cylinder1>::PName() const;

template <>
void ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_CylindricalSurface,
  Cylinder1>::Write(WriteData& theWriteData) const;

//=======================================================================
// Spherical
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_SphericalSurface,
  Sphere3>::PName() const;

template <>
void ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_SphericalSurface,
  Sphere3>::Write(WriteData& theWriteData) const;

//=======================================================================
// Toroidal
//=======================================================================
template <>
Standard_CString ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_ToroidalSurface,
  gp_Torus>::PName() const;

template <>
void ShapePersistent_Geom::instance<
  ShapePersistent_Geom::subBase_gp<ShapePersistent_Geom::Surface, Ax3>,
  Geom_ToroidalSurface,
  gp_Torus>::Write(WriteData& theWriteData) const;

#endif
