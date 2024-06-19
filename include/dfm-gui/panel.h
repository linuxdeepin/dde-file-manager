// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANEL_H
#define PANEL_H

#include <dfm-gui/containment.h>

#include <QObject>

class QQuickWindow;

DFMGUI_BEGIN_NAMESPACE

// TODO:此部分是否和 Applet 分离，而非继承关系
class PanelPrivate;
class Panel : public Containment
{
    Q_OBJECT

public:
    explicit Panel(QObject *parent = nullptr);

    QQuickWindow *window() const;
    quint64 windId() const;

    virtual void loadState();
    virtual void saveState();

Q_SIGNALS:
    void aboutToOpen();
    void aboutToClose();

private:
    Q_DECLARE_PRIVATE_D(dptr, Panel);
    Q_DISABLE_COPY(Panel);
};

DFMGUI_END_NAMESPACE

#endif   // PANEL_H
