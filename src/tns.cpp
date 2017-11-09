//
// tns.cpp
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
#if defined(DOS) || defined(_WIN32) || defined(WINSOCK)
#pragma comment( lib, "oci.lib" )
#endif

// c
#include <stdlib.h>
#include <string.h>

// c++
#include <cctype>
#include <sstream>
#include <fstream>
#include <iostream> 
#include <stdexcept>
#include <algorithm>
#include <functional>

// boost

// local
#include "env.h"
#include "tns.h"
#include "ldp.h"
#include "ver.h"

//
#if defined(DOS) || defined(WINDOWS) || defined(WIN32) || defined(_WIN32) || defined(WINSOCK)
#pragma warning( disable : 4018 )
#endif

//
#define ATTR_IDX_CN                 0
//
#define ATTR_VAL_CN                 "cn"
#define ATTR_VAL_DESC               "orclnetdescstring"
#define ATTR_VAL_ALIAS              "aliasedobjectname"


#define LDAP_SEARCH                 "(|(|(objectclass=orclNetService)"       \
                                        "(objectclass=orclService))"         \
                                        "(objectclass=orclNetServiceAlias))"
// (|(|(objectclass=orclNetService)(objectclass=orclService))(objectclass=orclNetServiceAlias))


//
#ifndef ETC_DIR
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
#define ETC_DIR                     "/etc"
#else
#define ETC_DIR                     "\\"
#endif
#endif // ETC_DIR

//
#ifndef TNSNAMES_FILE
#define TNSNAMES_FILE               "tnsnames.ora"
#endif // TNSNAMES_FILE

//
#ifndef SQLNET_FILE
#define SQLNET_FILE                 "sqlnet.ora"
#endif // SQLNET_FILE

//
#ifndef LDAP_FILE
#define LDAP_FILE                   "ldap.ora"
#endif // LDAP_FILE

//
#ifndef WHITESPACE
#define WHITESPACE                  " \t"
#endif // WHITESPACE

//
#ifndef LDAP_ERROR
#define LDAP_ERROR                  -1
#endif // LDAP_ERROR

//
#ifndef LDAP_TIME_LIMIT
#define LDAP_TIME_LIMIT              0
#endif // LDAP_TIME_LIMIT

//
#ifndef MISSING_MESG_FILE
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
#define MISSING_MESG_FILE           "LDAP Method encountered, but the $ORACLE_HOME/ldap/mesg/ldapus.msb is missing!"
#else
#define MISSING_MESG_FILE           "LDAP Method encountered, but the %ORACLE_HOME%\\ldap\\mesg\\ldapus.msb is missing!"
#endif // !define...
#endif // MISSING_MESG_FILE

#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
#define NL                          "\n"
#else
#define NL                          "\r\n"
#endif // !WINDOWS

#define PADDING                     "    "

//
namespace mti {

//
std::string tns::trim( std::string str )
{
    return rtrim( ltrim( str ) );
}

//
std::string tns::ltrim( std::string str )
{
    size_t pos = str.find_first_not_of( WHITESPACE );
    return ( pos == std::string::npos ) ? "" : str.substr( pos );
}

//
std::string tns::rtrim( std::string str )
{
    size_t pos = str.find_last_not_of( WHITESPACE );
    return ( pos == std::string::npos ) ? "" : str.substr( 0, pos + 1 );
}

//
std::string tns::lcase( std::string str )
{
    transform( str.begin(), str.end(),str.begin(), ::tolower );
    return str;
}

//
std::string tns::ucase( std::string str )
{
    transform( str.begin(), str.end(),str.begin(), ::toupper );
    return str;
}

//
std::string tns::split( std::string str, int idx, std::string tok /*= ","*/ )
{
    int   itm = 0;
    char* dat;

#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
    //
    dat = ::strtok( &str[ 0 ], tok.c_str() );

    //
    while( dat != NULL )
    {
        if ( ++itm > idx )
            break;

        dat = ::strtok( NULL, tok.c_str() );
    }
#else
	char* nxt = NULL;

	dat = ::strtok_s( &str[ 0 ], tok.c_str(), &nxt );

	 while ( dat != NULL )
	 {
        if ( ++itm > idx )
            break;

		dat = ::strtok_s( NULL, tok.c_str(), &nxt );
	 }
#endif

	return trim( std::string( ( dat == NULL ) ? "" : dat ) );
}

//
bool tns::mesg()
{
    // The file ldapus.msb must be located in the $ORACLE_HOME/ldap/mesg/
    // directory which more than likely on an Instant Client installation will
    // not exist, but you can find a copy, as a courtesy, in the
    // <oraping_source>/mesg/ directory here within

    // The message file (ldapus.msb) in the correct directory before oraping
    // will search LDAP for TNS Descriptions
    //
    // Example:
    //     sudo mkdir -p $ORACLE_HOME/ldap/mesg/
    //     sudo cp ldapus.msb.11.2.0.3 $ORACLE_HOME/ldap/mesg/
    //     ( cd $ORACLE_HOME/ldap/mesg/ ; sudo ln -s ldapus.msb.11.2.0.3 ldapus.msb )

    // Looking for the message file $ORACLE_HOME/ldap/mesg/ldapus.msb
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
    return ::exists( env( "ORACLE_HOME" ) + "/ldap/mesg/ldapus.msb" );
#else
    return ::exists( env( "ORACLE_HOME" ) + "\\ldap\\mesg\\ldapus.msb" );
#endif
}

//
size_t tns::add( std::string name, std::string desc, entry::origin type )
{
    return add( entry( name, desc, type ) );
}

size_t tns::add( entry ent )
{
    //
    entries_.push_back( ent );

    //
    return entries_.size();
}

//
tns::entry tns::resolve( std::string name, std::string desc )
{
    entry e;

    //
    for ( item itm = entries_.begin(); itm != entries_.end(); ++ itm )
    {
        //
        if ( format( itm->name ) == format( name ) )
        {
            e.name = itm->name;
            e.desc = itm->desc;
            e.type = itm->type;

            break;
        }
        else
        {
            if ( format( itm->desc ) == format( name ) )
            {
                e.name = itm->name;
                e.desc = itm->desc;
                e.type = itm->type;

                break;
            }
        }
    }

    //
    if ( e.desc.length() == 0 )
    {
        e.desc = name;
        e.type = entry::EZCONNECT;
    }

    e.name = format( e.name );
    e.desc = format( e.desc );

    return e;
}

//
tns::entry& tns::find( std::string ent )
{
    //
    std::string desc;

    //
    if ( tnsnames().length() == 0 )
        throw exp ( "TNS*Names file not found!", EXP_MISSING );

    //
    if ( sqlnet().length() == 0 )
        throw exp ( "SQL*Net file not found!", EXP_MISSING );

    //
    if ( ldap().length() == 0 )
        throw exp ( "LDAP*Net file not found!", EXP_MISSING );

    //
    if ( load_methods() > 0 )
    {
        //
        typedef vector<std::string>::iterator itype;

        //
        for ( itype i = methods_.begin(); i != methods_.end(); ++i )
        {
            //
            if ( ( *i == "TNSNAMES" ) || ( *i == "EZCONNECT" ) )
			{
				try
				{
					load_tnsnames();
				}
				catch ( exp& x )
				{
					throw exp ( x.what(), x.code() );
				}
			}

			if ( *i == "LDAP" )
			{
				try
				{
					load_ldap();
				}
				catch ( exp& x )
				{
					throw exp ( x.what(), x.code() );
				}
			}

            if ( *i == "ONAMES" )
                throw exp ( "Unsupported naming type [" + (*i) + "]!", EXP_UNSUPPORTED );
        }
    }
    else
	{
		try
		{
			load_tnsnames();
		}
		catch ( exp& x )
		{
			cerr << "Could not load TNS Names file: " << x.what() << endl;
			throw x;
		}
	}

    //
    entry_ = resolve( ent, resolve( ent ) );
    return entry_;
}

//
std::string tns::env( std::string var )
{
    // defer to main.h
    return ::env( var );
}

//
std::string tns::locate( std::string name )
{
    std::string file;

    // search order ...
    //
    //    1) $TNS_ADMIN/tnsnames.ora
    //    2) $ORACLE_HOME/network/admin/tnsnames.ora
    //    3) /etc/oracle/tnsnames.ora
    //    4) $HOME/.tnsnames.ora

    // $TNS_ADMIN/<file>
    //
    if ( env( "TNS_ADMIN" ).length() > 0 )
    {
        //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
        if ( ::exists( env( "TNS_ADMIN" ) + "/" + name ) )
            file = env( "TNS_ADMIN" ) + "/" + name;
#else
        if ( ::exists( env( "TNS_ADMIN" ) + "\\" + name ) )
            file = env( "TNS_ADMIN" ) + "\\" + name;
#endif
	}

    // $ORACLE_HOME/network/admin/<file>
    //
    if ( file.length() == 0 )
    {
        if ( env( "ORACLE_HOME" ).length() > 0 )
        {
            //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
            if ( ::exists( env( "ORACLE_HOME" ) + "/" + name ) )
                file = env( "ORACLE_HOME" ) + "/" + name;
#else
            if ( ::exists( env( "ORACLE_HOME" ) + "\\" + name ) )
                file = env( "ORACLE_HOME" ) + "\\" + name;
#endif
		}
    }

    // /etc/oracle/<file>
    //
    if ( file.length() == 0 )
    {
        //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
        if ( ::exists( std::string( ETC_DIR ) + "/" + name ) )
            file = std::string( ETC_DIR ) + "/" + name;
#else
        if ( ::exists( std::string( ETC_DIR ) + "\\" + name ) )
            file = std::string( ETC_DIR ) + "\\" + name;
#endif
	}

    // $HOME/.<file>
    //
    if ( file.length() == 0 )
    {
        if ( env( "HOME" ).length() > 0 )
        {
            //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
            if ( ::exists( env( "HOME" ) + "/." + name ) )
                file = env( "HOME" ) + "/." + name;
#else
            if ( ::exists( env( "HOME" ) + "\\." + name ) )
                file = env( "HOME" ) + "\\." + name;
#endif
		}
    }

    // not found ...
    //
    if ( file.length() == 0 )
        throw exp ( "File [" + name + "] not found!", EXP_MISSING );

    //
    return file;
}

//
size_t tns::load_methods()
{
    //
    if ( sqlnet_.length() > 0 )
    {
        //
        ifstream ifs;

        try
        {
            std::string buf; // line buffer

            //
            ifs.open( sqlnet_.c_str(), ios::in );

            //
            while ( getline( ifs, buf ) )
            {
                size_t pos;

                //
                buf = trim( buf );

                //
                if ( buf.substr( 0, 1 ) == "#" )
                    continue;

                //
                if ( ( pos = ucase( buf ).find( "NAMES.DIRECTORY_PATH", 0 ) ) != std::string::npos )
                {
                    size_t lpos = buf.find_first_of( "(" );

                    if ( lpos != std::string::npos )
                    {
                        size_t rpos = buf.find_first_of( ")", lpos + 1 );

                        if ( rpos != std::string::npos )
                        {
                            int i = 0;
                            std::string path = buf.substr( lpos + 1, rpos - ( lpos + 1 ) );
                            std::string itm = split( path, i );

                            while ( ( itm = split( path, i++ ) ) != "" )
                                methods_.push_back( ucase( itm ) );
                        }
                    }
                }
            }
        }
        catch ( exception& x )
        {
            throw exp ( "File [" + sqlnet_ + "] read error: "  + std::string( x.what() ), EXP_OPEN );
        }
    }
    else
        throw exp ( "SQL*Net file is missing or undefined!", EXP_MISSING );

    //
    if ( methods_.size() == 0 )
    {
        methods_.push_back( "TNSNAMES" );
        methods_.push_back( "EZCONNECT" );
    }

    return methods_.size();
}

//
size_t tns::load_tnsnames()
{
    //
    if ( entries_.size() == 0 )
    {
        //
        if ( tnsnames_.length() > 0 )
        {
            //
            ifstream ifs;

            try
            {
                std::string buf; // line buffer

                std::string ent; // entry
                std::string dsc; // description

                bool flg;   // new item flag
                int  lvl;   // parenthisis level

                //
                ifs.open( tnsnames_.c_str(), ios::in );

                //
                flg = true;
                lvl = 0;

                //
                while ( std::getline( ifs, buf ) )
                {
                    //
                    buf = trim( buf );

                    //
                    if ( trim( buf ) == std::string( "" ) )
                    {
                        // reset on empty line
                        ent = dsc = ""; flg = true; lvl = 0;
                        continue;
                    }

                    //
                    if ( buf.substr( 0, 1 ) == "#" )
                    {
                        // reset on comment line, if we are not on an in-line comment
                        if ( flg )
                        {
                            ent = dsc = ""; flg = true; lvl = 0;
                        }

                        //
                        continue;
                    }

                    //
                    for ( size_t i = 0; i < buf.length(); ++i )
                    {
                        //
                        if ( buf[ i ] == ' ' )
                            continue;

                        //
                        if ( flg )
                        {
                            //
                            if ( buf[ i ] != '=' )
                            {
                                if ( ( buf[ i ] == ' ' ) || ( buf[ i ] == '\t' ) )
                                    continue;

                                ent += buf[ i ];
                            }
                            else
                                flg = false;    // start working on the description
                        }
                        else
                        {
                            //
                            if ( ( dsc.length() == 0 ) && ( buf[ i ] == '(' ) )
                            {
                                // started the description
                                dsc += buf[ i ]; ++lvl;
                            }
                            else
                            {
                                //
                                if ( buf[ i ] == '(' ) ++lvl;
                                if ( buf[ i ] == ')' ) --lvl;

                                //
                                dsc += buf[ i ];

                                //
                                if ( lvl == 0 )
                                {
                                    //
                                    flg = true; // next new entry
                                    add( format( ent ), format( dsc ), entry::TNSNAMES );
                                }
                            }
                        }
                    }
                }
            }
            catch ( exception& x )
            {
                throw exp ( "File [" + tnsnames_ + "] read error: "  + std::string( x.what() ), EXP_OPEN );
            }
        }
        else
            throw exp ( "TNS*Names missing or undefined!", EXP_MISSING );
    }

    //
    return entries_.size();
}

//
tns::store tns::resolve_directory()
{
    //
    if ( ldap_.length() > 0 )
    {
        //
        ifstream ifs;

        try
        {
            std::string buf; // line buffer

            //
            ifs.open( ldap_.c_str(), ios::in );

            //
            while ( getline( ifs, buf ) )
            {
                size_t pos;

                //
                buf = trim( buf );

                //
                if ( buf.substr( 0, 1 ) == "#" )
                    continue;

                //
                if ( ( pos = ucase( buf ).find( "DIRECTORY_SERVERS", 0 ) ) != std::string::npos )
                {
                    size_t lpos = buf.find_first_of( "(" );

                    if ( lpos != std::string::npos )
                    {
                        size_t rpos = buf.find_first_of( ")", lpos + 1 );

                        if ( rpos != std::string::npos )
                        {
                            std::string dat = buf.substr( lpos + 1, rpos - ( lpos + 1 ) );

                            store_.host = split( dat, 0, ":" );
                            store_.port = ::atoi( split( dat, 1, ":" ).c_str() );
                            store_.pssl = ::atoi( split( dat, 2, ":" ).c_str() );
                        }
                    }
                }

                //
                if ( ( pos = ucase( buf ).find( "DEFAULT_ADMIN_CONTEXT", 0 ) ) != std::string::npos )
                {
                    size_t lpos = buf.find_first_of( "\"" );

                    if ( lpos != std::string::npos )
                    {
                        size_t rpos = buf.find_first_of( "\"", lpos + 1 );

                        if ( rpos != std::string::npos )
                            store_.root = trim( buf.substr( lpos + 1, rpos - ( lpos + 1 ) ) );
                    }
                }

                //
                if ( ( pos = ucase( buf ).find( "DIRECTORY_SERVER_TYPE", 0 ) ) != std::string::npos )
                {
                    size_t lpos = buf.find_first_of( "=" );

                    if ( lpos != std::string::npos )
                    {
                        std::string typ = ucase( trim( buf.substr( lpos + 1 ) ) );

                        if ( typ != "OID" )
                            throw exp ( "Unsupported directory type [" + typ + "] encountered", EXP_UNSUPPORTED );
                    }
                }
            }
        }
        catch ( exception& x )
        {
            throw exp ( "File [" + sqlnet_ + "] read error: "  + std::string( x.what() ), EXP_OPEN );
        }
    }
    else
        throw exp ( "LDAP*Names file is missing or undefined!", EXP_MISSING );

    return store_;
}

//
size_t tns::load_ldap()
{
    if ( mesg() )
    {
        //
        LDAP* ld = NULL;
        char* dn = NULL;
        char* pw = NULL;

        //
        std::string url;
        int rc = LDAP_SUCCESS;

        try
        {
            if ( ! store_.ok() )
                store_ = resolve_directory();

            if ( store_.ok() )
            {
                //
                url = store_.url();

                //
                if ( ( ld = ::ldap_init( (char*)store_.host.c_str(), store_.port ) ) != NULL )
                {
                    // use the first element for sorting
                    const char* attrs[]  = { ATTR_VAL_CN, ATTR_VAL_DESC, ATTR_VAL_ALIAS, NULL };
                    const char  filter[] = LDAP_SEARCH;
                    struct timeval tv    = { 1, 0 };
                    LDAPMessage* res     = NULL;

                    //
                    if ( store_.dn.length() > 0 )
                    {
                        dn = (char*)::malloc( sizeof( char ) * ( store_.dn.length() + 1 ) );
                        ::memset( dn, 0, sizeof(char) * ( store_.dn.length() + 1 ) );
                        ::memcpy( dn, store_.dn.c_str(), ( store_.dn.length() + 1 ) );
                    }

                    //
                    if ( store_.pw.length() > 0 )
                    {
                        pw = (char*)::malloc( sizeof( char ) * ( store_.pw.length() + 1 ) );
                        ::memset( pw, 0, sizeof(char) * ( store_.pw.length() + 1 ) );
                        ::memcpy( pw, store_.pw.c_str(), ( store_.pw.length() + 1 ) );
                    }

                    //
                    if ( ( rc = ::ldap_simple_bind_s( ld, dn, pw ) ) != LDAP_SUCCESS )
                        throw exp ( "LDAP bind failed: " + std::string( ::ldap_err2string( rc ) ), rc );

                    //
                    if ( ( rc = ::ldap_search_ext_s( ld,
                                                     (char*)((store_.root.length() == 0 ) ? "" : store_.root.c_str()),
                                                     LDAP_SCOPE_SUBTREE,
                                                     (char*)filter,
                                                     (char**)attrs,
                                                     0,
                                                     NULL,
                                                     NULL,
                                                     &tv,
                                                     LDAP_NO_LIMIT,
                                                     &res ) ) == LDAP_SUCCESS )
                    {
                        LDAPMessage* ent = NULL;
                        BerElement*  ber = NULL;

                        //
                        ::ldap_sort_entries( ld, &res, (char*)attrs[ ATTR_IDX_CN ], (int(*)())::strcmp );

                        //
                        for ( ent = ::ldap_first_entry( ld, res );
                              ent != NULL;
                              ent = ::ldap_next_entry( ld, ent ) )
                        {
                            char* dn   = NULL;
                            char* attr = NULL;
                            entry itm;

                            //
                            itm.type = entry::LDAP;

                            //
                            if ( ( dn = ::ldap_get_dn( ld, ent ) ) != NULL )
                            {
                                //
                                for ( attr = ::ldap_first_attribute( ld, ent, &ber );
                                      attr != NULL;
                                      attr = ::ldap_next_attribute( ld, ent, ber ) )
                                {
                                    char** vals;

                                    if ( ( vals = ::ldap_get_values( ld, ent, attr ) ) != NULL )
                                    {
                                        //
                                        // (*sortval)( ld, vals, ::strcmp );

                                        //
                                        for ( int idx = 0; vals[ idx ] != NULL; idx++ )
                                        {
                                            //
                                            if ( format( std::string( attr ) ) == format( std::string( ATTR_VAL_CN ) ) )
                                            {
                                                if ( itm.name.length() == 0 )
                                                    itm.name = format( vals[ idx ] );
                                            }

                                            if ( format( std::string( attr ) ) == format( std::string( ATTR_VAL_ALIAS ) ) )
                                                itm.name = format( vals[ idx ] );

                                            if ( format( std::string( attr ) ) == format( std::string( ATTR_VAL_DESC ) ) )
                                                itm.desc = format( vals[ idx ] );
                                        }

                                        //
                                        ::ldap_value_free( vals );
                                    }

                                    //
                                    ::ldap_memfree( attr );
                                }

                                ::ldap_memfree( dn );
                            }
                            else
                                cerr << "NULL DN encountered" << endl;

                            //
                            ::ber_free( ber, 0 );

                            //
                            if ( ( itm.name.length() > 0  ) && ( itm.desc.length() > 0 ) )
                                add( itm );
                        }
                    }
                    else
                        throw exp ( "LDAP search failed: " + std::string( ::ldap_err2string( rc ) ), rc );
                }
                else
                {
                    throw exp ( "LDAP initialization failed", rc );
                }
            }
            else
                throw exp ( "Unresolved directory information", EXP_UNRESOLVED );
        }
        catch ( exp& x )
        {
            // report but don;t throw, for now
            cerr << url << " - " << x.what() << " [" << rc << "]" << endl << endl;
        }

        //
        if ( dn ) ::free( dn );
        if ( pw ) ::free( pw );

        //
        if ( ld )
        {
            //
            if ( ( rc = ldap_unbind_s( ld ) ) != LDAP_SUCCESS )
                throw exp ( "LDAP unbind failed: " + std::string( ::ldap_err2string( rc ) ), rc );
        }
    }
    else
        cerr << MISSING_MESG_FILE << endl << endl;

    //
    return entries_.size();
}

//
size_t tns::load_ldif( streambuf* dat )
{
    //
    if ( dat )
    {
        //
        std::string buf;
        istream ldif( dat );

        //
        std::string ent; // entry name
        std::string dsc; //       desc

        //
        while ( std::getline( ldif, buf ) )
        {
            //
            if ( buf.length() == 0 )
                continue;

            //
            if ( buf.substr( 0, 1 ) == "#" )
                continue;

            //
            if ( lcase( buf ).substr( 0, 3 ) == "dn:" )
            {
                ent = dsc = "";
                continue;
            }

            //
            if ( lcase( buf ).substr( 0, 12 ) == "objectclass:" )
                continue;

            //
            if ( lcase( buf ).substr( 0, 3 ) == "cn:" )
                ent = trim( buf.substr( 4, std::string::npos ) );

            //
            if ( lcase( buf ).substr( 0, 18 ) == "orclnetdescstring:" )
                dsc = trim( buf.substr( 19, std::string::npos ) );

            //
            if ( buf.substr( 0, 1 ) == " " )
            {
                dsc += trim( buf );
                continue;
            }

            //
            if ( ( ent.length() > 0 ) && ( dsc.length() > 0 ) )
                add( ent, dsc, entry::LDAP );
        }
    }
    else
        throw exp ( "Encountered NULL stream buffer", EXP_NULL );

    //
    return entries_.size();
}

//
bool tns::ldap_exists( LDAP* ld, char* dn )
{
    int rc = 0;

    //
    int cnt = 0;
    LDAPMessage* msg;
    char* qry = "(objectClass=*)";

    //
    if ( ( rc = ::ldap_search_s( ld, 
                          dn,
                          LDAP_SCOPE_BASE, 
                          qry, 
                          NULL, 
                          0, 
                          &msg ) ) != LDAP_SUCCESS )
    {
        cnt = 0;
    }
    else
        cnt = ::ldap_count_entries( ld, msg );

    //
    return ( cnt > 0 );
}

//
bool tns::ldap_delete( LDAP* ld, char* dn )
{
    int rc = LDAP_SUCCESS;

    try
    {
        //
        if (  ( rc = ::ldap_delete_s( ld, dn ) ) != LDAP_SUCCESS )
            throw exp ( "LDAP delete failed: " + std::string( ::ldap_err2string( rc ) ), rc );
    }
    catch ( exp& x )
    {
        cerr << store_.url() << " - " << x.what() << " [" << rc << "]" << endl << endl;
    }

    //
    return ( rc == LDAP_SUCCESS );
}

//
bool tns::save_ldap( entry& ent, bool repl /*= true*/ )
{
    int rc = LDAP_SUCCESS;

    if ( mesg() )
    {
        //
        LDAP* ld = NULL;
        char* dn = NULL;
        char* pw = NULL;

        //
        std::string url;

        try
        {
            if ( ! store_.ok() )
                store_ = resolve_directory();

            if ( store_.ok() )
            {
                //
                url = store_.url();

                //
                if ( ( ld = ::ldap_init( (char*)store_.host.c_str(), store_.port ) ) != NULL )
                {
                    std::string rdn = "cn=" + ent.name + "," + store_.root;

                    //
                    if ( store_.dn.length() > 0 )
                    {
                        dn = (char*)::malloc( sizeof( char ) * ( store_.dn.length() + 1 ) );
                        ::memset( dn, 0, sizeof(char) * ( store_.dn.length() + 1 ) );
                        ::memcpy( dn, store_.dn.c_str(), ( store_.dn.length() + 1 ) );
                    }

                    //
                    if ( store_.pw.length() > 0 )
                    {
                        pw = (char*)::malloc( sizeof( char ) * ( store_.pw.length() + 1 ) );
                        ::memset( pw, 0, sizeof(char) * ( store_.pw.length() + 1 ) );
                        ::memcpy( pw, store_.pw.c_str(), ( store_.pw.length() + 1 ) );
                    }

                    //
                    if ( ( rc = ::ldap_simple_bind_s( ld, dn, pw ) ) != LDAP_SUCCESS )
                        throw exp ( "LDAP bind failed: " + std::string( ::ldap_err2string( rc ) ), rc );

                    //
                    if ( ldap_exists( ld, (char*)store_.root.c_str() ) )
                    {
                        //
                        if ( ldap_exists( ld, (char*)rdn.c_str() ) )
                        {
                            //
                            if ( repl )
                            {
                                char* nsv[] = { NULL, NULL };

                                //
                                nsv[ 0 ] = (char*)::malloc( sizeof( char ) * ( ent.desc.length() + 1 ) );
                                //
                                ::memset( nsv[ 0 ], 0, sizeof(char) * ( ent.desc.length() + 1 ) );
                                ::memcpy( nsv[ 0 ], ent.desc.c_str(), ( ent.desc.length() + 1 ) );

                                LDAPMod ns;

                                //
                                LDAPMod* mod[ 2 ] = { NULL, NULL };

                                //
                                ns.mod_op     = LDAP_MOD_REPLACE;
                                ns.mod_type   = "orclNetDescString";
                                ns.mod_values = nsv;

                                //
                                mod[ 0 ] = &ns;
                                mod[ 1 ] = NULL;

                                //
                                try
                                {
                                    //
                                    if (  ( rc = ::ldap_modify_s( ld, (char*)rdn.c_str(), mod ) ) != LDAP_SUCCESS )
                                        throw exp ( "LDAP modify failed: " + std::string( ::ldap_err2string( rc ) ), rc );

                                    //
                                    if ( nsv[ 0 ] ) ::free( nsv[ 0 ] );
                                }
                                catch ( exp& )
                                {
                                    //
                                    if ( nsv[ 0 ] ) ::free( nsv[ 0 ] );

                                    //
                                    throw;
                                }
                            }
                            else
                                throw exp ( "LDAP modify did replace existing entry [" + rdn + "]!", EXP_EXISTS );
                        }
                        else
                        {
                            //
                            char* obv[] = { "top", "orclService", "orclNetService", NULL };
                            char* cnv[] = { NULL, NULL };
                            char* nsv[] = { NULL, NULL };

                            //
                            cnv[ 0 ] = (char*)::malloc( sizeof( char ) * ( ent.name.length() + 1 ) );
                            nsv[ 0 ] = (char*)::malloc( sizeof( char ) * ( ent.desc.length() + 1 ) );

                            //
                            ::memset( cnv[ 0 ], 0, sizeof(char) * ( ent.name.length() + 1 ) );
                            ::memset( nsv[ 0 ], 0, sizeof(char) * ( ent.desc.length() + 1 ) );

                            //
                            ::memcpy( cnv[ 0 ], ent.name.c_str(), ( ent.name.length() + 1 ) );
                            ::memcpy( nsv[ 0 ], ent.desc.c_str(), ( ent.desc.length() + 1 ) );

                            //
                            LDAPMod ob;
                            LDAPMod cn;
                            LDAPMod ns;

                            //
                            LDAPMod* mod[ 4 ] = { NULL, NULL, NULL, NULL };

                            //
                            ob.mod_op     = LDAP_MOD_ADD;
                            ob.mod_type   = "objectclass";
                            ob.mod_values = obv;

                            //
                            cn.mod_op     = LDAP_MOD_ADD;
                            cn.mod_type   = "cn";
                            cn.mod_values = cnv;

                            //
                            ns.mod_op     = LDAP_MOD_ADD;
                            ns.mod_type   = "orclNetDescString";
                            ns.mod_values = nsv;

                            //
                            mod[ 0 ] = &ob;
                            mod[ 1 ] = &cn;
                            mod[ 2 ] = &ns;
                            mod[ 3 ] = NULL;

                            //
                            try
                            {
                                //
                                if (  ( rc = ::ldap_add_s( ld, (char*)rdn.c_str(), mod ) ) != LDAP_SUCCESS )
                                    throw exp ( "LDAP add failed: " + std::string( ::ldap_err2string( rc ) ), rc );

                                //
                                if ( cnv[ 0 ] ) ::free( cnv[ 0 ] );
                                if ( nsv[ 0 ] ) ::free( nsv[ 0 ] );
                            }
                            catch ( exp& )
                            {
                                //
                                if ( cnv[ 0 ] ) ::free( cnv[ 0 ] );
                                if ( nsv[ 0 ] ) ::free( nsv[ 0 ] );

                                //
                                throw;
                            }
                        }
                    }
                    else
                        throw exp ( "LDAP base entry does not exist [" + store_.root + "]", EXP_EXISTS );
                }
                else
                {
                    throw exp ( "LDAP initialization failed", rc );
                }
            }
            else
                throw exp ( "Unresolved directory information", EXP_UNRESOLVED );
        }
        catch ( exp& x )
        {
            // 
            cerr << store_.url() << " - " << x.what() << " [" << rc << "]" << endl << endl;
        }

        //
        if ( dn ) ::free( dn );
        if ( pw ) ::free( pw );

        //
        if ( ld )
        {
            //
            if ( ( rc = ldap_unbind_s( ld ) ) != LDAP_SUCCESS )
                throw exp ( "LDAP unbind failed: " + std::string( ::ldap_err2string( rc ) ), rc );
        }
    }
    else
        cerr << MISSING_MESG_FILE << endl << endl;

    //
    return ( rc == LDAP_SUCCESS );
}

//
std::string tns::format( std::string str )
{
    str.erase( remove( str.begin(), str.end(), ' ' ),  str.end() );
    str.erase( remove( str.begin(), str.end(), '\r' ), str.end() );
    str.erase( remove( str.begin(), str.end(), '\n' ), str.end() );
    str.erase( remove( str.begin(), str.end(), '\t' ), str.end() );

    return trim( str );
}

//
std::string tns::resolve( std::string ent )
{
    std::string dsc;

    //
    ent = format( ent );

    //
    for ( item itm = entries_.begin(); itm != entries_.end(); ++itm )
    {
        //
        if ( ent == itm->name )
        {
            dsc = itm->desc;
            break;
        }
    }

    return dsc;
}

//
std::string tns::locate_tnsnames()
{
    std::string file;

    //
    try
    {
        //
        file = locate( TNSNAMES_FILE );
    }
    catch ( exp& x )
    {
        // ... do nothing
        if ( x.code() == EXP_MISSING )
            file = "";
        else
            throw exp ( "File [" + std::string( TNSNAMES_FILE ) + "] not found!", EXP_OPEN );
    }

    return file;
}

//
std::string tns::locate_ldapfile()
{
    std::string file;

    //
    try
    {
        //
        file = locate( LDAP_FILE );
    }
    catch ( exp& x )
    {
        // ... do nothing
        if ( x.code() == EXP_MISSING )
            file = "";
        else
            throw exp ( "File [" + std::string( LDAP_FILE ) + "] not found!", EXP_OPEN );
    }

    return file;
}

//
void tns::resolve_home()
{
    try
    {
        tnsadmin_ = tnsadmin();
        sqlnet_   = sqlnet();
        ldap_     = ldap();
        store_    = resolve_directory();

        load_methods(); // methods_
    }
    catch ( ... ) {}
}

//
std::string tns::compare( std::string dsc )
{
    std::string ent;

    //
    dsc = format( dsc );

    //
    for ( item itm = entries_.begin(); itm != entries_.end(); ++itm )
    {
        //
        if ( dsc == itm->desc )
        {
            ent = itm->name;
            break;
        }
    }

    return ent;
}

//
std::string tns::tnsnames()
{
    if ( tnsnames_.length() == 0 )
    {
        //
        try
        {
            //
            tnsnames_ = locate( TNSNAMES_FILE );
        }
        catch ( exp& x )
        {
            // ... do nothing
            if ( x.code() == EXP_MISSING )
                tnsnames_ = "";
            else
                throw exp ( "File [" + std::string( TNSNAMES_FILE ) + "] not found!", EXP_OPEN );
        }
    }

    return tnsnames_; 
}

//
std::string tns::tnsadmin()
{
    if ( tnsadmin_.length() == 0 )
    {
        std::string file;
        std::string name = "tnsnames.ora";

        if ( env( "TNS_ADMIN" ).length() > 0 )
        {
            //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
            if ( ::exists( env( "TNS_ADMIN" ) + "/" + name ) )
                file = env( "TNS_ADMIN" ) + "/" + name;
#else
            if ( ::exists( env( "TNS_ADMIN" ) + "\\" + name ) )
                file = env( "TNS_ADMIN" ) + "\\" + name;
#endif
	    }

        // $ORACLE_HOME/network/admin/<file>
        //
        if  ( file.length() == 0 )
        {
            if ( env( "ORACLE_HOME" ).length() > 0 )
            {
                //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
                if ( ::exists( env( "ORACLE_HOME" ) + "/" + name ) )
                    file = env( "ORACLE_HOME" ) + "/" + name;
#else
                if ( ::exists( env( "ORACLE_HOME" ) + "\\" + name ) )
                    file = env( "ORACLE_HOME" ) + "\\" + name;
#endif
		    }
        }

        // /etc/oracle/<file>
        //
        if ( file.length() == 0 )
        {
            //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
            if ( ::exists( std::string( ETC_DIR ) + "/" + name ) )
                file = std::string( ETC_DIR ) + "/" + name;
#else
            if ( ::exists( std::string( ETC_DIR ) + "\\" + name ) )
                file = std::string( ETC_DIR ) + "\\" + name;
#endif
	    }

        // $HOME/.<file>
        //
        if ( file.length() == 0 )
        {
            if ( env( "HOME" ).length() > 0 )
            {
            //
#if !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
                if ( ::exists( env( "HOME" ) + "/." + name ) )
                    file = env( "HOME" ) + "/." + name;
#else
                if ( ::exists( env( "HOME" ) + "\\." + name ) )
                    file = env( "HOME" ) + "\\." + name;
#endif
		    }
        }

        if ( file.length() > 0 )
            tnsadmin_ = file;
    }

    return tnsadmin_;
}

//
std::string tns::tnsadmin( std::string dir )
{
    tnsadmin_ = dir;
    return tnsadmin_;
}

//
std::string tns::sqlnet()
{
    if ( sqlnet_.length() == 0 )
    {
        //
        try
        {
            //
            sqlnet_ = locate( SQLNET_FILE );
        }
        catch ( exp& x )
        {
            // ... do nothing
            if ( x.code() == EXP_MISSING )
                sqlnet_ = "";
            else
                throw exp ( "File [" + std::string( SQLNET_FILE ) + "] not found!", EXP_OPEN );
        }
    }

    return sqlnet_; 
}

//
std::string tns::ldap()
{
    if ( ldap_.length() == 0 )
    {
        //
        try
        {
            //
            ldap_ = locate( LDAP_FILE );
        }
        catch ( exp& x )
        {
            // ... do nothing
            if ( x.code() == EXP_MISSING )
                ldap_ = "";
            else
                throw exp ( "File [" + std::string( LDAP_FILE ) + "] not found!", EXP_OPEN );
        }
    }

    return ldap_; 
}

//
std::string tns::tnsnames( std::string filename )
{
    if ( filename.length() == 0 )
    {
        tnsnames_ = tnsnames();
    }
    else
    {
        if ( ::exists( filename ) )
            tnsnames_ = filename;
        else
            throw exp ( "File [" + filename + "] not found!", EXP_MISSING );
    }

    return tnsnames_;
}

//
std::string tns::sqlnet( std::string filename )
{
    if ( filename.length() == 0 )
    {
        sqlnet_ = sqlnet();
    }
    else
    {
        if ( ::exists( filename ) )
            sqlnet_ = filename;
        else
            throw exp ( "File [" + filename + "] not found!", EXP_MISSING );
    }

    return sqlnet_;
}

//
std::string tns::ldap( std::string filename )
{
    if ( filename.length() == 0 )
    {
        ldap_ = ldap();
    }
    else
    {
        if ( ::exists( filename ) )
            ldap_ = filename;
        else
            throw exp ( "File [" + filename + "] not found!", EXP_MISSING );
    }

    return ldap_;
}

//
void tns::sort_entries( entries& ent )
{
    // sorts a elements while preserving order between equals
    std::stable_sort( ent.begin(), ent.end() );
}

std::string tns::pretty( entry& ent )
{
    std::string dat;
    std::string dsc = ent.desc;
    int    pad = 0;

    dat  = ent.name + " =";

    for ( int i = 0; i < dsc.length(); ++i )
    {
        if ( dsc[ i ] == ')' ) --pad;

        if ( dsc[ i ] == '(' )
        {
            dat += NL;

            ++pad;
            for ( int p = 0; p < pad; ++p )
                dat += PADDING;
        }

        dat += dsc[ i ];
    }

    return dat;
}

//
tns::store tns::ldapstore()
{
    //
    return store_;
}

//
tns::store tns::ldapstore( tns::store sto )
{
    //
    tns::store old = store_; 

    //
    store_ = sto; 
    return old;
}
}
