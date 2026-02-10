// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHEVENTRECEIVER_H
#define OPENWITHEVENTRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <QObject>

namespace dfmplugin_utils {
class OpenWithEventReceiver : public QObject
{
    Q_OBJECT
public:
    explicit OpenWithEventReceiver(QObject *parent = nullptr);

    void initEventConnect();

public:   //! slot event
    void showOpenWithDialog(quint64 winId, const QList<QUrl> &urls);
};
}
#endif   // OPENWITHEVENTRECEIVER_H
