#include "unity.h"
#include "mapper.h"
#include <slinky.h>
#include <gromer.h>

#include <string.h>


char* str1 = "foobar";
char* str2 = "diiduu";
char* str3 = "hiihaa";
char* str4 = "jiihaa";
char* str5 = "jippii";

typedef void ( *put_fn_p )( mp_t mp, const gr_d key, const gr_d value );
typedef gr_d ( *get_fn_p )( mp_t mp, const gr_d value );
typedef gr_d ( *del_fn_p )( mp_t mp, const gr_d key );



void put_fn_no_key( mp_t mp, const gr_d key, const gr_d value )
{
    /* Use key for shits and giggles. */
    if ( key )
        mp_put( mp, value );
    else
        mp_put( mp, value );
}


void test_basic( void )
{
    mp_t     mp;
    char*    get;
    put_fn_p put_fn;
    get_fn_p get_fn;
    del_fn_p del_fn;

    put_fn = put_fn_no_key;
    get_fn = mp_get;
    del_fn = mp_del;

    for ( gr_size_t i = 0; i < 2; i++ ) {

        mp = mp_new();

        put_fn( mp, str1, str1 );
        put_fn( mp, str2, str2 );
        put_fn( mp, str3, str3 );

        get = (char*)get_fn( mp, str2 );
        TEST_ASSERT_TRUE( !strcmp( get, str2 ) );

        get = (char*)get_fn( mp, str4 );
        TEST_ASSERT_TRUE( !get );

        TEST_ASSERT_TRUE( !strcmp( del_fn( mp, str2 ), str2 ) );
        TEST_ASSERT_TRUE( !strcmp( del_fn( mp, str1 ), str1 ) );
        TEST_ASSERT_TRUE( !strcmp( del_fn( mp, str3 ), str3 ) );

        put_fn( mp, str3, str3 );
        put_fn( mp, str3, str3 );
        TEST_ASSERT_TRUE( !strcmp( del_fn( mp, str3 ), str3 ) );

        /* Delete non-existing. */
        TEST_ASSERT_TRUE( del_fn( mp, str3 ) == NULL );

        mp_destroy( mp );

        put_fn = mp_put_key;
        get_fn = mp_get_key;
        del_fn = mp_del_key;
    }
}


void test_collision( void )
{
    mp_t     mp;
    char*    get;
    put_fn_p put_fn;
    get_fn_p get_fn;
    del_fn_p del_fn;

    put_fn = put_fn_no_key;
    get_fn = mp_get;
    del_fn = mp_del;

    for ( gr_size_t i = 0; i < 2; i++ ) {

        mp = mp_new_full( mp_key_hash_cstr, mp_key_comp_cstr, 4, 100 );

        put_fn( mp, str3, str3 );
        put_fn( mp, str1, str1 );
        put_fn( mp, str2, str2 );

        /* Cover collision with different string. */
        put_fn( mp, str2, str2 );
        TEST_ASSERT_TRUE( !strcmp( del_fn( mp, str2 ), str2 ) );
        put_fn( mp, str2, str2 );

        get = (char*)get_fn( mp, str2 );
        TEST_ASSERT_TRUE( !strcmp( get, str2 ) );

        get = (char*)get_fn( mp, str4 );
        TEST_ASSERT_TRUE( !get );

        /* Cover not found. */
        put_fn( mp, str4, str4 );

        get = (char*)get_fn( mp, str5 );
        TEST_ASSERT_TRUE( !get );

        /* Cover not found (for delete). */
        get = (char*)del_fn( mp, str5 );
        TEST_ASSERT_TRUE( !get );

        /* Cover rehash. */
        put_fn( mp, str5, str5 );
        TEST_ASSERT_TRUE( gr_size( mp->table ) == ( 8 * ( i + 1 ) ) );

        mp_destroy( mp );

        put_fn = mp_put_key;
        get_fn = mp_get_key;
        del_fn = mp_del_key;
    }
}


void del_each_fn( gr_d key, void* arg )
{
    sl_t sl = key;
    if ( arg )
        sl_del( &sl );
    else
        sl_del( &sl );
}


void del_each_key_fn( gr_d key, gr_d value, void* arg )
{
    sl_t sl = key;
    if ( arg && value )
        sl_del( &sl );
    else
        sl_del( &sl );
}


void test_slinky( void )
{
    mp_s  mp;
    char* get;

    sl_t sl1;
    sl_t sl2;
    sl_t sl3;


    /* Object Mode. */
    sl1 = sl_from_str_c( str1 );
    sl2 = sl_from_str_c( str2 );
    sl3 = sl_from_str_c( str3 );

    char gr_buf[ 128 ];
    mp = mp_use( gr_use( gr_buf, 32 ), mp_key_hash_slinky, mp_key_comp_slinky, 100 );

    mp_put( &mp, sl1 );
    mp_put( &mp, sl2 );

    get = (char*)mp_get( &mp, sl1 );
    TEST_ASSERT_TRUE( !strcmp( get, sl1 ) );

    get = (char*)mp_get( &mp, sl3 );
    TEST_ASSERT_TRUE( !get );

    mp_put( &mp, sl3 );

    mp_each( &mp, del_each_fn, NULL );


    /* Key Mode. */
    sl1 = sl_from_str_c( str1 );
    sl2 = sl_from_str_c( str2 );
    sl3 = sl_from_str_c( str3 );

    mp = mp_use( gr_use( gr_buf, 128 ), mp_key_hash_slinky, mp_key_comp_slinky, 100 );

    mp_put_key( &mp, sl1, sl1 );
    mp_put_key( &mp, sl2, sl2 );
    mp_put_key( &mp, sl3, sl3 );

    mp_each_key( &mp, del_each_key_fn, NULL );
}


struct pair_s
{
    int   number;
    char* name;
};



void test_key( void )
{
    struct pair_s  p1 = { 12, "pair_1" };
    struct pair_s  p2 = { 13, "pair_2" };
    struct pair_s* pp;

    mp_t mp;

    mp = mp_new();

    mp_put_key( mp, p1.name, &p1 );
    mp_put_key( mp, p2.name, &p2 );

    pp = mp_get_key( mp, p2.name );
    TEST_ASSERT_TRUE( !strcmp( pp->name, p2.name ) );
    TEST_ASSERT_TRUE( p2.number == pp->number );

    pp = mp_get_key( mp, p1.name );
    TEST_ASSERT_TRUE( !strcmp( pp->name, p1.name ) );
    TEST_ASSERT_TRUE( p1.number == pp->number );

    mp_destroy( mp );
}
