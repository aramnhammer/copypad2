#ifndef MACOS_HOTKEY_H
#define MACOS_HOTKEY_H

#ifdef __APPLE__
void RegisterGlobalHotkey(void);
void UnregisterGlobalHotkey(void);
bool IsAppHidden(void);
void SetWindowHidden(void);
#endif
#endif