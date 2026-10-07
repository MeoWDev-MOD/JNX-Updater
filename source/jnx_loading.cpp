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
        this->logo = nvgCreateImage(vg, "romfs:/jnx_logo_white.png", 0);
        if (!this->logo)
            this->logo = -1;
    }

    jnx::drawWallpaper(vg, x, y, width, height, 1.0f);

    const float cx = x + width / 2.0f;
    const float cy = y + height / 2.0f - 40.0f;

    // Logo de vidro "respirando"
    const float breathe = 1.0f + 0.025f * std::sin(t * 2.2f);
    const float s = 132.0f * breathe;
    const float tx = cx - s / 2, ty = cy - s / 2;

    NVGpaint halo = nvgRadialGradient(vg, cx, cy, s * 0.4f, s * 1.5f, nvgRGBA(139, 92, 246, 90), nvgRGBA(139, 92, 246, 0));
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, s * 1.5f);
    nvgFillPaint(vg, halo);
    nvgFill(vg);

    brls::glass::tile(vg, tx, ty, s, brls::glass::NONE, nvgRGB(167, 139, 250), nvgRGB(109, 40, 217), 1.0f);
    if (this->logo > 0) {
        const float pad = s * 0.20f;
        NVGpaint img = nvgImagePattern(vg, tx + pad, ty + pad, s - pad * 2, s - pad * 2, 0, this->logo, 1.0f);
        nvgBeginPath(vg);
        nvgRect(vg, tx + pad, ty + pad, s - pad * 2, s - pad * 2);
        nvgFillPaint(vg, img);
        nvgFill(vg);
    }

    // Título
    nvgFontFaceId(vg, ctx->fontStash->regular);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgFontSize(vg, 44);
    nvgFillColor(vg, nvgRGB(255, 255, 255));
    nvgBeginPath(vg);
    nvgText(vg, cx, cy + s / 2 + 52, "JNX MOD", nullptr);

    // Barra de progresso (anima suave até o passo atual)
    const float target = this->done ? 1.0f : (float)this->step / (float)this->total;
    this->shown += (target - this->shown) * 0.08f;
    const float bw = 340, bh = 8, bx = cx - bw / 2, by = cy + s / 2 + 96;

    nvgBeginPath(vg);
    nvgRoundedRect(vg, bx, by, bw, bh, bh / 2);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 34));
    nvgFill(vg);

    const float fw = std::max(bh, bw * (0.06f + 0.94f * this->shown));
    nvgBeginPath(vg);
    nvgRoundedRect(vg, bx, by, fw, bh, bh / 2);
    nvgFillPaint(vg, nvgLinearGradient(vg, bx, 0, bx + bw, 0, nvgRGB(jnx::LILAC_R, jnx::LILAC_G, jnx::LILAC_B), nvgRGB(jnx::VIOLET_R, jnx::VIOLET_G, jnx::VIOLET_B)));
    nvgFill(vg);

    // brilho que passa pela barra
    const float sweep = std::fmod(t * 0.9f, 1.6f) - 0.3f;
    nvgSave(vg);
    nvgIntersectScissor(vg, bx, by, fw, bh);
    nvgBeginPath(vg);
    nvgRect(vg, bx + sweep * bw - 40, by, 80, bh);
    nvgFillPaint(vg, nvgLinearGradient(vg, bx + sweep * bw - 40, 0, bx + sweep * bw + 40, 0, nvgRGBA(255, 255, 255, 0), nvgRGBA(255, 255, 255, 110)));
    nvgFill(vg);
    nvgRestore(vg);

    // Texto da etapa com pontinhos animados
    std::string text;
    {
        std::lock_guard<std::mutex> lock(this->labelMutex);
        text = this->label;
    }
    nvgFontSize(vg, 20);
    nvgFillColor(vg, ctx->theme->descriptionColor);
    nvgBeginPath(vg);
    nvgText(vg, cx, by + 36, text.c_str(), nullptr);

    nvgFontSize(vg, 16);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 90));
    nvgBeginPath(vg);
    nvgText(vg, cx, y + height - 34, "Design by MeoW", nullptr);

    // Abre o app quando terminou (e a tela ficou ao menos um instante visível)
    if (this->done && !this->opened && t > 0.9f && this->shown > 0.97f) {
        this->opened = true;
        brls::Application::pushView(new MainFrame());
    }
}
