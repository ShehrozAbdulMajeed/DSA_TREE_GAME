#include "TreeView.h"
#include "TextUtil.h"

TreeView::TreeView(const sf::Font& font) : label(font, "", 16) {
	ax = 0; ay = 0; aw = 800; ah = 500; showBalance = false; targetId = 0; hintId = 0; selectedId = 0; showSize = false;
}

void TreeView::drawLabel(sf::RenderTarget& rt, const char* s, unsigned size, float cx, float cy, sf::Color c) {
	label.setCharacterSize(size);
	label.setString(s);
	label.setFillColor(c);
	sf::FloatRect b = label.getLocalBounds();
	label.setOrigin({ b.position.x + b.size.x / 2.0f, b.position.y + b.size.y / 2.0f });
	label.setPosition({ cx, cy });
	rt.draw(label);
}

bool TreeView::mapOf(const TreeMirror& m, Map& mp) const {
	if (m.count() == 0) return false;
	float n = (float)(m.layoutWidth() > 0 ? m.layoutWidth() : 1);
	float d = (float)(m.layoutDepth() > 1 ? m.layoutDepth() - 1 : 1);
	mp.cellW = aw / n; if (mp.cellW > 70.0f) mp.cellW = 70.0f;
	mp.cellH = (ah - 50.0f) / d; if (mp.cellH > 80.0f) mp.cellH = 80.0f;
	mp.ox = ax + (aw - n * mp.cellW) / 2.0f + mp.cellW / 2.0f;
	mp.oy = ay + 30.0f;
	mp.radius = mp.cellW * 0.45f; if (mp.radius > 22.0f) mp.radius = 22.0f; if (mp.radius < 4.0f) mp.radius = 4.0f;
	return true;
}

int TreeView::hitTest(const TreeMirror& m, float mx, float my) const {
	Map mp;
	if (!mapOf(m, mp)) return 0;
	int best = 0; float bestD = 1e9f;
	for (int id = 1; id < m.capacity(); id++) {
		const VNode* v = m.node(id);
		if (!v) continue;
		float px = mp.ox + v->dx * mp.cellW, py = mp.oy + v->dy * mp.cellH;
		float ddx = mx - px, ddy = my - py, d2 = ddx * ddx + ddy * ddy, r = mp.radius + 6.0f;
		if (d2 <= r * r && d2 < bestD) { best = id; bestD = d2; }
	}
	return best;
}

void TreeView::draw(sf::RenderTarget& rt, const TreeMirror& m) {
	Map mp;
	if (!mapOf(m, mp)) {
		drawLabel(rt, "(empty tree)", 20, ax + aw / 2.0f, ay + ah / 2.0f, sf::Color(120, 130, 150));
		return;
	}
	float ox = mp.ox, oy = mp.oy, cellW = mp.cellW, cellH = mp.cellH, radius = mp.radius;

	for (int id = 1; id < m.capacity(); id++) {            // edges first
		const VNode* v = m.node(id);
		if (!v || v->parent == 0) continue;
		const VNode* p = m.node(v->parent);
		if (!p) continue;
		sf::Color c = (v->mark == VMark::SKIPPED) ? sf::Color(70, 70, 80) : sf::Color(150, 160, 185);
		float x1 = ox + p->dx * cellW, y1 = oy + p->dy * cellH;
		float x2 = ox + v->dx * cellW, y2 = oy + v->dy * cellH;
		for (int k = -1; k <= 1; k++) {
			sf::Vertex line[2];
			line[0].position = { x1 + (float)k, y1 }; line[0].color = c;
			line[1].position = { x2 + (float)k, y2 }; line[1].color = c;
			rt.draw(line, 2, sf::PrimitiveType::Lines);
		}
	}

	for (int id = 1; id < m.capacity(); id++) {
		const VNode* v = m.node(id);
		if (!v) continue;
		float px = ox + v->dx * cellW, py = oy + v->dy * cellH;

		sf::Color fill(55, 65, 95);
		switch (v->mark) {
		case VMark::CURRENT:  fill = sf::Color(60, 130, 235); break;   // blue = current visit
		case VMark::VISITED:  fill = sf::Color(45, 90, 150);  break;
		case VMark::SKIPPED:  fill = sf::Color(80, 80, 90);   break;   // grey = skipped by pruning
		case VMark::RESULT:   fill = sf::Color(215, 170, 40); break;   // gold = target / result
		case VMark::NEWNODE:  fill = sf::Color(50, 160, 100); break;
		default: break;
		}

		sf::CircleShape c(radius);
		c.setPointCount(28);
		c.setOrigin({ radius, radius });
		c.setPosition({ px, py });
		c.setFillColor(fill);
		c.setOutlineThickness(v->alert ? 4.0f : 2.0f);
		c.setOutlineColor(v->alert ? sf::Color(230, 60, 60) : sf::Color(200, 205, 220));
		rt.draw(c);

		if (radius >= 10.0f) {
			char buf[16]; buf[0] = 0;
			tAppendInt(buf, 16, v->key);
			drawLabel(rt, buf, radius >= 18.0f ? 16 : 12, px, py, sf::Color::White);
		}

		if (id == targetId || id == hintId) {                       // rings + text, never colour alone
			sf::CircleShape ring(radius + 7.0f);
			ring.setPointCount(32);
			ring.setOrigin({ radius + 7.0f, radius + 7.0f });
			ring.setPosition({ px, py });
			ring.setFillColor(sf::Color::Transparent);
			ring.setOutlineThickness(3.0f);
			ring.setOutlineColor(id == targetId ? sf::Color(255, 205, 60) : sf::Color(80, 220, 230));
			rt.draw(ring);
			drawLabel(rt, id == targetId ? "TARGET" : "HINT", 11, px, py - radius - 16.0f,
				id == targetId ? sf::Color(255, 205, 60) : sf::Color(80, 220, 230));
		}

		if (id == selectedId) {
			sf::CircleShape ring(radius + 4.0f);
			ring.setPointCount(32);
			ring.setOrigin({ radius + 4.0f, radius + 4.0f });
			ring.setPosition({ px, py });
			ring.setFillColor(sf::Color::Transparent);
			ring.setOutlineThickness(3.0f);
			ring.setOutlineColor(sf::Color::White);
			rt.draw(ring);
			drawLabel(rt, "SELECTED", 11, px, py + radius + 14.0f, sf::Color::White);
		}

		if (showSize && radius >= 10.0f && id != selectedId) {
			char sz[16]; tCopy(sz, 16, "n="); tAppendInt(sz, 16, v->size);
			drawLabel(rt, sz, 11, px, py + radius + 12.0f, sf::Color(170, 180, 205));
		}

		if (showBalance && radius >= 10.0f) {
			int bf = m.balanceOf(id);
			int a = bf < 0 ? -bf : bf;
			sf::Color bc = (a == 0) ? sf::Color(50, 170, 90) : (a == 1 ? sf::Color(220, 180, 40) : sf::Color(220, 60, 60));
			float br = radius * 0.5f;
			sf::CircleShape badge(br);
			badge.setOrigin({ br, br });
			badge.setPosition({ px + radius * 0.95f, py - radius * 0.95f });
			badge.setFillColor(bc);
			badge.setOutlineThickness(1.0f);
			badge.setOutlineColor(sf::Color::Black);
			rt.draw(badge);
			char t[8]; t[0] = 0;
			if (bf > 0) tAppend(t, 8, "+");
			tAppendInt(t, 8, bf);
			drawLabel(rt, t, 11, px + radius * 0.95f, py - radius * 0.95f, sf::Color::Black);
		}
	}
}