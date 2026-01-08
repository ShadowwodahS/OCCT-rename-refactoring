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

#include <STEPControl_Writer.hxx>

#include <APIHeaderSection_MakeHeader.hxx>
#include <DE_ShapeFixParameters.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Interface_Macros.hxx>
#include <STEPControl_ActorWrite.hxx>
#include <STEPControl_Controller.hxx>
#include <DESTEP_Parameters.hxx>
#include <StepData_StepModel.hxx>
#include <StepData_Protocol.hxx>
#include <StepData_StepWriter.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_ShapeProcessor.hxx>
#include <XSControl_WorkSession.hxx>
#include <UnitsMethods.hxx>

//=================================================================================================

StepFileWriter::StepFileWriter()
{
  STEPControl_Controller::Init();
  SetWS(new ExchangeSession);
}

//=================================================================================================

StepFileWriter::StepFileWriter(const Handle(ExchangeSession)& WS,
                                       const Standard_Boolean               scratch)
{
  STEPControl_Controller::Init();
  SetWS(WS, scratch);
}

//=================================================================================================

void StepFileWriter::SetWS(const Handle(ExchangeSession)& WS,
                               const Standard_Boolean               scratch)
{
  thesession = WS;
  thesession->SelectNorm("STEP");
  thesession->InitTransferReader(0);
  Handle(StepData_StepModel) model = Model(scratch);
}

//=================================================================================================

Handle(ExchangeSession) StepFileWriter::WS() const
{
  return thesession;
}

//=================================================================================================

Handle(StepData_StepModel) StepFileWriter::Model(const Standard_Boolean newone)
{
  DeclareAndCast(StepData_StepModel, model, thesession->Model());
  if (newone || model.IsNull())
    model = GetCasted(StepData_StepModel, thesession->NewModel());
  return model;
}

//=================================================================================================

void StepFileWriter::SetTolerance(const Standard_Real Tol)
{
  DeclareAndCast(STEPControl_ActorWrite, act, WS()->NormAdaptor()->ActorWrite());
  if (!act.IsNull())
    act->SetTolerance(Tol);
}

//=================================================================================================

void StepFileWriter::UnsetTolerance()
{
  SetTolerance(-1.);
}

//=================================================================================================

IFSelect_ReturnStatus StepFileWriter::Transfer(const TopoShape&             sh,
                                                   const STEPControl_StepModelType mode,
                                                   const Standard_Boolean          compgraph,
                                                   const Message_ProgressRange&    theProgress)
{
  Handle(StepData_StepModel) aStepModel = Handle(StepData_StepModel)::DownCast(thesession->Model());
  if (!aStepModel.IsNull())
  {
    aStepModel->InternalParameters.InitFromStatic();
  }
  return Transfer(sh, mode, aStepModel->InternalParameters, compgraph, theProgress);
}

IFSelect_ReturnStatus StepFileWriter::Transfer(const TopoShape&             sh,
                                                   const STEPControl_StepModelType mode,
                                                   const DESTEP_Parameters&        theParams,
                                                   const Standard_Boolean          compgraph,
                                                   const Message_ProgressRange&    theProgress)
{
  Standard_Integer mws = -1;
  switch (mode)
  {
    case STEPControl_AsIs:
      mws = 0;
      break;
    case STEPControl_FacetedBrep:
      mws = 1;
      break;
    case STEPControl_ShellBasedSurfaceModel:
      mws = 2;
      break;
    case STEPControl_ManifoldSolidBrep:
      mws = 3;
      break;
    case STEPControl_GeometricCurveSet:
      mws = 4;
      break;
    default:
      break;
  }
  if (mws < 0)
    return IFSelect_RetError; // cas non reconnu
  thesession->TransferWriter()->SetTransferMode(mws);
  if (!Model()->IsInitializedUnit())
  {
    XSAlgo_ShapeProcessor::PrepareForTransfer(); // update unit info
    Model()->SetLocalLengthUnit(UnitsMethods::GetCasCadeLengthUnit());
  }
  Model()->InternalParameters = theParams;
  APIHeaderSection_MakeHeader aHeaderMaker;
  aHeaderMaker.Apply(Model());
  Handle(STEPControl_ActorWrite) ActWrite =
    Handle(STEPControl_ActorWrite)::DownCast(WS()->NormAdaptor()->ActorWrite());
  ActWrite->SetGroupMode(
    Handle(StepData_StepModel)::DownCast(thesession->Model())->InternalParameters.WriteAssembly);
  InitializeMissingParameters();
  return thesession->TransferWriteShape(sh, compgraph, theProgress);
}

//=================================================================================================

IFSelect_ReturnStatus StepFileWriter::Write(const Standard_CString theFileName)
{
  Handle(StepData_StepModel) aModel = Model();
  if (aModel.IsNull())
  {
    return IFSelect_RetFail;
  }
  APIHeaderSection_MakeHeader aHeaderMaker;
  aHeaderMaker.Apply(aModel);
  return thesession->SendAll(theFileName);
}

//=================================================================================================

IFSelect_ReturnStatus StepFileWriter::WriteStream(std::ostream& theOStream)
{
  Handle(StepData_StepModel) aModel = Model();
  if (aModel.IsNull())
  {
    return IFSelect_RetFail;
  }

  Handle(StepData_Protocol) aProtocol = Handle(StepData_Protocol)::DownCast(aModel->Protocol());
  if (aProtocol.IsNull())
  {
    return IFSelect_RetFail;
  }

  StepData_StepWriter aWriter(aModel);
  aWriter.SendModel(aProtocol);
  APIHeaderSection_MakeHeader aHeaderMaker;
  aHeaderMaker.Apply(aModel);
  return aWriter.Print(theOStream) ? IFSelect_RetDone : IFSelect_RetFail;
}

//=================================================================================================

void StepFileWriter::PrintStatsTransfer(const Standard_Integer what,
                                            const Standard_Integer mode) const
{
  thesession->TransferWriter()->PrintStats(what, mode);
}

//=============================================================================

void StepFileWriter::SetShapeFixParameters(
  const XSAlgo_ShapeProcessor::ParameterMap& theParameters)
{
  if (Handle(Transfer_ActorOfFinderProcess) anActor = GetActor())
  {
    anActor->SetShapeFixParameters(theParameters);
  }
}

//=============================================================================

void StepFileWriter::SetShapeFixParameters(XSAlgo_ShapeProcessor::ParameterMap&& theParameters)
{
  if (Handle(Transfer_ActorOfFinderProcess) anActor = GetActor())
  {
    anActor->SetShapeFixParameters(std::move(theParameters));
  }
}

//=============================================================================

void StepFileWriter::SetShapeFixParameters(
  const ShapeFixParameters&               theParameters,
  const XSAlgo_ShapeProcessor::ParameterMap& theAdditionalParameters)
{
  if (Handle(Transfer_ActorOfFinderProcess) anActor = GetActor())
  {
    anActor->SetShapeFixParameters(theParameters, theAdditionalParameters);
  }
}

//=============================================================================

const XSAlgo_ShapeProcessor::ParameterMap& StepFileWriter::GetShapeFixParameters() const
{
  static const XSAlgo_ShapeProcessor::ParameterMap anEmptyMap;
  const Handle(Transfer_ActorOfFinderProcess)      anActor = GetActor();
  return anActor.IsNull() ? anEmptyMap : anActor->GetShapeFixParameters();
}

//=============================================================================

void StepFileWriter::SetShapeProcessFlags(const ShapeProcess1::OperationsFlags& theFlags)
{
  if (Handle(Transfer_ActorOfFinderProcess) anActor = GetActor())
  {
    anActor->SetShapeProcessFlags(theFlags);
  }
}

//=============================================================================

const XSAlgo_ShapeProcessor::ProcessingFlags& StepFileWriter::GetShapeProcessFlags() const
{
  static const XSAlgo_ShapeProcessor::ProcessingFlags anEmptyFlags;
  const Handle(Transfer_ActorOfFinderProcess)         anActor = GetActor();
  return anActor.IsNull() ? anEmptyFlags : anActor->GetShapeProcessFlags();
}

//=============================================================================

Handle(Transfer_ActorOfFinderProcess) StepFileWriter::GetActor() const
{
  Handle(ExchangeSession) aSession = WS();
  if (aSession.IsNull())
  {
    return nullptr;
  }

  Handle(XSControl_Controller) aController = aSession->NormAdaptor();
  if (aController.IsNull())
  {
    return nullptr;
  }

  return aController->ActorWrite();
}

//=============================================================================

void StepFileWriter::InitializeMissingParameters()
{
  if (GetShapeFixParameters().IsEmpty())
  {
    SetShapeFixParameters(DESTEP_Parameters::GetDefaultShapeFixParameters());
  }
  if (GetShapeProcessFlags().second == false)
  {
    ShapeProcess1::OperationsFlags aFlags;
    aFlags.set(ShapeProcess1::Operation::SplitCommonVertex);
    aFlags.set(ShapeProcess1::Operation::DirectFaces);
    SetShapeProcessFlags(aFlags);
  }
}
