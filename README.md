<p align="center"><img src="resources/jnx_logo.png" width="140" alt="JNX"></p>

# JNX MOD (jnx-update)

Aplicativo (homebrew `.nro`) do pacote [JNX](https://github.com/vendashson-rgb/JNX): atualiza o pacote e baixa
firmwares, traduções, modificações, ferramentas e cheats direto do GitHub do JNX, sem passar pelo PC.

## Como funciona

O app lê um único índice, o [`jnx-links.json`](https://github.com/vendashson-rgb/JNX/blob/main/jnx-links.json),
e mostra o que há de novo. Para lançar uma versão nova do pacote basta publicar um release no repositório JNX
e atualizar esse arquivo: todos os consoles passam a ver o aviso de atualização.

| Aba | O que faz |
|---|---|
| Início | Painel com o pacote instalado, firmware, Atmosphère e espaço do SD |
| Pacote JNX | Instala ou atualiza o pacote pelo release do GitHub |
| Firmwares | Baixa e extrai o firmware em `/firmware` (instalação pelo Daybreak) |
| Traduções / Modificações / Toolbox | Catálogos do `jnx-links.json` |
| Cheats | Banco de cheats da comunidade (nx-cheats-db) e CheatSlips |
| Ferramentas | Reiniciar em payload, atualizar o app, entre outras |
| Sobre o JNX | Versão, créditos e aviso legal |

## Compilar

Precisa do [devkitPro](https://devkitpro.org/) (devkitA64 + libnx + portlibs `switch-curl`, `switch-zlib`, `switch-mbedtls`, `switch-minizip`) e do devkitARM para o payload.

```sh
git clone --recurse-submodules https://github.com/vendashson-rgb/JNX-Updater.git
cd JNX-Updater
(cd aiosu-forwarder && make)
make
```

Gera `jnx-update.nro`. Coloque em `/switch/jnx-update/` no cartão SD.

## Origem e licença

Este projeto é um derivado do [AIO-Switch-Updater](https://github.com/HamletDuFromage/aio-switch-updater),
de HamletDuFromage, e usa a biblioteca [Borealis](https://github.com/natinusala/borealis), de natinusala.
Segue a licença **GPL-3.0** (veja [LICENSE](LICENSE)). As alterações do JNX (tema, painel, abas e fonte de dados)
estão neste repositório.

O JNX não inclui sigpatches, chaves, firmwares, jogos nem ROMs. Todo o crédito dos arquivos listados pertence
aos seus autores.
