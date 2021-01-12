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

#include <QFrame>

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMCrumbBarPrivate;
class DFMCrumbBar : public QFrame
{
    Q_OBJECT
public:
    explicit DFMCrumbBar(QWidget *parent = nullptr);
    ~DFMCrumbBar();

    void showAddressBar(const QString &text);
    void showAddressBar(const DUrl &url);
    void hideAddressBar();
    void updateCrumbs(const DUrl &url);

    void playAddressBarAnimation();
    void stopAddressBarAnimation();

public Q_SLOTS:
    void updateCurrentUrl(const DUrl &url);

private:
    QScopedPointer<DFMCrumbBarPrivate> d_ptr;

Q_SIGNALS:
    void addressBarShown();
    void addressBarHidden();
    void addressBarContentEntered(QString content);
    void crumbListItemSelected(const DUrl &url);

protected:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    void onListViewContextMenu(const QPoint &point);

    Q_DECLARE_PRIVATE(DFMCrumbBar)
};

DFM_END_NAMESPACE

#endif // DFMCRUMBBAR_H
