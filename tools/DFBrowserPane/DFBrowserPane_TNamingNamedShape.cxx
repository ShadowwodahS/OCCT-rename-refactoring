// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#include <inspector/DFBrowserPane_TNamingNamedShape.hxx>

#include <inspector/DFBrowserPane_AttributePaneModel.hxx>
#include <inspector/DFBrowserPane_ItemDelegateButton.hxx>
#include <inspector/DFBrowserPane_ItemRole.hxx>
#include <inspector/DFBrowserPane_HelperExport.hxx>
#include <inspector/DFBrowserPane_SelectionKind.hxx>
#include <inspector/DFBrowserPane_TableView.hxx>
#include <inspector/DFBrowserPane_Tools.hxx>
#include <inspector/TInspectorAPI_PluginParameters.hxx>

#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <TNaming_Builder.hxx>
#include <TNaming_Iterator.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QFileDialog>
#include <QGridLayout>
#include <QEvent>
#include <QHeaderView>
#include <QIcon>
#include <QMap>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVariant>
#include <QWidget>
#include <Standard_WarningsRestore.hxx>

static const int COLUMN_EXPORT_WIDTH = 20;

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
DFBrowserPane_TNamingNamedShape::DFBrowserPane_TNamingNamedShape()
    : DFBrowserPane_AttributePane(),
      myEvolutionTableView(0),
      myHelperExport(0)
{
  getPaneModel()->SetColumnCount(5);

  myEvolutionPaneModel = new DFBrowserPane_AttributePaneModel();
  myEvolutionPaneModel->SetColumnCount(10);
  QList<int> anItalicColumns;
  anItalicColumns << 1 << 5;
  myEvolutionPaneModel->SetItalicColumns(anItalicColumns);
  QItemSelectionModel* aSelectionModel = new QItemSelectionModel(myEvolutionPaneModel);
  mySelectionModels.push_back(aSelectionModel);
}

// =======================================================================
// function : CreateWidget
// purpose :
// =======================================================================
QWidget* DFBrowserPane_TNamingNamedShape::CreateWidget(QWidget* theParent)
{
  QWidget* aMainWidget = new QWidget(theParent);
  aMainWidget->setVisible(false);

  myTableView = new DFBrowserPane_TableView(aMainWidget);
  myTableView->GetTableView()->verticalHeader()->setVisible(false);
  myTableView->SetModel(myPaneModel);
  QTableView* aTableView = myTableView->GetTableView();
  aTableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  std::list<QItemSelectionModel*>::const_iterator aSelectionModelsIt = mySelectionModels.begin();
  aTableView->setSelectionModel(*aSelectionModelsIt);
  aSelectionModelsIt++;

  aTableView->horizontalHeader()->setStretchLastSection(false);
  aTableView->setColumnWidth(3, COLUMN_EXPORT_WIDTH);
  aTableView->setColumnWidth(4, COLUMN_EXPORT_WIDTH);
  DFBrowserPane_ItemDelegateButton* anItemDelegate =
    new DFBrowserPane_ItemDelegateButton(aTableView, ":/icons/export_shape.png");
  QList<int> aRows;
  aRows << 0 << 1;
  anItemDelegate->SetFreeRows(aRows);
  QObject::connect(anItemDelegate,
                   SIGNAL(buttonPressed(const QModelIndex&)),
                   &myHelperExport,
                   SLOT(OnButtonPressed(const QModelIndex&)));
  aTableView->setItemDelegateForColumn(3, anItemDelegate);

  DFBrowserPane_ItemDelegateButton* anItemDelegate2 =
    new DFBrowserPane_ItemDelegateButton(aTableView, ":/icons/folder_export.png");
  anItemDelegate2->SetFreeRows(aRows);
  aTableView->setItemDelegateForColumn(4, anItemDelegate2);

  myEvolutionTableView = new DFBrowserPane_TableView(aMainWidget);
  myEvolutionTableView->SetModel(myEvolutionPaneModel);
  aTableView = myEvolutionTableView->GetTableView();

  aTableView->setSelectionModel(*aSelectionModelsIt);

  aTableView->horizontalHeader()->setStretchLastSection(false);
  aTableView->setColumnWidth(4, COLUMN_EXPORT_WIDTH);
  aTableView->setColumnWidth(9, COLUMN_EXPORT_WIDTH);

  anItemDelegate = new DFBrowserPane_ItemDelegateButton(myEvolutionTableView->GetTableView(),
                                                        ":/icons/export_shape.png");
  QObject::connect(anItemDelegate,
                   SIGNAL(buttonPressed(const QModelIndex&)),
                   &myHelperExport,
                   SLOT(OnButtonPressed(const QModelIndex&)));
  myEvolutionTableView->GetTableView()->setItemDelegateForColumn(4, anItemDelegate);

  anItemDelegate = new DFBrowserPane_ItemDelegateButton(myEvolutionTableView->GetTableView(),
                                                        ":/icons/export_shape.png");
  QObject::connect(anItemDelegate,
                   SIGNAL(buttonPressed(const QModelIndex&)),
                   &myHelperExport,
                   SLOT(OnButtonPressed(const QModelIndex&)));
  myEvolutionTableView->GetTableView()->setItemDelegateForColumn(9, anItemDelegate);

  QGridLayout* aLay = new QGridLayout(aMainWidget);
  aLay->setContentsMargins(0, 0, 0, 0);
  aLay->addWidget(myTableView, 0, 0);
  aLay->addWidget(myEvolutionTableView, 1, 0);

  return aMainWidget;
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void DFBrowserPane_TNamingNamedShape::Init(const Handle(TDF_Attribute)& theAttribute)
{
  Handle(ShapeAttribute) aShapeAttr = Handle(ShapeAttribute)::DownCast(theAttribute);
  myHelperExport.Clear();

  QList<QVariant> aValues;
  aValues << "Version" << QString::number(aShapeAttr->Version()) << "" << "" << "";
  aValues << "Evolution"
          << DFBrowserPane_Tools::ToName(DB_NS_TYPE, aShapeAttr->Evolution()).ToCString() << ""
          << "" << "";

  NCollection_List<TopoShape> aShapes;
  QList<int>                     aFreeRows;
  aFreeRows << 0 << 1;

  TopoShape            aShape = aShapeAttr->Get();
  AsciiString1 aShapeInfo =
    !aShape.IsNull() ? Standard_Dump::GetPointerInfo(aShape.TShape()) : "";
  aValues << "Shape" << aShapeInfo.ToCString() << DFBrowserPane_Tools::ShapeTypeInfo(aShape) << ""
          << "";
  aShapes.Append(aShape);
  if (aShape.IsNull())
    aFreeRows << 2;

  TopoShape            aCurrentShape = Tool11::CurrentShape(aShapeAttr);
  AsciiString1 aCurrentShapeInfo =
    !aCurrentShape.IsNull() ? Standard_Dump::GetPointerInfo(aCurrentShape.TShape()) : "";
  aValues << "CurrentShape" << aCurrentShapeInfo.ToCString()
          << DFBrowserPane_Tools::ShapeTypeInfo(aCurrentShape) << "" << "";
  aShapes.Append(aCurrentShape);
  if (aCurrentShape.IsNull())
    aFreeRows << 3;

  TopoShape            anOriginalShape = Tool11::OriginalShape(aShapeAttr);
  AsciiString1 anOriginalShapeInfo =
    !anOriginalShape.IsNull() ? Standard_Dump::GetPointerInfo(anOriginalShape.TShape()) : "";
  aValues << "OriginalShape" << anOriginalShapeInfo.ToCString()
          << DFBrowserPane_Tools::ShapeTypeInfo(anOriginalShape) << "" << "";
  aShapes.Append(anOriginalShape);
  if (anOriginalShape.IsNull())
    aFreeRows << 4;

  DFBrowserPane_AttributePaneModel* aModel = getPaneModel();
  aModel->Init(aValues);
  if (myTableView)
  {
    QTableView* aTableView = myTableView->GetTableView();
    for (int i = 0; i < aModel->columnCount(); i++)
    {
      if (i == 3 || i == 4)
        dynamic_cast<DFBrowserPane_ItemDelegateButton*>(aTableView->itemDelegateForColumn(3))
          ->SetFreeRows(aFreeRows);
      else
        aTableView->resizeColumnToContents(i);
    }
  }
  QModelIndexList anIndices;
  int             aRowId = 2;
  for (NCollection_List<TopoShape>::Iterator aShapeIt(aShapes); aShapeIt.More();
       aShapeIt.Next(), aRowId++)
  {
    if (aShapeIt.Value().IsNull())
      continue;
    anIndices.clear();
    anIndices << aModel->index(aRowId, 1) << aModel->index(aRowId, 2) << aModel->index(aRowId, 3)
              << aModel->index(aRowId, 4);
    myHelperExport.AddShape(aShapeIt.Value(), anIndices);
  }

  // evolution table view filling
  aValues.clear();
  aRowId            = 0;
  bool aHasModified = false;
  for (Iterator1 aShapeAttrIt(aShapeAttr); aShapeAttrIt.More();
       aShapeAttrIt.Next(), aRowId++)
  {
    const TopoShape& anOldShape = aShapeAttrIt.OldShape();
    const TopoShape& aNewShape  = aShapeAttrIt.NewShape();

    Handle(ShapeAttribute) anOldAttr =
      Tool11::NamedShape1(anOldShape, aShapeAttr->Label());
    aValues << DFBrowserPane_Tools::ToName(DB_NS_TYPE, aShapeAttrIt.Evolution()).ToCString();
    aHasModified = aHasModified | aShapeAttrIt.IsModification();

    aValues << "New:";

    QString aLabelInfo;
    if (!anOldAttr.IsNull())
    {
      DataLabel anOldLabel = anOldAttr->Label();
      if (!anOldLabel.IsNull())
        aLabelInfo = QString(DFBrowserPane_Tools::GetEntry(anOldLabel).ToCString());
    }
    if (!aNewShape.IsNull())
      aValues << Standard_Dump::GetPointerInfo(aNewShape.TShape()->This()).ToCString()
              << DFBrowserPane_Tools::ShapeTypeInfo(aNewShape) << "";
    else
      aValues << "-" << "-" << "";
    aValues << "Old:";
    if (!anOldShape.IsNull())
      aValues << Standard_Dump::GetPointerInfo(anOldShape.TShape()->This()).ToCString()
              << DFBrowserPane_Tools::ShapeTypeInfo(anOldShape) << aLabelInfo << "";
    else
      aValues << "-" << "-" << "-" << "";
  }

  if (myEvolutionTableView)
  {
    myEvolutionTableView->setVisible(aValues.size() > 0);
    myEvolutionTableView->GetTableView()->setColumnHidden(1, !aHasModified);
    myEvolutionPaneModel->Init(aValues);

    aRowId = 0;
    for (Iterator1 aShapeAttrIt(aShapeAttr); aShapeAttrIt.More();
         aShapeAttrIt.Next(), aRowId++)
    {
      const TopoShape& anOldShape = aShapeAttrIt.OldShape();
      const TopoShape& aNewShape  = aShapeAttrIt.NewShape();

      if (!aNewShape.IsNull())
      {
        anIndices.clear();
        anIndices << myEvolutionPaneModel->index(aRowId, 2)
                  << myEvolutionPaneModel->index(aRowId, 3)
                  << myEvolutionPaneModel->index(aRowId, 4);
        myHelperExport.AddShape(aNewShape, anIndices);
      }
      if (!anOldShape.IsNull())
      {
        anIndices.clear();
        anIndices << myEvolutionPaneModel->index(aRowId, 6)
                  << myEvolutionPaneModel->index(aRowId, 7)
                  << myEvolutionPaneModel->index(aRowId, 9);
        myHelperExport.AddShape(anOldShape, anIndices);
      }
    }
    for (int i = 0; i < myEvolutionPaneModel->columnCount(); i++)
    {
      if (i == 5 || i == 10)
        continue;
      myEvolutionTableView->GetTableView()->resizeColumnToContents(i);
    }
  }
}

// =======================================================================
// function : GetValues
// purpose :
// =======================================================================
void DFBrowserPane_TNamingNamedShape::GetValues(const Handle(TDF_Attribute)&, QList<QVariant>&) {}

// =======================================================================
// function : GetAttributeInfo
// purpose :
// =======================================================================
QVariant DFBrowserPane_TNamingNamedShape::GetAttributeInfo(
  const Handle(TDF_Attribute)& theAttribute,
  int                          theRole,
  int                          theColumnId)
{
  if (theColumnId != 0)
    return DFBrowserPane_AttributePane::GetAttributeInfo(theAttribute, theRole, theColumnId);

  switch (theRole)
  {
    case Qt::DecorationRole:
      return QIcon(":/icons/named_shape.png");
    case DFBrowserPane_ItemRole_Decoration_40x40:
      return QIcon(":/icons/named_shape_40x40.png");
    case Qt::ForegroundRole: {
      TopoShape               aShape;
      Handle(ShapeAttribute) anAttribute = Handle(ShapeAttribute)::DownCast(theAttribute);
      if (!anAttribute.IsNull())
        aShape = anAttribute->Get();
      if (aShape.IsNull())
        return QColor(Qt::black);

      return QColor(aShape.Orientation() == TopAbs_FORWARD    ? Qt::darkGray
                    : aShape.Orientation() == TopAbs_REVERSED ? QColor(Qt::gray)
                                                              : Qt::black);
    }
    default:
      break;
  }
  return DFBrowserPane_AttributePane::GetAttributeInfo(theAttribute, theRole, theColumnId);
}

// =======================================================================
// function : GetShortAttributeInfo
// purpose :
// =======================================================================
void DFBrowserPane_TNamingNamedShape::GetShortAttributeInfo(
  const Handle(TDF_Attribute)& theAttribute,
  QList<QVariant>&             theValues)
{
  Handle(ShapeAttribute) aShapeAttribute = Handle(ShapeAttribute)::DownCast(theAttribute);

  if (aShapeAttribute->Get().IsNull())
    theValues.append("EMPTY SHAPE");
  else
  {
    theValues.append(
      QString("%1 : %2")
        .arg(DFBrowserPane_Tools::ToName(DB_SHAPE_TYPE, aShapeAttribute->Get().ShapeType())
               .ToCString())
        .arg(DFBrowserPane_Tools::ToName(DB_NS_TYPE, aShapeAttribute->Evolution()).ToCString()));
  }
}

// =======================================================================
// function : GetSelectionKind
// purpose :
// =======================================================================
int DFBrowserPane_TNamingNamedShape::GetSelectionKind(QItemSelectionModel* theModel)
{
  int aKind = DFBrowserPane_AttributePaneAPI::GetSelectionKind(theModel);

  QTableView* aTableView = myTableView->GetTableView();
  if (aTableView->selectionModel() != theModel)
    return aKind;

  QModelIndexList aSelectedIndices = theModel->selectedIndexes();
  if (aSelectedIndices.size() != 1)
    return aKind;

  QModelIndex aSelectedIndex = aSelectedIndices.first();
  int         aRow           = aSelectedIndex.row();
  if (aRow == 0 || aRow == 1)
    return aKind;

  if (aSelectedIndex.column() == 4)
    aKind = DFBrowserPane_SelectionKind_ExportToShapeViewer;

  return aKind;
}

// =======================================================================
// function : GetSelectionParameters
// purpose :
// =======================================================================
void DFBrowserPane_TNamingNamedShape::GetSelectionParameters(
  QItemSelectionModel*                          theModel,
  NCollection_List<Handle(RefObject)>& theParameters,
  NCollection_List<AsciiString1>&    theItemNames)
{
  QTableView* aTableView = myTableView->GetTableView();
  if (aTableView->selectionModel() != theModel)
    return;

  QModelIndexList aSelectedIndices = theModel->selectedIndexes();
  if (aSelectedIndices.size() != 1)
    return;

  QModelIndex aSelectedIndex = aSelectedIndices.first();
  if (aSelectedIndex.column() != 4)
    return;

  const TopoShape& aShape = myHelperExport.Shape(aSelectedIndex);
  if (aShape.IsNull())
    return;
  theParameters.Append(aShape.TShape());
  theItemNames.Append(TInspectorAPI_PluginParameters::ParametersToString(aShape));
}

// =======================================================================
// function : GetReferences
// purpose :
// =======================================================================
void DFBrowserPane_TNamingNamedShape::GetReferences(const Handle(TDF_Attribute)& theAttribute,
                                                    NCollection_List<DataLabel>& theRefLabels,
                                                    Handle(RefObject)&  theRefPresentation)
{
  if (!myEvolutionTableView)
    return;
  QStringList aSelectedEntries =
    DFBrowserPane_TableView::GetSelectedColumnValues(myEvolutionTableView->GetTableView(), 9);

  Handle(ShapeAttribute) aShapeAttr = Handle(ShapeAttribute)::DownCast(theAttribute);
  for (Iterator1 aShapeAttrIt(aShapeAttr); aShapeAttrIt.More(); aShapeAttrIt.Next())
  {
    const TopoShape& anOldShape = aShapeAttrIt.OldShape();

    Handle(ShapeAttribute) anOldAttr =
      Tool11::NamedShape1(anOldShape, aShapeAttr->Label());
    QString aLabelInfo;
    if (!anOldAttr.IsNull())
    {
      DataLabel anOldLabel = anOldAttr->Label();
      if (!anOldLabel.IsNull())
      {
        if (aSelectedEntries.contains(DFBrowserPane_Tools::GetEntry(anOldLabel).ToCString()))
          theRefLabels.Append(anOldLabel);
      }
    }
  }
  TopoShape aShape = getSelectedShapes();
  if (!aShape.IsNull())
  {
    Handle(VisualShape) aPresentation = new VisualShape(aShape);
    aPresentation->Attributes()->SetAutoTriangulation(Standard_False);
    theRefPresentation = aPresentation;
  }
}

// =======================================================================
// function : GetPresentation
// purpose :
// =======================================================================
Handle(RefObject) DFBrowserPane_TNamingNamedShape::GetPresentation(
  const Handle(TDF_Attribute)& theAttribute)
{
  Handle(RefObject) aPresentation;
  Handle(ShapeAttribute) aShapeAttr = Handle(ShapeAttribute)::DownCast(theAttribute);
  if (aShapeAttr.IsNull())
    return aPresentation;

  TopoShape aShape = aShapeAttr->Get();
  if (aShape.IsNull())
    return aPresentation;

  aPresentation = new VisualShape(aShape);
  return aPresentation;
}

// =======================================================================
// function : getSelectedShapes
// purpose :
// =======================================================================
TopoShape DFBrowserPane_TNamingNamedShape::getSelectedShapes()
{
  TopoShape aShape;

  if (!myTableView && !myEvolutionTableView)
    return aShape;

  // table view selected shapes
  QItemSelectionModel* aTableViewSelModel = myTableView->GetTableView()->selectionModel();
  QModelIndexList      anIndices          = aTableViewSelModel->selectedIndexes();

  ShapeBuilder    aBuilder;
  TopoCompound aComp;
  aBuilder.MakeCompound(aComp);
  bool aHasShapes = false;
  for (QModelIndexList::const_iterator anIt = anIndices.begin(), aLast = anIndices.end();
       anIt != aLast;
       anIt++)
  {
    QModelIndex anIndex = *anIt;
    if (!myHelperExport.HasShape(anIndex))
      continue;
    aBuilder.Add(aComp, myHelperExport.Shape(anIndex));
    aHasShapes = true;
  }

  // evolution table selected shapes
  aTableViewSelModel = myEvolutionTableView->GetTableView()->selectionModel();
  anIndices.clear();
  anIndices = aTableViewSelModel->selectedIndexes();
  for (QModelIndexList::const_iterator anIt = anIndices.begin(), aLast = anIndices.end();
       anIt != aLast;
       anIt++)
  {
    QModelIndex anIndex = *anIt;
    if (!myHelperExport.HasShape(anIndex))
      continue;
    aBuilder.Add(aComp, myHelperExport.Shape(anIndex));
    aHasShapes = true;
  }
  if (aHasShapes)
    aShape = aComp;
  return aShape;
}
