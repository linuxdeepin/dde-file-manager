/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef FILEFILTER_H
#define FILEFILTER_H

#include "dfm_desktop_service_global.h"

#include <QAbstractItemModel>
#include <QObject>

class QGSettings;
DSB_D_BEGIN_NAMESPACE

class FileFilter
{
    Q_DISABLE_COPY(FileFilter)
public:
    explicit FileFilter(QAbstractItemModel *model);
    virtual bool fileTraversalFilter(const QUrl &url);
    virtual bool fileDeletedFilter(const QUrl &url);
    virtual bool fileCreatedFilter(const QUrl &url);
    virtual bool fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl);
    virtual bool fileUpdatedFilter(const QUrl &url);
protected:
    QAbstractItemModel *model;
};

class CustomHiddenFilter : public FileFilter
{
public:
    using FileFilter::FileFilter;
    bool fileUpdatedFilter(const QUrl &url) override;
};

class InnerDesktopAppController : public QObject, public FileFilter
{
    Q_OBJECT
public:
    explicit InnerDesktopAppController(QAbstractItemModel *model, QObject *parent = nullptr);
    void update();
    void refreshModel();
    bool fileTraversalFilter(const QUrl &url) override;
    bool fileCreatedFilter(const QUrl &url) override;
    bool fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl) override;
public slots:
    void changed(const QString &key);
protected:
    QGSettings *gsettings = nullptr;
    QMap<QString, QUrl> keys;
    QMap<QString, bool> hidden;
};

DSB_D_END_NAMESPACE

#endif // FILEFILTER_H
