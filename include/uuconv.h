#ifndef __UUCONV_H
#define __UUCONV_H

#ifndef __MIME_H
#include "mime.h"
#endif

header_p convheader(header_p, FILE *, char *, char *);
int convaddr(char *zconnect_header, char *rfc_addr, int rfc2z, FILE *f);
int make_body(char *bigbuffer, long msglen, mime_header_info_struct *info,
	int binaer, char *smallbuffer, FILE *zconnect);

#endif
