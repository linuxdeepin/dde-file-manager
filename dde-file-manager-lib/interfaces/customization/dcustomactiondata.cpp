#include "dcustomactiondata.h"

DCustomActionData::DCustomActionData()
{

}

DCustomActionData::DCustomActionData(const DCustomActionData &other)
{
    m_name = other.m_name;
    m_icon = other.m_icon;
    m_position = other.m_position;
    m_separator = other.m_separator;
    m_command = other.m_command;
    m_childrenActions = other.m_childrenActions;
}

bool DCustomActionData::isMenu() const
{
    return !m_childrenActions.isEmpty();
}

bool DCustomActionData::isAction() const
{
    return m_childrenActions.isEmpty();
}

QString DCustomActionData::name() const
{
    return m_name;
}

int DCustomActionData::position() const
{
    return m_position;
}

QString DCustomActionData::icon() const
{
    return m_icon;
}

QString DCustomActionData::command() const
{
    return m_command;
}

DCustomActionDefines::Separator DCustomActionData::separator() const
{
    return m_separator;
}

QList<DCustomActionData> DCustomActionData::acitons() const
{
    return m_childrenActions;
}


DCustomActionFile::DCustomActionFile()
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

QString DCustomActionFile::package() const
{
    return m_package;
}

QString DCustomActionFile::version() const
{
    return m_version;
}

QString DCustomActionFile::comment() const
{
    return m_comment;
}

DCustomActionDefines::FileComboTypes DCustomActionFile::fileCombo() const
{
    return m_fileCombo;
}

QStringList DCustomActionFile::mimeTypes() const
{
    return m_mimeTypes;
}

QList<DCustomActionData> DCustomActionFile::rootActions() const
{
    return m_rootActions;
}
