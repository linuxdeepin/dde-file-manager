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

#include <cstdio>

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

QString DFMGlobal::toUnicode(const QByteArray &ba)
{
    if (ba.isEmpty())
        return QString();

    QList<QByteArray> codecList;

    codecList << "utf-8";

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
        const QString str_fileName = DFMGlobal::toUnicode(name);

        if (str_fileName == info.fileName() && str_fileName.toLocal8Bit() != name) {
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
