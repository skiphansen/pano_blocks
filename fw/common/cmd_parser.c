/*
 *  Copyright (C) 2022  Skip Hansen
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms and conditions of the GNU General Public License,
 *  version 2, as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// #include <ctype.h>      // NB: don't include this, we use replacments
int isspace(int c);
int tolower(int c);
int isdigit(int c);
int isxdigit(int c);

#include "printf.h"
#include "_string.h"
#include "cmd_parser.h"
#define DEBUG_LOGGING         1
// #define VERBOSE_DEBUG_LOGGING 1
#include "log.h"

static ParserPrintf gPrintf;
static const CommandTable_t *gCmdTbl;

#define PRINT_F(format, ...) (gPrintf)(format,## __VA_ARGS__)


char *SkipSpaces(char *In)
{
   while(isspace(*In)) {
      In++;
   }

   return In;
}

char *Skip2Space(char *In)
{
   while(*In != 0 && !isspace(*In)) {
      In++;
   }

   return In;
}

// CmdLine
// 0 = command doesn't match
// 1 = command matches
// 2 = exact command match
int MatchCmd(char *CmdLine,const char *Command)
{
   CmdLine = SkipSpaces(CmdLine);

   if(!*CmdLine) {
      return 0;
   }

   while(*CmdLine && *CmdLine != ' ' && *CmdLine != '\r' && *CmdLine != '\n') {
      if(tolower(*CmdLine) != tolower(*Command)) {
         return false;
      }
      CmdLine++;
      Command++;
   }
   if(*Command == 0) {
      // Exact match
      return 2;
   }
   return 1;
}

static const CommandTable_t *FindCmd(char *CmdLine,int bSilent) 
{
   int CmdsMatched = 0;
   int MatchResult;
   const char *cp;
   const CommandTable_t *pCmd = NULL;
   const CommandTable_t *Ret = NULL;

   cp = SkipSpaces(CmdLine);
   if(*cp) {
      pCmd = gCmdTbl;
      while(pCmd->CmdHandler != NULL) {
         MatchResult = MatchCmd(CmdLine,pCmd->CmdString);

         if(MatchResult) {
            CmdsMatched++;
            Ret = pCmd;
         }

         if(MatchResult == 2) {
            // Exact match, no point in looking further
            Ret = pCmd;
            CmdsMatched = 1;
            break;
         }
         pCmd++;
      }

      if(CmdsMatched == 0) {
      // command not found or command requires an exact match and it isn't
         Ret = NULL;
         if(!bSilent) {
            PRINT_F("Error: Unknown command.\n");
         }
      }
      else if(CmdsMatched > 1) {
         if(!bSilent) {
            PRINT_F("Error: Command is ambiguous.\n");
         }
         Ret = NULL;
      }
   }

   return Ret;
}

CommandResults_t ParseCmd(char *CmdLine)
{
   const CommandTable_t *pCurrentCmd;
   char *cp;
   CommandResults_t Ret = RESULT_OK; // assume the best
   bool bDisplayUsage = false;

   do {
      if(gPrintf == NULL || gCmdTbl == NULL) {
         Ret = RESULT_NO_INIT;
         break;
      }
      if((cp = SkipSpaces(CmdLine)) == NULL) {
      // Nothing to do
         break;
      }

      if((pCurrentCmd = FindCmd(cp,false)) != NULL) {
         cp = Skip2Space(cp);
         cp = SkipSpaces(cp);
         Ret = pCurrentCmd->CmdHandler(cp);
         if(Ret == RESULT_USAGE) {
            bDisplayUsage = true;
         }
         else if(Ret == RESULT_NO_SUPPORT) {
            PRINT_F("Error - Request not supported yet\n");
         }
         else if(Ret == RESULT_BAD_ARG) {
            bDisplayUsage = true;
            PRINT_F("Error - Invalid argument(s)\n");
         }
      }
   } while(false);

   if(bDisplayUsage) {
      PRINT_F("Usage:\n  %s\n",
             pCurrentCmd->Usage != NULL ? pCurrentCmd->Usage : 
             pCurrentCmd->HelpString);
   }

   return Ret;
}

int CmdParserInit(const CommandTable_t *CmdTbl,ParserPrintf Function)
{
   gPrintf = Function;
   gCmdTbl = CmdTbl;

   return 0;
}

int HelpCmd(char *CmdLine)
{
   const CommandTable_t *p = gCmdTbl;
   int Ret = RESULT_OK; // assume the best
   LOG("Called\n");

   do {
      if(p == NULL || gPrintf == NULL) {
         Ret = RESULT_NO_INIT;
         break;
      }

      PRINT_F("Commands:\n");
      while(p->CmdString != NULL) {
         if(p->HelpString != NULL) {
            PRINT_F("  %s - %s\n",p->CmdString,p->HelpString);
         }
         p++;
      }
   } while(false);

   return Ret;
}

int ConvertValue(char **Arg,uint32_t *Value)
{
   char *cp = *Arg;
   char *cp1;
   int Ret = 0;   // assume the best
   bool bIsHex = false;

   do {
      cp = SkipSpaces(cp);
      if(!*cp) {
      // No value
         Ret = 1;
         break;
      }
      if(cp[0] == '0' && cp[1] == 'x') {
      // Hex arg
         bIsHex = true;
         cp += 2;
      }
      cp1 = cp;
      while(*cp1) {
         if(isspace(*cp1)) {
            cp1++;
            break;
         }
         if((bIsHex && !isxdigit(*cp1)) || (!bIsHex && !isdigit(*cp1))) {
            Ret = 1;
            break;
         }
         cp1++;
      }
      if(Ret == 1) {
         break;
      }
      *Value = (uint32_t) (bIsHex ? atoi_hex(cp) : atoi(cp));
      *Arg = cp1;
   } while(false);

// LOG("Returning %d, Value 0x%x Arg '%s'\n",Ret,*Value,cp1);
   return Ret;
}
