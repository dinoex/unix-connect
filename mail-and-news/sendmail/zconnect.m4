# .../cf/mailer/zconnect.m4
# enthält den Sendmail-Mailer Mzconnect
#
PUSHDIVERT(-1)

ifdef(`ZCONNECT_MAILER_PATH',,
        `define(`ZCONNECT_MAILER_PATH',
/usr/local/lib/zconnect/mail.zconnect)')
ifdef(`ZCONNECT_MAILER_MAX',,
        `define(`ZCONNECT_MAILER_MAX', 1000000)')
POPDIVERT
###################################################################
###   ZCONNECT Mailer specification (for `Unix-connect-0.7xx')  ###
###################################################################

VERSIONID(`@(#)zconnect.m4      Nora E. Etukudo, Berlin, 19.11.1995')

Mzconnect,
        P=ZCONNECT_MAILER_PATH, F=DFMhnuSx, S=72/31,
        R=ifdef(`ALL_MASQUERADE_', `31', ),
        M=ZCONNECT_MAILER_MAX,
        A=mail.zconnect $u $h $f
