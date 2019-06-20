/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#ifndef DFMSIDEBARITEMGROUP_H
#define DFMSIDEBARITEMGROUP_H

#include <QVBoxLayout>

#include "dfmglobal.h"

#include "dfmstandardpaths.h"
#include "dfmsidebar.h"
#include "dfmsidebaritemseparator.h"

class UDiskDeviceInfo;
typedef QExplicitlySharedDataPointer<UDiskDeviceInfo> UDiskDeviceInfoPointer;

DFM_BEGIN_NAMESPACE
class DFMSideBarItemGroup : public QVBoxLayout
{
    Q_OBJECT

public:
    explicit DFMSideBarItemGroup(QString groupName);
    int itemIndex(const DFMSideBarItem *item) const;
    int appendItem(DFMSideBarItem *item);
    void insertItem(int index, DFMSideBarItem *item);
    void removeItem(int index);
    void removeItem(DFMSideBarItem *item);
    void saveItemOrder();
    DFMSideBarItem *findItem(const DUrl &url);
    DFMSideBarItem *takeItem(int index);
    DFMSideBarItem *takeItem(DFMSideBarItem *item);
    DUrlList itemOrder();
    int itemCount() const;
    int visibleItemCount() const;
    void setSaveItemOrder(bool saveItemOrder);
    void setAutoSort(bool autoSort);
    void sort();
    void setDisableUrlSchemes(const QSet<QString> &schemes);
    DFMSideBarItem *operator [](int index);

private:
    QString groupName;
    QVBoxLayout *itemHolder;
    bool m_saveItemOrder = false;
    bool m_autosort = false;
    QList<DFMSideBarItem *> itemList;
    DFMSideBarItemSeparator *bottomSeparator;

private slots:
    void reorderItem(DFMSideBarItem *ori, DFMSideBarItem *dst, bool insertBefore);

    void itemConnectionRegister(DFMSideBarItem *item);
    void itemConnectionUnregister(DFMSideBarItem *item);

signals:
    void itemReordered(int oldIndex, int newIndex, DFMSideBarItem *item);
    void itemDragRelease(QPoint cursorPos, Qt::DropAction action, const DFMSideBarItem* item);
};

DFM_END_NAMESPACE

#endif // DFMSIDEBARITEMGROUP_H
