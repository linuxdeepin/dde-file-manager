#include "dcustomactionbuilder.h"
#include <QDir>
#include <QUrl>

DCustomActionBuilder::DCustomActionBuilder(QObject *parent) : QObject(parent)
{

}

QMenu *DCustomActionBuilder::buildMenu(const DCustomActionData &actionData) const
{
    if (!actionData.isMenu())
        return nullptr;

    QMenu *menu;
}

QAction *DCustomActionBuilder::buildAciton(const DCustomActionData &actionData) const
{
    if (actionData.isMenu())
        return nullptr;
}

QIcon DCustomActionBuilder::getIcon(const QString &iconName) const
{
    QIcon ret;
    QFileInfo fileInfo(iconName.startsWith("~") ? (QDir::homePath() + iconName.mid(1)) : iconName);

    if (!fileInfo.exists())
        fileInfo.setFile(QUrl::fromUserInput(iconName).toLocalFile());

    if (fileInfo.exists()) {
        ret = QIcon(fileInfo.absoluteFilePath());
    }

    if (ret.isNull()) {
        ret = QIcon::fromTheme(iconName);
    }

    return ret;
}
