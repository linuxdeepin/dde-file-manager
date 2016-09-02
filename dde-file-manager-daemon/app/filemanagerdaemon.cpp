#include "filemanagerdaemon.h"
#include "controllers/appcontroller.h"

FileManagerDaemon::FileManagerDaemon(QObject *parent) : QObject(parent)
{
    initControllers();
    initConnect();
}

FileManagerDaemon::~FileManagerDaemon()
{

}

void FileManagerDaemon::initControllers()
{
    m_appController = new AppController(this);
}

void FileManagerDaemon::initConnect()
{

}
