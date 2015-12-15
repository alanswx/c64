/*
 *  FileHelpers.h
 *  c64carbon
 *
 *  Created by Alan Steremberg on 11/17/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>

OSErr SimpleNavPutFile(OSType fileType,OSType fileCreator,FSSpec *theFileSpec);
OSErr SimpleNavGetFile(FSSpec *theFileSpec);
