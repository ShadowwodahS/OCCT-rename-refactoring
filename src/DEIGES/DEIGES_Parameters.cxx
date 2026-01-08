// Copyright (c) 2023 OPEN CASCADE SAS
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

#include <Interface_Static.hxx>

#include <DEIGES_Parameters.hxx>

//=================================================================================================

void Parameters1::InitFromStatic()
{
  ReadBSplineContinuity =
    (ReadMode_BSplineContinuity)ExchangeConfig::IVal("read.iges.bspline.continuity");
  ReadPrecisionMode    = (ReadMode_Precision)ExchangeConfig::IVal("read.precision.mode");
  ReadPrecisionVal     = ExchangeConfig::RVal("read.precision.val");
  ReadMaxPrecisionMode = (ReadMode_MaxPrecision)ExchangeConfig::IVal("read.maxprecision.mode");
  ReadMaxPrecisionVal  = ExchangeConfig::RVal("read.maxprecision.val");
  ReadSameParamMode    = ExchangeConfig::IVal("read.stdsameparameter.mode") == 1;
  ReadSurfaceCurveMode = (ReadMode_SurfaceCurve)ExchangeConfig::IVal("read.surfacecurve.mode");
  EncodeRegAngle       = ExchangeConfig::RVal("read.encoderegularity.angle");

  ReadApproxd1       = ExchangeConfig::IVal("read.bspline.approxd1.mode") == 1;
  ReadFaultyEntities = ExchangeConfig::IVal("read.fau_lty.entities") == 1;
  ReadOnlyVisible    = ExchangeConfig::IVal("read.onlyvisible") == 1;
  ReadColor          = ExchangeConfig::IVal("read.color") == 1;
  ReadName           = ExchangeConfig::IVal("read.name") == 1;
  ReadLayer          = ExchangeConfig::IVal("read.layer") == 1;

  WriteBRepMode = (WriteMode_BRep)ExchangeConfig::IVal("write.brep.mode");
  WriteConvertSurfaceMode =
    (WriteMode_ConvertSurface)ExchangeConfig::IVal("write.convertsurface.mode");
  WriteHeaderAuthor   = ExchangeConfig::CVal("write.header.author");
  WriteHeaderCompany  = ExchangeConfig::CVal("write.header.company");
  WriteHeaderProduct  = ExchangeConfig::CVal("write.header.product");
  WriteHeaderReciever = ExchangeConfig::CVal("write.header.receiver");
  WritePrecisionMode  = (WriteMode_PrecisionMode)ExchangeConfig::IVal("write.precision.mode");
  WritePrecisionVal   = ExchangeConfig::RVal("write.precision.val");
  WritePlaneMode      = (WriteMode_PlaneMode)ExchangeConfig::IVal("write.plane.mode");
  WriteOffsetMode     = ExchangeConfig::IVal("write.offset") == 1;
  WriteColor          = ExchangeConfig::IVal("write.color") == 1;
  WriteName           = ExchangeConfig::IVal("write.name") == 1;
  WriteLayer          = ExchangeConfig::IVal("write.layer") == 1;
}

//=================================================================================================

void Parameters1::Reset()
{
  *this = Parameters1();
}

//=================================================================================================

ShapeFixParameters Parameters1::GetDefaultShapeFixParameters()
{
  ShapeFixParameters aShapeFixParameters;
  aShapeFixParameters.DetalizationLevel   = TopAbs_EDGE;
  aShapeFixParameters.CreateOpenSolidMode = ShapeFixParameters::FixMode::Fix;
  aShapeFixParameters.FixTailMode         = ShapeFixParameters::FixMode::FixOrNot;
  aShapeFixParameters.MaxTailAngle        = ShapeFixParameters::FixMode::FixOrNot;
  return aShapeFixParameters;
}
