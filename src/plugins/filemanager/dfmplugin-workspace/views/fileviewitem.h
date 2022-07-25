/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#ifndef FILEVIEWITEM_H
#define FILEVIEWITEM_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/base/schemefactory.h"

#include <QStandardItem>
#include <QMetaType>

namespace dfmplugin_workspace {

class FileViewItemPrivate;
class FileViewItem : public QStandardItem
{
    friend class FileViewItemPrivate;

public:
    explicit FileViewItem();
    explicit FileViewItem(FileViewItem *parent);
    explicit FileViewItem(FileViewItem *parent, const QUrl &url);
    explicit FileViewItem(const FileViewItem &other);
    virtual ~FileViewItem() override;

    FileViewItem &operator=(const FileViewItem &other);

    void refresh();

    QUrl url() const;
    void setUrl(const QUrl url);

    AbstractFileInfoPointer fileInfo() const;

    virtual QVariant data(int role) const override;

public:
    FileViewItemPrivate *const d;
    FileViewItem *parent { nullptr };
};

}

Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::FileViewItem);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::FileViewItem *);
Q_DECLARE_METATYPE(QSharedPointer<DPWORKSPACE_NAMESPACE::FileViewItem>);

#endif   // DFMFILEVIEWITEM_H
