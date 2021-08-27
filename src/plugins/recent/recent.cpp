//new lib
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"

#include "dfm-base/widgets/dfmfileview/fileview.h"
//#include "widgets/dfmfileview/dfilesystemmodel.h"

#include "dfm-base/base/dfmstandardpaths.h"
#include "dfm-base/base/dfmapplication.h"
#include "dfm-base/base/dfmschemefactory.h"

#include "dfm-base/localfile/dfmlocalfileinfo.h"
#include "dfm-base/localfile/dfmlocaldiriterator.h"
#include "dfm-base/localfile/dfmlocalfiledevice.h"
#include "dfm-base/base/dabstractfilewatcher.h"

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

//self files
#include "recent.h"
#include "recentutil.h"

//dfm-framework
#include "dfm-framework/lifecycle/plugincontext.h"

//services
#include "windowservice/windowservice.h"
#include "windowservice/browseview.h"

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

void Recent::initialize()
{
    qInfo() << Q_FUNC_INFO;
}

bool Recent::start(QSharedPointer<dpf::PluginContext> context)
{
    Q_UNUSED(context)
    qInfo() << Q_FUNC_INFO;
    qInfo() << RecentUtil::instance().loadRecentFile();

    return true;
}

dpf::Plugin::ShutdownFlag Recent::stop() {
    return Synch;
}
