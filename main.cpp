
#include <iostream>

#include "grid.h"

using namespace std;

extern bool g_LogSteps;
extern bool g_Verbose;
extern int  g_NbSteps;

int main( int argc, const char** argv )
{
	Grid grid;
	if( argc > 1 )
	{
		cout << "Reading file " << argv[argc-1];
		if( !grid.Load( argv[argc-1] ) )
		{
			cout << ": failure\n";
			return 1;
		}
		cout << ": success\n";

		for( int i=0; i<argc-1; i++ )
		{
			if( std::string( argv[i+1] ) == std::string( "-v" ) )
				g_Verbose = true;
			if( std::string( argv[i+1] ) == std::string( "-s" ) )
				g_LogSteps = true;
		}

	}
	grid.InitCandidates();

    cout << "Starting grid:\n" << grid << endl;
    if( g_Verbose )
		grid.PrintCandidates( cout, "start" );
    if( !grid.Check() )
    {
		std::cout << "Grid is invalid\n";
		return 1;
    }
    if( grid.Solve() )
		cout << "-solved with " << g_NbSteps << " steps\n";
	else
	{
		cout << "failure, used " << g_NbSteps << " steps\n";
		grid.PrintCandidates( cout, "final" );
	}
	cout << grid;
    return 0;
}
