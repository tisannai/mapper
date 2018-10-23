#ifndef MAPPER_H
#define MAPPER_H

/**
 * @file   mapper.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sun Apr 29 09:47:41 2018
 *
 * @brief  Mapper - Hash table (map).
 *
 */

#include <stdlib.h>
#include <stdint.h>

#include <gromer.h>
#include <alogir.h>


#ifndef MP_DEFAULT_SIZE
/** Default size for Mapper. */
#define MP_DEFAULT_SIZE 32
#endif

/** Default array fill level. */
#define MP_DEFAULT_FILL 50


#if MP_USE_MISS_CNT == 1
/** Default miss count limit for finding slot. */
#define MP_DEFAULT_MISS_CNT 16
#endif


/**
 * Calculate hash (64-bit) for key/object.
 */
typedef ag_hash_t ( *mp_key_hash_fn_p )( const gr_d key );


/**
 * Compare objects "obj" and "key". Return 1 if match, else 0.
 */
typedef int ( *mp_key_comp_fn_p )( const gr_d obj, const gr_d key );


/**
 * mp_each() action callback with used argument.
 */
typedef void ( *mp_each_fn_p )( gr_d value, void* arg );


/**
 * mp_each_key() action callback with used argument.
 */
typedef void ( *mp_each_key_fn_p )( gr_d key, gr_d value, void* arg );



/**
 * Mapper struct.
 */
struct mp_struct_s
{
    gr_t             table;    /**< Hash table (slots). */
    mp_key_hash_fn_p key_hash; /**< Key hashing function. */
    mp_key_comp_fn_p key_comp; /**< Key compare function. */
    gr_size_t        used_cnt; /**< Number of entries in table. */
    gr_size_t        fill_lim; /**< Storage limit percentage. */
#if MP_USE_MISS_CNT == 1
    gr_size_t miss_cnt; /**< Miss count limit for probing. */
#endif
};
typedef struct mp_struct_s mp_s; /**< Mapper struct. */
typedef mp_s*              mp_t; /**< Mapper pointer. */
typedef mp_t*              mp_p; /**< Mapper pointer reference. */



/* ------------------------------------------------------------
 * Create and destroy:
 */


/**
 * Create Mapper with defaults.
 *
 * Hash for string objects.
 *
 * @return Mapper.
 */
mp_t mp_new( void );


/**
 * Create Mapper with specific features.
 *
 * @param key_hash Key hash function.
 * @param key_comp Key compare function.
 * @param size     Size for hash table.
 * @param fill_lim Fill limit before resize (1-100%).
 *
 * @return Mapper.
 */
mp_t mp_new_full( mp_key_hash_fn_p key_hash,
                  mp_key_comp_fn_p key_comp,
                  gr_size_t        size,
                  gr_size_t        fill_lim );


/**
 * Create Mapper based on existing allocations.
 *
 * @param gr       Gromer handle.
 * @param key_hash Key hash function.
 * @param key_comp Key compare function.
 * @param fill_lim Fill limit before resize (1-100%).
 *
 * @return Mapper.
 */
mp_s mp_use( gr_t gr, mp_key_hash_fn_p key_hash, mp_key_comp_fn_p key_comp, gr_size_t fill_lim );


/**
 * Destroy Mapper.
 *
 * @param mp Mapper.
 *
 * @return NULL.
 */
mp_t mp_destroy( mp_t mp );


/**
 * Put value to Mapper.
 *
 * Key is expected to be included within the value and this should be
 * reflected in key-hash-function.
 *
 * @param mp    Mapper.
 * @param value Object including key.
 */
void mp_put( mp_t mp, const gr_d value );


/**
 * Get value from Mapper.
 *
 * Key is expected to be included within the value and this should be
 * reflected in key-hash-function.
 *
 * @param mp    Mapper.
 * @param value Object including key.
 *
 * @return Object.
 */
gr_d mp_get( mp_t mp, const gr_d value );


/**
 * Put value Mapper using Key.
 *
 * Key and object are stored separately. Uses double the space
 * compared to mp_put().
 *
 * @param mp    Mapper.
 * @param key   Hash key.
 * @param value Object.
 */
void mp_put_key( mp_t mp, const gr_d key, const gr_d value );


/**
 * Get value from Mapper using Key.
 *
 * @param mp  Mapper.
 * @param key Key to Object.
 *
 * @return Object.
 */
gr_d mp_get_key( mp_t mp, const gr_d key );


/**
 * Delete value from Mapper.
 *
 * @param mp    Mapper.
 * @param value Object including key.
 *
 * @return Deleted Object.
 */
gr_d mp_del( mp_t mp, const gr_d value );


/**
 * Delete value from Mapper using Key.
 *
 * @param mp  Mapper.
 * @param key Key to Object.
 *
 * @return Deleted Object.
 */
gr_d mp_del_key( mp_t mp, const gr_d key );



/* ------------------------------------------------------------
 * Access functions:
 */

/**
 * Hash function for C-string.
 *
 * @param key C-string.
 *
 * @return 64-bit hash.
 */
ag_hash_t mp_key_hash_cstr( const gr_d key );


/**
 * Compare for C-string objects.
 *
 * @param a Object a.
 * @param b Object b.
 *
 * @return 1 if match, else 0.
 */
int mp_key_comp_cstr( const gr_d a, const gr_d b );


/**
 * Hash function for Slinky.
 *
 * @param key Slinky.
 *
 * @return 64-bit hash.
 */
ag_hash_t mp_key_hash_slinky( const gr_d key );


/**
 * Compare for Slinky objects.
 *
 * @param a Object a.
 * @param b Object b.
 *
 * @return 1 if match, else 0.
 */
int mp_key_comp_slinky( const gr_d a, const gr_d b );


/**
 * Process each entry in Mapper.
 * 
 * @param mp     Mapper.
 * @param action Action for entry.
 * @param arg    User argument for action.
 */
void mp_each( mp_t mp, mp_each_fn_p action, void* arg );


/**
 * Process each entry in Mapper.
 * 
 * @param mp     Mapper.
 * @param action Action for entry.
 * @param arg    User argument for action.
 */
void mp_each_key( mp_t mp, mp_each_key_fn_p action, void* arg );


#endif
