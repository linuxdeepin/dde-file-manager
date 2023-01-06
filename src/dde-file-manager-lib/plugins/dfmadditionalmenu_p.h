// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMADDITIONALMENU_P_H
#define DFMADDITIONALMENU_P_H

#include <QObject>
#include <XdgDesktopFile>
#include <dfmglobal.h>
#include <dfilemenu.h>

DFM_BEGIN_NAMESPACE

class DFMAdditionalMenu;
class DFMAdditionalMenuPrivate : public QSharedData
{
    Q_DECLARE_PUBLIC(DFMAdditionalMenu)
public:
    const QStringList AllMenuTypes {
        "SingleFile",
        "SingleDir",
        "MultiFileDirs",
        "EmptyArea"
    };

    const QLatin1String MENU_TYPE_KEY {"X-DFM-MenuTypes"};
    const QLatin1String MIMETYPE_EXCLUDE_KEY {"X-DFM-ExcludeMimeTypes"};
    const QLatin1String MENU_HIDDEN_KEY {"X-DFM-NotShowIn"};     // "Desktop", "Filemanager"
    const QLatin1String SUPPORT_SCHEMES_KEY {"X-DFM-SupportSchemes"}; // file, trash, tag..
    const QLatin1String SUPPORT_SUFFIX_KEY {"X-DFM-SupportSuffix"}; // for deepin-compress *.7z.001,*.7z.002,*.7z.003...

    explicit DFMAdditionalMenuPrivate(DFMAdditionalMenu *qq);
    ~DFMAdditionalMenuPrivate();

    QStringList getValues(XdgDesktopFile &file, const QLatin1String &key, const QStringList &whiteList = {});
    bool isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes);
    bool isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes);
    bool isActionShouldShow(QAction *action, bool onDesktop);
    bool isSchemeSupport(QAction *action, const DUrl &url);
    bool isSuffixSupport(QAction *action, const DUrl &url, const bool ballEx7z = false);
    //都是7z分卷压缩文件
    bool isAllEx7zFile(const QStringList &files);
    QList<QAction *> emptyAreaActoins(const QString &currentDir, bool onDesktop);
private:
    QList<QAction *> actionList;
    QMap<QString, QList<QAction *> > actionListByType;
    QObject *menuActionHolder {nullptr};
    QList<DFileMenu *> menuList;
    QTimer *m_delayedLoadFileTimer;

    DFMAdditionalMenu *q_ptr;
};

DFM_END_NAMESPACE

#endif // DFMADDITIONALMENU_P_H
