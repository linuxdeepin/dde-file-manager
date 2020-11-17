#include "dcustomactiondata.h"

DCustomActionFile::DCustomActionFile(QObject *parent) : QObject(parent)
{

}

DCustomActionFile::DCustomActionFile(const DCustomActionFile &other)
{
    m_package = other.m_package;
    m_version = other.m_version;
    m_comment = other.m_comment;
    m_fileCombo = other.m_fileCombo;
    m_mimeTypes = other.m_mimeTypes;
    m_rootActions = other.m_rootActions;
}


DCustomActionData::DCustomActionData(QObject *parent) : QObject(parent)
{

}

DCustomActionData::DCustomActionData(const DCustomActionData &other)
{
    m_name = other.m_name;
    m_genericName = other.m_genericName;
    m_position = other.m_position;
    m_separator = other.m_separator;
    m_command = other.m_command;
    m_childrenActions = other.m_childrenActions;
}

