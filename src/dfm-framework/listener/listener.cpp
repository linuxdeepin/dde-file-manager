// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/listener/listener.h>

DPF_BEGIN_NAMESPACE

Listener::Listener(QObject *parent)
    : QObject(parent)
{
}

Listener *Listener::instance()
{
    static dpf::Listener listener;
    return &listener;
}

DPF_END_NAMESPACE
