#ifndef TAGUTIL_H
#define TAGUTIL_H


#include <QMap>
#include <QColor>
#include <QObject>
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
        ChangeFilesName,
        GetAllTags = 10,
        BeforeMakeFilesTags,
        GetTagsColor,
        ChangeTagColor
    };

extern const QMap<QString, QString> ColorsWithNames;
extern const QMap<QString, QColor> NamesWithColors;

const QMap<QString, QString> &ActualAndFakerName();

extern std::vector<QString> ColorName;
}


#endif // TAGUTIL_H
