#include "windowmanager.h"
#include "dfilemanagerwindow.h"

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{

}

WindowManager::~WindowManager()
{

}

void WindowManager::showNewWindow()
{
    DFileManagerWindow* window = new DFileManagerWindow;
    window->show();
    m_windows.insert(m_windows.count() + 1, window);
}
