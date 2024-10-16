#include "guis/gui_main.hpp"

#include "helpers/save.hpp"
#include "helpers/title.hpp"
#include "helpers/editor_config_parser.hpp"
#include "helpers/account.hpp"

#include "beta_bin.h"
#include "edizon_logo_bin.h"

#include <string>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <numeric>
#include "version.h"
#include "helpers/util.h"
#include "helpers/config.hpp"

static s64 xOffset, xOffsetNext;
static bool finishedDrawing = true;
static s64 startOffset = 0;

static color_t arrowColor;
static bool lastgamenotfound = false;
GuiMain::GuiMain() : Gui() {
  updateEditableTitlesList();

  arrowColor = COLOR_WHITE;
}

GuiMain::~GuiMain() {

}

void GuiMain::update() {
  Gui::update();

  if (xOffset != xOffsetNext && finishedDrawing) {
    double deltaOffset = xOffsetNext - xOffset;
    double scrollSpeed = deltaOffset / 30.0F;

    if (xOffsetNext > xOffset)
      xOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      xOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);

    startOffset = xOffsetNext;
  }

  if (xOffset > 0)
    arrowColor.a = arrowColor.a > 0 ? arrowColor.a - 1 : 0;
  else 
    arrowColor.a = arrowColor.a < 0xFF ? arrowColor.a + 1 : 0xFF;
}

void GuiMain::draw() {
  s64 x = 0, y = 32, currItem = 0;
  s64 selectedX = 0, selectedY = 0;
  bool tmpEditableOnly = m_editableOnly;
  static u32 splashCnt = 0;

  Gui::beginDraw();

  finishedDrawing = false;

  #if SPLASH_ENABLED

    if (!Gui::g_splashDisplayed) {
      Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x5D, 0x4F, 0x4E, 0xFF));
      Gui::drawImage(Gui::g_framebuffer_width / 2 - 128, Gui::g_framebuffer_height / 2 - 128, 256, 256, edizon_logo_bin, IMAGE_MODE_BGR24);

      // if (splashCnt++ >= 70)
        Gui::g_splashDisplayed = true;

      Gui::endDraw();
      return;
    }

  #endif

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  if (Title::g_titles.size() == 0) {
    Config::readConfig();
    if ((Config::getConfig()->lasttitle) != 0) lastgamenotfound = true;
    if (lastgamenotfound)
      Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "在系统里找不到最后一个游戏title的游戏存档！ 请按 \uE0E1 以退出EdiZon!", ALIGNED_CENTER);
    else
      Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "在系统里找不到游戏或存档！ 请按 \uE0E1 以退出EdiZon!", ALIGNED_CENTER);
    Gui::endDraw();
    Config::getConfig()->lasttitle = 0;
    Config::writeConfig();
    return;
  }

  if ((m_editableOnly ? EditorConfigParser::g_editableTitles.size() : Title::g_titles.size()) > 10) {
    Gui::drawRectangle(0, 544, Gui::g_framebuffer_width, 5, currTheme.tooltipColor);

    u32 scrollbarPos = (static_cast<double>(xOffset) / (std::ceil(std::round(m_editableOnly ? EditorConfigParser::g_editableTitles.size() : Title::g_titles.size()) / 4.0F) * 2 * 256)) * Gui::g_framebuffer_width;
    u32 scrollbarWidth = static_cast<double>(Gui::g_framebuffer_width) / ((std::round((m_editableOnly ? EditorConfigParser::g_editableTitles.size() : Title::g_titles.size()) / 2.0F) * 2) / 10.0F);

    Gui::drawRectangle(scrollbarPos, 544, scrollbarWidth, 5, currTheme.tooltipTextColor);
  }

  for (auto title : Title::g_titles) {
    if (currItem == m_selected.titleIndex) {
      selectedX = x - xOffset;
      selectedY = y;
      m_selected.titleId = title.first;
    }

    if (!tmpEditableOnly || EditorConfigParser::g_editableTitles.count(title.first)) {
      if (x - xOffset >= -256 && x - xOffset < Gui::g_framebuffer_width) {
        Gui::drawImage(x - xOffset, y, 256, 256, title.second->getTitleIcon(), IMAGE_MODE_RGB24);

        if (EditorConfigParser::g_betaTitles[title.first])
          Gui::drawImage(x - xOffset, y, 150, 150, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

        if (y == 320 || title.first == (--Title::g_titles.end())->first)
          Gui::drawShadow(x - xOffset, y, 256, 256);

        if (title.first == Title::g_activeTitle) {
          Gui::drawRectangled(x - xOffset, y, 256, 256, Gui::makeColor(0x30, 0x30, 0x30, 0xA0));
          Gui::drawTextAligned(fontTitle, x - xOffset + 245, y + 250, currTheme.selectedColor, "\uE12C", ALIGNED_RIGHT);
        }
      }

      y = y == 32 ? 288 : 32;
      x = floor(++currItem / 2.0F) * 256;
    }
  }

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, 32, currTheme.selectedButtonColor);
  Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 32);

  char timeBuffer[6];
  char batteryBuffer[5];
  getCurrTimeString(timeBuffer);
  getCurrBatteryPercentage(batteryBuffer);

  Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 8, 3, currTheme.separatorColor, timeBuffer, ALIGNED_RIGHT);
  Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 80, 3, currTheme.separatorColor, batteryBuffer, ALIGNED_RIGHT);
  Gui::drawTextAligned(font14, 8, 3, currTheme.separatorColor, "EdiZon SE v" VERSION_STRING, ALIGNED_LEFT);

  Gui::drawRectangled(Gui::g_framebuffer_width - 72, 5, 7, 18, currTheme.separatorColor);
  Gui::drawRectangled(Gui::g_framebuffer_width - 75, 8, 13, 18, currTheme.separatorColor);

  if (tmpEditableOnly && EditorConfigParser::g_editableTitles.size() == 0) {
    Gui::drawTextAligned(font24, (Gui::g_framebuffer_width / 2), (Gui::g_framebuffer_height / 2), currTheme.textColor, "系统里找不到可编辑的游戏！", ALIGNED_CENTER);
    Gui::endDraw();
    return;
  }
  else {
    if (m_selected.titleIndex != -1 && m_selected.extraOption == -1) {
      Gui::drawRectangled(selectedX - 5, selectedY - 5, 266, 266, currTheme.highlightColor);
      Gui::drawImage(selectedX, selectedY, 256, 256, Title::g_titles[m_selected.titleId]->getTitleIcon(), IMAGE_MODE_RGB24);

      if (EditorConfigParser::g_betaTitles[m_selected.titleId])
        Gui::drawImage(selectedX, selectedY, 150, 150, 256, 256, beta_bin, IMAGE_MODE_ABGR32);

      if (m_selected.titleId == Title::g_activeTitle) {
        Gui::drawRectangled(selectedX, selectedY, 256, 256, Gui::makeColor(0x30, 0x30, 0x30, 0xA0));
        Gui::drawTextAligned(fontTitle, selectedX + 245, selectedY + 250, currTheme.selectedColor, "\uE12C", ALIGNED_RIGHT);
      }

      Gui::drawShadow(selectedX - 5, selectedY - 5, 266, 266);
    }

    if (m_selected.extraOption != -1) {
      Gui::drawRectangled(455 + 150 * m_selected.extraOption, 557, 70, 70, currTheme.highlightColor);
    }

    Gui::drawRectangled(458, 560, 64, 64, currTheme.selectedButtonColor);
    Gui::drawRectangled(608, 560, 64, 64, currTheme.selectedButtonColor);
    Gui::drawRectangled(758, 560, 64, 64, currTheme.selectedButtonColor);

    Gui::drawTextAligned(fontTitle, 469, 608, Gui::makeColor(0x11, 0x75, 0xFB, 0xFF), "\uE02B", ALIGNED_LEFT);
    Gui::drawTextAligned(fontTitle, 619, 608, Gui::makeColor(0x34, 0xA8, 0x53, 0xFF), "\uE02E", ALIGNED_LEFT);
    Gui::drawTextAligned(fontTitle, 769, 608, Gui::makeColor(0xEA, 0x43, 0x35, 0xFF), "\uE017", ALIGNED_LEFT);


    Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

    if (m_selected.extraOption == 0)
      Gui::drawTextAligned(font14, 490, 623, currTheme.tooltipTextColor, "金手指", ALIGNED_CENTER);
    else if (m_selected.extraOption == 1)
      Gui::drawTextAligned(font14, 640, 623, currTheme.tooltipTextColor, "指南", ALIGNED_CENTER);
    else if (m_selected.extraOption == 2)
      Gui::drawTextAligned(font14, 790, 623, currTheme.tooltipTextColor, "关于", ALIGNED_CENTER);

    std::string buttonHintStr = "";

    buttonHintStr  = !tmpEditableOnly ? "\uE0E4 配置     \uE0E6 可编辑的所有游戏     " : "\uE0E6 所有游戏     ";
    buttonHintStr += m_backupAll ? "（\uE0E7） + \uE0E2 备份所有     " : "（\uE0E7） + \uE0E2 备份     ";
    buttonHintStr += "\uE0E1 返回     \uE0E0 确认";

    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, buttonHintStr.c_str(), ALIGNED_RIGHT);

    if (m_selected.titleIndex != -1)
      Gui::drawTooltip(selectedX + 128, 288, Title::g_titles[m_selected.titleId]->getTitleName().c_str(), currTheme.tooltipColor, currTheme.tooltipTextColor, 0xFF, m_selected.titleIndex % 2);
  }

  finishedDrawing = true;

  Gui::endDraw();
}

void GuiMain::onInput(u32 kdown) {
  if (kdown & HidNpadButton_B)
    Gui::g_requestExit = true;
  else if (kdown & HidNpadButton_L)
  {
    Gui::g_nextGui = GUI_CHOOSE_MISSION;
  }

  if (Title::g_titles.size() == 0) return;

  if (m_selected.extraOption == -1) { /* one of the titles is selected */
    if (kdown & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight | HidNpadButton_A | HidNpadButton_X)) {
      if (m_selected.titleIndex == -1 || (m_selected.titleIndex / 2 + 1) * 256 < xOffset || (m_selected.titleIndex / 2) * 256 > xOffset + 6 * 256) {
        m_selected.titleIndex = std::ceil(xOffset / 256.0F) * 2;
        return;
      }
    }

    if (kdown & HidNpadButton_AnyLeft) {
      if (static_cast<s16>(m_selected.titleIndex - 2) >= 0)
        m_selected.titleIndex -= 2;
    } else if (kdown & HidNpadButton_AnyRight) {
      if (static_cast<u16>(m_selected.titleIndex + 2) < ((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()))
        m_selected.titleIndex += 2;
    } else if (kdown & HidNpadButton_AnyUp) {
      if ((m_selected.titleIndex % 2) == 1) {
            m_selected.titleIndex--;
      }
    } else if (kdown & HidNpadButton_AnyDown) {
      if ((m_selected.titleIndex % 2) == 0) {
        if (static_cast<u16>(m_selected.titleIndex + 1) < ((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()))
          m_selected.titleIndex++;
        else
          m_selected.extraOption = 0;
      } else {
        if (m_selected.titleIndex < (std::ceil(xOffset / 256.0F) * 2 + 4))
          m_selected.extraOption = 0;
        else if (m_selected.titleIndex < (std::ceil(xOffset / 256.0F) * 2 + 6))
          m_selected.extraOption = 1;
        else
          m_selected.extraOption = 2;

        m_selected.titleIndex = -1;
      }
    }

    if (kdown & HidNpadButton_A) {
      if (m_selected.titleId == Title::g_activeTitle) {
        (new Snackbar("无法访问正在运行的游戏的存档文件。"))->show();
        return;
      }
      AccountUid userID = Gui::requestPlayerSelection();
      std::vector<AccountUid> users = Title::g_titles[m_selected.titleId]->getUserIDs();

      if(userID.uid[0] == 0x00 && userID.uid[1] == 0x00)
        return;

      if (std::find(users.begin(), users.end(), userID) != users.end()) {
        Title::g_currTitle = Title::g_titles[m_selected.titleId];
        Account::g_currAccount = Account::g_accounts[userID];
        Gui::g_nextGui = GUI_EDITOR;
      } else (new Snackbar("没有该用户的存档文件！"))->show();
    }

    if (kdown & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
      if (m_selected.titleIndex != -1) {
        if (m_selected.titleIndex / 2 - (xOffset / 256) > 3)
          xOffsetNext = std::min(static_cast<u32>((m_selected.titleIndex / 2 - 3) * 256), static_cast<u32>(std::ceil(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) / 2.0F - 5) * 256));

        if (m_selected.titleIndex / 2 - (xOffset / 256) < 1)
          xOffsetNext = std::max((m_selected.titleIndex / 2 - 1) * 256, 0);
      }
    }

    if (kdown & HidNpadButton_X) {
      if (m_selected.titleId == Title::g_activeTitle) {
        (new Snackbar("无法访问正在运行的游戏的存档文件。"))->show();
        return;
      }

      if (m_backupAll) {
        (new MessageBox("您确定要备份所有存档 \n 在这个控制台上？ \n 这可能需要一点时间。", MessageBox::YES_NO))->setSelectionAction([&](s8 selection) {
          bool batchFailed = false;

          char backupName[65];
          time_t t = time(nullptr);
          std::stringstream initialText;
          initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");

          if (selection) {
            if(!Gui::requestKeyboardInput("备份名", "请输入要保存的存档名称。", initialText.str(), SwkbdType_QWERTY, backupName, 32))
              return;

            (new MessageBox("创建批量备份。\n \n 这可能需要一点时间。...", MessageBox::NONE))->show();
            requestDraw();

            s16 res;
            u16 failed_titles = 0;

            for (auto title : Title::g_titles) {
              for (AccountUid userID : Title::g_titles[title.first]->getUserIDs()) {
                if((res = backupSave(title.first, userID, true, backupName))) {
                  batchFailed = true;
                  failed_titles++;
                }
              }

              Gui::g_currMessageBox->hide();

              if (!batchFailed)
                (new Snackbar("成功创建备份！"))->show();
              else {
                std::stringstream errorMessage;
                errorMessage << "备份失败 " << failed_titles << " title!";
                (new Snackbar(errorMessage.str()))->show();
              }
            }

            Gui::g_currMessageBox->hide();
          } else Gui::g_currMessageBox->hide();
        })->show();
      }
      else {
        bool batchFailed = false;
        s16 res;

        time_t t = time(nullptr);
        char backupName[65];
        std::stringstream initialText;
        initialText << std::put_time(std::gmtime(&t), "%Y%m%d_%H%M%S");

        if (!Gui::requestKeyboardInput("备份名", "请输入要保存的备份名称。", initialText.str(), SwkbdType_QWERTY, backupName, 32))
          return;

        for (AccountUid userID : Title::g_titles[m_selected.titleId]->getUserIDs()) {
          if((res = backupSave(m_selected.titleId, userID, true, backupName))) {
            batchFailed = true;
          }
        }

        if (!batchFailed)
          (new Snackbar("成功创建备份！"))->show();
        else {
          switch(res) {
            case 1: (new Snackbar("无法挂载存档文件！"))->show(); break;
            case 2: (new Snackbar("该名称的备份已经存在！"))->show(); break;
            case 3: (new Snackbar("创建备份失败！"))->show(); break;
          }
        }
      }
    }
  } else { /* One of the extra options (Cheats, Tutorial or Credits) is selected */
    if (kdown & HidNpadButton_AnyUp) {
      m_selected.titleIndex = std::min(static_cast<u32>(std::ceil(xOffset / 256.0F) * 2 + 2 * m_selected.extraOption + 3), static_cast<u32>(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) - 1));
      m_selected.extraOption = -1;
    }
    else if (kdown & HidNpadButton_AnyLeft) {
      if (m_selected.extraOption > 0)
        m_selected.extraOption--;
    } else if (kdown & HidNpadButton_AnyRight) {
      if (m_selected.extraOption < 2)
        m_selected.extraOption++;
    }

    if (kdown & HidNpadButton_A) {
      switch(m_selected.extraOption) {
        case 0:
          Gui::g_nextGui = GUI_CHEATS;
          break;
        case 1:
          Gui::g_nextGui = GUI_GUIDE;
          break;
        case 2:
          Gui::g_nextGui = GUI_ABOUT;
          break;
        default: break;
      }
    }
  }

  if (kdown & HidNpadButton_ZL) {
    m_editableOnly = !m_editableOnly;
    m_selected.titleIndex = 0;
    xOffsetNext = 0;
  }

  m_backupAll = (kdown & HidNpadButton_ZR) > 0;
}

void GuiMain::onTouch(const HidTouchState &touch) {
  if (((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) == 0) return;

  u8 x = floor((touch.x + xOffset) / 256.0F);
  u8 y = floor((touch.y - 32) / 256.0F);
  u8 title = y + x * 2;

  if (touch.y < 32) return;

  if (y < 2) {
    if (title < ((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size())) {
      if (m_editableOnly && title > (EditorConfigParser::g_editableTitles.size() - 1)) return;

      if (m_selected.titleIndex == title) {
        if (m_selected.titleId == Title::g_activeTitle) {
          (new Snackbar("无法访问正在运行的游戏的存档文件。"))->show();
          return;
        }

        AccountUid userID = Gui::requestPlayerSelection();

        Title::g_currTitle = Title::g_titles[m_selected.titleId];
        std::vector<AccountUid> users = Title::g_titles[m_selected.titleId]->getUserIDs();

        if(userID.uid[0] == 0x00 && userID.uid[1] == 0x00)
          return;

        if (std::find(users.begin(), users.end(), userID) != users.end()) {
          Title::g_currTitle = Title::g_titles[m_selected.titleId];
          Account::g_currAccount = Account::g_accounts[userID];
          Gui::g_nextGui = GUI_EDITOR;
        } else (new Snackbar("此用户无存档文件！"))->show();
      }

      m_selected.titleIndex = title;
      m_selected.extraOption = -1;
    }
  } else {
    if (touch.y > 560 && touch.y < 624) {
      if (touch.x > 458 && touch.x < 522) { // Touched cheats button
        if (m_selected.extraOption == 0)
          Gui::g_nextGui = GUI_CHEATS;
        else {
          m_selected.extraOption = 0;
          m_selected.titleIndex = -1;
        }
      } else if (touch.x > 608 && touch.x < 672) { // Touched guide button
        if (m_selected.extraOption == 1)
          Gui::g_nextGui = GUI_GUIDE;
        else {
          m_selected.extraOption = 1;
          m_selected.titleIndex = -1;
        }
      } else if (touch.x > 758 && touch.x < 822) { // Touched information button
        if (m_selected.extraOption == 2)
          Gui::g_nextGui = GUI_ABOUT;
        else {
          m_selected.extraOption = 2;
          m_selected.titleIndex = -1;
        }
      }
    }
  }
}

inline s8 sign(s32 value) {
  return (value > 0) - (value < 0);
}

void GuiMain::onGesture(const HidTouchState &startPosition, const HidTouchState &currPosition, bool finish) {
  static std::vector<s32> positions;
  static HidTouchState oldPosition;

  m_selected.titleIndex = -1;
  m_selected.extraOption = -1;

  if (((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) == 0) return;

  if (finish) {
    s32 velocity = (std::accumulate(positions.begin(), positions.end(), 0) / static_cast<s32>(positions.size())) * 2;

    xOffsetNext = std::min(std::max<s32>(xOffset + velocity * 1.5F, 0), 256 * static_cast<s32>(std::ceil(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) / 2.0F - 5)));

    startOffset = xOffsetNext;

    positions.clear();
    oldPosition = {0};
  }
  else {
    xOffset = startOffset + (static_cast<s32>(startPosition.x) - static_cast<s32>(currPosition.x));
    xOffset = std::min(std::max<s32>(xOffset, 0), 256 * static_cast<s32>(std::ceil(((!m_editableOnly) ?  Title::g_titles.size() : EditorConfigParser::g_editableTitles.size()) / 2.0F - 5)));
    xOffsetNext = xOffset;
  }

  if (oldPosition.x != 0x00) {
    s32 pos = static_cast<s32>(oldPosition.x) - static_cast<s32>(currPosition.x);
    if (std::abs(pos) < 400)
      positions.push_back(pos);
  }

  if (positions.size() > 10)
    positions.erase(positions.begin());

  oldPosition = currPosition;
}

void GuiMain::updateEditableTitlesList() {
  EditorConfigParser::g_editableTitles.clear();
  EditorConfigParser::g_betaTitles.clear();

  for (auto title : Title::g_titles) {
    if (EditorConfigParser::hasConfig(title.first) == 0) {
      EditorConfigParser::g_editableTitles.insert({title.first, true});
    }
  }
}
