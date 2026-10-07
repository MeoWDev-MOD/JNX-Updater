#pragma once

#include <switch.h>

#include <functional>
#include <string>
#include <vector>

#include "constants.hpp"

// Dados do JNX: catálogo (jnx-links.json no GitHub) e informações do console.
namespace jnx {

    struct Item
    {
        std::string name;
        std::string size;
        std::string desc;
        std::string url;
        std::string dest = "/";  // pasta de destino (zip) ou caminho completo do arquivo (nro/ovl)
        std::string type = "zip";
    };

    struct Pack
    {
        std::string version;
        std::string atmosphere;
        std::string date;
        std::string size;
        std::string url;
        std::string changelog;
    };

    struct Catalog
    {
        bool online = false;
        std::string motd;
        std::string appVersion;
        Pack pack;
        std::vector<Item> firmwares;
        std::vector<Item> translations;
        std::vector<Item> mods;
        std::vector<Item> toolbox;
    };

    enum class Model
    {
        Standard,  // Switch V1 / V2 (Joy-Cons removiveis)
        Lite,
        Oled,
    };

    struct System
    {
        std::string model;
        Model modelKind = Model::Standard;
        std::string firmware;
        std::string atmosphere;
        std::string installedPack;  // vazio = JNX não instalado
        s64 sdFree = -1;
        s64 sdTotal = -1;
    };

    // Baixa o catálogo uma vez e guarda em memória.
    const Catalog& catalog();
    // Última versão do aplicativo no GitHub e versão do banco de cheats (buscadas uma vez e guardadas).
    const std::string& appLatestTag();
    const std::string& cheatsVersion();
    // Faz todas as buscas de rede do início. progress(passo, total, texto) é chamado a cada etapa.
    void prefetch(const std::function<void(int, int, const char*)>& progress);
    // Lê o estado do console (modelo, firmware, Atmosphère, pacote instalado, SD).
    System readSystem();
    // < 0 se a < b, 0 se iguais, > 0 se a > b. Compara "23.0.0-1" por números.
    int compareVersions(const std::string& a, const std::string& b);
    bool hasPackUpdate(const System& sys, const Catalog& cat);
    std::string formatGB(s64 bytes);

}  // namespace jnx
