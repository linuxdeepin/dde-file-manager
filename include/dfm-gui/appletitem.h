// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLETITEM_H
#define APPLETITEM_H

#include "applet.h"

#include <QQuickItem>

DFMGUI_BEGIN_NAMESPACE

class AppletItemPrivate;
class AppletItem : public QQuickItem
{
    Q_OBJECT

public:
    explicit AppletItem(QQuickItem *parent = nullptr);
    ~AppletItem() override;

    Applet *applet() const;
    void setApplet(Applet *applet);

    bool isCreateComplete() const;

    Q_INVOKABLE QQuickWindow *itemWindow() const;
    Q_SIGNAL void itemWindowChanged(QQuickWindow *window);

    static AppletItem *itemForApplet(Applet *applet);

private:
    QScopedPointer<AppletItemPrivate> dptr;
    Q_DECLARE_PRIVATE_D(dptr, AppletItem)
    Q_DISABLE_COPY(AppletItem)
};

DFMGUI_END_NAMESPACE

#endif   // APPLETITEM_H
