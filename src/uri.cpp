//
// uri.cpp
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

// c++
#include <string>
#include <cctype>
#include <algorithm>
#include <functional>

// boost
#include <boost/lexical_cast.hpp>

// local
#include "uri.h"

//
using namespace std;
using namespace mti;

//
void uri::parse( const string& url )
{
    const string sep( "://" );

    //
    string::const_iterator idx = search( url.begin(), url.end(),
                                         sep.begin(), sep.end() );

    //
    proto_.reserve( distance( url.begin(), idx ) );

    // toLower()
    transform( url.begin(), idx,
               back_inserter( proto_ ),
               ptr_fun<int,int>( tolower ) );

    //
    if ( idx != url.end()  )
    {
        //
        advance( idx, sep.length() );
        string::const_iterator pdx = find( idx, url.end(), '/' );

        //
        host_.reserve( distance( idx, pdx ) );

        // toLower()
        transform( idx, 
                   pdx,
                   back_inserter( host_ ),
                   ptr_fun<int,int>( tolower ) );

        //
        string::const_iterator qdx = find( pdx, url.end(), '?' );
        path_.assign( pdx, qdx );

        //
        if (  qdx != url.end()  )
            ++qdx;

        //
        query_.assign( qdx, url.end() );

        //
        const string sep( ":" );
        idx = search( host_.begin(), host_.end(),
                      sep.begin(), sep.end() );

        //
        if ( idx != host_.end() )
        {
            pdx = host_.end();
            advance( idx, 1 );

            //
            try { port_ = boost::lexical_cast<int>( string( idx, pdx ) ); }
            catch ( ... ) { }
        }
    }
}
