// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEWPLUGIN_H
#define FILEPREVIEWPLUGIN_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

namespace dfmbase {
#define FilePreviewFactoryInterface_iid "com.deepin.filemanager.FilePreviewFactoryInterface_iid"

class AbstractBasePreview;
class AbstractFilePreviewPlugin : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFilePreviewPlugin(QObject *parent = nullptr);

    virtual AbstractBasePreview *create(const QString &key) = 0;
};
}
#endif   // FILEPREVIEWPLUGIN_H
