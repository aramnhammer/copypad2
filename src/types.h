// types.h
#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>
#include "../../../lib/clay/clay.h"

#define MAX_ITEMS 2046

const int FONT_ID_BODY_16 = 0;
const float screenWidth = 400.0f;
const float screenHeight = 592.0f;

typedef struct
{
    Clay_String title;
    Clay_String content;
    uint64_t hash;
} Item;

typedef struct {
    uint64_t hash;
    uint32_t index;
} ItemHashEntry;

typedef struct {
    ItemHashEntry entries[MAX_ITEMS];
    uint32_t count;
} ItemHashMap;

typedef struct
{
    Item *items;
    uint32_t itemCount;
} Items;

Items itemArray = {
    .items = NULL,
    .itemCount = 0,
};

typedef struct
{
    intptr_t offset;
    void *memory;
    ItemHashMap hashMap;
} ItemsArena;

typedef struct
{
    int32_t selectedItemIndex;
    float yOffset;
    ItemsArena itemsArena;
    Items *items;
} Item_Data;

typedef struct
{
    char *buffer;
    uint64_t hash;
} ClipboardData;

#endif