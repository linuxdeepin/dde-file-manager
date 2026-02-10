// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHHELPER_H
#define OPENWITHHELPER_H
#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_utils {
class OpenWithHelper : public QObject
{
    Q_OBJECT
public:
    explicit OpenWithHelper(QObject *parent = nullptr);

    static QWidget *createOpenWithWidget(const QUrl &url);
};
}
#endif   // OPENWITHHELPER_H
