// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPERATIONSETTINGS_H
#define OPERATIONSETTINGS_H

#include "dfmplugin_fileoperations_global.h"

#include <QWidget>

namespace dfmplugin_fileoperations {

class OperationSettings : public QObject
{
    Q_OBJECT
public:
    static QWidget *createSyncModeItem(QObject *opt);
};

}   // namespace dfmplugin_fileoperations

#endif   // OPERATIONSETTINGS_H
