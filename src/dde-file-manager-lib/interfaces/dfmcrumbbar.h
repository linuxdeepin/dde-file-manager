// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
