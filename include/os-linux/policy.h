/*
 * policy.h: gemeinsame Parameter mit dem UUCP-System.
 *
 *           Diese Datei erhaelt man am einfachsten, wenn man den Taylor-
 *           UUCP installiert hat: einfach dessen policy.h hierhin
 *           kopieren.
 *
 *  Dies ist ein Ausschnitt aus "policy.h", aus der Taylor-UUCP Distribution.
 */
 
/* policy.h
   Configuration file for policy decisions.  To be edited on site.

   Copyright (C) 1991, 1992 Ian Lance Taylor

   This file is part of the Taylor UUCP package.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   The author of the program may be contacted at ian@airs.com or
   c/o Infinity Development Systems, P.O. Box 520, Waltham, MA 02254.
   */

/* This header file contains macro definitions which must be set by
   each site before compilation.  The first few are system
   characteristics that can not be easily discovered by the
   configuration script.  Most are configuration decisions that must
   be made by the local administrator.  */

/* If you use other programs that also lock devices, such as cu or
   uugetty, the other programs and UUCP must agree on whether a device
   is locked.  This is typically done by creating a lock file in a
   specific directory; the lock files are generally named
   LCK..something or LK.something.  If the LOCKDIR macro is defined,
   these lock files will be placed in the named directory; otherwise
   they will be placed in the default spool directory.  On some HDB
   systems the lock files are placed in /etc/locks.  On some they are
   placed in /usr/spool/locks.  On the NeXT they are placed in
   /usr/spool/uucp/LCK.  */
/* #define LOCKDIR "/usr/spool/uucp" */
/* #define LOCKDIR "/usr/local/uucp/lock" */
/* #define LOCKDIR "/etc/locks" */
/* #define LOCKDIR "/usr/spool/locks" */
/* #define LOCKDIR "/usr/spool/uucp/LCK" */
#define LOCKDIR "/var/lock"

/* You must also specify the format of the lock files by setting
   exactly one of the following macros to 1.  Check an existing lock
   file to decide which of these choices is more appropriate.

   The HDB style is to write the locking process ID in ASCII, passed
   to ten characters, followed by a newline.

   The V2 style is to write the locking process ID as four binary
   bytes in the host byte order.  Many BSD derived systems use this
   type of lock file, including the NeXT.

   SCO lock files are similar to HDB lock files, but always lock the
   lowercase version of the tty (i.e., LCK..tty2a is created if you
   are locking tty2A).  They are appropriate if you are using Taylor
   UUCP on an SCO Unix, SCO Xenix, or SCO Open Desktop system.

   SVR4 lock files are also similar to HDB lock files, but they use a
   different naming convention.  The filenames are LK.xxx.yyy.zzz,
   where xxx is the major device number of the device holding the
   special device file, yyy is the major device number of the port
   device itself, and zzz is the minor device number of the port
   device.

   Coherent use a completely different method of terminal locking.
   See unix/cohtty for details.  For locks other than for terminals,
   HDB type lock files are used.  */
#define HAVE_V2_LOCKFILES 0
#define HAVE_HDB_LOCKFILES 1
#define HAVE_SCO_LOCKFILES 0
#define HAVE_SVR4_LOCKFILES 0
#define HAVE_COHERENT_LOCKFILES 0
