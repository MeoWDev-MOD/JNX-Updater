#pragma once

#include <borealis.hpp>

#include <string>
#include <vector>

#include "jnx.hpp"

// Telas do JNX MOD.

// Início: painel com o estado do console e do pacote.
class JnxHomeTab : public brls::List
{
public:
    JnxHomeTab();
};

// Pacote JNX: instala ou atualiza o pacote pelo release do GitHub.
class JnxPackTab : public brls::List
{
public:
    JnxPackTab();
};

enum class JnxKind
{
    Firmware,
    Item,
};

// Lista genérica de downloads do catálogo (firmwares, traduções, mods, toolbox).
class JnxCatalogTab : public brls::List
{
public:
    JnxCatalogTab(const std::string& description, const std::vector<jnx::Item>& items, JnxKind kind, const std::string& emptyText);
};

// Sobre o JNX e créditos.
class JnxAboutTab : public brls::List
{
public:
    JnxAboutTab();
};
