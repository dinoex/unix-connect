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
 *	rsmtp - batched mail handler
 *	============================
 *
 *	file: rsmtp.c
 *
 *	Last Edit-Date: Sat Apr 10 20:12:54 CEST 1999
 *
 *	-dm 1.70
 *	fixed de-escaping
 *	-dm 1.69
 *	fixed multiple RCPT TO:
 *	-dm 1.68
 *	option "-strict" added
 *	option "-gzip" added
 *	more arguments for SENDMAIL
 *	-dm 1.67
 *
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

/*---------------------------------------------------------------------------*
 *	configuration
 *---------------------------------------------------------------------------*/

#include "sysexits2.h"

#ifndef _PATH_SENDMAIL
#include <paths.h>
#endif
#define SENDMAIL	_PATH_SENDMAIL

#ifndef GZIP
#define GZIP		"/usr/bin/gzip"
#endif

#define	RCPT_MAX	1024

/*---------------------------------------------------------------------------*
 *	some macros
 *---------------------------------------------------------------------------*/

#define ON	1
#define OFF	0

#define PIPE_IN		0
#define PIPE_OUT	1

#define	LF	'\n'
#define	CR	'\r'

/* this is not a limit for input lines */
#define INPUT_LINE_MAX	1020

#include "version.h"

#define	SENDMAIL_ARGC	6

/*---------------------------------------------------------------------------*
 *	global instances
 *---------------------------------------------------------------------------*/

const char *const C_ = "";
const char *const SMTP_HELO = "HELO ";
const char *const SMTP_EHLO = "EHLO ";
const char *const SMTP_FROM = "MAIL FROM:";
const char *const SMTP_TO = "RCPT TO:";
const char *const SMTP_DATA = "DATA";
const char *const SMTP_QUIT = "QUIT";
const char *const BSMTP_END = "\r\n";
const char *const BSMTP_END2 = "\n";
const char *const Gzip = GZIP;
const char *const Sendmail = SENDMAIL;

char *Line_ptr = NULL;
int Flag_verbose = OFF;
int Flag_gzip = OFF;
int Flag_strict = OFF;

const char *Name;
const char *From_host;
const char *From_user;
const char *Eargv[ RCPT_MAX + 7 ];
char Line_buffer[ INPUT_LINE_MAX + SENDMAIL_ARGC + 1 ];
int Long_line;

/*---------------------------------------------------------------------------*
 *	merge two buffers to one string
 *---------------------------------------------------------------------------*/

static char *
merge_strings( const char *const s1, const char *const s2 )
{
	char *new_ptr;
	size_t len;
	size_t s1_len;

	s1_len = strlen( s1 );
	len = s1_len + strlen( s2 );

	new_ptr = malloc( len + 1 );
	if ( new_ptr == NULL ) {
		fprintf( stderr,
			"%s: out of memory,"
			" requesting %ld bytes for merge\n",
			Name, (long)len );
		exit( EX_TEMPFAIL );
	};
	strcpy( new_ptr, s1 );
	strcat( new_ptr + s1_len, s2 );
	return new_ptr;
}

/*---------------------------------------------------------------------------*
 *	get single line in buffer
 *---------------------------------------------------------------------------*/

static int
read_single_line( void )
{
	char *cptr;
	size_t len;

	do {
		Long_line = OFF;
		strcpy( Line_buffer, C_ );
		cptr = fgets( Line_buffer, INPUT_LINE_MAX, stdin );
		len = strlen( Line_buffer );

		if ( cptr == NULL ) {
			if ( len == 0 )
				return OFF;
			/* parse a line with EOF as a complete line */
		} else {
			/* paranoid */
			if ( len == 0 )
				continue;

			/* strip LF at the end of line */
			if ( Line_buffer[ len - 1 ] != LF ) {
				Long_line = ON;
			} else {
				Line_buffer[ -- len ] = 0;
				if ( len == 0 )
					continue;
			}
		};
		if ( Long_line == OFF ) {
			/* strip CR at the end of line */
			if ( Line_buffer[ len - 1 ] == CR ) {
				Line_buffer[ -- len ] = 0;
				if ( len == 0 )
					continue;
			}
		};

		break;

	} while ( ON );
	return ON;
}

/*---------------------------------------------------------------------------*
 *	get long lines
 *---------------------------------------------------------------------------*/

static int
read_long_line( void )
{
	int rc;
	char *backup_ptr;

	if ( Line_ptr != Line_buffer )
		free( Line_ptr );
	Line_ptr = NULL;
	do {
		/* save start of line */
		if ( Line_ptr == NULL ) {
			if ( Line_ptr == Line_buffer ) {
				Line_ptr = strdup( Line_ptr );
				if ( Line_ptr == NULL ) {
					fprintf( stderr,
					"%s: out of memory,"
					" requesting %ld bytes for line\n",
					Name, (long)strlen( Line_ptr ) );
					exit( EX_TEMPFAIL );
				}
			}
		};

		rc = read_single_line();
		if ( rc == OFF )
			return rc;

		if ( Line_ptr == NULL ) {
			Line_ptr = Line_buffer;
		} else {
			/* merge */
			backup_ptr = Line_ptr;
			Line_ptr = merge_strings( backup_ptr, Line_buffer );
			free( backup_ptr );
		}
	} while ( Long_line != OFF );
	return ON;
}

/*---------------------------------------------------------------------------*
 *	decompress data from stdin
 *	using "gzip" which supports "decompress" too
 *---------------------------------------------------------------------------*/

static void
uncompress( void )
{
	int rc;
	int phandle[ 2 ];
	pid_t pid;

	rc = pipe( phandle );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: create pipe failed: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	pid = fork();
	if ( pid < 0 ) {
		fprintf( stderr,
			"%s: can't fork: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	/* child process */
	if ( pid == 0 ) {
		rc = close( phandle[ PIPE_IN ] );
		if ( rc != 0 ) {
			fprintf( stderr,
				"%s: close pipe0 failed: %s\n",
				Name, strerror( errno ) );
			exit( EX_CANTCREAT );
		};

		/* return data to stdout */
		if ( phandle[ PIPE_OUT ] != STDOUT_FILENO ) {
			rc = dup2( phandle[ PIPE_OUT ], STDOUT_FILENO );
			/* STDOUT may not exist, but we don't care */
			if ( ( rc != 0 ) && ( errno != ENOENT ) ) {
				fprintf( stderr,
					"%s: dup2 stdout failed: %s\n",
					Name, strerror( errno ) );

				exit( EX_CANTCREAT );
			}
		};

		/* set up args and exec */
		Eargv[ 0 ] = Gzip;
		Eargv[ 1 ] = "-dc";
		Eargv[ 2 ] = NULL;
		execv( Gzip, (char *const *)Eargv );
		fprintf( stderr,
			"%s: exec %s failed: %s\n",
			Name, Gzip, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

        /* parent process */
	rc = close( phandle[ PIPE_OUT ] );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: close pipe1 failed: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	/* get data from pipe */
	if ( phandle[ PIPE_IN ] != STDIN_FILENO ) {
		rc = dup2( phandle[ PIPE_IN ], STDIN_FILENO );
		if ( rc != 0 ) {
			fprintf( stderr,
				"%s: dup2 stdin failed: %s\n",
				Name, strerror( errno ) );
			exit( EX_CANTCREAT );
		}
	}
}

/*---------------------------------------------------------------------------*
 *	feed the mail for delivery
 *---------------------------------------------------------------------------*/

static void
feed_mail( void )
{
	int rc;
	long nr_to;
	size_t len;
	int phandle[ 2 ];
	pid_t pid;
	pid_t wpid;
	int child_stat;

	/* read all rcpt */
	for ( nr_to = 0 ; nr_to < RCPT_MAX ; nr_to ++ ) {
		rc = read_long_line();
		if ( rc == OFF ) {
			fprintf( stderr,
				"%s: unexpected end of file in RCPT TO:\n",
				Name );
			exit( EX_DATAERR );
		};
		if ( strcasecmp( Line_ptr, SMTP_DATA ) == 0 )
			break;

		len = strlen( SMTP_TO );
		if ( strncasecmp( Line_ptr, SMTP_TO, len  ) != 0 ) {
			fprintf( stderr,
				"%s: SMTP <%s> not found\n",
				Name, SMTP_TO  );
			exit( EX_DATAERR );
		};

		Eargv[ SENDMAIL_ARGC + nr_to ] = strdup( Line_ptr + len );
		if ( Eargv[ SENDMAIL_ARGC + nr_to ] == NULL ) {
			fprintf( stderr,
				"%s: out of memory,"
				" requesting %ld bytes for TO\n",
				Name, (long)strlen( Line_ptr + len ) );
			exit( EX_TEMPFAIL );
		};
	};

	if ( nr_to == 0 ) {
		fprintf( stderr,
			"%s: SMTP <%s> not found\n",
			Name, SMTP_TO  );
		exit( EX_DATAERR );
	};

	rc = pipe( phandle );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: create pipe failed: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	pid = fork();
	if ( pid < 0 ) {
		fprintf( stderr,
			"%s: can't fork: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	/* child process */
	if ( pid == 0 ) {

		rc = close( phandle[ PIPE_OUT ] );
		if ( rc != 0 ) {
			fprintf( stderr,
				"%s: close pipe1 failed: %s\n",
				Name, strerror( errno ) );
			exit( EX_CANTCREAT );
		};

		/* get data from pipe */
		if ( phandle[ PIPE_IN ] != STDIN_FILENO ) {
			rc = dup2( phandle[ PIPE_IN ], STDIN_FILENO );
			if ( rc != 0 ) {
				fprintf( stderr,
					"%s: dup2 stdin failed: %s\n",
					Name, strerror( errno ) );
				exit( EX_CANTCREAT );
			}
		};

		/* set up args and exec */
		Eargv[ 0 ] = Sendmail;
		Eargv[ 1 ] = merge_strings( "-f", From_user );
		Eargv[ 2 ] = merge_strings("-pBSMTP:", From_host );
		Eargv[ 3 ] = "-oem";
		Eargv[ 4 ] = "-odb";
		Eargv[ 5 ] = "-oi";
		Eargv[ 6 + nr_to ] = NULL;

		if ( Flag_verbose != OFF ) {
			long j;
			fprintf( stderr, "exec:" );
			for ( j = 0; j < 6 + nr_to; j ++ )
				fprintf( stderr, " %s", Eargv[ j ] );
			fprintf( stderr, "\n" );
		};

		execv( Sendmail, (char *const *)Eargv );
		fprintf( stderr,
			"%s: exec %s failed: %s\n",
			Name, Sendmail, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

        /* parent process */
	rc = close( phandle[ PIPE_IN ] );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: close pipe0 failed: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	/* write data to pipe */
	Long_line = OFF;
	do {
		char *cptr;
		size_t wr_len;
		int first_part;
		int last_line;

		if ( Long_line == OFF )
			first_part = ON;
		else
			first_part = OFF;

		strcpy( Line_buffer, C_ );
		cptr = fgets( Line_buffer, INPUT_LINE_MAX, stdin );
		len = strlen( Line_buffer );

		if ( cptr == NULL ) {
			if ( len == 0 ) {
				fprintf( stderr,
					"%s: error read from stdin,"
					" %ld bytes\n",
					Name, (long)len );
				exit( EX_DATAERR );
			};
			/* parse a line with EOF as a complete line */
			Long_line = OFF;
		} else {
			/* paranoid */
			if ( len == 0 )
				continue;

			/* detect LF at the end of line */
			if ( Line_buffer[ len - 1 ] != LF )
				Long_line = ON;
			else
				Long_line = OFF;
		};

		cptr = Line_buffer;
		last_line = OFF;
		do {
			if ( first_part == OFF )
				break;
			if ( Long_line != OFF )
				break;

			if ( Line_buffer[ 0 ] != '.' )
				break;

			/* Remove extra Dot */
			cptr ++;
			len = strlen( cptr );

			if ( strcmp( cptr, BSMTP_END ) == 0 ) {
				last_line = ON;
				break;
			};

			if ( strcmp( cptr, BSMTP_END2 ) == 0 ) {
				last_line = ON;
				break;
			};

			break;
		} while ( OFF );

		if ( last_line != OFF )
			break;

		wr_len = write( phandle[ PIPE_OUT ], cptr, len );
		if ( wr_len != len ) {
			fprintf( stderr,
				"%s: error write to pipe,"
				" %ld bytes of %ld written\n",
				Name, (long)wr_len, (long)len );
			exit( EX_IOERR );
		};

	} while ( ON );

	rc = close( phandle[ PIPE_OUT ] );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: close pipe1 failed: %s\n",
			Name, strerror( errno ) );
		exit( EX_CANTCREAT );
	};

	wpid = waitpid( pid, &child_stat, 0 );
	if ( wpid != pid ) {
		fprintf( stderr,
			"%s: waitpid failed: %s\n",
			Name, strerror( errno ) );
		exit( EX_TEMPFAIL );
	};

	if ( ! WIFEXITED( child_stat ) ) {
		fprintf( stderr,
			"%s: waitpid returns: %s\n",
			Name, strerror( errno ) );
		exit( EX_TEMPFAIL );
	};

	rc = WEXITSTATUS( child_stat );
	if ( rc != 0 ) {
		fprintf( stderr,
			"%s: child exited with %d\n",
			Name, rc );
		if ( Flag_strict != OFF )
			exit( EX_TEMPFAIL );
	}
}

/*---------------------------------------------------------------------------*
 *	display help
 *---------------------------------------------------------------------------*/

static void
usage( void )
{
	fprintf( stderr, START_MESSAGE, "rsmtp" );
	fprintf( stderr,
"\n"
" Usage: | rsmtp  [ -verbose ] [ -strict ] [ -gzip ]\n"
"        | rcsmtp [ -verbose ] [ -strict ]\n"
"        | rgsmtp [ -verbose ] [ -strict ]\n"
"\n" );
	exit( EX_USAGE );
}

/*---------------------------------------------------------------------------*
 *	parse comand line and execute
 *---------------------------------------------------------------------------*/

int
main( int argc, const char *const *argv )
{
	const char	*cptr;
	char		ch;
	char		*buffer;
	char		*name;
	char		*work;
	size_t		len;
	int		rc;

	Name = argv[ 0 ];

	/* check for commandline */
	while ( --argc > 0 ) {
		cptr = *(++argv);
		if ( *cptr == '-' ) {
			ch = *(++cptr);
			switch ( tolower( ch ) ) {
			case 'g':
				Flag_gzip = ON;
				break;
			case 's':
				Flag_strict = ON;
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
		usage();
	};

	/* find basename */
	buffer = strdup( Name );
	if ( buffer == NULL ) {
		fprintf( stderr,
			"%s: out of memory,"
			" requesting %ld bytes for NAME\n",
			Name, (long)strlen( Name ) );
		exit( EX_TEMPFAIL );
	};
	work = strrchr( buffer, '/' );
	if (work == NULL ) {
		name = buffer;
	} else {
		name = work + 1;
	};
	if ( strcmp( name, "rcsmtp" ) == 0 ) {
		Flag_gzip = ON;
	};
	if ( strcmp( name, "rgsmtp" ) == 0 ) {
		Flag_gzip = ON;
	};
	if ( Flag_gzip != OFF ) {
		uncompress();
	};
	free( buffer );

	/* parse host */
	rc = read_long_line();
	if ( rc == OFF ) {
		fprintf( stderr,
			"%s: empty file\n",
			Name );
		exit( EX_DATAERR );
	};
	do {
		len = strlen( SMTP_HELO );
		if ( strncasecmp( Line_ptr, SMTP_HELO, len ) == 0 )
			break;
		len = strlen( SMTP_EHLO );
		if ( strncasecmp( Line_ptr, SMTP_EHLO, len ) == 0 )
			break;
		fprintf( stderr,
			"%s: SMTP <%s> not found\n",
			Name, SMTP_HELO );
		exit( EX_DATAERR );
	} while ( OFF );

	From_host = strdup( Line_ptr + len );
	if ( From_host == NULL ) {
		fprintf( stderr,
			"%s: out of memory,"
			" requesting %ld bytes for HOST\n",
			Name, (long)strlen( Line_ptr + len ) );
		exit( EX_TEMPFAIL );
	};

	do {
		rc = read_long_line();
		if ( rc == OFF ) {
			fprintf( stderr,
				"%s: unexpected end of file in DATA\n",
				Name );
			exit( EX_DATAERR );
		};
		if ( strcasecmp( Line_ptr, SMTP_QUIT ) == 0 )
			exit( EX_OK );

		len = strlen( SMTP_FROM );
		if ( strncasecmp( Line_ptr, SMTP_FROM, len  ) != 0 ) {
			fprintf( stderr,
				"%s: SMTP <%s> not found\n",
				Name, SMTP_FROM  );
			exit( EX_DATAERR );
		};

		From_user = strdup( Line_ptr + len );
		if ( From_user == NULL ) {
			fprintf( stderr,
				"%s: out of memory,"
				" requesting %ld bytes for FROM\n",
				Name, (long)strlen( Line_ptr + len ) );
			exit( EX_TEMPFAIL );
		};

		feed_mail();

	} while ( ON );
	exit( EX_CONFIG );
	return 0; /* gcc hat ein Problem */
}

/*---------------------------------------------------------------------------*
 *	end of file rsmtp.c
 *---------------------------------------------------------------------------*/
