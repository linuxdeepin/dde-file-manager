#ifndef MIMEUTILS_H
#define MIMEUTILS_H

#include "properties.h"

#include <QFileInfo>

/**
 * @class MimeUtils
 * @brief Helps with mime type management
 * @author Michal Rost
 * @date 29.4.2013
 */
class MimeUtils : public QObject {
  Q_OBJECT
public:
  explicit MimeUtils(QObject* parent = 0);
  virtual ~MimeUtils();
  void openInApp(QString exe, const QFileInfo &file, QObject* processOwner = 0);
  void openInApp(const QFileInfo &file, QObject* processOwner = 0);
  void setDefaultsFileName(const QString &fileName);
  void setDefault(const QString &mime, const QStringList &apps);
  QStringList getDefault(const QString &mime) const;
  QStringList getMimeTypes() const;
  QString getMimeType(const QString &path);
  QString getDefaultsFileName() const;
public slots:
  void generateDefaults();
  void saveDefaults();
  void loadDefaults();
private:
  bool defaultsChanged;
  QString defaultsFileName;
  Properties* defaults;
};

#endif // MIMEUTILS_H
