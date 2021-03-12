/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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
  QString getDisplayName() const;
  QString getExec() const;
  QString getIcon() const;
  QString getType() const;
  QString getDeepinId() const;
  QString getDeepinVendor() const;
  bool getNoShow() const;
  QStringList getCategories() const;
  QStringList getMimeType() const;
private:
  QString m_fileName;
  QString m_name;
  QString m_genericName;
  QString m_localName;
  QString m_exec;
  QString m_icon;
  QString m_type;
  QStringList m_categories;
  QStringList m_mimeType;
  QString m_deepinId;
  QString m_deepinVendor;
  bool m_noDisplay = false;
  bool m_hidden = false;
};

#endif // DESKTOPFILE_H
