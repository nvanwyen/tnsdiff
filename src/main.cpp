//
// main.cpp
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

//
#include "app.h"

//
#if defined(DOS) || defined(_WIN32) || defined(WINSOCK)
#pragma comment( lib, "oci.lib" )
#endif

//
int main( int argc, char** argv )
{
    try
    {
        mti::app app;
        return app.run( argc, argv );
    }
    catch ( ... )
    {
        return 1;
    }
}
