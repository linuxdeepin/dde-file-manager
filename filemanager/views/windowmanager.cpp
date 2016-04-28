#include "windowmanager.h"
#include "dfilemanagerwindow.h"

#include "../controllers/filejob.h"

#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../app/fmevent.h"
#include "../models/fmstate.h"
#include "../controllers/fmstatemanager.h"
#include "utils/qobjecthelper.h"
#include <QThread>
#include <QDebug>
#include <QApplication>

QHash<const QWidget*, int> WindowManager::m_windows;
int WindowManager::m_count = 0;

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{
    m_fmStateManager = new FMStateManager(this);
    initConnect();
}

WindowManager::~WindowManager()
{

}

void WindowManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requestOpenNewWindowByUrl, this, &WindowManager::showNewWindow);
    connect(fileSignalManager, &FileSignalManager::aboutToCloseLastActivedWindow, this, &WindowManager::onLastActivedWindowClosed);
}

void WindowManager::loadWindowState(DFileManagerWindow *window)
{
    m_fmStateManager->loadCache();
    FMState* state = m_fmStateManager->fmState();
    int width = state->width();
    int height = state->height();
    window->resize(width, height);
}


void WindowManager::saveWindowState(DFileManagerWindow *window)
{
    m_fmStateManager->fmState()->setViewMode(window->getFileViewMode());
    m_fmStateManager->fmState()->setWidth(window->size().width());
    m_fmStateManager->fmState()->setHeight(window->size().height());
    m_fmStateManager->saveCache();
}

void WindowManager::showNewWindow(const DUrl &url, bool isAlwaysOpen)
{
    if (!isAlwaysOpen){
        for(int i=0; i< m_windows.count(); i++){
            QWidget* window = const_cast<QWidget *>(m_windows.keys().at(i));
            DUrl currentUrl = static_cast<DFileManagerWindow *>(window)->currentUrl();
            if (currentUrl == url){
                qDebug() << currentUrl << static_cast<DFileManagerWindow *>(window);
                qApp->setActiveWindow(static_cast<DFileManagerWindow *>(window));
                return;
            }
        }
    }

    DFileManagerWindow* window = new DFileManagerWindow;
    connect(window, &DFileManagerWindow::aboutToClose,
            this, &WindowManager::onWindowClosed);
    qDebug() << "new window" << window->winId() << url;
    int winId = window->winId();
    m_windows.insert(window, winId);
    qDebug() << m_windows;
    window->show();

    if (m_windows.count() == 1){
        loadWindowState(window);
        window->moveCenter();
    }
    window->setFileViewMode(m_fmStateManager->fmState()->viewMode());
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

QWidget *WindowManager::getWindowById(int winId)
{
    for(int i=0; i< m_windows.count(); i++){
        if (m_windows.values().at(i) == winId){
            QWidget* window = const_cast<QWidget *>(m_windows.keys().at(i));
            return window;
        }
    }
    return Q_NULLPTR;
}

void WindowManager::onWindowClosed()
{
    if (m_windows.count() == 1){
        DFileManagerWindow* window = static_cast<DFileManagerWindow*>(sender());
        saveWindowState(window);
    }
    m_windows.remove(static_cast<const QWidget*>(sender()));

}

void WindowManager::onLastActivedWindowClosed(int winId)
{
    QList<int> winIds = m_windows.values();
    foreach (int id, winIds) {
        if (id != winId){
            getWindowById(id)->close();
        }
    }
    getWindowById(winId)->close();
}
