#!/usr/bin/perl -w
# $Id$

#  Aufruf: sendfile [-s Betreff] [-c <comment-file>] <data-file> <addr>*

while ($_ = $ARGV[0]) {
	shift;
	if (/^-s/) {
		$subj = $ARGV[0];
		shift;
		next;
	}
	if (/^-c/) {
		$com = $ARGV[0];
		shift;
		next;
	}
	if (! $file) {
		$file = $_;
		last;
	}
}

$adr = join(" ", @ARGV);

if (! $file || ! $adr) {
	print STDERR "Aufruf: sendfile  [-s Betreff] [-c <comment-file>] <data-file> <addr-spec>*\n";
	exit 1;
}

$subj = $file if (! $subj);

print STDERR "Versende $file ($subj) an $adr\n";
print STDERR "Kommentar: $com\n" if ($com);

open(MAIL, "|smail $adr") || die ("mail: $!\n");
print MAIL "Subject: $subj\n";
print MAIL "MIME-Version: 1.0\n";
if ($com) {
	print MAIL "Content-Type: multipart/mixed; boundary=\"--8<--\"\n";
} else {
	print MAIL "Content-Type: application/octet-stream\n";
	print MAIL "Content-Transfer-Encoding: X-uuencode\n";
}
print MAIL "\n";
if ($com) {
	print MAIL "This is a MIME mail. You may save it and read the first part,\n";
	print MAIL "the second part should be decoded by uudecode\n\n";
	print MAIL "----8<--\n";
	print MAIL "Content-Type: text/plain; charset=ISO-8859-1\n";
	print MAIL "Content-Transfer-Encoding: 8bit\n\n";
	open(COM, $com) || die("$com: $!\n");
	while (<COM>) {
		print MAIL $_;
	}
	close(COM);
	print MAIL "----8<--\n";
	print MAIL "Content-Type: application/octet-stream\n";
	print MAIL "Content-Transfer-Encoding: X-uuencode\n\n";
}
open(COM, "uuencode $file < $file |") || die ("uudecode: $!\n");
while (<COM>) {
	print MAIL $_;
}
close(COM);
close(MAIL);
