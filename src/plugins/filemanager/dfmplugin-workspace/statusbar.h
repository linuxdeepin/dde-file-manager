/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "dfm-base/dfm_base_global.h"

#include <dpicturesequenceview.h>
#include <dslider.h>

#include <QFrame>

class QLabel;
class QSlider;
class QString;
class QLineEdit;
class QComboBox;
class QHBoxLayout;
class QPushButton;

DWIDGET_USE_NAMESPACE

class FileViewItem;
class StatusBarPrivate;
class StatusBar : public QFrame
{
    Q_OBJECT
public:
    explicit StatusBar(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void resetScalingSlider(const int stepCount);
    void setScalingVisible(const bool visible);
    int scalingValue();
    QSlider *scalingSlider() const;

public Q_SLOTS:
    void itemSelected(const QList<const FileViewItem *> &itemList);
    void itemCounted(const int count);
    void updateStatusMessage();

private:
    void initUI();
    void initScalingSlider();
    void initLoadingIndicator();
    void initLayout();
    void clearLayoutAndAnchors();

    // normal status components
    QHBoxLayout *layout = nullptr;
    QLabel *label = nullptr;
    DPictureSequenceView *loadingIndicator = nullptr;
    QSlider *scaleSlider = nullptr;

    QSharedPointer<StatusBarPrivate> d;
    Q_DECLARE_PRIVATE_D(d, StatusBar)
};

#endif   // STATUSBAR_H
