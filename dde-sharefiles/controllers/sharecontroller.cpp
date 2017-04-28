#include "sharecontroller.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

ShareController::ShareController(const ShareController &)
    : ApplicationController()
{ }

void ShareController::share()
{
    // write code
    tError() << httpRequest().allParameters();

    QJsonArray array;

    for(int i=0; i< 1000; i++){
        QJsonObject obj;
        obj.insert("path", "/home/djf/workspace/ShareFiles");
        obj.insert("icon", "/home/djf/workspace/ShareFiles1");
        obj.insert("filename", "ShareFiles2");
        obj.insert("displayname", "ShareFiles3");
        const THttpRequest& request = httpRequest();
        foreach (QString key, request.allParameters().keys()) {
            obj.insert(key, request.queryItemValue(key));
        }
        array.append(QJsonValue(obj));
    }

    renderJson(array);
}

void ShareController::shareDir()
{
    share();
}

void ShareController::shareFiles()
{
    QJsonArray array;

    for(int i=0; i< 10; i++){
        QJsonObject obj;
        obj.insert("path", "/home/djf/workspace/ShareFiles");
        obj.insert("icon", "/home/djf/workspace/ShareFiles1");
        obj.insert("filename", "ShareFiles2");
        obj.insert("displayname", "ShareFiles3");
        const THttpRequest& request = httpRequest();
        foreach (QString key, request.allParameters().keys()) {
            obj.insert(key, request.queryItemValue(key));
        }
        array.append(QJsonValue(obj));
    }

    renderJson(array);
}

void ShareController::download()
{
    const THttpRequest& request = httpRequest();
    QMimeDatabase qMimeDatabase;
    QJsonArray array;
    foreach (QString key, request.allParameters().keys()) {
        QString f = request.queryItemValue(key);
        QFileInfo info(f);
        if (info.exists()){
            QMimeType mimeType = qMimeDatabase.mimeTypeForFile(info);
            sendFile(f, mimeType.name().toLocal8Bit());
        }else{
            QJsonObject obj;
            obj.insert(key, f);
            array.append(QJsonValue(obj));
        }
    }
    renderJson(array);
}


// Don't remove below this line
T_REGISTER_CONTROLLER(sharecontroller)
