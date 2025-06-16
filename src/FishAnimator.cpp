#include "FishAnimator.h"

using namespace sf;

namespace
{
    // Sprite sheet layout constants
    // Frames are laid out on a single horizontal line starting at (1,1)
    constexpr int START_X = 1;
    constexpr int START_Y = 1;

    // Size of each frame in the sheet
    constexpr int FRAME_W = 126;
    // The previous sheet used a cell height of 102px.  The updated sprites have
    // the same vertical size so we keep that value here.
    constexpr int FRAME_H = 102;
}

FishAnimator::FishAnimator(const Texture& texture) : m_texture(texture)
{
    m_sprite.setTexture(m_texture);
    buildAnimations();
}

void FishAnimator::setScale(const Vector2f& scale)
{
    m_scale = scale;
    applyScale();
}

void FishAnimator::applyScale()
{
    if (m_current && m_current->flipped)
        m_sprite.setScale(-m_scale.x, m_scale.y);
    else
        m_sprite.setScale(m_scale);
}

void FishAnimator::buildAnimations()
{
    auto makeClip = [&](int startFrame, int count, Time dur,
        bool loop = true, bool reverse = false) -> Clip
        {
            Clip c;
            c.frameTime = dur;
            c.loop = loop;
            for (int i = 0; i < count; ++i)
            {
                int index = reverse ? startFrame + count - 1 - i : startFrame + i;
                IntRect rect(START_X + index * FRAME_W,
                    START_Y,
                    FRAME_W, FRAME_H);
                c.frames.push_back(rect);
            }
            return c;
        };

    // Frame ranges follow the updated sprite sheet layout
    m_clips["eatLeft"] = makeClip(0, 6, milliseconds(100));
    m_clips["idleLeft"] = makeClip(6, 1, milliseconds(120));
    m_clips["swimLeft"] = makeClip(7, 15, milliseconds(80));
    m_clips["turnLeftToRight"] = makeClip(22, 5, milliseconds(90), /*loop*/ false);
    m_clips["turnRightToLeft"] = makeClip(22, 5, milliseconds(90), /*loop*/ false, /*reverse*/ true);


    // Create right-facing versions
    auto copyFlip = [&](const std::string& left, const std::string& right)
        {
            Clip c = m_clips[left];
            c.flipped = true;
            m_clips[right] = std::move(c);
        };

    copyFlip("eatLeft", "eatRight");
    copyFlip("idleLeft", "idleRight");
    copyFlip("swimLeft", "swimRight");
}

void FishAnimator::play(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it == m_clips.end())
        return;

    m_current = &it->second;
    m_currentName = name;
    m_index = 0;
    m_elapsed = Time::Zero;

    m_sprite.setTextureRect(m_current->frames[0]);
    if (m_current->flipped)
    {
        m_sprite.setOrigin(static_cast<float>(FRAME_W), 0.f);
        applyScale();
    }
    else
    {
        m_sprite.setOrigin(0.f, 0.f);
        applyScale();
    }
}

void FishAnimator::update(Time dt)
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

void FishAnimator::draw(RenderTarget& target, RenderStates states) const
{
    target.draw(m_sprite, states);
}