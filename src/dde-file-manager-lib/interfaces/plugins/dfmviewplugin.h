// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
