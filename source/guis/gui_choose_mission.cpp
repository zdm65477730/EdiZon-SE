#include "guis/gui_choose_mission.hpp"

#include "helpers/config.hpp"

GuiChooseMission::GuiChooseMission() : Gui() {
  Config::getConfig()->hideSX = false;
}

GuiChooseMission::~GuiChooseMission() {

}

void GuiChooseMission::update() {
  Gui::update();
}

void GuiChooseMission::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "\uE150", ALIGNED_CENTER);

  // Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "EdiZon detected that you're running the 'SX OS' CFW. Please note that this CFW has erroneously\n implemented services that can cause unexpected failures, corruption of save data\n or backups, the editor failing to load save files or configs, RAM editing not being\n supported and other issues. For the safety of your Switch, use a free open\n source CFW instead. \n To continue anyway press \uE0E0, otherwise press \uE0E1 to exit.", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "Use L, R, ZL, ZR and B to choose storage directory for your search press A to continue", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, m_edizon_dir.c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"

  // if (!Config::getConfig()->hideSX)
  //   Gui::drawRectangle(Gui::g_framebuffer_width / 2 - 228, Gui::g_framebuffer_height / 2 + 258, 14, 16, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));

  Gui::endDraw();
}
  // while (!(kheld & KEY_ZL))
  // {
  //   hidScanInput();
  //   kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
  //   kdown = hidKeysDown(CONTROLLER_PLAYER_1)|hidKeysDown(CONTROLLER_HANDHELD);
  //   Gui::beginDraw();
  //   Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  //   Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE12C", ALIGNED_CENTER);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "Use L, R and B to choose your sessoin directory press A to continue", ALIGNED_CENTER);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
  //   Gui::endDraw();
  // }
  // if (kheld & KEY_ZR)
  //   m_edizon_dir = "/switch/EdiZon1";
  // if (kheld & KEY_L)
  //   m_edizon_dir = "/switch/EdiZon2";
  // printf("%s\n", m_edizon_dir.c_str());
void GuiChooseMission::onInput(u32 kdown)
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
  else if (kdown & KEY_A)
    Gui::g_nextGui = GUI_CHEATS;
}

void GuiChooseMission::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiChooseMission::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
