//new lib
#include "widgets/dfmsidebar/dfmsidebar.h"
#include "widgets/dfmsidebar/dfmsidebaritem.h"
#include "widgets/dfmsidebar/dfmsidebarview.h"
#include "widgets/dfmsidebar/dfmsidebarmodel.h"

#include "widgets/dfmfileview/dfmfileview.h"
//#include "widgets/dfmfileview/dfilesystemmodel.h"

#include "base/dfmstandardpaths.h"
#include "base/dfmapplication.h"
#include "base/dfmschemefactory.h"

#include "localfile/dfmlocalfileinfo.h"
#include "localfile/dfmlocaldiriterator.h"
#include "localfile/dfmlocalfiledevice.h"
#include "base/dabstractfilewatcher.h"

//old lib
//#include "interfaces/dfmapplication.h"
//#include "views/dfmsidebar.h"
//#include "views/dfileview.h"
//#include "views/dfmsidebarview.h"
//#include "models/dfmsidebarmodel.h"
//#include "interfaces/dfmsidebaritem.h"
//#include "interfaces/dfilesystemmodel.h"
//#include "views/dfilemanagerwindow.h"
//#include "views/dtoolbar.h"

#include "recent.h"
#include "dfmrecentutil.h"

#include "dfmplugincontext.h"

//services
#include "previewservice/dfmoldpreviewservice.h"
#include "windowservice/dfmwindowservice.h"
#include "windowservice/dfmbrowseview.h"

//plugin base
#include "previewservice/oldpluginbase/dfmfilepreviewplugin.h"
#include "previewservice/oldpluginbase/dfmfilepreview.h"

#include "event/dfmglobaleventdispatcher.h"
#include "event/handler/dfmwindoweventhandler.h"

#include <QListWidget>
#include <QListView>
#include <QTreeView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QDockWidget>
#include <QStatusBar>
#include <QLabel>
#include <QFrame>
#include <QIcon>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSplitter>
#include <QDir>
#include <QSizePolicy>
#include <QToolButton>

#define DEFAULT_WINDOW_WIDTH 760
#define DEFAULT_WINDOW_HEIGHT 420

//DFM_USE_NAMESPACE

static DFMApplication* dfmApp = nullptr;

Q_GLOBAL_STATIC(DFMWindowEventHandler,_RecentWindowEventHandler)

void Recent::initialize()
{
    qCDebug(FrameworkLog) << Q_FUNC_INFO;
}

bool Recent::start(QSharedPointer<DFMPluginContext> context)
{
    qCDebug(FrameworkLog) << Q_FUNC_INFO;
    qInfo() << DFMRecentUtil::instance().loadRecentFile();
    return true;
}

DFMPlugin::ShutdownFlag Recent::stop() {
    return Synch;
}
