<p align="center"><img src="resources/jnx_logo.png" width="140" alt="JNX"></p>

# JNX MOD (jnx-update)

🌐 **[Ler em português](README.md)**

App (homebrew `.nro`) of the [JNX](https://github.com/MeoWDev-MOD/JNX) pack: updates the pack and downloads
firmwares, translations, mods, tools and cheats straight from the JNX GitHub, without going through a PC.

## How it works

The app reads a single index, [`jnx-links.json`](https://github.com/MeoWDev-MOD/JNX/blob/main/jnx-links.json),
and shows what is new. To release a new version of the pack, just publish a release in the JNX repository
and update that file: every console then sees the update notice.

| Tab | What it does |
|---|---|
| Home | Dashboard with the installed pack, firmware, Atmosphère and SD card space |
| JNX pack | Installs or updates the pack from the GitHub release |
| Firmwares | Downloads and extracts the firmware to `/firmware` (installed through Daybreak) |
| Translations / Mods / Toolbox | Catalogs from `jnx-links.json` |
| Cheats | Community cheat database (nx-cheats-db) and CheatSlips |
| Tools | Reboot to payload, update the app, among others |
| About JNX | Version, credits and legal notice |

The app's interface is in Portuguese.

## Building

Requires [devkitPro](https://devkitpro.org/) (devkitA64 + libnx + the `switch-curl`, `switch-zlib`, `switch-mbedtls`, `switch-minizip` portlibs) and devkitARM for the payload.

```sh
git clone --recurse-submodules https://github.com/MeoWDev-MOD/JNX-Updater.git
cd JNX-Updater
(cd aiosu-forwarder && make)
make
```

Produces `jnx-update.nro`. Put it in `/switch/jnx-update/` on the SD card.

## Origin and license

This project is derived from [AIO-Switch-Updater](https://github.com/HamletDuFromage/aio-switch-updater),
by HamletDuFromage, and uses the [Borealis](https://github.com/natinusala/borealis) library, by natinusala.
It follows the **GPL-3.0** license (see [LICENSE](LICENSE)). The JNX changes (theme, dashboard, tabs and data source)
are in this repository.

JNX does not include sigpatches, keys, firmwares, games or ROMs. All credit for the listed files belongs
to their authors.
