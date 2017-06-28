#ifndef BURNCONTROLLER_H
#define BURNCONTROLLER_H

#include "dabstractfilecontroller.h"

class BurnController: public DAbstractFileController
{
    Q_OBJECT

public:
    explicit BurnController(QObject *parent = 0);
    ~BurnController();

    const DAbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
    const QList<DAbstractFileInfoPointer> getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                     QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                     bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // BURNCONTROLLER_H
