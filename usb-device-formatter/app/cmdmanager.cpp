#include "cmdmanager.h"
#include <QStringList>

CMDManager *CMDManager::instance()
{
    static CMDManager * instance;
    if(!instance)
        instance = new CMDManager(0);

    return instance;
}

void CMDManager::process(const QApplication &app)
{
    init();
    m_parser.process(app);
}

void CMDManager::init()
{
    m_parser.addOption(m_modelModeOpt);
    m_parser.setApplicationDescription("Usb Device Formatter");
    m_parser.addHelpOption();
    m_parser.addVersionOption();
}

bool CMDManager::isSet(const QString &name)
{
    return m_parser.isSet(name);
}

QString CMDManager::getPath()
{
    return m_parser.positionalArguments().at(0);
}

int CMDManager::getWinId()
{
    QString winId = m_parser.value(m_modelModeOpt);
    if(winId.isEmpty())
        return -1;
    return winId.toInt();
}

CMDManager::CMDManager(QObject *parent) :
    QObject(parent),
    m_modelModeOpt(QStringList() << "m" << "model-mode",
                   "Enable model mode.",
                   "window ID")
{
}
