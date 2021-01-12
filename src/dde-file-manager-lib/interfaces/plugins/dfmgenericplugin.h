/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMGENERICPLUGIN_H
#define DFMGENERICPLUGIN_H

#include <QObject>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

#define DFMGenericFactoryInterface_iid "com.deepin.filemanager.DFMGenericFactoryInterface_iid"

class DFMGenericPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DFMGenericPlugin(QObject *parent = 0);

    virtual QObject *create(const QString &key) = 0;
};

DFM_END_NAMESPACE

#endif // DFMGENERICPLUGIN_H
