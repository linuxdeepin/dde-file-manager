#ifndef GLOBAL_H
#define GLOBAL_H

#define fileManagerApp Singleton<FileManagerApp>::instance()
#define searchHistoryManager  Singleton<SearchHistroyManager>::instance()
#define bookmarkManager  Singleton<BookMarkManager>::instance()
#define trashManager  Singleton<TrashManager>::instance()
#define fileMenuManger  Singleton<FileMenuManager>::instance()
#define fileSignalManager Singleton<FileSignalManager>::instance()
#define dialogManager Singleton<DialogManager>::instance()
#define appController fileManagerApp->getAppController()
#define fileIconProvider Singleton<IconProvider>::instance()
#define fileService FileServices::instance()
#define deviceListener Singleton<UDiskListener>::instance()
#define mimeAppsManager Singleton<MimesAppsManager>::instance()
#define systemPathManager Singleton<PathManager>::instance()
#define mimeTypeDisplayManager Singleton<MimeTypeDisplayManager>::instance()
#define thumbnailManager Singleton<ThumbnailManager>::instance()
#define networkManager Singleton<NetworkManager>::instance()
#define gvfsMountClient Singleton<GvfsMountClient>::instance()
#define secrectManager Singleton<SecrectManager>::instance()
#define userShareManager Singleton<UserShareManager>::instance()

#define defaut_icon ":/images/images/default.png"
#define defaut_computerIcon ":/images/images/computer.png"
#define defaut_trashIcon ":/images/images/user-trash-full.png"

// begin file item global define
#define TEXT_PADDING 5
#define ICON_MODE_ICON_SPACING 5
#define COLUMU_PADDING 10
#define LEFT_PADDING 10
#define RIGHT_PADDING 10
// end

// begin file view global define
#define LIST_MODE_LEFT_MARGIN 20
#define LIST_MODE_RIGHT_MARGIN 20
// end

#endif // GLOBAL_H
