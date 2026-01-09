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

#include <Extrema_CCLocFOfLocECC.hxx>

#include <Standard_OutOfRange.hxx>
#include <Adaptor3d_Curve.hxx>
#include <Extrema_CurveTool.hxx>
#include <Extrema_POnCurv.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <Extrema_SequenceOfPOnCurv.hxx>
#include <math_Matrix.hxx>

#define Curve1 Curve5
#define Curve1_hxx <Adaptor3d_Curve.hxx>
#define Tool1 CurveTool4
#define Tool1_hxx <Extrema_CurveTool.hxx>
#define Curve2 Curve5
#define Curve2_hxx <Adaptor3d_Curve.hxx>
#define Tool2 CurveTool4
#define Tool2_hxx <Extrema_CurveTool.hxx>
#define POnC PointOnCurve1
#define POnC_hxx <Extrema_POnCurv.hxx>
#define Pnt Point3d
#define Pnt_hxx <gp_Pnt.hxx>
#define Vec Vector3d
#define Vec_hxx <gp_Vec.hxx>
#define Extrema_SeqPOnC Extrema_SequenceOfPOnCurv
#define Extrema_SeqPOnC_hxx <Extrema_SequenceOfPOnCurv.hxx>
#define Extrema_FuncExtCC Extrema_CCLocFOfLocECC
#define Extrema_FuncExtCC_hxx <Extrema_CCLocFOfLocECC.hxx>
#include "../Extrema/Extrema_FuncExtCC.gxx"
