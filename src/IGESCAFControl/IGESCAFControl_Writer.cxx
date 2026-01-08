// Created on: 2000-08-17
// Created by: Andrey BETENEV
// Copyright (c) 2000-2014 OPEN CASCADE SAS
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

#include <IGESCAFControl.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <IGESData_IGESModel.hxx>
#include <IGESGraph_Color.hxx>
#include <IGESGraph_DefinitionLevel.hxx>
#include <IGESSolid_Face.hxx>
#include <IGESBasic_Name.hxx>
#include <Message_ProgressScope.hxx>
#include <NCollection_DataMap.hxx>
#include <Standard_Transient.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TCollection_HExtendedString.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDocStd_Document.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shape.hxx>
#include <Transfer_TransientListBinder.hxx>
#include <TransferBRep.hxx>
#include <TransferBRep_ShapeMapper.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_LengthUnit.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs.hxx>
#include <XCAFPrs_Style.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_ShapeProcessor.hxx>
#include <XSControl_WorkSession.hxx>
#include <UnitsMethods.hxx>

namespace
{
typedef NCollection_DataMap<TopoShape, UtfString> DataMapOfShapeNames;

void CollectShapeNames(const DataLabel&             theLabel,
                       const TopLoc_Location&       theLocation,
                       const Handle(NameAttribute)& thePrevName,
                       DataMapOfShapeNames&         theMapOfShapeNames)
{
  Standard_Boolean hasReferredShape = Standard_False;
  Standard_Boolean hasComponents    = Standard_False;
  DataLabel        aReferredLabel;

  Handle(NameAttribute) aName;
  theLabel.FindAttribute(NameAttribute::GetID(), aName);

  if (XCAFDoc_ShapeTool::GetReferredShape(theLabel, aReferredLabel))
  {
    TopLoc_Location aSubLocation = theLocation.Multiplied(XCAFDoc_ShapeTool::GetLocation(theLabel));
    CollectShapeNames(aReferredLabel, aSubLocation, aName, theMapOfShapeNames);
    hasReferredShape = Standard_True;
  }

  TDF_LabelSequence aSeq;
  if (XCAFDoc_ShapeTool::GetComponents(theLabel, aSeq))
  {
    for (Standard_Integer anIter = 1; anIter <= aSeq.Length(); anIter++)
    {
      CollectShapeNames(aSeq.Value(anIter), theLocation, aName, theMapOfShapeNames);
    }
    hasComponents = Standard_True;
  }

  aSeq.Clear();
  if (XCAFDoc_ShapeTool::GetSubShapes(theLabel, aSeq))
  {
    for (Standard_Integer anIter = 1; anIter <= aSeq.Length(); anIter++)
    {
      TopoShape aShape;
      if (!XCAFDoc_ShapeTool::GetShape(aSeq.Value(anIter), aShape))
        continue;
      if (!aSeq.Value(anIter).FindAttribute(NameAttribute::GetID(), aName))
        continue;
      theMapOfShapeNames.Bind(aShape, aName->Get());
    }
  }

  if (!hasReferredShape && !hasComponents && !thePrevName.IsNull())
  {
    TopoShape aShape;
    if (!XCAFDoc_ShapeTool::GetShape(theLabel, aShape))
      return;
    aShape.Move(theLocation, Standard_False);
    theMapOfShapeNames.Bind(aShape, thePrevName->Get());
  }
}
} // namespace

//=================================================================================================

IGESCAFControl_Writer::IGESCAFControl_Writer()
    : myColorMode(Standard_True),
      myNameMode(Standard_True),
      myLayerMode(Standard_True)
{
}

//=================================================================================================

IGESCAFControl_Writer::IGESCAFControl_Writer(const Handle(ExchangeSession)& WS,
                                             const Standard_Boolean /*scratch*/)
{
  // this code does things in a wrong way, it should be vice-versa
  WS->SetModel(Model());
  WS->SetMapWriter(TransferProcess());
  myColorMode = Standard_True;
  myNameMode  = Standard_True;
  myLayerMode = Standard_True;

  //  SetWS (WS,scratch); // this should be the only required command here
}

//=================================================================================================

IGESCAFControl_Writer::IGESCAFControl_Writer(const Handle(ExchangeSession)& WS,
                                             const Standard_CString               theUnit)
    : IgesFileWriter(theUnit)
{

  WS->SetModel(Model());
  WS->SetMapWriter(TransferProcess());
  myColorMode = Standard_True;
  myNameMode  = Standard_True;
  myLayerMode = Standard_True;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::Transfer(const Handle(AppDocument)& doc,
                                                 const Message_ProgressRange&    theProgress)
{
  // translate free top-level shapes of the DECAF document
  Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
  if (STool.IsNull())
    return Standard_False;

  TDF_LabelSequence labels;
  STool->GetFreeShapes(labels);
  return Transfer(labels, theProgress);
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::Transfer(const DataLabel&             label,
                                                 const Message_ProgressRange& theProgress)
{
  TDF_LabelSequence labels;
  labels.Append(label);
  return Transfer(labels, theProgress);
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::Transfer(const TDF_LabelSequence&     labels,
                                                 const Message_ProgressRange& theProgress)
{
  if (labels.Length() <= 0)
    return Standard_False;
  prepareUnit(labels.First()); // set local length unit to the model
  Message_ProgressScope aPS(theProgress, "Labels", labels.Length());
  for (Standard_Integer i = 1; i <= labels.Length() && aPS.More(); i++)
  {
    TopoShape shape = XCAFDoc_ShapeTool::GetShape(labels.Value(i));
    if (!shape.IsNull())
      AddShape(shape, aPS.Next());
    //      IgesFileWriter::Transfer ( shape );
  }

  // write colors
  if (GetColorMode())
    WriteAttributes(labels);

  // write layers
  if (GetLayerMode())
    WriteLayers(labels);

  // write names
  if (GetNameMode())
    WriteNames(labels);

  // refresh graph
  //  WS()->ComputeGraph ( Standard_True );
  ComputeModel();

  return Standard_True;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::Perform(const Handle(AppDocument)& doc,
                                                const Standard_CString          filename,
                                                const Message_ProgressRange&    theProgress)
{
  if (!Transfer(doc, theProgress))
    return Standard_False;
  return Write(filename) == IFSelect_RetDone;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::Perform(const Handle(AppDocument)& doc,
                                                const AsciiString1&  filename,
                                                const Message_ProgressRange&    theProgress)
{
  if (!Transfer(doc, theProgress))
    return Standard_False;
  return Write(filename.ToCString()) == IFSelect_RetDone;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::WriteAttributes(const TDF_LabelSequence& labels)
{
  // Iterate on labels
  if (labels.Length() <= 0)
    return Standard_False;
  for (Standard_Integer i = 1; i <= labels.Length(); i++)
  {
    DataLabel L = labels.Value(i);

    // collect color settings
    XCAFPrs_IndexedDataMapOfShapeStyle settings;
    TopLoc_Location                    loc;
    XCAFPrs1::CollectStyleSettings(L, loc, settings);
    if (settings.Extent() <= 0)
      continue;

    // get a target shape and try to find corresponding context
    // (all the colors set under that label will be put into that context)
    TopoShape S;
    if (!XCAFDoc_ShapeTool::GetShape(L, S))
      continue;

    // iterate on subshapes and create IGES styles
    XCAFPrs_DataMapOfStyleTransient colors;
    TopTools_MapOfShape             Map;
    const XCAFPrs_Style             inherit;
    MakeColors(S, settings, colors, Map, inherit);
  }

  return Standard_True;
}

//=================================================================================================

void IGESCAFControl_Writer::MakeColors(const TopoShape&                       S,
                                       const XCAFPrs_IndexedDataMapOfShapeStyle& settings,
                                       XCAFPrs_DataMapOfStyleTransient&          colors,
                                       TopTools_MapOfShape&                      Map,
                                       const XCAFPrs_Style&                      inherit)
{
  // skip already processed shapes
  if (!Map.Add(S))
    return;

  // check if shape has its own style (or inherits from ancestor)
  XCAFPrs_Style style = inherit;
  if (settings.Contains(S))
  {
    const XCAFPrs_Style& own = settings.FindFromKey(S);
    if (own.IsSetColorCurv())
      style.SetColorCurv(own.GetColorCurv());
    if (own.IsSetColorSurf())
      style.SetColorSurf(own.GetColorSurf());
    style.SetMaterial(own.Material());
  }

  // analyze whether current entity should get a color
  Standard_Boolean hasColor = Standard_False;
  Color1   col;
  if (S.ShapeType() == TopAbs_FACE || S.ShapeType() == TopAbs_SOLID)
  {
    if (style.IsSetColorSurf())
    {
      hasColor = Standard_True;
      col      = style.GetColorSurf();
    }
    else if (!style.Material().IsNull() && !style.Material()->IsEmpty())
    {
      hasColor = Standard_True;
      col      = style.Material()->BaseColor().GetRGB();
    }
  }
  else if (S.ShapeType() == TopAbs_EDGE || S.ShapeType() == TopAbs_WIRE)
  {
    if (style.IsSetColorCurv())
    {
      hasColor = Standard_True;
      col      = style.GetColorCurv();
    }
  }

  // if color has to be assigned, try to do this
  if (hasColor)
  {
    Handle(IGESGraph_Color) colent;
    Standard_Integer        rank = IGESCAFControl1::EncodeColor(col);
    if (!rank)
    {
      XCAFPrs_Style c; // style used as key in the map
      c.SetColorSurf(col);
      if (colors.IsBound(c))
      {
        colent = Handle(IGESGraph_Color)::DownCast(colors.Find(c));
      }
      else
      {
        Handle(TCollection_HAsciiString) str =
          new TCollection_HAsciiString(col.StringName(col.Name()));
        colent = new IGESGraph_Color;
        NCollection_Vec3<Standard_Real> aColor_sRGB;
        col.Values(aColor_sRGB.r(), aColor_sRGB.g(), aColor_sRGB.b(), Quantity_TOC_sRGB);
        colent->Init(aColor_sRGB.r() * 100., aColor_sRGB.g() * 100., aColor_sRGB.b() * 100., str);
        AddEntity(colent);
        colors.Bind(c, colent);
      }
    }
    Handle(Transfer_FinderProcess)   FP = TransferProcess();
    Handle(IGESData_IGESEntity)      ent;
    Handle(TransferBRep_ShapeMapper) mapper = TransferBRep1::ShapeMapper(FP, S);
    Handle(TransferBRep_ShapeMapper) aNoLocMapper =
      TransferBRep1::ShapeMapper(FP, S.Located(TopLoc_Location()));
    if (FP->FindTypedTransient(mapper, STANDARD_TYPE(IGESData_IGESEntity), ent)
        || FP->FindTypedTransient(aNoLocMapper, STANDARD_TYPE(IGESData_IGESEntity), ent))
    {
      ent->InitColor(colent, rank);
      Handle(IGESSolid_Face) ent_f = Handle(IGESSolid_Face)::DownCast(ent);
      if (!ent_f.IsNull())
      {
        if (!ent_f->Surface().IsNull())
          ent_f->Surface()->InitColor(colent, rank);
      }
    }
    else
    {
      // may be S was split during shape process
      Handle(Transfer_Binder) bnd = FP->Find(mapper);
      if (!bnd.IsNull())
      {
        Handle(Transfer_TransientListBinder) TransientListBinder =
          // Handle(Transfer_TransientListBinder)::DownCast( bnd->Next(Standard_True) );
          Handle(Transfer_TransientListBinder)::DownCast(bnd);
        Standard_Integer i = 0, nb = 0;
        if (!TransientListBinder.IsNull())
        {
          nb = TransientListBinder->NbTransients();
          for (i = 1; i <= nb; i++)
          {
            Handle(RefObject) t = TransientListBinder->Transient(i);
            ent                          = Handle(IGESData_IGESEntity)::DownCast(t);
            if (!ent.IsNull())
            {
              ent->InitColor(colent, rank);
              Handle(IGESSolid_Face) ent_f = Handle(IGESSolid_Face)::DownCast(ent);
              if (!ent_f.IsNull())
              {
                if (!ent_f->Surface().IsNull())
                  ent_f->Surface()->InitColor(colent, rank);
              }
            }
          }
        }
        /* // alternative: consider recursive mapping S -> compound -> entities
        else {
          TopoShape comp = TransferBRep1::ShapeResult(bnd);
          if ( ! comp.IsNull() && comp.ShapeType() < S.ShapeType() )
            for ( TopoDS_Iterator it(comp); it.More(); it.Next() ) {
              MakeColors ( it.Value(), settings, colors, Map, style );
            }
        }
        */
      }
    }
  }

  // iterate on subshapes (except vertices :)
  if (S.ShapeType() == TopAbs_EDGE)
    return;
  for (TopoDS_Iterator it(S); it.More(); it.Next())
  {
    MakeColors(it.Value(), settings, colors, Map, style);
  }
}

static void AttachLayer(const Handle(Transfer_FinderProcess)& FP,
                        const Handle(XCAFDoc_LayerTool)&      LTool,
                        const TopoShape&                   aSh,
                        const Standard_Integer                localIntName)
{

  TopTools_SequenceOfShape shseq;
  if (aSh.ShapeType() == TopAbs_COMPOUND)
  {
    TopoDS_Iterator aShIt(aSh);
    for (; aShIt.More(); aShIt.Next())
    {
      const TopoShape&                       newSh    = aShIt.Value();
      Handle(TColStd_HSequenceOfExtendedString) shLayers = new TColStd_HSequenceOfExtendedString;
      if (!LTool->GetLayers(newSh, shLayers) || newSh.ShapeType() == TopAbs_COMPOUND)
        AttachLayer(FP, LTool, newSh, localIntName);
    }
    return;
  }
  else if (aSh.ShapeType() == TopAbs_SOLID || aSh.ShapeType() == TopAbs_SHELL)
  {
    for (ShapeExplorer exp(aSh, TopAbs_FACE); exp.More(); exp.Next())
    {
      TopoFace entSh = TopoDS::Face(exp.Current());
      shseq.Append(entSh);
    }
  }
  else
  {
    shseq.Append(aSh);
  }

  for (Standard_Integer i = 1; i <= shseq.Length(); i++)
  {
    const TopoShape&              localShape = shseq.Value(i);
    Handle(IGESData_IGESEntity)      Igesent;
    Handle(TransferBRep_ShapeMapper) mapper = TransferBRep1::ShapeMapper(FP, localShape);
    if (FP->FindTypedTransient(mapper, STANDARD_TYPE(IGESData_IGESEntity), Igesent))
    {
      Igesent->InitLevel(0, localIntName);
    }
#ifdef OCCT_DEBUG
    else
      std::cout << "Warning: Can't find entity for shape in mapper" << std::endl;
#endif
  }
}

static void MakeLayers(const Handle(Transfer_FinderProcess)& FP,
                       const Handle(XCAFDoc_ShapeTool)&      STool,
                       const Handle(XCAFDoc_LayerTool)&      LTool,
                       const TDF_LabelSequence&              aShapeLabels,
                       const Standard_Integer                localIntName)
{
  for (Standard_Integer j = 1; j <= aShapeLabels.Length(); j++)
  {
    DataLabel    aShapeLabel = aShapeLabels.Value(j);
    TopoShape aSh;
    if (!STool->GetShape(aShapeLabel, aSh))
      continue;
    AttachLayer(FP, LTool, aSh, localIntName);
  }
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::WriteLayers(const TDF_LabelSequence& labels)
{
  if (labels.Length() <= 0)
    return Standard_False;
  Handle(XCAFDoc_ShapeTool) STool = XCAFDoc_DocumentTool::ShapeTool(labels(1));
  if (STool.IsNull())
    return Standard_False;
  Handle(XCAFDoc_LayerTool) LTool = XCAFDoc_DocumentTool::LayerTool(labels(1));
  if (LTool.IsNull())
    return Standard_False;

  Standard_Integer  globalIntName = 0;
  TDF_LabelSequence aLayerLabels;
  LTool->GetLayerLabels(aLayerLabels);

  Handle(Transfer_FinderProcess) FP = TransferProcess();
  for (Standard_Integer i = 1; i <= aLayerLabels.Length(); i++)
  {
    DataLabel aOneLayerL = aLayerLabels.Value(i);
    if (aOneLayerL.IsNull())
      continue;
    UtfString localName;
    LTool->GetLayer(aOneLayerL, localName);
    Standard_Integer        localIntName = 0;
    AsciiString1 asciiName(localName, '?');
    if (asciiName.IsIntegerValue())
    {
      localIntName = asciiName.IntegerValue();
      if (globalIntName < localIntName)
        globalIntName = localIntName;

      TDF_LabelSequence aShapeLabels;
      LTool->GetShapesOfLayer(aOneLayerL, aShapeLabels);
      if (aShapeLabels.Length() <= 0)
        continue;
      MakeLayers(FP, STool, LTool, aShapeLabels, localIntName);
    }
  }

  for (Standard_Integer i1 = 1; i1 <= aLayerLabels.Length(); i1++)
  {
    DataLabel aOneLayerL = aLayerLabels.Value(i1);
    if (aOneLayerL.IsNull())
      continue;
    UtfString localName;
    LTool->GetLayer(aOneLayerL, localName);
    Standard_Integer        localIntName = 0;
    AsciiString1 asciiName(localName, '?');
    if (asciiName.IsIntegerValue())
      continue;
    TDF_LabelSequence aShapeLabels;
    LTool->GetShapesOfLayer(aOneLayerL, aShapeLabels);
    if (aShapeLabels.Length() <= 0)
      continue;
    localIntName = ++globalIntName;
    MakeLayers(FP, STool, LTool, aShapeLabels, localIntName);
  }

  return Standard_True;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::WriteNames(const TDF_LabelSequence& theLabels)
{
  if (theLabels.Length() <= 0)
    return Standard_False;

  DataMapOfShapeNames aMapOfShapeNames;

  for (Standard_Integer anIter = 1; anIter <= theLabels.Length(); anIter++)
  {
    DataLabel aLabel = theLabels.Value(anIter);

    TopoShape          aShape;
    Handle(NameAttribute) aName;
    if (!XCAFDoc_ShapeTool::GetShape(aLabel, aShape))
      continue;
    if (!aLabel.FindAttribute(NameAttribute::GetID(), aName))
      continue;

    aMapOfShapeNames.Bind(aShape, aName->Get());

    // Collect names for subshapes
    TopLoc_Location aLocation;
    CollectShapeNames(aLabel, aLocation, aName, aMapOfShapeNames);
  }

  for (DataMapOfShapeNames::Iterator anIter(aMapOfShapeNames); anIter.More(); anIter.Next())
  {
    const TopoShape&               aShape = anIter.Key1();
    const UtfString& aName  = anIter.Value();

    Handle(Transfer_FinderProcess)   aFinderProcess = TransferProcess();
    Handle(IGESData_IGESEntity)      anIGESEntity;
    Handle(TransferBRep_ShapeMapper) aShapeMapper =
      TransferBRep1::ShapeMapper(aFinderProcess, aShape);

    if (aFinderProcess->FindTypedTransient(aShapeMapper,
                                           STANDARD_TYPE(IGESData_IGESEntity),
                                           anIGESEntity))
    {
      Handle(TCollection_HAsciiString) anAsciiName = new TCollection_HAsciiString("        ");
      Standard_Integer                 aNameLength = 8 - aName.Length();
      if (aNameLength < 0)
        aNameLength = 0;
      for (Standard_Integer aCharPos = 1; aNameLength < 8; aCharPos++, aNameLength++)
      {
        anAsciiName->SetValue(
          aNameLength + 1,
          IsAnAscii(aName.Value(aCharPos)) ? (Standard_Character)aName.Value(aCharPos) : '?');
      }
      anIGESEntity->SetLabel(anAsciiName);

      // Set long IGES name using 406 form 15 entity
      Handle(IGESBasic_Name)              aLongNameEntity = new IGESBasic_Name;
      Handle(TCollection_HExtendedString) aTmpStr         = new TCollection_HExtendedString(aName);
      aLongNameEntity->Init(1, new TCollection_HAsciiString(aTmpStr, '_'));

      anIGESEntity->AddProperty(aLongNameEntity);
      AddEntity(aLongNameEntity);
    }
  }

  return Standard_True;
}

//=================================================================================================

void IGESCAFControl_Writer::prepareUnit(const DataLabel& theLabel)
{
  Handle(XCAFDoc_LengthUnit) aLengthAttr;
  if (!theLabel.IsNull() && theLabel.Root().FindAttribute(XCAFDoc_LengthUnit::GetID(), aLengthAttr))
  {
    Model()->ChangeGlobalSection().SetCascadeUnit(aLengthAttr->GetUnitValue() * 1000);
  }
  else
  {
    XSAlgo_ShapeProcessor::PrepareForTransfer(); // update unit info
    Model()->ChangeGlobalSection().SetCascadeUnit(UnitsMethods1::GetCasCadeLengthUnit());
  }
}

//=================================================================================================

void IGESCAFControl_Writer::SetColorMode(const Standard_Boolean colormode)
{
  myColorMode = colormode;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::GetColorMode() const
{
  return myColorMode;
}

//=================================================================================================

void IGESCAFControl_Writer::SetNameMode(const Standard_Boolean namemode)
{
  myNameMode = namemode;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::GetNameMode() const
{
  return myNameMode;
}

//=================================================================================================

void IGESCAFControl_Writer::SetLayerMode(const Standard_Boolean layermode)
{
  myLayerMode = layermode;
}

//=================================================================================================

Standard_Boolean IGESCAFControl_Writer::GetLayerMode() const
{
  return myLayerMode;
}
