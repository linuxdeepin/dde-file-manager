#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>

class Listener : public QObject
{
    Q_OBJECT
public:
    explicit Listener(QObject *parent = nullptr);

signals:

public slots:
    QStringList plugins(){}
    QStringList services(){}
};

#endif // LISTENER_H
