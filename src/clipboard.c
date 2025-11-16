#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

ClipboardData poll_clipboard();
Clay_String clippedClayString(const char *str);
void insert_into_clipboard(const char *str);

// FNV-1a hash function 
uint64_t hash_buffer(const char *str) {
    uint64_t hash = 14695981039346656037ULL;
    while (*str) {
        hash ^= (unsigned char)(*str++);
        hash *= 1099511628211ULL;
    }
    return hash;
}

ClipboardData poll_clipboard()
{
    FILE *fp = popen("pbpaste", "r");
    if (!fp)
        return (ClipboardData){NULL, 0};

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
            return (ClipboardData){NULL, 0};
        }
        buffer = new_buffer;
        memcpy(buffer + len, temp, chunk);
        len += chunk;
        buffer[len] = '\0';
    }
    pclose(fp);
    return (ClipboardData){buffer, hash_buffer(buffer)};
}

void insert_into_clipboard(const char *str)
{
    FILE *fp = popen("pbcopy", "w");
    if (!fp)
        return;

    fwrite(str, sizeof(char), strlen(str), fp);
    pclose(fp);
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