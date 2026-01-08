// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#include <Interface_Static.hxx>

#include <Message_MsgFile.hxx>

#include "../XSMessage/XSMessage_XSTEP_us.pxx"

static int THE_Interface_Static_deja = 0;

void ExchangeConfig::Standards()
{
  if (THE_Interface_Static_deja)
  {
    return;
  }

  THE_Interface_Static_deja = 1;

  //   read precision
  // #74 rln 10.03.99 S4135: new values and default value
  ExchangeConfig::Init("XSTEP", "read.precision.mode", 'e', "");
  ExchangeConfig::Init("XSTEP", "read.precision.mode", '&', "ematch 0");
  ExchangeConfig::Init("XSTEP", "read.precision.mode", '&', "eval File");
  ExchangeConfig::Init("XSTEP", "read.precision.mode", '&', "eval User");
  ExchangeConfig::SetIVal("read.precision.mode", 0);

  ExchangeConfig::Init("XSTEP", "read.precision.val", 'r', "1.e-03");

  ExchangeConfig::Init("XSTEP", "read.maxprecision.mode", 'e', "");
  ExchangeConfig::Init("XSTEP", "read.maxprecision.mode", '&', "ematch 0");
  ExchangeConfig::Init("XSTEP", "read.maxprecision.mode", '&', "eval Preferred");
  ExchangeConfig::Init("XSTEP", "read.maxprecision.mode", '&', "eval Forced");
  ExchangeConfig::SetIVal("read.maxprecision.mode", 0);

  ExchangeConfig::Init("XSTEP", "read.maxprecision.val", 'r', "1.");

  //   encode regularity
  //  negatif ou nul : ne rien faire. positif : on y va
  ExchangeConfig::Init("XSTEP", "read.encoderegularity.angle", 'r', "0.01");

  //   compute surface curves
  //  0 : par defaut. 2 : ne garder que le 2D. 3 : ne garder que le 3D
  // gka S4054
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", 'e', "");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "ematch -3");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval 3DUse_Forced");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval 2DUse_Forced");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval ?");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval Default");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval ?");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval 2DUse_Preferred");
  ExchangeConfig::Init("XSTEP", "read.surfacecurve.mode", '&', "eval 3DUse_Preferred");
  ExchangeConfig::SetIVal("read.surfacecurve.mode", 0);

  //   write precision
  ExchangeConfig::Init("XSTEP", "write.precision.mode", 'e', "");
  ExchangeConfig::Init("XSTEP", "write.precision.mode", '&', "ematch -1");
  ExchangeConfig::Init("XSTEP", "write.precision.mode", '&', "eval Min");
  ExchangeConfig::Init("XSTEP", "write.precision.mode", '&', "eval Average");
  ExchangeConfig::Init("XSTEP", "write.precision.mode", '&', "eval Max");
  ExchangeConfig::Init("XSTEP", "write.precision.mode", '&', "eval User");
  ExchangeConfig::SetIVal("write.precision.mode", 0);

  ExchangeConfig::Init("XSTEP", "write.precision.val", 'r', "1.e-03");

  // Write surface curves
  // 0: write (defaut), 1: do not write, 2: write except for analytical surfaces
  ExchangeConfig::Init("XSTEP", "write.surfacecurve.mode", 'e', "");
  ExchangeConfig::Init("XSTEP", "write.surfacecurve.mode", '&', "ematch 0");
  ExchangeConfig::Init("XSTEP", "write.surfacecurve.mode", '&', "eval Off");
  ExchangeConfig::Init("XSTEP", "write.surfacecurve.mode", '&', "eval On");
  //  ExchangeConfig::Init("XSTEP"  ,"write.surfacecurve.mode", '&',"eval NoAnalytic");
  ExchangeConfig::SetIVal("write.surfacecurve.mode", 1);

  //  lastpreci : pour recuperer la derniere valeur codee (cf XSControl1)
  //    (0 pour dire : pas codee)
  //: S4136  ExchangeConfig::Init("std"    ,"lastpreci", 'r',"0.");

  // load messages if needed
  if (!MessageFile::HasMsg("XSTEP_1"))
  {
    if (!MessageFile::LoadFromEnv("CSF_XSMessage", "XSTEP"))
    {
      MessageFile::LoadFromString(XSMessage_XSTEP_us, sizeof(XSMessage_XSTEP_us) - 1);
    }
    if (!MessageFile::HasMsg("XSTEP_1"))
    {
      throw Standard_ProgramError(
        "Critical Error - message resources for ExchangeConfig are invalid or undefined!");
    }
  }
}
