#ifndef SENTENCE_H
#define SENTENCE_H

#include <QObject>
#include <QString>

class Sentence
{
public:
    QString english, korean, word, word2;

public:
    Sentence():
        english(""),
        korean(""),
        word(""),
        word2("")
    {}
    Sentence(QString english, QString korean, QString word, QString word2) :
        english(english),
        korean(korean),
        word(word),
        word2(word2)
    {}
};

#endif // SENTENCE_H
