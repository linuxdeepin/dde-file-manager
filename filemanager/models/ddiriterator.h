#ifndef DDIRITERATOR_H
#define DDIRITERATOR_H

#include "abstractfileinfo.h"

#include <QSharedData>

class DDirIterator : public QSharedData
{
public:
    virtual ~DDirIterator() {};

    virtual DUrl next() = 0;
    virtual bool hasNext() const = 0;

    virtual QString fileName() const = 0;
    virtual QString filePath() const = 0;
    virtual const AbstractFileInfoPointer fileInfo() const = 0;
    virtual QString path() const = 0;
};

typedef QExplicitlySharedDataPointer<DDirIterator> DDirIteratorPointer;

#endif // DDIRITERATOR_H
