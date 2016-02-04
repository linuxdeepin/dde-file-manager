#ifndef GLOBAL_H
#define GLOBAL_H


#include <QtCore>
#include "widgets/singleton.h"
#include "signalmanager.h"
#include "../controllers/dbuscontroller.h"
#include "utils/utils.h"
#define gridManager  Singleton<GridManager>::instance()
#define signalManager  Singleton<SignalManager>::instance()
#define dbusController Singleton<DBusController>::instance()

#define defaut_icon ":/images/images/default.png"
#define defaut_computerIcon ":/images/images/computer.png"
#define defaut_trashIcon ":/images/images/user-trash-full.png"

#endif // GLOBAL_H
