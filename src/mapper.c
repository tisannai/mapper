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


static po_size_t mp_next_pos( po_size_t pos, po_size_t size );
static po_size_t mp_next_key_pos( po_size_t pos, po_size_t size );
static void      mp_rehash( mp_t mp, po_size_t new_size );
static void      mp_rehash_key( mp_t mp, po_size_t new_size );



/* ------------------------------------------------------------
 * Create and destroy:
 */


mp_t mp_new( mp_t mp )
{
    return mp_new_full( mp, mp_key_hash_cstr, mp_key_comp_cstr, MP_DEFAULT_SIZE, MP_DEFAULT_FILL );
}


mp_t mp_new_full( mp_t mp,
                  mp_key_hash_fn_p key_hash,
                  mp_key_comp_fn_p key_comp,
                  po_size_t        size,
                  po_size_t        fill_lim )
{
    if ( mp == NULL ) {
        mp = po_malloc( sizeof( mp_s ) );
    }
    mp->table = po_new_sized( &mp->table_desc, size );
    mp->key_hash = key_hash;
    mp->key_comp = key_comp;
    mp->used_cnt = 0;
    mp->fill_lim = fill_lim;
    mp->rehash_cb = NULL;
    mp->rehash_env = NULL;

    return mp;
}


mp_t mp_use( mp_t mp, po_t po, mp_key_hash_fn_p key_hash, mp_key_comp_fn_p key_comp, po_size_t fill_lim )
{
    mp->table = po;
    mp->key_hash = key_hash;
    mp->key_comp = key_comp;
    mp->used_cnt = 0;
    mp->fill_lim = fill_lim;
    mp->rehash_cb = NULL;
    mp->rehash_env = NULL;

    return mp;
}


mp_t mp_destroy( mp_t mp )
{
    po_destroy_storage( mp->table );
    po_free( mp );
    return NULL;
}


void mp_destroy_table( mp_t mp )
{
    po_destroy_storage( mp->table );
}


void mp_clear( mp_t mp )
{
    mp->used_cnt = 0;
    po_clear( mp->table );
}


void mp_set_rehash_cb( mp_t mp, mp_rehash_fn_p cb, void* env )
{
    mp->rehash_cb = cb;
    mp->rehash_env = env;
}


po_size_t mp_get_index( mp_t mp, const po_d value )
{
    po_size_t pos;

    pos = mp->key_hash( value ) % po_size( mp->table );
    if ( po_item( mp->table, pos, po_d ) == NULL ) {
        return pos;
    } else if ( mp->key_comp( po_item( mp->table, pos, const po_d ), value ) ) {
        return pos;
    } else {
        for ( ;; ) {
            pos = mp_next_pos( pos, po_size( mp->table ) );
            if ( po_item( mp->table, pos, po_d ) == NULL ) {
                return pos;
            } else if ( mp->key_comp( po_item( mp->table, pos, const po_d ), value ) ) {
                return pos;
            }
        }
    }
}


po_d mp_get_with_index( mp_t mp, po_size_t index )
{
    return po_item( mp->table, index, po_d );
}


po_size_t mp_get_key_index( mp_t mp, const po_d key )
{
    po_size_t pos;

    pos = ( mp->key_hash( key ) % ( po_size( mp->table ) >> 1 ) ) << 1;

    if ( po_item( mp->table, pos, po_d ) == NULL ) {
        return pos;
    } else if ( mp->key_comp( po_item( mp->table, pos, const po_d ), key ) ) {
        return pos;
    } else {
        for ( ;; ) {
            pos = mp_next_key_pos( pos, po_size( mp->table ) );
            if ( po_item( mp->table, pos, po_d ) == NULL ) {
                return pos;
            } else if ( mp->key_comp( po_item( mp->table, pos, const po_d ), key ) ) {
                return pos;
            }
        }
    }
}


po_size_t mp_put( mp_t mp, const po_d value )
{
    if ( ( ( mp->used_cnt * 100 ) / po_size( mp->table ) ) >= mp->fill_lim ) {
        mp_rehash( mp, po_size( mp->table ) * 2 );
    }

    po_size_t pos;
    pos = mp_get_index( mp, value );
    if ( po_item( mp->table, pos, po_d ) == NULL )
        mp->used_cnt++;
    po_assign( mp->table, pos, value );
    return pos;
}


po_d mp_get( mp_t mp, const po_d value )
{
    po_size_t pos;
    po_size_t start;

    pos = mp->key_hash( value ) % po_size( mp->table );
    start = pos;

    do {

        if ( po_item( mp->table, pos, po_d ) == NULL )
            return NULL;

        if ( mp->key_comp( po_item( mp->table, pos, const po_d ), value ) )
            return po_item( mp->table, pos, po_d );

        pos = mp_next_pos( pos, po_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}


po_size_t mp_put_key( mp_t mp, const po_d key, const po_d value )
{
    if ( ( ( mp->used_cnt * 100 ) / po_size( mp->table ) ) >= mp->fill_lim ) {
        mp_rehash_key( mp, po_size( mp->table ) * 2 );
    }

    po_size_t pos;
    pos = mp_get_key_index( mp, key );
    if ( po_item( mp->table, pos, po_d ) == NULL )
        mp->used_cnt += 2;
    po_assign( mp->table, pos, key );
    po_assign( mp->table, pos + 1, value );
    return pos;
}


po_d mp_get_key( mp_t mp, const po_d key )
{
    po_size_t pos;
    po_size_t start;

    pos = ( mp->key_hash( key ) % ( po_size( mp->table ) >> 1 ) ) << 1;
    start = pos;

    do {

        if ( po_item( mp->table, pos, po_d ) == NULL )
            return NULL;

        if ( mp->key_comp( po_item( mp->table, pos, const po_d ), key ) )
            return po_item( mp->table, pos + 1, po_d );

        pos = mp_next_key_pos( pos, po_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}


po_d mp_del( mp_t mp, const po_d value )
{
    po_size_t pos;
    po_size_t start;
    po_d      ret;

    pos = mp->key_hash( value ) % po_size( mp->table );
    start = pos;

    do {

        if ( po_item( mp->table, pos, po_d ) == NULL )
            return NULL;

        if ( mp->key_comp( po_item( mp->table, pos, const po_d ), value ) ) {
            ret = po_item( mp->table, pos, po_d );
            po_assign( mp->table, pos, NULL );
            mp->used_cnt--;
            return ret;
        }

        pos = mp_next_pos( pos, po_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}


po_d mp_del_key( mp_t mp, const po_d key )
{
    po_size_t pos;
    po_size_t start;
    po_d      ret;

    pos = ( mp->key_hash( key ) % ( po_size( mp->table ) >> 1 ) ) << 1;
    start = pos;

    do {

        if ( po_item( mp->table, pos, po_d ) == NULL )
            return NULL;

        if ( mp->key_comp( po_item( mp->table, pos, const po_d ), key ) ) {
            ret = po_item( mp->table, pos + 1, po_d );
            po_assign( mp->table, pos, NULL );
            po_assign( mp->table, pos + 1, NULL );
            mp->used_cnt -= 2;
            return ret;
        }

        pos = mp_next_key_pos( pos, po_size( mp->table ) );

    } while ( start != pos );

    return NULL;
}



/* ------------------------------------------------------------
 * Access functions:
 */

ag_hash_t mp_key_hash_cstr( const po_d key )
{
    return aghs_64( (const void*)key, strlen( (char*)key ) );
}


int mp_key_comp_cstr( const po_d a, const po_d b )
{
    if ( strcmp( (char*)a, (char*)b ) == 0 )
        return 1;
    else
        return 0;
}


ag_hash_t mp_key_hash_slinky( const po_d key )
{
    return aghs_64( (const void*)key, sl_length( (sl_t)key ) );
}


int mp_key_comp_slinky( const po_d a, const po_d b )
{
    if ( sl_compare( (sl_t)a, (sl_t)b ) == 0 )
        return 1;
    else
        return 0;
}


void mp_each( mp_t mp, mp_each_fn_p action, void* arg )
{
    po_d key;

    for ( po_size_t i = 0; i < po_size( mp->table ); i++ ) {
        key = po_item( mp->table, i, po_d );
        if ( key )
            action( key, arg );
    }
}


void mp_each_key( mp_t mp, mp_each_key_fn_p action, void* arg )
{
    po_d key;
    po_d value;

    for ( po_size_t i = 0; i < po_size( mp->table ); i += 2 ) {
        key = po_item( mp->table, i, po_d );
        if ( key ) {
            value = po_item( mp->table, i + 1, po_d );
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
static po_size_t mp_next_pos( po_size_t pos, po_size_t size )
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
static po_size_t mp_next_key_pos( po_size_t pos, po_size_t size )
{
    return ( pos + 2 ) % size;
}


/**
 * Rehash table.
 *
 * @param mp       Mapper.
 * @param new_size New size.
 */
static void mp_rehash( mp_t mp, po_size_t new_size )
{
    po_s old_table;

    old_table = mp->table_desc;
    mp->table = po_new_sized( &mp->table_desc, new_size );
    mp->used_cnt = 0;

    po_d key;
    for ( po_size_t i = 0; i < po_size( &old_table ); i++ ) {
        key = po_item( &old_table, i, po_d );
        if ( key ) {
            mp_put( mp, key );
        }
    }

    if ( mp->rehash_cb ) {
        mp->rehash_cb( mp, mp->rehash_env );
    }

    po_destroy_storage( &old_table );
}


/**
 * Rehash key/value table.
 *
 * @param mp       Mapper.
 * @param new_size New size.
 */
static void mp_rehash_key( mp_t mp, po_size_t new_size )
{
    po_s old_table;

    old_table = mp->table_desc;
    mp->table = po_new_sized( &mp->table_desc, new_size );
    mp->used_cnt = 0;

    po_d key;
    po_d value;
    for ( po_size_t i = 0; i < po_size( &old_table ); i += 2 ) {
        key = po_item( &old_table, i, po_d );
        if ( key ) {
            value = po_item( &old_table, i + 1, po_d );
            mp_put_key( mp, key, value );
        }
    }

    if ( mp->rehash_cb ) {
        mp->rehash_cb( mp, mp->rehash_env );
    }

    po_destroy_storage( &old_table );
}
