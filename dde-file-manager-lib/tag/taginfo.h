#ifndef TAGINFO_H
#define TAGINFO_H


#include <array>
#include <utility>

#include "../interfaces/durl.h"

#include <QMap>
#include <QList>
#include <QString>
#include <QSharedPointer>



struct FileProperty
{
    QString m_fileName;
    std::array<QString, 3> m_tags;

    inline bool operator<(const FileProperty& other) const noexcept
    {
        return (m_fileName < other.m_fileName);
    }

    inline bool operator>(const FileProperty& other)const noexcept
    {
        return (m_fileName > other.m_fileName);
    }
};

struct TagProperty
{
    QString m_tagName;
    QString m_tagColor;
    std::size_t m_tagIndex;

    inline bool operator<(const TagProperty& other)const noexcept
    {
        return (m_tagName < other.m_tagName);
    }

    inline bool operator>(const TagProperty& other)const noexcept
    {
        return (m_tagName > other.m_tagName);
    }
};

class TagInfo
{
public:
    TagInfo()=default;
    ~TagInfo()=default;


    TagInfo(TagInfo&& other);
    TagInfo(const TagInfo& other);
    TagInfo& operator=(TagInfo&& other);
    TagInfo& operator=(const TagInfo& other);

    QSharedPointer<QList<DUrl>> underCurrentTagAllFiles()const;
    bool underTheTagWhetherHasThisFile(const DUrl& file)const;
    QSharedPointer<QString> currentTagName()const;


    inline void setTagProperty(const TagProperty &tagProperty)noexcept
    {
        m_filesUnderTag.first = tagProperty;
    }
    inline void addFileProperty(const FileProperty& fileProperty)
    {
        m_filesUnderTag.second.push_back(fileProperty);
    }

    void deleteAFileFromCurrentTag(const DUrl& file);

private:
    QPair<TagProperty, QList<FileProperty>> m_filesUnderTag{};
};



#endif // TAGINFO_H
