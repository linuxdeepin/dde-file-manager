// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/oemmenu_p.h"
#include "oemmenu.h"

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/protocolutils.h>

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QMenu>
#include <QDebug>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE

static const char *const kDesktopEntryGroup = "Desktop Entry";
static const char *const kNameKey = "Name";
static const char *const kIconKey = "Icon";
static const char *const kActionsKey = "Actions";
static const char *const kTypeKey = "Type";
static const char *const kLocaleKey = "default";
static const char *const kActionsGroup = "Desktop Action ";
static const char *const kMimeType = "MimeType";
static const char *const kMenuTypeKey = "X-DDE-FileManager-MenuTypes";
static const char *const kMenuTypeAliasKey = "X-DFM-MenuTypes";
static const char *const kMimeTypeExcludeKey = "X-DDE-FileManager-ExcludeMimeTypes";
static const char *const kMimeTypeExcludeAliasKey = "X-DFM-ExcludeMimeTypes";
static const char *const kMenuHiddenKey = "X-DDE-FileManager-NotShowIn";   // "Desktop", "Filemanager"
static const char *const kMenuHiddenAliasKey = "X-DFM-NotShowIn";
static const char *const kSupportSchemesKey = "X-DDE-FileManager-SupportSchemes";
static const char *const kSupportSchemesAliasKey = "X-DFM-SupportSchemes";
static const char *const kSupportSuffixKey = "X-DDE-FileManager-SupportSuffix";   // for deepin-compress *.7z.001,*.7z.002,*.7z.003...
static const char *const kSupportSuffixAliasKey = "X-DFM-SupportSuffix";
static const char *const kDesktop = "Desktop";
static const char *const kFilemanager = "Filemanager";
static const char *const kEmptyArea = "EmptyArea";
static const char *const kSingleFile = "SingleFile";
static const char *const kSingleDir = "SingleDir";
static const char *const kMultiFileDirs = "MultiFileDirs";

static const char *const kCommandKey = "Exec";
static const char *const kCommandArg[] { "%p", "%f", "%F", "%u", "%U" };

static QString oemMenuExtensionsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            + QStringLiteral("/deepin/dde-file-manager/oem-menuextensions");
}

OemMenuPrivate::OemMenuPrivate(OemMenu *qq)
    : q(qq)
{
    delayedLoadFileTimer.reset(new QTimer(q));
    delayedLoadFileTimer->setSingleShot(true);
    delayedLoadFileTimer->setInterval(500);

    QObject::connect(delayedLoadFileTimer.data(), &QTimer::timeout, q, &OemMenu::loadDesktopFile);

    oemMenuPath << QStringLiteral("/usr/etc/deepin/menu-extensions")
                << QStringLiteral("/etc/deepin/menu-extensions")
                << oemMenuExtensionsPath();

    menuTypes << kEmptyArea
              << kSingleFile
              << kSingleDir
              << kMultiFileDirs;

    actionProperties << kMimeType
                     << kMenuHiddenKey
                     << kMenuHiddenAliasKey
                     << kMimeTypeExcludeKey
                     << kMimeTypeExcludeAliasKey
                     << kSupportSchemesKey
                     << kSupportSchemesAliasKey
                     << kSupportSuffixKey
                     << kSupportSuffixAliasKey
                     << kCommandKey;

    for (auto path : oemMenuPath) {
        QUrl pathUrl = QUrl::fromLocalFile(path);
        auto watcher = new LocalFileWatcher(pathUrl, q);
        if (watcher) {
            QObject::connect(watcher, &LocalFileWatcher::fileDeleted, delayedLoadFileTimer.data(), [=]() {
                delayedLoadFileTimer->start();
            });
            QObject::connect(watcher, &LocalFileWatcher::subfileCreated, delayedLoadFileTimer.data(), [=]() {
                delayedLoadFileTimer->start();
            });
            QObject::connect(watcher, &LocalFileWatcher::fileAttributeChanged, delayedLoadFileTimer.data(), [=]() {
                delayedLoadFileTimer->start();
            });

            watcher->startWatcher();
        }
    }
}

OemMenuPrivate::~OemMenuPrivate()
{
    clearSubMenus();
}

QStringList OemMenuPrivate::getValues(const DDesktopEntry &entry, const QString &key, const QString &aliasKey, const QString &section, const QStringList &whiteList) const
{
    QStringList values(whiteList);
    if (entry.contains(key, section) || entry.contains(aliasKey, section)) {
        values = entry.stringListValue(key, section) + entry.stringListValue(aliasKey, section);

        if (whiteList.isEmpty())
            return values;

        for (const QString &value : values) {
            if (!whiteList.contains(value))
                values.removeAll(value);
        }
    }

    return values;
}

bool OemMenuPrivate::isMimeTypeSupport(const QString &mt, const QStringList &fileMimeTypes) const
{
    for (auto fmt : fileMimeTypes) {
        if (fmt.contains(mt, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

bool OemMenuPrivate::isMimeTypeMatch(const QStringList &fileMimeTypes, const QStringList &supportMimeTypes) const
{
    for (auto mt : supportMimeTypes) {
        if (fileMimeTypes.contains(mt, Qt::CaseInsensitive))
            return true;

        int index = mt.indexOf("*");
        if (index >= 0 && isMimeTypeSupport(mt.left(index), fileMimeTypes))
            return true;
    }

    return false;
}

bool OemMenuPrivate::isActionShouldShow(const QAction *action, bool onDesktop) const
{
    if (!action)
        return false;

    // X-DFM-NotShowIn not exist
    if (!action->property(kMenuHiddenKey).isValid() && !action->property(kMenuHiddenAliasKey).isValid())
        return true;

    QStringList notShowInList = action->property(kMenuHiddenKey).toStringList();
    notShowInList << action->property(kMenuHiddenAliasKey).toStringList();

    return (onDesktop && !notShowInList.contains(kDesktop, Qt::CaseInsensitive))
            || (!onDesktop && !notShowInList.contains(kFilemanager, Qt::CaseInsensitive));
}

bool OemMenuPrivate::isSchemeSupport(const QAction *action, const QUrl &url) const
{
    // X-DFM-SupportSchemes not exist
    if (!action || (!action->property(kSupportSchemesKey).isValid() && !action->property(kSupportSchemesAliasKey).isValid())) {
        return true;
    }

    QStringList supportList = action->property(kSupportSchemesKey).toStringList();
    supportList << action->property(kSupportSchemesAliasKey).toStringList();

    return supportList.contains(url.scheme(), Qt::CaseInsensitive);
}

bool OemMenuPrivate::isSuffixSupport(const QAction *action, FileInfoPointer fileInfo, const bool allEx7z) const
{
    // X-DFM-SupportSuffix not exist
    if (!fileInfo || fileInfo->isAttributes(OptInfoType::kIsDir) || !action || (!action->property(kSupportSuffixKey).isValid() && !action->property(kSupportSuffixAliasKey).isValid())) {
        if (allEx7z) {
            return false;
        }
        return true;
    }

    QStringList supportList = action->property(kSupportSuffixKey).toStringList();
    supportList << action->property(kSupportSuffixAliasKey).toStringList();

    const QString fileName = fileInfo->nameOf(NameInfoType::kFileName);

    // 优先使用 DMimeDatabase 识别真实后缀（基于 mime database，处理复式后缀和部分后缀区分）
    QString primarySuffix = mimeDatabase.suffixForFileName(fileName);

    // 处理分卷等特殊格式（如 7z.001），QMimeDatabase 对此类可能返回 7z.*
    // 此时需要回退到完整的完整后缀字符串
    if (primarySuffix.isEmpty() || primarySuffix.contains('*')) {
        primarySuffix = fileInfo->nameOf(NameInfoType::kCompleteSuffix);
    }

    QString cs = primarySuffix;
    if (supportList.contains(cs, Qt::CaseInsensitive)) {
        return true;
    }

    for (QString suffix : supportList) {
        int endPos = suffix.lastIndexOf("*");   // 7z.*
        if (endPos >= 0 && cs.length() > endPos && suffix.left(endPos) == cs.left(endPos)) {
            return true;
        }
    }

    return false;
}

bool OemMenuPrivate::isAllEx7zFile(const QList<QUrl> &files) const
{
    if (files.size() <= 1) {
        return false;
    }

    QString errString;
    for (const QUrl &f : files) {

        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(f, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (fileInfo.isNull()) {
            fmDebug() << errString;
            return false;
        }

        // 7z.001,7z.002, 7z.003 ... 7z.xxx
        QString cs = fileInfo->nameOf(NameInfoType::kCompleteSuffix);
        if (!cs.startsWith(QString("7z."))) {
            return false;
        }
    }

    return true;
}

bool OemMenuPrivate::isValid(const QAction *action, FileInfoPointer fileInfo, const bool onDesktop, const bool allEx7z) const
{
    if (!action)
        return false;

    return isActionShouldShow(action, onDesktop) && isSchemeSupport(action, fileInfo->urlOf(UrlInfoType::kUrl)) && isSuffixSupport(action, fileInfo, allEx7z);
}

void OemMenuPrivate::clearSubMenus()
{
    for (auto menu : subMenus) {
        menu->deleteLater();
        menu = nullptr;
    }
    subMenus.clear();
}

void OemMenuPrivate::setActionProperty(QAction *const action, const DDesktopEntry &entry, const QString &key, const QString &section) const
{
    if (!entry.contains(key, section))
        return;

    const QStringList &&values = entry.stringListValue(key, section);
    action->setProperty(key.toLatin1(), values);
}

QStringList OemMenuPrivate::splitCommand(const QString &cmd)
{
    QStringList args;
    bool inQuote = false;

    QString arg;
    for (int i = 0; i < cmd.count(); i++) {
        const bool isEnd = (cmd.size() == (i + 1));

        const QChar &ch = cmd.at(i);
        // quotation marks
        const bool isQuote = (ch == QLatin1Char('\'') || ch == QLatin1Char('\"'));

        // encountered quotation mark or last character
        if (!isEnd && isQuote) {
            // enter or exit quotation marks
            inQuote = !inQuote;
        } else {
            // use quotation marks or non spaces as an argument
            if ((!ch.isSpace() || inQuote) && !isQuote) {
                arg.append(ch);
            }

            // a space is encountered and a single parameter is no longer resolved in quotation marks
            if ((ch.isSpace() && !inQuote) || isEnd) {
                if (!arg.isEmpty()) {
                    args << arg;
                }
                arg.clear();
            }
        }
    }
    return args;
}

OemMenuPrivate::ArgType OemMenuPrivate::execDynamicArg(const QString &cmd) const
{
    int firstValidIndex = cmd.indexOf("%");
    auto cnt = cmd.length() - 1;
    if (0 == cnt || 0 > firstValidIndex)
        return kNoneArg;

    static const QHash<QString, ArgType> actionExecArg { { kCommandArg[kDirPath], kDirPath }, { kCommandArg[kFilePath], kFilePath }, { kCommandArg[kFilePaths], kFilePaths }, { kCommandArg[kUrlPath], kUrlPath }, { kCommandArg[kUrlPaths], kUrlPaths } };

    while (cnt > firstValidIndex) {
        auto tgStr = cmd.mid(firstValidIndex, 2);
        auto tempValue = actionExecArg.value(tgStr, kNoneArg);
        if (kNoneArg != tempValue) {
            return tempValue;
        }
        firstValidIndex = cmd.indexOf("%", firstValidIndex + 1);
        if (-1 == firstValidIndex)
            break;
    }

    return kNoneArg;
}

QStringList OemMenuPrivate::replace(QStringList &args, const QString &before, const QString &after) const
{
    QStringList rets;
    while (!args.isEmpty()) {
        QString arg = args.takeFirst();
        // find the first valid "before" matching value in the parameter and replace it with "after"
        // , and the subsequent will not be processed
        int index = arg.indexOf(before);
        if (index >= 0) {
            rets << arg.replace(index, before.size(), after);
            rets << args;
            args.clear();
        } else {
            rets << arg;
        }
    }
    return rets;
}

QStringList OemMenuPrivate::replaceList(QStringList &args, const QString &before, const QStringList &after) const
{
    QStringList rets;
    while (!args.isEmpty()) {
        QString arg = args.takeFirst();
        // only independent parameters are supported, and other combinations are not processed
        if (arg == before) {
            // file path
            rets << after;
            // original parameter
            rets << args;
            args.clear();
        } else {
            rets << arg;
        }
    }
    return rets;
}

QStringList OemMenuPrivate::urlListToLocalFile(const QList<QUrl> &files) const
{
    QStringList rets;
    for (auto it = files.begin(); it != files.end(); ++it) {
        rets << it->toLocalFile();
    }
    return rets;
}

QString OemMenuPrivate::urlToString(const QUrl &file) const
{
    return file.toLocalFile().isEmpty() ? QString::fromUtf8(file.toEncoded()) : file.toLocalFile();
}

QStringList OemMenuPrivate::urlListToString(const QList<QUrl> &files) const
{
    QStringList rets;
    for (auto it = files.begin(); it != files.end(); ++it) {
        rets << ((!it->toLocalFile().isEmpty()) ? it->toLocalFile() : QString::fromUtf8(it->toEncoded()));
    }
    return rets;
}

void OemMenuPrivate::appendParentMineType(const QStringList &parentmimeTypes, QStringList &mimeTypes) const
{
    if (parentmimeTypes.isEmpty())
        return;

    DFMBASE_NAMESPACE::DMimeDatabase db;
    QSet<QString> mimeTypeNames;
    QStringList allparentmimeTypes = parentmimeTypes;
    int count = 0;
    while (!allparentmimeTypes.isEmpty()) {
        if (count > 10000)   // 预防死循环
            break;
        const QString &mtName = allparentmimeTypes.takeFirst();
        if (mimeTypeNames.contains(mtName))
            continue;
        mimeTypeNames.insert(mtName);
        count++;
        QMimeType mt = db.mimeTypeForName(mtName);
        mimeTypes.append(mt.name());
        mimeTypes.append(mt.aliases());
        QStringList parentMimeTypes = mt.parentMimeTypes();

        for (const auto &type : parentMimeTypes) {
            if (mimeTypeNames.contains(type))
                continue;
            allparentmimeTypes.push_back(type);
        }
    }
}

OemMenu::OemMenu(QObject *parent)
    : QObject(parent), d(new OemMenuPrivate(this))
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
}

OemMenu::~OemMenu()
{
}

void OemMenu::loadDesktopFile()
{
    d->menuActionHolder.reset(new QObject(this));
    d->actionListByType.clear();
    d->clearSubMenus();

    for (auto path : d->oemMenuPath) {
        QDir oemDir(path);
        if (!oemDir.exists())
            continue;

        for (const QFileInfo &fileInfo : oemDir.entryInfoList({ "*.desktop" })) {
            DesktopFile df(fileInfo.absoluteFilePath());
            if (df.isNoShow())
                continue;

            DDesktopEntry entry(fileInfo.absoluteFilePath());
            QStringList &&menuTypes = d->getValues(entry, kMenuTypeKey, kMenuTypeAliasKey, kDesktopEntryGroup, d->menuTypes);

            menuTypes.removeAll("");
            if (menuTypes.isEmpty()) {
                fmDebug() << "[OEM Menu Support] Entry will probably not be shown due to empty or have no valid"
                          << kMenuTypeKey << " and " << kMenuTypeAliasKey << "key in the desktop file.";
                fmDebug() << "[OEM Menu Support] Details:" << fileInfo.filePath() << "with entry name" << entry.localizedValue(kNameKey, kLocaleKey, kDesktopEntryGroup);
                continue;
            }

            QString &&iconStr = entry.localizedValue(kIconKey, kLocaleKey, kDesktopEntryGroup);
            QString &&nameStr = entry.localizedValue(kNameKey, kLocaleKey, kDesktopEntryGroup);
            QAction *action = new QAction(QIcon::fromTheme(iconStr), nameStr, d->menuActionHolder.data());

            for (auto propery : d->actionProperties) {
                d->setActionProperty(action, entry, propery, kDesktopEntryGroup);
            }

            for (const QString &type : menuTypes) {
                d->actionListByType[type].append(action);
            }

            // sub action
            QStringList &&entryActions = entry.stringListValue(kActionsKey, kDesktopEntryGroup);
            entryActions.removeAll("");
            if (!entryActions.isEmpty()) {

                QMenu *menu = new QMenu();
                d->subMenus.append(menu);

                for (const QString &actionName : entryActions) {
                    QString subGroupName(kActionsGroup + actionName);
                    QString &&subActionIconStr = entry.localizedValue(kIconKey, kLocaleKey, subGroupName);
                    QString &&subActionNameStr = entry.localizedValue(kNameKey, kLocaleKey, subGroupName);
                    QAction *subAction = new QAction(QIcon(subActionIconStr), subActionNameStr, d->menuActionHolder.data());

                    QString &&cmd = entry.stringValue(kCommandKey, subGroupName);
                    subAction->setProperty(kCommandKey, cmd);

                    menu->addAction(subAction);
                }

                action->setMenu(menu);
            }
        }
    }
}

QList<QAction *> OemMenu::emptyActions(const QUrl &currentDir, bool onDesktop)
{
    QList<QAction *> actions = d->actionListByType[kEmptyArea];
    auto fileInfo = InfoFactory::create<FileInfo>(currentDir);

    auto it = actions.begin();
    while (it != actions.end()) {
        QAction *action = *it;
        if (!d->isValid(action, fileInfo, onDesktop)) {
            it = actions.erase(it);
            continue;
        }

        ++it;
    }

    return actions;
}

QList<QAction *> OemMenu::normalActions(const QList<QUrl> &files, bool onDesktop)
{
    QString menuType;

    QString errString;
    if (1 == files.count()) {
        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(files.first(), Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (!fileInfo) {
            fmDebug() << errString;
            return {};
        }

        menuType = fileInfo->isAttributes(OptInfoType::kIsDir) ? kSingleDir : kSingleFile;
    } else {
        menuType = kMultiFileDirs;
    }

    QList<QAction *> actions = d->actionListByType[menuType];
    if (actions.isEmpty())
        return actions;

    QStringList filePaths;
    bool bex7z = d->isAllEx7zFile(files);
    for (const QUrl &file : files) {

        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(file, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);

        if (!fileInfo) {
            fmWarning() << "createFileInfo failed: " << file;
            continue;
        }
        filePaths << file.path();

        QStringList fileMimeTypes, fmts;
        fileMimeTypes.append(fileInfo->fileMimeType().name());
        fileMimeTypes.append(fileInfo->fileMimeType().aliases());
        const QMimeType &mt = fileInfo->fileMimeType();
        fmts = fileMimeTypes;
        d->appendParentMineType(mt.parentMimeTypes(), fileMimeTypes);
        fileMimeTypes.removeAll({});
        fmts.removeAll({});

        for (auto it = actions.begin(); it != actions.end();) {
            QAction *action = *it;
            if (!d->isValid(action, fileInfo, onDesktop, bex7z)) {
                it = actions.erase(it);
                continue;
            }

            // compression is not supported on FTP
            if (action->text() == QObject::tr("Compress") && ProtocolUtils::isFTPFile(file)) {
                it = actions.erase(it);
                continue;
            }

            // match exclude mime types
            QStringList excludeMimeTypes = action->property(kMimeTypeExcludeKey).toStringList();
            excludeMimeTypes << action->property(kMimeTypeExcludeAliasKey).toStringList();

            excludeMimeTypes.removeAll({});
            // e.g. xlsx parentMimeTypes is application/zip
            bool match = d->isMimeTypeMatch(fmts, excludeMimeTypes);
            if (match) {
                it = actions.erase(it);
                continue;
            }

            // MimeType not exist == MimeType=*
            if (!action->property(kMimeType).isValid()) {
                ++it;
                continue;
            }

            // match support mime types
            QStringList supportMimeTypes = action->property(kMimeType).toStringList();
            supportMimeTypes.removeAll({});
            match = d->isMimeTypeMatch(fileMimeTypes, supportMimeTypes);

            // The file attributes of some MTP mounted device directories do not meet the specifications
            //(the ordinary directory mimeType is considered octet stream), so special treatment is required
            if (file.path().contains("/mtp:host") && supportMimeTypes.contains("application/octet-stream") && fileMimeTypes.contains("application/octet-stream")) {
                match = false;
            }

            if (!match) {
                it = actions.erase(it);
                continue;
            }

            ++it;
        }
    }

    return actions;
}

QList<QAction *> OemMenu::focusNormalActions(const QUrl &foucs, const QList<QUrl> &files, bool onDesktop)
{
    QList<QAction *> actions;

    QString errString;
    auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(foucs, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (!fileInfo) {
        fmWarning() << errString;
        return actions;
    }

    QString menuType;
    if (1 == files.count())
        menuType = fileInfo->isAttributes(OptInfoType::kIsDir) ? kSingleDir : kSingleFile;
    else
        menuType = kMultiFileDirs;

    // get actions surported menutype
    actions = d->actionListByType[menuType];
    if (actions.isEmpty())
        return actions;

    // get foucs file mimetype
    QStringList mimeTypes;
    QStringList siblingMimeTypes;
    {
        // get normal mimetype
        mimeTypes.append(fileInfo->fileMimeType().name());
        mimeTypes.append(fileInfo->fileMimeType().aliases());
        mimeTypes.removeAll(QString(""));

        // get parent mimetype
        const QMimeType &mt = fileInfo->fileMimeType();
        siblingMimeTypes = mimeTypes;
        d->appendParentMineType(mt.parentMimeTypes(), siblingMimeTypes);
        siblingMimeTypes.removeAll(QString(""));
    }

    // check each actons
    for (auto it = actions.begin(); it != actions.end();) {
        QAction *action = *it;
        // check Desktop, Scheme, Suffix
        if (!d->isValid(action, fileInfo, onDesktop, false)) {
            it = actions.erase(it);
            continue;
        }

        // compression is not supported on FTP
        if (action->text() == QObject::tr("Compress") && ProtocolUtils::isFTPFile(foucs)) {
            it = actions.erase(it);
            continue;
        }

        // match exclude mime types
        QStringList excludeMimeTypes = action->property(kMimeTypeExcludeKey).toStringList();
        excludeMimeTypes << action->property(kMimeTypeExcludeAliasKey).toStringList();

        excludeMimeTypes.removeAll({});
        // e.g. xlsx parentMimeTypes is application/zip
        bool match = d->isMimeTypeMatch(mimeTypes, excludeMimeTypes);
        if (match) {
            it = actions.erase(it);
            continue;
        }

        // MimeType not exist == MimeType=*
        if (!action->property(kMimeType).isValid()) {
            ++it;
            continue;
        }

        // match support mime types
        QStringList supportMimeTypes = action->property(kMimeType).toStringList();
        supportMimeTypes.removeAll({});
        match = d->isMimeTypeMatch(siblingMimeTypes, supportMimeTypes);

        // The file attributes of some MTP mounted device directories do not meet the specifications
        //(the ordinary directory mimeType is considered octet stream), so special treatment is required
        if (foucs.path().contains("/mtp:host") && supportMimeTypes.contains("application/octet-stream")
            && siblingMimeTypes.contains("application/octet-stream")) {
            match = false;
        }

        if (!match) {
            it = actions.erase(it);
            continue;
        }

        ++it;
    }

    return actions;
}

QPair<QString, QStringList> OemMenu::makeCommand(const QAction *action, const QUrl &dir, const QUrl &foucs, const QList<QUrl> &files)
{
    QPair<QString, QStringList> ret;
    if (Q_UNLIKELY(!action))
        return ret;

    QString cmd = action->property(kCommandKey).toString();
    if (Q_UNLIKELY(cmd.isEmpty()))
        return ret;

    auto args = d->splitCommand(cmd);
    if (args.isEmpty())
        return ret;

    // execution procedure
    ret.first = args.takeFirst();
    // no parameters
    if (args.isEmpty())
        return ret;

    auto type = d->execDynamicArg(cmd);

    // args
    switch (type) {
    case OemMenuPrivate::kDirPath:
        ret.second = d->replace(args, kCommandArg[type], dir.toLocalFile());
        break;
    case OemMenuPrivate::kFilePath:
        ret.second = d->replace(args, kCommandArg[type], foucs.toLocalFile());
        break;
    case OemMenuPrivate::kFilePaths:
        ret.second = d->replaceList(args, kCommandArg[type], d->urlListToLocalFile(files));
        break;
    case OemMenuPrivate::kUrlPath:
        ret.second = d->replace(args, kCommandArg[type], d->urlToString(foucs));
        break;
    case OemMenuPrivate::kUrlPaths:
        ret.second = d->replaceList(args, kCommandArg[type], d->urlListToString(files));
        break;
    default:
        ret.second = args;
        break;
    }
    return ret;
}
