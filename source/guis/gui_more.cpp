#include "guis/gui_More.hpp"
#include "helpers/config.hpp"

GuiMore::GuiMore() : Gui() {
  Config::getConfig()->option_once = false;
  // m_edizon_dir = Config::getConfig()->edizon_dir;
}
GuiMore::~GuiMore() {
}
void GuiMore::update() {
  Gui::update();
}
static const char *const optionNames[] = {"No Auto Attach \uE0A2", "No Auto Exit after detach \uE0A3", "Disable this screen \uE0B4"};
void GuiMore::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x30, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "More options", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_BLACK, "Use L, R, ZL, ZR and B to choose storage directory for your search press A to continue", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+60, COLOR_BLACK, "Use X, Y, - to toggle options, if you disable this screen use R+B to exit will show this on next launch", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->deletebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0C4 clear all bookmarks", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->freeze ? COLOR_WHITE : COLOR_BLACK, "\uE0C5 freeze game", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, m_edizon_dir.c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"
  for (u8 i = 0; i < 3; i++)
  {
    // Gui::drawRectangled((Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 270, 300, 60, currTheme.separatorColor);
    Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[i] ? COLOR_WHITE : COLOR_BLACK, optionNames[i], ALIGNED_CENTER);
  }
  Gui::drawTextAligned(font20, 65, Gui::g_framebuffer_height / 2 + 300, COLOR_BLACK, "About \uE0B3", ALIGNED_CENTER);
  Gui::endDraw();
}
// u32 kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
void GuiMore::onInput(u32 kdown)
{
  if (kdown & KEY_L)
  {
    m_edizon_dir = "/switch/EdiZon/1";
  }
  else if (kdown & KEY_R)
  {
    m_edizon_dir = "/switch/EdiZon/2";
  }
    if (kdown & KEY_ZL)
  {
    m_edizon_dir = "/switch/EdiZon/3";
  }
  else if (kdown & KEY_ZR)
  {
    m_edizon_dir = "/switch/EdiZon/4";
  }
  else if (kdown & KEY_B)
  {
    m_edizon_dir = "/switch/EdiZon";
  }
  else if (kdown & KEY_X)
  {
    Config::getConfig()->options[0] = !Config::getConfig()->options[0];
  }
  else if (kdown & KEY_Y)
  {
    Config::getConfig()->options[1] = !Config::getConfig()->options[1];
  }
  else if (kdown & KEY_MINUS)
  {
    Config::getConfig()->options[2] = !Config::getConfig()->options[2];
  }
  else if (kdown & KEY_PLUS)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & KEY_LSTICK) 
  {
    Config::getConfig()->deletebookmark = !Config::getConfig()->deletebookmark;
  }
  else if (kdown & KEY_RSTICK) 
  {
    Config::getConfig()->freeze = !Config::getConfig()->freeze;
  }
  else if (kdown & KEY_A)
  {
    Gui::g_nextGui = GUI_CHEATS;
    memcpy(Config::getConfig()->edizon_dir, m_edizon_dir.c_str(), m_edizon_dir.size());
    Config::getConfig()->edizon_dir[m_edizon_dir.size()] = 0;
    Config::writeConfig();
  }
}

void GuiMore::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiMore::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
