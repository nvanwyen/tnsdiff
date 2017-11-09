//
// ver.cpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2004-2013 Metasystems Technologies Inc. (MTI)
// All rights reserved
//
// Distributed under the MTI Software License, Version 0.1.
//
// as defined by accompanying file MTI-LICENSE-0.1.info or
// at http://www.mtihq.com/license/MTI-LICENSE-0.1.info
//

// c
#include <stdio.h>

#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#else
#include <windows.h>
#endif

// c++
#include <iostream>
#include <string>
#include <ctime>

// boost

// local
#include "ver.h"

//
#if defined(DOS) || defined(WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(WINSOCK)
#pragma warning( disable : 4996 )
#endif

using namespace std;

//
void version(const char* name)
{
    ::fprintf( stderr, "%s [%s] - %s\n",
                       APPLICATION,
                       name,
                       VERSION );
}

//
void copyright()
{
    ::fprintf( stderr, "\n%s\n%s\n\n",
                       CORPORATION,
                       COPYWRITE );
}

//
string now()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[ 80 ];
    
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    
    strftime( buffer, 80, "%Y-%m-%d %I:%M:%S", timeinfo );
    string str( buffer );
    
    return str;
}

//
string stamp()
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[ 80 ];
    
    time( &rawtime );
    timeinfo = localtime( &rawtime );
    
    strftime( buffer, 80, "%Y%m%d%I%M%S", timeinfo );
    string str( buffer );
    
    return str;
}

//
string password()
{
    string passwd;

    cout << "Password: ";

#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
     termios oldt;
 
     ::tcgetattr( STDIN_FILENO, &oldt );
     termios newt = oldt;
 
     newt.c_lflag &= ~ECHO;
     ::tcsetattr( STDIN_FILENO, TCSANOW, &newt );
 
     getline( cin, passwd );
 
     ::tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
#else
    HANDLE hStdin = GetStdHandle( STD_INPUT_HANDLE ); 
    DWORD mode = 0;

    ::GetConsoleMode( hStdin, &mode );
    ::SetConsoleMode( hStdin, mode & (~ENABLE_ECHO_INPUT) );

    getline( cin, passwd );

    ::SetConsoleMode( hStdin, mode );
#endif

    cout << endl;

    return passwd;
}

//
bool exists( string file )
{
    bool ok = false;

#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
	struct stat stat;

    //
    if ( ::stat( file.c_str(), &stat ) == 0 )
    {
        if ( S_ISREG( stat.st_mode ) || S_ISLNK( stat.st_mode ) )
            ok = true;
    }
#else
	WIN32_FIND_DATA dat;
	HANDLE hdl = NULL;
	wstring tmp = wstring( file.begin(), file.end() );


	if ( ( hdl = ::FindFirstFile( (LPCSTR)(LPCTSTR)tmp.c_str(), &dat ) ) != INVALID_HANDLE_VALUE )
	{
		::FindClose( hdl );
		ok = true;
	}
	else
	{
		ok = false;
	}
#endif // !define...

	//
    return ok;
}
