#include "main_frame.hpp"

#include <fstream>
#include <json.hpp>

#include "download.hpp"
#include "fs.hpp"
#include "jnx_tabs.hpp"
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
    std::string tag = util::getLatestTag(TAGS_INFO);
    this->setFooterText(fmt::format("menus/main/footer_text"_i18n,
                                    (!tag.empty() && tag != AppVersion) ? AppVersion + "menus/main/new_update"_i18n : AppVersion,
                                    R_SUCCEEDED(fs::getFreeStorageSD(freeStorage)) ? (float)freeStorage / 0x40000000 : -1));

    json hideStatus = fs::parseJsonFile(HIDE_TABS_JSON);
    const jnx::Catalog& cat = jnx::catalog();
    bool erista = util::isErista();

    this->addTab("Início", new JnxHomeTab());
    this->addTab("Pacote JNX", new JnxPackTab());
    this->addSeparator();
    this->addTab("Firmwares", new JnxCatalogTab("Baixa o firmware e extrai em /firmware. Depois instale com o Daybreak.", cat.firmwares, JnxKind::Firmware, "Nenhum firmware publicado ainda."));
    this->addTab("Traduções", new JnxCatalogTab("Traduções para português feitas pela comunidade. O conteúdo é extraído direto no cartão SD.", cat.translations, JnxKind::Item, "Em breve: as traduções chegam nas próximas versões."));
    this->addTab("Modificações", new JnxCatalogTab("Mods e patches para jogos. Leia a descrição de cada um antes de instalar.", cat.mods, JnxKind::Item, "Em breve: os mods chegam nas próximas versões."));
    this->addTab("Toolbox", new JnxCatalogTab("Baixe ou atualize homebrews e overlays sem reinstalar o pacote inteiro.", cat.toolbox, JnxKind::Item, "Nenhum componente publicado ainda."));
    if (!util::getBoolValue(hideStatus, "cheats"))
        this->addTab("Cheats", new ListDownloadTab(contentType::cheats));
    this->addSeparator();
    if (!util::getBoolValue(hideStatus, "tools"))
        this->addTab("Ferramentas", new ToolsTab(tag, nlohmann::ordered_json::object(), erista, hideStatus));
    this->addTab("Sobre o JNX", new JnxAboutTab());

    this->registerAction("", brls::Key::B, [this] { return true; });
}
