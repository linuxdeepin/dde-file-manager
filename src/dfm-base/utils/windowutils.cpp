#include "windowutils.h"

#include <QApplication>

bool dfmbase::WindowUtils::isWayLand()
{
    //! This function can only be called after QApplication to return a valid value, before it will return a null value
    Q_ASSERT(qApp);
    return QApplication::platformName() == "wayland";
}
