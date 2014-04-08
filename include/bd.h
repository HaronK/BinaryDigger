
#ifndef BD_H_
#define BD_H_

#include "types.h"
#include "defines.h"

BD_C_EXTERN_BEGIN

typedef enum
{
    BD_IT_TEMPL  = 0, // user defined template
    BD_IT_STRUCT = 1, // user defined plain structure

    // simple types
    BD_IT_CHAR,
    BD_IT_UCHAR,
    BD_IT_WORD,
    BD_IT_DWORD,
    BD_IT_QWORD,
    BD_IT_DOUBLE,
} bd_item_type;

// Template types
typedef bd_i8  CHAR_T;
typedef bd_u8  UCHAR_T;
typedef bd_i16 WORD_T;
typedef bd_i32 DWORD_T;
typedef bd_i64 QWORD_T;
typedef bd_f64 DOUBLE_T;

typedef struct bd_templ_blob
{
    bd_result (*get_pos)   (bd_templ_blob *self, bd_u64 *pos);
    bd_result (*set_pos)   (bd_templ_blob *self, bd_u64 pos);
    bd_result (*shift_pos) (bd_templ_blob *self, bd_u64 offset);

    bd_result (*get_data)  (bd_templ_blob *self, bd_u64 size, bd_pointer val);
    bd_result (*get_datap) (bd_templ_blob *self, bd_u64 pos, bd_u64 size, bd_pointer val);
} bd_templ_blob;

typedef struct bd_item
{
    bd_string name;      // user item name. cannot be changed outside of plugin
    bd_string type_name; // user item type name. cannot be changed outside of plugin

    bd_item_type type;

    bd_u64 offset; // element offset on the blob
    bd_u64 size;   // element size

    bd_bool is_array;   //
    bd_u64 elem_size;   // array element size or 0 for templates
    bd_u32 count;       // count of array elements

    bd_item *parent;
    struct
    {
        bd_item **child;
        bd_u32 count;
    } children;
} bd_item;

/**
 * Get text message by its result code.
 * @param result Result code.
 * @param msg [OUT] Message string.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_result_message(bd_result result, bd_string msg, bd_u32 msg_size);
typedef bd_result (*bd_result_message_t)(bd_result result, bd_string msg, bd_u32 msg_size);

/**
 * Initialize plugin.
 * @param name Plugin name.
 * @param templ_count [OUT] Return registered plugin templetes count.
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_initialize_plugin(bd_string *name, bd_u32 *templ_count);
typedef bd_result (*bd_initialize_plugin_t)(bd_string *name, bd_u32 *templ_count);

/**
 * Retrieve template name by index.
 * @param index Template index
 * @param name [OUT] Template name
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_template_name(bd_u32 index, bd_string *name);
typedef bd_result (*bd_template_name_t)(bd_u32 index, bd_string *name);

/**
 * Apply template to the blob data and return items hierarchy.
 * @param index Template index
 * @param blob Blob data accessor
 * @param item [OUT] Items hierarchy
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_apply_template(bd_u32 index, bd_templ_blob *blob, bd_item **item, bd_cstring script);
typedef bd_result (*bd_apply_template_t)(bd_u32 index, bd_templ_blob *blob, bd_item **item, bd_cstring script);

/**
 * Free all items hierarchy data.
 * @param index Template index
 * @param item Items hierarchy
 * @return Result code @see(bd_result) for details
 */
BD_EXPORT bd_result bd_free_template(bd_u32 index, bd_item *item);
typedef bd_result (*bd_free_template_t)(bd_u32 index, bd_item *item);

/**
 * Finalize plugin.
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
