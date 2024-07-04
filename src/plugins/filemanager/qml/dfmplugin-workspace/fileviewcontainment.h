// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWCONTAINMENT_H
#define FILEVIEWCONTAINMENT_H

#include <dfm-gui/containment.h>

#include <QObject>

DFMGUI_BEGIN_NAMESPACE
class AppletItem;
DFMGUI_END_NAMESPACE

namespace dfmplugin_workspace {

class FileViewContainment : public dfmgui::Containment
{
    Q_OBJECT
public:
    explicit FileViewContainment(QObject *parent = nullptr);

    Q_INVOKABLE quint64 getWinId();
};

}   // namespace dfmplugin_workspace

#endif   // FILEVIEWCONTAINMENT_H
