/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMFILEPREVIEWPLUGIN_H
#define DFMFILEPREVIEWPLUGIN_H

#include <QObject>

#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE
#define DFMFilePreviewFactoryInterface_iid "com.deepin.filemanager.DFMFilePreviewFactoryInterface_iid"

class DFMFilePreview;
class DFMFilePreviewPlugin : public QObject
{
    Q_OBJECT

public:
    explicit DFMFilePreviewPlugin(QObject *parent = 0);

    virtual DFMFilePreview *create(const QString &key) = 0;
};

DFM_END_NAMESPACE

#endif // DFMFILEPREVIEWPLUGIN_H
