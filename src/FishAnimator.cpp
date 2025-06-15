#include "FishAnimator.h"

using namespace sf;

namespace              // FishAnimator.cpp
{
    // sprite size inside the grid
    constexpr int CELL_W = 122;   // width  of one frame
    constexpr int CELL_H = 102;   // height of one frame
    // thickness of the separating grid lines
    constexpr int GRID = 2;
}

FishAnimator::FishAnimator(const Texture& texture) : m_texture(texture)
{
    m_sprite.setTexture(m_texture);
    buildAnimations();
}

void FishAnimator::buildAnimations()
{
    auto makeClip = [&](int row, int start, int count, Time dur,
        bool loop = true, bool reverse = false) -> Clip
        {
            Clip c;
            c.frameTime = dur;
            c.loop = loop;
            for (int i = 0; i < count; ++i)
            {
                int col = reverse ? start + count - 1 - i : start + i;
                IntRect rect(GRID + col * (CELL_W + GRID),
                    GRID + row * (CELL_H + GRID),
                    CELL_W, CELL_H);
                c.frames.push_back(rect);
            }
            return c;
        };

    m_clips["eatLeft"] = makeClip(0, 0, 6, milliseconds(100));
    m_clips["idleLeft"] = makeClip(1, 0, 6, milliseconds(120));
    m_clips["swimLeft"] = makeClip(2, 0, 15, milliseconds(80));
    m_clips["turnLeftToRight"] = makeClip(3, 0, 5, milliseconds(90), /*loop*/ false);
    m_clips["turnRightToLeft"] = makeClip(3, 0, 5, milliseconds(90), /*loop*/ false, /*reverse*/ true);


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
    m_index = 0;
    m_elapsed = Time::Zero;

    m_sprite.setTextureRect(m_current->frames[0]);
    if (m_current->flipped)
    {
        m_sprite.setOrigin(static_cast<float>(CELL_W), 0.f);
        m_sprite.setScale(-1.f, 1.f);
    }
    else
    {
        m_sprite.setOrigin(0.f, 0.f);
        m_sprite.setScale(1.f, 1.f);
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