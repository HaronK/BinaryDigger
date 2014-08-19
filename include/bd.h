
#ifndef BD_H_
#define BD_H_

#include "types.h"
#include "defines.h"

BD_C_EXTERN_BEGIN

typedef enum
{
    BD_TEMPL  = 0, // user defined template
    BD_STRUCT = 1, // user defined plain structure

    // simple types
#define BD_DECL_BLOCK_TYPE(name, type) BD_##name,
#include "block_types_decl.h"
} bd_block_type;

// Template types
#define BD_DECL_BLOCK_TYPE(name, type) typedef type name##_T;
#include "block_types_decl.h"

typedef struct bd_block_io
{
    bd_result (*get_pos)   (bd_block_io *self, bd_u64 *pos);
    bd_result (*set_pos)   (bd_block_io *self, bd_u64 pos);
    bd_result (*shift_pos) (bd_block_io *self, bd_u64 offset);

    bd_result (*get_data)  (bd_block_io *self, bd_u64 size, bd_pointer val);
    bd_result (*get_datap) (bd_block_io *self, bd_u64 pos, bd_u64 size, bd_pointer val);
} bd_block_io;

typedef struct bd_block
{
    bd_string name;      // user block name. cannot be changed outside of plugin
    bd_string type_name; // user block type name. cannot be changed outside of plugin

    bd_block_type type;

    bd_u64 offset; // element offset on the block blob
    bd_u64 size;   // element size

    bd_bool is_array; // block is array
    bd_u64 elem_size; // array element size or 0 for templates
    bd_u32 count;     // count of array elements

    bd_block *parent;
    struct
    {
        bd_block **child;
        bd_u32 count;
    } children;
} bd_block;

/**
 * Get text message by its result code.
 *
 * @param result Result code.
 * @param msg [OUT] Message string.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_result_message(bd_result result, bd_string msg, bd_u32 msg_size);
typedef bd_result (*bd_result_message_t)(bd_result result, bd_string msg, bd_u32 msg_size);

/**
 * Initialize plugin.
 *
 * @param name Plugin name.
 * @param templ_count [OUT] Return registered plugin templetes count.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_initialize_plugin(bd_string *name, bd_u32 *templ_count);
typedef bd_result (*bd_initialize_plugin_t)(bd_string *name, bd_u32 *templ_count);

/**
 * Retrieve template name by index.
 *
 * @param index Template index
 * @param name [OUT] Template name
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_template_name(bd_u32 index, bd_string *name);
typedef bd_result (*bd_template_name_t)(bd_u32 index, bd_string *name);

/**
 * Apply template to the blob data and return items hierarchy.
 *
 * @param index       Template index
 * @param block_io    Block blob data accessor
 * @param block [OUT] Blocks hierarchy
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_apply_template(bd_u32 index, bd_block_io *block_io, bd_block **block, bd_cstring script);
typedef bd_result (*bd_apply_template_t)(bd_u32 index, bd_block_io *block_io, bd_block **block, bd_cstring script);

/**
 * Free all items hierarchy data.
 *
 * @param index Template index
 * @param block Blocks hierarchy
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_free_template(bd_u32 index, bd_block *block);
typedef bd_result (*bd_free_template_t)(bd_u32 index, bd_block *block);

/**
 * Finalize plugin.
 *
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_finalize_plugin();
typedef bd_result (*bd_finalize_plugin_t)();

typedef struct
{
    bd_result_message_t    result_message;
    bd_initialize_plugin_t initialize_plugin;
    bd_template_name_t     template_name;
    bd_apply_template_t    apply_template;
    bd_free_template_t     free_template;
    bd_finalize_plugin_t   finalize_plugin;
} bd_plugin;

BD_C_EXTERN_END

#endif /* BD_H_ */
