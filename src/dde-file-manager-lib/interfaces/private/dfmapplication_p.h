// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMAPPLICATION_P_H
#define DFMAPPLICATION_P_H

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMApplication;
class DFMApplicationPrivate
{
public:
    explicit DFMApplicationPrivate(dde_file_manager::DFMApplication *qq);

    void _q_onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value, bool edited = false);
    void _q_onSettingsValueEdited(const QString &group, const QString &key, const QVariant &value);

    static DFMApplication *self;
};

DFM_END_NAMESPACE

#endif // DFMAPPLICATION_P_H
