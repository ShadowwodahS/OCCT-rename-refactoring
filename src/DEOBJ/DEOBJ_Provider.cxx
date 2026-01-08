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

#include <DEOBJ_Provider.hxx>

#include <BRep_Builder.hxx>
#include <DEOBJ_ConfigurationNode.hxx>
#include <RWObj_CafReader.hxx>
#include <RWObj_CafWriter.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DEOBJ_Provider, DE_Provider)

//=================================================================================================

DEOBJ_Provider::DEOBJ_Provider() {}

//=================================================================================================

DEOBJ_Provider::DEOBJ_Provider(const Handle(DE_ConfigurationNode)& theNode)
    : DE_Provider(theNode)
{
}

//=================================================================================================

bool DEOBJ_Provider::Read(const AsciiString1&  thePath,
                          const Handle(AppDocument)& theDocument,
                          Handle(ExchangeSession)&  theWS,
                          const Message_ProgressRange&    theProgress)
{
  (void)theWS;
  return Read(thePath, theDocument, theProgress);
}

//=================================================================================================

bool DEOBJ_Provider::Write(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           Handle(ExchangeSession)&  theWS,
                           const Message_ProgressRange&    theProgress)
{
  (void)theWS;
  return Write(thePath, theDocument, theProgress);
}

//=================================================================================================

bool DEOBJ_Provider::Read(const AsciiString1&  thePath,
                          const Handle(AppDocument)& theDocument,
                          const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    Message1::SendFail() << "Error in the DEOBJ_Provider during reading the file " << thePath
                        << "\t: theDocument shouldn't be null";
    return false;
  }
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DEOBJ_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEOBJ_ConfigurationNode during reading the file "
                        << thePath << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEOBJ_ConfigurationNode) aNode = Handle(DEOBJ_ConfigurationNode)::DownCast(GetNode());
  RWObj_CafReader                 aReader;
  aReader.SetSinglePrecision(aNode->InternalParameters.ReadSinglePrecision);
  aReader.SetSystemLengthUnit(aNode->GlobalParameters.LengthUnit / 1000);
  aReader.SetSystemCoordinateSystem(aNode->InternalParameters.SystemCS);
  aReader.SetFileLengthUnit(aNode->InternalParameters.FileLengthUnit);
  aReader.SetFileCoordinateSystem(aNode->InternalParameters.FileCS);
  aReader.SetDocument(theDocument);
  aReader.SetRootPrefix(aNode->InternalParameters.ReadRootPrefix);
  aReader.SetMemoryLimitMiB(aNode->InternalParameters.ReadMemoryLimitMiB);
  if (!aReader.Perform(thePath, theProgress))
  {
    Message1::SendFail() << "Error in the DEOBJ_ConfigurationNode during reading the file "
                        << thePath;
    return false;
  }
  XCAFDoc_DocumentTool::SetLengthUnit(theDocument,
                                      aNode->GlobalParameters.LengthUnit,
                                      UnitsMethods_LengthUnit_Millimeter);
  return true;
}

//=================================================================================================

bool DEOBJ_Provider::Write(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           const Message_ProgressRange&    theProgress)
{
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DEOBJ_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEOBJ_ConfigurationNode during writing the file "
                        << thePath << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEOBJ_ConfigurationNode) aNode = Handle(DEOBJ_ConfigurationNode)::DownCast(GetNode());

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
  Standard_Real                    aScaleFactorMM = 1.;
  if (XCAFDoc_DocumentTool::GetLengthUnit(theDocument,
                                          aScaleFactorMM,
                                          UnitsMethods_LengthUnit_Millimeter))
  {
    aConverter.SetInputLengthUnit(aScaleFactorMM / 1000.);
  }
  else
  {
    aConverter.SetInputLengthUnit(aNode->GlobalParameters.SystemUnit / 1000.);
    Message1::SendWarning()
      << "Warning in the DEOBJ_Provider during writing the file " << thePath
      << "\t: The document has no information on Units2. Using global parameter as initial Unit.";
  }
  aConverter.SetInputCoordinateSystem(aNode->InternalParameters.SystemCS);
  aConverter.SetOutputLengthUnit(aNode->GlobalParameters.LengthUnit / 1000.);
  aConverter.SetOutputCoordinateSystem(aNode->InternalParameters.FileCS);

  RWObj_CafWriter aWriter(thePath);
  aWriter.SetCoordinateSystemConverter(aConverter);
  if (!aWriter.Perform(theDocument, aFileInfo, theProgress))
  {
    Message1::SendFail() << "Error in the DEOBJ_ConfigurationNode during writing the file "
                        << thePath;
    return false;
  }
  return true;
}

//=================================================================================================

bool DEOBJ_Provider::Read(const AsciiString1& thePath,
                          TopoShape&                  theShape,
                          Handle(ExchangeSession)& theWS,
                          const Message_ProgressRange&   theProgress)
{
  (void)theWS;
  return Read(thePath, theShape, theProgress);
}

//=================================================================================================

bool DEOBJ_Provider::Write(const AsciiString1& thePath,
                           const TopoShape&            theShape,
                           Handle(ExchangeSession)& theWS,
                           const Message_ProgressRange&   theProgress)
{
  (void)theWS;
  return Write(thePath, theShape, theProgress);
}

//=================================================================================================

bool DEOBJ_Provider::Read(const AsciiString1& thePath,
                          TopoShape&                  theShape,
                          const Message_ProgressRange&   theProgress)
{
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DEOBJ_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DEOBJ_ConfigurationNode during writing the file "
                        << thePath << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DEOBJ_ConfigurationNode)  aNode = Handle(DEOBJ_ConfigurationNode)::DownCast(GetNode());
  RWMesh_CoordinateSystemConverter aConverter;
  aConverter.SetOutputLengthUnit(aNode->GlobalParameters.LengthUnit / 1000);
  aConverter.SetOutputCoordinateSystem(aNode->InternalParameters.SystemCS);
  aConverter.SetInputLengthUnit(aNode->InternalParameters.FileLengthUnit);
  aConverter.SetInputCoordinateSystem(aNode->InternalParameters.FileCS);

  RWObj_TriangulationReader aSimpleReader;
  aSimpleReader.SetTransformation(aConverter);
  aSimpleReader.SetSinglePrecision(aNode->InternalParameters.ReadSinglePrecision);
  aSimpleReader.SetCreateShapes(aNode->InternalParameters.ReadCreateShapes);
  aSimpleReader.SetSinglePrecision(aNode->InternalParameters.ReadSinglePrecision);
  aSimpleReader.SetMemoryLimit(aNode->InternalParameters.ReadMemoryLimitMiB);
  if (!aSimpleReader.Read(thePath, theProgress))
  {
    Message1::SendFail() << "Error in the DEOBJ_ConfigurationNode during reading the file "
                        << thePath;
    return false;
  }
  Handle(MeshTriangulation) aTriangulation = aSimpleReader.GetTriangulation();
  TopoFace                aFace;
  ShapeBuilder               aBuiler;
  aBuiler.MakeFace(aFace);
  aBuiler.UpdateFace(aFace, aTriangulation);
  theShape = aFace;
  return true;
}

//=================================================================================================

bool DEOBJ_Provider::Write(const AsciiString1& thePath,
                           const TopoShape&            theShape,
                           const Message_ProgressRange&   theProgress)
{
  Handle(AppDocument)  aDoc    = new AppDocument("BinXCAF");
  Handle(XCAFDoc_ShapeTool) aShTool = XCAFDoc_DocumentTool::ShapeTool(aDoc->Main());
  aShTool->AddShape(theShape);
  return Write(thePath, aDoc, theProgress);
}

//=================================================================================================

AsciiString1 DEOBJ_Provider::GetFormat() const
{
  return AsciiString1("OBJ");
}

//=================================================================================================

AsciiString1 DEOBJ_Provider::GetVendor() const
{
  return AsciiString1("OCC");
}
