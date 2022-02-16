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

#include "dfmplugin_computer_global.h"

#include "dfm-base/file/entry/entities/abstractentryfileentity.h"

DFMBASE_BEGIN_NAMESPACE
class DesktopFile;
DFMBASE_END_NAMESPACE

DPCOMPUTER_BEGIN_NAMESPACE

namespace ExtraPropertyName {
static constexpr char kExecuteCommand[] { "execute_command" };
}   // namespace ExtraPropertyName

class AppEntryFileEntity : public dfmbase::AbstractEntryFileEntity
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
    virtual dfmbase::EntryFileInfo::EntryOrder order() const override;
    virtual QVariantHash extraProperties() const override;
    virtual bool isAccessable() const override;
    virtual QMenu *createMenu() override;

private:
    QString getFormattedExecCommand() const;

private:
    QSharedPointer<dfmbase::DesktopFile> desktopInfo;
    QUrl fileUrl;
};

DPCOMPUTER_END_NAMESPACE
#endif   // APPENTRYFILEENTITY_H
