#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include "dfilemanagerwindow.h"

#include <QObject>
#include <QHash>

class WindowManager : public QObject
{
    Q_OBJECT

public:
    explicit WindowManager(QObject *parent = 0);
    ~WindowManager();
    void initConnect();

signals:
    void start(const QString &src);

public slots:
    void showNewWindow(const QString& url);

    static int getWindowId(const QWidget *window);

private slots:
    void onWindowDestroyed(const QObject *obj);
    inline QHash<const QWidget*, int> getWindows(){return m_windows;}

private:
     static QHash<const QWidget*, int> m_windows;
     static int m_count;
};

#endif // WINDOWMANAGER_H
