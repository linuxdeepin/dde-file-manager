// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CrumbBar_H
#define CrumbBar_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>

#include <QFrame>
#include <QUrl>

namespace dfmplugin_titlebar {

class CrumbInterface;
class CrumbBarPrivate;
class CrumbBar : public QFrame
{
    Q_OBJECT
    QScopedPointer<CrumbBarPrivate> d;

public:
    explicit CrumbBar(QWidget *parent = nullptr);
    virtual ~CrumbBar() override;

    CrumbInterface *controller() const;
    QUrl lastUrl() const;

Q_SIGNALS:
    void showAddressBarText(const QString &text);
    void hideAddressBar(bool cd);
    void selectedUrl(const QUrl &url);
    void editUrl(const QUrl &url);

public Q_SLOTS:
    void onCustomContextMenu(const QPoint &point);
    void onUrlChanged(const QUrl &url);
    void onKeepAddressBar(const QUrl &url);
    void onHideAddrAndUpdateCrumbs(const QUrl &url);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

}

#endif   // CrumbBar_H
