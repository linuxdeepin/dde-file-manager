#include "fileeventreceiver.h"

#include <dfm-base/utils/fileutils.h>

DSC_BEGIN_NAMESPACE

DFMBASE_USE_NAMESPACE

const QString TOPIC_FILE_EVENT {"fileEvent"};

namespace FileEventDatas
{
    const QString MKDIR {"mkdir"};
    const QString TOUCH {"touch"};
} //namespace FileEventDatas

namespace FileEvenPropertys
{
    const QString ROOTURL {"rootUrl"};
    const QString DIRNAMES {"dirNames"};
    const QString DIRNAME {"dirName"};
} //namespace FileEvenPropertys

dpf::EventHandler::Type FileEventReceiver::type()
{
    return EventHandler::Type::Async; // 异步执行当前对象
}

QStringList FileEventReceiver::topics()
{
    return QStringList() << TOPIC_FILE_EVENT; //订阅主题
}

void FileEventReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == TOPIC_FILE_EVENT) {
        if (event.data() == FileEventDatas::MKDIR) {

            // 获取当前操作目录
            QUrl rootUrl;
            if (event.property(FileEvenPropertys::ROOTURL).canConvert<QUrl>()) {
                rootUrl = qvariant_cast<QUrl>(event.property(FileEvenPropertys::ROOTURL));
            }

            // 创建多个文件夹
            if (event.property(FileEvenPropertys::DIRNAMES).canConvert<QList<QString>>()) {
                auto dirNames = qvariant_cast<QList<QString>>(event.property(FileEvenPropertys::DIRNAMES));
                for(auto val : dirNames) {
                    FileUtils::mkdir(rootUrl, val);
                }
            }

            // 创建单个文件夹
            if (event.property(FileEvenPropertys::DIRNAME).canConvert<QString>()) {
                auto dirName = event.property(FileEvenPropertys::DIRNAME).toString();
                FileUtils::mkdir(rootUrl, dirName);
            }
        }
    } else {
        abort(); //接收了非订阅主题为框架致命错误
    }
}

DSC_END_NAMESPACE


