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
    parser.process(app);
}

void CMDManager::init()
{
    parser.addOption(m_modelModeOpt);
    parser.setApplicationDescription("Usb Device Formatter");
    parser.addHelpOption();
    parser.addVersionOption();
}

bool CMDManager::isSet(const QString &name)
{
    return parser.isSet(name);
}

const QString CMDManager::getPath()
{
    return parser.positionalArguments().at(0);
}

const int CMDManager::getWinId()
{
    QString winId = parser.value(m_modelModeOpt);
    if(winId.isEmpty())
        return -1;
    return winId.toInt();
}

CMDManager::CMDManager(QObject *parent) :
    QObject(parent),
    m_modelModeOpt(QStringList() << "m" << "model-mode",
                   QCoreApplication::translate("main", "Enable model mode."),
                   QCoreApplication::translate("main", "window ID"))
{
}
