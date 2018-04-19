#include "tagutil.h"


namespace Tag {

const QMap<QString, QString> ColorsWithNames{
        { "#ffa503", "Orange"},
        { "#ff1c49", "Red"},
        { "#9023fc", "Purple"},
        { "#3468ff", "Navy-blue"},
        { "#00b5ff", "Azure"},
        { "#58df0a", "Grass-green"},
        { "#fef144", "Yellow"},
        { "#cccccc", "Gray" }
    };

const QMap<QString, QColor> NamesWithColors{
    {"Orange", "#ffa503"},
    {"Red", "#ff1c49"},
    {"Purple", "#9023fc"},
    {"Navy-blue", "#3468ff"},
    {"Azure", "#00b5ff"},
    {"Grass-green", "#58df0a"},
    {"Yellow", "#fef144"},
    {"Gray", "#cccccc"}
};


std::vector<QString> ColorName{
                                    "Orange",
                                    "Red",
                                    "Purple",
                                    "Navy-blue",
                                    "Azure",
                                    "Grass-green",
                                    "Yellow",
                                    "Gray"
                                  };


///###: Why has a faker name?
///###: pair.second represent the faker name,
///###: and them are used in translation.
const QMap<QString, QString> ActualAndFakerName{
    {"Orange", QObject::tr("Orange")},
    {"Red", QObject::tr("Red")},
    {"Purple", QObject::tr("Purple")},
    {"Navy-blue", QObject::tr("Navy-blue")},
    {"Azure", QObject::tr("Azure")},
    {"Grass-green", QObject::tr("Grass green")},
    {"Yellow", QObject::tr("Yellow")},
    {"Gray", QObject::tr("Gray")}
};

}

