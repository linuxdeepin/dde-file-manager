#ifndef SCREENOBJECT_H
#define SCREENOBJECT_H

#include "abstractscreen.h"
#include <QScreen>

class ScreenObject : public AbstractScreen
{
public:
    ScreenObject(QScreen *sc,QObject *parent = nullptr);
    ~ScreenObject() Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QRect geometry() const Q_DECL_OVERRIDE;
    QRect availableGeometry() const Q_DECL_OVERRIDE;
    QRect handleGeometry() const Q_DECL_OVERRIDE;
public:
    QScreen *screen() const;
private:
    void init();
private:
    QScreen *m_screen = nullptr;
};

typedef QSharedPointer<ScreenObject> ScreenObjectPointer;
#endif // SCREENOBJECT_H
