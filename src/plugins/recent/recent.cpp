//new lib
#include "dfm-base/widgets/dfmsidebar/sidebar.h"
#include "dfm-base/widgets/dfmsidebar/sidebaritem.h"
#include "dfm-base/widgets/dfmsidebar/sidebarview.h"
#include "dfm-base/widgets/dfmsidebar/sidebarmodel.h"

#include "dfm-base/widgets/dfmfileview/fileview.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/application.h"
#include "dfm-base/base/schemefactory.h"

#include "dfm-base/localfile/localfileinfo.h"
#include "dfm-base/localfile/localdiriterator.h"
#include "dfm-base/base/abstractfilewatcher.h"

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
