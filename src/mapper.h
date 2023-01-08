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

#include <postor.h>
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


struct mp_struct_s;
typedef struct mp_struct_s mp_s; /**< Mapper struct. */
typedef mp_s*              mp_t; /**< Mapper pointer. */
typedef mp_t*              mp_p; /**< Mapper pointer reference. */


/**
 * Calculate hash (64-bit) for key/object.
 */
typedef ag_hash_t ( *mp_key_hash_fn_p )( const po_d key );


/**
 * Compare objects "obj" and "key". Return 1 if match, else 0.
 */
typedef int ( *mp_key_comp_fn_p )( const po_d obj, const po_d key );


/**
 * mp_each() action callback with used argument.
 */
typedef void ( *mp_each_fn_p )( po_d value, void* arg );


/**
 * mp_each_key() action callback with used argument.
 */
typedef void ( *mp_each_key_fn_p )( po_d key, po_d value, void* arg );


/**
 * mp_rehash() and mp_rehash_key() action callback. Called after
 * rehash is done with user arg.
 */
typedef void ( *mp_rehash_fn_p )( mp_t mp, void* env );



/**
 * Mapper struct.
 */
struct mp_struct_s
{
    po_s             table_desc; /**< Poster descriptor. */
    po_t             table;      /**< Hash table (slots). */
    mp_key_hash_fn_p key_hash;   /**< Key hashing function. */
    mp_key_comp_fn_p key_comp;   /**< Key compare function. */
    po_size_t        used_cnt;   /**< Number of entries in table. */
    po_size_t        fill_lim;   /**< Storage limit percentage. */
    mp_rehash_fn_p   rehash_cb;  /**< Optional rehash callback. */
    void*            rehash_env; /**< Context for rehash callback. */
#if MP_USE_MISS_CNT == 1
    po_size_t miss_cnt; /**< Miss count limit for probing. */
#endif
};



/* ------------------------------------------------------------
 * Create and destroy:
 */


/**
 * Create Mapper with defaults.
 *
 * Hash for string objects.
 * 
 * If mp is NULL, descriptor is allocated from heap. This
 * type of descriptor must be freed by the user after use.
 *
 * @param mp Mapper or NULL.
 *
 * @return Mapper.
 */
mp_t mp_new( mp_t mp );


/**
 * Create Mapper with specific features.
 * 
 * If mp is NULL, descriptor is allocated from heap. This
 * type of descriptor must be freed by the user after use.
 *
 * @param mp       Mapper or NULL.
 * @param key_hash Key hash function.
 * @param key_comp Key compare function.
 * @param size     Size for hash table.
 * @param fill_lim Fill limit before resize (1-100%).
 *
 * @return Mapper.
 */
mp_t mp_new_full( mp_t mp,
                  mp_key_hash_fn_p key_hash,
                  mp_key_comp_fn_p key_comp,
                  po_size_t        size,
                  po_size_t        fill_lim );


/**
 * Create Mapper based on existing allocations.
 *
 * @param mp       Mapper.
 * @param po       Postor handle.
 * @param key_hash Key hash function.
 * @param key_comp Key compare function.
 * @param fill_lim Fill limit before resize (1-100%).
 *
 * @return Mapper.
 */
mp_t mp_use( mp_t mp, po_t po, mp_key_hash_fn_p key_hash, mp_key_comp_fn_p key_comp, po_size_t fill_lim );


/**
 * Destroy Mapper.
 *
 * @param mp Mapper.
 *
 * @return NULL.
 */
mp_t mp_destroy( mp_t mp );


/**
 * Destroy Mapper table.
 *
 * @param mp Mapper.
 *
 * @return NULL.
 */
void mp_destroy_table( mp_t mp );


/**
 * Reset Mapper, i.e. clear content.
 *
 * @param mp Mapper.
 *
 * @return NULL.
 */
void mp_reset( mp_t mp );


/**
 * Clear Mapper, i.e. reset and clear data.
 *
 * @param mp Mapper.
 */
void mp_clear( mp_t mp );


/**
 * Set hash callback function and env.
 *
 * @param mp  Mapper.
 * @param cb  Callback function.
 * @param env Callback env.
 */
void mp_set_rehash_cb( mp_t mp, mp_rehash_fn_p cb, void* env );


/**
 * Return table index.
 *
 * @param mp    Mapper.
 * @param value Object including key.
 *
 * @return Table index.
 */
po_size_t mp_get_index( mp_t mp, const po_d value );


/**
 * Return table index using key.
 *
 * @param mp    Mapper.
 * @param key   Key.
 *
 * @return Table index.
 */
po_size_t mp_get_key_index( mp_t mp, const po_d key );


/**
 * Get value from Mapper with index.
 *
 * @param mp    Mapper.
 * @param index Index.
 *
 * @return Object (or NULL).
 */
po_d mp_get_with_index( mp_t mp, po_size_t index );


/**
 * Put value to Mapper.
 *
 * Key is expected to be included within the value and this should be
 * reflected in key-hash-function.
 *
 * @param mp    Mapper.
 * @param value Object including key.
 *
 * @return Table index.
 */
po_size_t mp_put( mp_t mp, const po_d value );


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
po_d mp_get( mp_t mp, const po_d value );


/**
 * Put value Mapper using Key.
 *
 * Key and object are stored separately. Uses double the space
 * compared to mp_put().
 *
 * @param mp    Mapper.
 * @param key   Hash key.
 * @param value Object.
 *
 * @return Table index.
 */
po_size_t mp_put_key( mp_t mp, const po_d key, const po_d value );


/**
 * Get value from Mapper using Key.
 *
 * @param mp  Mapper.
 * @param key Key to Object.
 *
 * @return Object.
 */
po_d mp_get_key( mp_t mp, const po_d key );


/**
 * Delete value from Mapper.
 *
 * @param mp    Mapper.
 * @param value Object including key.
 *
 * @return Deleted Object.
 */
po_d mp_del( mp_t mp, const po_d value );


/**
 * Delete value from Mapper using Key.
 *
 * @param mp  Mapper.
 * @param key Key to Object.
 *
 * @return Deleted Object.
 */
po_d mp_del_key( mp_t mp, const po_d key );



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
ag_hash_t mp_key_hash_cstr( const po_d key );


/**
 * Compare for C-string objects.
 *
 * @param a Object a.
 * @param b Object b.
 *
 * @return 1 if match, else 0.
 */
int mp_key_comp_cstr( const po_d a, const po_d b );


/**
 * Hash function for Slinky.
 *
 * @param key Slinky.
 *
 * @return 64-bit hash.
 */
ag_hash_t mp_key_hash_slinky( const po_d key );


/**
 * Compare for Slinky objects.
 *
 * @param a Object a.
 * @param b Object b.
 *
 * @return 1 if match, else 0.
 */
int mp_key_comp_slinky( const po_d a, const po_d b );


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
