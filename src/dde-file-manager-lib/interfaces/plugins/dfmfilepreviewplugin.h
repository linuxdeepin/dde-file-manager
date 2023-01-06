// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
