/* $Id$ */
/*
 *  Der folgende Code ist (leicht modifiziert) aus Taylor UUCP 1.04,
 *  Datei unix/serial.c.
 *
 *  Ich denke schon die ganze Zeit ueber eine Moeglichkeit der Integration
 *  von ZCONNECT und uucico nach, speziell um auch die UUCP-i und -a Protokolle
 *  nutzen zu koennen.
 */

/*
 *  Bitte in include/systems/xxx.h die passenden Konstanten (HAVE_SVR4_LOCKFILES)
 *  etc. deklarieren.
 */

#include "config.h"
#include "zconnect.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <sys/stat.h>
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#include "utility.h"
#include "policy.h"

#define FALSE	0
#define TRUE	1

extern int fsdo_lock (const char *);
extern int fsdo_unlock (const char *);

/* This routine is used for both locking and unlocking.  It is the
   only routine which knows how to translate a device name into the
   name of a lock file.  If it can't figure out a name, it does
   nothing and returns TRUE.  */

int
lock_device (int flok, const char *device)
{
  const char *z;
  char *zalc;
  int fret;

  zalc = NULL;
  z = NULL;
  if (z == NULL)
    {
#if ! HAVE_SVR4_LOCKFILES
      {
	const char *zbase;
	size_t clen;

	zbase = strrchr (device, '/') + 1;
	clen = strlen (zbase);
	zalc = dalloc (sizeof "LCK.." + clen);
	memcpy (zalc, "LCK..", sizeof "LCK.." - 1);
	memcpy (zalc + sizeof "LCK.." - 1, zbase, clen + 1);
#if HAVE_SCO_LOCKFILES
	{
	  char *zl;

	  for (zl = zalc + sizeof "LCK.." - 1; *zl != '\0'; zl++)
	    if (isupper (*zl))
	      *zl = tolower (*zl);
	}
#endif
	z = zalc;
      }
#else /* ! HAVE_SVR4_LOCKFILES */
#if HAVE_SVR4_LOCKFILES
      {
	struct stat s;

	if (stat (device, &s) != 0)
	  {
	    fprintf (deblogfile, "stat (%s): %s", device,
		  strerror (errno));
	    return FALSE;
	  }
	zalc = dalloc (sizeof "LK.123.123.123");
	sprintf (zalc, "LK.%03d.%03d.%03d", major (s.st_dev),
		 major (s.st_rdev), minor (s.st_rdev));
	z = zalc;
      }
#else /* ! HAVE_SVR4_LOCKFILES */
      z = strrchr (device, '/') + 1;
#endif /* ! HAVE_SVR4_LOCKFILES */
#endif /* ! HAVE_SVR4_LOCKFILES */
    }

  if (flok)
    fret = fsdo_lock (z);
  else
    fret = fsdo_unlock (z);

#if HAVE_COHERENT_LOCKFILES
  if (fret)
    {
      if (flok)
	{
	  if (lockttyexist (z))
	    {
	      fprintf (deblogfile, "%s: port already locked", device);
	      fret = FALSE;
	    }
	  else
	    fret = fscoherent_disable_tty (z, &qsysdep->zenable);
	}
      else
	{
	  fret = TRUE;
	  if (qsysdep->zenable != NULL)
	    {
	      const char *azargs[3];
	      int aidescs[3];
	      pid_t ipid;

	      azargs[0] = "/etc/enable";
	      azargs[1] = qsysdep->zenable;
	      azargs[2] = NULL;
	      aidescs[0] = SPAWN_NULL;
	      aidescs[1] = SPAWN_NULL;
	      aidescs[2] = SPAWN_NULL;

	      ipid = ixsspawn (azargs, aidescs, TRUE, FALSE,
			       (const char *) NULL, TRUE, TRUE,
			       (const char *) NULL, (const char *) NULL,
			       (const char *) NULL);
	      if (ipid < 0)
		{
		  fprintf (deblogfile, "ixsspawn (/etc/enable %s): %s",
			qsysdep->zenable, strerror (errno));
		  fret = FALSE;
		}
	      else
		{
		  if (ixswait ((unsigned long) ipid, (const char *) NULL)
		      == 0)
		    fret = TRUE;
		  else
		    fret = FALSE;
		}
	      dfree (qsysdep->zenable);
	      qsysdep->zenable = NULL;
	    }
	}
    }
#endif /* HAVE_COHERENT_LOCKFILES */

  dfree (zalc);
  return fret;
}

