// Created on: 1993-10-29
// Created by: Christophe MARION
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

#ifndef _HLRAlgo_PolyData_HeaderFile
#define _HLRAlgo_PolyData_HeaderFile

#include <HLRAlgo_BiPoint.hxx>
#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <Standard_Integer.hxx>
#include <TColgp_HArray1OfXYZ.hxx>
#include <HLRAlgo_HArray1OfTData.hxx>
#include <HLRAlgo_HArray1OfPHDat.hxx>
#include <Standard_Transient.hxx>
#include <Standard_Boolean.hxx>

class EdgeStatus;

class PolyData1;
DEFINE_STANDARD_HANDLE(PolyData1, RefObject)

//! Data structure of a set of Triangles.
class PolyData1 : public RefObject
{

public:
  struct FaceIndices1
  {
    //! The default constructor.
    FaceIndices1()
        : Index(0),
          Min(0),
          Max(0)
    {
    }

    Standard_Integer Index, Min, Max;
  };

  struct Triangle1
  {
    Coords2d         V1, V2, V3;
    Standard_Real Param, TolParam, TolAng, Tolerance;
  };

  struct Box1
  {
    Standard_Real XMin, YMin, ZMin, XMax, YMax, ZMax;

    //! The default constructor.
    Box1()
        : XMin(0.0),
          YMin(0.0),
          ZMin(0.0),
          XMax(0.0),
          YMax(0.0),
          ZMax(0.0)
    {
    }

    //! The initializing constructor.
    Box1(const Standard_Real& theXMin,
        const Standard_Real& theYMin,
        const Standard_Real& theZMin,
        const Standard_Real& theXMax,
        const Standard_Real& theYMax,
        const Standard_Real& theZMax)
        : XMin(theXMin),
          YMin(theYMin),
          ZMin(theZMin),
          XMax(theXMax),
          YMax(theYMax),
          ZMax(theZMax)
    {
    }
  };

  Standard_EXPORT PolyData1();

  Standard_EXPORT void HNodes(const Handle(XYZArray)& HNodes);

  Standard_EXPORT void HTData(const Handle(HandleTDataArray)& HTData);

  Standard_EXPORT void HPHDat(const Handle(HandlePHDatArray)& HPHDat);

  void FaceIndex(const Standard_Integer I);

  Standard_Integer FaceIndex() const;

  TColgp_Array1OfXYZ& Nodes() const;

  HLRAlgo_Array1OfTData& TData() const;

  HLRAlgo_Array1OfPHDat& PHDat() const;

  Standard_EXPORT void UpdateGlobalMinMax(Box1& theBox);

  Standard_Boolean Hiding() const;

  //! process hiding between <Pt1> and <Pt2>.
  Standard_EXPORT void HideByPolyData(const BiPoint::PointsT1& thePoints,
                                      Triangle1&                       theTriangle,
                                      BiPoint::IndicesT1&      theIndices,
                                      const Standard_Boolean          HidingShell,
                                      EdgeStatus&             status);

  FaceIndices1& Indices() { return myFaceIndices; }

  DEFINE_STANDARD_RTTIEXT(PolyData1, RefObject)

private:
  //! evident.
  void hideByOneTriangle(const BiPoint::PointsT1& thePoints,
                         Triangle1&                       theTriangle,
                         const Standard_Boolean          Crossing,
                         const Standard_Boolean          HideBefore,
                         const Standard_Integer          TrFlags,
                         EdgeStatus&             status);

  FaceIndices1                    myFaceIndices;
  Handle(XYZArray)    myHNodes;
  Handle(HandleTDataArray) myHTData;
  Handle(HandlePHDatArray) myHPHDat;
};

#include <HLRAlgo_PolyData.lxx>

#endif // _HLRAlgo_PolyData_HeaderFile
