// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREHELPER_H
#define COREHELPER_H

#include "dfmplugin_core_global.h"

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
    void openNewWindow(const QUrl &url, const QVariant &opt = QVariant());
    void cacheDefaultWindow();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    explicit CoreHelper(QObject *parent = nullptr);
};

DPCORE_END_NAMESPACE

#endif   // COREHELPER_H
