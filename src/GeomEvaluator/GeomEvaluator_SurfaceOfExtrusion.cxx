// Created on: 2015-09-21
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

#include <GeomEvaluator_SurfaceOfExtrusion.hxx>

#include <GeomAdaptor_Curve.hxx>

IMPLEMENT_STANDARD_RTTIEXT(GeomEvaluator_SurfaceOfExtrusion, GeomEvaluator_Surface)

GeomEvaluator_SurfaceOfExtrusion::GeomEvaluator_SurfaceOfExtrusion(
  const Handle(Geom_Curve)& theBase,
  const Dir3d&             theExtrusionDir)
    : GeomEvaluator_Surface(),
      myBaseCurve(theBase),
      myDirection(theExtrusionDir)
{
}

GeomEvaluator_SurfaceOfExtrusion::GeomEvaluator_SurfaceOfExtrusion(
  const Handle(Adaptor3d_Curve)& theBase,
  const Dir3d&                  theExtrusionDir)
    : GeomEvaluator_Surface(),
      myBaseAdaptor(theBase),
      myDirection(theExtrusionDir)
{
}

void GeomEvaluator_SurfaceOfExtrusion::D0(const Standard_Real theU,
                                          const Standard_Real theV,
                                          Point3d&             theValue) const
{
  if (!myBaseAdaptor.IsNull())
    myBaseAdaptor->D0(theU, theValue);
  else
    myBaseCurve->D0(theU, theValue);

  Shift(theV, theValue);
}

void GeomEvaluator_SurfaceOfExtrusion::D1(const Standard_Real theU,
                                          const Standard_Real theV,
                                          Point3d&             theValue,
                                          Vector3d&             theD1U,
                                          Vector3d&             theD1V) const
{
  if (!myBaseAdaptor.IsNull())
    myBaseAdaptor->D1(theU, theValue, theD1U);
  else
    myBaseCurve->D1(theU, theValue, theD1U);

  theD1V = myDirection;
  Shift(theV, theValue);
}

void GeomEvaluator_SurfaceOfExtrusion::D2(const Standard_Real theU,
                                          const Standard_Real theV,
                                          Point3d&             theValue,
                                          Vector3d&             theD1U,
                                          Vector3d&             theD1V,
                                          Vector3d&             theD2U,
                                          Vector3d&             theD2V,
                                          Vector3d&             theD2UV) const
{
  if (!myBaseAdaptor.IsNull())
    myBaseAdaptor->D2(theU, theValue, theD1U, theD2U);
  else
    myBaseCurve->D2(theU, theValue, theD1U, theD2U);

  theD1V = myDirection;
  theD2V.SetCoord(0.0, 0.0, 0.0);
  theD2UV.SetCoord(0.0, 0.0, 0.0);

  Shift(theV, theValue);
}

void GeomEvaluator_SurfaceOfExtrusion::D3(const Standard_Real theU,
                                          const Standard_Real theV,
                                          Point3d&             theValue,
                                          Vector3d&             theD1U,
                                          Vector3d&             theD1V,
                                          Vector3d&             theD2U,
                                          Vector3d&             theD2V,
                                          Vector3d&             theD2UV,
                                          Vector3d&             theD3U,
                                          Vector3d&             theD3V,
                                          Vector3d&             theD3UUV,
                                          Vector3d&             theD3UVV) const
{
  if (!myBaseAdaptor.IsNull())
    myBaseAdaptor->D3(theU, theValue, theD1U, theD2U, theD3U);
  else
    myBaseCurve->D3(theU, theValue, theD1U, theD2U, theD3U);

  theD1V = myDirection;
  theD2V.SetCoord(0.0, 0.0, 0.0);
  theD2UV.SetCoord(0.0, 0.0, 0.0);
  theD3V.SetCoord(0.0, 0.0, 0.0);
  theD3UUV.SetCoord(0.0, 0.0, 0.0);
  theD3UVV.SetCoord(0.0, 0.0, 0.0);

  Shift(theV, theValue);
}

Vector3d GeomEvaluator_SurfaceOfExtrusion::DN(const Standard_Real theU,
                                            const Standard_Real,
                                            const Standard_Integer theDerU,
                                            const Standard_Integer theDerV) const
{
  Standard_RangeError_Raise_if(theDerU < 0, "GeomEvaluator_SurfaceOfExtrusion::DN(): theDerU < 0");
  Standard_RangeError_Raise_if(theDerV < 0, "GeomEvaluator_SurfaceOfExtrusion::DN(): theDerV < 0");
  Standard_RangeError_Raise_if(theDerU + theDerV < 1,
                               "GeomEvaluator_SurfaceOfExtrusion::DN(): theDerU + theDerV < 1");

  Vector3d aResult(0.0, 0.0, 0.0);
  if (theDerV == 0)
  {
    if (!myBaseAdaptor.IsNull())
      aResult = myBaseAdaptor->DN(theU, theDerU);
    else
      aResult = myBaseCurve->DN(theU, theDerU);
  }
  else if (theDerU == 0 && theDerV == 1)
    aResult = Vector3d(myDirection);
  return aResult;
}

Handle(GeomEvaluator_Surface) GeomEvaluator_SurfaceOfExtrusion::ShallowCopy() const
{
  Handle(GeomEvaluator_SurfaceOfExtrusion) aCopy;
  if (!myBaseAdaptor.IsNull())
  {
    aCopy = new GeomEvaluator_SurfaceOfExtrusion(myBaseAdaptor->ShallowCopy(), myDirection);
  }
  else
  {
    aCopy = new GeomEvaluator_SurfaceOfExtrusion(myBaseCurve, myDirection);
  }

  return aCopy;
}
