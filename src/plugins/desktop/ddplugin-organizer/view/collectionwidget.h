// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONWIDGET_H
#define COLLECTIONWIDGET_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include "DBlurEffectWidget"

#include <QAbstractItemModel>
#include <QItemSelectionModel>

namespace ddplugin_organizer {

class CollectionTitleBar;
class CollectionView;
class CollectionDataProvider;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class CanvasManagerShell;
class CollectionWidgetPrivate;

class CollectionWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
    friend class CollectionWidgetPrivate;

public:
    explicit CollectionWidget(const QString &uuid, CollectionDataProvider *dataProvider, QWidget *parent = nullptr);
    ~CollectionWidget() override;

    void setTitleName(const QString &name);
    QString titleName() const;
    void setRenamable(const bool renamable = false);
    bool renamable() const;
    void setClosable(const bool closable = false);
    bool closable() const;
    void setAdjustable(const bool adjustable = false);
    bool adjustable() const;
    void setCollectionSize(const CollectionFrameSize &size);
    CollectionFrameSize collectionSize() const;

    CollectionView *view() const;

    void setFreeze(bool freeze);

public Q_SLOTS:
    void cacheSnapshot();

signals:
    void sigRequestClose(const QString &id);
    void sigRequestAdjustSizeMode(const CollectionFrameSize &size);

protected slots:
    void updateMaskColor();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *) override;
#else
    void enterEvent(QEnterEvent *) override;
#endif
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QSharedPointer<CollectionWidgetPrivate> d = nullptr;
};

}

#endif   // COLLECTIONWIDGET_H
