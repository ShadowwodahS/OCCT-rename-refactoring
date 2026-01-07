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

DESTEP_Parameters::DESTEP_Parameters() {}

//=================================================================================================

void DESTEP_Parameters::InitFromStatic()
{
  ReadBSplineContinuity = (DESTEP_Parameters::ReadMode_BSplineContinuity)ExchangeConfig::IVal(
    "read.iges.bspline.continuity");
  ReadPrecisionMode =
    (DESTEP_Parameters::ReadMode_Precision)ExchangeConfig::IVal("read.precision.mode");
  ReadPrecisionVal = ExchangeConfig::RVal("read.precision.val");
  ReadMaxPrecisionMode =
    (DESTEP_Parameters::ReadMode_MaxPrecision)ExchangeConfig::IVal("read.maxprecision.mode");
  ReadMaxPrecisionVal = ExchangeConfig::RVal("read.maxprecision.val");
  ReadSameParamMode   = ExchangeConfig::IVal("read.stdsameparameter.mode") == 1;
  ReadSurfaceCurveMode =
    (DESTEP_Parameters::ReadMode_SurfaceCurve)ExchangeConfig::IVal("read.surfacecurve.mode");
  EncodeRegAngle = ExchangeConfig::RVal("read.encoderegularity.angle") * 180.0 / M_PI;
  AngleUnit      = (DESTEP_Parameters::AngleUnitMode)ExchangeConfig::IVal("step.angleunit.mode");

  ReadProductMode = ExchangeConfig::IVal("read.step.product.mode") == 1;
  ReadProductContext =
    (DESTEP_Parameters::ReadMode_ProductContext)ExchangeConfig::IVal("read.step.product.context");
  ReadShapeRepr =
    (DESTEP_Parameters::ReadMode_ShapeRepr)ExchangeConfig::IVal("read.step.shape.repr");
  ReadTessellated =
    (DESTEP_Parameters::RWMode_Tessellated)ExchangeConfig::IVal("read.step.tessellated");
  ReadAssemblyLevel =
    (DESTEP_Parameters::ReadMode_AssemblyLevel)ExchangeConfig::IVal("read.step.assembly.level");
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
    (DESTEP_Parameters::WriteMode_PrecisionMode)ExchangeConfig::IVal("write.precision.mode");
  WritePrecisionVal = ExchangeConfig::RVal("write.precision.val");
  WriteAssembly =
    (DESTEP_Parameters::WriteMode_Assembly)ExchangeConfig::IVal("write.step.assembly");
  WriteSchema =
    (DESTEP_Parameters::WriteMode_StepSchema)ExchangeConfig::IVal("write.step.schema");
  WriteTessellated =
    (DESTEP_Parameters::RWMode_Tessellated)ExchangeConfig::IVal("write.step.tessellated");
  WriteProductName    = ExchangeConfig::CVal("write.step.product.name");
  WriteSurfaceCurMode = ExchangeConfig::IVal("write.surfacecurve.mode") == 1;
  WriteUnit           = (UnitsMethods_LengthUnit)ExchangeConfig::IVal("write.step.unit");
  WriteVertexMode =
    (DESTEP_Parameters::WriteMode_VertexMode)ExchangeConfig::IVal("write.step.vertex.mode");
  WriteSubshapeNames = ExchangeConfig::IVal("write.stepcaf.subshapes.name") == 1;
  WriteColor         = ExchangeConfig::IVal("write.color") == 1;
  WriteNonmanifold   = ExchangeConfig::IVal("write.step.nonmanifold") == 1;
  WriteName          = ExchangeConfig::IVal("write.name") == 1;
  WriteLayer         = ExchangeConfig::IVal("write.layer") == 1;
  WriteProps         = ExchangeConfig::IVal("write.props") == 1;
  WriteModelType     = (STEPControl_StepModelType)ExchangeConfig::IVal("write.model.type");
}

//=================================================================================================

void DESTEP_Parameters::Reset()
{
  DESTEP_Parameters aParameters;
  *this = aParameters;
}

//=================================================================================================

ShapeFixParameters DESTEP_Parameters::GetDefaultShapeFixParameters()
{
  return ShapeFixParameters();
}
