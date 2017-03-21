#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H


#include "dabstractfilecontroller.h"

class NetworkController: public DAbstractFileController
{
    Q_OBJECT

public:
    explicit NetworkController(QObject *parent = 0);
    ~NetworkController();

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const Q_DECL_OVERRIDE;
};

#endif // NETWORKCONTROLLER_H
