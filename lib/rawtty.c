/* $Id$ */
/*
 *  UNIX-Connect, a ZCONNECT(r) Transport and Gateway/Relay.
 *  Copyright (C) 1993-1994  Martin Husemann
 *  Copyright (C) 1995       Christopher Creutzig
 *  Copyright (C) 1999       Matthias Andree
 *  Copyright (C) 1999-2000  Dirk Meyer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------
 * Eine deutsche Zusammenfassung zum Copyright dieser Programme sowie der
 * GNU General Public License finden Sie in der Datei README.1st.
 * ------------------------------------------------------------------------
 *
 *  Bugreports, suggestions for improvement, patches, ports to other systems
 *  etc. are welcome. Contact the maintainer by e-mail:
 *  dirk.meyer@dinoex.sub.org or snail-mail:
 *  Dirk Meyer, Im Grund 4, 34317 Habichtswald
 *
 *  There is a mailing-list for user-support:
 *   unix-connect-users@lists.sourceforge.net,
 *  write a mail with subject "Help" to
 *   unix-connect-users-request@lists.sourceforge.net
 *  for instructions on how to join this list.
 */


/*
 * TTY Initialisierung auf raw-mode
 *
 * Datum        NZ  Aenderungen
 * ===========  ==  ===========
 * 15-Feb-1993  MH  System V R4 Version erstellt
 * 17-Feb-1993  MH  Generalisierung auf beliebige tty's
 * 26-Mar-1993  MH  Modem Hangup, neues Config-Schema
 * 04-Oct-1993  MH  SCO Line settings (from: lord@olis.north.de)
 * 28-Jan-1994	MH  Line speed settings
 * 20-Feb-1994	MH  Save and restore original line settings
 *
 */

# include "config.h"
# include <stdio.h>
# include "line.h"

#ifdef HAS_POSIX_TERMIOS
# include <termios.h>
#endif

#ifdef HAS_SYSV_TERMIO
# include <termio.h>
#endif

#ifdef HAS_BSD_SGTTY
# include <sgtty.h>
#endif

/*
 *  Eine weitere Anleihe von Taylor 1.04:
 */
/* Determine bits to clear for the various terminal control fields for
   HAS_SYSV_TERMIO and HAS_POSIX_TERMIOS.  */
/* Matthias Andree:
   Entruempelt und CRTSCTS-Handling konsistent gemacht */

/* common for SYSV and POSIX */
#if defined(HAS_SYSV_TERMIO) || defined(HAS_POSIX_TERMIOS)
#ifdef NEED_CRTSCTS
#define ISET_CFLAG (CS8 | CREAD | HUPCL | CRTSCTS)
#else
#define ISET_CFLAG (CS8 | CREAD | HUPCL)
#endif
#endif

/* specific */
#ifdef HAS_SYSV_TERMIO
#define ICLEAR_IFLAG (IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK \
		      | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC \
		      | IXON | IXANY | IXOFF)
#define ICLEAR_OFLAG (OPOST | OLCUC | ONLCR | OCRNL | ONOCR | ONLRET \
		      | OFILL | OFDEL | NLDLY | CRDLY | TABDLY | BSDLY \
		      | VTDLY | FFDLY)
#define ICLEAR_CFLAG (/* CBAUD | */ CLOCAL | CSIZE | PARENB | PARODD)
#define ICLEAR_LFLAG (ISIG | ICANON | XCASE | ECHO | ECHOE | ECHOK \
		      | ECHONL | NOFLSH)
#endif
#ifdef HAS_POSIX_TERMIOS
#define ICLEAR_LFLAG (ECHO | ECHOE | ECHOK | ECHONL | ICANON | IEXTEN \
		      | ISIG | NOFLSH | TOSTOP)
#define ICLEAR_OFLAG (OPOST)
#ifdef SCO
#define ICLEAR_IFLAG (BRKINT | ICRNL | IGNBRK | IGNCR | IGNPAR \
                      | INLCR | INPCK | ISTRIP \
                      | PARMRK)
#define ICLEAR_CFLAG (CLOCAL | CSIZE | CRTSFL | PARENB | PARODD )
#else /* !SCO */
#define ICLEAR_IFLAG (BRKINT | ICRNL | IGNBRK | IGNCR | IGNPAR \
		      | INLCR | INPCK | ISTRIP | IXOFF | IXON \
		      | PARMRK)
#define ICLEAR_CFLAG (CLOCAL | CSIZE | PARENB | PARODD)
#endif /* !SCO */
#endif

/*@@
 *
 * NAME
 *   set_rawmode - Initialisierung eines TTY's fuer transparenten Datentransfer
 *
 * VISIBILITY
 *   global
 *
 * SYNOPSIS
 *   #include "line.h"
 *   void set_rawmode(int mfileno)
 *
 * DESCRIPTION
 *   Die Funktion stellt das TTY 'mfileno' so ein, dass darueber eine
 *   transparente Datenkommunikation mit einem Modem bzw. einer Gegenstelle
 *   am anderen Ende der Leitung durchgefuehrt werden kann:
 *
 *     - Zeichen werden mit minimaler Verzoegerung vom Treiber an das Programm
 *       gemeldet
 *     - Der Kanal ist 8 bit transparent (Hardware-Handshake fuer Modem wird
 *       vorrausgesetzt)
 *     - Kein Echo
 *     - Keine Vor- oder Nachbehandlung von Ein-/Ausgabe
 *
 * PARAMETER
 *   Die File-Handle des geoffneten tty's
 *
 * RESULT
 *
 * BUGS
 *
 */

void
set_rawmode(int mfileno)
{
#ifdef HAS_SYSV_TERMIO
	struct termio term;

	if (ioctl(mfileno, TCGETA, &term) != 0)
		return;
	term.c_lflag &= ~ICLEAR_LFLAG;
	term.c_iflag &= ~ICLEAR_IFLAG;
	term.c_oflag &= ~ICLEAR_OFLAG;
	term.c_cflag &= ~ICLEAR_CFLAG;
	term.c_cflag |= ISET_CFLAG;
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	ioctl(mfileno, TCSETAW, &term);
#endif
#ifdef HAS_POSIX_TERMIOS
	struct termios term;

	if (tcgetattr(mfileno, &term) !=0 )
		return;
	cfmakeraw(&term);
	term.c_cflag |= ISET_CFLAG;
	term.c_cflag &= ~CLOCAL;
	tcsetattr(mfileno, TCSADRAIN, &term);
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb term;

	if (gtty(mfileno, &term) !=0 )
		return;
	term.sg_flags = RAW;
	stty(mfileno, &term);
#endif
#ifdef TIOCCAR
	ioctl(mfileno, TIOCCAR, 0);
#endif
}

/*
 *  hangup - legt das Modem auf.
 */

void
hangup(int mfileno)
{
#ifdef HAS_SYSV_TERMIO
	struct termio term;

	if (ioctl(mfileno, TCGETA, &term) != 0)
		return;
#endif
#ifdef HAS_POSIX_TERMIOS
	struct termios term;

	if (tcgetattr(mfileno, &term) !=0 )
		return;
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb term;

	if (gtty(mfileno, &term) !=0 )
		return;
#endif

#ifdef HAS_BSD_SGTTY
	term.sg_ispeed = B0;
	term.sg_ospeed = B0;
#else
	term.c_cflag = B0;
#endif
#ifdef HAS_SYSV_TERMIO
	ioctl(mfileno, TCSETAW, &term);
#endif
#ifdef HAS_POSIX_TERMIOS
	tcsetattr(mfileno, TCSADRAIN, &term);
#endif
#ifdef HAS_BSD_SGTTY
	stty(mfileno, &term);
#endif
}

void
set_local(int mfileno, int local)
{
#ifdef HAS_SYSV_TERMIO
	struct termio term;

	if (ioctl(mfileno, TCGETA, &term) != 0)
		return;
#endif
#ifdef HAS_POSIX_TERMIOS
	struct termios term;

	if (tcgetattr(mfileno, &term) !=0 )
		return;
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb term;

	if (gtty(mfileno, &term) !=0 )
		return;
#endif

	if (local)
#ifdef HAS_BSD_SGTTY
		term.sg_flags |= CLOCAL;
#else
		term.c_cflag |= CLOCAL;
#endif
	else
#ifdef HAS_BSD_SGTTY
		term.sg_flags &= ~CLOCAL;
#else
		term.c_cflag &= ~CLOCAL;
#endif
#ifdef HAS_SYSV_TERMIO
	ioctl(mfileno, TCSETAW, &term);
#endif
#ifdef HAS_POSIX_TERMIOS
	tcsetattr(mfileno, TCSADRAIN, &term);
#endif
#ifdef HAS_BSD_SGTTY
	stty(mfileno, &term);
#endif
}

typedef struct {
	const char * name;
	long val;
} bd_table_t;

bd_table_t bd_table[] = {
	{ "300",	B300 },
	{ "1200", 	B1200 },
	{ "2400", 	B2400 },
	{ "4800", 	B4800 },
	{ "9600",	B9600 },
#ifdef B19200
	{ "19200",	B19200 },
#else
#ifdef EXTA
	{ "19200",	EXTA },
#endif
#endif
#ifdef B38400
	{ "38400",	B38400 },
#else
#ifdef EXTB
	{ "38400",	EXTB },
#endif
#endif
#ifdef B57600
	{ "57600",	B57600 },
#endif
#ifdef B115200
	{ "115200",	B115200 },
#endif
	{ NULL,		0 }
};

void
set_speed(int mfileno, const char *speed)
{
	bd_table_t *p;
#ifdef HAS_SYSV_TERMIO
	struct termio term;
#endif
#ifdef HAS_POSIX_TERMIOS
	struct termios term;
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb term;
#endif

	for (p=bd_table; p->name; p++)
		if (strcmp(speed, p->name) == 0) break;
	if (p->name) {
	} else {
		fprintf (stderr, "FATAL: unbekannte Geschwindigkeit: %s\n",
			speed);
		exit(1);
	}
#ifdef HAS_SYSV_TERMIO
	if (ioctl(mfileno, TCGETA, &term) != 0)
		return;
	term.c_cflag &= (~CBAUD);
	term.c_cflag |= p->val;
	ioctl(mfileno, TCSETAW, &term);
#endif
#ifdef HAS_POSIX_TERMIOS
	if (tcgetattr(mfileno, &term) !=0 )
		return;
	cfsetospeed(&term, p->val);
	cfsetispeed(&term, p->val);
	tcsetattr(mfileno, TCSADRAIN, &term);
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb term;

	if (gtty(mfileno, &term) !=0 )
		return;
	term.sg_ispeed = p->val;
	term.sg_ospeed = p->val;
	stty(mfileno, &term);
#endif
}

/*
 * variable to hold original settings (some gettys will not recover them
 * after an outgoing call via zconnect) and one for our twiggled one
 */
#ifdef HAS_SYSV_TERMIO
	struct termio save_term;
#endif
#ifdef HAS_POSIX_TERMIOS
	struct termios save_term;
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb save_term;
#endif

void
save_linesettings(int mfileno)
{
#ifdef HAS_SYSV_TERMIO
	struct termio term;

	if (ioctl(mfileno, TCGETA, &term) != 0)
		return;
#endif
#ifdef HAS_POSIX_TERMIOS
	struct termios term;

	if (tcgetattr(mfileno, &term) !=0 )
		return;
#endif
#ifdef HAS_BSD_SGTTY
	struct sgttyb term;

	if (gtty(mfileno, &term) !=0 )
		return;
#endif
	save_term = term;
}


void
restore_linesettings(int mfileno)
{
#ifdef HAS_SYSV_TERMIO
	save_term.c_cflag |= ISET_CFLAG;
	ioctl(mfileno, TCSETAW, &save_term);
#endif
#ifdef HAS_POSIX_TERMIOS
	save_term.c_cflag |= ISET_CFLAG;
	tcsetattr(mfileno, TCSADRAIN, &save_term);
#endif
#ifdef HAS_BSD_SGTTY
	stty(mfileno, &save_term);
#endif
}


void
flush_modem(int mfileno)
{
#ifdef HAS_POSIX_TERMIOS
	tcflush(mfileno, TCIFLUSH);
#endif
}

