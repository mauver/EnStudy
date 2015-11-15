#include "filemanager.h"

#include <QSettings>
#include <QDir>
#include <QDebug>

namespace FileManager{
    const static QString settingPath = "C:\\EnStudy\\settings.ini";
    static QSettings settings(settingPath, QSettings::IniFormat);

    const static QString dbFolder = "C:\\EnStudy\\db";

    void iniWrite(QString key, QString value){
        settings.setValue(key, value);
        settings.sync();
    }

    QString iniRead(QString key){
        return settings.value(key).toString();
    }

    QStringList dbList(){
        QStringList list;

        QDir dir(dbFolder);
        QStringList temp = dir.entryList();

        foreach(QString t, temp){
            QFileInfo info(t);

            if( info.completeSuffix().compare("sqlite") == 0 )
                list.push_back(t);
        }

        return list;
    }
}
