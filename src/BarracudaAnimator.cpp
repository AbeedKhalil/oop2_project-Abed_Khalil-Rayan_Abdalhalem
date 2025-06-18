#include "BarracudaAnimator.h"

using namespace sf;

namespace
{
    constexpr int START_X = 1;
    constexpr int FRAME_W = 270;
    constexpr int FRAME_H = 122;
    constexpr int EAT_Y = 1;
    constexpr int SWIM_Y = 124;
    constexpr int TURN_Y = 247;
}

BarracudaAnimator::BarracudaAnimator(const Texture& texture) : m_texture(texture)
{
    m_sprite.setTexture(m_texture);
    buildAnimations();
}

void BarracudaAnimator::setScale(const Vector2f& scale)
{
    m_scale = scale;
    applyScale();
}

void BarracudaAnimator::applyScale()
{
    if (m_current && m_current->flipped)
        m_sprite.setScale(-m_scale.x, m_scale.y);
    else
        m_sprite.setScale(m_scale);
}

void BarracudaAnimator::buildAnimations()
{
    auto makeClip = [&](int rowY, int startFrame, int count, Time dur,
        bool loop = true, bool reverse = false) -> Clip
        {
            Clip c;
            c.frameTime = dur;
            c.loop = loop;
            for (int i = 0; i < count; ++i)
            {
                int idx = reverse ? startFrame + count - 1 - i : startFrame + i;
                IntRect rect(START_X + idx * FRAME_W, rowY, FRAME_W, FRAME_H);
                c.frames.push_back(rect);
            }
            return c;
        };

    m_clips["eatLeft"] = makeClip(EAT_Y, 0, 6, milliseconds(100));
    m_clips["swimLeft"] = makeClip(SWIM_Y, 0, 14, milliseconds(80));
    m_clips["turnLeftToRight"] = makeClip(TURN_Y, 0, 5, milliseconds(90), false);
    m_clips["turnRightToLeft"] = makeClip(TURN_Y, 0, 5, milliseconds(90), false, true);

    auto copyFlip = [&](const std::string& left, const std::string& right)
        {
            Clip c = m_clips[left];
            c.flipped = true;
            m_clips[right] = std::move(c);
        };

    copyFlip("eatLeft", "eatRight");
    copyFlip("swimLeft", "swimRight");
}

void BarracudaAnimator::play(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it == m_clips.end())
        return;

    m_current = &it->second;
    m_currentName = name;
    m_index = 0;
    m_elapsed = Time::Zero;

    m_sprite.setTextureRect(m_current->frames[0]);
    m_sprite.setOrigin(static_cast<float>(FRAME_W) / 2.f,
        static_cast<float>(FRAME_H) / 2.f);
    applyScale();
}

void BarracudaAnimator::update(Time dt)
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

void BarracudaAnimator::draw(RenderTarget& target, RenderStates states) const
{
    target.draw(m_sprite, states);
}