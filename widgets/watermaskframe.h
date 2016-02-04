#ifndef WATERMASKFRAME_H
#define WATERMASKFRAME_H

#include <QFrame>
#include <QJsonObject>
#include <QLabel>
#include <QGSettings>

class WaterMaskFrame : public QFrame
{
    Q_OBJECT

public:
    static const QString WallpaperKey;
    static const QString DefaultWallpaper;
    explicit WaterMaskFrame(const QString& fileName, QWidget *parent = 0);
    ~WaterMaskFrame();
    bool checkConfigFile(const QString& fileName);
    void loadConfig(const QString& fileName);
    void initUI();
signals:

public slots:
    void handleBackgroundChanged(const QString &key);

private:
    QString m_configFile;
    QJsonObject m_configs;
    QLabel* m_logoLabel = NULL;
    QLabel* m_textLabel = NULL;
    QGSettings* m_gsettings;
    QString m_backgroundUrl;
    bool m_isMaskAlwaysOn = false;
};

#endif // WATERMASKFRAME_H
