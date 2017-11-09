//
// tns.h
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

#ifndef __TNS_H
#define __TNS_H

// c

// c++
#include <string>
#include <vector>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

// local
#include "exp.h"
#include "uri.h"

//
typedef struct ldap LDAP;

//
//using namespace std;

//
#define UNKNOWN_ORA     0
#define TNSNAMES_ORA    1
#define LDAP_ORA        2
#define SQLNET_ORA      4

//
namespace mti {

//
class tns
{
    public:
        //
        class entry
        {
            public:
                //
                enum origin { UNKNOWN, TNSNAMES, LDAP, EZCONNECT };

                //
                std::string name;
                std::string desc;
                origin      type;

                entry() : name( "" ), desc( "" ), type( UNKNOWN )
                {}

                entry( std::string n, std::string d, int t = UNKNOWN )
                {
                    name = n;
                    desc = d;
                    type = (origin) t;
                }

                //
                std::string source()
                {
                    std::string t;

                    switch ( type )
                    {
                        case TNSNAMES:
                            t = "TNSNAMES";
                            break;

                        case LDAP:
                            t = "LDAP";
                            break;

                        case EZCONNECT:
                            t = "EZCONNECT";
                            break;

                        default:
                            t = "UNKNOWN";
                    }

                    return t;
                }

                //
                bool operator==( const entry& ent ) const
                {
                    return name == ent.name;
                }

                //
                bool operator<( const entry& ent ) const
                {
                    return name < ent.name;
                }

                //
                bool operator>( const entry& ent ) const
                {
                    return ( ! operator<( ent ) );
                }

            protected:
            private:
        };

        //
        typedef vector<entry>        entries;
        typedef vector<std::string>  methods;

        typedef vector<entry>::iterator       item;
        typedef vector<std::string>::iterator method;

        //
        class store
        {
            //
            public:
                //
                std::string host;
                int    port;
                int    pssl;
                std::string root;
                //
                bool   secure;
                //
                std::string dn;
                std::string pw;
        
                //
                store()
                    : host( "" ), port( 0 ), pssl( 0 ), secure( false ), root( "" ) {}
        
                store( std::string h, int p )
                    : host( h ),  port( p ), pssl( 0 ), secure( false ), root( "" ) {}
        
                store( std::string h, int p, bool s )
                    : host( h ),  port( p ), pssl( 0 ), secure( s ),     root( "" ) {}
        
                store( std::string h, int p, bool s, std::string r )
                    : host( h ),  port( p ), pssl( 0 ), secure( s ),     root( r )  {}
        
                //
                store( std::string u ) { url( u ); }
        
                //
                ~store() {}
        
                //
                void url( std::string u )
                {
                    uri i( u ); 
        
                    //
                    host = i.host();
                    port = i.port();
        
                    //
                    if ( i.protocol().substr( i.protocol().length() - 1 ) == "s" )
                        secure = true;
                }
        
                //
                std::string url()
                {
                    std::string u;
        
                    //
                    if ( ! ok() )
                        throw exp( "Invalid ldap store object!", EXP_INVALID );
        
                    //
                    u = ( ( secure ) ? "ldaps" : "ldap" ) 
                           + std::string( "://" ) 
                           + host;
        
                    //
                    if ( ! ( ( secure && ( port == 636 ) ) || ( ! secure && ( port == 389 ) ) ) )
                    {
                        u += std::string( ":" ) 
                           + boost::lexical_cast<std::string>( port );
                    }
        
                    return u;
                }
        
                //
                bool ok()
                {
                    return ( ( host.length() > 0 ) && ( port > 0 ) );
                }

            protected:
            private:
        };
        
        //
        tns() {}
        ~tns() {}

        //
        entry& find( std::string ent );

        //
        std::string tnsadmin();
        std::string tnsadmin( std::string dir );

        //
        std::string sqlnet();
        std::string sqlnet( std::string filename );

        //
        std::string tnsnames();
        std::string tnsnames( std::string filename );

        //
        std::string ldap();
        std::string ldap( std::string filename );

        //
        store ldapstore();
        store ldapstore( store sto );

        //
        void set_store( std::string uri );

        //
        size_t load_methods();
        size_t load_tnsnames();
        size_t load_ldap();

        //
        size_t load_ldif( streambuf* buf );

        //
        bool save_ldap( entry& ent, bool repl = true );

        //
        methods tns_methods() { return methods_; }
        entries tns_entries() { return entries_; }

        //
        void sort_entries( entries& ent );

        //
        std::string pretty( entry& ent );

        //
        store resolve_directory();

        //
        std::string locate_tnsnames();                   // locate the $OH/tnsnames.ora file in use
        std::string locate_ldapfile();                   //            $OH/ldap.ora file

        //
        void resolve_home();

    protected:
    private:
        //
        entry entry_;

        //
        std::string tnsadmin_;

        // files
        std::string sqlnet_;

        //
        std::string tnsnames_;
        std::string ldap_;

        // ldap store
        tns::store store_;

        // ldap.ora
        std::string root_;              // root context

        // sqlnet.ora
        methods methods_;               // connection type (from sqlnet.ora)

        // tnsnames.ora and ldap://...
        entries entries_;               // list of all entries

        //
        std::string trim( std::string str );
        //
        std::string ltrim( std::string str );
        std::string rtrim( std::string str );

        //
        std::string lcase( std::string str );
        std::string ucase( std::string str );

        //
        std::string split( std::string str, int idx, std::string tok = "," );

        //
        bool mesg();
        bool ldap_exists( LDAP* ld, char* dn );
        bool ldap_delete( LDAP* ld, char* dn );

        //
        size_t add( std::string name, std::string desc, entry::origin type );
        size_t add( entry ent );

        // resolve a single entry
        entry resolve( std::string name, std::string desc );

        //
        std::string env( std::string var );         // get the value of a environment variable
        bool exists( std::string file );            // test if a file exists

        //
        std::string locate( std::string name );     // find a file based on the TNS search order

        //
        std::string format( std::string str );

        //
        std::string resolve( std::string ent );
        std::string compare( std::string dsc );
};

//
typedef boost::shared_ptr<tns> tnsnames;

}
#endif // __TNS_H
