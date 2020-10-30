/**
 * Copyright (C) 2017 Uniontech Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "string_utils.h"

bool
fs_str_is_empty (const char *str)
{
    // query is considered empty if:
    // - fist character is null terminator
    // - or it has only space characters
    while (*str != '\0') {
        if (!isspace (*str)) {
            return false;
        }
        str++;
    }
    return true;
}

bool
fs_str_has_upper (const char *strc)
{
    assert (strc != NULL);
    const char *ptr = strc;
    while (*ptr != '\0') {
        if (isupper (*ptr)) {
            return true;
        }
        ptr++;
    }
    return false;
}

char *
fs_str_copy (char *dest, char *end, const char *src)
{
    char *ptr = dest;
    while (ptr != end && *src != '\0') {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return ptr;
}

