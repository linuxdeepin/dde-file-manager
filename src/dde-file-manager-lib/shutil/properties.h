/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QVariant>
#include <QObject>
#include <QMap>

/**
 * @class Properties
 * @brief Read property files
 * @author Michal Rost
 * @date 26.1.2013
 */
class Properties {
public:
  explicit Properties(const QString &fileName = "", const QString &group = "");
  explicit Properties(const Properties &other);
  QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
  bool load(const QString &fileName, const QString &group = "");
  bool save(const QString &fileName, const QString &group = "");
  void set(const QString &key, const QVariant &value);
  bool contains(const QString &key) const;
  QStringList getKeys() const;
protected:
  QMap<QString, QVariant> data;
};

#endif // PROPERTIES_H
