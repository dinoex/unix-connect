/******************************************************************
*	VERSION.H
******************************************************************/

/******************************************************************
*
*	Version 1.70
*
*	Copyright (c) 1992,1993,1994,1995,1996,1997,1998,1999
*	by Dirk Meyer, All rights reserved.
*	Im Grund 4, 34317 Habichtswald, Germany
*	Email: dirk.meyer@dinoex.sub.org
*
*	1.64
*	- Angepasst an die OS/2-Version
*
******************************************************************/

#ifndef VERSION_INCLUDE
#define VERSION_INCLUDE

/******************************************************************
*
*	Version und Release Definition
*
******************************************************************/

#define TOOL_VERSION		"1.70"

#define ENV_CONFIG_DATEI	C_DINOEX
#define ENV_CONFIG_PORT		C_DINOEX_PORT

#if !defined(OS2)
#define START_MESSAGE		"\n-- Dinoex %s " \
				TOOL_VERSION " , " \
			"Copyright (c) 1992,93,94,95,96,97,98,99\n" \
			" by Dirk Meyer, All rights reserved," \
			" Email: dirk.meyer@dinoex.sub.org\n"
#else
#define START_MESSAGE		"\nDinoex %s/2 " \
				TOOL_VERSION ", " \
			"(c) 1992-98 Dirk Meyer, " \
			"Email: soft@dinoex.sub.org\n" \
			"OS/2 Edition (c) 1997-98 Hinrich Donner, " \
			"All rights reserved. EMail: hd@tiro.de\n"
#endif

#define GET_NEXT_DATA(x)	{ argv++; argc--; x = *argv;	\
				if (x == NULL) usage(); }

#endif

/******************************************************************
*	END OF FILE VERSION.H
******************************************************************/
