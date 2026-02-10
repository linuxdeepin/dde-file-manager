// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TESTINGEVENTRECEVIER_H
#define TESTINGEVENTRECEVIER_H

#include "dfmplugin_utils_global.h"

#include <QObject>

DPUTILS_BEGIN_NAMESPACE

class TestingEventRecevier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TestingEventRecevier)
public:
    static TestingEventRecevier *instance();
    void initializeConnections() const;

private:
    explicit TestingEventRecevier(QObject *parent = nullptr);
    void initAccessible() const;
    void handleAccessibleSetAccessibleName(QWidget *w, const QString &name);
    void handleAccessibleSetObjectName(QWidget *w, const QString &name);
};

DPUTILS_END_NAMESPACE

#endif // TESTINGEVENTRECEVIER_H
