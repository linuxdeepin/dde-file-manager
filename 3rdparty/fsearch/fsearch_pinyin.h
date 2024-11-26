/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FSEARCH_PINYIN_H
#define FSEARCH_PINYIN_H

// first chinese word's unicode is 19968 in of basic map
#define PINYIN_UNICODE_START 0x4E00
// last chinese word's unicode is 40896 of basic map
#define PINYIN_UNICODE_END 0x9FA5
#define MAX_PINYIN_WORD 20902
#define MAX_PINYIN_LEN 6

#define DICT_MAX_LEN (MAX_PINYIN_WORD * MAX_PINYIN_LEN + 1)

void utf8_to_pinyin(const char *in, char *out);
void convert_all_pinyin(const char *in, char *first, char *full);
// cat first and full words with '|' and return, need be freed in its invoker
char *cat_pinyin(const char *in);
int is_text_utf8(const char *str, long length);

#endif   // FSEARCH_PINYIN_H
