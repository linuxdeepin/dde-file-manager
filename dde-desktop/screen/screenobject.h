#ifndef SCREENOBJECT_H
#define SCREENOBJECT_H

#include "abstractscreen.h"
#include <QScreen>

class ScreenObject : public AbstractScreen
{
public:
    ScreenObject(QScreen *sc, QObject *parent = nullptr);
    ~ScreenObject() override;
    QString name() const override;
    QRect geometry() const override;
    QRect availableGeometry() const override;
    QRect handleGeometry() const override;
public:
    QScreen *screen() const;
private:
    void init();
private:
    QScreen *m_screen = nullptr;
};

typedef QSharedPointer<ScreenObject> ScreenObjectPointer;
#endif // SCREENOBJECT_H
