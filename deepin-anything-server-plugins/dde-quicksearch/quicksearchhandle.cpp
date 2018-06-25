/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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

#include "quicksearchhandle.h"

#include "quick_search/dquicksearch.h"

//filter

void QuickSearchHandle::onFileCreate(const QByteArrayList &files)
{
    if (!files.isEmpty()) {
        DQuickSearch::instance()->filesWereCreated(files);
    }
}



void QuickSearchHandle::onFileDelete(const QByteArrayList &files)
{
    if (!files.isEmpty()) {
        DQuickSearch::instance()->filesWereDeleted(files);
    }
}


void QuickSearchHandle::onFileRename(const QList<QPair<QByteArray, QByteArray>> &files)
{
    if (!files.isEmpty()) {
        DQuickSearch::instance()->filesWereRenamed(files);
    }
}
