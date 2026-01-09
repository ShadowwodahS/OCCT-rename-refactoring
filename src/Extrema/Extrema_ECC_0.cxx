// Created on: 1991-02-26
// Created by: Isabelle GRIGNON
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

#include <Extrema_ECC.hxx>

#include <Adaptor3d_Curve.hxx>
#include <Extrema_CurveTool.hxx>
#include <Extrema_ExtPC.hxx>
#include <Extrema_POnCurv.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

#define Curve1 Curve5
#define Curve1_hxx <Adaptor3d_Curve.hxx>
#define Tool1 CurveTool4
#define Tool1_hxx <Extrema_CurveTool.hxx>
#define Curve2 Curve5
#define Curve2_hxx <Adaptor3d_Curve.hxx>
#define Tool2 CurveTool4
#define Tool2_hxx <Extrema_CurveTool.hxx>
#define Handle_ArrayOfPnt Handle(PointArray1)
#define ArrayOfPnt PointArray1
#define ArrayOfPnt_hxx <TColgp_HArray1OfPnt.hxx>
#define POnC PointOnCurve1
#define POnC_hxx <Extrema_POnCurv.hxx>
#define Pnt Point3d
#define Pnt_hxx <gp_Pnt.hxx>
#define Vec Vector3d
#define Vec_hxx <gp_Vec.hxx>
#define Extrema_GExtPC Extrema_ExtPC
#define Extrema_GenExtCC CurveCurveExtrema1
#define Extrema_GenExtCC_hxx <Extrema_ECC.hxx>
#include "../Extrema/Extrema_GenExtCC.gxx"
