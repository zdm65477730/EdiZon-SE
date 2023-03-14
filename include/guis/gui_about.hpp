#pragma once

#include "guis/gui.hpp"

#include <string>

class GuiAbout : public Gui {
public:
  GuiAbout();
  ~GuiAbout();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(const HidTouchState &touch);
  void onGesture(const HidTouchState &startPosition, const HidTouchState &endPosition, bool finish);
};
