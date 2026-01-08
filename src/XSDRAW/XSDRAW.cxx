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

#include <DBRep.hxx>
#include <Draw_Appli.hxx>
#include <Draw_Printer.hxx>
#include <Draw_PluginMacro.hxx>
#include <IFSelect_Functions.hxx>
#include <IFSelect_SessionPilot.hxx>
#include <Interface_InterfaceModel.hxx>
#include <Interface_Macros.hxx>
#include <Interface_Protocol.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>
#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>
#include <TColStd_HSequenceOfAsciiString.hxx>
#include <TopoDS_Shape.hxx>
#include <Transfer_FinderProcess.hxx>
#include <Transfer_TransientProcess.hxx>
#include <TransferBRep.hxx>
#include <XSAlgo.hxx>
#include <XSControl.hxx>
#include <XSControl_Controller.hxx>
#include <XSControl_FuncShape.hxx>
#include <XSControl_Functions.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_TransferWriter.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAW.hxx>
#include <XSDRAW_Vars.hxx>
#include <UnitsMethods.hxx>
#include <Interface_Static.hxx>
#include <XCAFDoc_DocumentTool.hxx>

#include <iostream>
#include <string>

namespace
{
static int deja = 0, dejald = 0;

static NCollection_DataMap<AsciiString1, Standard_Integer> theolds;
static Handle(TColStd_HSequenceOfAsciiString)                         thenews;

static Handle(IFSelect_SessionPilot) thepilot; // detient Session, Model

//=================================================================================================

static void collectActiveWorkSessions(const Handle(ExchangeSession)& theWS,
                                      const AsciiString1&       theName,
                                      XSControl_WorkSessionMap&            theMap,
                                      const Standard_Boolean               theIsFirst)
{
  if (theIsFirst)
  {
    theMap.Clear();
  }
  if (theWS.IsNull())
  {
    return;
  }
  if (theMap.IsBound(theName))
  {
    return;
  }
  theMap.Bind(theName, theWS);
  for (XSControl_WorkSessionMap::Iterator anIter(theWS->Context()); anIter.More(); anIter.Next())
  {
    Handle(ExchangeSession) aWS = Handle(ExchangeSession)::DownCast(anIter.Value());
    collectActiveWorkSessions(aWS, anIter.Key1(), theMap, Standard_False);
  }
}
} // namespace

static Standard_Integer XSTEPDRAWRUN(DrawInterpreter& di, Standard_Integer argc, const char** argv)
{
  AsciiString1 mess;
  for (Standard_Integer i = 0; i < argc; i++)
  {
    mess.AssignCat(argv[i]);
    mess.AssignCat(" ");
  }

  const Handle(Message_Messenger)& aMsgMgr = Message1::DefaultMessenger();
  Message_SequenceOfPrinters       aPrinters;
  aPrinters.Append(aMsgMgr->ChangePrinters());
  aMsgMgr->AddPrinter(new Draw_Printer(di));

  IFSelect_ReturnStatus stat = thepilot->Execute(mess.ToCString());

  aMsgMgr->RemovePrinters(STANDARD_TYPE(Draw_Printer));
  aMsgMgr->ChangePrinters().Append(aPrinters);

  if (stat == IFSelect_RetError || stat == IFSelect_RetFail)
    return 1;
  else
    return 0;
}

void XSDRAW1::ChangeCommand(const Standard_CString oldname, const Standard_CString newname)
{
  Standard_Integer num = 0;
  if (newname[0] != '\0')
  {
    if (thenews.IsNull())
      thenews = new TColStd_HSequenceOfAsciiString();
    AsciiString1 newstr(newname);
    thenews->Append(newstr);
    num = thenews->Length();
  }
  theolds.Bind(oldname, num);
}

void XSDRAW1::RemoveCommand(const Standard_CString oldname)
{
  ChangeCommand(oldname, "");
}

Standard_Boolean XSDRAW1::LoadSession()
{
  if (deja)
    return Standard_False;
  deja                             = 1;
  thepilot                         = new IFSelect_SessionPilot("XSTEP-DRAW>");
  Handle(ExchangeSession) WS = new ExchangeSession;
  WS->SetVars(new XSDRAW_Vars);
  thepilot->SetSession(WS);

  IFSelectFunctions::Init();
  Functions1::Init();
  ShapeFunctions::Init();
  XSAlgo1::Init();
  //  XSDRAW_Shape::Init();   passe a present par theCommands
  return Standard_True;
}

void XSDRAW1::LoadDraw(DrawInterpreter& theCommands)
{
  if (dejald)
  {
    return;
  }
  dejald = 1;
  //  Pour tout faire d un coup : BRepTest1 & cie:
  LoadSession();

  // skl: we make remove commands "x" and "exit" in order to this commands are
  //      performed not in IFSelect_SessionPilot but in standard Tcl interpretor
  XSDRAW1::RemoveCommand("x");
  XSDRAW1::RemoveCommand("exit");

  //  if (!getenv("WBHOSTTOP")) XSDRAW1::RemoveCommand("xsnew");
  Handle(TColStd_HSequenceOfAsciiString) list = IFSelect_Activator::Commands(0);
  for (TColStd_HSequenceOfAsciiString::Iterator aCmdIter(*list); aCmdIter.More(); aCmdIter.Next())
  {
    Standard_Integer               num  = -1;
    const AsciiString1& aCmd = aCmdIter.Value();
    if (!theolds.IsEmpty())
    {
      theolds.Find(aCmd, num);
    }
    if (num == 0)
    {
      continue;
    }

    Standard_Integer           nact = 0;
    Handle(IFSelect_Activator) anAct;
    AsciiString1    aHelp;
    if (!IFSelect_Activator::Select(aCmd.ToCString(), nact, anAct))
    {
      aHelp = AsciiString1("type :  xhelp ") + aCmd + " for help";
    }
    else if (!anAct.IsNull())
    {
      aHelp = anAct->Help(nact);
    }

    const AsciiString1& aCmdName = num < 0 ? aCmd : thenews->Value(num);
    theCommands.Add(aCmdName.ToCString(), aHelp.ToCString(), "", XSTEPDRAWRUN, anAct->Group());
  }
}

Standard_Integer XSDRAW1::Execute(const Standard_CString command, const Standard_CString varname)
{
  char mess[100];
  Sprintf(mess, command, varname);
  thepilot->Execute(mess);
  return 1; // stat ?
}

Handle(IFSelect_SessionPilot) XSDRAW1::Pilot()
{
  return thepilot;
}

void XSDRAW1::SetSession(const Handle(ExchangeSession)& theSession)
{
  Pilot()->SetSession(theSession);
}

const Handle(ExchangeSession) XSDRAW1::Session()
{
  return XSControl1::Session(thepilot);
}

void XSDRAW1::SetController(const Handle(XSControl_Controller)& control)
{
  if (thepilot.IsNull())
    XSDRAW1::LoadSession();
  if (control.IsNull())
    std::cout << "XSTEP Controller not defined" << std::endl;
  else if (!Session().IsNull())
    Session()->SetController(control);
  else
    std::cout << "XSTEP Session badly or not defined" << std::endl;
}

Handle(XSControl_Controller) XSDRAW1::Controller()
{
  return Session()->NormAdaptor();
}

Standard_Boolean XSDRAW1::SetNorm(const Standard_CString norm)
{
  return Session()->SelectNorm(norm);
}

Handle(Interface_Protocol) XSDRAW1::Protocol()
{
  return thepilot->Session()->Protocol();
}

Handle(Interface_InterfaceModel) XSDRAW1::Model()
{
  return thepilot->Session()->Model();
}

void XSDRAW1::SetModel(const Handle(Interface_InterfaceModel)& model, const Standard_CString file)
{
  thepilot->Session()->SetModel(model);
  if (file && file[0] != '\0')
    thepilot->Session()->SetLoadedFile(file);
}

Handle(Interface_InterfaceModel) XSDRAW1::NewModel()
{
  return Session()->NewModel();
}

Handle(RefObject) XSDRAW1::Entity(const Standard_Integer num)
{
  return thepilot->Session()->StartingEntity(num);
}

Standard_Integer XSDRAW1::Number(const Handle(RefObject)& ent)
{
  return thepilot->Session()->StartingNumber(ent);
}

void XSDRAW1::SetTransferProcess(const Handle(RefObject)& ATP)
{
  DeclareAndCast(Transfer_FinderProcess, FP, ATP);
  DeclareAndCast(Transfer_TransientProcess, TP, ATP);

  //   Cas FinderProcess    ==> TransferWriter
  if (!FP.IsNull())
    Session()->SetMapWriter(FP);

  //   Cas TransientProcess ==> TransferReader
  if (!TP.IsNull())
  {
    if (!TP->Model().IsNull() && TP->Model() != Session()->Model())
      Session()->SetModel(TP->Model());
    Session()->SetMapReader(TP);
  }
}

Handle(Transfer_TransientProcess) XSDRAW1::TransientProcess()
{
  return Session()->TransferReader()->TransientProcess();
}

Handle(Transfer_FinderProcess) XSDRAW1::FinderProcess()
{
  return Session()->TransferWriter()->FinderProcess();
}

void XSDRAW1::InitTransferReader(const Standard_Integer mode)
{
  //   0 nullify  1 clear
  //   2 init TR avec contenu TP (roots)  3 init TP avec contenu TR
  //   4 init avec model (debut scratch)
  Session()->InitTransferReader(mode);
}

Handle(XSControl_TransferReader) XSDRAW1::TransferReader()
{
  return Session()->TransferReader();
}

//  ############  AUXILIAIRES  #############

Handle(RefObject) XSDRAW1::GetEntity(const Standard_CString name)
{
  return IFSelectFunctions::GiveEntity(Session(), name);
}

Standard_Integer XSDRAW1::GetEntityNumber(const Standard_CString name)
{
  return IFSelectFunctions::GiveEntityNumber(Session(), name);
}

Handle(TColStd_HSequenceOfTransient) XSDRAW1::GetList(const Standard_CString first,
                                                     const Standard_CString second)
{
  if (!first || first[0] == '\0')
  {
    std::string aLineFirst;
    std::cin >> aLineFirst;

    char terminateSymbol = '\0';
    std::cin.get(terminateSymbol);

    if (terminateSymbol == '\n')
      return XSDRAW1::GetList(aLineFirst.c_str(), nullptr);
    else
    {
      std::string aLineSecond;
      std::cin >> aLineSecond;
      return XSDRAW1::GetList(aLineFirst.c_str(), aLineSecond.c_str());
    }
  }
  return IFSelectFunctions::GiveList(Session(), first, second);
}

Standard_Boolean XSDRAW1::FileAndVar(const Standard_CString   file,
                                    const Standard_CString   var,
                                    const Standard_CString   def,
                                    AsciiString1& resfile,
                                    AsciiString1& resvar)
{
  return ShapeFunctions::FileAndVar(XSDRAW1::Session(), file, var, def, resfile, resvar);
}

Standard_Integer XSDRAW1::MoreShapes(Handle(TopTools_HSequenceOfShape)& list,
                                    const Standard_CString             name)
{
  return ShapeFunctions::MoreShapes(XSDRAW1::Session(), list, name);
}

//=================================================================================================

Standard_Real XSDRAW1::GetLengthUnit(const Handle(AppDocument)& theDoc)
{
  if (!theDoc.IsNull())
  {
    Standard_Real aUnit = 1.;
    if (XCAFDoc_DocumentTool::GetLengthUnit(theDoc, aUnit, UnitsMethods_LengthUnit_Millimeter))
    {
      return aUnit;
    }
  }
  if (ExchangeConfig::IsPresent("xstep.cascade.unit"))
  {
    UnitsMethods1::SetCasCadeLengthUnit(ExchangeConfig::IVal("xstep.cascade.unit"));
  }
  return UnitsMethods1::GetCasCadeLengthUnit();
}

//=================================================================================================

XSControl_WorkSessionMap& XSDRAW1::WorkSessionList()
{
  static std::shared_ptr<XSControl_WorkSessionMap> THE_PREVIOUS_WORK_SESSIONS;
  if (THE_PREVIOUS_WORK_SESSIONS == nullptr)
  {
    THE_PREVIOUS_WORK_SESSIONS = std::make_shared<XSControl_WorkSessionMap>();
  }
  return *THE_PREVIOUS_WORK_SESSIONS;
}

//=================================================================================================

void XSDRAW1::CollectActiveWorkSessions(const Handle(ExchangeSession)& theWS,
                                       const AsciiString1&       theName,
                                       XSControl_WorkSessionMap&            theMap)
{
  collectActiveWorkSessions(theWS, theName, theMap, Standard_True);
}

//=================================================================================================

void XSDRAW1::CollectActiveWorkSessions(const AsciiString1& theName)
{
  collectActiveWorkSessions(Session(), theName, WorkSessionList(), Standard_True);
}

//=================================================================================================

void XSDRAW1::Factory(DrawInterpreter& theDI)
{
  XSDRAW1::LoadDraw(theDI);
}

// Declare entry point PLUGINFACTORY
DPLUGIN(XSDRAW1)
