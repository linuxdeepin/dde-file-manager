/*
 * Copyright (C) 2017 Uniontech Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#pragma once
#include <unistd.h>
#include <stdbool.h>

bool
fs_str_is_empty (const char *str);

bool
fs_str_has_upper (const char *str);

char *
fs_str_copy (char *dest,
             char *end,
             const char *src);
