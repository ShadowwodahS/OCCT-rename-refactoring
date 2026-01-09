// Created on: 2015-05-14
// Created by: data exchange team
// Copyright (c) 2000-2015 OPEN CASCADE SAS
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

#include <XCAFDoc_Editor.hxx>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <NCollection_IncAllocator.hxx>
#include <Message.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFNoteObjects_NoteObject.hxx>
#include <XCAFDoc_AssemblyItemRef.hxx>
#include <XCAFDoc_AssemblyTool.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_DimTol.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDoc_Location.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_NoteBalloon.hxx>
#include <XCAFDoc_NoteBinData.hxx>
#include <XCAFDoc_NoteComment.hxx>
#include <XCAFDoc_NotesTool.hxx>
#include <XCAFDoc_ShapeMapTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_VisMaterial.hxx>
#include <XCAFDoc_VisMaterialTool.hxx>
#include <XCAFDoc_Volume.hxx>
#include <TDF_AttributeIterator.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_RelocationTable.hxx>
#include <TDF_Tool.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TNaming_NamedShape.hxx>
#include <TNaming_Builder.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS_Compound.hxx>

//=======================================================================
// function : Expand
// purpose  : Convert Shape to assembly
//=======================================================================
Standard_Boolean DocumentEditor::Expand(const DataLabel&       theDoc,
                                        const DataLabel&       theShape,
                                        const Standard_Boolean theRecursively)
{
  if (theDoc.IsNull() || theShape.IsNull())
  {
    return Standard_False;
  }
  Handle(XCAFDoc_ShapeTool) aShapeTool   = XCAFDoc_DocumentTool::ShapeTool(theDoc);
  Standard_Boolean          isAutoNaming = aShapeTool->AutoNaming();
  aShapeTool->SetAutoNaming(Standard_False);

  DataLabel aCompoundPartL = theShape;
  if (aShapeTool->IsReference(theShape))
    aShapeTool->GetReferredShape(aCompoundPartL, aCompoundPartL);

  TopoShape aS = aShapeTool->GetShape(aCompoundPartL);
  if (aShapeTool->Expand(aCompoundPartL))
  {
    // move attributes
    for (ChildIterator aPartIter(aCompoundPartL, Standard_True); aPartIter.More();
         aPartIter.Next())
    {
      DataLabel aChild = aPartIter.Value();
      // get part
      DataLabel aPart;
      if (aShapeTool->GetReferredShape(aChild, aPart))
      {
        CloneMetaData(aChild, aPart, NULL);
        // remove unnecessary links
        TopoShape aShape = aShapeTool->GetShape(aChild);
        if (!aShapeTool->GetShape(aPart.Father()).IsNull())
        {
          aPart.ForgetAttribute(XCAFDoc1::ShapeRefGUID());
          if (aShapeTool->GetShape(aPart.Father()).ShapeType() == TopAbs_COMPOUND)
          {
            aShapeTool->SetShape(aPart, aShape);
          }
          aPart.ForgetAttribute(XCAFDoc_ShapeMapTool::GetID());
          aChild.ForgetAllAttributes(Standard_False);
        }
        aChild.ForgetAttribute(ShapeAttribute::GetID());
        aChild.ForgetAttribute(XCAFDoc_ShapeMapTool::GetID());
      }
      else
      {
        // If new original shape is not created, try to process this child
        // as subshape of new part
        TDF_LabelSequence aUsers;
        if (aShapeTool->GetUsers(aChild, aUsers) > 0)
        {
          for (TDF_LabelSequence::Iterator anIter(aUsers); anIter.More(); anIter.Next())
          {
            DataLabel aSubLabel = anIter.Value();
            // remove unnecessary links
            aSubLabel.ForgetAttribute(XCAFDoc1::ShapeRefGUID());
            aSubLabel.ForgetAttribute(XCAFDoc_ShapeMapTool::GetID());
            CloneMetaData(aChild, aSubLabel, NULL);
          }
          aChild.ForgetAllAttributes(Standard_False);
        }
      }
    }
    // if assembly contains compound, expand it recursively(if flag theRecursively is true)
    if (theRecursively)
    {
      for (ChildIterator aPartIter(aCompoundPartL); aPartIter.More(); aPartIter.Next())
      {
        DataLabel aPart = aPartIter.Value();
        if (aShapeTool->GetReferredShape(aPart, aPart))
        {
          TopoShape aPartShape = aShapeTool->GetShape(aPart);
          if (!aPartShape.IsNull() && aPartShape.ShapeType() == TopAbs_COMPOUND)
            Expand(theDoc, aPart, theRecursively);
        }
      }
    }
    aShapeTool->SetAutoNaming(isAutoNaming);
    return Standard_True;
  }
  aShapeTool->SetAutoNaming(isAutoNaming);
  return Standard_False;
}

//=======================================================================
// function : Expand
// purpose  : Convert all compounds in Doc to assembly
//=======================================================================
Standard_Boolean DocumentEditor::Expand(const DataLabel&       theDoc,
                                        const Standard_Boolean theRecursively)
{
  if (theDoc.IsNull())
  {
    return Standard_False;
  }
  Standard_Boolean          aResult = Standard_False;
  TDF_LabelSequence         aLabels;
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(theDoc);
  aShapeTool->GetFreeShapes(aLabels);
  for (TDF_LabelSequence::Iterator anIter(aLabels); anIter.More(); anIter.Next())
  {
    const DataLabel    aLabel = anIter.Value();
    const TopoShape aS     = aShapeTool->GetShape(aLabel);
    if (!aS.IsNull() && aS.ShapeType() == TopAbs_COMPOUND && !aShapeTool->IsAssembly(aLabel))
    {
      if (Expand(theDoc, aLabel, theRecursively))
      {
        aResult = Standard_True;
      }
    }
  }
  return aResult;
}

//=================================================================================================

Standard_Boolean DocumentEditor::Extract(const TDF_LabelSequence& theSrcLabels,
                                         const DataLabel&         theDstLabel,
                                         const Standard_Boolean   theIsNoVisMat)
{
  if (theDstLabel.IsNull())
  {
    return Standard_False;
  }

  Handle(XCAFDoc_ShapeTool) aDstShapeTool = XCAFDoc_DocumentTool::ShapeTool(theDstLabel);
  NCollection_DataMap<Handle(XCAFDoc_VisMaterial), Handle(XCAFDoc_VisMaterial)> aVisMatMap;
  for (TDF_LabelSequence::Iterator aNewRootIter(theSrcLabels); aNewRootIter.More();
       aNewRootIter.Next())
  {
    // Shape
    TopLoc_Location           aLoc;
    TDF_LabelDataMap          aMap;
    const DataLabel           aSrcLabel     = aNewRootIter.Value();
    Handle(XCAFDoc_ShapeTool) aSrcShapeTool = XCAFDoc_DocumentTool::ShapeTool(aSrcLabel);
    Handle(XCAFDoc_Location)  aLocationAttr;
    if (aSrcLabel.FindAttribute(XCAFDoc_Location::GetID(), aLocationAttr))
    {
      aLoc = aLocationAttr->Get();
    }
    DataLabel aCompLabel = aSrcLabel;
    aSrcShapeTool->GetReferredShape(aSrcLabel, aCompLabel);
    DataLabel aResLabel     = CloneShapeLabel(aCompLabel, aSrcShapeTool, aDstShapeTool, aMap);
    DataLabel aNewCompLabel = aDstShapeTool->AddComponent(theDstLabel, aResLabel, aLoc);
    if (aNewCompLabel.IsNull())
    {
      TopoShape aNewShape = aDstShapeTool->GetShape(aResLabel);
      aNewShape.Move(aLoc);
      aNewCompLabel = aDstShapeTool->AddShape(aNewShape, false);
    }
    aMap.Bind(aSrcLabel, aNewCompLabel);
    aMap.Bind(aCompLabel, aResLabel);
    aDstShapeTool->UpdateAssemblies();
    // Attributes
    for (TDF_LabelDataMap::Iterator aLabelIter(aMap); aLabelIter.More(); aLabelIter.Next())
    {
      CloneMetaData(aLabelIter.Key1(),
                    aLabelIter.Value(),
                    &aVisMatMap,
                    true,
                    true,
                    true,
                    !theIsNoVisMat);
    }
  }
  return Standard_True;
}

//=================================================================================================

Standard_Boolean DocumentEditor::Extract(const DataLabel&       theSrcLabel,
                                         const DataLabel&       theDstLabel,
                                         const Standard_Boolean theIsNoVisMat)
{
  TDF_LabelSequence aSeq;
  aSeq.Append(theSrcLabel);
  return Extract(aSeq, theDstLabel, theIsNoVisMat);
}

//=================================================================================================

DataLabel DocumentEditor::CloneShapeLabel(const DataLabel&                 theSrcLabel,
                                          const Handle(XCAFDoc_ShapeTool)& theSrcShapeTool,
                                          const Handle(XCAFDoc_ShapeTool)& theDstShapeTool,
                                          TDF_LabelDataMap&                theMap)
{
  DataLabel aNewShL;
  if (theMap.Find(theSrcLabel, aNewShL))
  {
    return aNewShL;
  }

  // Location for main assembly
  if (theSrcShapeTool->IsAssembly(theSrcLabel))
  {
    // add assembly and iterate all its components
    TopoCompound aComp;
    ShapeBuilder().MakeCompound(aComp);
    aNewShL = theDstShapeTool->AddShape(aComp);
    theMap.Bind(theSrcLabel, aNewShL);

    TDF_LabelSequence aComponents;
    theSrcShapeTool->GetComponents(theSrcLabel, aComponents);
    for (TDF_LabelSequence::Iterator aCompIter(aComponents); aCompIter.More(); aCompIter.Next())
    {
      DataLabel aCompL = aCompIter.Value();
      DataLabel aRefL;
      theSrcShapeTool->GetReferredShape(aCompL, aRefL);
      DataLabel aCompOriginalL = CloneShapeLabel(aRefL, theSrcShapeTool, theDstShapeTool, theMap);
      Handle(XCAFDoc_Location) aLocationAttr;
      aCompL.FindAttribute(XCAFDoc_Location::GetID(), aLocationAttr);
      DataLabel aNewCompL =
        theDstShapeTool->AddComponent(aNewShL, aCompOriginalL, aLocationAttr->Get());
      theMap.Bind(aCompIter.Value(), aNewCompL);
    }
    return aNewShL;
  }

  // add part
  TopoShape aShape = theSrcShapeTool->GetShape(theSrcLabel);
  aNewShL             = theDstShapeTool->AddShape(aShape, false);
  theMap.Bind(theSrcLabel, aNewShL);

  // get original instead of auxiliary instance
  DataLabel anOldOriginalL = theSrcLabel;
  if (theSrcShapeTool->IsReference(theSrcLabel))
  {
    theSrcShapeTool->GetReferredShape(theSrcLabel, anOldOriginalL);
    theDstShapeTool->GetReferredShape(aNewShL, aNewShL);
    theMap.Bind(anOldOriginalL, aNewShL);
  }
  // copy subshapes
  TDF_LabelSequence anOldSubShapes;
  theSrcShapeTool->GetSubShapes(anOldOriginalL, anOldSubShapes);
  for (TDF_LabelSequence::Iterator aSubIter(anOldSubShapes); aSubIter.More(); aSubIter.Next())
  {
    TopoShape aSubShape     = theSrcShapeTool->GetShape(aSubIter.Value());
    DataLabel    aNewSubShapeL = theDstShapeTool->AddSubShape(aNewShL, aSubShape);
    theMap.Bind(aSubIter.Value(), aNewSubShapeL);
  }
  return aNewShL;
}

//=================================================================================================

void DocumentEditor::CloneMetaData(
  const DataLabel&                                                               theSrcLabel,
  const DataLabel&                                                               theDstLabel,
  NCollection_DataMap<Handle(XCAFDoc_VisMaterial), Handle(XCAFDoc_VisMaterial)>* theVisMatMap,
  const Standard_Boolean                                                         theToCopyColor,
  const Standard_Boolean                                                         theToCopyLayer,
  const Standard_Boolean                                                         theToCopyMaterial,
  const Standard_Boolean theToCopyVisMaterial,
  const Standard_Boolean theToCopyAttribute)
{
  if (theSrcLabel == theDstLabel || theSrcLabel.IsNull() || theDstLabel.IsNull())
  {
    return;
  }
  const Standard_Boolean toCopyColor =
    theToCopyColor && XCAFDoc_DocumentTool::CheckColorTool(theSrcLabel);
  const Standard_Boolean toCopyLayer =
    theToCopyLayer && XCAFDoc_DocumentTool::CheckLayerTool(theSrcLabel);
  const Standard_Boolean toCopyMaterial =
    theToCopyMaterial && XCAFDoc_DocumentTool::CheckMaterialTool(theSrcLabel);
  const Standard_Boolean toCopyVisMaterial =
    XCAFDoc_DocumentTool::CheckVisMaterialTool(theSrcLabel);
  // Colors
  if (toCopyColor)
  {
    Handle(XCAFDoc_ColorTool) aSrcColorTool = XCAFDoc_DocumentTool::ColorTool(theSrcLabel);
    Handle(XCAFDoc_ColorTool) aDstColorTool = XCAFDoc_DocumentTool::ColorTool(theDstLabel);
    const XCAFDoc_ColorType   aTypes[] = {XCAFDoc_ColorGen, XCAFDoc_ColorSurf, XCAFDoc_ColorCurv};
    for (int anInd = 0; anInd < 3; anInd++)
    {
      DataLabel aColorL;
      aSrcColorTool->GetColor(theSrcLabel, aTypes[anInd], aColorL);
      if (!aColorL.IsNull())
      {
        Quantity_ColorRGBA aColor;
        aSrcColorTool->GetColor(aColorL, aColor);
        aDstColorTool->SetColor(theDstLabel, aColor, aTypes[anInd]);
      }
    }
    aDstColorTool->SetVisibility(theDstLabel, aSrcColorTool->IsVisible(theSrcLabel));
  }
  // Layers
  if (toCopyLayer)
  {
    Handle(XCAFDoc_LayerTool) aSrcLayerTool = XCAFDoc_DocumentTool::LayerTool(theSrcLabel);
    Handle(XCAFDoc_LayerTool) aDstLayerTool = XCAFDoc_DocumentTool::LayerTool(theDstLabel);
    TDF_LabelSequence         aLayers;
    aSrcLayerTool->GetLayers(theSrcLabel, aLayers);
    for (TDF_LabelSequence::Iterator aLayerIter(aLayers); aLayerIter.More(); aLayerIter.Next())
    {
      UtfString aLayerName;
      aSrcLayerTool->GetLayer(aLayerIter.Value(), aLayerName);
      aDstLayerTool->SetLayer(theDstLabel, aLayerName);
    }
  }
  // Materials
  if (toCopyMaterial)
  {
    Handle(TDataStd_TreeNode) aMatNode;
    if (theSrcLabel.FindAttribute(XCAFDoc1::MaterialRefGUID(), aMatNode) && aMatNode->HasFather())
    {
      DataLabel aMaterialL = aMatNode->Father()->Label();
      if (!aMaterialL.IsNull())
      {
        Handle(XCAFDoc_MaterialTool) aSrcMaterialTool =
          XCAFDoc_DocumentTool::MaterialTool(theSrcLabel);
        Handle(XCAFDoc_MaterialTool) aDstMaterialTool =
          XCAFDoc_DocumentTool::MaterialTool(theDstLabel);
        double                           aDensity = 0.0;
        Handle(TCollection_HAsciiString) aName, aDescription, aDensName, aDensValType;
        if (aSrcMaterialTool
              ->GetMaterial(aMaterialL, aName, aDescription, aDensity, aDensName, aDensValType)
            && !aName.IsNull() && aName->Length() != 0)
        {
          aDstMaterialTool
            ->SetMaterial(theDstLabel, aName, aDescription, aDensity, aDensName, aDensValType);
        }
      }
    }
  }
  // Visual Materials
  if (toCopyVisMaterial && (theToCopyVisMaterial || toCopyColor))
  {
    Handle(XCAFDoc_VisMaterialTool) aSrcVisMatTool =
      XCAFDoc_DocumentTool::VisMaterialTool(theSrcLabel);
    DataLabel aVisMaterialL;
    aSrcVisMatTool->GetShapeMaterial(theSrcLabel, aVisMaterialL);
    if (!aVisMaterialL.IsNull())
    {
      Handle(XCAFDoc_VisMaterialTool) aDstVisMatTool;
      Handle(XCAFDoc_ColorTool)       aDstColorTool;
      if (theToCopyVisMaterial)
      {
        aDstVisMatTool = XCAFDoc_DocumentTool::VisMaterialTool(theDstLabel);
      }
      else
      {
        aDstColorTool = XCAFDoc_DocumentTool::ColorTool(theDstLabel);
      }
      Handle(XCAFDoc_VisMaterial) aVisMatSrc = aSrcVisMatTool->GetMaterial(aVisMaterialL);
      if (theToCopyVisMaterial)
      {
        Handle(XCAFDoc_VisMaterial) aVisMatDst;
        if (theVisMatMap != NULL)
        {
          if (!theVisMatMap->Find(aVisMatSrc, aVisMatDst))
          {
            aVisMatDst = new XCAFDoc_VisMaterial();
            aVisMatDst->SetCommonMaterial(aVisMatSrc->CommonMaterial());
            aVisMatDst->SetPbrMaterial(aVisMatSrc->PbrMaterial());
            aVisMatDst->SetAlphaMode(aVisMatSrc->AlphaMode(), aVisMatSrc->AlphaCutOff());
            aVisMatDst->SetFaceCulling(aVisMatSrc->FaceCulling());
            AsciiString1 aName;
            Handle(NameAttribute)   aNodeName;
            if (aVisMatSrc->Label().FindAttribute(NameAttribute::GetID(), aNodeName))
            {
              aName = aNodeName->Get();
            }
            aDstVisMatTool->AddMaterial(aVisMatDst, aName);
            theVisMatMap->Bind(aVisMatSrc, aVisMatDst);
          }
        }
        else
        {
          aVisMatDst = aVisMatSrc; // consider the same document
        }
        aDstVisMatTool->SetShapeMaterial(theDstLabel, aVisMatDst->Label());
      }
      else
      {
        aDstColorTool->SetColor(theDstLabel, aVisMatSrc->BaseColor(), XCAFDoc_ColorGen);
      }
    }
  }
  // Other attributes
  if (theToCopyAttribute)
  {
    // Finds the target attributes or creates them empty
    for (TDF_AttributeIterator anAttIter(theSrcLabel); anAttIter.More(); anAttIter.Next())
    {
      const Handle(TDF_Attribute) anAttSrc = anAttIter.Value();
      // protect against color and layer coping without link to colors and layers
      if (const TDataStd_TreeNode* aTreeNode =
            dynamic_cast<const TDataStd_TreeNode*>(anAttSrc.get()))
      {
        (void)aTreeNode;
        continue;
      }
      else if (const XCAFDoc_GraphNode* aGraphNode =
                 dynamic_cast<const XCAFDoc_GraphNode*>(anAttSrc.get()))
      {
        (void)aGraphNode;
        continue;
      }
      else if (const ShapeAttribute* aShapeAttr =
                 dynamic_cast<const ShapeAttribute*>(anAttSrc.get()))
      {
        (void)aShapeAttr;
        continue;
      }
      else if (const XCAFDoc_ShapeMapTool* aShMapTool =
                 dynamic_cast<const XCAFDoc_ShapeMapTool*>(anAttSrc.get()))
      {
        (void)aShMapTool;
        continue;
      }
      else if (const XCAFDoc_Location* aLocAttr =
                 dynamic_cast<const XCAFDoc_Location*>(anAttSrc.get()))
      {
        (void)aLocAttr;
        continue;
      }
      Handle(TDF_Attribute) anAttDst;
      if (!theDstLabel.FindAttribute(anAttSrc->ID(), anAttDst))
      {
        anAttDst = anAttSrc->NewEmpty();
        theDstLabel.AddAttribute(anAttDst);
      }
      Handle(RelocationTable1) aRT = new RelocationTable1();
      anAttSrc->Paste(anAttDst, aRT);
    }
  }
  // Name
  Handle(NameAttribute) aNameAttr;
  theSrcLabel.FindAttribute(NameAttribute::GetID(), aNameAttr);
  Handle(XCAFDoc_ShapeTool) aDstShapeTool = XCAFDoc_DocumentTool::ShapeTool(theDstLabel);
  if (!aNameAttr.IsNull())
  {
    DataLabel aRefLabel;
    if (aNameAttr->Get().Search("=>") < 0)
    {
      NameAttribute::Set(theDstLabel, aNameAttr->Get());
    }
    else if (aDstShapeTool->GetReferredShape(theDstLabel, aRefLabel))
    {
      AsciiString1 aRefName;
      Tool3::Entry(aRefLabel, aRefName);
      aRefName.Insert(1, "=>");
      NameAttribute::Set(theDstLabel, aRefName);
    }
  }
}

//=======================================================================
// function : rescaleDimensionRefLabels
// purpose  : Applies geometrical scale to dimension's reference shapes
//           not belonging to the assembly graph
//=======================================================================

static void rescaleDimensionRefLabels(const TDF_LabelSequence&             theRefLabels,
                                      BRepBuilderAPI_Transform&            theBRepTrsf,
                                      const Handle(XCAFDoc_AssemblyGraph)& theGraph,
                                      const AsciiString1&       theEntryDimension)
{
  for (TDF_LabelSequence::Iterator anIt(theRefLabels); anIt.More(); anIt.Next())
  {
    const DataLabel& aL = anIt.Value();
    if (!theGraph->GetNodes().Contains(aL))
    {
      Handle(ShapeAttribute) aNS;
      if (aL.FindAttribute(ShapeAttribute::GetID(), aNS))
      {
        TopoShape aShape = aNS->Get();
        theBRepTrsf.Perform(aShape, Standard_True, Standard_True);
        if (!theBRepTrsf.IsDone())
        {
          Standard_SStream aSS;
          aSS << "Dimension PMI " << theEntryDimension << " is not scaled.";
          Message1::SendWarning(aSS.str().c_str());
        }
        else
        {
          TopoShape    aScaledShape = theBRepTrsf.Shape();
          TNaming_Builder aBuilder(aL);
          aBuilder.Generated(aShape, aScaledShape);
        }
      }
    }
  }
}

//=======================================================================
// function : shouldRescaleAndCheckRefLabels
// purpose  : Checks if all PMI reference shapes belong to the assembly
//           graph. Returns true if at least one reference shape belongs
//           to the assembly graph.
//=======================================================================

static Standard_Boolean shouldRescaleAndCheckRefLabels(
  const Handle(Data2)&              theData,
  const TDF_LabelSequence&             theRefLabels,
  const Handle(XCAFDoc_AssemblyGraph)& theGraph,
  Standard_Boolean&                    theAllInG)
{
  theAllInG                       = Standard_True;
  Standard_Boolean aShouldRescale = Standard_False;
  for (TDF_LabelSequence::Iterator anIt1(theRefLabels); anIt1.More(); anIt1.Next())
  {
    const DataLabel& aL = anIt1.Value();
    if (theGraph->GetNodes().Contains(aL))
    {
      aShouldRescale = Standard_True;
    }
    else
    {
      Handle(XCAFDoc_AssemblyItemRef) anItemRefAttr;
      if (!aL.FindAttribute(XCAFDoc_AssemblyItemRef::GetID(), anItemRefAttr))
      {
        theAllInG = Standard_False;
        continue;
      }
      const AssemblyItemId& anItemId = anItemRefAttr->GetItem();
      if (anItemId.IsNull())
      {
        theAllInG = Standard_False;
        continue;
      }
      DataLabel aLRef;
      Tool3::Label(theData, anItemId.GetPath().Last(), aLRef, Standard_False);
      if (aLRef.IsNull() || !theGraph->GetNodes().Contains(aLRef))
      {
        theAllInG = Standard_False;
        continue;
      }
      aShouldRescale = Standard_True;
    }
  }
  return aShouldRescale;
}

//=================================================================================================

void DocumentEditor::GetChildShapeLabels(const DataLabel& theLabel, TDF_LabelMap& theRelatedLabels)
{
  if (theLabel.IsNull() || !XCAFDoc_ShapeTool::IsShape(theLabel))
  {
    return;
  }
  if (!theRelatedLabels.Add(theLabel))
  {
    return; // Label already processed
  }
  if (XCAFDoc_ShapeTool::IsAssembly(theLabel) || XCAFDoc_ShapeTool::IsSimpleShape(theLabel))
  {
    for (ChildIterator aChildIter(theLabel); aChildIter.More(); aChildIter.Next())
    {
      const DataLabel& aChildLabel = aChildIter.Value();
      GetChildShapeLabels(aChildLabel, theRelatedLabels);
    }
  }
  if (XCAFDoc_ShapeTool::IsReference(theLabel))
  {
    DataLabel aRefLabel;
    XCAFDoc_ShapeTool::GetReferredShape(theLabel, aRefLabel);
    GetChildShapeLabels(aRefLabel, theRelatedLabels);
  }
}

//=================================================================================================

void DocumentEditor::GetParentShapeLabels(const DataLabel& theLabel, TDF_LabelMap& theRelatedLabels)
{
  if (theLabel.IsNull() || !XCAFDoc_ShapeTool::IsShape(theLabel))
  {
    return;
  }
  if (!theRelatedLabels.Add(theLabel))
  {
    return; // Label already processed
  }
  if (XCAFDoc_ShapeTool::IsSubShape(theLabel) || XCAFDoc_ShapeTool::IsComponent(theLabel))
  {
    DataLabel aFatherLabel = theLabel.Father();
    GetParentShapeLabels(aFatherLabel, theRelatedLabels);
  }
  else
  {
    TDF_LabelSequence aUsers;
    XCAFDoc_ShapeTool::GetUsers(theLabel, aUsers);
    if (!aUsers.IsEmpty())
    {
      for (TDF_LabelSequence::Iterator aUserIter(aUsers); aUserIter.More(); aUserIter.Next())
      {
        const DataLabel& aUserLabel = aUserIter.Value();
        GetParentShapeLabels(aUserLabel, theRelatedLabels);
      }
    }
  }
}

//=================================================================================================

bool DocumentEditor::FilterShapeTree(const Handle(XCAFDoc_ShapeTool)& theShapeTool,
                                     const TDF_LabelMap&              theLabelsToKeep)
{
  if (theLabelsToKeep.IsEmpty())
  {
    return false;
  }
  Handle(NCollection_BaseAllocator) anAllocator = new NCollection_IncAllocator();
  TDF_LabelMap                      aLabelsToKeep(theLabelsToKeep.Size(), anAllocator);
  for (TDF_LabelMap::Iterator aLabelIter(theLabelsToKeep); aLabelIter.More(); aLabelIter.Next())
  {
    GetChildShapeLabels(aLabelIter.Key1(), aLabelsToKeep);
  }
  TDF_LabelMap aInternalLabels(1, anAllocator);
  for (TDF_LabelMap::Iterator aLabelIter(theLabelsToKeep); aLabelIter.More(); aLabelIter.Next())
  {
    GetParentShapeLabels(aLabelIter.Key1(), aInternalLabels);
    NCollection_MapAlgo::Unite(aLabelsToKeep, aInternalLabels);
    aInternalLabels.Clear(false);
  }
  for (ChildIterator aLabelIter(theShapeTool->Label(), true); aLabelIter.More();
       aLabelIter.Next())
  {
    const DataLabel& aLabel = aLabelIter.Value();
    if (!aLabelsToKeep.Contains(aLabel))
    {
      aLabel.ForgetAllAttributes(Standard_False);
    }
  }
  theShapeTool->UpdateAssemblies();
  return true;
}

//=======================================================================
// function : RescaleGeometry
// purpose  : Applies geometrical scale to all assembly parts, component
//           locations and related attributes
//=======================================================================
Standard_Boolean DocumentEditor::RescaleGeometry(const DataLabel&       theLabel,
                                                 const Standard_Real    theScaleFactor,
                                                 const Standard_Boolean theForceIfNotRoot)
{
  if (theLabel.IsNull())
  {
    Message1::SendFail("Null label.");
    return Standard_False;
  }

  if (Abs(theScaleFactor) <= gp1::Resolution())
  {
    Message1::SendFail("Scale factor is too small.");
    return Standard_False;
  }

  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(theLabel);
  if (aShapeTool.IsNull())
  {
    Message1::SendFail("Couldn't find XCAFDoc_ShapeTool attribute.");
    return Standard_False;
  }

  if (!theForceIfNotRoot && aShapeTool->Label() != theLabel)
  {
    TDF_LabelSequence aFreeLabels;
    aShapeTool->GetFreeShapes(aFreeLabels);
    Standard_Boolean aFound = Standard_False;
    for (TDF_LabelSequence::Iterator anIt(aFreeLabels); anIt.More(); anIt.Next())
    {
      if (theLabel == anIt.Value())
      {
        aFound = Standard_True;
        break;
      }
    }
    if (!aFound)
    {
      AsciiString1 anEntry;
      Tool3::Entry(theLabel, anEntry);
      Standard_SStream aSS;
      aSS << "Label " << anEntry << " is not a root. Set ForceIfNotRoot true to rescale forcibly.";
      Message1::SendFail(aSS.str().c_str());
      return Standard_False;
    }
  }

  Handle(XCAFDoc_AssemblyGraph) aG = new XCAFDoc_AssemblyGraph(theLabel);
  if (aG.IsNull())
  {
    Message1::SendFail("Couldn't create assembly graph.");
    return Standard_False;
  }

  Standard_Boolean anIsDone = Standard_True;

  Transform3d aTrsf;
  aTrsf.SetScaleFactor(theScaleFactor);
  BRepBuilderAPI_Transform aBRepTrsf(aTrsf);

  AssemblyTool::Traverse(
    aG,
    [](const Handle(XCAFDoc_AssemblyGraph)& theGraph,
       const Standard_Integer               theNode) -> Standard_Boolean {
      const XCAFDoc_AssemblyGraph::NodeType aNodeType = theGraph->GetNodeType(theNode);
      return (aNodeType == XCAFDoc_AssemblyGraph::NodeType_Part)
             || (aNodeType == XCAFDoc_AssemblyGraph::NodeType_Occurrence);
    },
    [&](const Handle(XCAFDoc_AssemblyGraph)& theGraph,
        const Standard_Integer               theNode) -> Standard_Boolean {
      const DataLabel&                      aLabel    = theGraph->GetNode(theNode);
      const XCAFDoc_AssemblyGraph::NodeType aNodeType = theGraph->GetNodeType(theNode);

      if (aNodeType == XCAFDoc_AssemblyGraph::NodeType_Part)
      {
        const TopoShape aShape = aShapeTool->GetShape(aLabel);
        aBRepTrsf.Perform(aShape, Standard_True, Standard_True);
        if (!aBRepTrsf.IsDone())
        {
          Standard_SStream        aSS;
          AsciiString1 anEntry;
          Tool3::Entry(aLabel, anEntry);
          aSS << "Shape " << anEntry << " is not scaled!";
          Message1::SendFail(aSS.str().c_str());
          anIsDone = Standard_False;
          return Standard_False;
        }
        TopoShape aScaledShape = aBRepTrsf.Shape();
        aShapeTool->SetShape(aLabel, aScaledShape);

        // Update sub-shapes
        TDF_LabelSequence aSubshapes;
        aShapeTool->GetSubShapes(aLabel, aSubshapes);
        for (TDF_LabelSequence::Iterator anItSs(aSubshapes); anItSs.More(); anItSs.Next())
        {
          const DataLabel&   aLSs = anItSs.Value();
          const TopoShape aSs  = aShapeTool->GetShape(aLSs);
          const TopoShape aSs1 = aBRepTrsf.ModifiedShape(aSs);
          aShapeTool->SetShape(aLSs, aSs1);
        }

        Handle(XCAFDoc_Area) aArea;
        if (aLabel.FindAttribute(XCAFDoc_Area::GetID(), aArea))
        {
          aArea->Set(aArea->Get() * theScaleFactor * theScaleFactor);
        }

        Handle(XCAFDoc_Centroid) aCentroid;
        if (aLabel.FindAttribute(XCAFDoc_Centroid::GetID(), aCentroid))
        {
          aCentroid->Set(aCentroid->Get().XYZ() * theScaleFactor);
        }

        Handle(XCAFDoc_Volume) aVolume;
        if (aLabel.FindAttribute(XCAFDoc_Volume::GetID(), aVolume))
        {
          aVolume->Set(aVolume->Get() * theScaleFactor * theScaleFactor * theScaleFactor);
        }
      }
      else if (aNodeType == XCAFDoc_AssemblyGraph::NodeType_Occurrence)
      {
        TopLoc_Location aLoc  = aShapeTool->GetLocation(aLabel);
        Transform3d         aTrsf = aLoc.Transformation();
        aTrsf.SetTranslationPart(aTrsf.TranslationPart() * theScaleFactor);
        XCAFDoc_Location::Set(aLabel, aTrsf);
      }

      return Standard_True;
    });

  if (!anIsDone)
  {
    return Standard_False;
  }

  aShapeTool->UpdateAssemblies();

  Handle(XCAFDoc_DimTolTool) aDimTolTool = XCAFDoc_DocumentTool::DimTolTool(theLabel);
  if (!aDimTolTool.IsNull())
  {
    TDF_LabelSequence aDimensions;
    aDimTolTool->GetDimensionLabels(aDimensions);
    for (TDF_LabelSequence::Iterator anItD(aDimensions); anItD.More(); anItD.Next())
    {
      const DataLabel& aDimension = anItD.Value();

      AsciiString1 anEntryDimension;
      Tool3::Entry(aDimension, anEntryDimension);

      Handle(XCAFDoc_Dimension) aDimAttr;
      if (aDimension.FindAttribute(XCAFDoc_Dimension::GetID(), aDimAttr))
      {
        Standard_Boolean  aShouldRescale = Standard_False;
        Standard_Boolean  aFirstLInG     = Standard_True;
        Standard_Boolean  aSecondLInG    = Standard_True;
        TDF_LabelSequence aShapeLFirst, aShapeLSecond;
        Standard_Boolean  aHasShapeRefs =
          aDimTolTool->GetRefShapeLabel(aDimension, aShapeLFirst, aShapeLSecond);
        if (aHasShapeRefs)
        {
          aShouldRescale =
            shouldRescaleAndCheckRefLabels(theLabel.Data(), aShapeLFirst, aG, aFirstLInG)
            || shouldRescaleAndCheckRefLabels(theLabel.Data(), aShapeLSecond, aG, aSecondLInG);
        }

        if (!aShouldRescale)
        {
          Standard_SStream aSS;
          aSS << "Dimension PMI " << anEntryDimension << " is not scaled!";
          Message1::SendWarning(aSS.str().c_str());
          continue;
        }

        Handle(XCAFDimTolObjects_DimensionObject) aDimObj = aDimAttr->GetObject();

        if (aDimObj->HasTextPoint())
        {
          aDimObj->SetPointTextAttach(aDimObj->GetPointTextAttach().XYZ() * theScaleFactor);
        }

        if (aDimObj->HasPoint())
        {
          aDimObj->SetPoint(aDimObj->GetPoint().XYZ() * theScaleFactor);
        }

        if (aDimObj->HasPoint2())
        {
          aDimObj->SetPoint2(aDimObj->GetPoint2().XYZ() * theScaleFactor);
        }

        if (aDimObj->HasPlane())
        {
          Frame3d aPln = aDimObj->GetPlane();
          aPln.SetLocation(aPln.Location().XYZ() * theScaleFactor);
          aDimObj->SetPlane(aPln);
        }

        Handle(TColStd_HArray1OfReal) aValues = aDimObj->GetValues();
        if (!aValues.IsNull())
        {
          if (!aFirstLInG || !aSecondLInG)
          {
            Standard_SStream aSS;
            aSS << "Dimension PMI " << anEntryDimension
                << " base shapes do not belong to the rescaled assembly!";
            Message1::SendWarning(aSS.str().c_str());
            continue;
          }
          Standard_Boolean      aRescaleOtherValues = Standard_False;
          TColStd_Array1OfReal& anArray             = aValues->ChangeArray1();
          switch (aDimObj->GetType())
          {
            case XCAFDimTolObjects_DimensionType_Location_None:
            case XCAFDimTolObjects_DimensionType_Location_CurvedDistance: {
              Standard_SStream aSS;
              aSS << "Dimension PMI " << anEntryDimension << " is not scaled.";
              Message1::SendWarning(aSS.str().c_str());
            }
            break;
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromCenterToOuter:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromCenterToInner:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToCenter:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToOuter:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromOuterToInner:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToCenter:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToOuter:
            case XCAFDimTolObjects_DimensionType_Location_LinearDistance_FromInnerToInner:
              anArray.ChangeFirst() *= theScaleFactor;
              aRescaleOtherValues = Standard_True;
              break;
            case XCAFDimTolObjects_DimensionType_Location_Angular:
            case XCAFDimTolObjects_DimensionType_Size_Angular:
              break;
            case XCAFDimTolObjects_DimensionType_Location_Oriented:
            case XCAFDimTolObjects_DimensionType_Location_WithPath: {
              Standard_SStream aSS;
              aSS << "Dimension PMI " << anEntryDimension << " is not scaled.";
              Message1::SendWarning(aSS.str().c_str());
            }
            break;
            case XCAFDimTolObjects_DimensionType_Size_CurveLength:
            case XCAFDimTolObjects_DimensionType_Size_Diameter:
            case XCAFDimTolObjects_DimensionType_Size_SphericalDiameter:
            case XCAFDimTolObjects_DimensionType_Size_Radius:
            case XCAFDimTolObjects_DimensionType_Size_SphericalRadius:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius:
            case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius:
            case XCAFDimTolObjects_DimensionType_Size_Thickness:
            case XCAFDimTolObjects_DimensionType_Size_WithPath:
              anArray.ChangeFirst() *= theScaleFactor;
              aRescaleOtherValues = Standard_True;
              break;
            case XCAFDimTolObjects_DimensionType_CommonLabel:
            case XCAFDimTolObjects_DimensionType_DimensionPresentation: {
              Standard_SStream aSS;
              aSS << "Dimension PMI " << anEntryDimension << " is not scaled.";
              Message1::SendWarning(aSS.str().c_str());
            }
            break;
            default: {
              Standard_SStream aSS;
              aSS << "Dimension PMI of unsupported type " << anEntryDimension << " is not scaled.";
              Message1::SendWarning(aSS.str().c_str());
            }
          }
          rescaleDimensionRefLabels(aShapeLFirst, aBRepTrsf, aG, anEntryDimension);
          rescaleDimensionRefLabels(aShapeLSecond, aBRepTrsf, aG, anEntryDimension);
          if (aRescaleOtherValues)
          {
            for (Standard_Integer i = anArray.Lower() + 1; i <= anArray.Upper(); ++i)
              anArray.ChangeValue(i) *= theScaleFactor;

            Handle(TCollection_HAsciiString) aName = aDimObj->GetSemanticName();
            if (!aName.IsNull())
            {
              aName->AssignCat(" (Rescaled to ");
              Standard_SStream aSS;
              aSS << aValues->First();
              aName->AssignCat(aSS.str().c_str());
              aName->AssignCat(")");
            }
          }
        }
        else
        {
          Standard_SStream aSS;
          aSS << "Dimension PMI values " << anEntryDimension << " are not scaled.";
          Message1::SendWarning(aSS.str().c_str());
        }

        aDimAttr->SetObject(aDimObj);
      }
    }

    TDF_LabelSequence aDatums;
    aDimTolTool->GetDatumLabels(aDatums);
    for (TDF_LabelSequence::Iterator anIt(aDatums); anIt.More(); anIt.Next())
    {
      const DataLabel& aDatum = anIt.Value();

      AsciiString1 anEntryDatum;
      Tool3::Entry(aDatum, anEntryDatum);

      Handle(XCAFDoc_Datum) aDatumAttr;
      if (aDatum.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttr))
      {
        Handle(XCAFDimTolObjects_DatumObject) aDatumObj = aDatumAttr->GetObject();

        if (aDatumObj->HasDatumTargetParams())
        {
          Frame3d anAxis = aDatumObj->GetDatumTargetAxis();
          anAxis.SetLocation(anAxis.Location().XYZ() * theScaleFactor);
          aDatumObj->SetDatumTargetAxis(anAxis);
          // TODO: Should we rescale target length and width?
          Standard_SStream aSS;
          aSS << "Datum PMI target length and width " << anEntryDatum << " are not scaled.";
          Message1::SendWarning(aSS.str().c_str());
          // aDatumObj->SetDatumTargetLength(aDatumObj->GetDatumTargetLength() * theScaleFactor);
          // aDatumObj->SetDatumTargetWidth(aDatumObj->GetDatumTargetWidth() * theScaleFactor);
        }

        if (aDatumObj->HasPointText())
        {
          aDatumObj->SetPointTextAttach(aDatumObj->GetPointTextAttach().XYZ() * theScaleFactor);
        }

        if (aDatumObj->HasPoint())
        {
          aDatumObj->SetPoint(aDatumObj->GetPoint().XYZ() * theScaleFactor);
        }

        if (aDatumObj->HasPlane())
        {
          Frame3d aPln = aDatumObj->GetPlane();
          aPln.SetLocation(aPln.Location().XYZ() * theScaleFactor);
          aDatumObj->SetPlane(aPln);
        }

        aDatumAttr->SetObject(aDatumObj);
      }
    }

    TDF_LabelSequence aDimTols;
    aDimTolTool->GetDimTolLabels(aDimTols);
    for (TDF_LabelSequence::Iterator anIt(aDimTols); anIt.More(); anIt.Next())
    {
      const DataLabel& aDimTol = anIt.Value();

      AsciiString1 anEntryDimTol;
      Tool3::Entry(aDimTol, anEntryDimTol);

      Handle(XCAFDoc_DimTol) aDimTolAttr;
      if (aDimTol.FindAttribute(XCAFDoc_DimTol::GetID(), aDimTolAttr))
      {
        Standard_SStream aSS;
        aSS << "DimTol PMI " << anEntryDimTol << " is not scaled.";
        Message1::SendWarning(aSS.str().c_str());
      }
    }
  }

  Handle(XCAFDoc_NotesTool) aNotesTool = XCAFDoc_DocumentTool::NotesTool(theLabel);
  if (!aNotesTool.IsNull())
  {
    TDF_LabelSequence aNotes;
    aNotesTool->GetNotes(aNotes);
    for (TDF_LabelSequence::Iterator anIt(aNotes); anIt.More(); anIt.Next())
    {
      const DataLabel& aNote = anIt.Value();

      Handle(XCAFDoc_Note) aNoteAttr;
      if (aNote.FindAttribute(XCAFDoc_NoteComment::GetID(), aNoteAttr)
          || aNote.FindAttribute(XCAFDoc_NoteBalloon::GetID(), aNoteAttr)
          || aNote.FindAttribute(XCAFDoc_NoteBinData::GetID(), aNoteAttr))
      {
        Handle(XCAFNoteObjects_NoteObject) aNoteObj = aNoteAttr->GetObject();

        if (aNoteObj->HasPointText())
        {
          aNoteObj->SetPointText(aNoteObj->GetPointText().XYZ() * theScaleFactor);
        }

        if (aNoteObj->HasPoint())
        {
          aNoteObj->SetPoint(aNoteObj->GetPoint().XYZ() * theScaleFactor);
        }

        if (aNoteObj->HasPlane())
        {
          Frame3d aPln = aNoteObj->GetPlane();
          aPln.SetLocation(aPln.Location().XYZ() * theScaleFactor);
          aNoteObj->SetPlane(aPln);
        }

        aNoteAttr->SetObject(aNoteObj);
      }
    }
  }

  return anIsDone;
}
