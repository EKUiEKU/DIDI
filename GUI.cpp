#include "GUI.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"

#include <array>
#include <ctime>
#include <iomanip>
#include <Pdh.h>
#include <sstream>
#include <vector>
#include <Windows.h>

#include "Config.h"
#include "Hooks.h"


GUI::GUI() noexcept
{
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(FindWindowW(L"Valve001", nullptr));

    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();

    style.ScrollbarSize = 13.0f;
    style.WindowTitleAlign = { 0.5f, 0.5f };

    ImGuiIO& io = ImGui::GetIO();

    auto fonts = io.Fonts;

    fonts->AddFontFromFileTTF(
        "c:/windows/fonts/simhei.ttf",
        13.0f,
        NULL,
        fonts->GetGlyphRangesChineseFull()
    );

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

}

void GUI::render() noexcept
{
    if (!open)
        return;
 
    ImGui::Begin(u8"药水哥 INS键显示/隐藏", nullptr,  ImGuiWindowFlags_NoResize  | ImGuiWindowFlags_NoCollapse);

    ImGuiColorEditFlags misc_flags = 0 | 0 | ImGuiColorEditFlags_AlphaPreview | 0;

    
    ImGui::Checkbox(u8"开启透视", &config->espEnable);

    ImGui::Checkbox(u8"方框透视3D", &config->boxEnable);
    ImGui::Text("  "); ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Checkbox(u8"显示队友", &config->BoxTeammate->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("box_teammate_visiable", config->BoxTeammate->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("box_teammate_invisiable", config->BoxTeammate->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::Checkbox(u8"显示敌人", &config->BoxEnemy->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("box_enemy_visiable", config->BoxEnemy->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("box_enemy_invisiable", config->BoxEnemy->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::EndGroup();


    ImGui::Checkbox(u8"显示血条", &config->healthBarEnable);
    ImGui::Text("  "); ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Checkbox(u8"队友血条", &config->HealthBarTeammate->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthbar_teammate_visiable", config->HealthBarTeammate->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthbar_teammate_invisiable", config->HealthBarTeammate->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::Checkbox(u8"敌人血条", &config->HealthBarEnemy->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthbar_enemy_visiable", config->HealthBarEnemy->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthbar_enemy_invisiable", config->HealthBarEnemy->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::EndGroup();



    ImGui::Checkbox(u8"显示血量", &config->healthValueEnable);
    ImGui::Text("  "); ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Checkbox(u8"队友血量", &config->HealthValueTeammate->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthvalue_teammate_visiable", config->HealthValueTeammate->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthvalue_teammate_invisiable", config->HealthValueTeammate->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::Checkbox(u8"敌人血量", &config->HealthValueEnemy->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthvalue_enemy_visiable", config->HealthValueEnemy->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("healthvalue_enemy_invisiable", config->HealthValueEnemy->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::EndGroup();




    ImGui::Checkbox(u8"显示线条", &config->lineEnable);
    ImGui::Text("  "); ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Checkbox(u8"队友线条", &config->LineTeammate->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("line_teammate_visiable", config->LineTeammate->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("line_teammate_invisiable", config->LineTeammate->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::Checkbox(u8"敌人线条", &config->LineEnemy->enable);
    ImGui::SameLine();
    ImGui::ColorEdit4("line_enemy_visiable", config->LineEnemy->visiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);
    ImGui::SameLine();
    ImGui::ColorEdit4("line_enemy_invisiable", config->LineEnemy->invisiableColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | misc_flags);

    ImGui::EndGroup();

    


    ImGui::NewLine();

    ImGui::Checkbox(u8"开启自瞄", &config->aimbotConfig->enable);

    const char* aimTypeItems[] = { u8"左键", u8"磁吸", u8"ALT" };
    ImGui::Combo(u8"自瞄方式", &config->aimbotConfig->aimType, aimTypeItems, IM_ARRAYSIZE(aimTypeItems));

    const char* aimItems[] = {u8"就近原则", u8"头部", u8"胸部", u8"裆部"};
    ImGui::Combo(u8"瞄准位置", &config->aimbotConfig->aimPlace, aimItems, IM_ARRAYSIZE(aimItems));

    ImGui::InputFloat(u8"瞄准范围", &config->aimbotConfig->aimAngleRange, 0.1f);

    ImGui::InputFloat(u8"拉枪速度", &config->aimbotConfig->smooth, 0.1f);
 

    if (config->aimbotConfig->aimAngleRange < 0.f)
        config->aimbotConfig->aimAngleRange = 0.f;

    if (config->aimbotConfig->aimAngleRange > 360.f)
        config->aimbotConfig->aimAngleRange = 360.f;

    if (config->aimbotConfig->smooth < 1.f)
        config->aimbotConfig->smooth = 1.f;

    if (config->aimbotConfig->smooth > 50.f)
        config->aimbotConfig->smooth = 50.f;



    ImGui::Checkbox(u8"自瞄压枪", &config->aimbotConfig->antPunch);

    ImGui::SliderFloat(u8"压枪X轴", &config->aimbotConfig->antPunshX, 1.0f, 3.0f);

    ImGui::SliderFloat(u8"压枪Y轴", &config->aimbotConfig->antPunshY, 1.0f, 3.0f);
 
    ImGui::Checkbox(u8"无视烟雾", &config->aimbotConfig->ignoreSmoke);
    ImGui::Checkbox(u8"无视闪光", &config->aimbotConfig->ignoreFlash);
    ImGui::Checkbox(u8"瞄准队友", &config->aimbotConfig->aimTeammate);

    const char* styleItems[] = { u8"演技", u8"普通", u8"变态" };
    if (ImGui::Combo(u8"快速配置", &config->aimbotConfig->style, styleItems, IM_ARRAYSIZE(styleItems))) {
        switch (config->aimbotConfig->style)
        {
        case 0:
            config->boxEnable = false;
            config->healthBarEnable = false;
            config->healthValueEnable = false;
            config->lineEnable = false;

            config->aimbotConfig->smooth = 5.f;
            config->aimbotConfig->aimAngleRange = 5.f;
            config->aimbotConfig->aimPlace = NEAST;
            break;
        case 1:
            config->boxEnable = true;
            config->BoxEnemy->enable = true;

            config->healthBarEnable = true;
            config->HealthBarEnemy->enable = true;

            config->healthValueEnable = true;
            config->HealthValueEnemy->enable = true;

            config->lineEnable = true;
            config->LineEnemy->enable = true;

            config->aimbotConfig->smooth = 2.f;
            config->aimbotConfig->aimAngleRange = 35.f;
            config->aimbotConfig->aimPlace = CHEAST;
            break;

        case 2:
            config->boxEnable = true;
            config->BoxEnemy->enable = true;

            config->healthBarEnable = true;
            config->HealthBarEnemy->enable = true;

            config->healthValueEnable = true;
            config->HealthValueEnemy->enable = true;

            config->lineEnable = true;
            config->LineEnemy->enable = true;

            config->aimbotConfig->smooth = 1.f;
            config->aimbotConfig->aimAngleRange = 360.f;
            config->aimbotConfig->aimPlace = HEAD;
            break;
        default:
            break;
        }
    }

    ImGui::NewLine();

    if (ImGui::Button(u8"             关闭             "))
    {
        hooks->restore();
    }
    
    ImGui::End();
}
