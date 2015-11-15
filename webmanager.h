#ifndef WEBMANAGER
#define WEBMANAGER

#include <QObject>
#include <QThread>

#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QFile>

#include <vector>

using namespace std;

class RequestData{
public:
    QString url;
    bool isFile;
    QString dstPath;

    RequestData(QString url, bool isFile, QString dstPath) :
        url(url),
        isFile(isFile),
        dstPath(dstPath){
    }
};

class QWebManager : public QObject {
    Q_OBJECT

private:
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QFile *file;
    bool isFile;
    QString resultString;

    bool isRunning;
    int index;

    vector<RequestData> requestList;

private slots:
    void httpFinished();
    void httpReadyRead();
    void httpProgress(qint64 bytesRead, qint64 totalBytes);

signals:
    void notifyFinished(QString dst);
    void notifyProgress(QString dst, qint64 bytesRead, qint64 totalBytes);

public:
    QWebManager();
    virtual ~QWebManager();

    void request(QString url, bool isFile, QString dstPath = "");
    void requestPush(QString url, bool isFile, QString dstPath);
    void requestRun();
    QString getResultString();
};

#endif // WEBMANAGER

