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
#ifndef COLLECTIONFRAME_H
#define COLLECTIONFRAME_H

#include "ddplugin_organizer_global.h"

#include <DFrame>

DDP_ORGANIZER_BEGIN_NAMESPACE

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

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONFRAME_H
