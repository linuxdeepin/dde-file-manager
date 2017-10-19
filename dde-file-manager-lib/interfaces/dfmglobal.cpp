/*
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


#include <cstdio>
#include <locale>
#include <sstream>
#include <fstream>
#include <uchardet/uchardet.h>

#ifdef __cplusplus
extern "C"
{
#endif __cplusplus

#include <iconv.h>

#ifdef __cplusplus
}
#endif __cplusplus

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

QString DFMGlobal::wordWrapText(const QString &text, int width, QTextOption::WrapMode wrapMode,
                                const QFont &font, int lineHeight, int *height)
{
    QTextLayout textLayout(text);
    QTextOption &text_option = *const_cast<QTextOption*>(&textLayout.textOption());
    text_option.setWrapMode(wrapMode);

    textLayout.setFont(font);
    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();
    QString  str;

    int text_height = 0;

    while (line.isValid()) {
        line.setLineWidth(width);

        const QString &tmp_str = text.mid(line.textStart(), line.textLength());

        str += tmp_str;

        if (tmp_str.indexOf('\n') >= 0)
            text_height += lineHeight;

        text_height += lineHeight;
        line = textLayout.createLine();

        if(line.isValid())
            str.append("\n");
    }

    textLayout.endLayout();

    if(height)
        *height = text_height;

    return str;
}

QString DFMGlobal::elideText(const QString &text, const QSize &size,
                          QTextOption::WrapMode wordWrap, const QFont &font,
                             Qt::TextElideMode mode, int lineHeight, int flags)
{
    int height = 0;

    QTextLayout textLayout(text);
    QString str;
    QFontMetrics fontMetrics(font);

    textLayout.setFont(font);
    const_cast<QTextOption*>(&textLayout.textOption())->setWrapMode(wordWrap);

    textLayout.beginLayout();

    QTextLine line = textLayout.createLine();

    while (line.isValid()) {
        height += lineHeight;

        if(height + lineHeight >= size.height()) {
            str += fontMetrics.elidedText(text.mid(line.textStart() + line.textLength() + 1), mode, size.width(), flags);

            break;
        }

        line.setLineWidth(size.width());

        const QString &tmp_str = text.mid(line.textStart(), line.textLength());

        if (tmp_str.indexOf('\n'))
            height += lineHeight;

        str += tmp_str;

        line = textLayout.createLine();

        if(line.isValid())
            str.append("\n");
    }

    textLayout.endLayout();

    return str;
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


QSharedPointer<QString> DFMGlobal::convertFileToUtf8(const DUrl& url)
{
    QSharedPointer<QString> convertedStr{ nullptr };
    QFileInfo info{ url.toLocalFile() };
    std::basic_ostringstream<char> fileContentStream;

    if(info.isFile() == true){

        std::string fileName{ url.toLocalFile().toStdString() };
        std::basic_ifstream<char> fstream{ fileName, std::ios_base::in | std::ios_base::out };

        if(fstream.is_open() == true){

            std::istream_iterator<char> istream_itr{ fstream };
            std::copy(istream_itr, std::istream_iterator<char>{},
                      std::ostream_iterator<char>{fileContentStream});

            std::string fileContent{ fileContentStream.str() };
            QByteArray qFileContent{ QByteArray::fromStdString(fileContent) };

            if(fileContent.empty() == false){
                QByteArray charsetName{ DFMGlobal::detectCharset(qFileContent) };

                if(charsetName.isEmpty() == false){

                    convertedStr = DFMGlobal::convertAnyCharsetToUtf8(charsetName, qFileContent);
                }
            }

        }
    }

    return convertedStr;
}


QSharedPointer<QString> DFMGlobal::convertStrToUtf8(const QByteArray &str)
{
    QByteArray charsetName;
    QSharedPointer<QString> convertedStr{ nullptr };

    if(str.isEmpty() == false){
        charsetName = DFMGlobal::detectCharset(str);

        if(charsetName.isEmpty() == false){
            convertedStr = DFMGlobal::convertAnyCharsetToUtf8(charsetName, str);
        }
    }

    return convertedStr;
}


QSharedPointer<QString> DFMGlobal::convertAnyCharsetToUtf8(const QByteArray& charsetName, QByteArray content)
{
    QSharedPointer<QString> convertedStr{ nullptr };


    if(charsetName != QByteArray{"utf-8"}){

        std::size_t inputBufSize{ content.size() };
        std::size_t outputBufSize{ inputBufSize * 4 };
        char* inputBuff{ content.data() };
        char* outputBuff{ new char[outputBufSize] };
        char* backupPtr{ outputBuff };
        std::string toCode{ "utf-8" };

        iconv_t code{ iconv_open(toCode.c_str(), charsetName.constData())};
        std::size_t retVal{ iconv(code, &inputBuff, &inputBufSize, &outputBuff, &outputBufSize) };//###: do conversion by code.

        std::size_t actuallyUsed{ outputBuff - backupPtr };

        convertedStr = QSharedPointer<QString>{ new QString{ QString::fromUtf8(QByteArray{backupPtr, actuallyUsed}) } };
        iconv_close(code);


        delete[] backupPtr;
        return convertedStr;
    }

    return QSharedPointer<QString>{ new QString{ QString::fromUtf8(content) } };

}


QByteArray DFMGlobal::detectCharset(const QByteArray& str)
{
    uchardet_t handle{ uchardet_new() };
    std::string charsetName;
    int returnedVal{ 0 };

    returnedVal = uchardet_handle_data(handle, str.constData(), str.size()); //start detecting.
    if(returnedVal != 0){ //if less than 0, it show the recognization failed.
        uchardet_data_end(handle);
        uchardet_delete(handle);

        return QByteArray::fromStdString(charsetName);
    }

    uchardet_data_end(handle);
    charsetName = std::string{ uchardet_get_charset(handle) };
    uchardet_delete(handle);

    //This function promise that When is converting the target charset is ASCII.
    const auto& facet = std::use_facet<std::ctype<char>>(std::locale{"C"});
    facet.tolower(&charsetName[0], &charsetName[charsetName.size()]);

    return QByteArray::fromStdString(charsetName);
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
        QSharedPointer<QString> str_fileName{ DFMGlobal::convertStrToUtf8(name) };
        QString strFileName{ "" };
        if(static_cast<bool>(str_fileName) == true){
            strFileName = *str_fileName;
        }

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



QString DFMGlobal::toUnicode(const QByteArray &ba)
{
    if (ba.isEmpty())
        return QString();

    QList<QByteArray> codecList;

    codecList << "utf-8" << "utf-16";

    switch (QLocale::system().script()) {
    case QLocale::SimplifiedChineseScript:
        codecList << "gbk";
        break;
    case QLocale::TraditionalChineseScript:
        codecList << "big5" << "gbk";
        break;
    case QLocale::JapaneseScript:
        codecList << "shift_jis" << "euc_jp" << "gbk";
        break;
    case QLocale::KoreanScript:
        codecList << "euc_kr";
        break;
    default:
        break;
    }

    for (const QByteArray &codec : codecList) {
        const QString &text = textDecoder(ba, codec);

        if (!text.isEmpty())
            return text;
    }

    return QString::fromLocal8Bit(ba);
}

namespace DThreadUtil {
class FunctionCallProxy_ : public FunctionCallProxy {};
Q_GLOBAL_STATIC(FunctionCallProxy_, fcpGlobal)
FunctionCallProxy *DThreadUtil::FunctionCallProxy::instance()
{
    return fcpGlobal;
}

FunctionCallProxy::FunctionCallProxy()
{
    // move to main thread
    moveToThread(qApp->thread());

    connect(this, &FunctionCallProxy::callInMainThread, this, [] (FunctionType *func) {
        (*func)();
    }, Qt::QueuedConnection);
}
} // end namespace DThreadUtil
