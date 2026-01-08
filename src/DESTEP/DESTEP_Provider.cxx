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

#include <DESTEP_Provider.hxx>

#include <DESTEP_ConfigurationNode.hxx>
#include <DESTEP_Parameters.hxx>
#include <Interface_Static.hxx>
#include <Message.hxx>
#include <STEPCAFControl_Controller.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <StepData_StepModel.hxx>
#include <UnitsMethods.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XSControl_WorkSession.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DESTEP_Provider, DE_Provider)

//=================================================================================================

DESTEP_Provider::DESTEP_Provider() {}

//=================================================================================================

DESTEP_Provider::DESTEP_Provider(const Handle(ConfigurationNode)& theNode)
    : DE_Provider(theNode)
{
}

//=================================================================================================

bool DESTEP_Provider::Read(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           Handle(ExchangeSession)&  theWS,
                           const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: theDocument shouldn't be null";
    return false;
  }
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DESTEP_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DESTEP_ConfigurationNode) aNode = Handle(DESTEP_ConfigurationNode)::DownCast(GetNode());
  personizeWS(theWS);
  XCAFDoc_DocumentTool::SetLengthUnit(theDocument,
                                      aNode->GlobalParameters.LengthUnit,
                                      UnitsMethods_LengthUnit_Millimeter);
  STEPCAFControl_Reader aReader;
  aReader.Init(theWS);
  aReader.SetColorMode(aNode->InternalParameters.ReadColor);
  aReader.SetNameMode(aNode->InternalParameters.ReadName);
  aReader.SetLayerMode(aNode->InternalParameters.ReadLayer);
  aReader.SetPropsMode(aNode->InternalParameters.ReadProps);
  aReader.SetMetaMode(aNode->InternalParameters.ReadMetadata);
  aReader.SetProductMetaMode(aNode->InternalParameters.ReadProductMetadata);
  aReader.SetShapeFixParameters(aNode->ShapeFixParameters);
  IFSelect_ReturnStatus aReadStat = IFSelect_RetVoid;
  Parameters2     aParams   = aNode->InternalParameters;
  aReadStat                       = aReader.ReadFile(thePath.ToCString(), aParams);
  if (aReadStat != IFSelect_RetDone)
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: abandon";
    return false;
  }

  if (!aReader.Transfer(theDocument, theProgress))
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: Cannot read any relevant data from the STEP file";
    return false;
  }
  return true;
}

//=================================================================================================

bool DESTEP_Provider::Write(const AsciiString1&  thePath,
                            const Handle(AppDocument)& theDocument,
                            Handle(ExchangeSession)&  theWS,
                            const Message_ProgressRange&    theProgress)
{
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DESTEP_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during writing the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DESTEP_ConfigurationNode) aNode = Handle(DESTEP_ConfigurationNode)::DownCast(GetNode());
  personizeWS(theWS);
  STEPCAFControl_Writer aWriter;
  aWriter.Init(theWS);
  Handle(StepData_StepModel) aModel =
    Handle(StepData_StepModel)::DownCast(aWriter.Writer().WS()->Model());
  STEPControl_StepModelType aMode =
    static_cast<STEPControl_StepModelType>(aNode->InternalParameters.WriteModelType);
  aWriter.SetColorMode(aNode->InternalParameters.WriteColor);
  aWriter.SetNameMode(aNode->InternalParameters.WriteName);
  aWriter.SetLayerMode(aNode->InternalParameters.WriteLayer);
  aWriter.SetPropsMode(aNode->InternalParameters.WriteProps);
  aWriter.SetShapeFixParameters(aNode->ShapeFixParameters);
  Parameters2 aParams        = aNode->InternalParameters;
  Standard_Real     aScaleFactorMM = 1.;
  if (XCAFDoc_DocumentTool::GetLengthUnit(theDocument,
                                          aScaleFactorMM,
                                          UnitsMethods_LengthUnit_Millimeter))
  {
    aModel->SetLocalLengthUnit(aScaleFactorMM);
  }
  else
  {
    aModel->SetLocalLengthUnit(aNode->GlobalParameters.SystemUnit);
    Message1::SendWarning()
      << "Warning in the DESTEP_Provider during writing the file " << thePath
      << "\t: The document has no information on Units2. Using global parameter as initial Unit.";
  }
  UnitsMethods_LengthUnit aTargetUnit =
    UnitsMethods1::GetLengthUnitByFactorValue(aNode->GlobalParameters.LengthUnit,
                                             UnitsMethods_LengthUnit_Millimeter);
  aParams.WriteUnit = aTargetUnit;
  aModel->SetWriteLengthUnit(aNode->GlobalParameters.LengthUnit);
  DataLabel aLabel;
  if (!aWriter.Transfer(theDocument, aParams, aMode, 0, theProgress))
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during writing the file " << thePath
                        << "\t: The document cannot be translated or gives no result";
    return false;
  }
  IFSelect_ReturnStatus aStatus = aWriter.Write(thePath.ToCString());
  switch (aStatus)
  {
    case IFSelect_RetVoid: {
      Message1::SendFail() << "Error in the DESTEP_Provider during writing the file " << thePath
                          << "\t: No file written";
      return false;
      ;
    }
    case IFSelect_RetDone: {
      break;
    }
    default: {
      Message1::SendFail() << "Error in the DESTEP_Provider during writing the file " << thePath
                          << "\t: Error on writing file";
      return false;
    }
  }
  return true;
}

//=================================================================================================

bool DESTEP_Provider::Read(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           const Message_ProgressRange&    theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Read(thePath, theDocument, aWS, theProgress);
}

//=================================================================================================

bool DESTEP_Provider::Write(const AsciiString1&  thePath,
                            const Handle(AppDocument)& theDocument,
                            const Message_ProgressRange&    theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Write(thePath, theDocument, aWS, theProgress);
}

//=================================================================================================

bool DESTEP_Provider::Read(const AsciiString1& thePath,
                           TopoShape&                  theShape,
                           Handle(ExchangeSession)& theWS,
                           const Message_ProgressRange&   theProgress)
{
  (void)theProgress;
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DESTEP_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DESTEP_ConfigurationNode) aNode = Handle(DESTEP_ConfigurationNode)::DownCast(GetNode());
  personizeWS(theWS);
  StepFileReader aReader;
  aReader.SetWS(theWS);
  aReader.SetShapeFixParameters(aNode->ShapeFixParameters);
  IFSelect_ReturnStatus aReadstat   = IFSelect_RetVoid;
  Parameters2     aParams     = aNode->InternalParameters;
  aReadstat                         = aReader.ReadFile(thePath.ToCString(), aParams);
  Handle(StepData_StepModel) aModel = aReader.StepModel();
  if (aReadstat != IFSelect_RetDone)
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: abandon, no model loaded";
    return false;
  }
  aModel->SetLocalLengthUnit(aNode->GlobalParameters.LengthUnit);
  if (aReader.TransferRoots() <= 0)
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t:Cannot read any relevant data from the STEP file";
    return false;
  }
  theShape = aReader.OneShape();
  return true;
}

//=================================================================================================

bool DESTEP_Provider::Write(const AsciiString1& thePath,
                            const TopoShape&            theShape,
                            Handle(ExchangeSession)& theWS,
                            const Message_ProgressRange&   theProgress)
{
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DESTEP_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DESTEP_ConfigurationNode) aNode = Handle(DESTEP_ConfigurationNode)::DownCast(GetNode());

  personizeWS(theWS);
  StepFileWriter aWriter;
  aWriter.SetWS(theWS);
  IFSelect_ReturnStatus      aWritestat = IFSelect_RetVoid;
  Handle(StepData_StepModel) aModel     = aWriter.Model();
  ;
  Parameters2 aParams = aNode->InternalParameters;
  aModel->SetLocalLengthUnit(aNode->GlobalParameters.SystemUnit);
  UnitsMethods_LengthUnit aTargetUnit =
    UnitsMethods1::GetLengthUnitByFactorValue(aNode->GlobalParameters.LengthUnit,
                                             UnitsMethods_LengthUnit_Millimeter);
  aParams.WriteUnit = aTargetUnit;
  if (aTargetUnit == UnitsMethods_LengthUnit_Undefined)
  {
    aModel->SetWriteLengthUnit(1.0);
    Message1::SendWarning()
      << "Custom units are not supported by STEP format, but LengthUnit global parameter doesn't "
         "fit any predefined unit. Units2 will be scaled to Millimeters";
  }
  else
  {
    aModel->SetWriteLengthUnit(aNode->GlobalParameters.LengthUnit);
  }
  aWriter.SetShapeFixParameters(aNode->ShapeFixParameters);
  aWritestat = aWriter.Transfer(theShape,
                                aNode->InternalParameters.WriteModelType,
                                aParams,
                                true,
                                theProgress);
  if (aWritestat != IFSelect_RetDone)
  {
    Message1::SendFail() << "Error in the DESTEP_Provider during reading the file " << thePath
                        << "\t: abandon, no model loaded";
    return false;
  }
  if (aWriter.Write(thePath.ToCString()) != IFSelect_RetDone)
  {
    Message1::SendFail() << "DESTEP_Provider: Error on writing file";
    return false;
  }
  return true;
}

//=================================================================================================

bool DESTEP_Provider::Read(const AsciiString1& thePath,
                           TopoShape&                  theShape,
                           const Message_ProgressRange&   theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Read(thePath, theShape, aWS, theProgress);
}

//=================================================================================================

bool DESTEP_Provider::Write(const AsciiString1& thePath,
                            const TopoShape&            theShape,
                            const Message_ProgressRange&   theProgress)
{
  Handle(ExchangeSession) aWS = new ExchangeSession();
  return Write(thePath, theShape, aWS, theProgress);
}

//=================================================================================================

AsciiString1 DESTEP_Provider::GetFormat() const
{
  return AsciiString1("STEP");
}

//=================================================================================================

AsciiString1 DESTEP_Provider::GetVendor() const
{
  return AsciiString1("OCC");
}

//=================================================================================================

void DESTEP_Provider::personizeWS(Handle(ExchangeSession)& theWS)
{
  if (theWS.IsNull())
  {
    Message1::SendWarning() << "Warning: DESTEP_Provider :"
                           << " Null work session, use internal temporary session";
    theWS = new ExchangeSession();
  }
  Handle(STEPCAFControl_Controller) aCntrl =
    Handle(STEPCAFControl_Controller)::DownCast(theWS->NormAdaptor());
  if (aCntrl.IsNull())
  {
    STEPCAFControl_Controller::Init();
    theWS->SelectNorm("STEP");
  }
}
