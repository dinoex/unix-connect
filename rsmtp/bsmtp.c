/*
 * Copyright (c) 1992,1993,1994,1995,1996,1997,1998,1999
 *	by Dirk Meyer, All rights reserved.
 *	Im Grund 4, 34317 Habichtswald, Germany
 *	Email: dirk.meyer@dinoex.sub.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *---------------------------------------------------------------------------*
 *
 *	bsmtp - batched mail handler
 *	============================
 *
 *	file: bsmtp.c
 *
 *	Last Edit-Date: Sun May  3 18:56:10 CEST 1998
 *
 *	-dm 1.68
 *
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sysexits.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

/*---------------------------------------------------------------------------*
 *	configuration
 *---------------------------------------------------------------------------*/

#ifndef MAILDIR
#define MAILDIR		"/var/spool/dinoex/u/mailin"
#endif

#define	DATANAME	"%s/bsmtp.%06ld"

#define	LOCKNAME	"%s/.sequence"

#define	MAX_SEQUENCE	999999

#define	MAX_BLOCK	32000

/*---------------------------------------------------------------------------*
 *	some macros
 *---------------------------------------------------------------------------*/

#define ON	1
#define OFF	0

#define	LF	'\n'
#define	CR	'\r'

#include "version.h"

/*---------------------------------------------------------------------------*
 *	global instances
 *---------------------------------------------------------------------------*/

const char *Maildir = MAILDIR;

const char *Name;
const char *From_user;

long Rcpt_to;

int Flag_verbose = OFF;

/*---------------------------------------------------------------------------*
 *	copy data
 *---------------------------------------------------------------------------*/

static void
file_cat( FILE *fsrc, FILE *fdest, size_t max_block )
{
	char		*buffer;
	size_t		block;
	size_t		status;

	buffer = malloc( max_block );
	if ( buffer == NULL ) {
		fprintf( stderr,
			"%s: out of memory for %ld bytes\n",
			Name, (long)max_block );
		exit( EX_TEMPFAIL );
	};
fprintf( stderr, "start\n" );
	while ( feof( fsrc ) == 0 ) {
		block = fread( buffer, 1L, max_block, fsrc );
fprintf( stderr, "block = %ld\n", (long)block );
		if ( block > 0 ) {
			status = fwrite( buffer, 1L, block, fdest );
			if ( status != block ) {
				fprintf( stderr,
					"%s: error writing data: %s\n",
					Name, strerror( errno ) );
				exit( EX_CANTCREAT );
			}
		};
		if ( ferror( fsrc ) != 0 ) {
			fprintf( stderr,
				"%s: error reading stdin: %s\n",
				Name, strerror( errno ) );
			exit( EX_IOERR );
		}
	};

	free( buffer );
}

/*---------------------------------------------------------------------------*
 *	sequence
 *---------------------------------------------------------------------------*/

static long
get_sequence( const char *lockfile, long max_value )
{
	int		handle;
	int		st;
	size_t		len;
	long		sequence;
	char		buffer[ 200 ];

	handle = open( lockfile, O_RDWR | O_CREAT, 0644 );
	if ( handle < 0 ) {
		fprintf( stderr, "%s: open <%s> handle <%d>: %s\n",
			Name, lockfile, handle, strerror( errno ) );
		exit( EX_CANTCREAT );
	};
	/* CONSTANTCONDITION */
	while ( 1 ) {
		st = flock( handle, LOCK_EX );
		if ( st == 0 )
			break;
		fprintf( stderr, "%s: lock <%s> handle <%d>: %s\n",
			Name, lockfile, handle, strerror( errno ) );
		if ( errno == ENOLCK ) {
			sleep( 20 );
			continue;
		};
		exit( EX_CANTCREAT );
	};

	/* CONSTANTCONDITION */
	while ( 1 ) {
		sequence = 0L;
		len = read( handle, buffer, 20 );
		if ( len > 0L ) {
			buffer[ len ] = 0;
			sequence = atol( buffer );
			if ( sequence >= max_value )
				sequence = 0L;
		};
		++ sequence;
		len = lseek( handle, 0L, 0 );
		if ( len != 0L ) {
			fprintf( stderr, "seek %ld, %d\n", (long)len, errno );
			sequence = -1L;
			break;
		};
		sprintf( buffer, "%ld\n", sequence );
		len = write( handle, buffer, strlen( buffer ) );
		if ( len != strlen( buffer ) ) {
			fprintf( stderr, "write %ld, %d\n", (long)len, errno );
			sequence = -1L;
			break;
		};
		break;
	};

/*	close() macht die Arbeit
	st = flock( handle, LOCK_UN );
	if ( st != 0 ) {
		fprintf( stderr, "%s: unlock <%s> returns <%d>: %s\n",
			Name, lockfile, st, strerror( errno ) );
		exit( EX_CANTCREAT );
	};
*/
	st = close( handle );
	if ( st != 0 ) {
		fprintf( stderr, "%s: close <%s> returns <%d>: %s\n",
			Name, lockfile, st, strerror( errno ) );
		sequence = -1L;
		exit( EX_CANTCREAT );
	};

	return sequence;
}

/*---------------------------------------------------------------------------*
 *	check for valid dir
 *---------------------------------------------------------------------------*/

static int
test_is_dir( const char *dir )
{
	int		rc;
	struct stat	sb;

	rc = stat( dir, &sb );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: can't stat directory %s: %s\n",
			Name, dir, strerror( errno ) );
		return OFF;
	};
	if ( ( sb.st_mode & S_IFDIR ) == 0 ) {
		fprintf( stderr,
			"%s: not a directory: %s\n",
			Name, dir );
		return OFF;
	};
	return ON;
}

/*---------------------------------------------------------------------------*
 *	display help
 *---------------------------------------------------------------------------*/

static void
usage( void )
{
	fprintf( stderr, START_MESSAGE, "bsmtp" );
	fprintf( stderr,
"\n"
" Usage: | bsmtp [ -verbose ] [-spool dir ] mail_from rcpt_to [ rcpt_to ... ]\n"
"\n" );
	exit( EX_USAGE );
}

/*---------------------------------------------------------------------------*
 *	parse comand line and execute
 *---------------------------------------------------------------------------*/

int
main( int argc, const char *const *argv )
{
	FILE		*fdata;
	const char	*cptr;
	char		ch;
	char		*buffer;
	long		nr;
	int		rc;

	Name = argv[ 0 ];
	From_user = NULL;
	Rcpt_to = 0;
	fdata = NULL;

	/* check for commandline */
	while ( --argc > 0 ) {
		cptr = *(++argv);
		if ( *cptr == '-' ) {
			ch = *(++cptr);
			switch ( tolower( ch ) ) {
			case 's':
				GET_NEXT_DATA( cptr );
				Maildir = cptr;
				break;
			case 'v':
				Flag_verbose = ON;
				break;
			default:
				usage();
				break;
			};
			continue;
		};
		if ( From_user == NULL ) {
			if ( test_is_dir( Maildir ) == OFF )
				exit( EX_CANTCREAT );

			buffer = malloc( MAX_BLOCK );
			if ( buffer == NULL ) {
				fprintf( stderr,
					"%s: out of memory for %ld bytes\n",
					Name, (long)MAX_BLOCK );
				exit( EX_TEMPFAIL );
			};

			sprintf( buffer, LOCKNAME, Maildir );
			nr = get_sequence( buffer, MAX_SEQUENCE );
			sprintf( buffer, DATANAME, Maildir, nr );

			fdata = fopen( buffer, "wb" );
			if ( fdata == NULL ) {
				fprintf( stderr,
					"%s: error writing file %s: %s\n",
					Name, buffer, strerror( errno ) );
				exit( EX_CANTCREAT );
			};
			free( buffer );

			From_user = cptr;

			fprintf( fdata, "MAIL FROM: <%s>\n", From_user );
			if ( ferror( fdata ) != 0 ) {
				fprintf( stderr,
					"%s: error writing data: %s\n",
					Name, strerror( errno ) );
				exit( EX_CANTCREAT );
			};
			continue;
		};
		fprintf( fdata, "RCPT TO: <%s>\n", cptr );
			if ( ferror( fdata ) != 0 ) {
				fprintf( stderr,
					"%s: error writing data: %s\n",
					Name, strerror( errno ) );
				exit( EX_CANTCREAT );
			};
		Rcpt_to ++;
	};
	if ( Rcpt_to == 0 )
		usage();

	fprintf( fdata, "DATA\n" );
	if ( ferror( fdata ) != 0 ) {
		fprintf( stderr,
			"%s: error writing data: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};
	file_cat( stdin, fdata, MAX_BLOCK );
	fprintf( fdata, ".\n" );
	if ( ferror( fdata ) != 0 ) {
		fprintf( stderr,
			"%s: error writing data: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};
	rc = fclose( fdata );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: error writing data: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};
	exit( EX_OK );
	return 0; /* gcc hat ein Problem */
}

/*---------------------------------------------------------------------------*
 *	end of file bsmtp.c
 *---------------------------------------------------------------------------*/
