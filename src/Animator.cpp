#include "Animator.h"

using namespace sf;

Animator::Animator(const sf::Texture& texture, int frameWidth, int frameHeight, int startX)
    : m_texture(texture), m_startX(startX), m_frameW(frameWidth), m_frameH(frameHeight)
{
    m_sprite.setTexture(m_texture);
}

void Animator::addClip(const std::string& name, const std::vector<IntRect>& frames,
    Time frameTime, bool loop, bool flipped)
{
    Clip c;
    c.frames = frames;
    c.frameTime = frameTime;
    c.loop = loop;
    c.flipped = flipped;
    m_clips[name] = std::move(c);
}

void Animator::addClipRow(const std::string& name, int rowY, int startFrame, int count,
    Time frameTime, bool loop, bool reverse)
{
    std::vector<IntRect> frames;
    frames.reserve(count);
    for (int i = 0; i < count; ++i)
    {
        int idx = reverse ? startFrame + count - 1 - i : startFrame + i;
        frames.emplace_back(m_startX + idx * m_frameW, rowY, m_frameW, m_frameH);
    }
    addClip(name, frames, frameTime, loop, false);
}

void Animator::copyFlip(const std::string& left, const std::string& right)
{
    auto it = m_clips.find(left);
    if (it == m_clips.end())
        return;
    Clip c = it->second;
    c.flipped = true;
    m_clips[right] = std::move(c);
}

void Animator::setScale(const Vector2f& scale)
{
    m_scale = scale;
    applyScale();
}

void Animator::applyScale()
{
    if (m_current && m_current->flipped)
        m_sprite.setScale(-m_scale.x, m_scale.y);
    else
        m_sprite.setScale(m_scale);
}

void Animator::play(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it == m_clips.end())
        return;

    m_current = &it->second;
    m_currentName = name;
    m_index = 0;
    m_elapsed = Time::Zero;

    m_sprite.setTextureRect(m_current->frames[0]);
    m_sprite.setOrigin(static_cast<float>(m_frameW) / 2.f,
        static_cast<float>(m_frameH) / 2.f);
    applyScale();
}

void Animator::update(Time dt)
{
    if (!m_current)
        return;

    m_elapsed += dt;
    if (m_elapsed >= m_current->frameTime)
    {
        m_elapsed -= m_current->frameTime;
        ++m_index;
        if (m_index >= m_current->frames.size())
        {
            if (m_current->loop)
                m_index = 0;
            else
                m_index = m_current->frames.size() - 1;
        }
        m_sprite.setTextureRect(m_current->frames[m_index]);
    }
}

void Animator::draw(RenderTarget& target, RenderStates states) const
{
    target.draw(m_sprite, states);
}

// Factory helpers -------------------------------------------------

Animator createFishAnimator(const sf::Texture& tex)
{
    Animator a(tex, 126, 102);

    auto makeClip = [&](const std::string& name, int rowY, int start, int count,
        Time dur, bool loop = true, bool reverse = false)
        {
            a.addClipRow(name, rowY, start, count, dur, loop, reverse);
        };

    const int EAT_Y = 1;
    const int IDLE_Y = 107;
    const int SWIM_Y = 213;
    const int TURN_Y = 319;

    makeClip("eatLeft", EAT_Y, 0, 6, milliseconds(50), false);
    makeClip("idleLeft", IDLE_Y, 0, 6, milliseconds(120));
    makeClip("swimLeft", SWIM_Y, 0, 14, milliseconds(80));
    makeClip("turnLeftToRight", TURN_Y, 0, 5, milliseconds(90), false);
    makeClip("turnRightToLeft", TURN_Y, 0, 5, milliseconds(90), false, true);

    a.copyFlip("eatLeft", "eatRight");
    a.copyFlip("idleLeft", "idleRight");
    a.copyFlip("swimLeft", "swimRight");

    return a;
}

Animator createBarracudaAnimator(const sf::Texture& tex)
{
    Animator a(tex, 270, 122);

    auto makeClip = [&](const std::string& name, int rowY, int start, int count,
        Time dur, bool loop = true, bool reverse = false)
        {
            a.addClipRow(name, rowY, start, count, dur, loop, reverse);
        };

    const int EAT_Y = 1;
    const int SWIM_Y = 124;
    const int TURN_Y = 247;

    makeClip("eatLeft", EAT_Y, 0, 6, milliseconds(100));
    makeClip("swimLeft", SWIM_Y, 0, 14, milliseconds(80));
    makeClip("turnLeftToRight", TURN_Y, 0, 5, milliseconds(90), false);
    makeClip("turnRightToLeft", TURN_Y, 0, 5, milliseconds(90), false, true);

    a.copyFlip("eatLeft", "eatRight");
    a.copyFlip("swimLeft", "swimRight");

    return a;
}