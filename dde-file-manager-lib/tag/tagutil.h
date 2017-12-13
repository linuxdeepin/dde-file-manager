#ifndef TAGUTIL_H
#define TAGUTIL_H

namespace Tag
{
    enum class ActionType : std::size_t
    {
        MakeFilesTags = 1,
        GetFilesThroughTag,
        GetTagsThroughFile,
        RemoveTagsOfFiles,
        DeleteTags,
        ChangeTagName
    };
}


#endif // TAGUTIL_H
