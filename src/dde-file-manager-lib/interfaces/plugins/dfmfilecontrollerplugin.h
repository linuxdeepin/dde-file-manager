// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMFILECONTROLLERPLUGIN_H
#define DFMFILECONTROLLERPLUGIN_H

#include <QObject>

#include "dfmglobal.h"

class DAbstractFileController;

DFM_BEGIN_NAMESPACE
#define DFMFileControllerFactoryInterface_iid "com.deepin.filemanager.DFMFileControllerFactoryInterface_iid"

class DFMFileControllerPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DFMFileControllerPlugin(QObject *parent = 0);

    virtual DAbstractFileController *create(const QString &key) = 0;
};

DFM_END_NAMESPACE

#endif // DFMFILECONTROLLERPLUGIN_H
