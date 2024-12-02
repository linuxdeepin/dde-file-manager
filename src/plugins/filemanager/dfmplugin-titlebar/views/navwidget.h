// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NavWidget_H
#define NavWidget_H

#include "dfmplugin_titlebar_global.h"

#include <DButtonBox>
#include <QUrl>
#include <QWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

namespace dfmplugin_titlebar {
class HistoryStack;
class NavWidgetPrivate;
class NavWidget : public QWidget
{
    Q_OBJECT
    friend class NavWidgetPrivate;
    NavWidgetPrivate *const d;

public:
    explicit NavWidget(QWidget *parent = nullptr);
    void pushUrlToHistoryStack(const QUrl &url);
    void removeUrlFromHistoryStack(const QUrl &url);

    void addHistroyStack();
    void moveNavStacks(int from, int to);
    void removeNavStackAt(int index);
    void switchHistoryStack(const int index);

public Q_SLOT:
    void onUrlChanged(const QUrl &url);
    void onNewWindowOpended();
    void back();
    void forward();
    void changeSizeMode();
    void onDevUnmounted(const QString &id, const QString &oldMpt);

private:
    void initializeUi();
    void initConnect();
};
}

#endif   // NavWidget_H
