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
enum class ActionType : std::size_t {
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
extern const QMap<QString, QString> ColorNameWithIconName; // <TagColorName, IconName>

const QMap<QString, QString> &ActualAndFakerName();

extern std::vector<QString> ColorName;


extern QString escaping_en_skim(const QString &source) noexcept;
extern QString restore_escaped_en_skim(const QString &source) noexcept;



}


#endif // TAGUTIL_H
