// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASINTERFACE_H
#define CANVASINTERFACE_H

#include "ddplugin_organizer_global.h"

#include <QObject>

namespace ddplugin_organizer {

class CanvasInterfacePrivate;
class CanvasInterface : public QObject
{
    Q_OBJECT
    friend class CanvasInterfacePrivate;
public:
    explicit CanvasInterface(QObject *parent = nullptr);
    ~CanvasInterface();
    bool initialize();
    int iconLevel();
    void setIconLevel(int);
    class FileInfoModelShell *fileInfoModel();
    class CanvasModelShell *canvasModel();
    class CanvasViewShell *canvasView();
    class CanvasGridShell *canvasGrid();
    class CanvasManagerShell *canvasManager();
    class CanvasSelectionShell *canvasSelectionShell();
signals:

public slots:
protected:

private:
    CanvasInterfacePrivate *d;
};

}

#endif // CANVASINTERFACE_H
