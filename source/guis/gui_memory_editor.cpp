#include "guis/gui_memory_editor.hpp"
#include "helpers/config.hpp"

GuiMemoryEditor::GuiMemoryEditor() : Gui() {
  Config::getConfig()->option_once = false;
  // m_edizon_dir = Config::getConfig()->edizon_dir;
}
GuiMemoryEditor::~GuiMemoryEditor() {
}
void GuiMemoryEditor::update() {
  Gui::update();
}
static const char *const optionNames[] = {"没有自动附加 \uE0A2", "分离后没有自动退出 \uE0A3", "禁用此屏幕 \uE0B4"};
void GuiMemoryEditor::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "欢迎", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "使用L，R，ZL，ZR和B键来选择要搜索的存储目录，请按A继续", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+60, COLOR_WHITE, "使用X，Y，-键来切换选项，如果禁用此屏幕，请使用R + B退出，将在下次启动时显示", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+200, Config::getConfig()->disablerangeonunknown ? COLOR_WHITE : COLOR_BLACK, "\uE0AF 禁用未知范围", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+150, Config::getConfig()->separatebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0B1 单独的书签", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+200, Config::getConfig()->enablecheats ? COLOR_WHITE : COLOR_BLACK, "\uE0B2 启用金手指覆写", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width *4 / 5, Gui::g_framebuffer_height / 2+200, COLOR_BLACK, "\uE0B0 更新金手指数据库", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->deletebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0C4 清除所有书签", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 250, (Config::getConfig()->freeze || Config::getConfig()->enabletargetedscan || Config::getConfig()->easymode) ? COLOR_WHITE : COLOR_BLACK, "\uE0C5 More options", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, m_edizon_dir.c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"
  for (u8 i = 0; i < 3; i++)
  {
    // Gui::drawRectangled((Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 270, 300, 60, currTheme.separatorColor);
    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[i] ? COLOR_WHITE : COLOR_BLACK, optionNames[i], ALIGNED_CENTER);
  }
  Gui::drawTextAligned(font20, 65, Gui::g_framebuffer_height / 2 + 300, COLOR_BLACK, "关于 \uE0B3", ALIGNED_CENTER);
  Gui::endDraw();
}
// u32 kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
void GuiMemoryEditor::onInput(u32 kdown)
{
  if (kdown & HidNpadButton_L)
  {
    m_edizon_dir = "/switch/EdiZon/1";
  }
  else if (kdown & HidNpadButton_R)
  {
    m_edizon_dir = "/switch/EdiZon/2";
  }
    if (kdown & HidNpadButton_ZL)
  {
    m_edizon_dir = "/switch/EdiZon/3";
  }
  else if (kdown & HidNpadButton_ZR)
  {
    m_edizon_dir = "/switch/EdiZon/4";
  }
  else if (kdown & HidNpadButton_B)
  {
    m_edizon_dir = "/switch/EdiZon";
  }
  else if (kdown & HidNpadButton_X)
  {
    Config::getConfig()->options[0] = !Config::getConfig()->options[0];
  }
  else if (kdown & HidNpadButton_Y)
  {
    Config::getConfig()->options[1] = !Config::getConfig()->options[1];
  }
  else if (kdown & HidNpadButton_Minus)
  {
    Config::getConfig()->options[2] = !Config::getConfig()->options[2];
  }
  else if (kdown & HidNpadButton_Up)
  {
    Config::getConfig()->disablerangeonunknown = !Config::getConfig()->disablerangeonunknown;
  }
  else if (kdown & HidNpadButton_Right)
  {
    Config::getConfig()->enablecheats = !Config::getConfig()->enablecheats;
  }
  else if (kdown & HidNpadButton_Left)
  {
    Config::getConfig()->separatebookmark = !Config::getConfig()->separatebookmark;
  }
    else if (kdown & HidNpadButton_Down)
  {
    Gui::g_nextGui = GUI_CHEATDB;
  }
  else if (kdown & HidNpadButton_Plus)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & HidNpadButton_StickL) 
  {
    Config::getConfig()->deletebookmark = !Config::getConfig()->deletebookmark;
  }
  else if (kdown & HidNpadButton_StickR) 
  {
    Gui::g_nextGui = GUI_MORE;
    // Config::getConfig()->freeze = !Config::getConfig()->freeze;
  }
  else if (kdown & HidNpadButton_A)
  {
    Gui::g_nextGui = GUI_CHEATS;
    memcpy(Config::getConfig()->edizon_dir, m_edizon_dir.c_str(), m_edizon_dir.size());
    Config::getConfig()->edizon_dir[m_edizon_dir.size()] = 0;
    Config::writeConfig();
  }
}

void GuiMemoryEditor::onTouch(const HidTouchState &touch) {
  if (touch.x > 400 && touch.x < 900 && touch.y > 600 && touch.y < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiMemoryEditor::onGesture(const HidTouchState &startPosition, const HidTouchState &endPosition, bool finish) {

}
