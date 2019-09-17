/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
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
#include "vaultcontroller.h"
#include "models/vaultfileinfo.h"
#include "dfileservices.h"

#include "dfmevent.h"

#include <QStandardPaths>

class VaultDirIterator : public DDirIterator
{
public:
    VaultDirIterator(const DUrl &url,
                     const QStringList &nameFilters,
                     QDir::Filters filter,
                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags);

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    QDirIterator *iterator;
};

VaultDirIterator::VaultDirIterator(const DUrl &url, const QStringList &nameFilters,
                                   QDir::Filters filter, QDirIterator::IteratorFlags flags)
    : DDirIterator()
{
    iterator = new QDirIterator(VaultController::vaultToLocal(url), nameFilters, filter, flags);
}

DUrl VaultDirIterator::next()
{
    return VaultController::localToVault(iterator->next());
}

bool VaultDirIterator::hasNext() const
{
    return iterator->hasNext();
}

QString VaultDirIterator::fileName() const
{
    return iterator->fileName();
}

DUrl VaultDirIterator::fileUrl() const
{
    return VaultController::localToVault(iterator->filePath());
}

const DAbstractFileInfoPointer VaultDirIterator::fileInfo() const
{
    return DFileService::instance()->createFileInfo(nullptr, fileUrl());
}

DUrl VaultDirIterator::url() const
{
    return fileUrl();
}

VaultController::VaultController(QObject *parent)
    : DAbstractFileController(parent)
{

}

const DAbstractFileInfoPointer VaultController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new VaultFileInfo(event->url()));
}

const DDirIteratorPointer VaultController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    return DDirIteratorPointer(new VaultDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
}

DUrl VaultController::localToVault(QString localPath)
{
    QString nextPath = localPath;
    int index = nextPath.indexOf("vault_unlocked");
    Q_ASSERT(index != -1);
    index += QString("vault_unlocked").length();
    DUrl newUrl;
    newUrl.setScheme(DFMVAULT_SCHEME);
    newUrl.setHost("files");
    newUrl.setPath(nextPath.mid(index));
    return newUrl;
}

QString VaultController::vaultToLocal(DUrl vaultUrl)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QDir::separator() + "vault_unlocked" + vaultUrl.path();
}
