/*
 *  FileHelpers.c
 *  c64carbon
 *
 *  Created by Alan Steremberg on 11/17/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "FileHelpers.h"

OSErr SimpleNavPutFile(   OSType fileType, 
OSType fileCreator, 
FSSpec *theFileSpec)
{
   OSStatus         theStatus;
   NavDialogRef   theDialog;
   NavReplyRecord theReply;
   AEDesc            aeDesc;
   FSRef            fsRefParent, fsRefDelete;
   UniChar         *nameBuffer;
   UniCharCount   nameLength;
   FInfo            fileInfo;
   OSErr            err = noErr;
   theStatus = NavCreatePutFileDialog(NULL, fileType, fileCreator,
NULL, NULL,
&theDialog);
   NavDialogRun(theDialog);
   theStatus = NavDialogGetReply ( theDialog, &theReply); 
   NavDialogDispose(theDialog);
         
   if(!theReply.validRecord)
   {
      // Assuming the user changed his/her mind? No harm; no foul.
      // Still need to indicate that a file has not been created
      return -1;   
   }   
                        
   err = AECoerceDesc(&theReply.selection, typeFSRef, &aeDesc);
   if(err != noErr) return err;
   err = AEGetDescData(&aeDesc, &fsRefParent, sizeof(FSRef));
   if(err != noErr) return err;
   nameLength = 
(UniCharCount)CFStringGetLength(theReply.saveFileName);
   nameBuffer = (UniChar *) NewPtr((long)nameLength);
   CFStringGetCharacters(theReply.saveFileName, 
CFRangeMake(0, (long)nameLength), 
&nameBuffer[0]);
   if(nameBuffer == NULL) return -1; // generic error
   if(theReply.replacing)
   {
      err = FSMakeFSRefUnicode(&fsRefParent, 
nameLength, nameBuffer, 
                        kTextEncodingUnicodeDefault, 
                        &fsRefDelete);
      if(err == noErr) err = FSDeleteObject(&fsRefDelete);
      if(err == fBsyErr)
      {
         DisposePtr((Ptr)nameBuffer);
         return err;
      }
   }
   
   err = FSCreateFileUnicode(&fsRefParent, nameLength, nameBuffer,
                      kFSCatInfoNone, NULL, NULL,
theFileSpec);
   
   err = FSpGetFInfo(theFileSpec, &fileInfo);
   fileInfo.fdType = fileType;
   fileInfo.fdCreator = fileCreator;
   err = FSpSetFInfo(theFileSpec, &fileInfo);
   
   return err;
}   


OSErr SimpleNavGetFile(FSSpec *theFileSpec)
{   
   OSStatus                        theStatus;
NavDialogRef                     theDialog;
   NavReplyRecord                theReply;
NavDialogCreationOptions    inOptions;
AEKeyword                        theKeyword;
   DescType                      actualType;
   Size                          actualSize;
   OSErr                           err = noErr;
   
   NavGetDefaultDialogCreationOptions(&inOptions);
   theStatus = NavCreateGetFileDialog(&inOptions, 
NULL, NULL, NULL, NULL, NULL, 
&theDialog);
   NavDialogRun(theDialog);
   theStatus = NavDialogGetReply ( theDialog, &theReply); 
   NavDialogDispose(theDialog);
      
   if(!theReply.validRecord)
   {
      return -1;      
// Assuming the user changed his/her mind? 
// No harm; no foul, but need to know 
// not to try to open the file.
   }   
                        
       // Get a pointer to selected file
err = AEGetNthPtr(&(theReply.selection), 1,
                           typeFSS, &theKeyword,
                           &actualType, theFileSpec,
                           sizeof(FSSpec),
                           &actualSize);
      
   return err;
}   

char *PtoCstr(unsigned char *s)
{
    int theLen;
    int t;

    theLen = s[0];

    for(t=0;t<theLen;t++)
        s[t] = s[t+1];
       
    s[theLen] = '\0';

    return (char *) s;
}
unsigned char *CtoPstr(char *s)
{
int theLen;
int t;

theLen = strlen(s);

for(t=theLen;t>=1;t--)
    s[t] = s[t-1];
   
s[0] = (char ) theLen;

return (unsigned char *) s;
}

#if 0
void CtoPstr (StringPtr outString, const char *inString)
{  
  unsigned char x = 0;
  do {
    *(((char*)outString) + x + 1) = *(inString + x);
    x++;
  } while ((*(inString + x) != 0)  && (x < 256));
  *((char*)outString) = (char) x;                  
}
#endif