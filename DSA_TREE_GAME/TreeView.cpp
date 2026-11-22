#include "TreeView.h"
#include "StringUtil.h"
#include <cstring>


TreeView::TreeView(const sf::Font& font) : label(font, "", 16) {
    ax = 0; ay = 0; aw = 800; ah = 500;
    showBalance = false; showSize = false; dragging = false;
    targetId = hintId = selectedId = hoverId = 0;
    zoom = 1.0f; panX = panY = lastMouseX = lastMouseY = hoverX = hoverY = 0.0f;
}

void TreeView::drawLabel(sf::RenderTarget& rt, const char* s, unsigned size, float cx, float cy, sf::Color c) {
    label.setCharacterSize(size); label.setString(s); label.setFillColor(c);
    sf::FloatRect b = label.getLocalBounds();
    label.setOrigin({ b.position.x + b.size.x / 2.0f,b.position.y + b.size.y / 2.0f });
    label.setPosition({ cx,cy }); rt.draw(label);
}

void TreeView::handleEvent(const sf::Event& e) {
    if (const sf::Event::MouseMoved* mm = e.getIf<sf::Event::MouseMoved>()) {
        float x = (float)mm->position.x * (2.0f / 3.0f);
        float y = (float)mm->position.y * (2.0f / 3.0f);
        hoverX = x; hoverY = y;
        if (dragging) { panX += x - lastMouseX; panY += y - lastMouseY; }
        lastMouseX = x; lastMouseY = y;
    }
    if (const sf::Event::MouseButtonPressed* bp = e.getIf<sf::Event::MouseButtonPressed>()) {
        if (bp->button == sf::Mouse::Button::Middle || bp->button == sf::Mouse::Button::Right) {
            dragging = true;
            lastMouseX = (float)bp->position.x * (2.0f / 3.0f);
            lastMouseY = (float)bp->position.y * (2.0f / 3.0f);
        }
    }
    if (const sf::Event::MouseButtonReleased* br = e.getIf<sf::Event::MouseButtonReleased>()) {
        if (br->button == sf::Mouse::Button::Middle || br->button == sf::Mouse::Button::Right) dragging = false;
    }
    if (const sf::Event::MouseWheelScrolled* wheel = e.getIf<sf::Event::MouseWheelScrolled>()) {
        float x = (float)wheel->position.x * (2.0f / 3.0f);
        float y = (float)wheel->position.y * (2.0f / 3.0f);
        if (x >= ax && x <= ax + aw && y >= ay && y <= ay + ah) {
            float old = zoom;
            zoom *= (wheel->delta > 0 ? 1.12f : 1.0f / 1.12f);
            if (zoom < 0.45f) zoom = 0.45f;
            if (zoom > 2.8f) zoom = 2.8f;
            float ratio = zoom / old;
            panX = (panX - (x - (ax + aw / 2.0f))) * ratio + (x - (ax + aw / 2.0f));
            panY = (panY - (y - (ay + ah / 2.0f))) * ratio + (y - (ay + ah / 2.0f));
        }
    }
}

bool TreeView::mapOf(const TreeMirror& m, Map& mp) const {
    if (m.count() == 0) return false;
    float n = (float)(m.layoutWidth() > 0 ? m.layoutWidth() : 1);
    float d = (float)(m.layoutDepth() > 1 ? m.layoutDepth() - 1 : 1);
    mp.cellW = aw / n; if (mp.cellW > 70.0f) mp.cellW = 70.0f;
    mp.cellH = (ah - 50.0f) / d; if (mp.cellH > 80.0f) mp.cellH = 80.0f;
    mp.cellW *= zoom; mp.cellH *= zoom;
    mp.ox = ax + (aw - n * mp.cellW) / 2.0f + mp.cellW / 2.0f + panX;
    mp.oy = ay + 30.0f + panY;
    mp.radius = mp.cellW * 0.45f;
    if (mp.radius > 22.0f * zoom) mp.radius = 22.0f * zoom;
    if (mp.radius < 4.0f) mp.radius = 4.0f;
    return true;
}

int TreeView::hitTest(const TreeMirror& m, float mx, float my) const {
    Map mp; if (!mapOf(m, mp)) return 0;
    int best = 0; float bestD = 1e9f;
    for (int id = 1;id < m.capacity();id++) {
        const VNode* v = m.node(id); if (!v) continue;
        float px = mp.ox + v->dx * mp.cellW, py = mp.oy + v->dy * mp.cellH;
        float dx = mx - px, dy = my - py, d2 = dx * dx + dy * dy, r = mp.radius + 6.0f;
        if (d2 <= r * r && d2 < bestD) { best = id;bestD = d2; }
    }
    return best;
}

void TreeView::draw(sf::RenderTarget& rt, const TreeMirror& m) {
    Map mp;
    if (!mapOf(m, mp)) { drawLabel(rt, "(empty tree)", 20, ax + aw / 2.0f, ay + ah / 2.0f, sf::Color(120, 130, 150));return; }
    float ox = mp.ox, oy = mp.oy, cw = mp.cellW, ch = mp.cellH, radius = mp.radius;
    hoverId = hitTest(m, hoverX, hoverY);

    for (int id = 1;id < m.capacity();id++) {
        const VNode* v = m.node(id); if (!v || v->parent == 0) continue;
        const VNode* p = m.node(v->parent); if (!p) continue;
        sf::Color col = (v->mark == VMark::SKIPPED) ? sf::Color(70, 70, 80) : sf::Color(150, 160, 185);
        sf::Vertex line[2];
        line[0].position = { ox + p->dx * cw,oy + p->dy * ch }; line[0].color = col;
        line[1].position = { ox + v->dx * cw,oy + v->dy * ch }; line[1].color = col;
        rt.draw(line, 2, sf::PrimitiveType::Lines);
    }

    for (int id = 1;id < m.capacity();id++) {
        const VNode* v = m.node(id); if (!v) continue;
        float px = ox + v->dx * cw, py = oy + v->dy * ch;
        sf::Color fill(55, 65, 95);
        switch (v->mark) {
        case VMark::CURRENT: fill = sf::Color(60, 130, 235); break;
        case VMark::VISITED: fill = sf::Color(45, 90, 150); break;
        case VMark::SKIPPED: fill = sf::Color(80, 80, 90); break;
        case VMark::RESULT: fill = sf::Color(215, 170, 40); break;
        case VMark::NEWNODE: fill = sf::Color(50, 160, 100); break;
        default: break;
        }
        sf::CircleShape node(radius); node.setPointCount(28); node.setOrigin({ radius,radius });
        node.setPosition({ px,py }); node.setFillColor(fill);
        node.setOutlineThickness(v->alert ? 4.0f : 2.0f);
        node.setOutlineColor(v->alert ? sf::Color(230, 60, 60) : sf::Color(200, 205, 220)); rt.draw(node);

        if (radius >= 10.0f) { char key[16] = { 0 };appendInt(key, 16, v->key);drawLabel(rt, key, radius >= 18 ? 16 : 12, px, py, sf::Color::White); }
        // State is encoded by a letter as well as colour.
        const char* state = "";
        switch (v->mark) {
        case VMark::CURRENT: state = "C"; break;
        case VMark::VISITED: state = "V"; break;
        case VMark::SKIPPED: state = "X"; break;
        case VMark::RESULT: state = "R"; break;
        case VMark::NEWNODE: state = "N"; break;
        default: break;
        }
        if (state[0]) drawLabel(rt, state, 10, px + radius * 0.85f, py + radius * 0.78f, sf::Color::White);

        if (id == targetId || id == hintId) {
            sf::CircleShape ring(radius + 7.0f);ring.setPointCount(32);ring.setOrigin({ radius + 7.0f,radius + 7.0f });
            ring.setPosition({ px,py });ring.setFillColor(sf::Color::Transparent);ring.setOutlineThickness(3.0f);
            ring.setOutlineColor(id == targetId ? sf::Color(255, 205, 60) : sf::Color(80, 220, 230));rt.draw(ring);
            drawLabel(rt, id == targetId ? "TARGET" : "HINT", 11, px, py - radius - 16.0f, id == targetId ? sf::Color(255, 205, 60) : sf::Color(80, 220, 230));
        }
        if (id == selectedId) {
            sf::CircleShape ring(radius + 4.0f);ring.setPointCount(32);ring.setOrigin({ radius + 4.0f,radius + 4.0f });
            ring.setPosition({ px,py });ring.setFillColor(sf::Color::Transparent);ring.setOutlineThickness(3.0f);
            ring.setOutlineColor(sf::Color::White);rt.draw(ring);
        }
        if (showSize && radius >= 10.0f) { char sz[16] = "n=";appendInt(sz, 16, v->size);drawLabel(rt, sz, 11, px, py + radius + 12.0f, sf::Color(190, 200, 220)); }
        if (showBalance && radius >= 10.0f) {
            int bf = m.balanceOf(id); float br = radius * 0.5f;
            sf::CircleShape badge(br);badge.setOrigin({ br,br });badge.setPosition({ px + radius * .95f,py - radius * .95f });
            badge.setFillColor(sf::Color(40, 45, 60));badge.setOutlineThickness(1);badge.setOutlineColor(sf::Color::White);rt.draw(badge);
            char t[8] = { 0 };if (bf > 0)appendText(t, 8, "+");appendInt(t, 8, bf);
            drawLabel(rt, t, 10, px + radius * .95f, py - radius * .95f, sf::Color::White);
        }
    }

    if (hoverId) {
        const VNode* v = m.node(hoverId);
        if (v) {
            char a[128] = "Key: "; appendInt(a, 128, v->key);
            appendText(a, 128, " | Height: ");appendInt(a, 128, v->height);
            appendText(a, 128, " | Balance: ");appendInt(a, 128, m.balanceOf(hoverId));
            appendText(a, 128, " | Size: ");appendInt(a, 128, v->size);
            sf::RectangleShape bg({ 300.0f,30.0f });bg.setPosition({ ax + 8.0f,ay + ah - 38.0f });
            bg.setFillColor(sf::Color(15, 18, 28, 235));bg.setOutlineThickness(1);bg.setOutlineColor(sf::Color(180, 190, 210));rt.draw(bg);
            label.setCharacterSize(13);label.setString(a);label.setFillColor(sf::Color::White);
            label.setPosition({ ax + 16.0f,ay + ah - 34.0f });rt.draw(label);
        }
    }
    drawLabel(rt, "Wheel: zoom   Right/middle drag: pan   Home: reset", 12, ax + aw / 2.0f, ay + ah - 10.0f, sf::Color(145, 155, 180));
}
