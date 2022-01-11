/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DFM_GLOBAL_DEFINES_H
#define DFM_GLOBAL_DEFINES_H

#include "dfm_base_global.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE
namespace Global {
enum class ViewMode {
    kIconMode = 0x01,
    kListMode = 0x02,
    kExtendMode = 0x04,
    kAllViewMode = kIconMode | kListMode | kExtendMode
};
}   //namespace Global

DFMBASE_END_NAMESPACE

Q_DECLARE_METATYPE(DFMBASE_NAMESPACE::Global::ViewMode);

#endif   // DFM_GLOBAL_DEFINES_H
