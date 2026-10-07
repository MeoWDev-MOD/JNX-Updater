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

        void text(NVGcontext* vg, brls::FrameContext* ctx, float x, float y, float size, NVGcolor color, const std::string& s, int align = NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE)
        {
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgFontSize(vg, size);
            nvgTextAlign(vg, align);
            nvgFillColor(vg, a(color));
            nvgBeginPath(vg);
            nvgText(vg, x, y, s.c_str(), nullptr);
        }

        void chip(NVGcontext* vg, brls::FrameContext* ctx, float right, float cy, Tone tone, const std::string& s)
        {
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgFontSize(vg, 20);
            float bounds[4];
            nvgTextBounds(vg, 0, 0, s.c_str(), nullptr, bounds);
            float w = bounds[2] - bounds[0] + 40, h = 36, x = right - w;
            NVGcolor c = toneColor(tone);
            NVGcolor bg = c;
            bg.a = 0.18f;
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, cy - h / 2, w, h, h / 2);
            nvgFillColor(vg, a(bg));
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgCircle(vg, x + 18, cy, 5);
            nvgFillColor(vg, a(c));
            nvgFill(vg);
            text(vg, ctx, x + 32, cy, 20, c, s);
        }

        void card(NVGcontext* vg, brls::FrameContext* ctx, float x, float y, float w, float h, const std::string& label, const std::string& value, const std::string& sub, Tone tone, float bar = -1.0f)
        {
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, y, w, h, 10);
            nvgFillColor(vg, a(nvgRGB(24, 19, 46)));
            nvgFill(vg);
            // faixa de cor no topo
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, y, w, 4, 2);
            nvgFillColor(vg, a(toneColor(tone)));
            nvgFill(vg);

            text(vg, ctx, x + 16, y + 26, 16, ctx->theme->descriptionColor, label);
            text(vg, ctx, x + 16, y + 56, 30, ctx->theme->textColor, value);
            if (bar >= 0.0f) {
                nvgBeginPath(vg);
                nvgRoundedRect(vg, x + 16, y + 78, w - 32, 6, 3);
                nvgFillColor(vg, a(nvgRGB(46, 38, 82)));
                nvgFill(vg);
                nvgBeginPath(vg);
                nvgRoundedRect(vg, x + 16, y + 78, std::max(6.0f, (w - 32) * bar), 6, 3);
                nvgFillColor(vg, a(nvgRGB(jnx::VIOLET_R, jnx::VIOLET_G, jnx::VIOLET_B)));
                nvgFill(vg);
                text(vg, ctx, x + 16, y + 100, 16, ctx->theme->descriptionColor, sub);
            }
            else {
                text(vg, ctx, x + 16, y + 86, 17, ctx->theme->descriptionColor, sub);
            }
        }

    public:
        Dashboard() : cat(jnx::catalog())
        {
            this->sys = jnx::readSystem();
            this->setHeight(256);
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
                this->logo = nvgCreateImage(vg, "romfs:/jnx_logo.png", 0);
                if (!this->logo)
                    this->logo = -1;
            }

            const float W = (float)width;
            const float heroH = 120;

            // --- faixa principal ---
            NVGpaint grad = nvgLinearGradient(vg, x, y, x + W, y + heroH, nvgRGB(jnx::DEEP_R, jnx::DEEP_G, jnx::DEEP_B), nvgRGB(30, 18, 70));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, x, y, W, heroH, 12);
            nvgFillPaint(vg, grad);
            nvgFill(vg);

            // estrelinhas de pixel subindo (mesma linguagem do ícone)
            static const int stars[][3] = {{0, 96, 200}, {12, 74, 150}, {12, 96, 110}, {50, 58, 230}, {62, 80, 160}, {100, 100, 120}, {112, 42, 220},
                                           {124, 66, 140}, {170, 88, 190}, {182, 28, 120}, {200, 56, 230}, {236, 90, 150}, {260, 34, 110}, {280, 70, 200}};
            for (const auto& s : stars) {
                float sx = x + W - 330 + s[0];
                nvgBeginPath(vg);
                nvgRect(vg, sx, y + s[1], 6, 6);
                nvgFillColor(vg, a(nvgRGBA(255, 255, 255, s[2])));
                nvgFill(vg);
            }

            if (this->logo > 0) {
                NVGpaint img = nvgImagePattern(vg, x + 20, y + 16, 88, 88, 0, this->logo, this->getAlpha());
                nvgBeginPath(vg);
                nvgRoundedRect(vg, x + 20, y + 16, 88, 88, 14);
                nvgFillPaint(vg, img);
                nvgFill(vg);
            }
            text(vg, ctx, x + 128, y + 44, 50, nvgRGB(255, 255, 255), "JNX");
            text(vg, ctx, x + 128, y + 88, 20, nvgRGB(jnx::LILAC_R, jnx::LILAC_G, jnx::LILAC_B), "Pacote de homebrews para Switch");

            // estado geral
            Tone tone;
            std::string state;
            if (!cat.online) {
                tone = Tone::Bad;
                state = "Sem conexão";
            }
            else if (jnx::hasPackUpdate(sys, cat)) {
                tone = Tone::Warn;
                state = sys.installedPack.empty() ? "Pacote não instalado" : "Atualização disponível";
            }
            else {
                tone = Tone::Ok;
                state = "Tudo em dia";
            }
            chip(vg, ctx, x + W - 20, y + 32, tone, state);

            // --- cartões ---
            const float gap = 12, cy = y + heroH + 14, ch = 116;
            const float cw = (W - gap * 3) / 4.0f;

            std::string packValue = sys.installedPack.empty() ? "—" : sys.installedPack;
            std::string packSub = !cat.online ? "Não foi possível verificar" : (jnx::hasPackUpdate(sys, cat) ? "Nova: " + cat.pack.version : "Versão mais recente");
            card(vg, ctx, x, cy, cw, ch, "PACOTE JNX", packValue, packSub, tone);

            card(vg, ctx, x + (cw + gap), cy, cw, ch, "FIRMWARE", sys.firmware, sys.model, Tone::Info);

            std::string amsSub = cat.pack.atmosphere.empty() ? "em uso" : "pacote traz " + cat.pack.atmosphere;
            card(vg, ctx, x + (cw + gap) * 2, cy, cw, ch, "ATMOSPHÈRE", sys.atmosphere, amsSub, Tone::Info);

            float used = (sys.sdTotal > 0 && sys.sdFree >= 0) ? 1.0f - (float)sys.sdFree / (float)sys.sdTotal : 0.0f;
            card(vg, ctx, x + (cw + gap) * 3, cy, cw, ch, "CARTÃO SD", jnx::formatGB(sys.sdFree), sys.sdTotal > 0 ? "livres de " + jnx::formatGB(sys.sdTotal) : "livres", Tone::Info, used);
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
    this->addView(note("◆ O JNX não detém direitos sobre nenhum arquivo listado; todo o crédito pertence aos seus autores.\n◆ O JNX não inclui sigpatches, chaves, firmwares, jogos nem ROMs.\n◆ Código-fonte (GPL-3.0): github.com/vendashson-rgb/JNX-Updater"));
}
