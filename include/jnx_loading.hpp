#pragma once

#include <borealis.hpp>

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

namespace jnx {

    // Fundo do JNX: céu estrelado suavizado, por trás de todo o vidro.
    void drawWallpaper(NVGcontext* vg, int x, int y, unsigned width, unsigned height, float alpha);

}  // namespace jnx

// Tela de carregamento: aparece na hora ao abrir o app e mostra o andamento
// enquanto as buscas na internet rodam em segundo plano. Depois abre o app.
class LoadingPage : public brls::View
{
private:
    std::thread* worker = nullptr;
    std::atomic<int> step{0};
    std::atomic<bool> done{false};
    int total = 3;
    std::mutex labelMutex;
    std::string label = "Iniciando...";
    float shown = 0.0f;
    bool started = false;
    bool opened = false;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    int logo = 0;
    NVGcontext* vgForLogo = nullptr;

public:
    LoadingPage();
    ~LoadingPage();

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) override;
};
