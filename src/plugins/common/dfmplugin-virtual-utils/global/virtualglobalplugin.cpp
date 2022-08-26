#include "virtualglobalplugin.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/file/local/desktopfileinfo.h"

using namespace dfmplugin_utils;

static QSharedPointer<dfmbase::AbstractFileInfo> transFileInfo(QSharedPointer<dfmbase::AbstractFileInfo> fileInfo)
{
    // no translate for gvfs file, invoking suffix/mimeTypeName might cost huge time
    if (fileInfo->url().path().contains(QRegularExpression(DFMBASE_NAMESPACE::Global::Regex::kGvfsRoot)))
        return fileInfo;

    const QString &suffix = fileInfo->suffix();
    const QString &mimeTypeName = fileInfo->mimeTypeName();
    if (suffix == DFMBASE_NAMESPACE::Global::Scheme::kDesktop && mimeTypeName == "application/x-desktop") {
        const QUrl &url = fileInfo->url();
        return DFMLocalFileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url));
    }
    return fileInfo;
}

void VirtualGlobalPlugin::initialize()
{
    eventReceiver->initEventConnect();

    DFMBASE_NAMESPACE::InfoFactory::regInfoTransFunc<DFMBASE_NAMESPACE::AbstractFileInfo>(DFMBASE_NAMESPACE::Global::Scheme::kFile, transFileInfo);
}

bool VirtualGlobalPlugin::start()
{
    return true;
}
