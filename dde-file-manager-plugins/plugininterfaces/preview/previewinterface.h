#ifndef PREVIEWINTERFACE_H
#define PREVIEWINTERFACE_H

#include <QStringList>
#include <QWidget>
#include <QIcon>

#define DEFALT_PREVIEW_MIN_SIZE QSize(600, 300)

class PreviewInterface : public QObject
{
public:
    explicit PreviewInterface(QObject *parent = 0)
        : QObject(parent) {}

    virtual void init(const QString& uri){
        Q_UNUSED(uri)
    }

    virtual QWidget* previewWidget(){
        return NULL;
    }

    virtual QSize previewWidgetMinSize() const {
        return DEFALT_PREVIEW_MIN_SIZE;
    }

    virtual bool canPreview() const{
        return false;
    }

    virtual QWidget* toolBarItem(){
        return NULL;
    }

    virtual QString pluginName() const{
        return "";
    }

    virtual QIcon pluginLogo() const{
        return QIcon();
    }

    virtual QString pluginDescription() const{
        return "";
    }
};

#define PreviewInterface_iid "com.deepin.dde-file-manager.PreviewInterface"

Q_DECLARE_INTERFACE(PreviewInterface, PreviewInterface_iid)

#endif // PREVIEWINTERFACE_H
