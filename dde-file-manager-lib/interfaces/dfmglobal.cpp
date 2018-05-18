/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dfmglobal.h"
#include "chinese2pinyin.h"
#include "dfmstandardpaths.h"
#include "dfileservices.h"
#include "singleton.h"
#include "dialogs/dialogmanager.h"
#include "app/define.h"
#include "plugins/pluginmanager.h"
#include "app/filesignalmanager.h"
#include "shutil/mimesappsmanager.h"
#include "controllers/searchhistroymanager.h"
#include "controllers/bookmarkmanager.h"
#include "interfaces/dfilemenumanager.h"
#include "controllers/pathmanager.h"
#include "gvfs/gvfsmountclient.h"
#include "gvfs/gvfsmountmanager.h"
#include "gvfs/networkmanager.h"
#include "gvfs/secretmanager.h"
#include "controllers/appcontroller.h"
#include "../deviceinfo/udisklistener.h"
#include "../usershare/usersharemanager.h"
#include "dfmsetting.h"
#include "models/desktopfileinfo.h"
#include "shutil/viewstatesmanager.h"
#include "dfmplaformmanager.h"
#include "controllers/operatorrevocation.h"
#include "tag/tagmanager.h"

#include "dabstractfilewatcher.h"
#include <dfmstandardpaths.h>

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QTranslator>
#include <QTimer>
#include <QThread>
#include <QtConcurrent>
#include <QTextCodec>
#include <QLocale>
#include <QMediaPlayer>
#include <QDBusObjectPath>
#include <QGSettings>

#include <private/qtextengine_p.h>

#include <cstdio>
#include <locale>
#include <sstream>
#include <fstream>

#include <KCodecs>
#include <KEncodingProber>

#include <QMimeType>
#include <QMimeDatabase>


#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include <iconv.h>

#ifdef __cplusplus
}
#endif //__cplusplus

namespace GlobalData {
static QList<QUrl> clipboardFileUrls;
static DFMGlobal::ClipboardAction clipboardAction = DFMGlobal::UnknowAction;

void onClipboardDataChanged()
{
    clipboardFileUrls.clear();
    const QByteArray &data = qApp->clipboard()->mimeData()->data("x-special/gnome-copied-files");

    if (data.startsWith("cut")) {
        clipboardAction = DFMGlobal::CutAction;
    } else if (data.startsWith("copy")) {
        clipboardAction = DFMGlobal::CopyAction;
    } else {
        clipboardAction = DFMGlobal::UnknowAction;
    }

    for (QUrl url : qApp->clipboard()->mimeData()->urls()) {
        if (url.scheme().isEmpty()){
            url.setScheme("file");
        }
        clipboardFileUrls << url;
    }
}

class DFMGlobalPrivate : public DFMGlobal {};
Q_GLOBAL_STATIC(DFMGlobalPrivate, dfmGlobal)
}

QStringList DFMGlobal::PluginLibraryPaths;
QStringList DFMGlobal::MenuExtensionPaths;
bool DFMGlobal::IsFileManagerDiloagProcess = false;

DFMGlobal *DFMGlobal::instance()
{
    return GlobalData::dfmGlobal;
}

QString DFMGlobal::organizationName()
{
    return QMAKE_ORGANIZATION_NAME;
}

QString DFMGlobal::applicationName()
{
    return QMAKE_TARGET;
}

QString DFMGlobal::applicationVersion()
{
    return QMAKE_VERSION;
}

bool DFMGlobal::installTranslator()
{
    QTranslator *translator = new QTranslator(QGuiApplication::instance());

    QString transLatorPath = DFMStandardPaths::standardLocation(DFMStandardPaths::TranslationPath) +
            QDir::separator() + DFMGlobal::applicationName() + "_" + QLocale::system().name();

    if (translator->load(transLatorPath)) {
        return qApp->installTranslator(translator);
    }

    return false;
}

void DFMGlobal::setUrlsToClipboard(const QList<QUrl> &list, DFMGlobal::ClipboardAction action)
{
    if (action == UnknowAction)
        return;

    QMimeData *mimeData = new QMimeData;

    QByteArray ba = (action == DFMGlobal::CutAction) ? "cut" : "copy";
    QString text;

    for(const QUrl &url : list) {
        ba.append("\n");
        ba.append(url.toString());

        const QString &path = url.toLocalFile();

        if (!path.isEmpty()) {
            text += path + '\n';
        }
    }

    mimeData->setText(text.endsWith('\n') ? text.left(text.length() - 1) : text);
    mimeData->setData("x-special/gnome-copied-files", ba);
    mimeData->setUrls(list);

    qApp->clipboard()->setMimeData(mimeData);
}

void DFMGlobal::clearClipboard()
{
    qApp->clipboard()->setText(QString());
}

void DFMGlobal::clearTrash()
{
    appController->actionClearTrash();
}

void DFMGlobal::addPluginLibraryPath(const QString &path)
{
    PluginLibraryPaths.append(path);
    refreshPlugins();
}

void DFMGlobal::addPluginLibraryPaths(const QStringList &paths)
{
    foreach (QString path, paths) {
        PluginLibraryPaths.append(path);
    }
    refreshPlugins();
}

void DFMGlobal::addMenuExtensionPath(const QString &path)
{
    MenuExtensionPaths.append(path);
}

void DFMGlobal::addMenuExtensionPaths(const QStringList &paths)
{
    foreach (QString path, paths) {
        MenuExtensionPaths.append(path);
    }
}

void DFMGlobal::autoLoadDefaultPlugins()
{
    addPluginLibraryPath(PluginManager::PluginDir());
}

void DFMGlobal::autoLoadDefaultMenuExtensions()
{
    QString configPath = DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationConfigPath);
    QString menuExtensionPath = QString("%1/%2").arg(configPath, "menuextensions");
    DFMGlobal::addMenuExtensionPath(menuExtensionPath);
}

void DFMGlobal::initPluginManager()
{
    refreshPlugins();
}

void DFMGlobal::initSearchHistoryManager()
{
    searchHistoryManager;
}

void DFMGlobal::initBookmarkManager()
{
    bookmarkManager;
}

void DFMGlobal::initFileMenuManager()
{
    fileMenuManger;
}

void DFMGlobal::initFileSiganlManager()
{
    fileSignalManager;
}

void DFMGlobal::initMimesAppsManager()
{
    mimeAppsManager;
    QTimer::singleShot(1000, [](){
        mimeAppsManager->requestUpdateCache();
    });
}

void DFMGlobal::initSystemPathManager()
{
    systemPathManager;
}

void DFMGlobal::initMimeTypeDisplayManager()
{
    mimeAppsManager;
}

void DFMGlobal::initNetworkManager()
{
    networkManager;
}

void DFMGlobal::initGvfsMountClient()
{
    gvfsMountClient;
}

void DFMGlobal::initGvfsMountManager()
{
    gvfsMountManager;
    QtConcurrent::run(QThreadPool::globalInstance(), gvfsMountManager,
                                             &GvfsMountManager::startMonitor);
}

void DFMGlobal::initSecretManager()
{
    secretManager;
}

void DFMGlobal::initDialogManager()
{
    dialogManager;
}

void DFMGlobal::initFileService()
{
    fileService;
}

void DFMGlobal::initAppcontroller()
{
    appController;
}

void DFMGlobal::initDeviceListener()
{
    deviceListener;
}

void DFMGlobal::initUserShareManager()
{
    userShareManager;
}

void DFMGlobal::initGlobalSettings()
{
    globalSetting;
}

void DFMGlobal::initViewStatesManager()
{
    viewStatesManager;
}

void DFMGlobal::initOperatorRevocation()
{
    Q_UNUSED(OperatorRevocation::instance())
}

void DFMGlobal::initTagManagerConnect()
{
    connect(TagManager::instance(), static_cast<void(TagManager::*)(const QMap<QString, QString>&)>(&TagManager::changeTagColor),
            [] (const QMap<QString, QString>& tag_and_new_color) {
        for (auto i = tag_and_new_color.constBegin(); i != tag_and_new_color.constEnd(); ++i) {
            const QString &tag_name = i.key();
            const QStringList &files = TagManager::instance()->getFilesThroughTag(tag_name);

            for (const QString &file : files) {
                DUrl url = DUrl::fromLocalFile(file);
                DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url);
            }
        }
    });
    connect(TagManager::instance(), &TagManager::filesWereTagged, [] (const QMap<QString, QList<QString>>& files_were_tagged) {
        for (auto i = files_were_tagged.constBegin(); i != files_were_tagged.constEnd(); ++i) {
            // is trash files
            if (i.key().startsWith(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath)))
                return;

            DUrl url = DUrl::fromLocalFile(i.key());
            DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url);

            // for tag watcher
            for (const QString &tag : i.value()) {
                const DUrl &parent_url = DUrl::fromUserTaggedFile(tag, QString());
                const DUrl &file_url = DUrl::fromUserTaggedFile(tag, i.key());

                DAbstractFileWatcher::ghostSignal(parent_url, &DAbstractFileWatcher::subfileCreated, file_url);
            }
        }
    });
    connect(TagManager::instance(), &TagManager::untagFiles, [] (const QMap<QString, QList<QString>>& tag_be_removed_files) {
        for (auto i = tag_be_removed_files.constBegin(); i != tag_be_removed_files.constEnd(); ++i) {
            DUrl url = DUrl::fromLocalFile(i.key());
            DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url);

            // for tag watcher
            for (const QString &tag : i.value()) {
                const DUrl &parent_url = DUrl::fromUserTaggedFile(tag, QString());
                const DUrl &file_url = DUrl::fromUserTaggedFile(tag, i.key());

                DAbstractFileWatcher::ghostSignal(parent_url, &DAbstractFileWatcher::fileDeleted, file_url);
            }
        }
    });

    // for tag watcher
    connect(TagManager::instance(), &TagManager::addNewTag, [] (const QList<QString>& new_tags) {
        for (const QString &tag : new_tags) {
            DAbstractFileWatcher::ghostSignal(DUrl(TAG_ROOT), &DAbstractFileWatcher::subfileCreated, DUrl::fromUserTaggedFile(tag, QString()));
        }
    });
    connect(TagManager::instance(), &TagManager::deleteTag, [] (const QList<QString>& new_tags) {
        for (const QString &tag : new_tags) {
            DAbstractFileWatcher::ghostSignal(DUrl(TAG_ROOT), &DAbstractFileWatcher::fileDeleted, DUrl::fromUserTaggedFile(tag, QString()));
        }
    });
    connect(TagManager::instance(), static_cast<void(TagManager::*)(const QMap<QString, QString>&)>(&TagManager::changeTagName),
            [] (const QMap<QString, QString>& old_and_new_name) {
        for (auto i = old_and_new_name.constBegin(); i != old_and_new_name.constEnd(); ++i) {
            const DUrl &old_url = DUrl::fromUserTaggedFile(i.key(), QString());
            const DUrl &new_url = DUrl::fromUserTaggedFile(i.value(), QString());

            DAbstractFileWatcher::ghostSignal(DUrl(TAG_ROOT), &DAbstractFileWatcher::fileMoved, old_url, new_url);
        }
    });
}

void DFMGlobal::autoMountAllDisks()
{
    TIMER_SINGLESHOT(1000, {
        gvfsMountManager->autoMountAllDisks();
    })
}

QString DFMGlobal::getUser()
{
    static QString user = QString::fromLocal8Bit(qgetenv("USER"));

    return user;
}

int DFMGlobal::getUserId()
{
    return getuid();
}

bool DFMGlobal::isRootUser()
{
    return getUserId() == 0;
}

QList<QUrl> DFMGlobal::clipboardFileUrlList() const
{
    return GlobalData::clipboardFileUrls;
}

DFMGlobal::ClipboardAction DFMGlobal::clipboardAction() const
{
    return GlobalData::clipboardAction;
}

QIcon DFMGlobal::standardIcon(DFMGlobal::Icon iconType) const
{
    static QIcon linkIcon(":/images/images/link_large.png");
    static QIcon lockIcon (":/images/images/lock_large.png");
    static QIcon unreadableIcon(":/images/images/unreadable_large.png");
    static QIcon shareIcon(":/images/images/share_large.png");

    switch (iconType) {
    case LinkIcon:
            return linkIcon;
    case LockIcon:
        return lockIcon;
    case UnreadableIcon:
        return unreadableIcon;
    case ShareIcon:
        return shareIcon;
    }

    return QIcon();
}

QString DFMGlobal::wordWrapText(const QString &text, qreal width, QTextOption::WrapMode wrapMode,
                                const QFont &font, qreal lineHeight, qreal *height)
{
    QTextLayout layout(text);

    layout.setFont(font);

    QStringList lines;

    wordWrapText(&layout, width, wrapMode, lineHeight, &lines);

    if (height)
        *height = lines.count() * lineHeight;

    return lines.join('\n');
}

DFMGlobal::DFMGlobal()
{
    connect(qApp->clipboard(), &QClipboard::dataChanged, this, &DFMGlobal::onClipboardDataChanged);
    GlobalData::onClipboardDataChanged();
}

void DFMGlobal::onClipboardDataChanged()
{
    GlobalData::onClipboardDataChanged();

    emit clipboardDataChanged();
}

void DFMGlobal::refreshPlugins()
{
    PluginManager::instance()->loadPlugin();
}

void DFMGlobal::wordWrapText(QTextLayout *layout, qreal width, QTextOption::WrapMode wrapMode,
                             qreal lineHeight, QStringList *lines)
{
    elideText(layout, QSize(width, INT_MAX), wrapMode, Qt::ElideNone, lineHeight, 0, lines);
}

void DFMGlobal::elideText(QTextLayout *layout, const QSizeF &size, QTextOption::WrapMode wordWrap,
                          Qt::TextElideMode mode, qreal lineHeight, int flags, QStringList *lines,
                          QPainter *painter, QPointF offset, const QColor &shadowColor, const QPointF &shadowOffset,
                          const QBrush &background, qreal backgroundRadius, QList<QRectF> *boundingRegion)
{
    qreal height = 0;
    bool drawBackground = background.style() != Qt::NoBrush;
    bool drawShadow = shadowColor.isValid();

    QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
    QTextOption &text_option = *const_cast<QTextOption*>(&layout->textOption());

    text_option.setWrapMode(wordWrap);

    if (flags & Qt::AlignRight)
        text_option.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        text_option.setAlignment(Qt::AlignHCenter);

    if (painter) {
        text_option.setTextDirection(painter->layoutDirection());
        layout->setFont(painter->font());
    } else {
        // dont paint
        layout->engine()->ignoreBidi = true;
    }

    auto naturalTextRect = [&] (const QRectF rect) {
        QRectF new_rect = rect;

        new_rect.setHeight(lineHeight);

        return new_rect;
    };

    auto drawShadowFun = [&] (const QTextLine &line) {
        const QPen pen = painter->pen();

        painter->setPen(shadowColor);
        line.draw(painter, shadowOffset);

        // restore
        painter->setPen(pen);
    };

    layout->beginLayout();

    QTextLine line = layout->createLine();
    QRectF lastLineRect;

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > size.height()) {
            const QString &end_str = layout->engine()->elidedText(mode, qRound(size.width() - 1), flags, line.textStart());

            layout->endLayout();
            layout->setText(end_str);

            if (layout->engine()->block.docHandle()) {
                const_cast<QTextDocument*>(layout->engine()->block.document())->setPlainText(end_str);
            }

            text_option.setWrapMode(QTextOption::NoWrap);
            layout->beginLayout();
            line = layout->createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        line.setPosition(offset);

        const QRectF rect = naturalTextRect(line.naturalTextRect());

        if (painter) {
            if (drawBackground) {
                const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
                QRectF backBounding = rect;
                QPainterPath path;

                if (lastLineRect.isValid()) {
                    if (qAbs(rect.width() - lastLineRect.width()) < backgroundRadius * 2) {
                        backBounding.setWidth(lastLineRect.width());
                        backBounding.moveCenter(rect.center());
                        path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                        path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                        path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
                        path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
                        path.lineTo(backBounding.x(), backBounding.bottom());
                        path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
                        lastLineRect = backBounding;
                    } else if (lastLineRect.width() > rect.width()) {
                        backBounding += margins;
                        path.moveTo(backBounding.x() - backgroundRadius, backBounding.y() - 1);
                        path.arcTo(backBounding.x() - backgroundRadius * 2, backBounding.y() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 90, -90);
                        path.lineTo(backBounding.x(), backBounding.bottom() - backgroundRadius);
                        path.arcTo(backBounding.x(), backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
                        path.lineTo(backBounding.right() - backgroundRadius, backBounding.bottom());
                        path.arcTo(backBounding.right() - backgroundRadius * 2, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                        path.lineTo(backBounding.right(), backBounding.top() + backgroundRadius);
                        path.arcTo(backBounding.right(), backBounding.top() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 180, -90);
                        path.closeSubpath();
                        lastLineRect = rect;
                    } else {
                        backBounding += margins;
                        path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
                        path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                        path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
                        path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);

//                        path.arcTo(lastLineRect.x() - backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 180, 90);
//                        path.lineTo(lastLineRect.x() - backgroundReaius * 3, lastLineRect.bottom());
//                        path.moveTo(lastLineRect.right(), lastLineRect.bottom());
//                        path.arcTo(lastLineRect.right() - backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 270, 90);
//                        path.arcTo(lastLineRect.right() + backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 180, 90);
//                        path.lineTo(lastLineRect.right(), lastLineRect.bottom());

                        path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
                        lastLineRect = rect;
                    }
                } else {
                    lastLineRect = backBounding;
                    path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
                }

                bool a = painter->testRenderHint(QPainter::Antialiasing);
                qreal o = painter->opacity();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setOpacity(1);
                painter->fillPath(path, background);
                painter->setRenderHint(QPainter::Antialiasing, a);
                painter->setOpacity(o);
            }

            if (drawShadow) {
                drawShadowFun(line);
            }

            line.draw(painter, QPointF(0, 0));
        }

        if (boundingRegion) {
            boundingRegion->append(rect);
        }

        offset.setY(offset.y() + lineHeight);

//        // find '\n'
//        int text_length_line = line.textLength();
//        for (int start = line.textStart(); start < line.textStart() + text_length_line; ++start) {
//            if (text.at(start) == '\n')
//                height += lineHeight;
//        }

        if (lines) {
            lines->append(text.mid(line.textStart(), line.textLength()));
        }

        if (height + lineHeight > size.height())
            break;

        line = layout->createLine();
    }

    layout->endLayout();
}

QString DFMGlobal::elideText(const QString &text, const QSizeF &size,
                             QTextOption::WrapMode wordWrap, const QFont &font,
                             Qt::TextElideMode mode, qreal lineHeight, qreal flags)
{
    QTextLayout textLayout(text);

    textLayout.setFont(font);

    QStringList lines;

    elideText(&textLayout, size, wordWrap, mode, lineHeight, flags, &lines);

    return lines.join('\n');
}

QString DFMGlobal::toPinyin(const QString &text)
{
    return Pinyin::Chinese2Pinyin(text);
}

bool DFMGlobal::startWithHanzi(const QString &text)
{
    if (text.isEmpty())
        return false;

    return text.at(0).script() == QChar::Script_Han;
}

static QString textDecoder(const QByteArray &ba, const QByteArray &codecName)
{
    QTextDecoder decoder(QTextCodec::codecForName(codecName));

    const QString &text = decoder.toUnicode(ba);

    return decoder.hasFailure() ? QString() : text;
}

///###: forward-declare.
static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country);

QByteArray DFMGlobal::detectCharset(const QByteArray &data, const QString &fileName)
{
    // Return local encoding if nothing in file.
    if (data.isEmpty()) {
        return QTextCodec::codecForLocale()->name();
    }

    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        return c->name();
    }

    QMimeDatabase mime_database;
    const QMimeType &mime_type = fileName.isEmpty() ? mime_database.mimeTypeForData(data) : mime_database.mimeTypeForFileNameAndData(fileName, data);
    const QString &mimetype_name = mime_type.name();
    KEncodingProber::ProberType proberType = KEncodingProber::Universal;

    if (mimetype_name == QStringLiteral("application/xml")
            || mimetype_name == QStringLiteral("text/html")
            || mimetype_name == QStringLiteral("application/xhtml+xml")) {
        const QString &_data = QString::fromLatin1(data);
        QRegularExpression pattern("<\\bmeta.+\\bcharset=(?'charset'\\S+?)\\s*['\"/>]");

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        const QString &charset = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                               QRegularExpression::DontCheckSubjectStringMatchOption).captured("charset");

        if (!charset.isEmpty()) {
            return charset.toLatin1();
        }

        pattern.setPattern("<\\bmeta\\s+http-equiv=\"Content-Language\"\\s+content=\"(?'language'[a-zA-Z-]+)\"");

        const QString &language = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                                QRegularExpression::DontCheckSubjectStringMatchOption).captured("language");

        if (!language.isEmpty()) {
            QLocale l(language);

            switch (l.script()) {
            case QLocale::ArabicScript:
                proberType = KEncodingProber::Arabic;
                break;
            case QLocale::SimplifiedChineseScript:
                proberType = KEncodingProber::ChineseSimplified;
                break;
            case QLocale::TraditionalChineseScript:
                proberType = KEncodingProber::ChineseTraditional;
                break;
            case QLocale::CyrillicScript:
                proberType = KEncodingProber::Cyrillic;
                break;
            case QLocale::GreekScript:
                proberType = KEncodingProber::Greek;
                break;
            case QLocale::HebrewScript:
                proberType = KEncodingProber::Hebrew;
                break;
            case QLocale::JapaneseScript:
                proberType = KEncodingProber::Japanese;
                break;
            case QLocale::KoreanScript:
                proberType = KEncodingProber::Korean;
                break;
            case QLocale::ThaiScript:
                proberType = KEncodingProber::Thai;
                break;
            default:
                break;
            }
        }
    } else if (mimetype_name == "text/x-python") {
        QRegularExpression pattern("^#coding\\s*:\\s*(?'coding'\\S+)$");
        QTextStream stream(data);

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        stream.setCodec("latin1");

        while (!stream.atEnd()) {
            const QString &_data = stream.readLine();
            const QString &coding = pattern.match(_data, 0).captured("coding");

            if (!coding.isEmpty()) {
                return coding.toLatin1();
            }
        }
    }

    // for CJK
    const QList<QPair<KEncodingProber::ProberType, QLocale::Country>> fallback_list {
        {KEncodingProber::ChineseSimplified, QLocale::China},
        {KEncodingProber::ChineseTraditional, QLocale::China},
        {KEncodingProber::Japanese, QLocale::Japan},
        {KEncodingProber::Korean, QLocale::NorthKorea},
        {proberType, QLocale::system().country()}
    };

    KEncodingProber prober(proberType);
    QTextCodec *def_codec = QTextCodec::codecForLocale();
    QByteArray encoding;
    float confidence = 0;

    for (const auto i : fallback_list) {
        prober.setProberType(i.first);
        prober.feed(data);

        if (prober.confidence() == 0)
            continue;

        if (QTextCodec *codec = QTextCodec::codecForName(prober.encoding())) {
            if (def_codec == codec)
                def_codec = nullptr;

            float c = codecConfidenceForData(codec, data, i.second);

            if (prober.confidence() > 0.5) {
                c = c / 2 + prober.confidence() / 2;
            } else {
                c = c / 3 * 2 + prober.confidence() / 3;
            }

            if (c > confidence) {
                confidence = c;
                encoding = prober.encoding();
            }

            if (i.first == KEncodingProber::ChineseTraditional && c < 0.5) {
                // test Big5
                c = codecConfidenceForData(QTextCodec::codecForName("Big5"), data, QLocale::China);

                if (c > 0.5 && c > confidence) {
                    confidence = c;
                    encoding = "Big5";
                }
            }
        }
    }

    if (def_codec && codecConfidenceForData(def_codec, data, QLocale::system().country()) > confidence) {
        return def_codec->name();
    }

    return encoding;
}




bool DFMGlobal::keyShiftIsPressed()
{
    return qApp->keyboardModifiers() == Qt::ShiftModifier;
}

bool DFMGlobal::keyCtrlIsPressed()
{
    return qApp->keyboardModifiers() == Qt::ControlModifier;
}

bool DFMGlobal::fileNameCorrection(const QString &filePath)
{
    QFileInfo info(filePath);
    QProcess ls;

    ls.start("ls", QStringList() << "-1" << "--color=never" << info.absolutePath());
    ls.waitForFinished();

    const QByteArray &request = ls.readAllStandardOutput();

    for (const QByteArray &name : request.split('\n')) {
        QString strFileName{ DFMGlobal::toUnicode(name) };

        if (strFileName == info.fileName() && strFileName.toLocal8Bit() != name) {
            const QByteArray &path = info.absolutePath().toLocal8Bit() + QDir::separator().toLatin1() + name;

            return fileNameCorrection(path);
        }
    }

    return false;
}

bool DFMGlobal::fileNameCorrection(const QByteArray &filePath)
{
    const QByteArray &newFilePath = QString::fromLocal8Bit(filePath).toLocal8Bit();

    if (filePath == newFilePath)
        return true;

    return std::rename(filePath.constData(), newFilePath.constData());
}

bool DFMGlobal::isDesktopFile(const DUrl &url)
{
    return url.toLocalFile().endsWith(".desktop");
}

bool DFMGlobal::isTrashDesktopFile(const DUrl &url)
{
    if(isDesktopFile(url)){
        DesktopFile df(url.toLocalFile());
        return df.getDeepinId() == DDE_TRASH_ID;
    }
    return false;
}

bool DFMGlobal::isComputerDesktopFile(const DUrl &url)
{
    if(isDesktopFile(url)){
        DesktopFile df(url.toLocalFile());
        return df.getDeepinId() == DDE_COMPUTER_ID;
    }
    return false;
}

bool DFMGlobal::isTrashDesktopFileUrl(const DUrl &url)
{
    if (DesktopFileInfo::trashDesktopFileUrl() == url)
        return isTrashDesktopFile(url);
    return false;
}

bool DFMGlobal::isComputerDesktopFileUrl(const DUrl &url)
{
    if (DesktopFileInfo::computerDesktopFileUrl() == url)
        return isComputerDesktopFile(url);
    return false;
}

bool DFMGlobal::isDisableUnmount(const QDiskInfo &diskInfo)
{
    if (diskInfo.type() == "native"){
        if (dfmPlatformManager->isDisableUnMount()){
            return true;
        }
    }
    return false;
}

void DFMGlobal::showMultiFilesRenameDialog(const QList<DUrl> &selectedFiles)
{
    dialogManager->showMultiFilesRenameDialog( selectedFiles );
}

void DFMGlobal::showFilePreviewDialog(const DUrlList &selectUrls, const DUrlList &entryUrls)
{
    emit fileSignalManager->requestShowFilePreviewDialog(selectUrls, entryUrls);
}

void DFMGlobal::showPropertyDialog(QObject * const sender, const QList<DUrl>& selectedFiles)
{
    dialogManager->showPropertyDialog(DFMUrlListBaseEvent{sender, selectedFiles});
}

void DFMGlobal::playSound(const QUrl &soundUrl)
{
    QGSettings settings("com.deepin.dde.sound-effect", "/com/deepin/dde/sound-effect/");
    if(!settings.get("enabled").toBool())
        return;

    //check if is global sound off
    QDBusInterface audioIface("com.deepin.daemon.Audio",
                              "/com/deepin/daemon/Audio",
                              "com.deepin.daemon.Audio",
                              QDBusConnection::sessionBus());

    QString defaultSink = qvariant_cast<QDBusObjectPath>(audioIface.property("DefaultSink")).path();

    QDBusInterface audioSinkIface("com.deepin.daemon.Audio",
                                  defaultSink,
                                  "com.deepin.daemon.Audio.Sink",
                                  QDBusConnection::sessionBus());
    bool isGlobalSoundDisabled = audioSinkIface.property("Mute").toBool();

    if(isGlobalSoundDisabled)
        return;

    QMediaPlayer* player = new QMediaPlayer;
    player->setMedia(soundUrl);
    player->setVolume(100);
    player->play();
    connect(player, &QMediaPlayer::positionChanged, [=](const qint64& position){
        if(position >= player->duration()){
            player->deleteLater();
        }
    });
}



QString DFMGlobal::toUnicode(const QByteArray &data, const QString &fileName)
{
    if (data.isEmpty())
        return QString();

    const QByteArray &encoding = detectCharset(data, fileName);

    if (QTextCodec *codec = QTextCodec::codecForName(encoding)) {
        return codec->toUnicode(data);
    }

    return QString::fromLocal8Bit(data);
}

QString DFMGlobal::cutString(const QString &text, int dataByteSize, const QTextCodec *codec)
{
    QString new_text;
    int bytes = 0;

    for (int i = 0; i < text.size(); ++i) {
        const QChar &ch = text.at(i);
        QByteArray data;
        QString full_char;

        if (ch.isSurrogate()) {
            if ((++i) >= text.size())
                break;

            const QChar &next_ch = text.at(i);

            if (!ch.isHighSurrogate() || !next_ch.isLowSurrogate())
                break;

            data = codec->fromUnicode(text.data() + i - 1, 2);
            full_char.setUnicode(text.data() + i - 1, 2);
        } else {
            data = codec->fromUnicode(text.data() + i, 1);
            full_char.setUnicode(text.data() + i, 1);
        }

        if (codec->toUnicode(data) != full_char) {
            qWarning() << "Failed convert" << full_char << "to" << codec->name() << "coding";
            continue;
        }

        bytes += data.size();

        if (bytes > dataByteSize)
            break;

        new_text.append(ch);

        if (ch.isSurrogate())
            new_text.append(text.at(i));
    }

    return new_text;
}

namespace DThreadUtil {
FunctionCallProxy::FunctionCallProxy()
{
    connect(this, &FunctionCallProxy::callInLiveThread, this, [] (FunctionType *func) {
        (*func)();
    }, Qt::QueuedConnection);
}
} // end namespace DThreadUtil


///###: Do not modify it.
///###: it's auxiliary.
float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country)
{
    qreal hep_count = 0;
    int non_base_latin_count = 0;
    qreal unidentification_count = 0;
    int replacement_count = 0;

    QTextDecoder decoder(codec);
    const QString &unicode_data = decoder.toUnicode(data);

    for (int i = 0; i < unicode_data.size(); ++i) {
        const QChar &ch = unicode_data.at(i);

        if (ch.unicode() > 0x7f)
            ++non_base_latin_count;

        switch (ch.script()) {
        case QChar::Script_Hiragana:
        case QChar::Script_Katakana:
            hep_count += country == QLocale::Japan ? 1.2 : 0.5;
            unidentification_count += country == QLocale::Japan ? 0 : 0.3;
            break;
        case QChar::Script_Han:
            hep_count += country == QLocale::China ? 1.2 : 0.5;
            unidentification_count += country == QLocale::China ? 0 : 0.3;
            break;
        case QChar::Script_Hangul:
            hep_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 0 : 0.3;
            break;
        default:
            // full-width character, emoji, 常用标点, 拉丁文补充1
            if ((ch.unicode() >= 0xff00 && ch <= 0xffef)
                    || (ch.unicode() >= 0x2600 && ch.unicode() <= 0x27ff)
                    || (ch.unicode() >= 0x2000 && ch.unicode() <= 0x206f)
                    || (ch.unicode() >= 0x80 && ch.unicode() <= 0xff)) {
                ++hep_count;
            } else if (ch.isSurrogate() && ch.isHighSurrogate()) {
                ++i;

                if (i < unicode_data.size()) {
                    const QChar &next_ch = unicode_data.at(i);

                    if (!next_ch.isLowSurrogate()) {
                        --i;
                        break;
                    }

                    uint unicode = QChar::surrogateToUcs4(ch, next_ch);

                    // emoji
                    if (unicode >= 0x1f000 && unicode <= 0x1f6ff) {
                        hep_count += 2;
                    }
                }
            } else if (ch.unicode() == QChar::ReplacementCharacter) {
                ++replacement_count;
            } else if (ch.unicode() > 0x7f) {
                // 因为UTF-8编码的容错性很低，所以未识别的编码只需要判断是否为 QChar::ReplacementCharacter 就能排除
                if (codec->name() != "UTF-8")
                    ++unidentification_count;
            }
            break;
        }
    }

    float c = qreal(hep_count) / non_base_latin_count / 1.2;

    c -= qreal(replacement_count) / non_base_latin_count;
    c -= qreal(unidentification_count) / non_base_latin_count;

    return qMax(0.0f, c);
}










