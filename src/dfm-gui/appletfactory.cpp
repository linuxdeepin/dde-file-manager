// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-gui/appletfactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/finallyutil.h>

DFMGUI_BEGIN_NAMESPACE

/*!
 * \class AppletFactory
 * \brief 类似 SchemeFactory 提供拓展 Applet 类创建函数的注册管理
 */

AppletFactory::AppletFactory()
{
}

AppletFactory::~AppletFactory()
{
}

AppletFactory *AppletFactory::instance()
{
    static AppletFactory ins;
    return &ins;
}

bool AppletFactory::regCreator(const AppletFactory::Key &url, CreateFunc creator, QString *errorString)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() {
        if (errorString)
            *errorString = error;
    });

    if (constructList.contains(url)) {
        error = "The current url has registered "
                "the associated construction class";
        return false;
    }

    constructList.insert(url, creator);
    finally.dismiss();
    return true;
}

AppletFactory::AppletPtr AppletFactory::create(const AppletFactory::Key &url, Containment *parent, QString *errorString)
{
    QString error;
    dfmbase::FinallyUtil finally([&]() {
        if (errorString)
            *errorString = error;
    });

    CreateFunc constantFunc = constructList.value(url);
    if (!constantFunc) {
        error = "url should be call registered 'regClass()' function "
                "before create function";
        return nullptr;
    }

    AppletPtr info = constantFunc(url, parent, &error);
    return info;
}

DFMGUI_END_NAMESPACE
