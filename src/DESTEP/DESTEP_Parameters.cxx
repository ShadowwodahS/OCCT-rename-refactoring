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

#include <DESTEP_Parameters.hxx>

//=================================================================================================

Parameters2::Parameters2() {}

//=================================================================================================

void Parameters2::InitFromStatic()
{
  ReadBSplineContinuity = (Parameters2::ReadMode_BSplineContinuity)ExchangeConfig::IVal(
    "read.iges.bspline.continuity");
  ReadPrecisionMode =
    (Parameters2::ReadMode_Precision)ExchangeConfig::IVal("read.precision.mode");
  ReadPrecisionVal = ExchangeConfig::RVal("read.precision.val");
  ReadMaxPrecisionMode =
    (Parameters2::ReadMode_MaxPrecision)ExchangeConfig::IVal("read.maxprecision.mode");
  ReadMaxPrecisionVal = ExchangeConfig::RVal("read.maxprecision.val");
  ReadSameParamMode   = ExchangeConfig::IVal("read.stdsameparameter.mode") == 1;
  ReadSurfaceCurveMode =
    (Parameters2::ReadMode_SurfaceCurve)ExchangeConfig::IVal("read.surfacecurve.mode");
  EncodeRegAngle = ExchangeConfig::RVal("read.encoderegularity.angle") * 180.0 / M_PI;
  AngleUnit      = (Parameters2::AngleUnitMode)ExchangeConfig::IVal("step.angleunit.mode");

  ReadProductMode = ExchangeConfig::IVal("read.step.product.mode") == 1;
  ReadProductContext =
    (Parameters2::ReadMode_ProductContext)ExchangeConfig::IVal("read.step.product.context");
  ReadShapeRepr =
    (Parameters2::ReadMode_ShapeRepr)ExchangeConfig::IVal("read.step.shape.repr");
  ReadTessellated =
    (Parameters2::RWMode_Tessellated)ExchangeConfig::IVal("read.step.tessellated");
  ReadAssemblyLevel =
    (Parameters2::ReadMode_AssemblyLevel)ExchangeConfig::IVal("read.step.assembly.level");
  ReadRelationship       = ExchangeConfig::IVal("read.step.shape.relationship") == 1;
  ReadShapeAspect        = ExchangeConfig::IVal("read.step.shape.aspect") == 1;
  ReadConstrRelation     = ExchangeConfig::IVal("read.step.constructivegeom.relationship") == 1;
  ReadSubshapeNames      = ExchangeConfig::IVal("read.stepcaf.subshapes.name") == 1;
  ReadCodePage           = (Resource_FormatType)ExchangeConfig::IVal("read.step.codepage");
  ReadNonmanifold        = ExchangeConfig::IVal("read.step.nonmanifold") == 1;
  ReadIdeas              = ExchangeConfig::IVal("read.step.ideas") == 1;
  ReadAllShapes          = ExchangeConfig::IVal("read.step.all.shapes") == 1;
  ReadRootTransformation = ExchangeConfig::IVal("read.step.root.transformation") == 1;

  WritePrecisionMode =
    (Parameters2::WriteMode_PrecisionMode)ExchangeConfig::IVal("write.precision.mode");
  WritePrecisionVal = ExchangeConfig::RVal("write.precision.val");
  WriteAssembly =
    (Parameters2::WriteMode_Assembly)ExchangeConfig::IVal("write.step.assembly");
  WriteSchema =
    (Parameters2::WriteMode_StepSchema)ExchangeConfig::IVal("write.step.schema");
  WriteTessellated =
    (Parameters2::RWMode_Tessellated)ExchangeConfig::IVal("write.step.tessellated");
  WriteProductName    = ExchangeConfig::CVal("write.step.product.name");
  WriteSurfaceCurMode = ExchangeConfig::IVal("write.surfacecurve.mode") == 1;
  WriteUnit           = (UnitsMethods_LengthUnit)ExchangeConfig::IVal("write.step.unit");
  WriteVertexMode =
    (Parameters2::WriteMode_VertexMode)ExchangeConfig::IVal("write.step.vertex.mode");
  WriteSubshapeNames = ExchangeConfig::IVal("write.stepcaf.subshapes.name") == 1;
  WriteColor         = ExchangeConfig::IVal("write.color") == 1;
  WriteNonmanifold   = ExchangeConfig::IVal("write.step.nonmanifold") == 1;
  WriteName          = ExchangeConfig::IVal("write.name") == 1;
  WriteLayer         = ExchangeConfig::IVal("write.layer") == 1;
  WriteProps         = ExchangeConfig::IVal("write.props") == 1;
  WriteModelType     = (STEPControl_StepModelType)ExchangeConfig::IVal("write.model.type");
}

//=================================================================================================

void Parameters2::Reset()
{
  Parameters2 aParameters;
  *this = aParameters;
}

//=================================================================================================

ShapeFixParameters Parameters2::GetDefaultShapeFixParameters()
{
  return ShapeFixParameters();
}
