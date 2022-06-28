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
#include "organizer_defines.h"

#include "DBlurEffectWidget"

#include <QAbstractItemModel>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionTitleBar;
class CollectionView;
class CollectionDataProvider;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class CollectionWidgetPrivate;

class CollectionWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
    friend class CollectionWidgetPrivate;
public:
    explicit CollectionWidget(const QString &uuid, CollectionDataProvider *dataProvider, QWidget *parent = nullptr);
    ~CollectionWidget() override;

    void setCanvasModelShell(CanvasModelShell *sh);
    void setCanvasViewShell(CanvasViewShell *sh);
    void setCanvasGridShell(CanvasGridShell *sh);

    void setModel(QAbstractItemModel *model);
    void setDragEnabled(bool enable);
    bool dragEnabled() const;

    void setTitleName(const QString &name);
    QString titleName() const;
    void setRenamable(const bool renamable = false);
    bool renamable() const;
    void setClosable(const bool closable = false);
    bool closable() const;
    void setAdjustable(const bool adjustable = false);
    bool adjustable() const;

signals:
    void sigRequestClose(const QString &id);
    void sigRequestAdjustSize(const CollectionFrameSize &size);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QSharedPointer<CollectionWidgetPrivate> d = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONWIDGET_H
