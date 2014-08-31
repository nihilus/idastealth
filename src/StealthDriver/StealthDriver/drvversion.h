///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Defines for the version information in the resource file
///
/// (File was in the PUBLIC DOMAIN  - Author: ddkwizard.assarbad.net)
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __DRVVERSION_H_VERSION__
#define __DRVVERSION_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


// ---------------------------------------------------------------------------
// Several defines have to be given before including this file. These are:
// ---------------------------------------------------------------------------
#define TEXT_AUTHOR            Jan Newger // author (optional value)
#define PRD_MAJVER             1 // major product version
#define PRD_MINVER             3 // minor product version
#define PRD_BUILD              0 // build number for product
#define FILE_MAJVER            1 // major file version
#define FILE_MINVER            3 // minor file version
#define FILE_BUILD             0 // build file number
#define DRV_YEAR               2009-2010 // current year or timespan (e.g. 2003-2007)
#define TEXT_WEBSITE           http:/##/newgre.net // website
#define TEXT_PRODUCTNAME       StealthDriver // product's name
#define TEXT_FILEDESC          StealthDriver // component description
#define TEXT_COMPANY           // company
#define TEXT_MODULE            StealthDriver // module name
#define TEXT_COPYRIGHT         Copyright \xA9 DRV_YEAR TEXT_COMPANY // copyright information
// #define TEXT_SPECIALBUILD      // optional comment for special builds
#define TEXT_INTERNALNAME      StealthDriver.sys // copyright information
// #define TEXT_COMMENTS          // optional comments
// ---------------------------------------------------------------------------
// ... well, that's it. Pretty self-explanatory ;)
// ---------------------------------------------------------------------------

#endif // __DRVVERSION_H_VERSION__
