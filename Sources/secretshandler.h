#ifndef SECRETSHANDLER_H
#define SECRETSHANDLER_H

#include "ui_mainwindow.h"
#include "secretcard.h"
#include <QObject>


//Secretos
#define AVENGE QString("FP1_020")
#define NOBLE_SACRIFICE QString("EX1_130")
#define REPENTANCE QString("EX1_379")
#define REDEMPTION QString("EX1_136")
#define EYE_FOR_AN_EYE QString("EX1_132")

#define FREEZING_TRAP QString("EX1_611")
#define EXPLOSIVE_TRAP QString("EX1_610")
#define SNIPE QString("EX1_609")
#define MISDIRECTION QString("EX1_533")
#define SNAKE_TRAP QString("EX1_554")

#define MIRROR_ENTITY QString("EX1_294")
#define DUPLICATE QString("FP1_018")
#define ICE_BARRIER QString("EX1_289")
#define VAPORIZE QString("EX1_594")
#define COUNTERSPELL QString("EX1_287")
#define SPELLBENDER QString("tt_010")
#define ICE_BLOCK QString("EX1_295")


class ActiveSecret
{
public:
    int id;
    SecretCard root;
    QList<SecretCard> children;
};


class SecretsHandler : public QObject
{
    Q_OBJECT
public:
    SecretsHandler(QObject *parent, Ui::MainWindow *ui);
    ~SecretsHandler();

//Variables
private:
    Ui::MainWindow *ui;
    QList<ActiveSecret> activeSecretList;


//Metodos
private:
    void completeUI();
    void discardSecretOption(QString code);
    void checkLastSecretOption(ActiveSecret activeSecret);

public:
    void redrawDownloadedCardImage(QString code);

signals:
    void checkCardImage(QString code);

public slots:
    void secretPlayed(int id, SecretHero hero);
    void secretRevealed(int id);
    void resetSecretsInterface();
    void playerSpellPlayed();
    void playerSpellObjPlayed();
    void playerMinionPlayed();
    void enemyMinionDead();
    void avengeTested();
    void playerAttack(bool isHeroFrom, bool isHeroTo);
};

#endif // SECRETSHANDLER_H