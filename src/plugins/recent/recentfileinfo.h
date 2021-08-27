#ifndef DFMRECENTFILEINFO_H
#define DFMRECENTFILEINFO_H

#include "dfm-base/localfile/dfmlocalfileinfo.h"

#include <QXmlStreamReader>

// 该类为二次关联类
// 从QUrl创建的fileInfo为此类，但是此路径为虚拟路径
// 而真实路径地址应在内部缓存
class RecentFileInfo : public DFMLocalFileInfo
{

public:
    explicit RecentFileInfo(const QUrl &url = QUrl())
    {
        Q_UNUSED(url);
    }
};

#endif // DFMRECENTFILEINFO_H
