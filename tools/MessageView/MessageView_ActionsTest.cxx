// Created on: 2021-04-27
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2021 OPEN CASCADE SAS
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

#include <inspector/MessageView_ActionsTest.hxx>

#include <inspector/MessageModel_ItemReport.hxx>
#include <inspector/MessageModel_ItemRoot.hxx>
#include <inspector/MessageModel_ItemAlert.hxx>
#include <inspector/MessageModel_TreeModel.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <Message.hxx>
#include <Message_AlertExtended.hxx>
#include <Message_Level.hxx>
#include <Message_Messenger.hxx>

#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_OBB.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <OSD_Chronometer.hxx>
#include <Quantity_Color.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopoDS_AlertAttribute.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QAction>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

#include <ctime>

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
MessageView_ActionsTest::MessageView_ActionsTest(QWidget*                theParent,
                                                 MessageModel_TreeModel* theTreeModel,
                                                 QItemSelectionModel*    theModel)
    : QObject(theParent),
      myTreeModel(theTreeModel),
      mySelectionModel(theModel)
{
  myActions.insert(
    MessageModel_ActionType_TestMetric,
    ViewControl_Tools::CreateAction("Test <metric>", SLOT(OnTestMetric()), parent(), this));
  myActions.insert(MessageModel_ActionType_TestMessenger,
                   ViewControl_Tools::CreateAction("Test <Message_Messenger>",
                                                   SLOT(OnTestMessenger()),
                                                   parent(),
                                                   this));
  myActions.insert(MessageModel_ActionType_TestReportTree,
                   ViewControl_Tools::CreateAction("Test <Tree of messages>",
                                                   SLOT(OnTestReportTree()),
                                                   parent(),
                                                   this));
}

// =======================================================================
// function : AddMenuActions
// purpose :
// =======================================================================
void MessageView_ActionsTest::AddMenuActions(const QModelIndexList& theSelectedIndices,
                                             QMenu*                 theMenu)
{
  MessageModel_ItemReportPtr aReportItem;
  for (QModelIndexList::const_iterator aSelIt = theSelectedIndices.begin();
       aSelIt != theSelectedIndices.end();
       aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex(anIndex);
    if (!anItemBase)
      continue;

    MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot>(anItemBase);
    if (aRootItem)
      continue;

    aReportItem = itemDynamicCast<MessageModel_ItemReport>(anItemBase);
    if (aReportItem)
      break;

    MessageModel_ItemAlertPtr anAlertItem = itemDynamicCast<MessageModel_ItemAlert>(anItemBase);
    if (anAlertItem)
      continue;
  }

  if (aReportItem && !aReportItem->GetReport().IsNull())
  {
    theMenu->addAction(myActions[MessageModel_ActionType_TestMetric]);
    theMenu->addAction(myActions[MessageModel_ActionType_TestMessenger]);
    theMenu->addAction(myActions[MessageModel_ActionType_TestReportTree]);

    bool isReportEnabled = aReportItem->GetReport()->IsActiveInMessenger();
    myActions[MessageModel_ActionType_TestMetric]->setEnabled(isReportEnabled);
    myActions[MessageModel_ActionType_TestMessenger]->setEnabled(isReportEnabled);
    myActions[MessageModel_ActionType_TestReportTree]->setEnabled(isReportEnabled);
  }
  theMenu->addSeparator();
}

// =======================================================================
// function : getSelectedReport
// purpose :
// =======================================================================
Handle(Message_Report) MessageView_ActionsTest::getSelectedReport(QModelIndex& theReportIndex) const
{
  MessageModel_ItemReportPtr aReportItem;
  QModelIndexList            aSelectedIndices = mySelectionModel->selectedIndexes();
  for (QModelIndexList::const_iterator aSelIt = aSelectedIndices.begin();
       aSelIt != aSelectedIndices.end();
       aSelIt++)
  {
    QModelIndex anIndex = *aSelIt;
    if (anIndex.column() != 0)
      continue;

    TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex(anIndex);
    if (!anItemBase)
      continue;

    aReportItem    = itemDynamicCast<MessageModel_ItemReport>(anItemBase);
    theReportIndex = anIndex;
    if (aReportItem)
      break;
  }
  if (!aReportItem)
    return NULL;

  return aReportItem->GetReport();
}

// =======================================================================
// function : OnTestMetric
// purpose :
// =======================================================================
void MessageView_ActionsTest::OnTestMetric()
{
  QModelIndex            aReportIndex;
  Handle(Message_Report) aReport = getSelectedReport(aReportIndex);
  if (aReport.IsNull())
    return;

  OCCT_ADD_MESSAGE_LEVEL_SENTRY("MessageModel_Actions::OnTestMetric()");
  clock_t start_time = clock();

  Standard_Integer aCounter = 1500;
  Standard_Real    aValue = 0., aValue2 = 0.1;

  for (int aTopIt = 0; aTopIt < 4; aTopIt++)
  {
    Message1::SendInfo() << "Calculate";
    for (int j = 0; j < aCounter; j++)
    {
      for (int i = 0; i < aCounter; i++)
      {
        aValue = (aValue * 2. + 3.) * 0.5 - 0.3 * 0.5;

        Standard_Real aValue3 = aValue + aValue2 * 0.2;
        (void)aValue3;
      }
    }
  }

  myTreeModel->UpdateTreeModel();

  clock_t end_time = clock();
  std::cout << "clock() = " << end_time - start_time << std::endl;
}

// =======================================================================
// function : createShapeOnLevel
// purpose :
// =======================================================================
void createShapeOnLevel()
{
  OCCT_ADD_MESSAGE_LEVEL_SENTRY("createShapeOnLevel")

  Message_Messenger::StreamBuffer sout = Message1::SendInfo();

  EdgeMaker aBuilder(Point3d(0., 0., 0.), Point3d(20., 10., 20.));
  TopoShape            aShape = aBuilder.Shape();

  Message1::DefaultMessenger() << aShape;
}

// =======================================================================
// function : createShape
// purpose :
// =======================================================================
void createShape()
{
  Message_Messenger::StreamBuffer sout = Message1::SendInfo();
  EdgeMaker         aBuilder(Point3d(0., 0., 0.), Point3d(20., 10., 20.));
  TopoShape                    aShape = aBuilder.Shape();

  Message1::DefaultMessenger() << aShape;
  createShapeOnLevel();
}

// =======================================================================
// function : OnTestMessenger
// purpose :
// =======================================================================
void MessageView_ActionsTest::OnTestMessenger()
{
  // string messages
  OCCT_ADD_MESSAGE_LEVEL_SENTRY("MessageModel_Actions::OnTestMessenger()")

  Message1::DefaultMessenger()->Send("Values");
  Message1::DefaultMessenger()->Send("Values second");

  Message_Messenger::StreamBuffer sout = Message1::SendInfo();
  // Coords3d
  {
    Coords3d aCoords(1.3, 2.3, 3.4);
    aCoords.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Dir3d
  {
    Dir3d aDir(0.3, 0.3, 0.4);
    aDir.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Axis3d
  {
    Axis3d aCoords(Point3d(1.3, 2.3, 3.4), Dir3d(0.3, 0.3, 0.4));
    aCoords.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Frame3d
  {
    Frame3d aCoords(Point3d(10.3, 20.3, 30.4), Dir3d(0.3, 0.3, 0.4));
    aCoords.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Ax3
  {
    Ax3 aPln(Point3d(10., 20., 15.), Dir3d(0., 0., 1.), Dir3d(1., 0., 0.));
    aPln.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Transform3d
  {
    Transform3d aTrsf;
    aTrsf.SetRotation(gp1::OZ(), 0.3);
    aTrsf.SetTranslationPart(Vector3d(15., 15., 15.));
    aTrsf.SetScaleFactor(3.);

    aTrsf.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Box2
  {
    Box2 aBox(Point3d(20., 15., 10.), Point3d(25., 20., 15.));
    aBox.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // OrientedBox
  {
    OrientedBox anOBB(Point3d(-10., -15., -10.),
                  Dir3d(1., 0., 0.),
                  Dir3d(0., 1., 0.),
                  Dir3d(0., 0., 1.),
                  5.,
                  10.,
                  5.);
    anOBB.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Quantity_ColorRGBA
  {
    Quantity_ColorRGBA aColor(0.2f, 0.8f, 0.8f, 0.2f);
    aColor.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // Color1
  {
    Color1 aColor(0.8, 0.8, 0.8, Quantity_TOC_RGB);
    aColor.DumpJson(sout);
    sout.Flush(Standard_True);
  }
  // shape messages
  {
    createShape();
  }
  myTreeModel->UpdateTreeModel();
}

// =======================================================================
// function : levelAlerts
// purpose :
// =======================================================================
void levelAlerts(const int theCurrentLevel, const int theTopLevel)
{
  if (theTopLevel - theCurrentLevel <= 0)
    return;

  OCCT_ADD_MESSAGE_LEVEL_SENTRY(AsciiString1("Level: ") + theCurrentLevel)

  Message_Messenger::StreamBuffer sout = Message1::SendInfo();
  sout << "Alert(" << theCurrentLevel << "): " << 1 << ", " << 2 << std::endl;
  sout << "Alert(" << theCurrentLevel << "): " << 3 << ", " << 4 << std::endl;

  levelAlerts(theCurrentLevel + 1, theTopLevel);

  sout << "Alert(" << theCurrentLevel << "): " << 4 << ", " << 5 << std::endl;
}

// =======================================================================
// function : levelAlert
// purpose :
// =======================================================================
void levelAlert(const int theCurrentLevel, const int theTopLevel)
{
  if (theTopLevel - theCurrentLevel <= 0)
    return;

  OCCT_ADD_MESSAGE_LEVEL_SENTRY("levelAlert")

  Message_Messenger::StreamBuffer sout = Message1::SendInfo();
  sout << "Level: " << theCurrentLevel << "(Single, no alerts on the level)" << std::endl;

  levelAlerts(theCurrentLevel + 1, theTopLevel);
}

// =======================================================================
// function : OnTestReportTree
// purpose :
// =======================================================================
void MessageView_ActionsTest::OnTestReportTree()
{
  OCCT_ADD_MESSAGE_LEVEL_SENTRY("MessageModel_Actions::OnTestReportTree()")
  Message_Messenger::StreamBuffer sout = Message1::SendInfo();

  int aTopLevel = 3;
  levelAlerts(1, aTopLevel);

  sout << "Alert: " << 4 << std::endl;
  levelAlert(1, aTopLevel);

  myTreeModel->UpdateTreeModel();
}
