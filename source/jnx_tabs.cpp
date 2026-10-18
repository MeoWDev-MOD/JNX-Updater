#include "jnx_tabs.hpp"

#include <fmt/core.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>

#include "confirm_page.hpp"
#include "dialogue_page.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include <borealis/glass.hpp>

#include "jnx_theme.hpp"
#include "progress_event.hpp"
#include "utils.hpp"
#include "worker_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

namespace {

    // ---------------------------------------------------------------------------------
    // Instalação
    // ---------------------------------------------------------------------------------

    void downloadTo(const std::string& url, const std::string& path)
    {
        fs::createTree(CONFIG_PATH);
        ProgressEvent::instance().setStatusCode(download::downloadFile(url, path, OFF));
    }

    void requireArchive(const std::string& path)
    {
        if (!util::isArchive(path))
            brls::Application::crash("O arquivo baixado não é um zip válido.\nTente novamente mais tarde.");
    }

    std::string parentDir(const std::string& path)
    {
        size_t pos = path.find_last_of('/');
        return pos == std::string::npos ? "/" : path.substr(0, pos + 1);
    }

    void installPack(const jnx::Pack& pack)
    {
        auto* stages = new brls::StagedAppletFrame();
        stages->setTitle("Pacote JNX");
        stages->addStage(new ConfirmPage(stages, fmt::format("Baixando:\nJNX {} ({})\n\nDe:\n{}", pack.version, pack.size, pack.url)));
        stages->addStage(new WorkerPage(stages, "menus/common/downloading"_i18n, [url = pack.url]() { downloadTo(url, JNX_PACK_FILENAME); }));
        stages->addStage(new WorkerPage(stages, "menus/common/extracting"_i18n, []() {
            chdir(ROOT_PATH);
            requireArchive(JNX_PACK_FILENAME);
            int overwrite = util::showDialogBoxBlocking("Sobrescrever os seus arquivos .ini (Hekate, Atmosphère)?\n\nEscolha \"Não\" para manter as suas configurações atuais.", "Sim", "Não");
            extract::extract(JNX_PACK_FILENAME, ROOT_PATH, overwrite == 1);
        }));
        stages->addStage(new ConfirmPage_Done(stages, "Pacote JNX atualizado!\n\nReinicie o console pelo RCM para aplicar tudo."));
        brls::Application::pushView(stages);
    }

    void installFirmware(const jnx::Item& item)
    {
        auto* stages = new brls::StagedAppletFrame();
        stages->setTitle("Firmware");
        stages->addStage(new ConfirmPage(stages, fmt::format("Baixando:\n{} ({})\n\nDe:\n{}", item.name, item.size, item.url)));
        stages->addStage(new WorkerPage(stages, "menus/common/downloading"_i18n, [url = item.url]() { downloadTo(url, FIRMWARE_FILENAME); }));
        stages->addStage(new WorkerPage(stages, "menus/common/extracting"_i18n, []() {
            chdir(ROOT_PATH);
            requireArchive(FIRMWARE_FILENAME);
            fs::removeDir(FIRMWARE_PATH);
            fs::createTree(FIRMWARE_PATH);
            extract::extract(FIRMWARE_FILENAME, FIRMWARE_PATH);
        }));
        const std::string done = "Firmware extraído em /firmware.\n\nAbra o Daybreak para instalá-lo.";
        if (std::filesystem::exists(DAYBREAK_PATH))
            stages->addStage(new DialoguePage_fw(stages, done));
        else
            stages->addStage(new ConfirmPage_Done(stages, done));
        brls::Application::pushView(stages);
    }

    void installItem(const jnx::Item& item)
    {
        const bool zip = (item.type == "zip");
        auto* stages = new brls::StagedAppletFrame();
        stages->setTitle(item.name);
        stages->addStage(new ConfirmPage(stages, fmt::format("Baixando:\n{} ({})\n\nDe:\n{}", item.name, item.size, item.url)));
        stages->addStage(new WorkerPage(stages, "menus/common/downloading"_i18n, [item, zip]() {
            if (!zip)
                fs::createTree(parentDir(item.dest));
            downloadTo(item.url, zip ? JNX_ITEM_FILENAME : item.dest);
        }));
        if (zip) {
            stages->addStage(new WorkerPage(stages, "menus/common/extracting"_i18n, [item]() {
                chdir(ROOT_PATH);
                requireArchive(JNX_ITEM_FILENAME);
                fs::createTree(item.dest.back() == '/' ? item.dest : item.dest + "/");
                extract::extract(JNX_ITEM_FILENAME, item.dest);
            }));
        }
        stages->addStage(new ConfirmPage_Done(stages, "Tudo pronto!"));
        brls::Application::pushView(stages);
    }

    void showText(const std::string& title, const std::string& text)
    {
        auto* dialog = new brls::Dialog(title + "\n\n" + text);
        dialog->addButton("OK", [dialog](brls::View*) { dialog->close(); });
        dialog->setCancelable(true);
        dialog->open();
    }

    brls::Label* note(const std::string& text)
    {
        return new brls::Label(brls::LabelStyle::DESCRIPTION, text, true);
    }

    // ---------------------------------------------------------------------------------
    // Painel do Início (desenhado à mão, não é uma lista de textos)
    // ---------------------------------------------------------------------------------

    enum class Tone
    {
        Ok,
        Warn,
        Bad,
        Info
    };

    NVGcolor toneColor(Tone t)
    {
        switch (t) {
            case Tone::Ok:
                return nvgRGB(52, 211, 153);
            case Tone::Warn:
                return nvgRGB(251, 191, 36);
            case Tone::Bad:
                return nvgRGB(248, 113, 113);
            default:
                return nvgRGB(jnx::LILAC_R, jnx::LILAC_G, jnx::LILAC_B);
        }
    }

    class Dashboard : public brls::View
    {
    private:
        jnx::System sys;
        const jnx::Catalog& cat;
        int logo = 0;
        NVGcontext* vgForLogo = nullptr;

        // Tamanho de fonte que cabe em maxW (reduz até minSize).
        float fit(NVGcontext* vg, brls::FrameContext* ctx, float size, float minSize, float maxW, const std::string& s)
        {
            nvgFontFaceId(vg, ctx->fontStash->regular);
            for (; size > minSize; size -= 1.0f) {
                nvgFontSize(vg, size);
                float bounds[4];
                nvgTextBounds(vg, 0, 0, s.c_str(), nullptr, bounds);
                if (bounds[2] - bounds[0] <= maxW)
                    break;
            }
            return size;
        }

        void text(NVGcontext* vg, brls::FrameContext* ctx, float x, float y, float size, NVGcolor color, const std::string& s, float maxW = 0, float minSize = 12)
        {
            if (maxW > 0)
                size = fit(vg, ctx, size, minSize, maxW, s);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgFontSize(vg, size);
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgFillColor(vg, a(color));
            nvgBeginPath(vg);
            nvgText(vg, x, y, s.c_str(), nullptr);
        }

        void chip(NVGcontext* vg, brls::FrameContext* ctx, float right, float cy, Tone tone, const std::string& s)
        {
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgFontSize(vg, 19);
            float bounds[4];
            nvgTextBounds(vg, 0, 0, s.c_str(), nullptr, bounds);
            float w = bounds[2] - bounds[0] + 44, h = 34, x = right - w;
            NVGcolor c = toneColor(tone);
            NVGcolor bg = c;
            bg.a = 0.20f;
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, cy - h / 2, w, h, h / 2);
            nvgFillColor(vg, a(bg));
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x + 0.5f, cy - h / 2 + 0.5f, w - 1, h - 1, h / 2);
            nvgStrokeColor(vg, a(nvgRGBA(255, 255, 255, 46)));
            nvgStrokeWidth(vg, 1.0f);
            nvgStroke(vg);
            nvgBeginPath(vg);
            nvgCircle(vg, x + 19, cy, 5);
            nvgFillColor(vg, a(c));
            nvgFill(vg);
            text(vg, ctx, x + 33, cy, 19, c, s);
        }

        // Pixel art do console (grade de 32x14 "pixels" de tamanho p).
        void pixels(NVGcontext* vg, float x, float y, int p, int col, int row, int w, int h, NVGcolor c)
        {
            nvgBeginPath(vg);
            nvgRect(vg, x + col * p, y + row * p, w * p, h * p);
            nvgFillColor(vg, a(c));
            nvgFill(vg);
        }

        void screen(NVGcontext* vg, float x, float y, int p, int col, int row, int w, int h, NVGcolor top, NVGcolor bottom)
        {
            for (int i = 0; i < h; i++) {
                float k = h > 1 ? (float)i / (h - 1) : 0.0f;
                NVGcolor c = nvgRGB((int)((top.r + (bottom.r - top.r) * k) * 255), (int)((top.g + (bottom.g - top.g) * k) * 255), (int)((top.b + (bottom.b - top.b) * k) * 255));
                pixels(vg, x, y, p, col, row + i, w, 1, c);
            }
            // reflexo diagonal da tela
            for (int i = 0; i < 4 && i < h - 1; i++)
                pixels(vg, x, y, p, col + 1 + i * 2, row + 1 + i, 3, 1, nvgRGBA(255, 255, 255, 70));
        }

        void console(NVGcontext* vg, float x, float y, int p, jnx::Model kind)
        {
            const NVGcolor dark = nvgRGB(24, 22, 40), body = nvgRGB(38, 36, 58);
            if (kind == jnx::Model::Lite) {
                const NVGcolor t = nvgRGB(45, 212, 191), t2 = nvgRGB(13, 148, 136), tl = nvgRGB(153, 246, 228);
                pixels(vg, x, y, p, 1, 0, 30, 1, t);
                pixels(vg, x, y, p, 0, 1, 32, 12, t);
                pixels(vg, x, y, p, 1, 13, 30, 1, t2);
                pixels(vg, x, y, p, 0, 12, 32, 1, t2);
                pixels(vg, x, y, p, 1, 1, 30, 1, tl);
                pixels(vg, x, y, p, 7, 1, 18, 12, dark);
                screen(vg, x, y, p, 8, 2, 16, 10, nvgRGB(150, 112, 255), nvgRGB(66, 36, 180));
                pixels(vg, x, y, p, 2, 3, 3, 3, dark);   // analógico esquerdo
                pixels(vg, x, y, p, 3, 8, 1, 3, dark);   // direcional
                pixels(vg, x, y, p, 2, 9, 3, 1, dark);
                pixels(vg, x, y, p, 27, 3, 1, 1, dark);  // botões A B X Y
                pixels(vg, x, y, p, 26, 4, 1, 1, dark);
                pixels(vg, x, y, p, 28, 4, 1, 1, dark);
                pixels(vg, x, y, p, 27, 5, 1, 1, dark);
                pixels(vg, x, y, p, 26, 8, 3, 3, dark);  // analógico direito
                return;
            }

            const bool oled = (kind == jnx::Model::Oled);
            const NVGcolor L = oled ? nvgRGB(236, 236, 242) : nvgRGB(56, 189, 248);
            const NVGcolor Ld = oled ? nvgRGB(190, 190, 204) : nvgRGB(14, 116, 190);
            const NVGcolor R = oled ? nvgRGB(236, 236, 242) : nvgRGB(248, 83, 83);
            const NVGcolor Rd = oled ? nvgRGB(190, 190, 204) : nvgRGB(185, 28, 28);
            const NVGcolor btn = oled ? nvgRGB(90, 90, 110) : nvgRGB(20, 40, 70);
            const NVGcolor btnR = oled ? nvgRGB(90, 90, 110) : nvgRGB(80, 14, 14);

            // Joy-Con esquerdo
            pixels(vg, x, y, p, 1, 0, 4, 1, L);
            pixels(vg, x, y, p, 0, 1, 6, 12, L);
            pixels(vg, x, y, p, 1, 13, 4, 1, Ld);
            pixels(vg, x, y, p, 0, 12, 6, 1, Ld);
            pixels(vg, x, y, p, 5, 1, 1, 11, Ld);
            pixels(vg, x, y, p, 2, 3, 2, 2, btn);
            pixels(vg, x, y, p, 3, 8, 1, 3, btn);
            pixels(vg, x, y, p, 2, 9, 3, 1, btn);
            // Joy-Con direito
            pixels(vg, x, y, p, 27, 0, 4, 1, R);
            pixels(vg, x, y, p, 26, 1, 6, 12, R);
            pixels(vg, x, y, p, 27, 13, 4, 1, Rd);
            pixels(vg, x, y, p, 26, 12, 6, 1, Rd);
            pixels(vg, x, y, p, 26, 1, 1, 11, Rd);
            pixels(vg, x, y, p, 29, 3, 1, 1, btnR);
            pixels(vg, x, y, p, 28, 4, 1, 1, btnR);
            pixels(vg, x, y, p, 30, 4, 1, 1, btnR);
            pixels(vg, x, y, p, 29, 5, 1, 1, btnR);
            pixels(vg, x, y, p, 28, 8, 2, 2, btnR);
            // corpo e tela
            pixels(vg, x, y, p, 6, 0, 20, 14, oled ? dark : body);
            if (oled)
                screen(vg, x, y, p, 7, 1, 18, 12, nvgRGB(150, 112, 255), nvgRGB(66, 36, 180));
            else
                screen(vg, x, y, p, 8, 2, 16, 10, nvgRGB(130, 98, 240), nvgRGB(60, 34, 160));
        }

        void modelCard(NVGcontext* vg, brls::FrameContext* ctx, float x, float y, float w, float h)
        {
            brls::glass::panel(vg, x, y, w, h, 28, this->getAlpha());
            brls::glass::tile(vg, x + 14, y + 14, 34, brls::glass::CHIP, nvgRGB(96, 165, 250), nvgRGB(37, 99, 235), this->getAlpha());
            text(vg, ctx, x + 58, y + 31, 15, ctx->theme->descriptionColor, "MODELO", w - 70, 11);

            const int p = std::max(2, std::min(4, (int)((w - 24) / 32)));
            const float ax = x + std::floor((w - 32 * p) / 2.0f), ay = y + 56;
            // sombra no chão
            nvgBeginPath(vg);
            nvgEllipse(vg, x + w / 2, ay + 14 * p + 3, 16 * p, 3);
            nvgFillColor(vg, a(nvgRGBA(0, 0, 0, 70)));
            nvgFill(vg);
            console(vg, ax, ay, p, sys.modelKind);

            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            float size = fit(vg, ctx, 19, 12, w - 20, sys.model);
            nvgFontSize(vg, size);
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgBeginPath(vg);
            nvgText(vg, x + w / 2, y + h - 18, sys.model.c_str(), nullptr);
        }

        void card(NVGcontext* vg, brls::FrameContext* ctx, float x, float y, float w, float h, int glyph, NVGcolor top, NVGcolor bottom,
                  const std::string& label, const std::string& value, const std::string& sub, float bar = -1.0f)
        {
            brls::glass::panel(vg, x, y, w, h, 28, this->getAlpha());
            brls::glass::tile(vg, x + 14, y + 14, 34, glyph, top, bottom, this->getAlpha());

            const float inner = w - 28;
            text(vg, ctx, x + 58, y + 31, 15, ctx->theme->descriptionColor, label, w - 70, 11);
            text(vg, ctx, x + 14, y + 76, 30, ctx->theme->textColor, value, inner, 16);
            if (bar >= 0.0f) {
                nvgBeginPath(vg);
                nvgRoundedRect(vg, x + 14, y + 98, inner, 6, 3);
                nvgFillColor(vg, a(nvgRGBA(255, 255, 255, 36)));
                nvgFill(vg);
                nvgBeginPath(vg);
                nvgRoundedRect(vg, x + 14, y + 98, std::max(6.0f, inner * bar), 6, 3);
                nvgFillPaint(vg, nvgLinearGradient(vg, x + 14, 0, x + 14 + inner, 0, a(nvgRGB(jnx::LILAC_R, jnx::LILAC_G, jnx::LILAC_B)), a(nvgRGB(jnx::VIOLET_R, jnx::VIOLET_G, jnx::VIOLET_B))));
                nvgFill(vg);
                text(vg, ctx, x + 14, y + 117, 15, ctx->theme->descriptionColor, sub, inner, 11);
            }
            else {
                text(vg, ctx, x + 14, y + 106, 15, ctx->theme->descriptionColor, sub, inner, 11);
            }
        }

    public:
        Dashboard() : cat(jnx::catalog())
        {
            this->sys = jnx::readSystem();
            this->setHeight(262);
        }

        ~Dashboard()
        {
            if (this->logo > 0 && this->vgForLogo)
                nvgDeleteImage(this->vgForLogo, this->logo);
        }

        void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override
        {
            if (!this->logo) {
                this->vgForLogo = vg;
                this->logo = nvgCreateImage(vg, "romfs:/jnx_logo_white.png", 0);
                if (!this->logo)
                    this->logo = -1;
            }

            const float W = (float)width;
            const float heroH = 108;

            // --- faixa principal: painel de vidro com o logo ---
            brls::glass::panel(vg, x, y, W, heroH, 34, this->getAlpha());
            const float t = 76;
            brls::glass::tile(vg, x + 16, y + (heroH - t) / 2, t, brls::glass::NONE, nvgRGB(167, 139, 250), nvgRGB(109, 40, 217), this->getAlpha());
            if (this->logo > 0) {
                const float pad = 15;
                NVGpaint img = nvgImagePattern(vg, x + 16 + pad, y + (heroH - t) / 2 + pad, t - pad * 2, t - pad * 2, 0, this->logo, this->getAlpha());
                nvgBeginPath(vg);
                nvgRect(vg, x + 16 + pad, y + (heroH - t) / 2 + pad, t - pad * 2, t - pad * 2);
                nvgFillPaint(vg, img);
                nvgFill(vg);
            }
            text(vg, ctx, x + 108, y + 38, 38, nvgRGB(255, 255, 255), "JNX");
            std::string subtitle = sys.installedPack.empty() ? "Pacote JNX não instalado" : "Pacote JNX " + sys.installedPack;
            if (!cat.online)
                subtitle += "  ·  sem conexão";
            else if (jnx::hasPackUpdate(sys, cat) && !sys.installedPack.empty())
                subtitle += "  ·  nova: " + cat.pack.version;
            text(vg, ctx, x + 108, y + 76, 18, ctx->theme->descriptionColor, subtitle, W - 108 - 200, 12);

            // estado geral
            Tone tone;
            std::string state;
            if (!cat.online) {
                tone = Tone::Bad;
                state = "Sem conexão";
            }
            else if (jnx::hasPackUpdate(sys, cat)) {
                tone = Tone::Warn;
                state = sys.installedPack.empty() ? "Não instalado" : "Atualização";
            }
            else {
                tone = Tone::Ok;
                state = "Em dia";
            }
            chip(vg, ctx, x + W - 18, y + heroH / 2, tone, state);

            // --- cartões de vidro ---
            const float gap = 12, cy = y + heroH + 16, ch = 138;
            const float cw = (W - gap * 3) / 4.0f;

            modelCard(vg, ctx, x, cy, cw, ch);

            card(vg, ctx, x + (cw + gap), cy, cw, ch, brls::glass::CHIP, nvgRGB(96, 165, 250), nvgRGB(37, 99, 235), "FIRMWARE", sys.firmware, sys.model);

            std::string amsSub = cat.pack.atmosphere.empty() ? "em uso" : "pacote: " + cat.pack.atmosphere;
            card(vg, ctx, x + (cw + gap) * 2, cy, cw, ch, brls::glass::GLOBE, nvgRGB(45, 212, 191), nvgRGB(13, 148, 136), "ATMOSPHÈRE", sys.atmosphere, amsSub);

            float used = (sys.sdTotal > 0 && sys.sdFree >= 0) ? 1.0f - (float)sys.sdFree / (float)sys.sdTotal : 0.0f;
            card(vg, ctx, x + (cw + gap) * 3, cy, cw, ch, brls::glass::CARD, nvgRGB(251, 191, 36), nvgRGB(217, 119, 6), "CARTÃO SD", jnx::formatGB(sys.sdFree),
                 sys.sdTotal > 0 ? "livres de " + jnx::formatGB(sys.sdTotal) : "livres", used);
        }
    };

}  // namespace

// ---------------------------------------------------------------------------------
// Início
// ---------------------------------------------------------------------------------

JnxHomeTab::JnxHomeTab()
{
    const jnx::Catalog& cat = jnx::catalog();
    jnx::System sys = jnx::readSystem();

    this->addView(new Dashboard());

    if (!cat.motd.empty())
        this->addView(note("◆ " + cat.motd));

    this->addView(new brls::Header("Ações rápidas"));

    if (jnx::hasPackUpdate(sys, cat)) {
        auto* update = new brls::ListItem(sys.installedPack.empty() ? "Instalar o pacote JNX " + cat.pack.version : "Atualizar o pacote JNX para " + cat.pack.version);
        update->setHeight(LISTITEM_HEIGHT);
        update->setValue(cat.pack.size, true);
        update->getClickEvent()->subscribe([pack = cat.pack](brls::View*) { installPack(pack); });
        this->addView(update);
    }

    if (!cat.pack.changelog.empty()) {
        auto* news = new brls::ListItem("Ver novidades da versão " + cat.pack.version);
        news->setHeight(LISTITEM_HEIGHT);
        news->getClickEvent()->subscribe([cat](brls::View*) { showText("JNX " + cat.pack.version, cat.pack.changelog); });
        this->addView(news);
    }

    if (!cat.online)
        this->addView(note("◆ Sem internet: configure o Wi-Fi do console e reabra o aplicativo para verificar as atualizações."));
}

// ---------------------------------------------------------------------------------
// Pacote JNX
// ---------------------------------------------------------------------------------

JnxPackTab::JnxPackTab()
{
    const jnx::Catalog& cat = jnx::catalog();
    jnx::System sys = jnx::readSystem();

    this->addView(note("O JNX é o pacote completo para o seu Switch: Atmosphère, Hekate e os homebrews essenciais, já em português do Brasil.\n\nOs arquivos vêm direto do GitHub do JNX."));
    this->addView(note("Instalado agora: " + (sys.installedPack.empty() ? std::string("nenhum") : sys.installedPack)));

    if (!cat.online || cat.pack.url.empty()) {
        this->addView(note("◆ Não foi possível consultar o GitHub do JNX. Verifique a conexão."));
        return;
    }

    this->addView(new brls::Header("Versão disponível"));
    auto* item = new brls::ListItem("JNX " + cat.pack.version, "Atmosphère " + cat.pack.atmosphere + "  ·  " + cat.pack.date);
    item->setValue(cat.pack.size, true);
    item->getClickEvent()->subscribe([pack = cat.pack](brls::View*) { installPack(pack); });
    this->addView(item);

    if (!cat.pack.changelog.empty()) {
        auto* news = new brls::ListItem("Ver novidades");
        news->setHeight(LISTITEM_HEIGHT);
        news->getClickEvent()->subscribe([cat](brls::View*) { showText("JNX " + cat.pack.version, cat.pack.changelog); });
        this->addView(news);
    }
}

// ---------------------------------------------------------------------------------
// Catálogos
// ---------------------------------------------------------------------------------

JnxCatalogTab::JnxCatalogTab(const std::string& description, const std::vector<jnx::Item>& items, JnxKind kind, const std::string& emptyText)
{
    this->addView(note(description));

    if (!jnx::catalog().online) {
        this->addView(note("◆ Não foi possível consultar o GitHub do JNX. Verifique a conexão."));
        return;
    }
    if (items.empty()) {
        this->addView(note("◆ " + emptyText));
        return;
    }

    for (const auto& it : items) {
        auto* row = new brls::ListItem(it.name, it.desc);
        row->setHeight(it.desc.empty() ? LISTITEM_HEIGHT : brls::Application::getStyle()->List.Item.heightWithSubLabel);
        row->setValue(it.size, true);
        row->getClickEvent()->subscribe([it, kind](brls::View*) {
            if (kind == JnxKind::Firmware)
                installFirmware(it);
            else
                installItem(it);
        });
        this->addView(row);
    }
}

// ---------------------------------------------------------------------------------
// Sobre
// ---------------------------------------------------------------------------------

JnxAboutTab::JnxAboutTab()
{
    this->addView(new brls::Header("JNX MOD"));
    this->addView(note(fmt::format("Versão {}\nDesign by MeoW\n\nAplicativo gratuito de atualização do pacote JNX. Se você pagou por ele, você foi enganado.", APP_VERSION)));

    this->addView(new brls::Header("Créditos"));
    this->addView(note("◆ Este aplicativo é baseado no AIO-Switch-Updater, de HamletDuFromage (GPL-3.0).\n◆ Interface: Borealis, de natinusala.\n◆ Cheats: nx-cheats-db (sthetix) e CheatSlips.\n◆ Bootloader de payload: Team Neptune (TegraExplorer)."));

    this->addView(new brls::Header("Aviso"));
    this->addView(note("◆ O JNX não detém direitos sobre nenhum arquivo listado; todo o crédito pertence aos seus autores.\n◆ O JNX não inclui sigpatches, chaves, firmwares, jogos nem ROMs.\n◆ Código-fonte (GPL-3.0): github.com/MeoWDev-MOD/JNX-Updater"));
}
