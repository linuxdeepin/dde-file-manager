// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SORT_H
#define SORT_H

#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
typedef struct
{
  char *name;
  ino_t ino;
} direntry_t;

char *savedir (char const *dir);
int direntry_cmp_name (void const *a, void const *b);
int direntry_cmp_inode (void const *a, void const *b);


#endif // SORT_H
