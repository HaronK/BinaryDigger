
#ifndef BD_H_
#define BD_H_

#include "types.h"
#include "defines.h"

BD_C_EXTERN_BEGIN

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

#define is_string(block) (block != BD_NULL && ((block->type == BD_CHAR && block->is_array == BD_TRUE) || block->type == BD_STRING))

typedef struct bd_message
{
    bd_result  code;  // message result code
    bd_cstring msg;   // message text
    bd_pointer block; // block to which this message relates
    bd_u32     count; // number of occurrences of this message (to avoid duplications)
} bd_message;

typedef struct bd_messages
{
    bd_message *message; // array of messages
    bd_u16      count;   // number of messages
} bd_messages;

/**
 * Get last operation messages.
 *
 * @param messages [OUT] Array of messages.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_get_messages(bd_messages **messages);
typedef bd_result (*bd_get_messages_t)(bd_messages **messages);

/**
 * Get text message by its result code.
 *
 * @param result    Result code.
 * @param msg [OUT] Message buffer.
 * @param msg_size  Message buffer size.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_result_message(bd_result result, bd_string msg, bd_u32 msg_size);
typedef bd_result (*bd_result_message_t)(bd_result result, bd_string msg, bd_u32 msg_size);

/**
 * Initialize plugin.
 *
 * @param name              Plugin name.
 * @param templ_count [OUT] Return registered plugin templetes count.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_initialize_plugin(bd_string name, bd_u32 name_size, bd_u32 *templ_count);
typedef bd_result (*bd_initialize_plugin_t)(bd_string name, bd_u32 name_size, bd_u32 *templ_count);

/**
 * Finalize plugin.
 *
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_finalize_plugin();
typedef bd_result (*bd_finalize_plugin_t)();

/**
 * Retrieve template name by index.
 *
 * @param index      Template index
 * @param name [OUT] Template name
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_template_name(bd_u32 index, bd_string name, bd_u32 name_size);
typedef bd_result (*bd_template_name_t)(bd_u32 index, bd_string name, bd_u32 name_size);

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
 * Get string representation of block value. Can be applied only to the basic types: int, string, ...
 *
 * @param block       Blocks item
 * @param buf   [OUT] Result buffer
 * @param size        Result buffer size
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_get_string_value(bd_block *block, bd_string buf, bd_u32 size);
typedef bd_result (*bd_get_string_value_t)(bd_block *block, bd_string buf, bd_u32 size);


typedef struct bd_plugin
{
    bd_get_messages_t      get_messages;
    bd_result_message_t    result_message; // obsolete

    bd_initialize_plugin_t initialize_plugin;
    bd_finalize_plugin_t   finalize_plugin;

    bd_template_name_t     template_name;
    bd_apply_template_t    apply_template;
    bd_free_template_t     free_template;

    bd_get_string_value_t  get_string_value;
} bd_plugin;

BD_C_EXTERN_END

#endif /* BD_H_ */
