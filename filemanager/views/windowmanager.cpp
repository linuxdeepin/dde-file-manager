#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include <QThread>
#include "../controllers/filejob.h"
#include <QDebug>

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

void WindowManager::progressPercent(int value)
{
    qDebug() << "progress = " << value;
}

void WindowManager::error(QString content)
{
    qDebug() << "error = " << content;
}

void WindowManager::result(QString content)
{
    qDebug() << "result = " << content;
}
