//
// uri.h
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

#ifndef __URI_H
#define __URI_H

// c

// c++
#include <string>

// boost

// local

//
namespace mti {

//
class uri
{
    public:
        //
        uri( const std::string& url ) : proto_( "" ),
                                        host_( "" ),
                                        port_( 0 ),
                                        path_( "" ),
                                        query_( "" ) { parse( url ); }
        ~uri() {}

        //
        std::string protocol() { return proto_; }
        std::string host()     { return host_;  }
        int         port()     { return port_;  }

        //
        std::string path()     { return path_;  }
        std::string query()    { return query_; }

    protected:
    private:
        //
        void parse( const std::string& url );
    
        //
        std::string proto_;
        std::string host_;
        int         port_;
    
        //
        std::string path_;
        std::string query_;
};

} // mti

#endif // __URI_H
