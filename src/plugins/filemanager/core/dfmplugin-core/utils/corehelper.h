// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREHELPER_H
#define COREHELPER_H

#include "dfmplugin_core_global.h"

#include <dfm-gui/windowmanager.h>

#include <QObject>
#include <QVariant>

DPCORE_BEGIN_NAMESPACE

class CoreHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CoreHelper)

public:
    static CoreHelper &instance();

public:
    void cd(quint64 windowId, const QUrl &url);
    void openWindow(const QUrl &url, const QVariant &opt = QVariant());
    void cacheDefaultWindow();
    void loadPlugin(const QString &name);

private:
    DFMGUI_NAMESPACE::WindowHandle defaultWindow();
    DFMGUI_NAMESPACE::WindowHandle createNewWindow(const QUrl &url);
    DFMGUI_NAMESPACE::WindowHandle findExistsWindow(const QUrl &url);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit CoreHelper(QObject *parent = nullptr);
};

DPCORE_END_NAMESPACE

#endif   // COREHELPER_H
