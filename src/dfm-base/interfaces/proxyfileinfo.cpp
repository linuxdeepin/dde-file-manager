#include "proxyfileinfo.h"

DFMBASE_USE_NAMESPACE

ProxyFileInfo::ProxyFileInfo(const QUrl &url)
    : FileInfo(url)
{
}

ProxyFileInfo::~ProxyFileInfo()
{
}

void ProxyFileInfo::setProxy(const FileInfoPointer &proxy)
{
    this->proxy = proxy;
}
