// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMUPERPROPERTYHELPER_H
#define COMUPERPROPERTYHELPER_H
#include "dfmplugin_propertydialog_global.h"

#include <QObject>
#include <QWidget>

namespace dfmplugin_propertydialog {
class ComputerPropertyHelper : public QObject
{
    Q_OBJECT
private:
    explicit ComputerPropertyHelper(QObject *parent = nullptr) = delete;

public:
    static QString scheme();

public:
    static QWidget *createComputerProperty(const QUrl &url);
};
}
#endif   //COMUPERPROPERTYHELPER_H
