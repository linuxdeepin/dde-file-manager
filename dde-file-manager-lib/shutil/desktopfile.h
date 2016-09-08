#ifndef DESKTOPFILE_H
#define DESKTOPFILE_H

#include <QStringList>

/**
 * @class DesktopFile
 * @brief Represents a linux desktop file
 * @author Michal Rost
 * @date 13.1.2013
 */
class DesktopFile {
public:
  explicit DesktopFile(const QString &fileName = "");
  QString getFileName() const;
  QString getPureFileName() const;
  QString getName() const;
  QString getLocalName() const;
  QString getExec() const;
  QString getIcon() const;
  QString getType() const;
  QStringList getCategories() const;
  QStringList getMimeType() const;
private:
  QString m_fileName;
  QString m_name;
  QString m_localName;
  QString m_exec;
  QString m_icon;
  QString m_type;
  QStringList m_categories;
  QStringList m_mimeType;
};

#endif // DESKTOPFILE_H
