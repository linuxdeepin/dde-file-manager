/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#ifndef SEARCH_DEFINES_H
#define SEARCH_DEFINES_H

#include "dfm_filemanager_service_global.h"

#include <QObject>

DSB_FM_BEGIN_NAMESPACE

namespace Search {

struct CustomSearchInfo
{
    QString scheme;
    QString redirectedPath;
    bool isDisableSearch { false };
};

}

DSB_FM_END_NAMESPACE

Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::Search::CustomSearchInfo);

#endif   // SEARCH_DEFINES_H
