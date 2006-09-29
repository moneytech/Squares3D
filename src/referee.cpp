#include "referee.h"
#include "player.h"
#include "body.h"
#include "ball.h"
#include "geometry.h"
#include "messages.h"
#include "message.h"
#include "scoreboard.h"
#include "video.h"
#include "language.h"

bool isBallInField(const Vector& position, 
                   const Vector& lowerLeft, 
                   const Vector& upperRight,
                   const bool    useMiddleLines = true,
                   const bool    useOutterLines = false)
{
    //just for the real field - excluding middle line

    Vector _lowerLeft(lowerLeft);
    Vector _upperRight(upperRight);

    //find out which of the 4 fields we are in
    Vector center = getSquareCenter(_lowerLeft, _upperRight);

    int quadrant = getQuadrant(center);

    //TODO: make universaly proportional to field size?
    float lineWeight = 0.2f;

    //adjust the field size to take middle line into account
    switch(quadrant)
    {
    case 1: if (useMiddleLines)
            {
                _lowerLeft.x += lineWeight;
                _lowerLeft.z += lineWeight;
            }
            if (useOutterLines)
            {
                _upperRight.x -= lineWeight;
                _upperRight.z -= lineWeight;
            }
            break;
    case 2: if (useMiddleLines)
            {
                _upperRight.x -= lineWeight;
                _lowerLeft.z += lineWeight;
            }
            if (useOutterLines)
            {
                _lowerLeft.x += lineWeight;
                _upperRight.z -= lineWeight;
            }
            break;
    case 3: if (useMiddleLines)
            {
                _upperRight.x -= lineWeight;
                _upperRight.z -= lineWeight;
            }
            if (useOutterLines)
            {
                _lowerLeft.x += lineWeight;
                _lowerLeft.z += lineWeight;
            }
            break;
    case 4: if (useMiddleLines)
            {
                _lowerLeft.x += lineWeight;
                _upperRight.z -= lineWeight;
            }
            if (useOutterLines)
            {
                _lowerLeft.z += lineWeight;
                _upperRight.x -= lineWeight;
            }
            break;
    }

    //return the bool "if point is into adjusted field (middle line excluded)"
    return isPointInRectangle(position, _lowerLeft, _upperRight);
}

Referee::Referee(Messages* messages, ScoreBoard* scoreBoard): 
    m_gameOver(false),
    m_messages(messages),
    m_scoreBoard(scoreBoard),
    m_matchPoints(21),
    m_faultTime(0),
    m_mustResetBall(false),
    m_timer()
{
    initEvents();
}

void Referee::registerFaultTime()
{
    m_faultTime = m_timer.read();
    m_mustResetBall = false;
}

void Referee::update()
{
    if (m_mustResetBall && !m_gameOver)
    {
        if (m_timer.read() - m_faultTime > 1.0f)
        {
            resetBall();
            m_mustResetBall = false;
        }
    }
}

void Referee::resetBall()
{
    Vector resetPosition = Vector(0, 1.5f, 0);
    Vector velocity = Vector::Zero;

    if ((m_lastTouchedObject != NULL) 
     || (m_lastTouchedPlayer != NULL)
     || (m_lastFieldOwner != NULL))
    {
        //the game was in progress
        if (m_lastTouchedObject == NULL)
        {
            //middle line -> reset coords in center
            resetPosition = Vector(0, resetPosition.y * 3, 0);
        }
        else if (m_lastTouchedObject == m_ground)
        {
            //ball has left game field from middle line one of players fields
            //also happens when player touches twice
            //reset from last owner (m_lastFieldOwner)
            Vector center = m_players.find(m_lastFieldOwner)->second.second->getFieldCenter();
            //set the reset position to center of players field
            resetPosition = Vector(center.x, resetPosition.y, center.z);
            velocity = (Vector::Zero - resetPosition) * 2;
        }
        else
        {
            //ball has left game field from one of the players
            //reset from m_lastTouchedPlayer
            Vector center = m_players.find(m_lastTouchedPlayer)->second.second->getFieldCenter();
            //set the reset position to center of players field
            resetPosition = Vector(center.x, resetPosition.y, center.z);
            velocity = (Vector::Zero - resetPosition) * 2;
        }
    }
    else
    {
        //the game has just begun
        //reset coords in center and ball must hit the ground 3 times (TODO)
        //before it can be touched by players
        resetPosition = Vector(0, resetPosition.y * 3, 0);
    }

    //m_ball->set
    m_ball->setTransform(resetPosition, Vector::Zero);
    NewtonBodySetOmega(m_ball->m_newtonBody, Vector::Zero.v);
    NewtonBodySetVelocity(m_ball->m_newtonBody, velocity.v);
    initEvents();
}

void Referee::initEvents()
{
    m_lastFieldOwner    = NULL;
    m_lastTouchedObject = NULL;
    m_lastTouchedPlayer = NULL;
    m_scoreBoard->resetCombo();
}

void Referee::processCriticalEvent()
{
    StringIntPair maxScore = m_scoreBoard->getMostScoreData();
    if (maxScore.second >= m_matchPoints)
    {
        Vector center = Vector(static_cast<float>(Video::instance->getResolution().first) / 2,
                               static_cast<float>(Video::instance->getResolution().second) / 2,
                               0.0f);
        m_messages->add2D(new BlinkingMessage(Language::instance->get(TEXT_GAME_OVER), 
                                              center, 
                                              Vector(1,0,0), 
                                              Font::Align_Center,
                                              0.8f));
        m_gameOver = true;
    }
    else
    {
        registerFaultTime();
        m_mustResetBall = true;
    }

}

void Referee::registerBall(Ball* ball)
{
    ball->m_referee = this;
    m_ball = ball->m_body;
    resetBall();
}

void Referee::registerPlayer(const string& name, Player* player)
{
    m_players[player->m_body] = make_pair(name, player);
    m_scoreBoard->registerPlayer(name);
    player->m_referee = this;
}

void Referee::process(const Body* body1, const Body* body2)
{
    if (!(m_gameOver || m_mustResetBall))
    {
        if (body1 == m_ball)
        {
            registerBallEvent(body1, body2);
        }
        if (foundInMap(m_players, body1))
        {
            registerPlayerEvent(body1, body2);
        }
    }
}

void Referee::registerPlayerEvent(const Body* player, const Body* otherBody)
{
    if (otherBody == m_ground)
    {
        processPlayerGround(player);
    }
}

void Referee::registerBallEvent(const Body* ball, const Body* otherBody)
{
    if (otherBody == m_ground)
    {
        processBallGround();
    }
    else if (foundInMap(m_players, otherBody))
    {
        processBallPlayer(otherBody);
    }
}

void Referee::processPlayerGround(const Body* player)
{
    Vector playerPos = player->getPosition();
    string currentPlayerName = m_players.find(player)->second.first;
    Player* currentPlayer = m_players[player].second;
    if (!isPointInRectangle(playerPos, currentPlayer->m_lowerLeft, currentPlayer->m_upperRight)
        && isPointInRectangle(playerPos, 
                              Vector(- FIELDLENGTH, 0 , - FIELDLENGTH),
                              Vector(FIELDLENGTH, 0 , FIELDLENGTH)))
    {
        //clog << "pleijeris " + currentPlayerName + " ir iekaapis cita pleijera laukumaa!!.." << endl;
    }
}

void Referee::processBallGround()
{
    Vector ballPos(m_ball->getPosition());

    if (!isPointInRectangle(ballPos, 
                            Vector(- FIELDLENGTH, 0.0f, - FIELDLENGTH), 
                            Vector(FIELDLENGTH, 0.0f, FIELDLENGTH))) 
    {
        //critical event
        //BALL HAS HIT THE FIELD OUTSIDE
        //and we handle score here

        if (m_lastTouchedObject != NULL) //if last touched was not middle line
        {
            if (foundInMap(m_players, m_lastTouchedObject))
            {
                //player has kicked the ball out
                int points = m_scoreBoard->addSelfTotalPoints(m_players[m_lastTouchedObject].first);
                
                m_messages->add3D(new FlowingMessage(
                    Language::instance->get(TEXT_PLAYER_KICKS_OUT_BALL)(m_players[m_lastTouchedObject].first)(points),
                    m_lastTouchedObject->getPosition(),
                    Vector(1, 0, 0),
                    Font::Align_Center));
            }

            else if (m_lastFieldOwner != NULL) //if ground was touched in one of the players field last add points to owner
            {
                string owner = m_players[m_lastFieldOwner].first;
                int points;

                if (m_lastTouchedPlayer != NULL)
                {
                    string lastPlayerName = m_players[m_lastTouchedPlayer].first;
                    if (m_lastFieldOwner == m_lastTouchedPlayer)
                    {
                        points = m_scoreBoard->addSelfTotalPoints(owner);
                    }
                    else
                    {
                        points = m_scoreBoard->addTotalPoints(owner);
                    }
                }
                else
                {
                    //case when noone touched the ball after throwing minus
                    points = m_scoreBoard->addPoint(owner);
                }

                m_messages->add3D(new FlowingMessage(
                    Language::instance->get(TEXT_OUT_FROM_FIELD)(owner)(points),
                    m_ball->getPosition(),
                    Vector(1, 0, 0),
                    Font::Align_Center));
            }
        }
        else
        {
            m_messages->add3D(new FlowingMessage(
                    Language::instance->get(TEXT_OUT_FROM_MIDDLE_LINE),
                    m_ball->getPosition(),
                    Vector(1, 0, 0),
                    Font::Align_Center));
        }

        //finishing handling critical branch
        processCriticalEvent();
    }
    else
    {
        //non critical event - update events status
        //BALL HAS HIT THE FIELD INSIDE
        //save the touched field
        for each_const(BodyToPlayerDataMap, m_players, player)
        {
            //field excluding middle line
            if (isBallInField(ballPos, 
                              player->second.second->m_lowerLeft, 
                              player->second.second->m_upperRight))
            {
               m_lastFieldOwner =    player->first;
               m_lastTouchedObject = m_ground;
               break;
            }
            m_lastTouchedObject = NULL; //if has hit the middle line
        }
    }
}

void Referee::processBallPlayer(const Body* player)
{
    //ball + player

    string playerName = m_players[player].first;

    if (m_lastTouchedObject == NULL)
    {
        m_scoreBoard->resetCombo();
        m_scoreBoard->incrementCombo(playerName, m_ball->getPosition()); //(+1)
    }
    else //picked from ground or player
    {
        if (m_lastTouchedObject == m_ground) //picked from ground inside
        {
            m_scoreBoard->resetCombo(); //clear combo

            m_scoreBoard->incrementCombo(playerName, m_ball->getPosition()); //(+1)

            if ((m_lastFieldOwner == player)
                && (m_lastTouchedPlayer == player)
                && (isBallInField(m_ball->getPosition(), 
                                  m_players[player].second->m_lowerLeft, 
                                  m_players[player].second->m_upperRight,
                                  true,
                                  true)))
            {
                //critical event. double-touched -> fault

                int points = m_scoreBoard->addPoint(playerName);
                m_messages->add3D(new FlowingMessage(
                    Language::instance->get(TEXT_PLAYER_TOUCHES_TWICE)(playerName)(points),
                    player->getPosition(),
                    Vector(1, 0, 0),
                    Font::Align_Center));

                processCriticalEvent();
                //TODO: restructure ifs?
                goto end;
            }
        }
        else //picked from player
        {
            if (m_lastTouchedPlayer != player) //reset own combo, when picked from other
            {        
                m_scoreBoard->resetOwnCombo(playerName);
            }
            m_scoreBoard->incrementCombo(playerName, m_ball->getPosition()); //(+1)
        }
    }
    
    m_lastTouchedObject = player;
    m_lastTouchedPlayer = player;
end:;
    
}
