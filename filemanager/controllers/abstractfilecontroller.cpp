#include "abstractfilecontroller.h"

AbstractFileController::AbstractFileController(QObject *parent)
    : QObject(parent)
{

}

bool AbstractFileController::openFile(const QString &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return false;
}

const QList<AbstractFileInfo*> AbstractFileController::getChildren(const QString &fileUrl, QDir::Filters filter, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(filter)

    accepted = false;

    return QList<AbstractFileInfo*>();
}

AbstractFileInfo *AbstractFileController::createFileInfo(const QString &fileUrl, bool &accepted) const
{
    Q_UNUSED(fileUrl)

    accepted = false;

    return Q_NULLPTR;
}
