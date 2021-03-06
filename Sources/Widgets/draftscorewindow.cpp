#include "draftscorewindow.h"
#include "../themehandler.h"
#include <QtWidgets>


DraftScoreWindow::DraftScoreWindow(QWidget *parent, QRect rect, QSize sizeCard, int screenIndex) :
    QMainWindow(parent, Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint)
{
    scoreWidth = sizeCard.width()*0.7;

    QList<QScreen *> screens = QGuiApplication::screens();
    QScreen *screen = screens[screenIndex];
    if (!screen) return;

    QRect rectScreen = screen->geometry();
    int midCards = (rect.width() - 3*sizeCard.width())/2;
    resize(rect.width() + 2*MARGIN + midCards,
           rect.height() + 2*MARGIN - (sizeCard.height()-scoreWidth));
    move(rectScreen.x() + rect.x() - MARGIN - midCards/2,
         rectScreen.y() + rect.y() - MARGIN + 2.65*sizeCard.height());
    int synergyWidth = this->width()/3-10;  //List Widget need 10 px (maybe 11px) extra space more than the sizeCard.


    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *verLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *horLayoutScores = new QHBoxLayout();
    QHBoxLayout *horLayoutSynergies = new QHBoxLayout();
    verLayout->addLayout(horLayoutScores);
    verLayout->addLayout(horLayoutSynergies);

    for(int i=0; i<3; i++)
    {
        scoresPushButton[i] = new ScoreButton(centralWidget, LightForge);
        scoresPushButton[i]->setFixedHeight(scoreWidth);
        scoresPushButton[i]->setFixedWidth(scoreWidth);
        connect(scoresPushButton[i], SIGNAL(spreadLearningShow(bool)),
                this, SLOT(spreadLearningShow(bool)));

        scoresPushButton2[i] = new ScoreButton(centralWidget, HearthArena);
        scoresPushButton2[i]->setFixedHeight(scoreWidth);
        scoresPushButton2[i]->setFixedWidth(scoreWidth);
        connect(scoresPushButton2[i], SIGNAL(spreadLearningShow(bool)),
                this, SLOT(spreadLearningShow(bool)));

        //Opacity effects
        QGraphicsOpacityEffect *effect;
        effect = new QGraphicsOpacityEffect(scoresPushButton[i]);
        effect->setOpacity(0);
        scoresPushButton[i]->setGraphicsEffect(effect);
        effect = new QGraphicsOpacityEffect(scoresPushButton2[i]);
        effect->setOpacity(0);
        scoresPushButton2[i]->setGraphicsEffect(effect);

        horLayoutScores->addStretch();
        horLayoutScores->addWidget(scoresPushButton[i]);
        horLayoutScores->addWidget(scoresPushButton2[i]);
        horLayoutScores->addStretch();


        QVBoxLayout *verLayoutSynergy = new QVBoxLayout();

        horLayoutMechanics[i] = new QHBoxLayout();

        synergiesListWidget[i] = new MoveListWidget(centralWidget);
        synergiesListWidget[i]->setFixedHeight(0);
        synergiesListWidget[i]->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        synergiesListWidget[i]->setIconSize(QSize(synergyWidth, synergyWidth/218.0*35));
        synergiesListWidget[i]->setMouseTracking(true);
        hideSynergies(i);

        connect(synergiesListWidget[i], SIGNAL(itemEntered(QListWidgetItem*)),
                this, SLOT(findSynergyCardEntered(QListWidgetItem*)));
        connect(synergiesListWidget[i], SIGNAL(leave()),
                this, SIGNAL(cardLeave()));

        verLayoutSynergy->addLayout(horLayoutMechanics[i]);
        verLayoutSynergy->addWidget(synergiesListWidget[i]);
        verLayoutSynergy->addStretch();

        horLayoutSynergies->addStretch();
        horLayoutSynergies->addLayout(verLayoutSynergy);
        horLayoutSynergies->addStretch();
    }

    maxSynergyHeight = rectScreen.y() + rectScreen.height() - this->y() - 2*MARGIN - 2*scoreWidth; //Extra scoreWidth
    setCentralWidget(centralWidget);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowTitle("AT Scores");
}


DraftScoreWindow::~DraftScoreWindow()
{
    for(int i=0; i<3; i++)
    {
        synergiesListWidget[i]->clear();
        synergiesDeckCardLists[i].clear();
    }
}


void DraftScoreWindow::setLearningMode(bool value)
{
    for(int i=0; i<3; i++)
    {
        scoresPushButton[i]->setLearningMode(value);
        scoresPushButton2[i]->setLearningMode(value);
    }
}


void DraftScoreWindow::spreadLearningShow(bool value)
{
    for(int i=0; i<3; i++)
    {
        scoresPushButton[i]->setLearningShow(value);
        scoresPushButton2[i]->setLearningShow(value);
    }
}


void DraftScoreWindow::setDraftMethod(DraftMethod draftMethod)
{
    switch(draftMethod)
    {
        case All:
            for(int i=0; i<3; i++)
            {
                scoresPushButton[i]->show();
                scoresPushButton2[i]->show();
            }
            break;
        case LightForge:
            for(int i=0; i<3; i++)
            {
                scoresPushButton[i]->show();
                scoresPushButton2[i]->hide();
            }
            break;
        case HearthArena:
            for(int i=0; i<3; i++)
            {
                scoresPushButton[i]->hide();
                scoresPushButton2[i]->show();
            }
            break;
        default:
            for(int i=0; i<3; i++)
            {
                scoresPushButton[i]->hide();
                scoresPushButton2[i]->hide();
            }
            break;
    }
}


void DraftScoreWindow::setScores(double rating1, double rating2, double rating3,
                                 DraftMethod draftMethod)
{
    double bestRating = std::max(std::max(rating1, rating2), rating3);
    double ratings[3] = {rating1, rating2, rating3};

    for(int i=0; i<3; i++)
    {
        if(draftMethod == LightForge)
        {
            scoresPushButton[i]->setScore(ratings[i], ratings[i]==bestRating);
            Utility::fadeInWidget(scoresPushButton[i]);
        }
        else if(draftMethod == HearthArena)
        {
            scoresPushButton2[i]->setScore(ratings[i], ratings[i]==bestRating);
            QPropertyAnimation *animation = Utility::fadeInWidget(scoresPushButton2[i]);

            if(i==0 && animation!=NULL)     connect(animation, SIGNAL(finished()), this, SLOT(showSynergies()));
        }
    }
}


void DraftScoreWindow::setSynergies(int posCard, QMap<QString,int> &synergies, QStringList &mechanicIcons)
{
    if(posCard < 0 || posCard > 2)  return;

    synergiesListWidget[posCard]->clear();
    synergiesDeckCardLists[posCard].clear();

    QMap<int,DeckCard> deckCardMap;
    for(const QString &code: synergies.keys())
    {
        int total = synergies[code];
        DeckCard deckCard(code);
        deckCard.total = deckCard.remaining = total;
        deckCardMap.insertMulti(deckCard.getCost(), deckCard);
    }

    for(DeckCard &deckCard: deckCardMap.values())
    {
        deckCard.listItem = new QListWidgetItem(synergiesListWidget[posCard]);
        deckCard.draw();
        synergiesDeckCardLists[posCard].append(deckCard);
    }


    //Add mechanic icons
    Utility::clearLayout(horLayoutMechanics[posCard], true);
    horLayoutMechanics[posCard]->addStretch();

    for(const QString &mechanicIcon: mechanicIcons)
    {
        QLabel *label = new QLabel();
        QPixmap pixmap(mechanicIcon);
        label->setPixmap(pixmap.scaledToWidth(scoreWidth/2,Qt::SmoothTransformation));
        label->setToolTip(getMechanicTooltip(mechanicIcon));
        label->hide();
        horLayoutMechanics[posCard]->addWidget(label);
    }

    horLayoutMechanics[posCard]->addStretch();
}


QString DraftScoreWindow::getMechanicTooltip(QString iconName)
{
    if(iconName == ThemeHandler::aoeMechanicFile())             return "AOE";
    else if(iconName == ThemeHandler::tauntMechanicFile())      return "Taunt";
    else if(iconName == ThemeHandler::survivalMechanicFile())   return "Survival";
    else if(iconName == ThemeHandler::drawMechanicFile())       return "Draw";
    else if(iconName == ThemeHandler::pingMechanicFile())       return "Ping";
    else if(iconName == ThemeHandler::damageMechanicFile())     return "Removal";
    else if(iconName == ThemeHandler::destroyMechanicFile())    return "Hard\nRemoval";
    else if(iconName == ThemeHandler::reachMechanicFile())      return "Reach";
    else    return "";
}


int DraftScoreWindow::getCard(QString &name, QString &code)
{
    //Mechwarper (2x)
    QRegularExpressionMatch match;
    if(name.contains(QRegularExpression("^(.*[^ ]) +\\((\\d+)x\\)$"), &match))
    {
        code = Utility::cardEnCodeFromName(match.captured(1));
        return match.captured(2).toInt();
    }
    else
    {
        code = Utility::cardEnCodeFromName(name);
        return 1;
    }
}


void DraftScoreWindow::hideScores(bool quick)
{
    for(int i=0; i<3; i++)
    {
        if(quick)
        {
            QGraphicsOpacityEffect *eff = (QGraphicsOpacityEffect *)scoresPushButton[i]->graphicsEffect();
            eff->setOpacity(0);
            eff = (QGraphicsOpacityEffect *)scoresPushButton2[i]->graphicsEffect();
            eff->setOpacity(0);
        }
        else
        {
            QPropertyAnimation *animation = Utility::fadeOutWidget(scoresPushButton[i]);
            Utility::fadeOutWidget(scoresPushButton2[i]);

            if(i==0 && animation!=NULL)     connect(animation, SIGNAL(finished()), this, SLOT(update()));
        }

        hideSynergies(i);
    }
    this->update();
}


void DraftScoreWindow::showSynergies()
{
    resizeSynergyList();

    for(int i=0; i<3; i++)
    {
        synergiesListWidget[i]->show();
        Utility::showItemsLayout(horLayoutMechanics[i]);
    }
    this->update();
}


void DraftScoreWindow::hideSynergies(int index)
{
    synergiesListWidget[index]->hide();
    synergiesListWidget[index]->clear();
    synergiesDeckCardLists[index].clear();
    Utility::clearLayout(horLayoutMechanics[index], true);
}


void DraftScoreWindow::redrawSynergyCards()
{
    for(int i=0; i<3; i++)
    {
        for(DeckCard &deckCard: synergiesDeckCardLists[i])
        {
            deckCard.draw();
        }
    }
    resizeSynergyList();
}


void DraftScoreWindow::resizeSynergyList()
{
    int width = 0;
    for(int i=0; i<3; i++)
    {
        QListWidget *list = synergiesListWidget[i];
        int rowHeight = list->sizeHintForRow(0);
        int rows = list->count();
        int height = rows*rowHeight + 2*list->frameWidth();
        if(height>maxSynergyHeight)    height = maxSynergyHeight;

        list->setFixedHeight(height);
        if(rows>0)  width = list->sizeHintForColumn(0) + 2 * list->frameWidth();
    }

    if(width>0)
    {
        for(int i=0; i<3; i++)
        {
            synergiesListWidget[i]->setFixedWidth(width);
        }
    }
}


void DraftScoreWindow::findSynergyCardEntered(QListWidgetItem * item)
{
    //Detect synergy list
    QListWidget * listWidget = item->listWidget();
    int indexList = -1;

    for(int i=0; i<3; i++)
    {
        if(synergiesListWidget[i] == listWidget)
        {
            indexList = i;
            break;
        }
    }
    if(indexList == -1) return;

    QString code = synergiesDeckCardLists[indexList][listWidget->row(item)].getCode();

    QRect rectCard = listWidget->visualItemRect(item);
    QPoint posCard = listWidget->mapToGlobal(rectCard.topLeft());
    QRect globalRectCard = QRect(posCard, rectCard.size());

    int synergyListTop = listWidget->mapToGlobal(QPoint(0,0)).y();
    int synergyListBottom = listWidget->mapToGlobal(QPoint(0,listWidget->height())).y();
    emit cardEntered(code, globalRectCard, synergyListTop, synergyListBottom);
}



