#include "proxyfileinfo.h"

DFMBASE_USE_NAMESPACE

#define CALL_PROXY(Fun) \
    if (proxy) return proxy->Fun;

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
