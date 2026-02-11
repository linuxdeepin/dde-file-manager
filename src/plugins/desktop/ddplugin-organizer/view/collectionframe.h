// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONFRAME_H
#define COLLECTIONFRAME_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <DFrame>

namespace ddplugin_organizer {

// Use `tiny grid` as unit instead of pixel
static const int kMinCellWidth = 12;
static const int kMinCellHeight = 16;
static const QMap<CollectionFrameSize, QSize> kDefaultCollectionSize {
    { kSmall, { kMinCellWidth, kMinCellHeight } },
    { kMiddle, { 24, 16 } },
    { kLarge, { 24, 32 } },
    { kFree, { 0, 0 } }
};

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

public Q_SLOTS:
    void adjustSizeMode(const CollectionFrameSize &size);

signals:
    void sizeModeChanged(const CollectionFrameSize &size);
    void geometryChanged();
    void editingStatusChanged(bool editing);
    void requestChangeSurface(const QString &cursorScreenName, const QString &oldScreenName);
    void surfaceChanged(QWidget *surface);
    void requestDeactiveAllPredictors();
    void moveStateChanged(bool moving);

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
