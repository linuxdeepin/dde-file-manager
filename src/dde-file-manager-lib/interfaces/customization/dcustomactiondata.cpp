#include "dcustomactiondata.h"

DCustomActionData::DCustomActionData() :
    m_position(0)
    , m_nameArg(DCustomActionDefines::NoneArg)
    , m_cmdArg(DCustomActionDefines::NoneArg)
    , m_separator(DCustomActionDefines::None)
{

}

DCustomActionData::DCustomActionData(const DCustomActionData &other)
    : m_comboPos(other.m_comboPos)
    , m_position(other.m_position)
    , m_nameArg(other.m_nameArg)
    , m_cmdArg(other.m_cmdArg)
    , m_name(other.m_name)
    , m_icon(other.m_icon)
    , m_command(other.m_command)
    , m_separator(other.m_separator)
    , m_childrenActions(other.m_childrenActions)
{

}

DCustomActionData &DCustomActionData::operator=(const DCustomActionData &other)
{
    if (this == &other)
        return *this;
    m_nameArg = other.m_nameArg;
    m_cmdArg = other.m_cmdArg;
    m_name = other.m_name;
    m_comboPos = other.m_comboPos;
    m_position = other.m_position;
    m_separator = other.m_separator;
    m_icon = other.m_icon;
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

int DCustomActionData::position(DCustomActionDefines::ComboType combo) const
{
    auto it = m_comboPos.find(combo);
    if (it != m_comboPos.end())
        return it.value();
    else
        return m_position;
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

DCustomActionDefines::ActionArg DCustomActionData::nameArg() const
{
    return m_nameArg;
}

DCustomActionDefines::ActionArg DCustomActionData::commandArg() const
{
    return m_cmdArg;
}


DCustomActionEntry::DCustomActionEntry()
{

}

DCustomActionEntry::DCustomActionEntry(const DCustomActionEntry &other)
    : m_package(other.m_package)
    , m_version(other.m_version)
    , m_comment(other.m_comment)
    , m_sign(other.m_sign)
    , m_fileCombo(other.m_fileCombo)
    , m_mimeTypes(other.m_mimeTypes)
    , m_excludeMimeTypes(other.m_excludeMimeTypes)
    , m_supportSchemes(other.m_supportSchemes)
    , m_notShowIn(other.m_notShowIn)
    , m_supportSuffix(other.m_supportSuffix)
    , m_data(other.m_data)
{

}

DCustomActionEntry &DCustomActionEntry::operator=(const DCustomActionEntry &other)
{
    if (this == &other)
        return *this;
    m_package = other.m_package;
    m_version = other.m_version;
    m_comment = other.m_comment;
    m_fileCombo = other.m_fileCombo;
    m_mimeTypes = other.m_mimeTypes;
    m_excludeMimeTypes = other.m_excludeMimeTypes;
    m_supportSchemes = other.m_supportSchemes;
    m_notShowIn = other.m_notShowIn;
    m_supportSuffix = other.m_supportSuffix;
    m_sign = other.m_sign;
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

QStringList DCustomActionEntry::excludeMimeTypes() const
{
    return m_excludeMimeTypes;
}

QStringList DCustomActionEntry::surpportSchemes() const
{
    return m_supportSchemes;
}

QStringList DCustomActionEntry::notShowIn() const
{
    return m_notShowIn;
}

QStringList DCustomActionEntry::supportStuffix() const
{
    return m_supportSuffix;
}

DCustomActionData DCustomActionEntry::data() const
{
    return m_data;
}
