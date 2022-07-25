/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef BASICSTATUSBAR_H
#define BASICSTATUSBAR_H

#include "dfm-base/dfm_base_global.h"

#include <QFrame>

namespace dfmbase {

class AbstractFileInfo;
class BasicStatusBarPrivate;
class BasicStatusBar : public QFrame
{
    Q_OBJECT
public:
    explicit BasicStatusBar(QWidget *parent = nullptr);

    virtual QSize sizeHint() const override;
    virtual void clearLayoutAndAnchors();

    void itemSelected(const QList<AbstractFileInfo *> &infoList);
    void itemCounted(const int count);

    void updateStatusMessage();

protected:
    void insertWidget(const int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

    void setTipText(const QString &tip);

private:
    void initUI();
    QSharedPointer<BasicStatusBarPrivate> d;
};

}

#endif   // BASICSTATUSBAR_H
