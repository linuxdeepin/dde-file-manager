// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREDQMLENGINE_H
#define SHAREDQMLENGINE_H

#include <dfm-gui/dfm_gui_global.h>

#include <QObject>
#include <QQmlComponent>

class QQmlEngine;
class QQmlContext;

DFMGUI_BEGIN_NAMESPACE

class Applet;
class SharedQmlEnginePrivate;
class SharedQmlEngine : public QObject
{
    Q_OBJECT

public:
    explicit SharedQmlEngine(QObject *parent = nullptr);
    virtual ~SharedQmlEngine() override;

    QObject *rootObject() const;
    QQmlContext *rootContext() const;
    QQmlComponent *mainComponent() const;

    bool create(Applet *applet, bool async = false);
    bool completeCreation(const QVariantMap &args = {});

    static QObject *createObject(const QUrl &url, const QVariantMap &args = {});

    QQmlComponent::Status status();
    Q_SIGNAL void statusChanged(QQmlComponent::Status status);
    Q_SIGNAL void createFinished(bool success);

private:
    QScopedPointer<SharedQmlEnginePrivate> dptr;
    Q_DECLARE_PRIVATE_D(dptr, SharedQmlEngine);
    Q_DISABLE_COPY(SharedQmlEngine);
};

DFMGUI_END_NAMESPACE

#endif   // SHAREDQMLENGINE_H
