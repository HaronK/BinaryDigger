/*
 * PluginLibrary.h
 *
 *  Created on: Sep 12, 2015
 *      Author: oleg
 */

#ifndef TESTS_PLUGINLIBRARY_H_
#define TESTS_PLUGINLIBRARY_H_


bd_u64 calc_item_tree_size(const bd_block *item)
{
    bd_u64 result = sizeof(bd_block);
    if (item->children.count > 0)
    {
        for (bd_u32 i = 0; i < item->children.count; ++i)
        {
            result += calc_item_tree_size(item->children.child[i]);
        }
    }
    return result;
}

const char single_indent[] = "  ";
void dump_item(const bd_block *item, const std::string& indent = "")
{
    printf("%-7s %-15s<%d>: %8lXh %6lXh",
            item->type_name, item->name, item->children.count,
            item->offset, (item->size * item->count));
    if (item->is_array)
    {
        printf(" [%lu %u]", item->size, item->count);
    }
    printf("\n");
    if (item->children.count > 0)
    {
        for (bd_u32 i = 0; i < item->children.count; ++i)
        {
            printf("%s[%u] ", indent.c_str(), i);
            dump_item(item->children.child[i], indent + single_indent + "  ");
        }
    }
}

#endif /* TESTS_PLUGINLIBRARY_H_ */
