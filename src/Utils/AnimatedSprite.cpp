#include "Utils/AnimatedSprite.h"

AnimatedSprite::AnimatedSprite(const sf::Texture& texture)
    : m_texture(texture)
{
    m_sprite.setTexture(texture);
}

void AnimatedSprite::addAnimation(const std::string& name, const Animation& anim)
{
    m_anims[name] = anim;
}

void AnimatedSprite::play(const std::string& name)
{
    auto it = m_anims.find(name);
    if (it == m_anims.end())
        return;

    m_current = &it->second;
    m_index = 0;
    m_elapsed = sf::Time::Zero;
    m_finished = false;
    m_sprite.setTextureRect(m_current->frames[0]);
    auto bounds = m_sprite.getLocalBounds();
    m_sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}

void AnimatedSprite::update(sf::Time dt)
{
    if (!m_current || m_finished)
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
            {
                m_index = m_current->frames.size() - 1;
                m_finished = true;
            }
        }
        m_sprite.setTextureRect(m_current->frames[m_index]);
    }
}

void AnimatedSprite::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

void AnimatedSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.draw(m_sprite, states);
}

