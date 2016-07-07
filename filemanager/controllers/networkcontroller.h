#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H


#include "abstractfilecontroller.h"

class NetworkController: public AbstractFileController
{
    Q_OBJECT

public:
    explicit NetworkController(QObject *parent = 0);
    ~NetworkController();

    const AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, const FMEvent &event, bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // NETWORKCONTROLLER_H
