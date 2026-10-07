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

// Ícone de vidro polido: quadrado arredondado em degradê, brilho no topo, borda de luz e sombra suave.
inline void tile(NVGcontext* vg, float x, float y, float s, int g, NVGcolor top, NVGcolor bottom, float alpha = 1.0f)
{
    const float r = s * 0.27f;

    // sombra suave
    NVGpaint shadow = nvgBoxGradient(vg, x, y + s * 0.07f, s, s, r, s * 0.30f, nvgRGBA(0, 0, 0, (int)(120 * alpha)), nvgRGBA(0, 0, 0, 0));
    nvgBeginPath(vg);
    nvgRect(vg, x - s * 0.5f, y - s * 0.4f, s * 2.0f, s * 2.0f);
    nvgFillPaint(vg, shadow);
    nvgFill(vg);

    // corpo
    NVGpaint body = nvgLinearGradient(vg, x, y, x, y + s, scaleAlpha(top, alpha), scaleAlpha(bottom, alpha));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, s, s, r);
    nvgFillPaint(vg, body);
    nvgFill(vg);

    // luz subindo da base
    NVGpaint glow = nvgRadialGradient(vg, x + s * 0.5f, y + s * 1.05f, s * 0.05f, s * 0.75f, nvgRGBA(255, 255, 255, (int)(80 * alpha)), nvgRGBA(255, 255, 255, 0));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, s, s, r);
    nvgFillPaint(vg, glow);
    nvgFill(vg);

    // brilho do vidro (metade de cima)
    NVGpaint gloss = nvgLinearGradient(vg, x, y, x, y + s * 0.55f, nvgRGBA(255, 255, 255, (int)(115 * alpha)), nvgRGBA(255, 255, 255, (int)(10 * alpha)));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + s * 0.045f, y + s * 0.045f, s * 0.91f, s * 0.50f, r * 0.8f);
    nvgFillPaint(vg, gloss);
    nvgFill(vg);

    // borda de luz
    NVGpaint rim = nvgLinearGradient(vg, x, y, x, y + s, nvgRGBA(255, 255, 255, (int)(160 * alpha)), nvgRGBA(255, 255, 255, (int)(28 * alpha)));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 0.6f, y + 0.6f, s - 1.2f, s - 1.2f, r);
    nvgStrokeWidth(vg, 1.3f);
    nvgStrokePaint(vg, rim);
    nvgStroke(vg);

    if (g >= 0)
    {
        const float u = s / 100.0f * 1.02f;
        glyph(vg, g, x + s * 0.5f, y + s * 0.5f + s * 0.035f, u, nvgRGBA(0, 0, 0, (int)(70 * alpha)), nvgRGBA(0, 0, 0, 0));
        glyph(vg, g, x + s * 0.5f, y + s * 0.5f, u, nvgRGBA(255, 255, 255, (int)(245 * alpha)), scaleAlpha(bottom, alpha));
    }
}

// Painel de vidro fosco: preenchimento translúcido, brilho no topo e borda de luz.
inline void panel(NVGcontext* vg, float x, float y, float w, float h, float r, float alpha = 1.0f)
{
    NVGpaint shadow = nvgBoxGradient(vg, x, y + 4, w, h, r, 16, nvgRGBA(0, 0, 0, (int)(70 * alpha)), nvgRGBA(0, 0, 0, 0));
    nvgBeginPath(vg);
    nvgRect(vg, x - 24, y - 20, w + 48, h + 56);
    nvgFillPaint(vg, shadow);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, r);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, (int)(20 * alpha)));
    nvgFill(vg);

    NVGpaint sheen = nvgLinearGradient(vg, x, y, x, y + h * 0.6f, nvgRGBA(255, 255, 255, (int)(34 * alpha)), nvgRGBA(255, 255, 255, 0));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, r);
    nvgFillPaint(vg, sheen);
    nvgFill(vg);

    NVGpaint rim = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(255, 255, 255, (int)(78 * alpha)), nvgRGBA(255, 255, 255, (int)(18 * alpha)));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, r);
    nvgStrokeWidth(vg, 1.2f);
    nvgStrokePaint(vg, rim);
    nvgStroke(vg);
}

} // namespace glass
} // namespace brls
