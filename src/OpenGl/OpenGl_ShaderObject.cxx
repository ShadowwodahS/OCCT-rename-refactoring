// Created on: 2013-09-19
// Created by: Denis BOGOLEPOV
// Copyright (c) 2013-2014 OPEN CASCADE SAS
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

#include <Graphic3d_ShaderObject.hxx>
#include <OpenGl_Context.hxx>
#include <OpenGl_ShaderObject.hxx>
#include <OSD_File.hxx>
#include <OSD_Process.hxx>
#include <OSD_Protection.hxx>
#include <Standard_Assert.hxx>
#include <TCollection_AsciiString.hxx>

#ifdef _WIN32
  #include <malloc.h> // for alloca()
#endif

IMPLEMENT_STANDARD_RTTIEXT(OpenGl_ShaderObject, OpenGl_Resource)

//! Puts line numbers to the output of GLSL program source code.
static AsciiString1 putLineNumbers(const AsciiString1& theSource)
{
  std::stringstream aStream;
  theSource.Print(aStream);
  std::string             aLine;
  Standard_Integer        aLineNumber = 1;
  AsciiString1 aResultSource;
  while (std::getline(aStream, aLine))
  {
    AsciiString1 anAsciiString = AsciiString1(aLine.c_str());
    anAsciiString.Prepend(AsciiString1("\n") + AsciiString1(aLineNumber)
                          + ": ");
    aResultSource += anAsciiString;
    aLineNumber++;
  }
  return aResultSource;
}

//! Return GLSL shader stage title.
static AsciiString1 getShaderTypeString(GLenum theType)
{
  switch (theType)
  {
    case GL_VERTEX_SHADER:
      return "Vertex Shader";
    case GL_FRAGMENT_SHADER:
      return "Fragment Shader";
    case GL_GEOMETRY_SHADER:
      return "Geometry1 Shader";
    case GL_TESS_CONTROL_SHADER:
      return "Tessellation Control Shader";
    case GL_TESS_EVALUATION_SHADER:
      return "Tessellation Evaluation Shader";
    case GL_COMPUTE_SHADER:
      return "Compute Shader";
  }
  return "Shader";
}

// =======================================================================
// function : OpenGl_ShaderObject
// purpose  : Creates uninitialized shader object
// =======================================================================
OpenGl_ShaderObject::OpenGl_ShaderObject(GLenum theType)
    : myType(theType),
      myShaderID(NO_SHADER)
{
  //
}

// =======================================================================
// function : ~OpenGl_ShaderObject
// purpose  : Releases resources of shader object
// =======================================================================
OpenGl_ShaderObject::~OpenGl_ShaderObject()
{
  Release(NULL);
}

//=================================================================================================

Standard_Boolean OpenGl_ShaderObject::LoadAndCompile(const Handle(OpenGl_Context)&  theCtx,
                                                     const AsciiString1& theId,
                                                     const AsciiString1& theSource,
                                                     bool                           theIsVerbose,
                                                     bool theToPrintSource)
{
  if (!theIsVerbose)
  {
    return LoadSource(theCtx, theSource) && Compile(theCtx);
  }

  if (!LoadSource(theCtx, theSource))
  {
    if (theToPrintSource)
    {
      theCtx->PushMessage(GL_DEBUG_SOURCE_APPLICATION,
                          GL_DEBUG_TYPE_ERROR,
                          0,
                          GL_DEBUG_SEVERITY_HIGH,
                          theSource);
    }
    theCtx->PushMessage(GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_ERROR,
                        0,
                        GL_DEBUG_SEVERITY_HIGH,
                        AsciiString1("Error! Failed to set ")
                          + getShaderTypeString(myType) + " [" + theId + "] source");
    return false;
  }

  if (!Compile(theCtx))
  {
    if (theToPrintSource)
    {
      theCtx->PushMessage(GL_DEBUG_SOURCE_APPLICATION,
                          GL_DEBUG_TYPE_ERROR,
                          0,
                          GL_DEBUG_SEVERITY_HIGH,
                          putLineNumbers(theSource));
    }
    AsciiString1 aLog;
    FetchInfoLog(theCtx, aLog);
    if (aLog.IsEmpty())
    {
      aLog = "Compilation log is empty.";
    }
    theCtx->PushMessage(GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_ERROR,
                        0,
                        GL_DEBUG_SEVERITY_HIGH,
                        AsciiString1("Failed to compile ") + getShaderTypeString(myType)
                          + " [" + theId + "]. Compilation log:\n" + aLog);
    return false;
  }
  else if (theCtx->caps->glslWarnings)
  {
    AsciiString1 aLog;
    FetchInfoLog(theCtx, aLog);
    if (!aLog.IsEmpty() && !aLog.IsEqual("No errors.\n"))
    {
      theCtx->PushMessage(GL_DEBUG_SOURCE_APPLICATION,
                          GL_DEBUG_TYPE_PORTABILITY,
                          0,
                          GL_DEBUG_SEVERITY_LOW,
                          getShaderTypeString(myType) + " [" + theId + "] compilation log:\n"
                            + aLog);
    }
  }
  return true;
}

//=================================================================================================

void OpenGl_ShaderObject::DumpSourceCode(const Handle(OpenGl_Context)&  theCtx,
                                         const AsciiString1& theId,
                                         const AsciiString1& theSource) const
{
  theCtx->PushMessage(GL_DEBUG_SOURCE_APPLICATION,
                      GL_DEBUG_TYPE_OTHER,
                      0,
                      GL_DEBUG_SEVERITY_MEDIUM,
                      getShaderTypeString(myType) + " [" + theId + "] source code:\n" + theSource);
}

// =======================================================================
// function : LoadSource
// purpose  : Loads shader source code
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::LoadSource(const Handle(OpenGl_Context)&  theCtx,
                                                 const AsciiString1& theSource)
{
  if (myShaderID == NO_SHADER)
  {
    return Standard_False;
  }

  const GLchar* aLines = theSource.ToCString();
  theCtx->core20fwd->glShaderSource(myShaderID, 1, &aLines, NULL);
  return Standard_True;
}

// =======================================================================
// function : Compile
// purpose  : Compiles the shader object
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::Compile(const Handle(OpenGl_Context)& theCtx)
{
  if (myShaderID == NO_SHADER)
  {
    return Standard_False;
  }

  // Try to compile shader
  theCtx->core20fwd->glCompileShader(myShaderID);

  // Check compile status
  GLint aStatus = GL_FALSE;
  theCtx->core20fwd->glGetShaderiv(myShaderID, GL_COMPILE_STATUS, &aStatus);
  return aStatus != GL_FALSE;
}

// =======================================================================
// function : FetchInfoLog
// purpose  : Fetches information log of the last compile operation
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::FetchInfoLog(const Handle(OpenGl_Context)& theCtx,
                                                   AsciiString1&      theLog)
{
  if (myShaderID == NO_SHADER)
  {
    return Standard_False;
  }

  // Load information log of the compiler
  GLint aLength = 0;
  theCtx->core20fwd->glGetShaderiv(myShaderID, GL_INFO_LOG_LENGTH, &aLength);
  if (aLength > 0)
  {
    GLchar* aLog = (GLchar*)alloca(aLength);
    memset(aLog, 0, aLength);
    theCtx->core20fwd->glGetShaderInfoLog(myShaderID, aLength, NULL, aLog);
    theLog = aLog;
  }

  return Standard_True;
}

// =======================================================================
// function : Create
// purpose  : Creates new empty shader object of specified type
// =======================================================================
Standard_Boolean OpenGl_ShaderObject::Create(const Handle(OpenGl_Context)& theCtx)
{
  if (myShaderID == NO_SHADER && theCtx->core20fwd != NULL)
  {
    myShaderID = theCtx->core20fwd->glCreateShader(myType);
  }

  return myShaderID != NO_SHADER;
}

// =======================================================================
// function : Release
// purpose  : Destroys shader object
// =======================================================================
void OpenGl_ShaderObject::Release(OpenGl_Context* theCtx)
{
  if (myShaderID == NO_SHADER)
  {
    return;
  }

  Standard_ASSERT_RETURN(
    theCtx != NULL,
    "OpenGl_ShaderObject destroyed without GL context! Possible GPU memory leakage...", );

  if (theCtx->core20fwd != NULL && theCtx->IsValid())
  {
    theCtx->core20fwd->glDeleteShader(myShaderID);
  }
  myShaderID = NO_SHADER;
}

//! Return GLSL shader stage file extension.
static const char* getShaderExtension(GLenum theType)
{
  switch (theType)
  {
    case GL_VERTEX_SHADER:
      return ".vs";
    case GL_FRAGMENT_SHADER:
      return ".fs";
    case GL_GEOMETRY_SHADER:
      return ".gs";
    case GL_TESS_CONTROL_SHADER:
      return ".tcs";
    case GL_TESS_EVALUATION_SHADER:
      return ".tes";
    case GL_COMPUTE_SHADER:
      return ".cs";
  }
  return ".glsl";
}

//! Expand substring with additional tail.
static void insertSubString(AsciiString1&       theString,
                            const char&                    thePattern,
                            const AsciiString1& theSubstitution)
{
  const int aSubLen = theSubstitution.Length();
  for (int aCharIter = 1, aNbChars = theString.Length(); aCharIter <= aNbChars; ++aCharIter)
  {
    if (theString.Value(aCharIter) == thePattern)
    {
      theString.Insert(aCharIter + 1, theSubstitution);
      aCharIter += aSubLen;
      aNbChars += aSubLen;
    }
  }
}

//! Dump GLSL shader source code into file.
static bool dumpShaderSource(const AsciiString1& theFileName,
                             const AsciiString1& theSource,
                             bool                           theToBeautify)
{
  SystemFile aFile(theFileName);
  aFile.Build(OSD_WriteOnly, OSD_Protection());
  AsciiString1 aSource = theSource;
  if (theToBeautify)
  {
    insertSubString(aSource, ';', "\n");
    insertSubString(aSource, '{', "\n");
    insertSubString(aSource, '}', "\n");
  }
  if (!aFile.IsOpen())
  {
    Message1::SendFail(AsciiString1("Error: File '") + theFileName
                      + "' cannot be opened to save shader");
    return false;
  }

  if (aSource.Length() > 0)
  {
    aFile.Write(aSource.ToCString(), aSource.Length());
  }
  aFile.Close();
  Message1::SendWarning(AsciiString1("Shader source dumped into '") + theFileName + "'");
  return true;
}

//! Read GLSL shader source code from file dump.
static bool restoreShaderSource(AsciiString1&       theSource,
                                const AsciiString1& theFileName)
{
  SystemFile aFile(theFileName);
  aFile.Open(OSD_ReadOnly, OSD_Protection());
  if (!aFile.IsOpen())
  {
    Message1::SendFail(AsciiString1("File '") + theFileName
                      + "' cannot be opened to load shader");
    return false;
  }

  const Standard_Integer aSize = (Standard_Integer)aFile.Size();
  if (aSize > 0)
  {
    theSource = AsciiString1(aSize, '\0');
    aFile.Read(theSource, aSize);
  }
  aFile.Close();
  Message1::SendWarning(AsciiString1("Restored shader dump from '") + theFileName + "'");
  return true;
}

//=================================================================================================

Standard_Boolean OpenGl_ShaderObject::updateDebugDump(const Handle(OpenGl_Context)&  theCtx,
                                                      const AsciiString1& theProgramId,
                                                      const AsciiString1& theFolder,
                                                      Standard_Boolean               theToBeautify,
                                                      Standard_Boolean               theToReset)
{
  const AsciiString1 aFileName =
    theFolder + "/" + theProgramId + getShaderExtension(myType);
  if (!theToReset)
  {
    SystemFile aFile(aFileName);
    if (aFile.Exists())
    {
      const Quantity_Date aDate = aFile.AccessMoment();
      if (aDate > myDumpDate)
      {
        AsciiString1 aNewSource;
        if (restoreShaderSource(aNewSource, aFileName))
        {
          LoadAndCompile(theCtx, theProgramId, aNewSource);
          return Standard_True;
        }
      }
      return Standard_False;
    }
  }

  bool isDumped = false;
  if (myShaderID != NO_SHADER)
  {
    GLint aLength = 0;
    theCtx->core20fwd->glGetShaderiv(myShaderID, GL_SHADER_SOURCE_LENGTH, &aLength);
    if (aLength > 0)
    {
      AsciiString1 aSource(aLength - 1, '\0');
      theCtx->core20fwd->glGetShaderSource(myShaderID, aLength, NULL, (GLchar*)aSource.ToCString());
      dumpShaderSource(aFileName, aSource, theToBeautify);
      isDumped = true;
    }
  }
  if (!isDumped)
  {
    dumpShaderSource(aFileName, "", false);
  }
  myDumpDate = OSD_Process().SystemDate();
  return Standard_False;
}
