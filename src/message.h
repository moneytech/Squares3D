#ifndef __MESSAGE__H__
#define __MESSAGE__H__

#include "common.h"
#include "font.h"
#include "vmath.h"

class Messages;
class Font;

class Message : NoCopy
{
    friend class Messages;
public:
    Message(
        const wstring&  message, 
        const Vector&   position, 
        const Vector&   color, 
        Font::AlignType align = Font::Align_Left,
        int             fontSize = 32);
    virtual ~Message() {}

    const Vector&   getPosition() const { return m_position; }

    virtual wstring getText() const;
    virtual int     getFontSize() const { return m_fontSize; }
    
protected:
   
    virtual void applyFlow(float delta);

    virtual bool applyDelta(float delta);

    virtual void Message::render(const Font* font) const;

    wstring         m_text;
    Vector          m_position;
    Vector          m_color;
    Font::AlignType m_align;
    int             m_fontSize;
};

class BlinkingMessage : public Message
{
public:
    BlinkingMessage(
        const wstring&  message, 
        const Vector&   position, 
        const Vector&   color, 
        Font::AlignType align = Font::Align_Left,
        int             fontSize = 32,
        float           blinkIntensity = 0.2f);
    virtual ~BlinkingMessage() {}

protected:
    bool  m_visible;
    float m_blinkCurrent;
    float m_blinkIntensity;
    bool applyDelta(float delta);
    void render(const Font* font) const;
};

class FlowingMessage : public Message
{
public:
    FlowingMessage(
        const wstring&  message, 
        const Vector&   position, 
        const Vector&   color, 
        Font::AlignType align = Font::Align_Left,
        float           timeToLive = 2.0f);
    virtual ~FlowingMessage() {}

protected:

    void applyFlow(float delta);

    bool applyDelta(float delta);

    float m_timeToLive;
};


class ScoreMessage : public Message
{
public:
    ScoreMessage(
        const wstring&  message, 
        const Vector&   position, 
        const Vector&   color,
        int             score,
        Font::AlignType align = Font::Align_Left);
    virtual ~ScoreMessage() {}

    int m_score;

protected:

    wstring getText() const;
};

class ComboMessage : public Message
{
public:
    ComboMessage(
        const wstring&  message, 
        const Vector&   position, 
        const Vector&   color,
        int             points,
        Font::AlignType align = Font::Align_Center,
        int             fontSize = 32);
    virtual ~ComboMessage() {}

    bool applyDelta(float delta);
    bool render();

    int m_points;

protected:
    int m_previousPoints;
    wstring getText() const;
};

#endif
