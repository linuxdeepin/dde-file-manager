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

DCustomActionData &DCustomActionData::operator=(const DCustomActionData &other)
{
    if (this == &other)
        return *this;
    m_name = other.m_name;
    m_position = other.m_position;
    m_separator = other.m_separator;
    m_command = other.m_command;
    m_childrenActions = other.m_childrenActions;
    return *this;
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


DCustomActionEntry::DCustomActionEntry()
{

}

DCustomActionEntry::DCustomActionEntry(const DCustomActionEntry &other)
{
    m_package = other.m_package;
    m_version = other.m_version;
    m_comment = other.m_comment;
    m_fileCombo = other.m_fileCombo;
    m_mimeTypes = other.m_mimeTypes;
    m_data = other.m_data;
}

DCustomActionEntry &DCustomActionEntry::operator=(const DCustomActionEntry &other)
{
    if (this == &other)
        return *this;
    m_package = other.m_package;
    m_version = other.m_version;
    m_comment = other.m_comment;
    m_mimeTypes = other.m_mimeTypes;
    m_data = other.m_data;
    return *this;
}

QString DCustomActionEntry::package() const
{
    return m_package;
}

QString DCustomActionEntry::version() const
{
    return m_version;
}

QString DCustomActionEntry::comment() const
{
    return m_comment;
}

DCustomActionDefines::ComboTypes DCustomActionEntry::fileCombo() const
{
    return m_fileCombo;
}

QStringList DCustomActionEntry::mimeTypes() const
{
    return m_mimeTypes;
}

DCustomActionData DCustomActionEntry::data() const
{
    return m_data;
}
