/* (c) Copyright 1991-2005,2006 by Inlab Software GmbH, Gruenwald
 * and Thomas Obermair ( obermair@acm.org )
 * All Rights Reserved / Alle Rechte vorbehalten
 * $Id: boot.h,v 4.102 2006/04/29 09:13:22 tommy Exp $
 */

#define PrimitiveProcedure(function,name,arguments) {\
  extern function();\
  SchemeInstallPrimitive(function,name,arguments); \
  };
