// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSTERMODE_H
#define MUSTERMODE_H

#include "ddplugin_organizer_global.h"
#include "private/surface.h"
#include "organizer_defines.h"

#include <QObject>
#include <QMap>

class QMimeData;

namespace ddplugin_organizer {
class CollectionModel;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class CanvasManagerShell;
class CanvasSelectionShell;
class OrganizerCreator
{
public:
    static class CanvasOrganizer *createOrganizer(OrganizerMode mode);
};

class CanvasOrganizer : public QObject
{
    Q_OBJECT
public:
    explicit CanvasOrganizer(QObject *parent = nullptr);
    ~CanvasOrganizer() override;
    virtual OrganizerMode mode() const = 0;
    virtual bool initialize(CollectionModel *) = 0;
    inline CollectionModel *getModel() const { return model; }
    inline QList<SurfacePointer> getSurfaces() const { return surfaces; }
    virtual void layout();
    virtual void detachLayout();
    virtual void setCanvasModelShell(CanvasModelShell *sh);
    virtual void setCanvasViewShell(CanvasViewShell *sh);
    virtual void setCanvasGridShell(CanvasGridShell *sh);
    virtual void setCanvasManagerShell(CanvasManagerShell *sh);
    virtual void setCanvasSelectionShell(CanvasSelectionShell *sh);
    virtual void setSurfaces(const QList<SurfacePointer> &surfaces);
    virtual void reset();
    virtual bool isEditing() const;

signals:
    void collectionChanged();
    void hideAllKeyPressed() const;

protected slots:
    virtual bool filterDataRested(QList<QUrl> *urls);
    virtual bool filterDataInserted(const QUrl &url);
    virtual bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl);
    virtual bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData);
    virtual bool filterShortcutkeyPress(int viewIndex, int key, int modifiers) const;
    virtual bool filterKeyPress(int viewIndex, int key, int modifiers) const;
    virtual bool filterWheel(int viewIndex, const QPoint &angleDelta, bool ctrl) const;
    virtual bool filterContextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos) const;
    //virtual bool filterMousePress(int viewIndex, int button, const QPoint &viewPos) const;
protected:
    CollectionModel *model = nullptr;
    CanvasModelShell *canvasModelShell = nullptr;
    CanvasViewShell *canvasViewShell = nullptr;
    CanvasGridShell *canvasGridShell = nullptr;
    CanvasManagerShell *canvasManagerShell = nullptr;
    CanvasSelectionShell *canvasSelectionShell = nullptr;
    QList<SurfacePointer> surfaces;
    bool editing = false;
};

}

#endif   // MUSTERMODE_H
