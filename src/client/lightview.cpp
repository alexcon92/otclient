/*
 * Copyright (c) 2010-2022 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "lightview.h"
#include "map.h"
#include "mapview.h"
#include "spritemanager.h"
#include <framework/graphics/drawpoolmanager.h>

LightView::LightView() : m_pool(g_drawPool.get<DrawPoolFramed>(DrawPoolType::LIGHT)) {}

void LightView::setSmooth(bool enabled) { m_pool->setSmooth(enabled); }

void LightView::resize(const Size& size, const uint8_t tileSize) { m_pool->resize(size * (m_tileSize = tileSize)); }

void LightView::addLightSource(const Point& pos, const Light& light)
{
    if (!isDark()) return;

    if (!m_sources.empty()) {
        auto& prevLight = m_sources.back();
        if (prevLight.pos == pos && prevLight.color == light.color) {
            prevLight.intensity = std::max<uint16_t>(prevLight.intensity, light.intensity);
            return;
        }
    }

    m_sources.emplace_back(pos, light.color, light.intensity, g_drawPool.getOpacity());
}

void LightView::draw(const Rect& dest, const Rect& src)
{
    // draw light, only if there is darkness
    m_pool->setEnable(isDark());
    if (!isDark()) return;

    g_drawPool.use(m_pool->getType(), dest, src, m_globalLightColor);

    const float size = m_tileSize * 3.3;

    bool _clr = true;
    for (auto& light : m_sources) {
        if (light.color) {
            const Color color = Color::from8bit(light.color, std::min<float>(light.opacity, light.intensity / 6.f));
            const uint16_t radius = light.intensity * m_tileSize;
            g_drawPool.addTexturedRect(Rect(light.pos - radius, Size(radius * 2)), g_sprites.getLightTexture(), color);
            g_drawPool.setBlendEquation(BlendEquation::MAX, true);
            _clr = true;
        } else {
            // Empty the lightings references
            if (_clr) { g_drawPool.flush(); _clr = false; }

            g_drawPool.setOpacity(light.opacity);
            g_drawPool.addTexturedRect(Rect(light.pos - m_tileSize * 1.8, size, size), g_sprites.getShadeTexture(), m_globalLightColor);
            g_drawPool.resetOpacity();
        }
    }

    m_sources.clear();
}
