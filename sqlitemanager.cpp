#include "sqlitemanager.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlResult>
#include <QSqlRecord>
#include <ctime>
#include <cstdlib>
#include <assert.h>

#include <QDir>

sqliteManager::sqliteManager()
{
    if( !QDir("C:\\EnStudy\\db").exists() ){
        QDir().mkpath("C:\\EnStudy\\db");
    }

    db = QSqlDatabase::addDatabase("QSQLITE");

    // For random shuffle
    std::srand(unsigned (std::time(0)));
}

sqliteManager::~sqliteManager(){
}

bool sqliteManager::initTable(){
    if( !db.open() ){
        return false;
    }

    QSqlQuery qry;
    qry.prepare("CREATE TABLE IF NOT EXISTS sentence (id INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT, english blob not null,"
                " korean blob not null, word blob not null, word2 blob not null, correct int not null, incorrect int not null"
                ", studied boolean not null, time text not null)");
    if( !qry.exec() ){
        db.close();
        return false;
    }

    db.close();
    return true;
}

void sqliteManager::initializeDB(QString dbName){
    db.setDatabaseName(QString("C:\\EnStudy\\db\\").append(dbName));
    if( !initTable() )
        QMessageBox::information(NULL, "Notification", "The sqlite is not working! contact to developer");
}

int sqliteManager::getAllCount(countMode mode){
    assert(mode >= begin && mode <= end);

    int res = 0;

    if( !db.open() )
        return -1;

    QString queryMessage = "select count(id) from sentence";

    if( mode == studied )
        queryMessage.append(" where studied == 1");
    else if( mode == not_studied )
        queryMessage.append(" where studied == 0");
    else if( mode == easy )
        queryMessage.append(" where (correct-incorrect) >= 5");
    else if( mode == hard )
        queryMessage.append(" where (correct-incorrect) <= -3");
    else if( mode == newly )
        queryMessage.append(" where time==date('now')");

    QSqlQuery qry;
    qry.prepare(queryMessage);

    if( !qry.exec() ){
        db.close();
        return -1;
    }

    if( qry.next() )
        res = qry.value(0).toInt();

    db.close();
    return res;
}

int sqliteManager::getStudiedCount(){
    return getAllCount(studied);
}

int sqliteManager::getNotStudiedCount(){
    return getAllCount(not_studied);
}

int sqliteManager::getEasyCount(){
    return getAllCount(easy);
}

int sqliteManager::getHardCount(){
    return getAllCount(hard);
}

int sqliteManager::getNewCount(){
    return getAllCount(newly);
}

int sqliteManager::checkSentence(QString eSentence){
    if( !db.open() )
        return -1;

    QSqlQuery qry;
    qry.prepare(QString("select id from sentence where english =='%1'").arg(eSentence));
    if( !qry.exec() ){
        db.close();
        return -1;
    }

    db.close();
    return qry.next();
}

bool sqliteManager::insertSentence(QString eSentence, QString kSentence, QString word, QString word2){
    if( !db.open() )
        return false;

    QSqlQuery qry;
    qry.prepare(QString("INSERT INTO sentence (english,korean,word,word2,correct,incorrect,studied) VALUES ('%1','%2','%3','%4',0,0,0)")
                .arg(eSentence, kSentence, word, word2));
    if( !qry.exec() ){
        db.close();
        return false;
    }

    db.close();
    return true;
}

bool sqliteManager::getStudySentences(vector<Sentence> & studyList, bool isAll, bool isStudied, bool isNotStudied, bool isHard, bool isNew){
    if( !db.open() )
        return false;

    QString queryMessage = "select * from sentence";

    if( !isAll ){
        if(isStudied)
            queryMessage.append(" where studied==1");
        else if(isNotStudied)
            queryMessage.append(" where studied==0");
        else if(isHard)
            queryMessage.append(" where (correct-incorrect) <= -3");
        else if(isNew)
            queryMessage.append(" where time==date('now')");
    }

    QSqlQuery qry;
    qry.prepare(queryMessage);

    if( !qry.exec() ){
        db.close();
        return false;
    }

    studyList.clear();

    while(qry.next()){
        studyList.push_back(Sentence(qry.value("english").toString(),
                                     qry.value("korean").toString(),
                                     qry.value("word").toString(),
                                     qry.value("word2").toString()));
    }

    random_shuffle(studyList.begin(), studyList.end());

    db.close();
    return true;
}

bool sqliteManager::setStudyResult(QString eSentence, bool isCorrect){
    if( !db.open() )
        return false;

    QString queryMessage = QString("update sentence set studied=%1, correct=correct+%2, incorrect=incorrect+%3 where english == '%4'")
            .arg(QString::number((int)isCorrect), QString::number((int)isCorrect), QString::number((int)!isCorrect), eSentence);

    QSqlQuery qry;
    qry.prepare(queryMessage);
    if( !qry.exec() ){
        db.close();
        return false;
    }

    db.close();
    return true;
}

bool sqliteManager::searchSentence(vector<Sentence>& searchList, vector<int>& idList, QString eSentence){
    if( !db.open() )
        return false;

    QSqlQuery qry;
    qry.prepare("select * from sentence where english LIKE '" + eSentence + "%'");
    if( !qry.exec() ){
        db.close();
        return false;
    }

    searchList.clear();
    idList.clear();

    while( qry.next() ){
        searchList.push_back(Sentence(qry.value("english").toString(),
                                     qry.value("korean").toString(),
                                     qry.value("word").toString(),
                                     qry.value("word2").toString()));
        idList.push_back(qry.value("id").toInt());
    }

    return true;
}

bool sqliteManager::modifySentence(int id, QString eSentence, QString kSentence, QString word, QString word2){
    if( !db.open() )
        return false;

    QSqlQuery qry;
    qry.prepare(QString("update sentence set english='%1', korean='%2', word='%3', word2='%4' where id=%5")
                .arg(eSentence, kSentence, word, word2, QString::number(id)));
    if( !qry.exec() ){
        db.close();
        qDebug() << qry.lastQuery();
        return false;
    }

    return true;
}

bool sqliteManager::deleteSentence(int id){
    if( !db.open() )
        return false;

    QSqlQuery qry;
    qry.prepare(QString("delete from sentence where id=%1").arg(QString::number(id)));
    if( !qry.exec() ){
        db.close();
        return false;
    }
    return true;
}

int sqliteManager::getSentenceLevel(QString eSentence){
    if( !db.open() )
        return -1;

    QSqlQuery qry;
    qry.prepare(QString("select correct, incorrect from sentence where english =='%1'").arg(eSentence));
    if( !qry.exec() ){
        db.close();
        return -1;
    }

    int correct = 0, incorrect = 0;
    int res = level::commom;

    if( qry.next() ){
        correct = qry.value("correct").toInt();
        incorrect = qry.value("incorrect").toInt();
    }

    db.close();

    if( (correct-incorrect) >= 5 )
        res = level::easy;
    else if( (correct-incorrect) <= -3 )
        res = level::hard;

    return res;
}
