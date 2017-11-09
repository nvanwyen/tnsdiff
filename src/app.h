//
// app.h
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

#ifndef __APP_H
#define __APP_H

// c

// c++
#include <string>
#include <vector>

// local
#include "tns.h"

//
namespace mti {

//
class app
{
    public:
        //
        enum input_type
        {
            home_type,
            file_type,
            ldap_type
        };

        //
        app() {}
        ~app() {}

        //
        int run( int argc, char** agv );

    protected:
        //
        struct item
        {
            mti::tns::entry entry;
            size_t index;

            item( mti::tns::entry e, 
                  size_t i ) : entry( e ),
                               index( i )
            {}
        };

        //
        struct result
        {
            //
            enum compared
            {
                none,
                added,
                removed,
                missing,
                changed
            };

            //
            item left;
            item right;

            compared differ;

            //
            result( item      l,
                    item      r,
                    compared  d ) : left( l ),
                                    right( r ),
                                    differ( d )
            {}
        };

        //
        typedef vector<result> results;
        typedef vector<result>::iterator event;

    private:
        //
        bool fdif_;         // flag; diff
        bool fmrg_;         //       merge

        //
        std::string ldev_;  // left input device (file or ldap)
        std::string rdev_;  // right ...

        //
        input_type ltyp_;   // left input type (file or ldap)
        input_type rtyp_;   // right ...

        //
        void usage( bool hlp = false );
        bool options( int c, char** v );

        //
        std::string load_home();
        mti::tns::entries load_entries( std::string dev, input_type typ );
        size_t load_differences( mti::tns::entries& lft, mti::tns::entries& rgt, app::results& chg );

        //
        input_type intype( std::string& in );

        //
        int diffs();
        int merge();

        //
        mti::tns::item find( mti::tns::entries& ent, std::string itm );
        event changes( app::results& chg, mti::tns::item& it );

        //
        std::string diff( mti::tns::entry& ent, std::string pre );
        std::string replace( std::string str, const std::string fm, const std::string to );

        //
        void debug();
        void debug( mti::tns::entries& ent );
        void debug( results& res );
};
}
#endif // __APP_H
