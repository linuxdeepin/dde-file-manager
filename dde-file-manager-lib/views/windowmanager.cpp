#include "windowmanager.h"
#include "dfilemanagerwindow.h"

#include "app/global.h"
#include "app/filesignalmanager.h"
#include "fmevent.h"

#include "models/fmstate.h"

#include "fileoperations/filejob.h"
#include "controllers/fmstatemanager.h"

#include "dialogs/dialogmanager.h"

#include "qobjecthelper.h"

#include "widgets/singleton.h"

#include <QThread>
#include <QDebug>
#include <QApplication>
#include <QX11Info>
#include <QScreen>

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
    connect(fileSignalManager, &FileSignalManager::requestQuitApplication, this, &WindowManager::quit);
}

void WindowManager::loadWindowState(DFileManagerWindow *window)
{
    m_fmStateManager->loadCache();
    FMState* state = m_fmStateManager->fmState();
    int x = state->x();
    int y = state->y();
    int width = state->width();
    int height = state->height();
    int windowState = state->windowState();
    window->resize(width, height);
    window->setWindowState(static_cast<Qt::WindowState>(windowState));
}


void WindowManager::saveWindowState(DFileManagerWindow *window)
{
    m_fmStateManager->fmState()->setViewMode(window->getFileViewMode());
    m_fmStateManager->fmState()->setX(window->x());
    m_fmStateManager->fmState()->setY(window->y());
    m_fmStateManager->fmState()->setWidth(window->size().width());
    m_fmStateManager->fmState()->setHeight(window->size().height());
    m_fmStateManager->fmState()->setWindowState(window->windowState());
    m_fmStateManager->saveCache();
}

DUrl WindowManager::getUrlByWindowId(int windowId)
{
    if (getWindowById(windowId)){
         DFileManagerWindow* window = qobject_cast<DFileManagerWindow*>(getWindowById(windowId));
         return window->currentUrl();
    }
    return DUrl::fromLocalFile(QDir::homePath());
}

bool WindowManager::tabAddableByWinId(const int &winId)
{
    DFileManagerWindow* window = qobject_cast<DFileManagerWindow*>(getWindowById(winId));
    if(window)
        return window->tabAddable();
    return false;

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

    QX11Info::setAppTime(QX11Info::appUserTime());
    DFileManagerWindow *window = new DFileManagerWindow();
    loadWindowState(window);
    window->show();
    qDebug() << "new window" << window->winId() << url;

    connect(window, &DFileManagerWindow::aboutToClose,
            this, &WindowManager::onWindowClosed);

    m_windows.insert(window, window->winId());

    if (m_windows.count() == 1){
        QPoint pos = QCursor::pos();
        QRect currentScreenGeometry;

        for (QScreen *screen : qApp->screens()) {
            if (screen->geometry().contains(pos)) {
                currentScreenGeometry = screen->geometry();
            }
        }

        if (currentScreenGeometry.isEmpty()) {
            currentScreenGeometry = qApp->primaryScreen()->geometry();
        }

        window->moveCenter(currentScreenGeometry.center());
    }
    window->setFileViewMode(m_fmStateManager->fmState()->viewMode());

    FMEvent event;
    if (!url.isEmpty()){
        event << url;
    }else{
        event << DUrl::fromLocalFile(QDir::homePath());
    }
    event << window->winId();
    emit fileSignalManager->requestChangeCurrentUrl(event);

    qApp->setActiveWindow(window);
}



int WindowManager::getWindowId(const QWidget *window)
{
    int winId = m_windows.value(window->topLevelWidget(), -1);

    if (winId != -1)
        return winId;

    while (window) {
        if (window->inherits("DFileManagerWindow")) {
            return window->winId();
        }

        window = window->parentWidget();
    }

    return -1;
}

QWidget *WindowManager::getWindowById(int winId)
{
    const QWidget *widget = m_windows.key(winId);

    if (widget)
        return const_cast<QWidget*>(widget);

    for (QWidget *widget : qApp->topLevelWidgets()) {
        if ((int)widget->winId() == winId)
            return widget;
    }

    return Q_NULLPTR;
}

void WindowManager::onWindowClosed()
{
    if (m_windows.count() == 1){
        DFileManagerWindow* window = static_cast<DFileManagerWindow*>(sender());
        saveWindowState(window);
        dialogManager->closeAllPropertyDialog();
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

void WindowManager::quit()
{
    if (m_windows.count() == 0){
        qApp->quit();
    }
}
