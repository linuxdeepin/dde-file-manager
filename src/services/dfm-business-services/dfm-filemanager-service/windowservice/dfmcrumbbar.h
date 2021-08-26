/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#ifndef DFMCRUMBBAR_H
#define DFMCRUMBBAR_H

#include "dfm-base/base/dfmurlroute.h"
#include "dfm-base/base/dfmstandardpaths.h"
#include "dfm-base/base/dfmschemefactory.h"

#include <QFrame>
#include <QUrl>

class DFMCrumbBarPrivate;
class DFMCrumbBar : public QFrame
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DFMCrumbBar)
    QScopedPointer<DFMCrumbBarPrivate> d_ptr;

public:
    explicit DFMCrumbBar(QWidget *parent = nullptr);
    virtual ~DFMCrumbBar() override;

    void setRootUrl(const QUrl &url);

Q_SIGNALS:
    void selectedUrl(const QUrl &url);

public Q_SLOTS:
    void onCustomContextMenu(const QPoint &point);

protected:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // DFMCRUMBBAR_H
