#define CLAY_IMPLEMENTATION
#include "raylib.h"
#include "../../lib/clay/clay.h"
#include "../../lib/clay/renderers/raylib/clay_renderer_raylib.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/types.h"
#include "src/items.c"
#include "src/clipboard.c"
#include "src/errors.c"

#ifdef __APPLE__
#include "src/macos_hotkeys.h"
#endif

Color COLOR_LIGHT = {224, 215, 210, 255};
Color COLOR_GREY = {150, 145, 148, 1};
Color COLOR_ORANGE = {225, 138, 50, 255};
Clay_Color COLOR_WHITE = {255, 255, 255, 255};
Clay_Color COLOR_RED = {168, 66, 28, 255};
Clay_Color COLOR_BLACK = {0, 0, 0, 255};
Clay_Color COLOR_BLUE = {100, 150, 255, 255};

Color BACKGROUND_COLOR = {27, 2, 2, 1};
Color ITEMBOX_BACKGROUND_COLOR = {27, 2, 2, 1};

void HandleButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData)
{
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        const char *text = (const char *)userData;
        insert_into_clipboard(text);
    }
}

void ButtonComponent(Clay_String buttonText)
{
    CLAY_AUTO_ID({.layout = {

                      .sizing = {CLAY_SIZING_GROW(0)},
                      .padding = CLAY_PADDING_ALL(8)},
                  .backgroundColor = COLOR_RED,
                  .cornerRadius = CLAY_CORNER_RADIUS(4)})
    {
        Clay_OnHover(HandleButtonClick, (intptr_t)buttonText.chars);
        CLAY_TEXT(buttonText, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                .fontSize = 24,
                                                .textColor = COLOR_WHITE}));
    }
}

void HandleSettingsButtonClick(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData)
{
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME)
    {
        bool *settingsOpen = (bool *)userData;
        *settingsOpen = !(*settingsOpen);
    }
}

void SettingsButton(bool *settingsOpen)
{
    CLAY_AUTO_ID({.layout = {

                      .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                      .padding = CLAY_PADDING_ALL(8)},
                  .backgroundColor = COLOR_BLUE,
                  .cornerRadius = CLAY_CORNER_RADIUS(4)})
    {
        Clay_OnHover(HandleSettingsButtonClick, (intptr_t)settingsOpen);
        CLAY_TEXT(CLAY_STRING("Settings"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                             .fontSize = 16,
                                                             .textColor = COLOR_WHITE}));
    }
}

Clay_RenderCommandArray createMainLayout(Item_Data *data, bool mouseOnText, const char *searchText, int cursorBlinkCounter, bool *settingsOpen)
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
                    displayText[len] = '|';
                    displayText[len + 1] = '\0';
                }
            }
            static char *dynamicText = NULL;
            static size_t dynamicTextSize = 0;

            CLAY(CLAY_ID("searchInput"), {.layout = {
                                              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(40)},
                                              .padding = CLAY_PADDING_ALL(16),
                                          },
                                          .backgroundColor = mouseOnText ? COLOR_BLUE : COLOR_WHITE,
                                          .cornerRadius = CLAY_CORNER_RADIUS(4)
                                        }
                                        )
            {

                /// need to use a static buffer here to ensure the Clay_String chars pointer remains valid every frame
                static char persistentBuffer[110] = {0};
                /// Copy searchText into persistentBuffer because Clay_String needs a valid pointer every frame
                strncpy(persistentBuffer, searchText, 100);
                persistentBuffer[100] = '\0';

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

        CLAY(CLAY_ID("idItemContainer"), {.clip = {.vertical = true, .childOffset = Clay_GetScrollOffset()},
                                          .layout = {
                                              .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                                              .padding = CLAY_PADDING_ALL(16),
                                              .childGap = 16,
                                              .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                          },
                                          .backgroundColor = {200, 200, 100, 255},
                                          .cornerRadius = CLAY_CORNER_RADIUS(5)})

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
        CLAY(CLAY_ID("settingsButtonContainer"), {.layout = {
                                                      .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_FIXED(20)},
                                                      .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER}}})
        {

            SettingsButton(settingsOpen);
        }
    }

    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    return renderCommands;
}

void BackButton(bool *settingsOpen)
{
    CLAY_AUTO_ID({.layout = {

                      .sizing = {CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
                      .padding = CLAY_PADDING_ALL(8)},
                  .backgroundColor = COLOR_BLUE,
                  .cornerRadius = CLAY_CORNER_RADIUS(4)})
    {
        Clay_OnHover(HandleSettingsButtonClick, (intptr_t)settingsOpen);
        CLAY_TEXT(CLAY_STRING("Back"), CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
                                                         .fontSize = 16,
                                                         .textColor = COLOR_WHITE}));
    }
}

Clay_RenderCommandArray createSettingsPageLayout(bool *settingsOpen)
{
    Clay_BeginLayout();
    CLAY_AUTO_ID({.layout = {
                      .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)},
                      .padding = CLAY_PADDING_ALL(40),
                      .childGap = 32,
                      .layoutDirection = CLAY_TOP_TO_BOTTOM,
                  }})
    {
        CLAY_TEXT(CLAY_STRING("Settings Page"),
                  CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16, .fontSize = 24, .textColor = COLOR_BLACK}));
        BackButton(settingsOpen);
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

    char searchText[101] = "\0"; // Search filter text
    int letterCount = 0;
    bool mouseOnText = false;

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    static bool wasMinimized = false;
    static bool settingsOpen = false;

#ifdef __APPLE__
    RegisterGlobalHotkey();
#endif

    while (!WindowShouldClose())
    {
        letterCount = strlen(searchText);

        // Update window dimensions for Clay
        Clay_SetLayoutDimensions((Clay_Dimensions){GetScreenWidth(), GetScreenHeight()});

        // Check if mouse is over the search area (approximate)
        Vector2 mousePos = GetMousePosition();
        Rectangle searchArea = {40, 40, GetScreenWidth() - 80, 80};
        mouseOnText = CheckCollisionPointRec(mousePos, searchArea);
        Clay_Vector2 mousePosition = {mousePos.x, mousePos.y};
        bool isPointerDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        Clay_SetPointerState(mousePosition, isPointerDown);

        Clay_Vector2 scrollDelta = {0, 0};
        scrollDelta.y = GetMouseWheelMove();
        float deltaTime = GetFrameTime();

        Clay_UpdateScrollContainers(
            true,        // Enable drag scrolling
            scrollDelta, // Clay_Vector2 scrollwheel / trackpad scroll x and y delta this frame
            deltaTime    // Time since last frame in seconds as a float e.g. 8ms is 0.008f
        );

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

        clipboardPollCounter++;
        cursorBlinkCounter++;
        if (clipboardPollCounter % 30 == 0)
        {
            ClipboardData current_clipboard_data = poll_clipboard();
            bool isDuplicate = false;
            for (uint32_t i = 0; i < itemData.items->itemCount; i++)
            {
                if (itemData.items->items[i].hash == current_clipboard_data.hash)
                {
                    free(current_clipboard_data.buffer);
                    current_clipboard_data.buffer = NULL;
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate && current_clipboard_data.buffer)
            {
                Item *new_items = malloc(sizeof(Item) * (itemArray.itemCount + 1));
                for (uint32_t i = 0; i < itemArray.itemCount; i++)
                {
                    new_items[i] = itemArray.items[i];
                }
                new_items[itemArray.itemCount] = (Item){
                    .title = clippedClayString(current_clipboard_data.buffer),
                    .content = (Clay_String){
                        .isStaticallyAllocated = false,
                        .length = strlen(current_clipboard_data.buffer),
                        .chars = current_clipboard_data.buffer},
                    .hash = current_clipboard_data.hash};
                if (itemArray.items)
                    free(itemArray.items);
                itemArray.items = new_items;
                itemArray.itemCount += 1;
                itemData.items = &itemArray;
            }
            clipboardPollCounter = 0;
        }

        // Render
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        if (settingsOpen)
        {
            Clay_Raylib_Render(createSettingsPageLayout(&settingsOpen), fonts);
        }
        else
        {
            Clay_Raylib_Render(createMainLayout(&itemData, mouseOnText, searchText, cursorBlinkCounter, &settingsOpen), fonts);
        }
        EndDrawing();
    }

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
#ifdef __APPLE__
    UnregisterGlobalHotkey();
#endif
    free(clayArena.memory);
    free(itemData.itemsArena.memory);
    UnloadFont(fonts[FONT_ID_BODY_16]);

    CloseWindow();
    return 0;
}