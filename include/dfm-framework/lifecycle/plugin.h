// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGIN_H
#define PLUGIN_H

#include <dfm-framework/dfm_framework_global.h>

#include <QObject>
#include <QSharedData>

DPF_BEGIN_NAMESPACE

class Plugin : public QObject
{
    Q_OBJECT
public:
    virtual void initialize();

    /*!
     * \brief start
     * \return bool If it returns true, it means that the current plugin start function is executing properly,
     * false that a problem with current internal implementation
     */
    virtual bool start() = 0;

    virtual void stop();
};

DPF_END_NAMESPACE

#endif   // PLUGIN_H
