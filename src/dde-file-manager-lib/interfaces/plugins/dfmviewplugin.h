/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMVIEWPLUGIN_H
#define DFMVIEWPLUGIN_H

#include <QObject>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE
#define DFMViewFactoryInterface_iid "com.deepin.filemanager.DFMViewFactoryInterface_iid"

class DFMBaseView;
class DFMViewPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DFMViewPlugin(QObject *parent = 0);
    ~DFMViewPlugin();

    virtual DFMBaseView *create(const QString &key) = 0;
};
DFM_END_NAMESPACE

#endif // DFMVIEWPLUGIN_H
