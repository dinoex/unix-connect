
#include "xprog.c"

int main( int argc, const char **argv )
{
        initlog("testrun");
        minireadstat();

	runcommand( "/bin/echo", NULL );
	runcommand( "/bin/echo -n A", NULL );
	runcommand( "/bin/echo -n\\ A", NULL );

	exit( 0 );
}

