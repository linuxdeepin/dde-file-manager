// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testingeventrecevier.h"
#include "testing/accessible/acframefunctions.h"

#include <dfm-framework/event/event.h>

#include <QAccessible>

DPUTILS_USE_NAMESPACE

TestingEventRecevier *TestingEventRecevier::instance()
{
    static TestingEventRecevier ins;
    return &ins;
}

void TestingEventRecevier::initializeConnections() const
{
    initAccessible();
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPUTILS_NAMESPACE), "slot_Accessible_SetAccessibleName",
                            TestingEventRecevier::instance(),
                            &TestingEventRecevier::handleAccessibleSetAccessibleName);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPUTILS_NAMESPACE), "slot_Accessible_SetObjectName",
                            TestingEventRecevier::instance(),
                            &TestingEventRecevier::handleAccessibleSetObjectName);
}

TestingEventRecevier::TestingEventRecevier(QObject *parent)
    : QObject(parent)
{
}

void TestingEventRecevier::initAccessible() const
{
    QAccessible::installFactory(accessibleFactory);
    QAccessible::setActive(true);
}

void TestingEventRecevier::handleAccessibleSetAccessibleName(QWidget *w, const QString &name)
{
    if (w)
        w->setAccessibleName(name);
}

void TestingEventRecevier::handleAccessibleSetObjectName(QWidget *w, const QString &name)
{
    Q_UNUSED(w)
    Q_UNUSED(name)
}
