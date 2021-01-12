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
    inline QMap<ScreenPointer, CanvasViewPointer> canvas() const{return m_canvasMap;}
signals:
public slots:
    void onCanvasViewBuild(int mode);
private slots:
    void onBackgroundEnableChanged();
    void onScreenGeometryChanged();
    void onSyncOperation(int so, QVariant var);
    void onSyncSelection(CanvasGridView *v, DUrlList selected);
private:
    void init();
    void arrageEditDeal(const QString &);
private:
    BackgroundManager *m_background = nullptr;
    QMap<ScreenPointer, CanvasViewPointer> m_canvasMap;
};

#endif // CANVASVIEWMANAGER_H
