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
#ifndef MASTEREDMEDIAFILEINFO_H
#define MASTEREDMEDIAFILEINFO_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

DPOPTICAL_BEGIN_NAMESPACE
class MasteredMediaFileInfoPrivate;
class MasteredMediaFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
    Q_GADGET
    friend class MasteredMediaFileInfoPrivate;

public:
    explicit MasteredMediaFileInfo(const QUrl &url);

    bool exists() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool isDir() const override;
    QString fileDisplayName() const override;
    QVariantHash extraProperties() const override;
    QVector<DFMBASE_NAMESPACE::ActionType> menuActionList(DFMBASE_NAMESPACE::AbstractMenu::MenuType type) const override;
    bool canRedirectionFileUrl() const override;
    QUrl redirectedFileUrl() const override;
    QUrl parentUrl() const override;
    bool canDrop() const override;
    bool canRename() const override;
    QSet<DFMBASE_NAMESPACE::ActionType> disableMenuActionList() const override;
    void refresh() override;
    bool canDragCompress() const override;
    virtual QString emptyDirectoryTip() const override;

private:
    void backupInfo(const QUrl &url);

private:
    QUrl backerUrl;
};

DPOPTICAL_END_NAMESPACE

#endif   // MASTEREDMEDIAFILEINFO_H
