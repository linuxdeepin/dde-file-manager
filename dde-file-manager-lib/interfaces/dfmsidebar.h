/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFMSIDEBAR_H
#define DFMSIDEBAR_H

#include <QScrollArea>

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMSideBarItem;
class DFMSideBarPrivate;
class DFMSideBar : public QScrollArea
{
    Q_OBJECT

public:

    enum GroupName {
        Common,
        Device,
        Bookmark,
        Network,
        Tag,
        Other,
        Unknow
    };

    explicit DFMSideBar(QWidget *parent = nullptr);
    ~DFMSideBar();

    bool contextMenuEnabled() const;
    int count() const;
    QStringList groupList() const;

    void setCurrentUrl(const DUrl &url);
    void setContextMenuEnabled(bool enable);
    void setDisableUrlSchemes(const QSet<QString> &schemes);
    void setGroupSaveItemOrder(bool saveItemOrder, const QString &group);
    QSet<QString> disableUrlSchemes() const;

    int addItem(DFMSideBarItem *item, const QString &group = QString());
    void insertItem(int index, DFMSideBarItem *item, const QString &group = QString());
    void removeItem(int index, const QString &group = QString());
    void removeItem(DFMSideBarItem *item);
    int itemIndex(const DFMSideBarItem *item) const;
    void appendListWithOrder(QList<DFMSideBarItem *> itemList, const QString &group) const;
    DFMSideBarItem *itemAt(int index, const QString &group = QString()) const;
    DFMSideBarItem *itemAt(const DUrl &url) const;
    DFMSideBarItem *takeItem(int index, const QString &group = QString());
    int itemCount(const QString &group = QString()) const;

    QRect groupGeometry(const QString &group = QString()) const;

    static QString groupName(GroupName group);
    static GroupName groupFromName(const QString &name);

Q_SIGNALS:
    void clicked(DFMSideBarItem *item);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QScopedPointer<DFMSideBarPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_ptr, void _q_onItemDragRelease(QPoint cursorPos, Qt::DropAction action, const DFMSideBarItem* item))

    Q_DECLARE_PRIVATE(DFMSideBar)
};

DFM_END_NAMESPACE

#endif // DFMSIDEBAR_H
