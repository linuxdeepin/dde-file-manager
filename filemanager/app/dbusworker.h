#ifndef DBUSWORKER_H
#define DBUSWORKER_H

#include <QObject>

class QTimer;

class DBusWorker : public QObject
{
    Q_OBJECT
public:
    explicit DBusWorker(QObject *parent = 0);

signals:

public slots:
    void loadDesktopItems();
    void start();
    void stop();

private:
    QTimer* m_requestTimer;
    int m_count = 0;
};

#endif // DBUSWORKER_H
