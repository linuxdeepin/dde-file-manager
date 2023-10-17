// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include <dfm-base/interfaces/proxyfileinfo.h>

#include <QUrl>
#include <QObject>

namespace dfmbase {

class DesktopFileInfoPrivate;
class DesktopFileInfo : public ProxyFileInfo
{
public:
    explicit DesktopFileInfo(const QUrl &fileUrl);
    explicit DesktopFileInfo(const QUrl &fileUrl, const FileInfoPointer &info);
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
    virtual void updateAttributes(const QList<FileInfoAttributeID> &types = {}) override;
    static QMap<QString, QVariant> desktopFileInfo(const QUrl &fileUrl);

private:
    QSharedPointer<DesktopFileInfoPrivate> d = nullptr;
};
}

#endif   // DESKTOPFILEINFO_H
