#include "fileeventreceiver.h"

#include <dfm-base/utils/fileutils.h>

DSC_BEGIN_NAMESPACE

DFMBASE_USE_NAMESPACE

dpf::EventHandler::Type FileEventReceiver::type()
{
    return EventHandler::Type::Async; // 异步执行当前对象
}

QStringList FileEventReceiver::topics()
{
    return QStringList() << FileEventTypes::TOPIC_FILE_EVENT; //订阅主题
}

void FileEventReceiver::eventProcess(const dpf::Event &event)
{
    if (event.topic() == FileEventTypes::TOPIC_FILE_EVENT) {
        if (event.data() == FileEventTypes::DATA_MKDIR) {

            // 获取当前操作目录
            QUrl rootUrl;
            if (event.property(FileEventTypes::PROPERTY_KEY_ROOTURL).canConvert<QUrl>()) {
                rootUrl = qvariant_cast<QUrl>(event.property(FileEventTypes::PROPERTY_KEY_ROOTURL));
            }

            // 创建多个文件夹
            if (event.property(FileEventTypes::PROPERTY_KEY_DIRNAMES).canConvert<QList<QString>>()) {
                auto dirNames = qvariant_cast<QList<QString>>(event.property(FileEventTypes::PROPERTY_KEY_DIRNAMES));
                for(auto val : dirNames) {
                    FileUtils::mkdir(rootUrl, val);
                }
            }

            // 创建单个文件夹
            if (event.property(FileEventTypes::PROPERTY_KEY_DIRNAME).canConvert<QString>()) {
                auto dirName = event.property(FileEventTypes::PROPERTY_KEY_DIRNAME).toString();
                FileUtils::mkdir(rootUrl, dirName);
            }
        }
    } else {
        abort(); //接收了非订阅主题为框架致命错误
    }
}

DSC_END_NAMESPACE


