// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRESSNOTIFIER_H
#define PROGRESSNOTIFIER_H

#include "service_textindex_global.h"

#include <QObject>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class ProgressNotifier : public QObject
{
    Q_OBJECT
public:
    static ProgressNotifier *instance();

Q_SIGNALS:
    void progressChanged(qint64 count, qint64 total);

private:
    explicit ProgressNotifier(QObject *parent = nullptr)
        : QObject(parent) { }
};

SERVICETEXTINDEX_END_NAMESPACE
#endif   // PROGRESSNOTIFIER_H
