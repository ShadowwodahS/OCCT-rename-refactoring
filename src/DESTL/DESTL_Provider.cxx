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

#include <DESTL_Provider.hxx>

#include <BRep_Builder.hxx>
#include <DESTL_ConfigurationNode.hxx>
#include <Message.hxx>
#include <RWStl.hxx>
#include <StlAPI.hxx>
#include <StlAPI_Writer.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

IMPLEMENT_STANDARD_RTTIEXT(DESTL_Provider, DE_Provider)

//=================================================================================================

DESTL_Provider::DESTL_Provider() {}

//=================================================================================================

DESTL_Provider::DESTL_Provider(const Handle(DE_ConfigurationNode)& theNode)
    : DE_Provider(theNode)
{
}

//=================================================================================================

bool DESTL_Provider::Read(const AsciiString1&  thePath,
                          const Handle(AppDocument)& theDocument,
                          Handle(ExchangeSession)&  theWS,
                          const Message_ProgressRange&    theProgress)
{
  (void)theWS;
  return Read(thePath, theDocument, theProgress);
}

//=================================================================================================

bool DESTL_Provider::Write(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           Handle(ExchangeSession)&  theWS,
                           const Message_ProgressRange&    theProgress)
{
  (void)theWS;
  return Write(thePath, theDocument, theProgress);
}

//=================================================================================================

bool DESTL_Provider::Read(const AsciiString1&  thePath,
                          const Handle(AppDocument)& theDocument,
                          const Message_ProgressRange&    theProgress)
{
  if (theDocument.IsNull())
  {
    Message1::SendFail() << "Error in the DESTL_Provider during reading the file " << thePath
                        << "\t: theDocument shouldn't be null";
    return false;
  }
  TopoShape aShape;
  if (!Read(thePath, aShape, theProgress))
  {
    return false;
  }
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(theDocument->Main());
  aShapeTool->AddShape(aShape);
  return true;
}

//=================================================================================================

bool DESTL_Provider::Write(const AsciiString1&  thePath,
                           const Handle(AppDocument)& theDocument,
                           const Message_ProgressRange&    theProgress)
{
  TopoShape              aShape;
  TDF_LabelSequence         aLabels;
  Handle(XCAFDoc_ShapeTool) aSTool = XCAFDoc_DocumentTool::ShapeTool(theDocument->Main());
  aSTool->GetFreeShapes(aLabels);
  if (aLabels.Length() <= 0)
  {
    Message1::SendFail() << "Error in the DESTL_Provider during writing the file " << thePath
                        << "\t: Document contain no shapes";
    return false;
  }

  Handle(DESTL_ConfigurationNode) aNode = Handle(DESTL_ConfigurationNode)::DownCast(GetNode());
  if (aNode->GlobalParameters.LengthUnit != 1.0)
  {
    Message1::SendWarning()
      << "Warning in the DESTL_Provider during writing the file " << thePath
      << "\t: Target Units for writing were changed, but current format doesn't support scaling";
  }

  if (aLabels.Length() == 1)
  {
    aShape = aSTool->GetShape(aLabels.Value(1));
  }
  else
  {
    TopoCompound aComp;
    ShapeBuilder    aBuilder;
    aBuilder.MakeCompound(aComp);
    for (Standard_Integer anIndex = 1; anIndex <= aLabels.Length(); anIndex++)
    {
      TopoShape aS = aSTool->GetShape(aLabels.Value(anIndex));
      aBuilder.Add(aComp, aS);
    }
    aShape = aComp;
  }
  return Write(thePath, aShape, theProgress);
}

//=================================================================================================

bool DESTL_Provider::Read(const AsciiString1& thePath,
                          TopoShape&                  theShape,
                          Handle(ExchangeSession)& theWS,
                          const Message_ProgressRange&   theProgress)
{
  (void)theWS;
  return Read(thePath, theShape, theProgress);
}

//=================================================================================================

bool DESTL_Provider::Write(const AsciiString1& thePath,
                           const TopoShape&            theShape,
                           Handle(ExchangeSession)& theWS,
                           const Message_ProgressRange&   theProgress)
{
  (void)theWS;
  return Write(thePath, theShape, theProgress);
}

//=================================================================================================

bool DESTL_Provider::Read(const AsciiString1& thePath,
                          TopoShape&                  theShape,
                          const Message_ProgressRange&   theProgress)
{
  Message1::SendWarning()
    << "OCCT Stl reader does not support model scaling according to custom length unit";
  if (!GetNode()->IsKind(STANDARD_TYPE(DESTL_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DESTL_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return true;
  }
  Handle(DESTL_ConfigurationNode) aNode = Handle(DESTL_ConfigurationNode)::DownCast(GetNode());
  double aMergeAngle                    = aNode->InternalParameters.ReadMergeAngle * M_PI / 180.0;
  if (aMergeAngle != M_PI_2)
  {
    if (aMergeAngle < 0.0 || aMergeAngle > M_PI_2)
    {
      Message1::SendFail() << "Error in the DESTL_Provider during reading the file " << thePath
                          << "\t: The merge angle is out of the valid range";
      return false;
    }
  }
  if (!aNode->InternalParameters.ReadBRep)
  {
    Handle(MeshTriangulation) aTriangulation =
      RWStl1::ReadFile(thePath.ToCString(), aMergeAngle, theProgress);

    TopoFace  aFace;
    ShapeBuilder aB;
    aB.MakeFace(aFace);
    aB.UpdateFace(aFace, aTriangulation);
    theShape = aFace;
  }
  else
  {
    Standard_DISABLE_DEPRECATION_WARNINGS if (!StlAPI1::Read(theShape, thePath.ToCString()))
    {
      Message1::SendFail() << "Error in the DESTL_Provider during reading the file " << thePath;
      return false;
    }
    Standard_ENABLE_DEPRECATION_WARNINGS
  }
  return true;
}

//=================================================================================================

bool DESTL_Provider::Write(const AsciiString1& thePath,
                           const TopoShape&            theShape,
                           const Message_ProgressRange&   theProgress)
{
  Message1::SendWarning()
    << "OCCT Stl writer does not support model scaling according to custom length unit";
  if (GetNode().IsNull() || !GetNode()->IsKind(STANDARD_TYPE(DESTL_ConfigurationNode)))
  {
    Message1::SendFail() << "Error in the DESTL_Provider during reading the file " << thePath
                        << "\t: Incorrect or empty Configuration Node";
    return false;
  }
  Handle(DESTL_ConfigurationNode) aNode = Handle(DESTL_ConfigurationNode)::DownCast(GetNode());
  if (aNode->GlobalParameters.LengthUnit != 1.0)
  {
    Message1::SendWarning()
      << "Warning in the DESTL_Provider during writing the file " << thePath
      << "\t: Target Units for writing were changed, but current format doesn't support scaling";
  }

  StlWriter aWriter;
  aWriter.ASCIIMode() = aNode->InternalParameters.WriteAscii;
  if (!aWriter.Write(theShape, thePath.ToCString(), theProgress))
  {
    Message1::SendFail() << "Error in the DESTL_Provider during reading the file " << thePath
                        << "\t: Mesh1 writing has been failed";
    return false;
  }
  return true;
}

//=================================================================================================

AsciiString1 DESTL_Provider::GetFormat() const
{
  return AsciiString1("STL");
}

//=================================================================================================

AsciiString1 DESTL_Provider::GetVendor() const
{
  return AsciiString1("OCC");
}
