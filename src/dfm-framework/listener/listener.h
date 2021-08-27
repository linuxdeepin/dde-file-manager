#ifndef LISTENER_H
#define LISTENER_H

#include <QObject>

class Listener final : public QObject
{
    Q_OBJECT
public:
    explicit Listener(QObject *parent = nullptr);

signals:

public slots:
    QStringList plugins(){ return {};}
    QStringList services(){ return {};}
};

#endif // LISTENER_H
