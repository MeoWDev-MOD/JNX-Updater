#include "main_frame.hpp"

#include <fstream>
#include <json.hpp>

#include "download.hpp"
#include "fs.hpp"
#include "jnx_loading.hpp"
#include "jnx_tabs.hpp"
#include <borealis/glass.hpp>
#include "list_download_tab.hpp"
#include "tools_tab.hpp"
#include "utils.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
using json = nlohmann::ordered_json;

namespace {
    constexpr const char AppTitle[] = APP_TITLE;
    constexpr const char AppVersion[] = APP_VERSION;
}  // namespace

MainFrame::MainFrame() : TabFrame()
{
    this->setIcon("romfs:/gui_icon.png");
    this->setTitle(AppTitle);

    s64 freeStorage;
    std::string tag = jnx::appLatestTag();
    this->setFooterText(fmt::format("menus/main/footer_text"_i18n,
                                    (!tag.empty() && tag != AppVersion) ? AppVersion + "menus/main/new_update"_i18n : AppVersion,
                                    R_SUCCEEDED(fs::getFreeStorageSD(freeStorage)) ? (float)freeStorage / 0x40000000 : -1));

    json hideStatus = fs::parseJsonFile(HIDE_TABS_JSON);
    const jnx::Catalog& cat = jnx::catalog();
    bool erista = util::isErista();

    using brls::glass::Glyph;
    auto tab = [this](const char* label, brls::View* view, int glyph, NVGcolor top, NVGcolor bottom) {
        this->addTab(label, view)->setGlyph(glyph, top, bottom);
    };

    tab("Início", new JnxHomeTab(), Glyph::HOME, nvgRGB(167, 139, 250), nvgRGB(109, 40, 217));
    tab("Pacote JNX", new JnxPackTab(), Glyph::PACKAGE, nvgRGB(244, 114, 182), nvgRGB(190, 24, 93));
    this->addSeparator();
    tab("Firmwares", new JnxCatalogTab("Baixa o firmware e extrai em /firmware. Depois instale com o Daybreak.", cat.firmwares, JnxKind::Firmware, "Nenhum firmware publicado ainda."), Glyph::CHIP, nvgRGB(96, 165, 250), nvgRGB(37, 99, 235));
    tab("Traduções", new JnxCatalogTab("Traduções para português feitas pela comunidade. O conteúdo é extraído direto no cartão SD.", cat.translations, JnxKind::Item, "Em breve: as traduções chegam nas próximas versões."), Glyph::GLOBE, nvgRGB(45, 212, 191), nvgRGB(13, 148, 136));
    tab("Modificações", new JnxCatalogTab("Mods e patches para jogos. Leia a descrição de cada um antes de instalar.", cat.mods, JnxKind::Item, "Em breve: os mods chegam nas próximas versões."), Glyph::SLIDERS, nvgRGB(251, 146, 60), nvgRGB(234, 88, 12));
    tab("Toolbox", new JnxCatalogTab("Baixe ou atualize homebrews e overlays sem reinstalar o pacote inteiro.", cat.toolbox, JnxKind::Item, "Nenhum componente publicado ainda."), Glyph::TOOLBOX, nvgRGB(251, 191, 36), nvgRGB(217, 119, 6));
    if (!util::getBoolValue(hideStatus, "cheats"))
        tab("Cheats", new ListDownloadTab(contentType::cheats), Glyph::STAR, nvgRGB(74, 222, 128), nvgRGB(22, 163, 74));
    this->addSeparator();
    if (!util::getBoolValue(hideStatus, "tools"))
        tab("Ferramentas", new ToolsTab(tag, nlohmann::ordered_json::object(), erista, hideStatus), Glyph::GEAR, nvgRGB(148, 163, 184), nvgRGB(71, 85, 105));
    tab("Sobre o JNX", new JnxAboutTab(), Glyph::INFO, nvgRGB(192, 132, 252), nvgRGB(126, 34, 206));

    this->registerAction("", brls::Key::B, [this] { return true; });
}


void MainFrame::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    // Fundo: céu estrelado do JNX, por trás de todo o vidro
    jnx::drawWallpaper(vg, x, y, width, height, this->getAlpha());
    TabFrame::draw(vg, x, y, width, height, style, ctx);
}
