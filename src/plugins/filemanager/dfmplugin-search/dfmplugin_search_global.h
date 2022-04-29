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
#ifndef DFMPLUGIN_SEARCH_GLOBAL_H
#define DFMPLUGIN_SEARCH_GLOBAL_H

#define DPSEARCH_BEGIN_NAMESPACE namespace dfmplugin_search {
#define DPSEARCH_END_NAMESPACE }
#define DPSEARCH_USE_NAMESPACE using namespace dfmplugin_search;
#define DPSEARCH_NAMESPACE dfmplugin_search

DPSEARCH_BEGIN_NAMESPACE
namespace SearchActionId {
static constexpr char kOpenFileLocation[] { "open-file-location" };
}
DPSEARCH_END_NAMESPACE

#endif   // DFMPLUGIN_SEARCH_GLOBAL_H
