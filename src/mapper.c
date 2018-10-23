/**
 * @file   mapper.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sun Apr 29 09:47:41 2018
 *
 * @brief  Mapper - Hash table (map).
 *
 */

#include <string.h>
#include <stdlib.h>

#include "mapper.h"
#include "slinky.h"
#include "ag_hash.h"


static gr_size_t mp_next_pos( gr_size_t pos, gr_size_t size );
static gr_size_t mp_next_pos_key( gr_size_t pos, gr_size_t size );
static void mp_rehash( mp_t mp, gr_size_t new_size );
static void mp_rehash_key( mp_t mp, gr_size_t new_size );



/* ------------------------------------------------------------
 * Create and destroy:
 */


mp_t mp_new( void )
{
    return mp_new_full( mp_key_hash_cstr, mp_key_comp_cstr, MP_DEFAULT_SIZE, MP_DEFAULT_FILL );
}


mp_t mp_new_full( mp_key_hash_fn_p key_hash,
                  mp_key_comp_fn_p key_comp,
                  gr_size_t        size,
                  gr_size_t        fill_lim )
{
    mp_t mp;

    mp = gr_malloc( sizeof( mp_s ) );
    mp->table = gr_new_sized( size );
    mp->key_hash = key_hash;
    mp->key_comp = key_comp;
    mp->used_cnt = 0;
    mp->fill_lim = fill_lim;

    return mp;
}


mp_s mp_use( gr_t gr, mp_key_hash_fn_p key_hash, mp_key_comp_fn_p key_comp, gr_size_t fill_lim )
{
    mp_s mp;

    mp.table = gr;
    mp.key_hash = key_hash;
    mp.key_comp = key_comp;
    mp.used_cnt = 0;
    mp.fill_lim = fill_lim;

    return mp;
}


mp_t mp_destroy( mp_t mp )
{
    gr_destroy( &mp->table );
    gr_free( mp );
    return NULL;
}


void mp_put( mp_t mp, const gr_d value )
{
    if ( ( ( mp->used_cnt * 100 ) / gr_size( mp->table ) ) >= mp->fill_lim ) {
        mp_rehash( mp, gr_size( mp->table ) * 2 );
    }

    gr_size_t pos;

    pos = mp->key_hash( value ) % gr_size( mp->table );

    if ( gr_item( mp->table, pos, gr_d ) == NULL ) {

        mp->used_cnt++;
        gr_assign( mp->table, pos, value );

    } else if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), value ) ) {

        gr_assign( mp->table, pos, value );

    } else {

        for ( ;; ) {

            pos = mp_next_pos( pos, gr_size( mp->table ) );

            if ( gr_item( mp->table, pos, gr_d ) == NULL ) {
                mp->used_cnt++;
                gr_assign( mp->table, pos, value );
                break;
            } else if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), value ) ) {
                gr_assign( mp->table, pos, value );
                break;
            }
        }
    }
}


gr_d mp_get( mp_t mp, const gr_d value )
{
    gr_size_t pos;
    gr_size_t start;

    pos = mp->key_hash( value ) % gr_size( mp->table );
    start = pos;

    do {

        if ( gr_item( mp->table, pos, gr_d ) == NULL )
            return NULL;

        if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), value ) )
            return gr_item( mp->table, pos, gr_d );

        pos = mp_next_pos( pos, gr_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}


void mp_put_key( mp_t mp, const gr_d key, const gr_d value )
{
    if ( ( ( mp->used_cnt * 100 ) / gr_size( mp->table ) ) >= mp->fill_lim ) {
        mp_rehash_key( mp, gr_size( mp->table ) * 2 );
    }

    gr_size_t pos;

//    pos = ( mp->key_hash( key ) % ( gr_size( mp->table ) / 2 ) ) << 1;
    pos = ( mp->key_hash( key ) % ( gr_size( mp->table ) >> 1 ) ) << 1;
//    pos = ( mp->key_hash( key ) % gr_size( mp->table ) ) & ( UINT64_MAX - 1);

    if ( gr_item( mp->table, pos, gr_d ) == NULL ) {

        mp->used_cnt += 2;
        gr_assign( mp->table, pos, key );
        gr_assign( mp->table, pos + 1, value );

    } else if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), key ) ) {

        gr_assign( mp->table, pos, key );
        gr_assign( mp->table, pos + 1, value );

    } else {

        for ( ;; ) {

            pos = mp_next_pos_key( pos, gr_size( mp->table ) );

            if ( gr_item( mp->table, pos, gr_d ) == NULL ) {
                mp->used_cnt += 2;
                gr_assign( mp->table, pos, key );
                gr_assign( mp->table, pos + 1, value );
                break;
            } else if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), key ) ) {
                gr_assign( mp->table, pos, key );
                gr_assign( mp->table, pos + 1, value );
                break;
            }
        }
    }
}


gr_d mp_get_key( mp_t mp, const gr_d key )
{
    gr_size_t pos;
    gr_size_t start;

    pos = ( mp->key_hash( key ) % ( gr_size( mp->table ) >> 1 ) ) << 1;
    start = pos;

    do {

        if ( gr_item( mp->table, pos, gr_d ) == NULL )
            return NULL;

        if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), key ) )
            return gr_item( mp->table, pos + 1, gr_d );

        pos = mp_next_pos_key( pos, gr_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}


gr_d mp_del( mp_t mp, const gr_d value )
{
    gr_size_t pos;
    gr_size_t start;
    gr_d      ret;

    pos = mp->key_hash( value ) % gr_size( mp->table );
    start = pos;

    do {

        if ( gr_item( mp->table, pos, gr_d ) == NULL )
            return NULL;

        if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), value ) ) {
            ret = gr_item( mp->table, pos, gr_d );
            gr_assign( mp->table, pos, NULL );
            mp->used_cnt--;
            return ret;
        }

        pos = mp_next_pos( pos, gr_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}


gr_d mp_del_key( mp_t mp, const gr_d key )
{
    gr_size_t pos;
    gr_size_t start;
    gr_d      ret;

    pos = ( mp->key_hash( key ) % ( gr_size( mp->table ) >> 1 ) ) << 1;
    start = pos;

    do {

        if ( gr_item( mp->table, pos, gr_d ) == NULL )
            return NULL;

        if ( mp->key_comp( gr_item( mp->table, pos, const gr_d ), key ) ) {
            ret = gr_item( mp->table, pos + 1, gr_d );
            gr_assign( mp->table, pos, NULL );
            gr_assign( mp->table, pos + 1, NULL );
            mp->used_cnt -= 2;
            return ret;
        }

        pos = mp_next_pos_key( pos, gr_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}



/* ------------------------------------------------------------
 * Access functions:
 */

ag_hash_t mp_key_hash_cstr( const gr_d key )
{
    return aghs_64( (const void*)key, strlen( (char*)key ) );
}


int mp_key_comp_cstr( const gr_d a, const gr_d b )
{
    if ( strcmp( (char*)a, (char*)b ) == 0 )
        return 1;
    else
        return 0;
}


ag_hash_t mp_key_hash_slinky( const gr_d key )
{
    return aghs_64( (const void*)key, sl_length( (sl_t)key ) );
}


int mp_key_comp_slinky( const gr_d a, const gr_d b )
{
    if ( sl_compare( (sl_t)a, (sl_t)b ) == 0 )
        return 1;
    else
        return 0;
}


void mp_each( mp_t mp, mp_each_fn_p action, void* arg )
{
    gr_d key;

    for ( gr_size_t i = 0; i < gr_size( mp->table ); i++ ) {
        key = gr_item( mp->table, i, gr_d );
        if ( key )
            action( key, arg );
    }
}


void mp_each_key( mp_t mp, mp_each_key_fn_p action, void* arg )
{
    gr_d key;
    gr_d value;

    for ( gr_size_t i = 0; i < gr_size( mp->table ); i += 2 ) {
        key = gr_item( mp->table, i, gr_d );
        if ( key ) {
            value = gr_item( mp->table, i+1, gr_d );
            action( key, value, arg );
        }
    }
}



/* ------------------------------------------------------------
 * Internal support:
 */


/**
 * Return next slot position.
 *
 * Wrap index according to given size.
 *
 * @param pos  Current position.
 * @param size Table size.
 *
 * @return Next position.
 */
static gr_size_t mp_next_pos( gr_size_t pos, gr_size_t size )
{
    return ( pos + 1 ) % size;
}


/**
 * Return next slot position for key/value table.
 *
 * Wrap index according to given size.
 *
 * @param pos  Current position.
 * @param size Table size.
 *
 * @return Next position.
 */
static gr_size_t mp_next_pos_key( gr_size_t pos, gr_size_t size )
{
    return ( pos + 2 ) % size;
}


/**
 * Rehash table.
 *
 * @param mp       Mapper.
 * @param new_size New size.
 */
static void mp_rehash( mp_t mp, gr_size_t new_size )
{
    gr_t old_table;

    old_table = mp->table;
    mp->table = gr_new_sized( new_size );
    mp->used_cnt = 0;

    gr_d key;
    for ( gr_size_t i = 0; i < gr_size( old_table ); i++ ) {
        key = gr_item( old_table, i, gr_d );
        if ( key )
            mp_put( mp, key );
    }
}


/**
 * Rehash key/value table.
 *
 * @param mp       Mapper.
 * @param new_size New size.
 */
static void mp_rehash_key( mp_t mp, gr_size_t new_size )
{
    gr_t old_table;

    old_table = mp->table;
    mp->table = gr_new_sized( new_size );
    mp->used_cnt = 0;

    gr_d key;
    gr_d value;
    for ( gr_size_t i = 0; i < gr_size( old_table ); i += 2 ) {
        key = gr_item( old_table, i, gr_d );
        if ( key ) {
            value = gr_item( old_table, i+1, gr_d );
            mp_put_key( mp, key, value );
        }
    }
}
