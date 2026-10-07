#pragma once

// Estilo "vidro polido" do JNX: ícones em quadrado arredondado com brilho, e painéis translúcidos.
// Só desenho (nanovg), sem estado. Tudo em coordenadas de tela.

#include <nanovg/nanovg.h>

#include <cmath>

namespace brls
{
namespace glass
{

enum Glyph
{
    NONE = -1,
    HOME,
    PACKAGE,
    CHIP,
    GLOBE,
    SLIDERS,
    TOOLBOX,
    STAR,
    GEAR,
    INFO,
    CARD,
};

inline NVGcolor scaleAlpha(NVGcolor c, float a)
{
    c.a *= a;
    return c;
}

// Desenha o símbolo dentro de uma caixa de 100x100 unidades centrada em (cx, cy), escalada por u.
inline void glyph(NVGcontext* vg, int g, float cx, float cy, float u, NVGcolor ink, NVGcolor cut)
{
    nvgSave(vg);
    nvgTranslate(vg, cx, cy);
    nvgScale(vg, u, u);
    nvgStrokeColor(vg, ink);
    nvgFillColor(vg, ink);
    nvgStrokeWidth(vg, 6.0f);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);

    switch (g)
    {
        case HOME:
            nvgBeginPath(vg);
            nvgMoveTo(vg, -28, -2);
            nvgLineTo(vg, 0, -27);
            nvgLineTo(vg, 28, -2);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, -19, -2, 38, 30, 4);
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, -5, 10, 10, 18, 2);
            nvgFillColor(vg, cut);
            nvgFill(vg);
            break;

        case PACKAGE:
            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, -28);
            nvgLineTo(vg, 24, -14);
            nvgLineTo(vg, 24, 14);
            nvgLineTo(vg, 0, 28);
            nvgLineTo(vg, -24, 14);
            nvgLineTo(vg, -24, -14);
            nvgClosePath(vg);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgMoveTo(vg, -24, -14);
            nvgLineTo(vg, 0, 0);
            nvgLineTo(vg, 24, -14);
            nvgMoveTo(vg, 0, 0);
            nvgLineTo(vg, 0, 28);
            nvgStroke(vg);
            break;

        case CHIP:
            nvgBeginPath(vg);
            nvgRoundedRect(vg, -17, -17, 34, 34, 6);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, -7, -7, 14, 14, 3);
            nvgFill(vg);
            nvgBeginPath(vg);
            for (float p : {-8.0f, 8.0f})
            {
                nvgMoveTo(vg, p, -17); nvgLineTo(vg, p, -27);
                nvgMoveTo(vg, p, 17);  nvgLineTo(vg, p, 27);
                nvgMoveTo(vg, -17, p); nvgLineTo(vg, -27, p);
                nvgMoveTo(vg, 17, p);  nvgLineTo(vg, 27, p);
            }
            nvgStroke(vg);
            break;

        case GLOBE:
            nvgBeginPath(vg);
            nvgCircle(vg, 0, 0, 27);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgEllipse(vg, 0, 0, 12, 27);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgMoveTo(vg, -27, 0);
            nvgLineTo(vg, 27, 0);
            nvgStroke(vg);
            break;

        case SLIDERS:
        {
            const float ys[3] = {-17, 0, 17};
            const float ks[3] = {-9, 11, -3};
            nvgBeginPath(vg);
            for (int i = 0; i < 3; i++)
            {
                nvgMoveTo(vg, -27, ys[i]);
                nvgLineTo(vg, 27, ys[i]);
            }
            nvgStroke(vg);
            for (int i = 0; i < 3; i++)
            {
                nvgBeginPath(vg);
                nvgCircle(vg, ks[i], ys[i], 8);
                nvgFillColor(vg, ink);
                nvgFill(vg);
                nvgBeginPath(vg);
                nvgCircle(vg, ks[i], ys[i], 3.2f);
                nvgFillColor(vg, cut);
                nvgFill(vg);
            }
            break;
        }

        case TOOLBOX:
            nvgBeginPath(vg);
            nvgMoveTo(vg, -11, -8);
            nvgLineTo(vg, -11, -17);
            nvgLineTo(vg, 11, -17);
            nvgLineTo(vg, 11, -8);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, -28, -9, 56, 38, 7);
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgRect(vg, -28, 6, 56, 4);
            nvgFillColor(vg, cut);
            nvgFill(vg);
            break;

        case STAR:
            nvgBeginPath(vg);
            for (int i = 0; i < 10; i++)
            {
                float ang = -1.5707963f + i * 0.6283185f;
                float r = (i % 2 == 0) ? 29.0f : 12.5f;
                float px = std::cos(ang) * r, py = std::sin(ang) * r + 2;
                if (i == 0) nvgMoveTo(vg, px, py); else nvgLineTo(vg, px, py);
            }
            nvgClosePath(vg);
            nvgFill(vg);
            nvgStroke(vg);
            break;

        case GEAR:
            for (int i = 0; i < 8; i++)
            {
                nvgSave(vg);
                nvgRotate(vg, i * 0.7853982f);
                nvgBeginPath(vg);
                nvgRoundedRect(vg, -5.5f, -28, 11, 14, 2.5f);
                nvgFill(vg);
                nvgRestore(vg);
            }
            nvgBeginPath(vg);
            nvgCircle(vg, 0, 0, 21);
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgCircle(vg, 0, 0, 8.5f);
            nvgFillColor(vg, cut);
            nvgFill(vg);
            break;

        case INFO:
            nvgBeginPath(vg);
            nvgCircle(vg, 0, 0, 27);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgCircle(vg, 0, -12, 4);
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, -2);
            nvgLineTo(vg, 0, 15);
            nvgStroke(vg);
            break;

        case CARD:
            nvgBeginPath(vg);
            nvgMoveTo(vg, -17, -28);
            nvgLineTo(vg, 7, -28);
            nvgLineTo(vg, 21, -14);
            nvgLineTo(vg, 21, 28);
            nvgLineTo(vg, -17, 28);
            nvgClosePath(vg);
            nvgFill(vg);
            nvgFillColor(vg, cut);
            for (float px : {-9.0f, -1.5f, 6.0f})
            {
                nvgBeginPath(vg);
                nvgRoundedRect(vg, px, -22, 4, 12, 1.5f);
                nvgFill(vg);
            }
            break;

        default:
            break;
    }
    nvgRestore(vg);
}

// ---------------------------------------------------------------------------------
// Liquid Glass: material translúcido com luz nas bordas, no estilo do sistema mais recente da Apple.
// ---------------------------------------------------------------------------------

// Luz de borda ("specular"): forte no canto superior esquerdo, suave no inferior direito.
inline void rim(NVGcontext* vg, float x, float y, float w, float h, float r, float alpha, float width = 1.4f)
{
    NVGpaint tl = nvgLinearGradient(vg, x, y, x + w * 0.55f, y + h * 0.55f, nvgRGBA(255, 255, 255, (int)(200 * alpha)), nvgRGBA(255, 255, 255, 0));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 0.7f, y + 0.7f, w - 1.4f, h - 1.4f, r);
    nvgStrokeWidth(vg, width);
    nvgStrokePaint(vg, tl);
    nvgStroke(vg);

    NVGpaint br = nvgLinearGradient(vg, x + w, y + h, x + w * 0.45f, y + h * 0.45f, nvgRGBA(255, 255, 255, (int)(100 * alpha)), nvgRGBA(255, 255, 255, 0));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 0.7f, y + 0.7f, w - 1.4f, h - 1.4f, r);
    nvgStrokeWidth(vg, width);
    nvgStrokePaint(vg, br);
    nvgStroke(vg);
}

// Brilho suave que sobe pelas bordas por dentro (dá a sensação de vidro espesso).
inline void innerGlow(NVGcontext* vg, float x, float y, float w, float h, float r, float alpha, float strength)
{
    NVGpaint g = nvgBoxGradient(vg, x + 3, y + 3, w - 6, h - 6, r * 0.8f, std::min(w, h) * 0.38f, nvgRGBA(255, 255, 255, 0), nvgRGBA(255, 255, 255, (int)(strength * alpha)));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, r);
    nvgFillPaint(vg, g);
    nvgFill(vg);
}

// Ícone de vidro: cor viva em degradê, luz vinda do canto superior esquerdo, borda iluminada.
inline void tile(NVGcontext* vg, float x, float y, float s, int g, NVGcolor top, NVGcolor bottom, float alpha = 1.0f)
{
    const float r = s * 0.30f;

    // sombra suave
    NVGpaint shadow = nvgBoxGradient(vg, x, y + s * 0.10f, s, s, r, s * 0.38f, nvgRGBA(0, 0, 0, (int)(105 * alpha)), nvgRGBA(0, 0, 0, 0));
    nvgBeginPath(vg);
    nvgRect(vg, x - s * 0.6f, y - s * 0.4f, s * 2.2f, s * 2.2f);
    nvgFillPaint(vg, shadow);
    nvgFill(vg);

    // corpo
    NVGpaint body = nvgLinearGradient(vg, x, y, x + s * 0.65f, y + s, scaleAlpha(top, alpha), scaleAlpha(bottom, alpha));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, s, s, r);
    nvgFillPaint(vg, body);
    nvgFill(vg);

    // luz difusa do canto superior esquerdo
    NVGpaint lens = nvgRadialGradient(vg, x + s * 0.25f, y + s * 0.12f, 0, s * 0.95f, nvgRGBA(255, 255, 255, (int)(80 * alpha)), nvgRGBA(255, 255, 255, 0));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, s, s, r);
    nvgFillPaint(vg, lens);
    nvgFill(vg);

    innerGlow(vg, x, y, s, s, r, alpha, 70.0f);
    rim(vg, x, y, s, s, r, alpha, 1.3f);

    if (g >= 0)
    {
        const float u = s / 100.0f * 1.02f;
        glyph(vg, g, x + s * 0.5f, y + s * 0.5f + s * 0.025f, u, nvgRGBA(0, 0, 0, (int)(46 * alpha)), nvgRGBA(0, 0, 0, 0));
        glyph(vg, g, x + s * 0.5f, y + s * 0.5f, u, nvgRGBA(255, 255, 255, (int)(246 * alpha)), scaleAlpha(bottom, alpha));
    }
}

// Painel de Liquid Glass: quase transparente, luz nas bordas e brilho difuso no canto.
inline void panel(NVGcontext* vg, float x, float y, float w, float h, float r, float alpha = 1.0f)
{
    NVGpaint shadow = nvgBoxGradient(vg, x, y + 8, w, h, r, 30, nvgRGBA(0, 0, 0, (int)(78 * alpha)), nvgRGBA(0, 0, 0, 0));
    nvgBeginPath(vg);
    nvgRect(vg, x - 40, y - 30, w + 80, h + 90);
    nvgFillPaint(vg, shadow);
    nvgFill(vg);

    NVGpaint fill = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(255, 255, 255, (int)(34 * alpha)), nvgRGBA(255, 255, 255, (int)(14 * alpha)));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, r);
    nvgFillPaint(vg, fill);
    nvgFill(vg);

    NVGpaint lens = nvgRadialGradient(vg, x + w * 0.14f, y, 0, std::max(w, h) * 0.6f, nvgRGBA(222, 205, 255, (int)(52 * alpha)), nvgRGBA(222, 205, 255, 0));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, r);
    nvgFillPaint(vg, lens);
    nvgFill(vg);

    innerGlow(vg, x, y, w, h, r, alpha, 38.0f);
    rim(vg, x, y, w, h, r, alpha, 1.4f);
}

// Cápsula de vidro (raio = metade da altura).
inline void capsule(NVGcontext* vg, float x, float y, float w, float h, float alpha = 1.0f)
{
    panel(vg, x, y, w, h, h * 0.5f, alpha);
}

} // namespace glass
} // namespace brls
