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
        UnknowAction
    };

    enum Icon {
        LinkIcon,
        LockIcon,
        UnreadableIcon,
        ShareIcon
    };

    enum MenuAction {
        Open,
        OpenDisk,
        OpenInNewWindow,
        OpenInNewTab,
        OpenDiskInNewWindow,
        OpenAsAdmain,
        OpenWith,
        OpenWithCustom,
        OpenFileLocation,
        Compress,
        Decompress,
        DecompressHere,
        Cut,
        Copy,
        Paste,
        Rename,
        Remove,
        CreateSymlink,
        SendToDesktop,
        AddToBookMark,
        Delete,
        Property,
        NewFolder,
        NewWindow,
        SelectAll,
        Separator,
        ClearRecent,
        ClearTrash,
        DisplayAs, /// sub menu
        SortBy, /// sub menu
        NewDocument, /// sub menu
        NewWord, /// sub menu
        NewExcel, /// sub menu
        NewPowerpoint, /// sub menu
        NewText, /// sub menu
        OpenInTerminal,
        Restore,
        RestoreAll,
        CompleteDeletion,
        Mount,
        Unmount,
        Eject,
        Name,
        Size,
        Type,
        CreatedDate,
        LastModifiedDate,
        DeletionDate,
        SourcePath,
        AbsolutePath,
        Settings,
        Help,
        About,
        Exit,
        IconView,
        ListView,
        ExtendView,
        SetAsWallpaper,
        ForgetPassword,
        UnShare,
        Unknow
    };

    Q_ENUM(MenuAction)

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
