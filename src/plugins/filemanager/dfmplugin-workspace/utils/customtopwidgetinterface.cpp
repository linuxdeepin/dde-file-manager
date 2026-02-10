// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customtopwidgetinterface.h"

#include <QWidget>

using namespace dfmplugin_workspace;
CustomTopWidgetInterface::CustomTopWidgetInterface(QObject *parent)
    : QObject(parent)
{
}

QWidget *CustomTopWidgetInterface::create(QWidget *parent)
{
    if (createTopWidgetFunc) {
        auto widget = createTopWidgetFunc();
        widget->setParent(parent);
        return widget;
    }
    return nullptr;
}

bool CustomTopWidgetInterface::isShowFromUrl(QWidget *w, const QUrl &url)
{
    return showTopWidgetFunc && showTopWidgetFunc(w, url);
}

void dfmplugin_workspace::CustomTopWidgetInterface::setKeepShow(bool keep)
{
    keepShow = keep;
}

bool CustomTopWidgetInterface::isKeepShow() const
{
    return keepShow;
}

void CustomTopWidgetInterface::setKeepTop(bool keep)
{
    keepTop = keep;
}

bool CustomTopWidgetInterface::isKeepTop() const
{
    return keepTop;
}

void CustomTopWidgetInterface::registeCreateTopWidgetCallback(const ShowTopWidgetCallback &func)
{
    showTopWidgetFunc = func;
}

void dfmplugin_workspace::CustomTopWidgetInterface::registeCreateTopWidgetCallback(const CreateTopWidgetCallback &func)
{
    createTopWidgetFunc = func;
}
