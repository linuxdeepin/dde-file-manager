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

void WindowManager::loadWindowState(DMainWindow *window)
{
    m_fmStateManager->loadCache();
    FMState* state = m_fmStateManager->fmState();
    int x = state->x();
    int y = state->y();
    int width = state->width();
    int height = state->height();
    int windowState = state->windowState();
    window->setGeometry(x, y, width, height);
    window->setWindowState(static_cast<Qt::WindowState>(windowState));
    window->fileManagerWindow()->getTitleBar()->setWindowState(static_cast<Qt::WindowState>(windowState));
}


void WindowManager::saveWindowState(DMainWindow *window)
{
    m_fmStateManager->fmState()->setViewMode(window->fileManagerWindow()->getFileViewMode());
    m_fmStateManager->fmState()->setX(window->x());
    m_fmStateManager->fmState()->setY(window->y());
    m_fmStateManager->fmState()->setWidth(window->size().width());
    m_fmStateManager->fmState()->setHeight(window->size().height());
    m_fmStateManager->fmState()->setWindowState(window->windowState());
    m_fmStateManager->saveCache();
}

void WindowManager::showNewWindow(const DUrl &url, bool isAlwaysOpen)
{
    if (!isAlwaysOpen){
        for(int i=0; i< m_windows.count(); i++){
            QWidget* window = const_cast<QWidget *>(m_windows.keys().at(i));
            DUrl currentUrl = static_cast<DMainWindow *>(window)->fileManagerWindow()->currentUrl();
            if (currentUrl == url){
                qDebug() << currentUrl << static_cast<DMainWindow *>(window);
                qApp->setActiveWindow(static_cast<DMainWindow *>(window));
                return;
            }
        }
    }

    DMainWindow *window = new DMainWindow();
    connect(window, &DMainWindow::aboutToClose,
            this, &WindowManager::onWindowClosed);

    qDebug() << "new window" << window->winId() << url;
    m_windows.insert(window, window->winId());

    loadWindowState(window);
//    if (m_windows.count() == 1){
//        window->moveCenter();
//    }
    window->fileManagerWindow()->setFileViewMode(m_fmStateManager->fmState()->viewMode());
    window->show();

    FMEvent event;
    if (!url.isEmpty()){
        event = url;
    }else{
        event = DUrl::fromLocalFile(QDir::homePath());
    }
    event = window->winId();
    emit fileSignalManager->requestChangeCurrentUrl(event);

    qApp->setActiveWindow(window);
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
        DMainWindow* window = static_cast<DMainWindow*>(sender());
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
