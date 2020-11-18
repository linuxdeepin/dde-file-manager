#include "dcustomactionparser.h"

#include <QDir>

DCustomActionParser::DCustomActionParser(QObject *parent) : QObject(parent)
{

}

bool DCustomActionParser::loadDir(const QString &dirPath)
{
    return true;
}

QList<DCustomActionEntry> DCustomActionParser::getActionFiles()
{
    return m_actionFiles;
}
