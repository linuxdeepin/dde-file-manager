#ifndef NETWORKCONTROLLER_H
#define NETWORKCONTROLLER_H


#include "dabstractfilecontroller.h"

class NetworkController: public DAbstractFileController
{
    Q_OBJECT

public:
    explicit NetworkController(QObject *parent = 0);
    ~NetworkController();

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                     QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                     bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // NETWORKCONTROLLER_H
