#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include <QThread>
#include "../controllers/filejob.h"
#include <QDebug>
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"

QHash<const QWidget*, int> WindowManager::m_windows;
int WindowManager::m_count = 0;

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    initConnect();
}

WindowManager::~WindowManager()
{

}

void WindowManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestOpenNewWindowByUrl, this, &WindowManager::showNewWindow);
}

void WindowManager::showNewWindow(const QString &url)
{
    DFileManagerWindow* window = new DFileManagerWindow;

    connect(window, &DFileManagerWindow::destroyed,
            this, &WindowManager::onWindowDestroyed);
    int winId = window->winId();
    m_windows.insert(window, winId);
    qDebug() << m_windows;
    window->show();

    qApp->setActiveWindow(window);

    FMEvent event;
    event = url;
    event = winId;
    emit fileSignalManager->requestChangeCurrentUrl(event);
}

int WindowManager::getWindowId(const QWidget *window)
{
    return m_windows.value(window, -1);
}

void WindowManager::onWindowDestroyed(const QObject *obj)
{
    m_windows.remove(static_cast<const QWidget*>(obj));
}
