/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "bookmarkservice.h"
#include "private/bookmarkservice_p.h"
#include "dfm-base/utils/universalutils.h"

DSB_FM_BEGIN_NAMESPACE

namespace BookMark {

namespace EventType {
extern const int kBookMarkDisabled = DFMBASE_NAMESPACE::UniversalUtils::registerEventType();
}

}

DSB_FM_END_NAMESPACE

DSB_FM_USE_NAMESPACE
DPF_USE_NAMESPACE

BookMarkService::BookMarkService(QObject *parent)
    : PluginService(parent),
      AutoServiceRegister<BookMarkService>(),
      d(new BookMarkServicePrivate(this))
{
}

BookMarkService::~BookMarkService() {}

BookMarkServicePrivate::BookMarkServicePrivate(BookMarkService *q) : QObject(nullptr), service(q) {}
