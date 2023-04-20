// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_P_H
#define APPLICATION_P_H

#include <dfm-base/dfm_base_global.h>

#include <QString>
#include <QVariant>

namespace dfmbase {
class Application;
class ApplicationPrivate
{
public:
    explicit ApplicationPrivate(Application *qq);
    virtual ~ApplicationPrivate();
    void _q_onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value, bool edited = false);
    void _q_onSettingsValueEdited(const QString &group, const QString &key, const QVariant &value);

    static Application *self;
};
}
#endif   // APPLICATION_P_H
