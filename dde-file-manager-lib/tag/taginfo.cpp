
#include "taginfo.h"


TagInfo::TagInfo(TagInfo&& other)
        :m_filesUnderTag{ std::move(other.m_filesUnderTag) }{}

TagInfo::TagInfo(const TagInfo& other)
        :m_filesUnderTag{ other.m_filesUnderTag }{}

TagInfo& TagInfo::operator=(TagInfo&& other)
{
    this->m_filesUnderTag = std::move(other.m_filesUnderTag);

    return *this;
}

TagInfo& TagInfo::operator=(const TagInfo& other)
{
    this->m_filesUnderTag = other.m_filesUnderTag;

    return *this;
}

QSharedPointer<QList<DUrl> > TagInfo::underCurrentTagAllFiles() const
{

    QSharedPointer<QList<DUrl>> allFiles{ nullptr };
    const QList<FileProperty>& files = m_filesUnderTag.second;

    if(files.empty() == false){

        allFiles = QSharedPointer<QList<DUrl>>{ new QList<DUrl>{} };
        for(const FileProperty& property : files){
            DUrl uri{ DUrl::fromLocalFile(property.m_fileName) };
            allFiles->push_back(uri);
        }

    }

    return allFiles;
}

bool TagInfo::underTheTagWhetherHasThisFile(const DUrl& file) const
{
    QString localeFile{ file.toLocalFile() };

    if(m_filesUnderTag.second.empty() == false){

        QList<FileProperty>::const_iterator cbeg{ m_filesUnderTag.second.cbegin() };
        QList<FileProperty>::const_iterator cend{ m_filesUnderTag.second.cend() };

        QList<FileProperty>::const_iterator resultItr{ std::find_if(cbeg, cend, [&](const FileProperty& property)
                                                                                {  return (localeFile == property.m_fileName);  })
                                                      };
        if(resultItr != cend){
            return true;
        }
    }

    return false;
}

QSharedPointer<QString> TagInfo::currentTagName() const
{
    QSharedPointer<QString> tagNamePtr{ nullptr };

    if(m_filesUnderTag.first.m_tagName.isEmpty() == false){

        tagNamePtr = QSharedPointer<QString>{ new QString{ m_filesUnderTag.first.m_tagName } };
    }

    return tagNamePtr;
}



void TagInfo::deleteAFileFromCurrentTag(const DUrl& file)
{
    QString fileStr{ file.toLocalFile() };

    QList<FileProperty>::iterator first{ m_filesUnderTag.second.begin() };
    QList<FileProperty>::iterator last{ m_filesUnderTag.second.end() };
    QList<FileProperty>::iterator resultItr{ std::find_if(first, last,
                                                       [&](const FileProperty& fileProperry){ return (fileProperry.m_fileName == fileStr); })
                                        };

    if(resultItr != last){
        m_filesUnderTag.second.erase(resultItr, resultItr + 1);
    }
}











