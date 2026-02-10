// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONHOLDER_H
#define COLLECTIONHOLDER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <DFrame>

#include <QObject>
#include <QSharedPointer>
#include <QItemSelectionModel>
#include <QPropertyAnimation>

namespace ddplugin_organizer {

class CollectionModel;
class CollectionFrame;
class CollectionWidget;
class CollectionDataProvider;
class CollectionView;
class Surface;
class CollectionHolderPrivate;
class CollectionHolder : public QObject
{
    Q_OBJECT
    friend class CollectionHolderPrivate;

public:
    explicit CollectionHolder(const QString &uuid, CollectionDataProvider *dataProvider, QObject *parent = nullptr);
    ~CollectionHolder() override;
    QString id() const;
    QString name();
    void setName(const QString &);
    Dtk::Widget::DFrame *frame() const;
    CollectionWidget *widget() const;
    CollectionView *itemView() const;
    void createFrame(Surface *surface, CollectionModel *model);
    void setSurface(Surface *surface);
    Surface *surface() const;
    void show();
    void openEditor(const QUrl &url);
    void selectUrl(const QUrl &url, const QItemSelectionModel::SelectionFlag &flags);

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
    void setFileShiftable(bool enable);
    bool fileShiftable() const;

    QPropertyAnimation *createAnimation();
    void selectFiles(const QList<QUrl> &urls);
    void setFreeze(bool freeze);

    void setStyle(const CollectionStyle &style);
    CollectionStyle style() const;
signals:
    void styleChanged(const QString &id);
    void sigRequestClose(const QString &id);
    void frameSurfaceChanged(QWidget *surface);

private:
    QSharedPointer<CollectionHolderPrivate> d = nullptr;
};

typedef QSharedPointer<CollectionHolder> CollectionHolderPointer;

}

#endif   // COLLECTIONHOLDER_H
