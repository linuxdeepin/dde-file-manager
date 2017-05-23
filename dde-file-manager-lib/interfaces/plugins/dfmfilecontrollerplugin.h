/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
