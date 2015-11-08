#ifndef SQLITEMANAGER_H
#define SQLITEMANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <vector>
#include <algorithm>

#include "sentence.h"

using namespace std;

namespace level{
    const static int commom = 0;
    const static int easy = 1;
    const static int hard = 2;
}

class sqliteManager
{
private:
    QSqlDatabase db;

    bool initTable();

    enum countMode{
        begin = 0,
        all = 0,
        studied,
        easy,
        hard,
        end = 3,
    };

public:
    sqliteManager();
    virtual ~sqliteManager();

    int getAllCount(countMode mode = all);
    int getStudiedCount();
    int getEasyCount();
    int getHardCount();
    int checkSentence(QString eSentence);
    bool insertSentence(QString eSentence, QString kSentence, QString word, QString word2);
    bool getStudySentences(vector<Sentence> & studyList, bool isAll, bool isEasy, bool isHard);
    bool setStudyResult(QString eSentence, bool isCorrect);
    bool searchSentence(vector<Sentence>& searchList, vector<int>& idList, QString eSentence);
    bool modifySentence(int id, QString eSentence, QString kSentence, QString word, QString word2);
    bool deleteSentence(int id);

    int getSentenceLevel(QString eSentence);
};

#endif // SQLITEMANAGER_H
