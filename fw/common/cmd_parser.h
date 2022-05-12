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

#ifndef _CMD_PARSER_H_
#define _CMD_PARSER_H_

typedef enum {
   RESULT_OK,        // All is well
   RESULT_NO_SUPPORT,// Comamnd is defined, but not supported (yet/here)
   RESULT_BAD_ARG,   // One or more of the arguments are wrong or missing
   RESULT_USAGE,     // Display command usage
   RESULT_NO_INIT,   // CmdParserInit() hasn't been called
   RESULT_ERR,       // Command failed, but display it's own error message
   RESULT_INTERNAL_ERR
} CommandResults_t;

typedef int (*ParserPrintf)(const char *format, ...);

typedef struct {
   const char *CmdString;     // Command name
   const char *HelpString;    // short summary on command usage
   const char *Usage;         // detailed command usage (optional)
   int (*CmdHandler)(char *CmdLine);
} CommandTable_t;

// Init command line parser, with printf like and command table
int CmdParserInit(const CommandTable_t *CmdTbl,ParserPrintf Function);

// Parse a command line, calling any matched commands
CommandResults_t ParseCmd(char *CmdLine);
int ConvertValue(char **Arg,uint32_t *Value);

int HelpCmd(char *CmdLine);
char *SkipSpaces(char *In);
char *Skip2Space(char *In);

#endif   // _CMD_PARSER_H_

