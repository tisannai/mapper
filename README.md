# Mapper - Hash table (map)

Mapper is hash table implementation with Gromer based table
storage. It grows automatically when adjustable fill level is
reached. Mapper can be used as object store (Object Mode), where the
hash key is contained within the object. Mapper can also be used as
key/value store (Key Mode), where the hash key is stored separately
from the object (value). Object Mode consumes half the memory compared
to Key Mode.

Mapper is uses open addressing and linear probing. Both are selected
for best possible cache locality. Mapper has adjustable storage
limit. Typically we want to fill only half of the available slots, in
order to reduce the number of collisions. When table requires
resizing, the table size is doubled and all keys are rehashed (in one
go).

Mapper can be created with fresh allocations from heap or with
existing allocations. Simplest way to create a new Mapper:

    mp_t mp;
    mp = mp_new();

This creates a new Mapper from heap with default size (32) and default
fill limit (50%). The key hash and key compare functions are for
C-strings. This is not what user typically wants, and hence it is
common to create the Mapper with full specification.

    mp = mp_new_full( hash_obj, comp_obj, 128, 50 );

This creates Mapper with custom object hash (`hash_obj`) and object
compare (`comp_obj`) functions. The table size is 128 (objects) and
fill limit is 50%. Note that if Key Mode is in use, only 64 (half of
128) key/value pairs will fit into the table.

User can add entries in either Object or Key Mode. In Object Mode we
do:

    mp_put( mp, obj );

The `obj` parameter contains the key and `hash_obj` (stored in `mp`)
is used to create the hash key. If there is a collision to an existing
entry, then additionally `comp_obj` function is used to compare the
colliding entries. Linear probing continues until a free slot has been
found. `mp_put` consumes one slot from table, since only `obj` is
stored.

Key Mode storing is performed as:

    mp_put_key( mp, key, obj );

`key` is used for hashing and a free slot is searched. In Key Mode
Mapper stores first the `key` and then the related `obj` into
consecutive slots. Two slots are thus consumed for each key/value
entry.

Entries are retreived with:

    ret = mp_get( mp, obj );

The stored object (`obj`) is returned if it is found from Mapper, and
otherwise `NULL`. In Object Mode Mapper is hence used for lookup
purposes.

In Key Mode the operation is:

    obj = mp_get_key( mp, key );

Entries can be deleted with `mp_del` and `mp_del_key` for Object and
Key Mode respectively.

When Mapper is not needed any more, it can be destroyed with:

    mp_destroy( mp );

Mapper is destroyed, but stored entries is are not affected. If user
wants to process keys and values, `mp_each` or `mp_each_key` can be
used for this purpose.



## Mapper API documentation

See Doxygen documentation. Documentation can be created with:

    shell> doxygen .doxygen


## Examples

All functions and their use is visible in tests. Please refer `test`
directory for testcases.


## Building

Ceedling based flow is in use:

    shell> ceedling

Testing:

    shell> ceedling test:all

User defines can be placed into `project.yml`. Please refer to
Ceedling documentation for details.


## Ceedling

Mapper uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new mapper

in the directory above Mapper. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.
