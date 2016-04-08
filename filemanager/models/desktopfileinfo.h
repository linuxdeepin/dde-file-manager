#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "fileinfo.h"

#include <QObject>

#define DESKTOP_SURRIX "desktop"

class DesktopFileInfo : public FileInfo
{
public:
    DesktopFileInfo();
    DesktopFileInfo(const QString & fileUrl);
    DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo();

    void setFile(const QString &fileUrl) Q_DECL_OVERRIDE;

    QString getName() const;
    QString getExec() const;
    QString getIconName() const;
    QString getType() const;
    QStringList getCategories() const;

    QIcon fileIcon() const Q_DECL_OVERRIDE;

    static QMap<QString, QVariant> getDesktopFileInfo(const QString &fileUrl);

private:
    QString name;
    QString exec;
    QString iconName;
    QString type;
    QStringList categories;
    QStringList mimeType;

    void init(const QString &fileUrl);
};

#endif // DESKTOPFILEINFO_H
