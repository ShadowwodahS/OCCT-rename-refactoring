// Created: 1998-06-03
//
// Copyright (c) 1999-2013 OPEN CASCADE SAS
//
// This file is part of commercial software by OPEN CASCADE SAS,
// furnished in accordance with the terms and conditions of the contract
// and with the inclusion of this copyright notice.
// This file or any part thereof may not be provided or otherwise
// made available to any third party.
//
// No ownership title to the software is transferred hereby.
//
// OPEN CASCADE SAS makes no representation or warranties with respect to the
// performance of this software, and specifically disclaims any responsibility
// for any damages, special or consequential, connected with its use.

#ifndef _GeomConvert_SurfToAnaSurf_HeaderFile
#define _GeomConvert_SurfToAnaSurf_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <Standard_Real.hxx>
#include <Standard_Boolean.hxx>
#include <GeomConvert_ConvType.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <TColgp_HArray1OfXYZ.hxx>
class GeomSurface;
class Geom_SurfaceOfRevolution;
class GeomCircle;

//! Converts a surface to the analytical form with given
//! precision. Conversion is done only the surface is bspline
//! of bezier and this can be approximated by some analytical
//! surface with that precision.
class GeomConvert_SurfToAnaSurf
{
public:
  DEFINE_STANDARD_ALLOC

  Standard_EXPORT GeomConvert_SurfToAnaSurf();

  Standard_EXPORT GeomConvert_SurfToAnaSurf(const Handle(GeomSurface)& S);

  Standard_EXPORT void Init(const Handle(GeomSurface)& S);

  void SetConvType(const GeomConvert_ConvType theConvType = GeomConvert_Simplest)
  {
    myConvType = theConvType;
  }

  void SetTarget(const GeomAbs_SurfaceType theSurfType = GeomAbs_Plane) { myTarget = theSurfType; }

  //! Returns maximal deviation of converted surface from the original
  //! one computed by last call to ConvertToAnalytical
  Standard_Real Gap() const { return myGap; }

  //! Tries to convert the Surface to an Analytic form
  //! Returns the result
  //! In case of failure, returns a Null Handle
  //!
  Standard_EXPORT Handle(GeomSurface) ConvertToAnalytical(const Standard_Real InitialToler);
  Standard_EXPORT Handle(GeomSurface) ConvertToAnalytical(const Standard_Real InitialToler,
                                                           const Standard_Real Umin,
                                                           const Standard_Real Umax,
                                                           const Standard_Real Vmin,
                                                           const Standard_Real Vmax);

  //! Returns true if surfaces is same with the given tolerance
  Standard_EXPORT static Standard_Boolean IsSame(const Handle(GeomSurface)& S1,
                                                 const Handle(GeomSurface)& S2,
                                                 const Standard_Real         tol);

  //! Returns true, if surface is canonical
  Standard_EXPORT static Standard_Boolean IsCanonical(const Handle(GeomSurface)& S);

private:
  //! static method for checking surface of revolution
  //! To avoid two-parts cone-like surface
  static void CheckVTrimForRevSurf(const Handle(Geom_SurfaceOfRevolution)& aRevSurf,
                                   Standard_Real&                          V1,
                                   Standard_Real&                          V2);

  //! static method to try create cylindrical or conical surface
  static Handle(GeomSurface) TryCylinerCone(const Handle(GeomSurface)& theSurf,
                                             const Standard_Boolean      theVCase,
                                             const Handle(GeomCurve3d)&   theUmidiso,
                                             const Handle(GeomCurve3d)&   theVmidiso,
                                             const Standard_Real         theU1,
                                             const Standard_Real         theU2,
                                             const Standard_Real         theV1,
                                             const Standard_Real         theV2,
                                             const Standard_Real         theToler);

  //! static method to try create cylinrical surface using least square method
  static Standard_Boolean GetCylByLS(const Handle(TColgp_HArray1OfXYZ)& thePoints,
                                     const Standard_Real                theTol,
                                     Ax3&                            thePos,
                                     Standard_Real&                     theR,
                                     Standard_Real&                     theGap);

  //! static method to try create cylinrical surface based on its Gauss field
  static Handle(GeomSurface) TryCylinderByGaussField(
    const Handle(GeomSurface)& theSurf,
    const Standard_Real         theU1,
    const Standard_Real         theU2,
    const Standard_Real         theV1,
    const Standard_Real         theV2,
    const Standard_Real         theToler,
    const Standard_Integer      theNbU         = 20,
    const Standard_Integer      theNbV         = 20,
    const Standard_Boolean      theLeastSquare = Standard_False);

  //! static method to try create toroidal surface.
  //! In case <isTryUMajor> = Standard_True try to use V isoline radius as minor radaius.
  static Handle(GeomSurface) TryTorusSphere(const Handle(GeomSurface)& theSurf,
                                             const Handle(GeomCircle)&  circle,
                                             const Handle(GeomCircle)&  otherCircle,
                                             const Standard_Real         Param1,
                                             const Standard_Real         Param2,
                                             const Standard_Real         aParam1ToCrv,
                                             const Standard_Real         aParam2ToCrv,
                                             const Standard_Real         toler,
                                             const Standard_Boolean      isTryUMajor);

  static Standard_Real ComputeGap(const Handle(GeomSurface)& theSurf,
                                  const Standard_Real         theU1,
                                  const Standard_Real         theU2,
                                  const Standard_Real         theV1,
                                  const Standard_Real         theV2,
                                  const Handle(GeomSurface)& theNewSurf,
                                  const Standard_Real         theTol = RealLast());

protected:
private:
  Handle(GeomSurface) mySurf;
  Standard_Real        myGap;
  GeomConvert_ConvType myConvType;
  GeomAbs_SurfaceType  myTarget;
};

#endif // _GeomConvert_SurfToAnaSurf_HeaderFile
