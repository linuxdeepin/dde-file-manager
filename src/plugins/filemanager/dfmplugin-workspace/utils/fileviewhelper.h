/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef FILEVIEWHELPER_H
#define FILEVIEWHELPER_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QStyleOptionViewItem;
QT_END_NAMESPACE

DPWORKSPACE_BEGIN_NAMESPACE
extern const char *const kEidtorShowSuffix;
class BaseItemDelegate;
class FileViewModel;
class FileView;

class FileViewHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileViewHelper(FileView *parent = nullptr);
    ~FileViewHelper();

    FileView *parent() const;
    virtual bool isTransparent(const QModelIndex &index) const;
    const AbstractFileInfoPointer fileInfo(const QModelIndex &index) const;
    QMargins fileViewViewportMargins() const;
    QWidget *indexWidget(const QModelIndex &index) const;
    int selectedIndexsCount() const;
    bool isSelected(const QModelIndex &index) const;
    bool isDropTarget(const QModelIndex &index) const;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    void updateGeometries();
    void keyboardSearch(const QString &search);
    bool isEmptyArea(const QPoint &pos);

    static int caculateListItemIndex(const QSize &itemSize, const QPoint &pos);
    static int caculateIconItemIndex(const FileView *view, const QSize &itemSize, const QPoint &pos);

public slots:
    void handleCommitData(QWidget *editor) const;
    void selectFiles(const QList<QUrl> &files);

private slots:
    void clipboardDataChanged();
    void clearSearchKey();

signals:
    void triggerEdit(const QModelIndex &index);

private:
    void init();
    BaseItemDelegate *itemDelegate() const;
    QModelIndex findIndex(const QByteArray &keys, bool matchStart,
                          int current, bool reverseOrder, bool excludeCurrent) const;

    QByteArray keyboardSearchKeys;
    QTimer *keyboardSearchTimer;
};

DPWORKSPACE_END_NAMESPACE

#endif   // FILEVIEWHELPER_H
