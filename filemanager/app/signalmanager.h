#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include "dbusinterface/dbustype.h"
#include <QtCore>


class SignalManager: public QObject
{
    Q_OBJECT
public:
    explicit SignalManager(){}
    ~SignalManager(){}

signals:
    /*leftsidebar signal*/
    void destinationFolderChanged(const QString& destination);
    void historyRecordChecked();

    /*keyboard signal*/
    void keyUpPressed();
    void keyDownPressed();
    void keyLeftPressed();
    void keyRightPressed();

    void keyShiftLeftPressed();
    void keyShiftRightPressed();
    void keyShiftUpPressed();
    void keyShiftDownPressed();

    void keyCtrlAPressed();
    void keyCtrlCPressed();
    void keyCtrlVPressed();
    void keyCtrlXPressed();

    void keyHomePressed();
    void keyEndPressed();

    void keyEnterPressed();

    void keyShiftHomePressed();
    void keyShiftEndPressed();

    /*desktop menu*/
    void sortByKey(QString key);

    /*double open file*/
    void openFile(FileItemInfo info);
    /*drop open files*/
    void openFiles(FileItemInfo destinationDesktopItemInfo, QList<FileItemInfo> desktopItemInfos);
    void openFiles(FileItemInfo destinationDesktopItemInfo, QStringList urls);

    /*trash files*/
    void trashEmptyChanged(bool flag);
    void trashingAboutToExcute(QStringList files); /*used for keyboard delete/ shift delete*/
    void trashingAboutToAborted();
    void trashingFileChanged(QString file);
    void deletingFileChanged(QString file);
    void processAmountChanged(qlonglong progress, ushort info);

    void requestEmptyTrash();

    /*copy files*/
    void filesCopied(QStringList urls);

    /*cut files*/
    void filesCuted(QStringList urls);
    void cancelFilesCuted(QStringList urls);
    void cancleFilesCuted();

    /*paste files*/
    void pasteFilesToDesktop();
    void moveFilesExcuted(const QStringList& files, QString destination);
    void moveJobAboutToAbort();
    void movingFileChaned(QString file);
    void movingProcessAmountChanged(qlonglong progress, ushort info);

    /*dbus delete files*/
    void deleteFilesExcuted(const QMap<QString, QString>& files);
    void deleteJobAboutToAbort();
    void deletingFileChaned(QString file);
    void deletingProcessAmountChanged(qlonglong progress, ushort info);

    /*dbus copy files*/
    void copyFilesExcuted(QStringList files, QString destination);
    void copyJobAboutToAbort();
    void copyingFileChaned(QString file);
    void copyingProcessAmountChanged(qlonglong progress, ushort info);

    /*update desktop icon like computer trash....*/
    void desktoItemIconUpdated(QString url, QString iconUl, uint size);

    /*update app group desktop icon*/
    void requestCreatingAppGroup(QStringList urls);
    void requestMergeIntoAppGroup(QStringList urls, QString group_url);

    void desktopItemsChanged(FileItemInfoMap desktopnItems);
    void appGounpItemsChanged(QString group_url, FileItemInfoMap appItems);

    /*monitor desktop folder*/
    void itemCreated(FileItemInfo fileInfo);
    void itemDeleted(QString url);

    void itemShoudBeMoved(QString url);
    void itemMoved(FileItemInfo& fileInfo);

    /*F2 renamed*/
    void renameStarted();
    void renameFinished();
    void renameJobCreated(QString url, QString newName);

    /*context menu renamed*/
    void requestRenamed(QString url);

    /*dock mode change*/
    void dockModeChanged(int dockMode);

    /*create folder/file/template finised*/
    void fileCreated(QString filename);

    /*request desktop items from dbus*/
    void startRequest();
    void stopRequest();

    /*update gridmanager status when move desktopitem*/
    void gridStatusUpdated();

    /*screen geometry changed*/
    void screenGeometryChanged();

    /*gtk wather monitor icon theme changed signal*/
    void gtkIconThemeChanged();

    /*dbus to copy job dialog */
    void copyJobAdded(const QMap<QString, QString>& jobDetail);
    void copyJobRemoved(const QMap<QString, QString>& jobDetail);

    void copyJobDataUpdated(const QMap<QString, QString>& jobDetail,
                           const QMap<QString, QString>& data);

    /*copy job dialog to dbus*/
    void abortCopyTask(const QMap<QString, QString>& jobDetail);


    /*dbus to move job dialog */
    void moveJobAdded(const QMap<QString, QString>& jobDetail);
    void moveJobRemoved(const QMap<QString, QString>& jobDetail);

    void moveJobDataUpdated(const QMap<QString, QString>& jobDetail,
                           const QMap<QString, QString>& data);

    /*move job dialog to dbus*/
    void abortMoveTask(const QMap<QString, QString>& jobDetail);

    /*delete confirm dialog to delete woker*/
    void filesDeleted(const QStringList& files);

    /*dbus to delete job dialog */
    void deleteJobAdded(const QMap<QString, QString>& jobDetail);
    void deleteJobRemoved(const QMap<QString, QString>& jobDetail);

    void deleteJobDataUpdated(const QMap<QString, QString>& jobDetail,
                           const QMap<QString, QString>& data);

    /*move job dialog to dbus*/
    void abortDeleteTask(const QMap<QString, QString>& jobDetail);


    /*copy/move job conflict dialog show */
    void conflictDialogShowed(const QMap<QString, QString>& jobDetail);
    void conflictRepsonseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

    void conflictTimerStoped(const QMap<QString, QString>& jobDetail);
    void conflictTimerReStarted(const QMap<QString, QString>& jobDetail);

    /*refresh icon when file copy finished*/
    void refreshCopyFileIcon(QString file);

    /*request thumbnail when desktopitem renamed finished*/
    void thumbnailRequested(QString url);

    /*mutisreen desktop main frame view positionchanged*/
    void desktopFrameRectChanged(QRect rect);

    /*confirm dialog*/
    void confimClear(int count);
    void actionHandled(int index);

    /*shift released*/
    void shiftReleased(bool flag);

    /*app group right clicked*/
    void appGroupItemRightClicked(bool flag);

    /*desktop textedit contextMenu show by conetnt*/
    void showTextEditMenuBySelectContent(QString url, QString fullText, QString selectText, QPoint pos);
    void desktopItemNameCuted(QString url);
    void desktopItemNameCopyed(QString url);
    void desktopItemNamePasted(QString url);
    void desktopItemNameSelectAll(QString url);

    /*set right bottom grid as a container*/
    void rightBottomItemChangedtoBeContainer(bool flag);

    /*Menu Show/Hide signal*/
    void  contextMenuShowed(bool flag);

    /*rename tip dialog show*/
    void renameDialogShowed(QString name);

    /*send pinyin result to desktopitem manager*/
    void pinyinResultChanged(const QList<FileItemInfo>& infos);

    /*growing edit enter/return pressed*/
    void returnEnterPressed();


    /*press F1 to excute dman dde*/
    void f1Preesed();

    /*active changed signal from dock dbus client*/
    void activeWindowChanged(uint windowId);

    /*File metaDataChanged like readable or permission*/
    void fileMetaDataChanged(const QString& path);

private:
    Q_DISABLE_COPY(SignalManager)
};

#endif // SIGNALMANAGER_H
