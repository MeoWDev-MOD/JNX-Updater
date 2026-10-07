#pragma once

#include <borealis.hpp>

// Identidade visual do JNX: fundo quase preto com tom violeta, destaque violeta e lilás.
// Sempre escuro, independente do tema do console.
namespace jnx {

    constexpr unsigned char VIOLET_R = 139, VIOLET_G = 92, VIOLET_B = 246;   // destaque principal
    constexpr unsigned char DEEP_R = 92, DEEP_G = 22, DEEP_B = 214;          // violeta profundo (ícone)
    constexpr unsigned char LILAC_R = 196, LILAC_G = 181, LILAC_B = 253;     // lilás claro

    class Theme : public brls::Theme
    {
      public:
        Theme()
        {
            this->backgroundColor[0] = 0.043f;
            this->backgroundColor[1] = 0.035f;
            this->backgroundColor[2] = 0.082f;
            this->backgroundColorRGB = nvgRGB(11, 9, 21);

            this->textColor        = nvgRGB(244, 241, 255);
            this->descriptionColor = nvgRGB(152, 143, 184);

            this->notificationTextColor = nvgRGB(255, 255, 255);
            this->backdropColor         = nvgRGBA(6, 4, 14, 190);

            this->separatorColor = nvgRGBA(255, 255, 255, 0);

            this->sidebarColor          = nvgRGBA(255, 255, 255, 13);
            this->activeTabColor        = nvgRGB(VIOLET_R, VIOLET_G, VIOLET_B);
            this->sidebarSeparatorColor = nvgRGBA(255, 255, 255, 34);

            this->highlightBackgroundColor = nvgRGBA(139, 92, 246, 46);
            this->highlightColor1          = nvgRGB(DEEP_R + 32, DEEP_G + 48, DEEP_B + 32);
            this->highlightColor2          = nvgRGB(LILAC_R, LILAC_G, LILAC_B);

            this->listItemSeparatorColor  = nvgRGBA(255, 255, 255, 26);
            this->listItemValueColor      = nvgRGB(LILAC_R, LILAC_G, LILAC_B);
            this->listItemFaintValueColor = nvgRGB(96, 88, 128);

            this->tableEvenBackgroundColor = nvgRGBA(255, 255, 255, 14);
            this->tableBodyTextColor       = nvgRGB(160, 152, 196);

            this->dropdownBackgroundColor = nvgRGBA(6, 4, 14, 200);

            this->nextStageBulletColor = nvgRGB(96, 88, 128);

            this->spinnerBarColor = nvgRGBA(139, 92, 246, 110);

            this->scrollBarColor       = nvgRGB(139, 92, 246);
            this->scrollBarAlphaNormal = 0.25f;
            this->scrollBarAlphaFull   = 0.6f;

            this->clickAnimationAlpha = 0.3f;

            this->headerRectangleColor = nvgRGB(VIOLET_R, VIOLET_G, VIOLET_B);

            this->buttonPrimaryEnabledBackgroundColor  = nvgRGB(DEEP_R + 32, DEEP_G + 36, DEEP_B + 32);
            this->buttonPrimaryDisabledBackgroundColor = nvgRGB(52, 46, 78);
            this->buttonPrimaryEnabledTextColor        = nvgRGB(255, 255, 255);
            this->buttonPrimaryDisabledTextColor       = nvgRGB(110, 102, 140);
            this->buttonBorderedBorderColor            = nvgRGB(LILAC_R, LILAC_G, LILAC_B);
            this->buttonBorderedTextColor              = nvgRGB(LILAC_R, LILAC_G, LILAC_B);
            this->buttonRegularBackgroundColor         = nvgRGB(34, 27, 64);
            this->buttonRegularTextColor               = nvgRGB(244, 241, 255);
            this->buttonRegularBorderColor             = nvgRGB(60, 50, 104);

            this->dialogColor                = nvgRGB(27, 21, 52);
            this->dialogBackdrop             = nvgRGBA(6, 4, 14, 150);
            this->dialogButtonColor          = nvgRGB(LILAC_R, LILAC_G, LILAC_B);
            this->dialogButtonSeparatorColor = nvgRGB(60, 50, 104);
        }
    };

    // Proporções do JNX: barra lateral compacta para caber os 9 itens, cantos mais redondos.
    class Style : public brls::HorizonStyle
    {
      public:
        Style()
        {
            this->Sidebar.width        = 380;
            this->Sidebar.marginLeft   = 36;
            this->Sidebar.marginRight  = 20;
            this->Sidebar.marginTop    = 20;
            this->Sidebar.marginBottom = 16;
            this->Sidebar.Item.height  = 50;
            this->Sidebar.Item.textSize = 22;
            this->Sidebar.Separator.height = 16;

            this->Highlight.strokeWidth  = 3;
            this->Highlight.cornerRadius = 24.0f;

            this->List.marginLeftRight = 34;
            this->List.marginTopBottom = 24;
        }
    };

}  // namespace jnx
