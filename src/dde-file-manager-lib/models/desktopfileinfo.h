// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "dfileinfo.h"

#include <QObject>

#define DESKTOP_SURRIX "desktop"

class DesktopFileInfoPrivate;
class DesktopFileInfo : public DFileInfo
{
public:
    explicit DesktopFileInfo(const DUrl &fileUrl);
    explicit DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo() override;

    QString getName() const;
    QString getExec() const;
    QString getIconName() const;
    QString getType() const;
    QStringList getCategories() const;

    QIcon fileIcon() const override;
    QString fileDisplayName() const override;
    QString fileNameOfRename() const override;
    QString baseNameOfRename() const override;
    QString suffixOfRename() const override;

    void refresh(const bool isForce = false) override;

    QString iconName() const override;
    QString genericIconName() const override;

    static QMap<QString, QVariant> getDesktopFileInfo(const DUrl &fileUrl);

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;
    QSet<MenuAction> disableMenuActionList() const override;
    QList<QIcon> additionalIcon() const;
    Qt::DropActions supportedDragActions() const override;

    bool canDrop() const override;
    bool canTag() const override;
    bool canMoveOrCopy() const override;

    static DUrl trashDesktopFileUrl();
    static DUrl computerDesktopFileUrl();
    static DUrl homeDesktopFileUrl();

private:
    Q_DECLARE_PRIVATE(DesktopFileInfo)
};

#endif // DESKTOPFILEINFO_H
