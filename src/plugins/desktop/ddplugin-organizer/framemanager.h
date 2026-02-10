// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAMEMANAGER_H
#define FRAMEMANAGER_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <QObject>

namespace ddplugin_organizer {

class FrameManagerPrivate;
class FrameManager : public QObject
{
    Q_OBJECT
    friend class FrameManagerPrivate;

public:
    explicit FrameManager(QObject *parent = nullptr);
    ~FrameManager() override;
    bool initialize();
    void layout();
    void turnOn(bool build = true);
    void turnOff();

    bool organizerEnabled();

signals:

public slots:
    void onBuild();
    void onWindowShowed();
    void onDetachWindows();
    void onGeometryChanged();

protected:
    void switchMode(OrganizerMode mode);

private:
    FrameManagerPrivate *d = nullptr;
};

}

#endif   // FRAMEMANAGER_H
