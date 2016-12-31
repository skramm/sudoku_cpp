
#include <iostream>

#include "grid.h"

using namespace std;


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

		if( std::string( argv[1] ) == std::string( "-v" ) )
			grid._verbose = true;
	}
	grid.InitCandidates();

    cout << "Starting grid:\n" << grid << endl;
    if( grid._verbose )
		grid.PrintCandidates( cout, "start" );
    if( !grid.Check() )
    {
		std::cout << "Grid is invalid\n";
		return 1;
    }
    if( grid.Solve() )
		cout << "solved:\n";
	else
	{
		cout << "failure\n";
		grid.PrintCandidates( cout, "final" );
	}
	cout << grid;
    return 0;
}
