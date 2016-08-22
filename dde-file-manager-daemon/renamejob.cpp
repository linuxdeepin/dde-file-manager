#include "renamejob.h"
#include <QDebug>

RenameJob::RenameJob(QObject *parent) : QObject(parent)
{

}

RenameJob::~RenameJob()
{

}

void RenameJob::Execute()
{
    qDebug() << "RenameJob execute";
}

