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
#ifndef COLLECTIONHOLDER_H
#define COLLECTIONHOLDER_H

#include "ddplugin_organizer_global.h"

#include <DFrame>

#include <QObject>
#include <QSharedPointer>

DDP_ORGANIZER_BEGIN_NAMESPACE

class FileProxyModel;
class CollectionFrame;
class CollectionWidget;
class CollectionDataProvider;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class Surface;
class CollectionHolderPrivate;
class CollectionHolder : public QObject
{
    Q_OBJECT
    friend class CollectionHolderPrivate;
public:
    explicit CollectionHolder(const QString &uuid, CollectionDataProvider *dataProvider, QObject *parent = nullptr);
    ~CollectionHolder() override;
    void setCanvasModelShell(CanvasModelShell *sh);
    void setCanvasViewShell(CanvasViewShell *sh);
    void setCanvasGridShell(CanvasGridShell *sh);
    QString id() const;
    QString name();
    void setName(const QString &);
    Dtk::Widget::DFrame *frame() const;
    void createFrame(Surface *surface, FileProxyModel *model);
    Surface *surface() const;
    void show();

    void setMovable(const bool movable = false);
    bool movable() const;
    void setClosable(const bool closable = false);
    bool closable() const;
    void setFloatable(const bool floatable = false);
    bool floatable() const;
    void setHiddableCollection(const bool hiddable = false);
    bool hiddableCollection() const;
    void setStretchable(const bool stretchable = false);
    bool stretchable() const;
    void setAdjustable(const bool adjustable = false);
    bool adjustable() const;
    void setHiddableTitleBar(const bool hiddable = false);
    bool hiddableTitleBar() const;
    void setHiddableView(const bool hiddable = false);
    bool hiddableView() const;
    void setRenamable(const bool renamable = false);
    bool renamable() const;
    void setDragEnabled(bool enable);
    bool dragEnabled() const;

signals:
    void sigRequestClose(const QString &id);

private:
    QSharedPointer<CollectionHolderPrivate> d = nullptr;
};

typedef QSharedPointer<CollectionHolder> CollectionHolderPointer;

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONHOLDER_H
