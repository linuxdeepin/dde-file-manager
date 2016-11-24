#include "filesignalmanager.h"

#include "dfmevent.h"
#include "dabstractfileinfo.h"
#include "durl.h"
#include <QDir>


FileSignalManager::FileSignalManager(QObject *parent):
    QObject(parent)
{
    qRegisterMetaType<DFMEvent>(QT_STRINGIFY(DFMEvent));
    qRegisterMetaType<QDir::Filters>(QT_STRINGIFY(QDir::Filters));
    qRegisterMetaType<QList<DAbstractFileInfoPointer>>(QT_STRINGIFY(QList<DAbstractFileInfoPointer>));
    qRegisterMetaType<DUrl>(QT_STRINGIFY(DUrl));
    qRegisterMetaType<DUrl>();
    qRegisterMetaType<DUrlList>();
}
