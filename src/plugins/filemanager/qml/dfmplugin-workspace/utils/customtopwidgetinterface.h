// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMTOPWIDGETINTERFACE_H
#define CUSTOMTOPWIDGETINTERFACE_H

#include "dfmplugin_workspace_global.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class CustomTopWidgetInterface : public QObject
{
    Q_OBJECT
public:
    explicit CustomTopWidgetInterface(QObject *parent = nullptr);

    QWidget *create(QWidget *parent = nullptr);
    bool isShowFromUrl(QWidget *w, const QUrl &url);
    void setKeepShow(bool keep);
    bool isKeepShow() const;
    void registeCreateTopWidgetCallback(const CreateTopWidgetCallback &func);
    void registeCreateTopWidgetCallback(const ShowTopWidgetCallback &func);

private:
    bool keepShow { false };
    CreateTopWidgetCallback createTopWidgetFunc;
    ShowTopWidgetCallback showTopWidgetFunc;
};

}
#endif   // CUSTOMTOPWIDGETINTERFACE_H
