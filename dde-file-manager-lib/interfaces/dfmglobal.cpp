#include "dfmglobal.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QIcon>
#include <QUrl>

namespace GlobalData {
static QList<QUrl> clipboardFileUrls;
static DFMGlobal::ClipboardAction clipboardAction = DFMGlobal::Unknow;

void onClipboardDataChanged()
{
    clipboardFileUrls.clear();
    const QByteArray &data = qApp->clipboard()->mimeData()->data("x-special/gnome-copied-files");

    if (data.startsWith("cut")) {
        clipboardAction = DFMGlobal::CutAction;
    } else if (data.startsWith("copy")) {
        clipboardAction = DFMGlobal::CopyAction;
    } else {
        clipboardAction = DFMGlobal::Unknow;
    }

    for (const QUrl &url : qApp->clipboard()->mimeData()->urls()) {
        clipboardFileUrls << url;
    }
}

class DFMGlobalPrivate : public DFMGlobal {};
Q_GLOBAL_STATIC(DFMGlobalPrivate, dfmGlobal);
}

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

void DFMGlobal::setUrlsToClipboard(const QList<QUrl> &list, DFMGlobal::ClipboardAction action)
{
    if (action == Unknow)
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
    qApp->clipboard()->clear();
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
    static QIcon unreadableIcon(":/images/images/unreadable.svg");
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
