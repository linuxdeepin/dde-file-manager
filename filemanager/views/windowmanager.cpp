#include "windowmanager.h"
#include "dfilemanagerwindow.h"

#include "../controllers/filejob.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"

#include <QThread>
#include <QDebug>
#include <QApplication>

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
    connect(fileSignalManager, &FileSignalManager::requestActiveWindow, this, &WindowManager::activefirstOpenedWindow);
}

void WindowManager::showNewWindow(const DUrl &url)
{
    DFileManagerWindow* window = new DFileManagerWindow;
    if (!m_firstOpenedWindow){
        m_firstOpenedWindow = window;
    }
    connect(window, &DFileManagerWindow::aboutToClose,
            this, &WindowManager::onWindowClosed);
    qDebug() << "new window" << window->winId();
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

void WindowManager::activefirstOpenedWindow()
{
    qDebug() << m_firstOpenedWindow;
    if (m_firstOpenedWindow){
        qApp->setActiveWindow(m_firstOpenedWindow);
    }else{
        showNewWindow(DUrl::fromLocalFile(QDir::homePath()));
    }
}

int WindowManager::getWindowId(const QWidget *window)
{
    return m_windows.value(window, -1);
}

void WindowManager::onWindowClosed()
{
    qDebug() << sender() <<  m_firstOpenedWindow;
    if (sender() == m_firstOpenedWindow){
        m_firstOpenedWindow = NULL;
    }
    m_windows.remove(static_cast<const QWidget*>(sender()));
}
