#ifndef FILEMANAGER
#define FILEMANAGER

#include <QString>

namespace FileManager{
    void iniWrite(QString key, QString value);
    QString iniRead(QString key);
    QStringList dbList();
}

#endif // FILEMANAGER

