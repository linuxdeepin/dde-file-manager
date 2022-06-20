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

DPWORKSPACE_BEGIN_NAMESPACE

class FileViewItemPrivate;

/*!
 * \brief The IconLayer class 脚本图层变量
 */
class IconLayer
{
    QPointF apos;   // 相对与文件icon的坐标
    QIcon aicon;   // 添加到icon上的图标
public:
    explicit IconLayer() {}
    explicit IconLayer(const QPointF &pos, const QIcon &icon)
        : apos(pos), aicon(icon) {}
    void setPos(const QPointF &pos) { IconLayer::apos = pos; }
    void setIcon(const QIcon &icon) { IconLayer::aicon = icon; }
    QIcon icon() { return IconLayer::aicon; }
    QPointF pos() { return IconLayer::apos; }
};

typedef QList<IconLayer> IconLayers;   //多个图标叠加图层

class FileViewItem : public QStandardItem
{
    friend class FileViewItemPrivate;
    FileViewItemPrivate *const d;

public:
    enum CornerMark {
        kTopLeft,
        kTopRight,
        kBottomLeft,
        kBottomRight,
    };

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
    virtual void setCornerMark(const QIcon &tl, const QIcon &tr,
                               const QIcon &bl, const QIcon &br);
    virtual void setCornerMark(CornerMark flag, const QIcon &icon);
    virtual QIcon cornerMarkTL();
    virtual QIcon cornerMarkTR();
    virtual QIcon cornerMarkBL();
    virtual QIcon cornerMarkBR();
    virtual void setIconLayers(const IconLayers &layers);
    virtual IconLayers iconLayers();
    virtual QVariant data(int role) const override;

    inline static QString dateTimeFormat()
    {
        return "yyyy/MM/dd HH:mm:ss";
    }

public:
    FileViewItem *parent { nullptr };
};

DPWORKSPACE_END_NAMESPACE

Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::FileViewItem);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::FileViewItem *);
Q_DECLARE_METATYPE(QSharedPointer<DPWORKSPACE_NAMESPACE::FileViewItem>);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::IconLayer);
Q_DECLARE_METATYPE(DPWORKSPACE_NAMESPACE::IconLayers);

#endif   // DFMFILEVIEWITEM_H
