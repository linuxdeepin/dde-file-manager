#ifndef ABSTRACTSCREEN_H
#define ABSTRACTSCREEN_H

#include <QObject>
#include <QSharedPointer>

class AbstractScreen : public QObject
{
    Q_OBJECT
public:
    explicit AbstractScreen(QObject *parent = nullptr);
    virtual ~AbstractScreen();
    virtual QString name() const = 0;
    virtual QRect geometry() const = 0;
    virtual QRect availableGeometry() const = 0;
    virtual QRect handleGeometry() const = 0;
signals:
    void sigGeometryChanged(const QRect &);
    void sigAvailableGeometryChanged(const QRect &);
public slots:
private:
    Q_DISABLE_COPY(AbstractScreen)
};

typedef QSharedPointer<AbstractScreen> ScreenPointer;
#endif // ABSTRACTSCREEN_H
