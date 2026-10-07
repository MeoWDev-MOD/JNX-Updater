#include "jnx_loading.hpp"

#include <borealis/glass.hpp>
#include <cmath>

#include "jnx.hpp"
#include "jnx_theme.hpp"
#include "main_frame.hpp"

namespace jnx {

    void drawWallpaper(NVGcontext* vg, int x, int y, unsigned width, unsigned height, float alpha)
    {
        static int wallpaper = 0;
        if (!wallpaper) {
            wallpaper = nvgCreateImage(vg, "romfs:/jnx_wallpaper.jpg", 0);
            if (!wallpaper)
                wallpaper = -1;
        }
        if (wallpaper > 0) {
            NVGpaint paint = nvgImagePattern(vg, x, y, width, height, 0, wallpaper, alpha);
            nvgBeginPath(vg);
            nvgRect(vg, x, y, width, height);
            nvgFillPaint(vg, paint);
            nvgFill(vg);
        }
    }

}  // namespace jnx

LoadingPage::LoadingPage() {}

LoadingPage::~LoadingPage()
{
    if (this->worker) {
        if (this->worker->joinable())
            this->worker->join();
        delete this->worker;
    }
    if (this->logo > 0 && this->vgForLogo)
        nvgDeleteImage(this->vgForLogo, this->logo);
}

void LoadingPage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    using clock = std::chrono::steady_clock;
    const float t = std::chrono::duration<float>(clock::now() - this->begin).count();

    // As buscas de rede rodam fora da tela, para o app não congelar.
    if (!this->started) {
        this->started = true;
        this->worker = new std::thread([this]() {
            jnx::prefetch([this](int step, int total, const char* text) {
                this->total = total;
                this->step = step;
                std::lock_guard<std::mutex> lock(this->labelMutex);
                this->label = text;
            });
            this->done = true;
        });
    }

    if (!this->logo) {
        this->vgForLogo = vg;
        this->logo = nvgCreateImage(vg, "romfs:/jnx_logo_glass.png", 0);
        if (!this->logo)
            this->logo = -1;
    }

    jnx::drawWallpaper(vg, x, y, width, height, 1.0f);

    const float cx = x + width / 2.0f;
    const float oy = y + 205.0f;  // centro do disco de vidro
    const float breathe = 1.0f + 0.018f * std::sin(t * 2.0f);
    const float R = 120.0f * breathe;

    // luz ambiente atrás do disco
    NVGpaint ambient = nvgRadialGradient(vg, cx, oy, R * 0.6f, R * 2.6f, nvgRGBA(139, 92, 246, 120), nvgRGBA(139, 92, 246, 0));
    nvgBeginPath(vg);
    nvgCircle(vg, cx, oy, R * 2.6f);
    nvgFillPaint(vg, ambient);
    nvgFill(vg);

    // disco de Liquid Glass com o gato dentro
    brls::glass::panel(vg, cx - R, oy - R, R * 2, R * 2, R, 1.0f);
    if (this->logo > 0) {
        const float L = R * 1.42f;
        NVGpaint halo = nvgRadialGradient(vg, cx, oy, 0, L * 0.7f, nvgRGBA(196, 181, 253, 70), nvgRGBA(196, 181, 253, 0));
        nvgBeginPath(vg);
        nvgCircle(vg, cx, oy, L * 0.7f);
        nvgFillPaint(vg, halo);
        nvgFill(vg);
        NVGpaint img = nvgImagePattern(vg, cx - L / 2, oy - L / 2, L, L, 0, this->logo, 1.0f);
        nvgBeginPath(vg);
        nvgRect(vg, cx - L / 2, oy - L / 2, L, L);
        nvgFillPaint(vg, img);
        nvgFill(vg);
    }

    // Título com brilho violeta
    nvgFontFaceId(vg, ctx->fontStash->regular);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFontSize(vg, 76);
    nvgFontBlur(vg, 16);
    nvgFillColor(vg, nvgRGBA(139, 92, 246, 210));
    nvgBeginPath(vg);
    nvgText(vg, cx, y + 392, "JNX MOD", nullptr);
    nvgFontBlur(vg, 0);
    nvgFillColor(vg, nvgRGB(255, 255, 255));
    nvgBeginPath(vg);
    nvgText(vg, cx, y + 392, "JNX MOD", nullptr);

    nvgFontSize(vg, 24);
    nvgFillColor(vg, nvgRGB(jnx::LILAC_R, jnx::LILAC_G, jnx::LILAC_B));
    nvgBeginPath(vg);
    nvgText(vg, cx, y + 448, "Design by MeoW", nullptr);

    // Cápsula de vidro com a etapa e a barra de progresso
    const float cw = 560, ch = 96, bx0 = cx - cw / 2, by0 = y + 506;
    brls::glass::panel(vg, bx0, by0, cw, ch, ch / 2, 1.0f);

    std::string text;
    {
        std::lock_guard<std::mutex> lock(this->labelMutex);
        text = this->label;
    }
    nvgFontSize(vg, 21);
    nvgFillColor(vg, nvgRGB(244, 241, 255));
    nvgBeginPath(vg);
    nvgText(vg, cx, by0 + 32, text.c_str(), nullptr);

    const float target = this->done ? 1.0f : (float)this->step / (float)this->total;
    this->shown += (target - this->shown) * 0.08f;
    const float bw = cw - 80, bh = 12, bx = cx - bw / 2, by = by0 + 58;

    nvgBeginPath(vg);
    nvgRoundedRect(vg, bx, by, bw, bh, bh / 2);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 36));
    nvgFill(vg);

    const float fw = std::max(bh, bw * (0.05f + 0.95f * this->shown));

    // brilho violeta embaixo do preenchimento
    NVGpaint glow = nvgBoxGradient(vg, bx, by + 2, fw, bh, bh / 2, 14, nvgRGBA(139, 92, 246, 170), nvgRGBA(139, 92, 246, 0));
    nvgBeginPath(vg);
    nvgRect(vg, bx - 20, by - 18, fw + 40, bh + 44);
    nvgFillPaint(vg, glow);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, bx, by, fw, bh, bh / 2);
    nvgFillPaint(vg, nvgLinearGradient(vg, bx, 0, bx + bw, 0, nvgRGB(jnx::LILAC_R, jnx::LILAC_G, jnx::LILAC_B), nvgRGB(jnx::VIOLET_R, jnx::VIOLET_G, jnx::VIOLET_B)));
    nvgFill(vg);

    // reflexo que passa pela barra
    const float sweep = std::fmod(t * 0.9f, 1.6f) - 0.3f;
    nvgSave(vg);
    nvgIntersectScissor(vg, bx, by, fw, bh);
    nvgBeginPath(vg);
    nvgRect(vg, bx + sweep * bw - 40, by, 80, bh);
    nvgFillPaint(vg, nvgLinearGradient(vg, bx + sweep * bw - 40, 0, bx + sweep * bw + 40, 0, nvgRGBA(255, 255, 255, 0), nvgRGBA(255, 255, 255, 120)));
    nvgFill(vg);
    nvgRestore(vg);

    // Abre o app quando terminou (e a tela ficou ao menos um instante visível)
    if (this->done && !this->opened && t > 0.9f && this->shown > 0.97f) {
        this->opened = true;
        brls::Application::pushView(new MainFrame());
    }
}
