#include "jnx.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <fstream>
#include <json.hpp>

#include "current_cfw.hpp"
#include "download.hpp"
#include "utils.hpp"

namespace jnx {

    namespace {

        std::string str(const nlohmann::ordered_json& j, const char* key, const std::string& def = "")
        {
            auto it = j.find(key);
            return (it != j.end() && it->is_string()) ? it->get<std::string>() : def;
        }

        std::vector<Item> items(const nlohmann::ordered_json& root, const char* key, const std::string& defType)
        {
            std::vector<Item> out;
            auto it = root.find(key);
            if (it == root.end() || !it->is_array())
                return out;
            for (const auto& e : *it) {
                if (!e.is_object())
                    continue;
                Item item;
                item.name = str(e, "name");
                item.url = str(e, "url");
                if (item.name.empty() || item.url.empty())
                    continue;
                item.size = str(e, "size");
                item.desc = str(e, "desc");
                item.dest = str(e, "dest", "/");
                item.type = str(e, "type", defType);
                out.push_back(std::move(item));
            }
            return out;
        }

        std::vector<int> numbers(const std::string& s)
        {
            std::vector<int> out;
            int cur = 0;
            bool has = false;
            for (char c : s) {
                if (c >= '0' && c <= '9') {
                    cur = cur * 10 + (c - '0');
                    has = true;
                }
                else if (has) {
                    out.push_back(cur);
                    cur = 0;
                    has = false;
                }
            }
            if (has)
                out.push_back(cur);
            return out;
        }

        std::string trim(const std::string& s)
        {
            size_t a = s.find_first_not_of(" \t\r\n\xEF\xBB\xBF");
            size_t b = s.find_last_not_of(" \t\r\n");
            return a == std::string::npos ? "" : s.substr(a, b - a + 1);
        }

        std::string modelName()
        {
            SetSysProductModel model;
            if (R_FAILED(setsysGetProductModel(&model)))
                return "Switch";
            switch (model) {
                case SetSysProductModel_Hoag:
                    return "Switch Lite";
                case SetSysProductModel_Aula:
                    return "Switch OLED";
                case SetSysProductModel_Iowa:
                    return "Switch (v2)";
                default:
                    return "Switch";
            }
        }

    }  // namespace

    const Catalog& catalog()
    {
        static Catalog cat;
        static bool loaded = false;
        if (loaded)
            return cat;
        loaded = true;

        nlohmann::ordered_json root;
        download::getRequest(JNX_LINKS_URL, root);
        if (!root.is_object() || root.empty())
            return cat;

        cat.online = true;
        if (root.contains("motd") && root["motd"].is_object() && root["motd"].value("enabled", false))
            cat.motd = str(root["motd"], "message");
        if (root.contains("app") && root["app"].is_object())
            cat.appVersion = str(root["app"], "version");
        if (root.contains("pack") && root["pack"].is_object()) {
            const auto& p = root["pack"];
            cat.pack.version = str(p, "version");
            cat.pack.atmosphere = str(p, "atmosphere");
            cat.pack.date = str(p, "date");
            cat.pack.size = str(p, "size");
            cat.pack.url = str(p, "url");
            cat.pack.changelog = str(p, "changelog");
        }
        cat.firmwares = items(root, "firmwares", "zip");
        cat.translations = items(root, "translations", "zip");
        cat.mods = items(root, "mods", "zip");
        cat.toolbox = items(root, "toolbox", "zip");
        return cat;
    }

    const std::string& appLatestTag()
    {
        static std::string tag;
        static bool loaded = false;
        if (!loaded) {
            loaded = true;
            tag = util::getLatestTag(TAGS_INFO);
        }
        return tag;
    }

    const std::string& cheatsVersion()
    {
        static std::string version;
        static bool loaded = false;
        if (!loaded) {
            loaded = true;
            version = util::getCheatsVersion();
        }
        return version;
    }

    void prefetch(const std::function<void(int, int, const char*)>& progress)
    {
        progress(0, 3, "Conectando ao GitHub do JNX...");
        catalog();
        progress(1, 3, "Verificando atualizações do aplicativo...");
        appLatestTag();
        progress(2, 3, "Preparando os cheats...");
        cheatsVersion();
        progress(3, 3, "Pronto!");
    }

    System readSystem()
    {
        System sys;
        sys.model = modelName();

        SetSysFirmwareVersion fw;
        sys.firmware = R_SUCCEEDED(setsysGetFirmwareVersion(&fw)) ? fw.display_version : "?";

        std::string ams = CurrentCfw::getAmsInfo();
        size_t bar = ams.find('|');
        sys.atmosphere = (ams.rfind("Couldn't", 0) == 0) ? "?" : ams.substr(0, bar);

        std::ifstream info(JNX_INFO_FILE);
        std::string line;
        if (info && std::getline(info, line))
            sys.installedPack = trim(line);

        nsGetFreeSpaceSize(NcmStorageId_SdCard, &sys.sdFree);
        nsGetTotalSpaceSize(NcmStorageId_SdCard, &sys.sdTotal);
        return sys;
    }

    int compareVersions(const std::string& a, const std::string& b)
    {
        auto x = numbers(a), y = numbers(b);
        size_t n = std::max(x.size(), y.size());
        x.resize(n, 0);
        y.resize(n, 0);
        for (size_t i = 0; i < n; i++) {
            if (x[i] != y[i])
                return x[i] < y[i] ? -1 : 1;
        }
        return 0;
    }

    bool hasPackUpdate(const System& sys, const Catalog& cat)
    {
        return cat.online && !cat.pack.version.empty() && !cat.pack.url.empty() &&
               (sys.installedPack.empty() || compareVersions(sys.installedPack, cat.pack.version) < 0);
    }

    std::string formatGB(s64 bytes)
    {
        if (bytes < 0)
            return "?";
        return fmt::format("{:.1f} GB", (double)bytes / 1073741824.0);
    }

}  // namespace jnx
