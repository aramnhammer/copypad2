#define CLAY_IMPLEMENTATION
#include "raylib.h"
#include "../../lib/clay/clay.h"
#include "../../lib/clay/renderers/raylib/clay_renderer_raylib.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int FONT_ID_BODY_16 = 0;
Color COLOR_LIGHT = {224, 215, 210, 255};
Color COLOR_GREY = {150, 145, 148, 1};
Color COLOR_ORANGE = {225, 138, 50, 255};
Clay_Color COLOR_WHITE = {255, 255, 255, 255};
Clay_Color COLOR_RED = {168, 66, 28, 255};
Clay_Color COLOR_BLACK = {0, 0, 0, 255};
Clay_Color COLOR_BLUE = {100, 150, 255, 255};

const float screenWidth = 1480.0f;
const float screenHeight = 792.0f;

char *poll_clipboard()
{
    FILE *fp = popen("pbpaste", "r");
    if (!fp)
        return NULL;

    char *buffer = NULL;
    size_t size = 0;
    size_t len = 0;
    char temp[256];

    while (fgets(temp, sizeof(temp), fp))
    {
        size_t chunk = strlen(temp);
        char *new_buffer = realloc(buffer, len + chunk + 1);
        if (!new_buffer)
        {
            free(buffer);
            pclose(fp);
            return NULL;
        }
        buffer = new_buffer;
        memcpy(buffer + len, temp, chunk);
        len += chunk;
        buffer[len] = '\0';
    }
    pclose(fp);
    return buffer;
}

Clay_String clippedClayString(const char *str)
{
    size_t max_len = 100;
    size_t str_len = strlen(str);
    size_t copy_len = str_len > max_len ? max_len : str_len;
    char *clipped = malloc(copy_len + 1);
    if (!clipped)
    {
        return (Clay_String){.isStaticallyAllocated = false, .length = 0, .chars = NULL};
    }
    strncpy(clipped, str, copy_len);
    clipped[copy_len] = '\0';

    return (Clay_String){
        .isStaticallyAllocated = false,
        .length = copy_len,
        .chars = clipped};
}

void HandleClayErrors(Clay_ErrorData errorData)
{
    printf("%s", errorData.errorText.chars);
}

typedef struct
{
    Clay_String title;
    Clay_String content;
} Item;

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
} ItemsArena;

typedef struct
{
    int32_t selectedItemIndex;
    float yOffset;
    ItemsArena itemsArena;
    Items *items;
} Item_Data;

void ButtonComponent(Clay_String buttonText)
{
    CLAY_AUTO_ID({.layout = {
                      .padding = CLAY_PADDING_ALL(8)},
                  .backgroundColor = COLOR_RED})
    {
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                .fontSize = 24,
                                                .textColor = COLOR_WHITE}));
    }
}

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

Clay_RenderCommandArray createMainLayout(Item_Data *data, bool mouseOnText, const char *searchText, int cursorBlinkCounter)
{
    Clay_BeginLayout();

    CLAY(CLAY_ID("mainContainer"), {.layout = {
                                        .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                                        .padding = CLAY_PADDING_ALL(40),
                                        .childGap = 32,
                                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                    }})
    {
        // Search Box
        CLAY(CLAY_ID("searchBox"), {.layout = {
                                        .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(80)},
                                        .padding = CLAY_PADDING_ALL(16),
                                        .childGap = 8,
                                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                    }})
        {
            CLAY_TEXT(CLAY_STRING("Search Items:"),
                      CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = 18, .textColor = COLOR_BLACK}));

            // Create search input display
            char displayText[110];
            strncpy(displayText, searchText, 100);
            displayText[100] = '\0';

            // Add cursor blink effect when focused
            if (mouseOnText && (cursorBlinkCounter / 20) % 2 == 0)
            {
                size_t len = strlen(displayText);
                if (len < 100)
                {
                    displayText[len] = '_';
                    displayText[len + 1] = '\0';
                }
            }
            static char *dynamicText = NULL;
            static size_t dynamicTextSize = 0;

            CLAY(CLAY_ID("searchInput"), {.layout = {
                                              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(40)},
                                              .padding = CLAY_PADDING_ALL(12),
                                          },
                                          .backgroundColor = mouseOnText ? COLOR_BLUE : COLOR_WHITE})
            {

                /// need to use a static buffer here to ensure the Clay_String chars pointer remains valid every frame
                static char persistentBuffer[110] = {0};

                // Copy your search text
                strncpy(persistentBuffer, searchText, 100);
                persistentBuffer[100] = '\0';

                // Add cursor if needed
                if (mouseOnText && (cursorBlinkCounter / 20) % 2 == 0)
                {
                    size_t len = strlen(persistentBuffer);
                    if (len < 100)
                    {
                        persistentBuffer[len] = '_';
                        persistentBuffer[len + 1] = '\0';
                    }
                }

                Clay_String searchString = {
                    .chars = persistentBuffer, // Use the static buffer
                    .length = (int32_t)strlen(persistentBuffer),
                    .isStaticallyAllocated = true // It's static, so mark as statically allocated
                };

                CLAY_TEXT(searchString, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                          .fontSize = 20,
                                                          .textColor = COLOR_BLACK}));
            }
        }

        // Item List with filtering
        CLAY(CLAY_ID("idItemContainer"), {.layout = {
                                              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                                              .padding = CLAY_PADDING_ALL(16),
                                              .childGap = 16,
                                              .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                          },
                                          .backgroundColor = {200, 200, 100, 255}})
        {
            char itemCountText[50];
            int visibleItems = 0;

            // Count visible items first
            for (int i = 0; i < data->items->itemCount; i++)
            {
                if (itemMatchesFilter(&data->items->items[i], searchText))
                {
                    visibleItems++;
                }
            }

            sprintf(itemCountText, "Items (%d/%d)", visibleItems, data->items->itemCount);
            Clay_String titleString = {
                .chars = itemCountText,
                .length = strlen(itemCountText),
                .isStaticallyAllocated = true};
            CLAY_TEXT(titleString, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                     .fontSize = 24,
                                                     .textColor = COLOR_BLACK}));

            // Display filtered items
            for (int i = 0; i < data->items->itemCount; i++)
            {
                Item item = data->items->items[i];
                if (itemMatchesFilter(&item, searchText))
                {
                    ButtonComponent(item.title);
                }
            }

            // Show message if no items match
            if (visibleItems == 0 && strlen(searchText) > 0)
            {
                CLAY_TEXT(CLAY_STRING("No items match your search"),
                          CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                            .fontSize = 18,
                                            .textColor = {150, 0, 0, 255}}));
            }
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    return renderCommands;
}

int main(void)
{
    Clay_Raylib_Initialize(screenWidth, screenHeight, "CopyPad2", FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

    uint64_t clayMemorySize = Clay_MinMemorySize();
    Clay_Arena clayArena = {
        .memory = malloc(clayMemorySize),
        .capacity = clayMemorySize,
    };

    Clay_Dimensions initialDimensions = {
        .width = screenWidth,
        .height = screenHeight,
    };
    Clay_Initialize(clayArena, initialDimensions, (Clay_ErrorHandler){HandleClayErrors});

    Font fonts[1];
    // fonts[FONT_ID_BODY_16] = LoadFontEx("resources/Roboto-Regular.ttf", 48, 0, 400);
    fonts[FONT_ID_BODY_16] = LoadFontEx("resources/Roboto-Regular.ttf", 24, 0, 400);
    // fonts[FONT_ID_BODY_16] = GetFontDefault();
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    Item_Data itemData = InitItems();
    int cursorBlinkCounter = 0;
    int clipboardPollCounter = 0;
    char *last_clipboard_content = NULL;

    char searchText[101] = "\0"; // Search filter text
    int letterCount = 0;
    bool mouseOnText = false;

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    static bool wasMinimized = false;

    while (!WindowShouldClose())
    {
        letterCount = strlen(searchText);

        // Update window dimensions for Clay
        Clay_SetLayoutDimensions((Clay_Dimensions){GetScreenWidth(), GetScreenHeight()});

        // Check if mouse is over the search area (approximate)
        Vector2 mousePos = GetMousePosition();
        Rectangle searchArea = {40, 40, GetScreenWidth() - 80, 80};
        mouseOnText = CheckCollisionPointRec(mousePos, searchArea);

        // Handle text input when focused
        if (mouseOnText)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 32) && (key <= 125) && (letterCount < 100))
                {
                    searchText[letterCount] = (char)key;
                    searchText[letterCount + 1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
                printf("Key pressed: %lc\n", key);
            }

            if (IsKeyPressed(KEY_BACKSPACE) && letterCount > 0)
            {
                if (letterCount > 0)
                {
                    letterCount--;
                    searchText[letterCount] = '\0';
                }
            }
        }

        // Poll clipboard periodically
        clipboardPollCounter++;
        cursorBlinkCounter++;
        if (clipboardPollCounter % 30 == 0)
        {
            char *current_clipboard_content = poll_clipboard();
            if (current_clipboard_content)
            {
                if (!last_clipboard_content || strcmp(current_clipboard_content, last_clipboard_content) != 0)
                {
                    // Clipboard content has changed
                    Item *new_items = malloc(sizeof(Item) * (itemArray.itemCount + 1));

                    // Copy old items
                    for (uint32_t i = 0; i < itemArray.itemCount; i++)
                    {
                        new_items[i] = itemArray.items[i];
                    }

                    new_items[itemArray.itemCount] = (Item){
                        .title = clippedClayString(current_clipboard_content),
                        .content = (Clay_String){
                            .isStaticallyAllocated = false,
                            .length = strlen(current_clipboard_content),
                            .chars = current_clipboard_content}};

                    if (itemArray.items)
                        free(itemArray.items);
                    itemArray.items = new_items;
                    itemArray.itemCount += 1;

                    if (last_clipboard_content)
                        free(last_clipboard_content);
                    last_clipboard_content = current_clipboard_content;
                }
                else
                {
                    free(current_clipboard_content);
                }
            }
            clipboardPollCounter = 0;
        }

        // Render
        BeginDrawing();
        ClearBackground(COLOR_ORANGE);

        Clay_Raylib_Render(createMainLayout(&itemData, mouseOnText, searchText, cursorBlinkCounter), fonts);
        EndDrawing();
    }

    // Cleanup
    if (last_clipboard_content)
        free(last_clipboard_content);
    if (itemArray.items)
    {
        for (uint32_t i = 0; i < itemArray.itemCount; i++)
        {
            if (!itemArray.items[i].title.isStaticallyAllocated)
            {
                free((void *)itemArray.items[i].title.chars);
            }
            if (!itemArray.items[i].content.isStaticallyAllocated)
            {
                free((void *)itemArray.items[i].content.chars);
            }
        }
        free(itemArray.items);
    }
    free(clayArena.memory);
    free(itemData.itemsArena.memory);

    CloseWindow();
    return 0;
}