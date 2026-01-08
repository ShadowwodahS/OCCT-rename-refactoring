// Copyright (c) 2022 OPEN CASCADE SAS
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

#include <DEIGES_Provider.hxx>

#include <DEIGES_ConfigurationNode.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <IGESData.hxx>
#include <IGESData_IGESModel.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <UnitsMethods.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XSControl_WorkSession.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DEIGES_Provider, DE_Provider)

//=================================================================================================

DEIGES_Provider::DEIGES_Provider() {}

//=================================================================================================

DEIGES_Provider::DEIGES_Provider(const Handle(DE_ConfigurationNode)& theNode)
    : DE_Provider(theNode)
{
}

//=================================================================================================

void DEIGES_Provider::personizeWS(Handle(ExchangeSession)& theWS)
{
  if (theWS.IsNull())
  {
    Message1::SendWarning() << "Warning: DEIGES_Provider :"
                           << " Null work session, use internal temporary session";
    theWS = new ExchangeSession();
  }
  Handle(IGESControl_Controller) aCntrl =
    Handle(IGESControl_Controller)::DownCast(theWS->NormAdaptor());
  if (aCntrl.IsNull())
  {
    IGESControl_Controller::Init();
    theWS->SelectNorm("IGES");
  }
}

//=================================================================================================

void DEIGES_Provider::initStatic(const Handle(DE_ConfigurationNode)& theNode)
{
  Handle(DEIGES_ConfigurationNode) aNode = Handle(DEIGES_ConfigurationNode)::DownCast(theNode);
  IGESData1::Init();

  // Get previous values
  myOldValues.ReadBSplineContinuity =
    (DEIGES_Parameters::ReadMode_BSplineContinuity)ExchangeConfig::IVal(
      "read.iges.bspline.continuity");
  myOldValues.ReadPrecisionMode =
    (DEIGES_Parameters::ReadMode_Precision)ExchangeConfig::IVal("read.precision.mode");
  myOldValues.ReadPrecisionVal = ExchangeConfig::RVal("read.precision.val");
  myOldValues.ReadMaxPrecisionMode =
    (DEIGES_Parameters::ReadMode_MaxPrecision)ExchangeConfig::IVal("read.maxprecision.mode");
  myOldValues.ReadMaxPrecisionVal = ExchangeConfig::RVal("read.maxprecision.val");
  myOldValues.ReadSameParamMode   = ExchangeConfig::IVal("read.stdsameparameter.mode") == 1;
  myOldValues.ReadSurfaceCurveMode =
    (DEIGES_Parameters::ReadMode_SurfaceCurve)ExchangeConfig::IVal("read.surfacecurve.mode");
  myOldValues.EncodeRegAngle = ExchangeConfig::RVal("read.encoderegularity.angle") * 180.0 / M_PI;

  myOldValues.ReadApproxd1       = ExchangeConfig::IVal("read.iges.bspline.approxd1.mode") == 1;
  myOldValues.ReadFaultyEntities = ExchangeConfig::IVal("read.iges.faulty.entities") == 1;
  myOldValues.ReadOnlyVisible    = ExchangeConfig::IVal("read.iges.onlyvisible") == 1;

  myOldValues.WriteBRepMode =
    (DEIGES_Parameters::WriteMode_BRep)ExchangeConfig::IVal("write.iges.brep.mode");
  myOldValues.WriteConvertSurfaceMode =
    (DEIGES_Parameters::WriteMode_ConvertSurface)ExchangeConfig::IVal(
      "write.convertsurface.mode");
  myOldValues.WriteHeaderAuthor   = ExchangeConfig::CVal("write.iges.header.author");
  myOldValues.WriteHeaderCompany  = ExchangeConfig::CVal("write.iges.header.company");
  myOldValues.WriteHeaderProduct  = ExchangeConfig::CVal("write.iges.header.product");
  myOldValues.WriteHeaderReciever = ExchangeConfig::CVal("write.iges.header.receiver");
  myOldValues.WritePrecisionMode =
    (DEIGES_Parameters::WriteMode_PrecisionMode)ExchangeConfig::IVal("write.precision.mode");
  myOldValues.WritePrecisionVal = ExchangeConfig::RVal("write.precision.val");
  myOldValues.WritePlaneMode =
    (DEIGES_Parameters::WriteMode_PlaneMode)ExchangeConfig::IVal("write.iges.plane.mode");
  myOldValues.WriteOffsetMode = ExchangeConfig::IVal("write.iges.offset.mode") == 1;

  myOldLengthUnit = ExchangeConfig::IVal("xstep.cascade.unit");

  // Set new values
  UnitsMethods1::SetCasCadeLengthUnit(aNode->GlobalParameters.LengthUnit,
                                     UnitsMethods_LengthUnit_Millimeter);
  AsciiString1 aStrUnit(
    UnitsMethods1::DumpLengthUnit(aNode->GlobalParameters.LengthUnit));
  aStrUnit.UpperCase();
  ExchangeConfig::SetCVal("xstep.cascade.unit", aStrUnit.ToCString());
  setStatic(aNode->InternalParameters);
}

//=================================================================================================

void DEIGES_Provider::setStatic(const DEIGES_Parameters& theParameter)
{
  ExchangeConfig::SetIVal("read.iges.bspline.continuity", theParameter.ReadBSplineContinuity);
  ExchangeConfig::SetIVal("read.precision.mode", theParameter.ReadPrecisionMode);
  ExchangeConfig::SetRVal("read.precision.val", theParameter.ReadPrecisionVal);
  ExchangeConfig::SetIVal("read.maxprecision.mode", theParameter.ReadMaxPrecisionMode);
  ExchangeConfig::SetRVal("read.maxprecision.val", theParameter.ReadMaxPrecisionVal);
  ExchangeConfig::SetIVal("read.stdsameparameter.mode", theParameter.ReadSameParamMode);
  ExchangeConfig::SetIVal("read.surfacecurve.mode", theParameter.ReadSurfaceCurveMode);
  ExchangeConfig::SetRVal("read.encoderegularity.angle",
                            theParameter.EncodeRegAngle * M_PI / 180.0);

  ExchangeConfig::SetIVal("read.iges.bspline.approxd1.mode", theParameter.ReadApproxd1);
  ExchangeConfig::SetIVal("read.iges.faulty.entities", theParameter.ReadFaultyEntities);
  ExchangeConfig::SetIVal("read.iges.onlyvisible", theParameter.ReadOnlyVisible);

  ExchangeConfig::SetIVal("write.iges.brep.mode", theParameter.WriteBRepMode);
  ExchangeConfig::SetIVal("write.convertsurface.mode", theParameter.WriteConvertSurfaceMode);
  ExchangeConfig::SetCVal("write.iges.header.author", theParameter.WriteHeaderAuthor.ToCString());
  ExchangeConfig::SetCVal("write.iges.header.company",
                            theParameter.WriteHeaderCompany.ToCString());
  ExchangeConfig::SetCVal("write.iges.header.product",
                            theParameter.WriteHeaderProduct.ToCString());
  ExchangeConfig::SetCVal("write.iges.header.receiver",
                            theParameter.WriteHeaderReciever.ToCString());
  ExchangeConfig::SetIVal("write.precision.mode", theParameter.WritePrecisionMode);
  ExchangeConfig::SetRVal("write.precision.val", theParameter.WritePrecisionVal);
  ExchangeConfig::SetIVal("write.iges.plane.mode", theParameter.WritePlaneMode);
  ExchangeConfig::SetIVal("write.iges.offset.mode", theParameter.WriteOffsetMode);
}

//=================================================================================================

void DEIGES_Provider::resetStatic()
{
  ExchangeConfig::SetIVal("xstep.cascade.unit", myOldLengthUnit);
  UnitsMethods1::SetCasCadeLengthUnit(myOldLengthUnit);
  setStatic(myOldValues);
}

//=================================================================================================

bool DEIGES_Provider::Read(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           Handle(ExchangeSession)&  theWS,
                           const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: theDocument shouldn't be null";
    return false;
  }
  if (!GetNode()->IsKind(STANDARD_TYPE(DEIGES_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEIGES_ConfigurationNode) aNode = Handle(DEIGES_ConfigurationNode)::DownCast(GetNode());
  personizeWS(theWS);
  initStatic(aNode);
  XCAFDoc_DocumentTool::SetLengthUnit(theDocument,
                                      aNode->GlobalParameters.LengthUnit,
                                      UnitsMethods_LengthUnit_Millimeter);
  IGESCAFControl_Reader aReader;
  aReader.SetWS(theWS);

  aReader.SetReadVisible(aNode->InternalParameters.ReadOnlyVisible);

  aReader.SetColorMode(aNode->InternalParameters.ReadColor);
  aReader.SetNameMode(aNode->InternalParameters.ReadName);
  aReader.SetLayerMode(aNode->InternalParameters.ReadLayer);
  aReader.SetShapeFixParameters(aNode->ShapeFixParameters);
  IFSelect_ReturnStatus aReadStat = IFSelect_RetVoid;
  aReadStat                       = aReader.ReadFile(thePath.ToCString());
  if (aReadStat != IFSelect_RetDone)
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: abandon, no model loaded";
    resetStatic();
    return false;
  }

  if (!aReader.Transfer(theDocument, theProgress))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Cannot read any relevant data from the IGES file";
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}

//=================================================================================================

bool DEIGES_Provider::Write(const AsciiString1&  thePath,
                            const Handle(AppDocument)& theDocument,
                            Handle(ExchangeSession)&  theWS,
                            const Message_ProgressRange&    theProgress)
{
  if (!GetNode()->IsKind(STANDARD_TYPE(DEIGES_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEIGES_ConfigurationNode) aNode = Handle(DEIGES_ConfigurationNode)::DownCast(GetNode());
  personizeWS(theWS);
  initStatic(aNode);
  Standard_Integer aFlag = IGESData_BasicEditor::GetFlagByValue(aNode->GlobalParameters.LengthUnit);
  IGESCAFControl_Writer  aWriter(theWS,
                                (aFlag > 0) ? IGESData_BasicEditor::UnitFlagName(aFlag) : "MM");
  IGESData_GlobalSection aGS            = aWriter.Model()->GlobalSection();
  Standard_Real          aScaleFactorMM = 1.;
  Standard_Boolean       aHasUnits =
    XCAFDoc_DocumentTool::GetLengthUnit(theDocument,
                                        aScaleFactorMM,
                                        UnitsMethods_LengthUnit_Millimeter);
  if (aHasUnits)
  {
    aGS.SetCascadeUnit(aScaleFactorMM);
  }
  else
  {
    aGS.SetCascadeUnit(aNode->GlobalParameters.SystemUnit);
    Message1::SendWarning()
      << "Warning in the DEIGES_Provider during writing the file " << thePath
      << "\t: The document has no information on Units2. Using global parameter as initial Unit.";
  }
  if (aFlag == 0)
  {
    aGS.SetScale(aNode->GlobalParameters.LengthUnit);
  }
  aWriter.Model()->SetGlobalSection(aGS);
  aWriter.SetColorMode(aNode->InternalParameters.WriteColor);
  aWriter.SetNameMode(aNode->InternalParameters.WriteName);
  aWriter.SetLayerMode(aNode->InternalParameters.WriteLayer);
  aWriter.SetShapeFixParameters(aNode->ShapeFixParameters);
  if (!aWriter.Transfer(theDocument, theProgress))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: The document cannot be translated or gives no result";
    resetStatic();
    return false;
  }
  if (!aWriter.Write(thePath.ToCString()))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Write failed";
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}

//=================================================================================================

bool DEIGES_Provider::Read(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           const Message_ProgressRange&    theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Read(thePath, theDocument, aWS, theProgress);
}

//=================================================================================================

bool DEIGES_Provider::Write(const AsciiString1&  thePath,
                            const Handle(AppDocument)& theDocument,
                            const Message_ProgressRange&    theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Write(thePath, theDocument, aWS, theProgress);
}

//=================================================================================================

bool DEIGES_Provider::Read(const AsciiString1& thePath,
                           TopoShape&                  theShape,
                           Handle(ExchangeSession)& theWS,
                           const Message_ProgressRange&   theProgress)
{
  (void)theProgress;
  if (!GetNode()->IsKind(STANDARD_TYPE(DEIGES_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEIGES_ConfigurationNode) aNode = Handle(DEIGES_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);
  personizeWS(theWS);
  IgesFileReader aReader;
  aReader.SetWS(theWS);
  aReader.SetReadVisible(aNode->InternalParameters.ReadOnlyVisible);
  aReader.SetShapeFixParameters(aNode->ShapeFixParameters);

  IFSelect_ReturnStatus aReadStat = IFSelect_RetVoid;
  aReadStat                       = aReader.ReadFile(thePath.ToCString());
  if (aReadStat != IFSelect_RetDone)
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Could not read file, no model loaded";
    resetStatic();
    return false;
  }
  if (aReader.TransferRoots() <= 0)
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Cannot read any relevant data from the IGES file";
    resetStatic();
    return false;
  }
  theShape = aReader.OneShape();
  resetStatic();
  return true;
}

//=================================================================================================

bool DEIGES_Provider::Write(const AsciiString1& thePath,
                            const TopoShape&            theShape,
                            Handle(ExchangeSession)& theWS,
                            const Message_ProgressRange&   theProgress)
{
  (void)theWS;
  (void)theProgress;
  if (!GetNode()->IsKind(STANDARD_TYPE(DEIGES_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEIGES_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEIGES_ConfigurationNode) aNode = Handle(DEIGES_ConfigurationNode)::DownCast(GetNode());
  initStatic(aNode);
  Standard_Integer aFlag = IGESData_BasicEditor::GetFlagByValue(aNode->GlobalParameters.LengthUnit);
  IgesFileWriter aWriter((aFlag > 0) ? IGESData_BasicEditor::UnitFlagName(aFlag) : "MM",
                             aNode->InternalParameters.WriteBRepMode);
  IGESData_GlobalSection aGS = aWriter.Model()->GlobalSection();
  aGS.SetCascadeUnit(aNode->GlobalParameters.SystemUnit);
  if (!aFlag)
  {
    aGS.SetScale(aNode->GlobalParameters.LengthUnit);
  }
  aWriter.Model()->SetGlobalSection(aGS);
  aWriter.SetShapeFixParameters(aNode->ShapeFixParameters);
  Standard_Boolean aIsOk = aWriter.AddShape(theShape);
  if (!aIsOk)
  {
    Message1::SendFail() << "DEIGES_Provider: Shape not written";
    resetStatic();
    return false;
  }

  if (!(aWriter.Write(thePath.ToCString())))
  {
    Message1::SendFail() << "DEIGES_Provider: Error on writing file " << thePath;
    resetStatic();
    return false;
  }
  resetStatic();
  return true;
}

//=================================================================================================

bool DEIGES_Provider::Read(const AsciiString1& thePath,
                           TopoShape&                  theShape,
                           const Message_ProgressRange&   theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Read(thePath, theShape, aWS, theProgress);
}

//=================================================================================================

bool DEIGES_Provider::Write(const AsciiString1& thePath,
                            const TopoShape&            theShape,
                            const Message_ProgressRange&   theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Write(thePath, theShape, aWS, theProgress);
}

//=================================================================================================

AsciiString1 DEIGES_Provider::GetFormat() const
{
  return AsciiString1("IGES");
}

//=================================================================================================

AsciiString1 DEIGES_Provider::GetVendor() const
{
  return AsciiString1("OCC");
}
