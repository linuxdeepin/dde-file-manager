/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef FILETREATER_H
#define FILETREATER_H

#include "canvasmodel.h"
#include <QThread>
#include <QObject>
#include <QMutex>
#include <QUrl>

DSB_D_BEGIN_NAMESPACE

class FileTreater : public QObject
{
    Q_OBJECT
public:
    explicit FileTreater(CanvasModel *parent = nullptr);
    ~FileTreater() override;

    CanvasModel *model() const;
    void insertChild(const QUrl &url);
    void removeChild(const QUrl &url);
    void renameChild(const QUrl &oldUrl, const QUrl &newUrl);
    void updateChild(const QUrl &url);

    DFMLocalFileInfoPointer fileInfo(const QUrl &url);
    DFMLocalFileInfoPointer fileInfo(int index);
    int childrenCount() const;
    QList<QUrl> files() const;
    int indexOfChild(AbstractFileInfoPointer info);

    bool sort();
    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder order);
    int sortRole() const;
    void setSortRole(const dfmbase::AbstractFileInfo::SortKey role, const Qt::SortOrder order = Qt::AscendingOrder);

public slots:
    void onUpdateChildren(const QList<QUrl> &children);
    void onTraversalFinished();

private:
    bool doSort(QList<DFMLocalFileInfoPointer> &files) const;
    void specialSort(QList<DFMLocalFileInfoPointer> &files) const;
    void sortMainDesktopFile(QList<DFMLocalFileInfoPointer> &files, Qt::SortOrder order) const;

private:
    QList<QUrl> fileList;
    QMap<QUrl, DFMLocalFileInfoPointer> fileMap;
    QMutex childrenMutex;

    dfmbase::AbstractFileInfo::SortKey fileSortRole = dfmbase::AbstractFileInfo::kSortByFileName;
    Qt::SortOrder fileSortOrder = Qt::AscendingOrder;
};
DSB_D_END_NAMESPACE

#endif // FILETREATER_H
