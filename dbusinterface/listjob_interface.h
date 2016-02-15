#ifndef LISTJOBINTERFACE_H
#define LISTJOBINTERFACE_H

#include "dbustype.h"

#include <QObject>

class ListJobInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit ListJobInterface(const QString &service, const QString &path,
                              const QString &interface, QObject *parent = 0);

    ~ListJobInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> Abort()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Abort"), argumentList);
    }

    inline QDBusPendingReply<FileItemInfoList> Execute()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Execute"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void Aborted();
    void Done();
    /// TODO
    //void EntryInfo();
    void ProcessedAmount(qlonglong in0, ushort in1);
};

#endif // LISTJOBINTERFACE_H
