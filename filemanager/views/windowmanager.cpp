#include "windowmanager.h"

WindowManager::WindowManager(QObject *parent) : QObject(parent)
{

}

WindowManager::~WindowManager()
{

}

void WindowManager::showNewWindow()
{
    FileManagerWindow* window = new FileManagerWindow;
    window->show();
    m_windows.insert(m_windows.count() + 1, window);
}
