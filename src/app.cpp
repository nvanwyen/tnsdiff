//
// app.cpp
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
#include <limits>
#include <iostream>
#include <algorithm>

//
#include "app.h"

//
#include "exp.h"
#include "opt.h"
#include "uri.h"
#include "ver.h"

#ifndef APP
#define APP "tnsdiff"
#endif

#ifndef PREFIX_LEFT
#define PREFIX_LEFT     "< "
#endif

#ifndef PREFIX_RIGHT
#define PREFIX_RIGHT    "> "
#endif

#ifndef PREFIX_ADD
#define PREFIX_ADD      "+ "
#endif

#ifndef PREFIX_SUB
#define PREFIX_SUB      "- "
#endif

#ifndef PREFIX_OLD
#define PREFIX_OLD      "# "
#endif

#ifndef PREFIX_OTHER
#define PREFIX_OTHER    "? "
#endif

#ifndef PREFIX_NONE
#define PREFIX_NONE     ""
#endif

#ifndef NPOS
#define NPOS            std::numeric_limits<std::size_t>::max()
#endif

//
namespace mti {

//
int app::run( int argc, char** argv )
{
    //
    version( APP );
    copyright();

    int rc = ( options( argc, argv ) ? 0 : 1 );

    //
    if ( rc == 0 )
    {
        //
        if ( ltyp_ == app::home_type )
        {
            //
            ldev_ = load_home();
            ltyp_ = intype( ldev_ );
        }

        //
        if ( rtyp_ == app::home_type )
        {
            //
            rdev_ = load_home();
            rtyp_ = intype( rdev_ );
        }

        //
        if ( ldev_ != rdev_ )
        {
            //
            if ( fdif_ )
                rc = diffs();

            //
            if ( fmrg_ )
                rc = merge();
        }
        else
        {
            //
            rc = 1;
            std::cerr << "Left and Right inputs cannot be the same!" << std::endl;
        }
    }

    //
    return rc;
}

//
void app::usage( bool hlp /*= false*/ )
{
    //
    std::cout << "Usage: " << APP << "  options  file|ldap|-  file|ldap|-" << std::endl;
    std::cout << std::endl;
    std::cout << "    where:" << std::endl;
    std::cout << "       file            : \"Left\" or \"Right\" TNS Names compliant input file" << std::endl;
    std::cout << "       ldap            : \"Left\" or \"Right\" LDAP input: ldap[s]://host:port" << std::endl;
    std::cout << "       -               : Use the current $TNS_ADMIN/$ORACLE_HOME configuration" << std::endl;
    std::cout << std::endl;
    std::cout << "    options:" << std::endl;
    std::cout << "       -d | --diff     : Show differences (cannot be used with -m|--merge)" << std::endl;
    std::cout << "       -m | --merge    : Merge differences (cannot be used with -d|--diff)" << std::endl;
    std::cout << "       -? | --help     : Show usage, notes, and examples" << std::endl;
    std::cout << std::endl;

    //
    if ( hlp )
    {
        //
        std::cout << "notes:" << std::endl;
        std::cout << "   * The \"Left\" and \"Right\" sources cannot be the same. Error when detected." << std::endl;
        std::cout << "   * As defined by Oracle, LDAP access must be anonymous. No DN/passwords needed" << std::endl;
        std::cout << "   * This utility will *not* retain file or in-line comments when merging files." << std::endl;
        std::cout << "   * Required by the Oracle client, the file \"$ORACLE_HOME/ldap/mesg/ldapus.msb\"" << std::endl;
        std::cout << "     must exist. This file can be empty (e.g. use touch to create) as this" << std::endl;
        std::cout << "     utility does not need the contents to exist; but the Oracle libraries must" << std::endl;
        std::cout << "     find a file at that location, otherwise an error is thrown, when LDAP is" << std::endl;
        std::cout << "     requested" << std::endl;
        std::cout << std::endl;
        std::cout << "examples:" << std::endl;
        std::cout << "   $ tnsdiff --d     tnsnames1.ora tnsnames2.ora" << std::endl;
        std::cout << "   $ tnsdiff --diff  tnsnames1.ora tnsnames2.ora" << std::endl;
        std::cout << "   $ tnsdiff --diff  -             tnsnames2.ora" << std::endl;
        std::cout << "   $ tnsdiff --diff  tnsnames1.ora -" << std::endl;
        std::cout << std::endl;
        std::cout << "   $ tnsdiff --m     tnsnames1.ora tnsnames2.ora > tnsnames3.ora" << std::endl;
        std::cout << "   $ tnsdiff --merge tnsnames1.ora tnsnames2.ora > tnsnames3.ora" << std::endl;
        std::cout << "   $ tnsdiff --merge -             tnsnames2.ora > tnsnames3.ora" << std::endl;
        std::cout << "   $ tnsdiff --merge tnsnames1.ora -             > tnsnames3.ora" << std::endl;
        std::cout << std::endl;
        std::cout << "   $ tnsdiff --diff  ldap://myldap:389 ldaps://yourldap:636" << std::endl;
        std::cout << "   $ tnsdiff --merge ldap://myldap:389 tnsnames2.ora > tnsnames3.ora" << std::endl;
        std::cout << std::endl;
    }
}

//
bool app::options( int c, char** v )
{
    //
    mti::opt ops( c, v );
    ops.exceptions_all();

    //
    try
    {
        //
        if ( ops >> mti::OptionPresent( '?', "help" ) )
        {
            //
            usage( true );
            return false;
        }

        //
        if ( ops >> mti::OptionPresent( 'd', "diff" ) )
        {
            //
            fdif_ = true;
        }
        else
        {
            //
            fdif_ = false;
        }

        //
        if ( ops >> mti::OptionPresent( 'm', "merge" ) )
        {
            //
            fmrg_ = true;
        }
        else
        {
            //
            fmrg_ = false;
        }

        //
        std::vector<std::string> opt;
        ops >> mti::GlobalOption( opt );

        //
        if ( opt.size() == 0 )
        {
            //
            std::cerr << "Missing \"Left\" input" << std::endl;
            usage();

            return false;
        }
        else
        {
            //
            ldev_ = opt.at( 0 );
        }

        //
        if ( opt.size() == 1 )
        {
            //
            std::cerr << "Missing \"Right\" input" << std::endl;
            usage();

            return false;
        }
        else
        {
            //
            if ( opt.size() == 2 )
                rdev_ = opt.at( 1 );
            else
            {
                //
                std::cerr << "Too many inputs specified!" << std::endl;
                usage();

                return false;
            }
        }

        //
        ops.end_of_options();

        //
        if ( ( fdif_ == true ) && ( fmrg_ == true ) )
            throw mti::exp( "Cannot specify both diff [-d|--diff] and merge [-m|--merge] simultaneously!" );

        //
        if ( ( fdif_ == false ) && ( fmrg_ == false ) )
            throw mti::exp( "Either diff [-d|--diff] or merge [-m|--merge] must be specified!" );

        //
        try
        {
            //
            ltyp_ = intype( ldev_ );
        }
        catch ( exp ex )
        {
            //
            std::cerr << "Left: " << ex.message() << std::endl;
            return false;
        }

        try
        {
            //
            rtyp_ = intype( rdev_ );
        }
        catch ( exp ex )
        {
            //
            std::cerr << "Right: " << ex.message() << std::endl;
            return false;
        }
    }
    catch ( mti::OptionNotFoundEx ex )
    {
        //
        std::cerr << "Incomplete options specified! " << ex.what() << std::endl;
        usage();

        //
        return false;
    }
    catch ( mti::TooManyOptionsEx ex )
    {
        //
        std::cerr << "Too many options specified! " << ex.what() << std::endl;
        usage();

        //
        return false;
    }
    catch ( mti::optex ex )
    {
        //
        std::cerr << "Invalid or missing option! " << ex.what() << std::endl;
        usage();

        //
        return false;
    }
    catch ( mti::exp ex )
    {
        //
        std::cerr << ex.what() << std::endl;
        usage();

        //
        return false;
    }
    catch ( std::exception ex )
    {
        //
        std::cerr << ex.what() << std::endl;
        usage();

        //
        return false;
    }
    catch ( ... )
    {
        //
        std::cerr << "Something went wrong!" << std::endl;
        usage();

        //
        return false;
    }

    //
    return true;
}

//
app::input_type app::intype( std::string& in )
{
    //
    if ( in != "-" )
    {
        //
        if ( ::exists( in ) )
            return file_type;
        else
        {
            //
            uri uri( in );

            //
            if ( uri.protocol().substr( 0, 4 ) == "ldap" )
                return ldap_type;
        }
    }
    else
        return home_type;

    //
    throw exp( "Unknown input type!" );
}

//
std::string app::load_home()
{
    //
    std::string home;
    mti::tns tns;

    //
    tns.resolve_home();

    //
    if ( tns.tns_methods().size() > 0 )
    {
        //
        if ( ( ( *( tns.tns_methods().begin() ) ) == "TNSNAMES" )
          || ( ( *( tns.tns_methods().begin() ) ) == "EZCONNECT" ) )
            home = tns.tnsnames();

        //
        if ( ( *( tns.tns_methods().begin() ) ) == "LDAP" )
            home = tns.ldapstore().url();
    }

    //
    return home;
}

//
mti::tns::entries app::load_entries( std::string dev, input_type typ )
{
    //
    mti::tns::entries ent;
    mti::tns tns;

    //
    if ( typ == file_type )
    {
        //
        tns.tnsnames( dev );

        //
        if ( tns.load_tnsnames() > 0 )
            ent = tns.tns_entries();
    }
    else if ( typ == ldap_type )
    {
        //
        tns.ldapstore( mti::tns::store( dev ) );

        //
        if ( tns.load_ldap() > 0 )
            ent = tns.tns_entries();
    }
    else
        throw exp( "Cannot load emtries for unknown type!" );

    //
    if ( ent.size() > 0 )
        tns.sort_entries( ent );

    //
    return ent;
}

//
size_t app::load_differences( mti::tns::entries& lft, 
                              mti::tns::entries& rgt, 
                              app::results& chg )
{
#ifndef NDEBUG    
    std::cout << "*** Running: load_differences()" << std::endl;
    debug();
#endif
    //
    lft = load_entries( ldev_, ltyp_ );
    rgt = load_entries( rdev_, rtyp_ );

#ifndef NDEBUG
    debug( lft );
    debug( rgt );
#endif

    // items which have been changed
    for ( mti::tns::item lt = lft.begin(); lt != lft.end(); ++lt )
    {
        //
        mti::tns::item rt = find( rgt, (*lt).name );

        //
        if ( rt != rgt.end() )
        {
            //
            if ( (*rt).desc != (*lt).desc )
                chg.push_back( app::result( app::item( (*lt), lt - lft.begin() ),
                                            app::item( (*rt), rt - rgt.begin() ),
                                            app::result::changed ) );
        }
        else // items which have been added
            chg.push_back( app::result( app::item( (*lt), lt - lft.begin() ),
                                        app::item( mti::tns::entry(), NPOS ),
                                        app::result::added ) );
    }

    // items which have been deleted
    for ( mti::tns::item rt = rgt.begin(); rt != rgt.end(); ++rt )
    {
        //
        if ( find( lft, (*rt).name ) == lft.end() )
            chg.push_back( app::result( app::item( mti::tns::entry(), NPOS ),
                                        app::item( (*rt), rt - rgt.begin() ),
                                        app::result::missing ) );
    }

#ifndef NDEBUG
    std::cout << "Changes:" << chg.size() << std::endl;
    debug( chg );
#endif

    //
    return chg.size();
}

//
int app::diffs()
{
    //
    int rc = 0;

    //
    mti::tns::entries lft;
    mti::tns::entries rgt;

    //
    app::results chg;

    //
    if ( load_differences( lft, rgt, chg ) > 0 )
    {
        //
        std::cout << "# Left:  " << ldev_   << std::endl;
        std::cout << "# Right: " << rdev_   << std::endl;
        std::cout << "# Stamp: " << ::now() << std::endl;
        std::cout << std::endl;

        //
        for ( app::event i = chg.begin(); i != chg.end(); ++i )
        {
            //
            switch ( (*i).differ )
            {
                //
                case app::result::added:
                    //
                    std::cout << "# Only left [" << ldev_ << "]" << std::endl;
                    std::cout << diff( (*i).left.entry, PREFIX_ADD ) << std::endl;
                    break;

                //
                case app::result::removed:
                    //
                    std::cout << "# Only right [" << rdev_ << "]" << std::endl;
                    std::cout << diff( (*i).right.entry, PREFIX_SUB ) << std::endl;
                    break;

                //
                case app::result::missing:
                    //
                    std::cout << "# Only right [" << rdev_ << "]" << std::endl;
                    std::cout << diff( (*i).right.entry, PREFIX_SUB ) << std::endl;
                    break;

                //
                case app::result::changed:
                    //
                    std::cout << "# changes left [" << ldev_ << "] from right [" << rdev_ << "]" << std::endl;
                    std::cout << diff( (*i).left.entry,  PREFIX_LEFT ) << std::endl;
                    std::cout << diff( (*i).right.entry, PREFIX_RIGHT ) << std::endl;
                    break;

                //
                default:
                    //
                    std::cerr << "Unknown change entry type!" << std::endl;
                    break;

            };

            //
            std::cout << std::endl;
        }

        //
        rc = 0;
    }
    else
        rc = 1;

    //
    return rc;
}

//
int app::merge()
{
    //
    int rc = 0;

    //
    mti::tns::entries lft;
    mti::tns::entries rgt;

    //
    app::results chg;

    //
    if ( load_differences( lft, rgt, chg ) > 0 )
    {
        //
        std::cout << "# Merged: " << ldev_   << std::endl;
        std::cout << "#   into: " << rdev_   << std::endl;
        std::cout << "#  Stamp: " << ::now() << std::endl;
        std::cout << std::endl;

        //
        for ( mti::tns::item rt = rgt.begin(); rt != rgt.end(); ++rt )
        {
            bool lfcr = false;

            //
            event            dif = changes( chg, rt );
            result::compared evt = ( ( dif != chg.end() ) ? (*dif).differ : result::none );

            //
            switch ( evt )
            {
                // this never hits here ... see appending loop below
                case result::added:
                    std::cout << "# Added: " << ldev_ << std::endl;
                    std::cout << diff( (*dif).left.entry, PREFIX_NONE ) << std::endl;
                    lfcr = true;
                    break;

                //
                case result::changed:
                    std::cout << "# Updated: " << ldev_ << std::endl;
                    std::cout << diff( (*dif).left.entry, PREFIX_NONE ) << std::endl;
                    std::cout << "# Original: " << rdev_ << std::endl;
                    std::cout << diff( (*rt), PREFIX_OLD ) << std::endl;
                    lfcr = true;
                    break;

                //
                case result::removed:
                case result::missing:
                    std::cout << "#" << std::endl;
                    std::cout << diff( (*rt), PREFIX_NONE ) << std::endl;
                    lfcr = true;
                    break;

                case result::none:
                default:
                    std::cout << "#" << std::endl;
                    std::cout << diff( (*rt), PREFIX_NONE ) << std::endl;
                    lfcr = true;
                    break;

            };

            if ( lfcr )
                std::cout << std::endl;
        }

        //
        for ( event it = chg.begin(); it != chg.end(); ++it )
        {
            bool lfcr = false;

            if ( (*it).differ == result::added )
            {
                std::cout << "# Added: " << ldev_ << std::endl;
                std::cout << diff( (*it).left.entry, PREFIX_NONE ) << std::endl;
                lfcr = true;
            }

            if ( lfcr )
                std::cout << std::endl;
        }

        rc = 0;
    }
    else
        rc = 1;

    //
    return rc;
}

//
mti::tns::item app::find( mti::tns::entries& ent, std::string itm )
{
    //
    if ( itm.length() == 0 )
        return ent.end();

    //
    for ( mti::tns::item i = ent.begin(); i != ent.end(); ++i )
    {
        //
        if ( (*i).name == itm )
            return i;
    }

    //
    return ent.end();
}

//
app::event app::changes( app::results& chg, mti::tns::item& it )
{
    //
    event evt = chg.end();

    //
    for ( app::event ch = chg.begin(); ch != chg.end(); ++ch )
    {
        if ( ( (*it) == (*ch).left.entry )
          || ( (*it) == (*ch).right.entry ) )
        {
            evt = ch;
            break;
        }
    }

    //
    return evt;
}

//
std::string app::diff( mti::tns::entry& ent, std::string pre )
{
    //
    std::string val = mti::tns().pretty( ent );

    //
    val = pre + replace( val, "\n", "\n" + pre );
    return val;
}

//
std::string app::replace( std::string str, const std::string fm, const std::string to )
{
    //
    if ( fm.empty() )
        return "";

    //
    size_t start_pos = 0;

    //
    while ( ( start_pos = str.find( fm, start_pos ) ) != std::string::npos )
    {
        //
        str.replace(start_pos, fm.length(), to);
        start_pos += to.length(); // In case 'to' contains 'fm', like replacing 'x' with 'yx'
    }

    //
    return str;
}

//
void app::debug()
{
#ifndef NDEBUG  // if defined; mode=release
    std::cout << "flag:  diff   = " << ( ( fdif_ ) ? "true" : "false" ) << std::endl;
    std::cout << "       merge  = " << ( ( fmrg_ ) ? "true" : "false" ) << std::endl;
    std::cout << std::endl;

    std::cout << "left:  device = " << ldev_ << std::endl;
    std::cout << "       type   = " << ( ( ltyp_ == home_type ) ? "home_type" :
                                         ( ltyp_ == file_type ) ? "file_type" :
                                         ( ltyp_ == ldap_type ) ? "ldap_type" : "unknown" ) << std::endl;
    std::cout << std::endl;

    std::cout << "right: device = " << rdev_ << std::endl;
    std::cout << "       type   = " << ( ( rtyp_ == home_type ) ? "home_type" :
                                         ( rtyp_ == file_type ) ? "file_type" :
                                         ( rtyp_ == ldap_type ) ? "ldap_type" : "unknown" ) << std::endl;
    std::cout << std::endl;
#endif // NDEBUG
}

//
void app::debug( mti::tns::entries& ent  )
{
#ifndef NDEBUG
    for ( mti::tns::item i = ent.begin(); i != ent.end(); ++i )
        std::cout << diff( (*i), PREFIX_NONE ) << std::endl;
#if 0    
        std::cout << "Emtry name: " << (*i).name << std::endl
                  << "      desc: " << (*i).desc << std::endl << std::endl;
#endif
    std::cout << std::endl;
#endif
}

//
void app::debug( app::results& res )
{
#ifndef NDEBUG
    for ( app::event i = res.begin(); i != res.end(); ++i )
        std::cout << "Result left    ["   << ( ( (*i).left.index != NPOS ) ? std::to_string( (*i).left.index ) : "x" )
                  <<                "]: " << (*i).left.entry.name  << std::endl
                  << "       right   ["   << ( ( (*i).right.index != NPOS ) ? std::to_string( (*i).right.index ) : "x" )
                  <<                "]: " << (*i).right.entry.name << std::endl
                  << "       change: "    << ( ( (*i).differ == app::result::added )   ? "added" :
                                               ( (*i).differ == app::result::removed ) ? "removed" :
                                               ( (*i).differ == app::result::missing ) ? "missing" :
                                               ( (*i).differ == app::result::changed ) ? "changed" : "unknown" )
                  << std::endl << std::endl;
#endif
}
}
