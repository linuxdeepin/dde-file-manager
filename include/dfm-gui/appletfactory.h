// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLETFACTORY_H
#define APPLETFACTORY_H

#include <dfm-gui/dfm_gui_global.h>
#include <dfm-base/utils/threadcontainer.h>

DFMGUI_BEGIN_NAMESPACE

class Applet;
class Containment;
class AppletFactory
{
    explicit AppletFactory();
    ~AppletFactory();

public:
    using AppletPtr = Applet *;
    using Key = QString;
    using CreateFunc = std::function<AppletPtr(const Key &url, Containment *parent, QString *error)>;

    static AppletFactory *instance();

    bool regCreator(const Key &url, CreateFunc creator, QString *errorString = nullptr);
    AppletPtr create(const Key &url, Containment *parent = nullptr, QString *errorString = nullptr);

private:
    dfmbase::DThreadMap<Key, CreateFunc> constructList {};
};

DFMGUI_END_NAMESPACE

#endif   // APPLETFACTORY_H
