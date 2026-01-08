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

#include <DEPLY_Provider.hxx>

#include <BRep_Builder.hxx>
#include <DEPLY_ConfigurationNode.hxx>
#include <DE_Wrapper.hxx>
#include <Message.hxx>
#include <RWMesh_FaceIterator.hxx>
#include <RWPly_CafWriter.hxx>
#include <RWPly_PlyWriterContext.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFPrs_DocumentExplorer.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DEPLY_Provider, DE_Provider)

//=================================================================================================

DEPLY_Provider::DEPLY_Provider() {}

//=================================================================================================

DEPLY_Provider::DEPLY_Provider(const Handle(DE_ConfigurationNode)& theNode)
    : DE_Provider(theNode)
{
}

//=================================================================================================

bool DEPLY_Provider::Write(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           Handle(ExchangeSession)&  theWS,
                           const Message_ProgressRange&    theProgress)
{
  (void)theWS;
  return Write(thePath, theDocument, theProgress);
}

//=================================================================================================

bool DEPLY_Provider::Write(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           const Message_ProgressRange&    theProgress)
{
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DEPLY_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEPLY_Provider during writing the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEPLY_ConfigurationNode) aNode = Handle(DEPLY_ConfigurationNode)::DownCast(GetNode());

  TDF_LabelSequence         aRootLabels;
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(theDocument->Main());
  aShapeTool->GetFreeShapes(aRootLabels);
  if (aRootLabels.IsEmpty())
  {
    return Standard_True;
  }

  TColStd_IndexedDataMapOfStringString aFileInfo;
  if (!aNode->InternalParameters.WriteAuthor.IsEmpty())
  {
    aFileInfo.Add("Author", aNode->InternalParameters.WriteAuthor);
  }
  if (!aNode->InternalParameters.WriteComment.IsEmpty())
  {
    aFileInfo.Add("Comments", aNode->InternalParameters.WriteComment);
  }
  RWMesh_CoordinateSystemConverter aConverter;
  Standard_Real                    aScaleFactorM = 1.;
  if (XCAFDoc_DocumentTool::GetLengthUnit(theDocument, aScaleFactorM))
  {
    aConverter.SetInputLengthUnit(aScaleFactorM);
  }
  else
  {
    aConverter.SetInputLengthUnit(aNode->GlobalParameters.SystemUnit / 1000.);
    Message1::SendWarning()
      << "Warning in the DEPLY_Provider during writing the file " << thePath
      << "\t: The document has no information on Units2. Using global parameter as initial Unit.";
  }
  aConverter.SetInputCoordinateSystem(aNode->InternalParameters.SystemCS);
  aConverter.SetOutputLengthUnit(aNode->GlobalParameters.LengthUnit / 1000.);
  aConverter.SetOutputCoordinateSystem(aNode->InternalParameters.FileCS);

  RWPly_CafWriter aPlyCtx(thePath);
  aPlyCtx.SetNormals(aNode->InternalParameters.WriteNormals);
  aPlyCtx.SetColors(aNode->InternalParameters.WriteColors);
  aPlyCtx.SetTexCoords(aNode->InternalParameters.WriteTexCoords);
  aPlyCtx.SetPartId(aNode->InternalParameters.WritePartId);
  aPlyCtx.SetFaceId(aNode->InternalParameters.WriteFaceId);
  if (!aPlyCtx.Perform(theDocument, aFileInfo, theProgress))
  {
    Message1::SendFail() << "Error in the DEPLY_Provider during writing the file " << thePath
                        << "\t: Cannot perform the document";
    return false;
  }

  return true;
}

//=================================================================================================

bool DEPLY_Provider::Write(const AsciiString1& thePath,
                           const TopoShape&            theShape,
                           Handle(ExchangeSession)& theWS,
                           const Message_ProgressRange&   theProgress)
{
  (void)theWS;
  return Write(thePath, theShape, theProgress);
}

//=================================================================================================

bool DEPLY_Provider::Write(const AsciiString1& thePath,
                           const TopoShape&            theShape,
                           const Message_ProgressRange&   theProgress)
{
  Handle(AppDocument)  aDoc    = new AppDocument("BinXCAF");
  Handle(XCAFDoc_ShapeTool) aShTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  aShTool->AddShape(theShape);
  return Write(thePath, aDoc, theProgress);
}

//=================================================================================================

AsciiString1 DEPLY_Provider::GetFormat() const
{
  return AsciiString1("PLY");
}

//=================================================================================================

AsciiString1 DEPLY_Provider::GetVendor() const
{
  return AsciiString1("OCC");
}
