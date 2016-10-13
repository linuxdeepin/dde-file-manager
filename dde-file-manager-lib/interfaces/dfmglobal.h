#ifndef DFMGLOBAL_H
#define DFMGLOBAL_H

#include <QObject>

class DFMGlobal : public QObject
{
    Q_OBJECT

public:
    enum ClipboardAction {
        CutAction,
        CopyAction,
        Unknow
    };

    enum Icon {
        LinkIcon,
        LockIcon,
        UnreadableIcon,
        ShareIcon
    };

    static DFMGlobal *instance();

    static QString organizationName();
    static QString applicationName();
    // libdde-file-manager version
    static QString applicationVersion();

    static void setUrlsToClipboard(const QList<QUrl> &list, DFMGlobal::ClipboardAction action);
    static void clearClipboard();

    QList<QUrl> clipboardFileUrlList() const;
    ClipboardAction clipboardAction() const;
    QIcon standardIcon(Icon iconType) const;

signals:
    void clipboardDataChanged();

protected:
    DFMGlobal();

private:
    void onClipboardDataChanged();
};

#endif // DFMGLOBAL_H
