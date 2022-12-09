/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "localfileinfo.h"

#include <QUrl>
#include <QObject>

namespace dfmbase {

class DesktopFileInfoPrivate;
class DesktopFileInfo : public LocalFileInfo
{
public:
    explicit DesktopFileInfo(const QUrl &fileUrl);
    bool canTag() const;

public:
    virtual ~DesktopFileInfo() override;
    QString desktopName() const;
    QString desktopExec() const;
    QString desktopIconName() const;
    QString desktopType() const;
    QStringList desktopCategories() const;
    QIcon fileIcon() override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    void refresh() override;
    //QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;
    //QSet<MenuAction> disableMenuActionList() const override;
    virtual Qt::DropActions supportedOfAttributes(const SupportType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    static QMap<QString, QVariant> desktopFileInfo(const QUrl &fileUrl);

private:
    QSharedPointer<DesktopFileInfoPrivate> d = nullptr;
};
}

#endif   // DESKTOPFILEINFO_H
