// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTMENUSCENE_P_H
#define ABSTRACTMENUSCENE_P_H

#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/interfaces/fileinfo.h>

namespace dfmbase {

class AbstractMenuScenePrivate : public QObject
{
    Q_OBJECT
public:
    friend class AbstractMenuScene;
    explicit AbstractMenuScenePrivate(AbstractMenuScene *qq);

    virtual bool initializeParamsIsValid();

public:
    QUrl currentDir;
    QList<QUrl> selectFiles;
    QUrl focusFile;
    bool onDesktop { false };
    bool isEmptyArea { false };
    bool isFocusOnDDEDesktopFile { false };
    bool isDDEDesktopFileIncluded { false };
    bool isSystemPathIncluded { false };
    quint64 windowId { 0 };
    Qt::ItemFlags indexFlags;

    FileInfoPointer focusFileInfo;
    QMap<QString, QAction *> predicateAction;   // id -- instance
    QMap<QString, QString> predicateName;   // id -- text
};

}
#endif   // ABSTRACTMENUSCENE_P_H
