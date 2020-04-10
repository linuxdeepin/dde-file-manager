#ifndef CANVASVIEWMANAGER_H
#define CANVASVIEWMANAGER_H

#include "backgroundmanager.h"
#include "canvasgridview.h"
#include <QObject>

typedef QSharedPointer<CanvasGridView> CanvasViewPointer;

class CanvasViewManager : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewManager(BackgroundManager *bmrg,QObject *parent = nullptr);
    ~CanvasViewManager();
signals:
public slots:
    void onCanvasViewBuild(int mode);
private slots:
    void onBackgroundEnableChanged();
    void onScreenGeometryChanged(ScreenPointer);
private:
    void init();
private:
    BackgroundManager *m_background = nullptr;
    QMap<ScreenPointer, CanvasViewPointer> m_canvasMap;
};

#endif // CANVASVIEWMANAGER_H
