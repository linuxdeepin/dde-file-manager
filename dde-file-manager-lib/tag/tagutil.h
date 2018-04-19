#ifndef TAGUTIL_H
#define TAGUTIL_H


#include <QMap>
#include <QString>

#include <vector>


///###: ODR!
namespace Tag
{
    enum class ActionType : std::size_t
    {
        MakeFilesTags = 1,
        GetFilesThroughTag,
        GetTagsThroughFile,
        RemoveTagsOfFiles,
        DeleteTags,
        ChangeTagName,
        DeleteFiles,
        MakeFilesTagThroughColor,
        ChangeFilesName
    };

extern const QMap<QString, QString> ColorsWithNames;


extern std::vector<QString> ColorName;
}


#endif // TAGUTIL_H
