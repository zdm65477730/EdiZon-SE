#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <unordered_map>
#include <stdbool.h>
extern std::string m_edizon_dir; 
class GuiChooseMission : public Gui {
public:
  GuiChooseMission();
  ~GuiChooseMission();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);
};
