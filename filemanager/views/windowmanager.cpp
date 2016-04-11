#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include <QThread>
#include "../controllers/filejob.h"
#include <QDebug>

QHash<const QWidget*, int> WindowManager::m_windows;
int WindowManager::m_count = 0;

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{

}

WindowManager::~WindowManager()
{

}

void WindowManager::showNewWindow()
{
    DFileManagerWindow* window = new DFileManagerWindow;

    connect(window, &DFileManagerWindow::destroyed,
            this, &WindowManager::onWindowDestroyed);
    m_windows.insert(window, m_count++);

    window->show();
}

int WindowManager::getWindowId(const QWidget *window)
{
    return m_windows.value(window, -1);
}

void WindowManager::onWindowDestroyed(const QObject *obj)
{
    m_windows.remove(static_cast<const QWidget*>(obj));
}
