/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#include "dblockdevice.h"
#include "ddiskdevice.h"

#include <QStorageInfo>

#include "views/computerview.h"
#include "shutil/fileutils.h"
#include "computermodel.h"

ComputerModel::ComputerModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_diskm(new DDiskManager(this))
{
    m_diskm->setWatchChanges(true);
    par = qobject_cast<ComputerView2*>(parent);
    addItem(DUrl("splitter://#lorem ipsum"));
    for (auto& dev : m_diskm->blockDevices()) {
        QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(dev));
        if (blkdev->hasFileSystem()) {
            addItem(DUrl::fromDeviceId(dev));
        }
    }
}

ComputerModel::~ComputerModel()
{
}

QModelIndex ComputerModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, (void*)&m_items[row]);
}

QModelIndex ComputerModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int ComputerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

int ComputerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ComputerModel::data(const QModelIndex &index, int role) const
{
    if (static_cast<ComputerModelItemData*>(index.internalPointer())->cat == ComputerModelItemData::Category::cat_widget) {
        par->view()->setIndexWidget(index, static_cast<ComputerModelItemData*>(index.internalPointer())->widget);
    }

    if (role == Qt::DisplayRole) {
        if (index.data(DataRoles::ICategoryRole) == ComputerModelItemData::Category::cat_splitter) {
            return static_cast<ComputerModelItemData*>(index.internalPointer())->sptext;
        }
        if (index.row() >= m_items.size()) return "<I SHAN'T BE HERE>";
        QSharedPointer<DBlockDevice> blkdev(m_items.at(index.row()).blkdev);
        if (blkdev->mountPoints().size() == 1 && blkdev->mountPoints().front() == QString("/"))
            return QString("System disk");
        if (blkdev->idLabel().length() == 0)
            return QString("%1 Volume").arg(FileUtils::formatSize(blkdev->size()));
        return blkdev->idLabel();
    }

    if (role == Qt::DecorationRole) {
        return QIcon::fromTheme("drive-harddisk");
    }

    if (role == DataRoles::FileSystemRole) {
        return QString(QMetaEnum::fromType<DBlockDevice::FSType>().valueToKey(m_items.at(index.row()).blkdev->fsType())).toUpper();
    }

    if (role == DataRoles::SizeInUseRole) {
        QSharedPointer<DBlockDevice> blkdev(m_items.at(index.row()).blkdev);
        if (blkdev->mountPoints().empty()) {
            return quint64(blkdev->size() + 1);
        }
        QStorageInfo si(blkdev->mountPoints().front());
        return quint64(si.bytesTotal() - si.bytesFree());
    }

    if (role == DataRoles::SizeTotalRole) {
        QSharedPointer<DBlockDevice> blkdev(m_items.at(index.row()).blkdev);
        return quint64(blkdev->size());
    }

    if (role == DataRoles::UsgWidgetRole) {
        ProgressLine *pl = m_items.at(index.row()).pl;
        quint64 u = data(index, DataRoles::SizeInUseRole).toULongLong();
        quint64 t = data(index, DataRoles::SizeTotalRole).toULongLong();
        if (u > t) {
            u = 0;
        }
        pl->setValue(10000. * u / t);
        pl->update();
        return QVariant::fromValue(pl);
    }

    if (role == DataRoles::ICategoryRole) {
        return m_items.at(index.row()).cat;
    }

    return QVariant();
}

Qt::ItemFlags ComputerModel::flags(const QModelIndex &index) const
{
    if (index.data(DataRoles::ICategoryRole) != ComputerModelItemData::Category::cat_splitter) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }
    return Qt::ItemIsEnabled;
}

void ComputerModel::addItem(const DUrl &url, QWidget* w)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+1);
    ComputerModelItemData id;
    id.url = url;
    id.blkdev.reset(DDiskManager::createBlockDevice(url.path()));
    if (url.scheme() == "splitter") {
        id.cat = ComputerModelItemData::Category::cat_splitter;
        id.sptext = url.fragment();
    }
    if (url.scheme() == "widget") {
        id.cat = ComputerModelItemData::Category::cat_widget;
        id.widget = w;
    }
    ProgressLine *pl = id.pl = new ProgressLine(qobject_cast<ComputerView2*>(this->QObject::parent()));
    pl->setRoundRadius(2);
    pl->setMin(0);
    pl->setMax(10000);
    pl->setFixedHeight(6);
    pl->hide();
    m_items.append(id);
    endInsertRows();
}

void ComputerModel::insertAfter(const DUrl &url, const DUrl &ref, QWidget *w)
{
}

void ComputerModel::insertBefore(const DUrl &url, const DUrl &ref, QWidget *w)
{
}

void ComputerModel::removeItem(const DUrl &url)
{
}
