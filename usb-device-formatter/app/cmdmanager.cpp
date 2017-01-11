#include "cmdmanager.h"

CMDManager *CMDManager::instance()
{
    static CMDManager * instance;
    if(!instance)
        instance = new CMDManager(0);

    return instance;
}

void CMDManager::process(const QApplication &app)
{
    parser.process(app);
}

void CMDManager::init()
{
}

bool CMDManager::isSet(const QString &name)
{
    return parser.isSet(name);
}

const QString CMDManager::getPath()
{
    return parser.positionalArguments().at(0);
}

CMDManager::CMDManager(QObject *parent) : QObject(parent)
{
    init();
}
