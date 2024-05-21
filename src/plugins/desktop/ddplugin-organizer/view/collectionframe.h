// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONFRAME_H
#define COLLECTIONFRAME_H

#include "ddplugin_organizer_global.h"

#include <DFrame>

namespace ddplugin_organizer {

class CollectionFramePrivate;

class CollectionFrame : public Dtk::Widget::DFrame
{
    Q_OBJECT
    friend class CollectionFramePrivate;

public:
    enum CollectionFrameFeature {
        NoCollectionFrameFeatures = 0x00,
        CollectionFrameClosable = 0x01,
        CollectionFrameMovable = 0x02,
        CollectionFrameFloatable = 0x04,
        CollectionFrameHiddable = 0x08,
        CollectionFrameAdjustable = 0x10,
        CollectionFrameStretchable = 0x20,

        CollectionGroupFeatureMask = 0x40
    };
    Q_DECLARE_FLAGS(CollectionFrameFeatures, CollectionFrameFeature)
    Q_FLAG(CollectionFrameFeatures)

    enum CollectionFrameStretchStyle {
        CollectionFrameStretchStep = 0,
        CollectionFrameStretchUnLimited,
    };

    explicit CollectionFrame(QWidget *parent = nullptr);
    ~CollectionFrame() Q_DECL_OVERRIDE;

    void setWidget(QWidget *w);
    QWidget *widget() const;

    void setCollectionFeatures(const CollectionFrameFeatures &features);
    CollectionFrameFeatures collectionFeatures() const;

    void setStretchStyle(const CollectionFrameStretchStyle &style);
    CollectionFrameStretchStyle stretchStyle() const;

    void setStretchStep(const int step);
    int stretchStep() const;

signals:
    void geometryChanged();
    void dragStarted();
    void dragStopped();

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void focusOutEvent(QFocusEvent *event) override;

private:
    void initUi();

private:
    QSharedPointer<CollectionFramePrivate> d = nullptr;
};

}

#endif   // COLLECTIONFRAME_H
