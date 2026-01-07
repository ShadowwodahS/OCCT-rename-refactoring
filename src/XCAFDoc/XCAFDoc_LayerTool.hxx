// Created on: 2000-09-26
// Created by: Pavel TELKOV.
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

#ifndef _XCAFDoc_LayerTool_HeaderFile
#define _XCAFDoc_LayerTool_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TDataStd_GenericEmpty.hxx>
#include <TDF_LabelSequence.hxx>
#include <TColStd_HSequenceOfExtendedString.hxx>
class XCAFDoc_ShapeTool;
class DataLabel;
class Standard_GUID;
class UtfString;
class TopoShape;

class XCAFDoc_LayerTool;
DEFINE_STANDARD_HANDLE(XCAFDoc_LayerTool, TDataStd_GenericEmpty)

//! Provides tools to store and retrieve attributes (Layers)
//! of TopoShape in and from AppDocument
//! A Document is intended to hold different
//! attributes of ONE shape and it's sub-shapes
//! Provide tools for management of Layers section of document.
class XCAFDoc_LayerTool : public TDataStd_GenericEmpty
{

public:
  Standard_EXPORT XCAFDoc_LayerTool();

  //! Creates (if not exist) LayerTool.
  Standard_EXPORT static Handle(XCAFDoc_LayerTool) Set(const DataLabel& L);

  Standard_EXPORT static const Standard_GUID& GetID();

  //! returns the label under which Layers are stored
  Standard_EXPORT DataLabel BaseLabel() const;

  //! Returns internal XCAFDoc_ShapeTool tool
  Standard_EXPORT const Handle(XCAFDoc_ShapeTool)& ShapeTool();

  //! Returns True if label belongs to a Layertable and
  //! is a Layer definition
  Standard_EXPORT Standard_Boolean IsLayer(const DataLabel& lab) const;

  //! Returns Layer defined by label lab
  //! Returns False if the label is not in Layertable
  //! or does not define a Layer
  Standard_EXPORT Standard_Boolean GetLayer(const DataLabel&            lab,
                                            UtfString& aLayer) const;

  //! Finds a Layer definition in a Layertable and returns
  //! its label if found
  //! Returns False if Layer is not found in Layertable
  Standard_EXPORT Standard_Boolean FindLayer(const UtfString& aLayer,
                                             DataLabel&                        lab) const;

  //! Finds a Layer definition in a Layertable by name
  //! Returns first founded label with the same name if <theToFindWithProperty> is false
  //! If <theToFindWithProperty> is true returns first label that
  //! contains or not contains visible attr, according to the <theToFindVisible> parameter
  Standard_EXPORT DataLabel
    FindLayer(const UtfString& aLayer,
              const Standard_Boolean            theToFindWithProperty = Standard_False,
              const Standard_Boolean            theToFindVisible      = Standard_True) const;

  //! Adds a Layer definition to a Layertable and returns
  //! its label (returns existing label if the same Layer
  //! is already defined)
  Standard_EXPORT DataLabel AddLayer(const UtfString& theLayer) const;

  //! Adds a Layer definition to a Layertable and returns its label
  //! Returns existing label (if it is already defined)
  //! of visible or invisible layer, according to <theToFindVisible> parameter
  Standard_EXPORT DataLabel AddLayer(const UtfString& theLayer,
                                     const Standard_Boolean            theToFindVisible) const;

  //! Removes Layer from the Layertable
  Standard_EXPORT void RemoveLayer(const DataLabel& lab) const;

  //! Returns a sequence of Layers currently stored
  //! in the Layertable
  Standard_EXPORT void GetLayerLabels(TDF_LabelSequence& Labels) const;

  //! Sets a link from label <L> to Layer
  //! defined by <LayerL>
  //! optional parameter <shapeInOneLayer> show could shape be
  //! in number of layers or only in one.
  Standard_EXPORT void SetLayer(const DataLabel&       L,
                                const DataLabel&       LayerL,
                                const Standard_Boolean shapeInOneLayer = Standard_False) const;

  //! Sets a link from label <L> to Layer <aLayer>
  //! in the Layertable
  //! Adds a Layer as necessary
  //! optional parameter <shapeInOneLayer> show could shape be
  //! in number of layers or only in one.
  Standard_EXPORT void SetLayer(const DataLabel&                  L,
                                const UtfString& aLayer,
                                const Standard_Boolean shapeInOneLayer = Standard_False) const;

  //! Removes a link from label <L> to all layers
  Standard_EXPORT void UnSetLayers(const DataLabel& L) const;

  //! Remove link from label <L> and Layer <aLayer>.
  //! returns FALSE if no such layer.
  Standard_EXPORT Standard_Boolean UnSetOneLayer(const DataLabel&                  L,
                                                 const UtfString& aLayer) const;

  //! Remove link from label <L> and Layer <aLayerL>.
  //! returns FALSE if <aLayerL> is not a layer label.
  Standard_EXPORT Standard_Boolean UnSetOneLayer(const DataLabel& L,
                                                 const DataLabel& aLayerL) const;

  //! Returns True if label <L> has a Layer associated
  //! with the <aLayer>.
  Standard_EXPORT Standard_Boolean IsSet(const DataLabel&                  L,
                                         const UtfString& aLayer) const;

  //! Returns True if label <L> has a Layer associated
  //! with the <aLayerL> label.
  Standard_EXPORT Standard_Boolean IsSet(const DataLabel& L, const DataLabel& aLayerL) const;

  //! Return sequence of strings <aLayerS> that associated with label <L>.
  Standard_EXPORT Standard_Boolean GetLayers(const DataLabel&                           L,
                                             Handle(TColStd_HSequenceOfExtendedString)& aLayerS);

  //! Return sequence of labels <aLayerSL> that associated with label <L>.
  Standard_EXPORT Standard_Boolean GetLayers(const DataLabel& L, TDF_LabelSequence& aLayerLS);

  //! Return sequence of strings that associated with label <L>.
  Standard_EXPORT Handle(TColStd_HSequenceOfExtendedString) GetLayers(const DataLabel& L);

  //! Return sequanese of shape labels that assigned with layers to <ShLabels>.
  Standard_EXPORT static void GetShapesOfLayer(const DataLabel&   theLayerL,
                                               TDF_LabelSequence& theShLabels);

  //! Return TRUE if layer is visible, FALSE if invisible.
  Standard_EXPORT Standard_Boolean IsVisible(const DataLabel& layerL) const;

  //! Set the visibility of layer. If layer is invisible when on it's layer
  //! will set UAttribute with corresponding GUID.
  Standard_EXPORT void SetVisibility(const DataLabel&       layerL,
                                     const Standard_Boolean isvisible = Standard_True) const;

  //! Sets a link from label that containing shape <Sh>
  //! with layer that situated at label <LayerL>.
  //! optional parameter <shapeInOneLayer> show could shape be
  //! in number of layers or only in one.
  //! return FALSE if no such shape <Sh> or label <LayerL>
  Standard_EXPORT Standard_Boolean
    SetLayer(const TopoShape&    Sh,
             const DataLabel&       LayerL,
             const Standard_Boolean shapeInOneLayer = Standard_False);

  //! Sets a link from label that containing shape <Sh>
  //! with layer <aLayer>. Add <aLayer> to LayerTable if nessesery.
  //! optional parameter <shapeInOneLayer> show could shape be
  //! in number of layers or only in one.
  //! return FALSE if no such shape <Sh>.
  Standard_EXPORT Standard_Boolean
    SetLayer(const TopoShape&               Sh,
             const UtfString& aLayer,
             const Standard_Boolean            shapeInOneLayer = Standard_False);

  //! Remove link between shape <Sh> and all Layers at LayerTable.
  //! return FALSE if no such shape <Sh> in XCAF Document.
  Standard_EXPORT Standard_Boolean UnSetLayers(const TopoShape& Sh);

  //! Remove link between shape <Sh> and layer <aLayer>.
  //! returns FALSE if no such layer <aLayer> or shape <Sh>.
  Standard_EXPORT Standard_Boolean UnSetOneLayer(const TopoShape&               Sh,
                                                 const UtfString& aLayer);

  //! Remove link between shape <Sh> and layer <aLayerL>.
  //! returns FALSE if no such layer <aLayerL> or shape <Sh>.
  Standard_EXPORT Standard_Boolean UnSetOneLayer(const TopoShape& Sh, const DataLabel& aLayerL);

  //! Returns True if shape <Sh> has a Layer associated
  //! with the <aLayer>.
  Standard_EXPORT Standard_Boolean IsSet(const TopoShape&               Sh,
                                         const UtfString& aLayer);

  //! Returns True if shape <Sh> has a Layer associated
  //! with the <aLayerL>.
  Standard_EXPORT Standard_Boolean IsSet(const TopoShape& Sh, const DataLabel& aLayerL);

  //! Return sequence of strings <aLayerS> that associated with shape <Sh>.
  Standard_EXPORT Standard_Boolean GetLayers(const TopoShape&                        Sh,
                                             Handle(TColStd_HSequenceOfExtendedString)& aLayerS);

  //! Return sequence of labels <aLayerLS> that associated with shape <Sh>.
  Standard_EXPORT Standard_Boolean GetLayers(const TopoShape& Sh, TDF_LabelSequence& aLayerLS);

  //! Return sequence of strings that associated with shape <Sh>.
  Standard_EXPORT Handle(TColStd_HSequenceOfExtendedString) GetLayers(const TopoShape& Sh);

  Standard_EXPORT const Standard_GUID& ID() const Standard_OVERRIDE;

  //! Dumps the content of me into the stream
  Standard_EXPORT virtual void DumpJson(Standard_OStream& theOStream,
                                        Standard_Integer  theDepth = -1) const Standard_OVERRIDE;

  DEFINE_DERIVED_ATTRIBUTE(XCAFDoc_LayerTool, TDataStd_GenericEmpty)

private:
  Handle(XCAFDoc_ShapeTool) myShapeTool;
};

#endif // _XCAFDoc_LayerTool_HeaderFile
