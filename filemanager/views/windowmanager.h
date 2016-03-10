#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QHash>
#include "dfilemanagerwindow.h"

class WindowManager : public QObject
{
    Q_OBJECT
public:
    explicit WindowManager(QObject *parent = 0);
    ~WindowManager();

signals:

public slots:
    void showNewWindow();

private:
     QHash<int, DFileManagerWindow*> m_windows;
     int m_count = 0;
};

#endif // WINDOWMANAGER_H
