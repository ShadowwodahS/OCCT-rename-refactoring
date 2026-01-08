// Copyright (c) 1997-1999 Matra Datavision
// Copyright (c) 1999-2015 OPEN CASCADE SAS
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

#ifndef _ViewerTest_HeaderFile
#define _ViewerTest_HeaderFile

#include <Aspect_TypeOfLine.hxx>
#include <Aspect_TypeOfMarker.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <Draw_Interpretor.hxx>
#include <Graphic3d_TypeOfShadingModel.hxx>
#include <Graphic3d_Vec2.hxx>
#include <Graphic3d_ZLayerId.hxx>
#include <TColStd_HArray1OfTransient.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_HArray1OfShape.hxx>
#include <Quantity_ColorRGBA.hxx>

class VisualContext;
class VisualEntity;
class Image_PixMap;
class ViewWindow;
class ViewManager;
class ViewerTest_EventManager;
class TopoShape;
class WNT_WClass;

//! Parameters for creating new view.
struct ViewerTest_VinitParams
{
  AsciiString1       ViewName;
  AsciiString1       DisplayName;
  Handle(ViewWindow)              ViewToClone;
  Handle(ViewWindow)              ParentView;
  Graphic3d_Vec2d               Offset;
  Graphic3d_Vec2d               Size;
  Aspect_TypeOfTriedronPosition Corner;
  Graphic3d_Vec2i               SubviewMargins;
  Standard_Boolean              IsVirtual;
  Standard_Boolean              IsComposer;

  ViewerTest_VinitParams()
      : Corner(Aspect_TOTP_LEFT_UPPER),
        IsVirtual(false),
        IsComposer(false)
  {
  }
};

class ViewerTest1
{
public:
  DEFINE_STANDARD_ALLOC

  //! Loads all Draw1 commands of  V2d & V3d1. Used for plugin.
  Standard_EXPORT static void Factory(DrawInterpreter& theDI);

  //! Creates view with default or custom name and adds this name in map to manage multiple views.
  //! Implemented in ViewerTest_ViewerCommands.cxx.
  Standard_EXPORT static AsciiString1 ViewerInit(
    const ViewerTest_VinitParams& theParams);

  //! Creates view.
  static AsciiString1 ViewerInit(const AsciiString1& theViewName = "")
  {
    ViewerTest_VinitParams aParams;
    aParams.ViewName = theViewName;
    return ViewerInit(aParams);
  }

  //! Creates view.
  static AsciiString1 ViewerInit(
    const Standard_Integer         thePxLeft,
    const Standard_Integer         thePxTop,
    const Standard_Integer         thePxWidth,
    const Standard_Integer         thePxHeight,
    const AsciiString1& theViewName,
    const AsciiString1& theDisplayName = "",
    const Handle(ViewWindow)&        theViewToClone = Handle(ViewWindow)(),
    const Standard_Boolean         theIsVirtual   = false)
  {
    ViewerTest_VinitParams aParams;
    aParams.Offset.SetValues((float)thePxLeft, (float)thePxTop);
    aParams.Size.SetValues((float)thePxWidth, (float)thePxHeight);
    aParams.ViewName    = theViewName;
    aParams.DisplayName = theDisplayName;
    aParams.ViewToClone = theViewToClone;
    aParams.IsVirtual   = theIsVirtual;
    return ViewerInit(aParams);
  }

  Standard_EXPORT static void RemoveViewName(const AsciiString1& theName);

  Standard_EXPORT static void InitViewName(const AsciiString1& theName,
                                           const Handle(ViewWindow)&        theView);

  Standard_EXPORT static AsciiString1 GetCurrentViewName();

  //! Make the view active
  Standard_EXPORT static void ActivateView(const Handle(ViewWindow)& theView,
                                           Standard_Boolean        theToUpdate);

  //! Removes view and clear all maps
  //! with information about its resources if necessary
  Standard_EXPORT static void RemoveView(const AsciiString1& theViewName,
                                         const Standard_Boolean theToRemoveContext = Standard_True);

  //! Removes view and clear all maps
  //! with information about its resources if necessary
  Standard_EXPORT static void RemoveView(const Handle(ViewWindow)& theView,
                                         const Standard_Boolean theToRemoveContext = Standard_True);

  //! Display AIS1 object in active Viewer and register it in the map of Interactive Objects with
  //! specified name.
  //! @param theName            key to be associated to displayed interactive object
  //! @param theObject          object to display
  //! @param theToUpdate        redraw viewer after displaying object
  //! @param theReplaceIfExists replace the object assigned to specified key
  //! @return true if new object has been displayed
  Standard_EXPORT static Standard_Boolean Display(
    const AsciiString1&       theName,
    const Handle(VisualEntity)& theObject,
    const Standard_Boolean               theToUpdate        = Standard_True,
    const Standard_Boolean               theReplaceIfExists = Standard_True);

  //! waits until a shape of type <aType> is picked in the AIS1 Viewer and returns it.
  //! if <aType> == TopAbs_Shape, any shape can be picked...
  //! MaxPick  is the Max number before exiting, if no pick is successful
  Standard_EXPORT static TopoShape PickShape(const TopAbs_ShapeEnum aType,
                                                const Standard_Integer MaxPick = 5);

  //! wait until the array is filled with picked shapes.
  //! returns True if the array is filled.
  //! exit if number of unsuccessful picks =  <MaxPick>
  Standard_EXPORT static Standard_Boolean PickShapes(const TopAbs_ShapeEnum           aType,
                                                     Handle(HArray1OfShape)& thepicked,
                                                     const Standard_Integer           MaxPick = 5);

  Standard_EXPORT static void Commands(DrawInterpreter& theCommands);

  Standard_EXPORT static void ViewerCommands(DrawInterpreter& theCommands);

  Standard_EXPORT static void RelationCommands(DrawInterpreter& theCommands);

  Standard_EXPORT static void ObjectCommands(DrawInterpreter& theCommands);

  Standard_EXPORT static void FilletCommands(DrawInterpreter& theCommands);

  Standard_EXPORT static void OpenGlCommands(DrawInterpreter& theCommands);

  Standard_EXPORT static void GetMousePosition(Standard_Integer& xpix, Standard_Integer& ypix);

  Standard_EXPORT static Handle(ViewManager) GetViewerFromContext();

  Standard_EXPORT static Handle(ViewManager) GetCollectorFromContext();

  Standard_EXPORT static const Handle(VisualContext)& GetAISContext();

  Standard_EXPORT static void SetAISContext(const Handle(VisualContext)& aContext);

  Standard_EXPORT static const Handle(ViewWindow)& CurrentView();

  Standard_EXPORT static void CurrentView(const Handle(ViewWindow)& aViou);

  Standard_EXPORT static void Clear();

  //! puts theMgr as current eventmanager (the move,select,...will be applied to theMgr)
  Standard_EXPORT static void SetEventManager(const Handle(ViewerTest_EventManager)& theMgr);

  //! removes the last EventManager from the list.
  Standard_EXPORT static void UnsetEventManager();

  //! clear the list of EventManagers and
  //! sets the default EventManager as current
  Standard_EXPORT static void ResetEventManager();

  Standard_EXPORT static Handle(ViewerTest_EventManager) CurrentEventManager();

  Standard_EXPORT static void RemoveSelected();

  //! redraws all defined views.
  Standard_EXPORT static void RedrawAllViews();

  //! Splits "parameter=value" string into separate
  //! parameter and value strings.
  //! @return TRUE if the string matches pattern "<string>=<empty or string>"
  Standard_EXPORT static Standard_Boolean SplitParameter(const AsciiString1& theString,
                                                         AsciiString1&       theName,
                                                         AsciiString1&       theValue);

  //! Returns list of selected shapes.
  Standard_EXPORT static void GetSelectedShapes(ShapeList& theShapes);

  //! Parses line type argument.
  //! Handles either enumeration (integer) value or string constant.
  Standard_EXPORT static Standard_Boolean ParseLineType(Standard_CString   theArg,
                                                        Aspect_TypeOfLine& theType,
                                                        uint16_t&          thePattern);

  //! Parses line type argument.
  //! Handles either enumeration (integer) value or string constant.
  static Standard_Boolean ParseLineType(Standard_CString theArg, Aspect_TypeOfLine& theType)
  {
    uint16_t aPattern = 0xFFFF;
    return ParseLineType(theArg, theType, aPattern);
  }

  //! Parses marker type argument.
  //! Handles either enumeration (integer) value or string constant.
  Standard_EXPORT static Standard_Boolean ParseMarkerType(Standard_CString      theArg,
                                                          Aspect_TypeOfMarker&  theType,
                                                          Handle(Image_PixMap)& theImage);

  //! Parses shading model argument.
  //! Handles either enumeration (integer) value or string constant.
  Standard_EXPORT static Standard_Boolean ParseShadingModel(Standard_CString              theArg,
                                                            Graphic3d_TypeOfShadingModel& theModel);

  //! Parses ZLayer name.
  //! @param[in] theArg  layer name or enumeration alias
  //! @param[out] theLayer  layer index
  //! @return TRUE if layer has been identified, note that Graphic3d_ZLayerId_UNKNOWN is also valid
  //! value
  static Standard_Boolean ParseZLayerName(Standard_CString theArg, Graphic3d_ZLayerId& theLayer)
  {
    return parseZLayer(theArg, false, theLayer);
  }

  //! Parses ZLayer name.
  //! @param[in] theArg  layer name, enumeration alias or index (of existing Layer)
  //! @param[out] theLayer  layer index
  //! @return TRUE if layer has been identified, note that Graphic3d_ZLayerId_UNKNOWN is also valid
  //! value
  static Standard_Boolean ParseZLayer(Standard_CString theArg, Graphic3d_ZLayerId& theLayer)
  {
    return parseZLayer(theArg, true, theLayer);
  }

  //! Auxiliary method to parse transformation persistence flags
  Standard_EXPORT static Standard_Boolean ParseCorner(Standard_CString               theArg,
                                                      Aspect_TypeOfTriedronPosition& theCorner);

public: //! @name deprecated methods
  //! Parses RGB(A) color argument(s) specified within theArgVec[0], theArgVec[1], theArgVec[2] and
  //! theArgVec[3].
  Standard_DEPRECATED("Method has been moved to Draw1::ParseColor()")
  Standard_EXPORT static Standard_Integer ParseColor(const Standard_Integer   theArgNb,
                                                     const char* const* const theArgVec,
                                                     Quantity_ColorRGBA&      theColor);

  //! Parses RGB color argument(s).
  //! Returns number of handled arguments (1 or 3) or 0 on syntax error.
  Standard_DEPRECATED("Method has been moved to Draw1::ParseColor()")
  Standard_EXPORT static Standard_Integer ParseColor(const Standard_Integer   theArgNb,
                                                     const char* const* const theArgVec,
                                                     Color1&          theColor);

  //! Parses boolean argument.
  //! Handles either flag specified by 0|1 or on|off.
  Standard_DEPRECATED("Method has been moved to Draw1::ParseOnOff()")
  Standard_EXPORT static Standard_Boolean ParseOnOff(Standard_CString  theArg,
                                                     Standard_Boolean& theIsOn);

  Standard_DEPRECATED("Method has been moved to Color1::ColorFromName()")
  Standard_EXPORT static Quantity_NameOfColor GetColorFromName(const Standard_CString name);

private:
  //! Parses ZLayer name.
  //! @param[in] theArg  layer name, enumeration alias or index (of existing Layer)
  //! @param[in] theToAllowInteger  when TRUE, the argument will be checked for existing layer index
  //! @param[out] theLayer  layer index
  //! @return TRUE if layer has been identified, note that Graphic3d_ZLayerId_UNKNOWN is also valid
  //! value
  Standard_EXPORT static Standard_Boolean parseZLayer(Standard_CString    theArg,
                                                      Standard_Boolean    theToAllowInteger,
                                                      Graphic3d_ZLayerId& theLayer);

  //! Returns a window class that implements standard behavior of
  //! all windows of the ViewerTest1. This includes usual Open CASCADE
  //! view conventions for mouse buttons (e.g. Ctrl+MB1 for zoom,
  //! Ctrl+MB2 for pan, etc) and keyboard shortcuts.
  //! This method is relevant for MS Windows only and respectively
  //! returns WNT_WClass handle.
  static const Handle(WNT_WClass)& WClass();
};

#endif // _ViewerTest_HeaderFile
