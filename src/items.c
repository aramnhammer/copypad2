#include "types.h"
#include <stdlib.h>
#include <string.h>

/*
FORWARD DECLARATIONS
*/
bool itemMatchesFilter(const Item *item, const char *filter);
Clay_String clippedClayString(const char *str);
Item_Data InitItems(void);

Item_Data InitItems()
{
    itemArray.items = malloc(sizeof(Item) * 10);
    Item_Data itemData = {
        .selectedItemIndex = 0,
        .yOffset = 0.0f,
        .itemsArena = {
            .memory = malloc(1024 * 1024),
            .offset = 0},
        .items = &itemArray};

    return itemData;
}

// Function to check if an item matches the search filter
bool itemMatchesFilter(const Item *item, const char *filter)
{
    if (strlen(filter) == 0)
        return true; // Show all items if no filter

    // Convert both strings to lowercase for case-insensitive search
    char itemTitle[256];
    char filterLower[256];

    strncpy(itemTitle, item->title.chars, sizeof(itemTitle) - 1);
    itemTitle[sizeof(itemTitle) - 1] = '\0';
    strncpy(filterLower, filter, sizeof(filterLower) - 1);
    filterLower[sizeof(filterLower) - 1] = '\0';

    // Simple lowercase conversion
    for (int i = 0; itemTitle[i]; i++)
    {
        if (itemTitle[i] >= 'A' && itemTitle[i] <= 'Z')
        {
            itemTitle[i] = itemTitle[i] + 32;
        }
    }
    for (int i = 0; filterLower[i]; i++)
    {
        if (filterLower[i] >= 'A' && filterLower[i] <= 'Z')
        {
            filterLower[i] = filterLower[i] + 32;
        }
    }

    return strstr(itemTitle, filterLower) != NULL;
}
