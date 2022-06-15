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
#ifndef COLLECTIONWIDGET_H
#define COLLECTIONWIDGET_H

#include "ddplugin_organizer_global.h"

#include "DBlurEffectWidget"

#include <QAbstractItemModel>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionTitleBar;
class CollectionView;
class CollectionWidgetPrivate;

class CollectionWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
    friend class CollectionWidgetPrivate;
public:
    explicit CollectionWidget(QWidget *parent = nullptr);
    ~CollectionWidget();

    void setModel(QAbstractItemModel *model);
    QList<QUrl> urls() const;
    void setUrls(const QList<QUrl> &urls);
    void setTitleName(const QString &name);
    QString titleName() const;
    void setRenamable(const bool renamable = true);
    bool renamable() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QSharedPointer<CollectionWidgetPrivate> d = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONWIDGET_H
