#include "webmanager.h"

#include <QDebug>
#include <QByteArray>
#include <QMessageBox>

QWebManager::QWebManager() :
    isFile(false),
    resultString(""),
    isRunning(false),
    index(-1) {
}

QWebManager::~QWebManager(){

}

void QWebManager::request(QString url, bool isFile, QString dstPath){
    this->isFile = isFile;

    if( isFile ){
        file = new QFile(dstPath);
        if( !file->open(QIODevice::WriteOnly) ){
        }
    }

    reply = qnam.get(QNetworkRequest(QUrl(url)));

    connect(reply, SIGNAL(finished()), this, SLOT(httpFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(httpProgress(qint64,qint64)));
}

void QWebManager::requestPush(QString url, bool isFile, QString dstPath){
    requestList.push_back(RequestData(url, isFile, dstPath));
}

void QWebManager::requestRun(){
    isRunning = true;
    index = 0;
    request(requestList[index].url, requestList[index].isFile, requestList[index].dstPath);
}

QString QWebManager::getResultString(){
    return resultString;
}

void QWebManager::httpFinished(){
    QString dstPath = "";

    if( !isFile ){
        QByteArray res = reply->readAll();
        resultString = QString::fromLocal8Bit(res.data());
    }
    else{
        if( file ){
            file->flush();
            file->close();
        }
    }

    if( isRunning ){
        dstPath = requestList[index].dstPath;
        index++;
        if( index >= requestList.size() ){
            index = -1;
            isRunning = false;
            requestList.clear();
        }
        else{
            request(requestList[index].url, requestList[index].isFile, requestList[index].dstPath);
        }
    }

    // bypass message to user defined dialog
    emit notifyFinished(dstPath);
}

void QWebManager::httpReadyRead(){
    // actual file transfer operation.
    if( isFile && file )
        file->write(reply->readAll());
}

void QWebManager::httpProgress(qint64 bytesRead, qint64 totalBytes){
    // bypass message to user defined dialog
    QString dstPath = "";

    if( isRunning )
        dstPath = requestList[index].dstPath;

    emit notifyProgress(dstPath, bytesRead, totalBytes);
}
