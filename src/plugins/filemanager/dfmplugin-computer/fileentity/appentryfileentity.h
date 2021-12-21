/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef APPENTRYFILEENTITY_H
#define APPENTRYFILEENTITY_H

#include "dfm-base/dfm_base_global.h"
#include "file/entry/entities/abstractentryfileentity.h"
#include "dfmplugin_computer_global.h"

DFMBASE_BEGIN_NAMESPACE
namespace SuffixInfo {
const char *const kAppEntry { "appentry" };
}   // namespace SuffixInfo
DFMBASE_END_NAMESPACE

DPCOMPUTER_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

class AppEntryFileEntity : public AbstractEntryFileEntity
{
public:
    explicit AppEntryFileEntity(const QUrl &url);

    // EntryFileEntity interface
    virtual QString displayName() const override;
    virtual QIcon icon() const override;
    virtual bool exists() const override;
    virtual bool showProgress() const override;
    virtual bool showTotalSize() const override;
    virtual bool showUsageSize() const override;
    virtual void onOpen() override;
    virtual QString description() const override;
    virtual EntryFileInfo::EntryOrder order() const override;
};

DPCOMPUTER_END_NAMESPACE
#endif   // APPENTRYFILEENTITY_H
