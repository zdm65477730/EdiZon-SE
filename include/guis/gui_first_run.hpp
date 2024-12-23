#pragma once

#include "guis/gui.hpp"

#include <string>

class Guifirstrun : public Gui {
public:
  Guifirstrun();
  ~Guifirstrun();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(const HidTouchState &touch);
  void onGesture(const HidTouchState &startPosition, const HidTouchState &endPosition, bool finish);
};
