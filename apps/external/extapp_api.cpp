#include <ion.h>
#include <kandinsky.h>
#include <escher.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "archive.h"
#include "extapp_api.h"
#include "../apps_container.h"
#include "../global_preferences.h"

#ifdef DEVICE
#include <ion/src/device/shared/drivers/reset.h>
#include <ion/src/device/shared/drivers/board.h>
#endif

#include <python/port/port.h>

extern "C" {
#include <python/port/mphalport.h>
}

bool iskeydown(int k) {
  Ion::Keyboard::State scan = Ion::Keyboard::scan();
  return scan.keyDown(Ion::Keyboard::Key(k));
}

uint64_t extapp_millis() {
  return Ion::Timing::millis();
}

void extapp_msleep(uint32_t ms) {
  Ion::Timing::msleep(ms);
}

uint64_t extapp_scanKeyboard() {
  return Ion::Keyboard::scan();
}

void extapp_pushRect(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t * pixels) {
  KDRect rect(x, y, w, h);

  Ion::Display::pushRect(rect, reinterpret_cast<const KDColor *>(pixels));
}

void extapp_pushRectUniform(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
  KDRect rect(x, y, w, h);

  Ion::Display::pushRectUniform(rect, KDColor::RGB16(color));
}

void extapp_pullRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t * pixels) {
  KDRect rect(x, y, w, h);

  Ion::Display::pullRect(rect, (KDColor *)pixels);
}

int16_t extapp_drawTextLarge(const char * text, int16_t x, int16_t y, uint16_t fg, uint16_t bg, bool fake) {
  KDPoint point(x, y);

  auto ctx = KDIonContext::sharedContext();
  ctx->setClippingRect(KDRect(0, 0, 320, fake ? 0 : 240));
  ctx->setOrigin(KDPoint(0, 0));
  point = ctx->drawString(text, point, KDFont::LargeFont, KDColor::RGB16(fg), KDColor::RGB16(bg));

  return point.x();
}

int16_t extapp_drawTextSmall(const char * text, int16_t x, int16_t y, uint16_t fg, uint16_t bg, bool fake) {
  KDPoint point(x, y);

  auto ctx = KDIonContext::sharedContext();
  ctx->setClippingRect(KDRect(0, 0, 320, fake ? 0 : 240));
  ctx->setOrigin(KDPoint(0, 0));
  point = ctx->drawString(text, point, KDFont::SmallFont, KDColor::RGB16(fg), KDColor::RGB16(bg));

  return point.x();
}

bool extapp_waitForVBlank() {
  return Ion::Display::waitForVBlank();
}

void extapp_clipboardStore(const char * text) {
  Clipboard::sharedClipboard()->store(text);
}

const char * extapp_clipboardText() {
  return Clipboard::sharedClipboard()->storedText();
}

bool match(const char * filename, const char * extension) {
  return strcmp(filename + strlen(filename) - strlen(extension), extension) == 0;
}

int extapp_fileListWithExtension(const char ** filenames, int maxrecords, const char * extension, int storage) {
  int j = 0;
  if (storage == EXTAPP_RAM_FILE_SYSTEM || storage == EXTAPP_BOTH_FILE_SYSTEM) {
    int n = Ion::Storage::sharedStorage()->numberOfRecordsWithExtension(extension);
    if (n > maxrecords) {
      n = maxrecords;
    }
    for (; j < n; j++) {
      filenames[j] = Ion::Storage::sharedStorage()->recordWithExtensionAtIndex(extension, j).fullName();
    }
    if (j == maxrecords) {
      return j;
    }
  }
  // Don't read external files the exam mode is enabled
  if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode()) return j;
  if (storage == EXTAPP_FLASH_FILE_SYSTEM || storage == EXTAPP_BOTH_FILE_SYSTEM) {
    int n = External::Archive::numberOfFiles();
    for (int i = 0; i < n && j < maxrecords; i++) {
      External::Archive::File entry;
      // Filter extension
      if (External::Archive::fileAtIndex(i, entry) && match(entry.name, extension)) {
        filenames[j] = entry.name;
        ++j;
      }
    }
  }
  return j;
}

bool extapp_fileExists(const char * filename, int storage) {
  if (storage == EXTAPP_RAM_FILE_SYSTEM || storage == EXTAPP_BOTH_FILE_SYSTEM) {
    if (!Ion::Storage::sharedStorage()->recordNamed(filename).isNull())
      return true;
  }
  if (storage == EXTAPP_FLASH_FILE_SYSTEM || storage == EXTAPP_BOTH_FILE_SYSTEM) {
    return External::Archive::indexFromName(filename) >= 0;
  }
  return false;
}

bool extapp_fileErase(const char * filename, int storage) {
  if (storage == EXTAPP_RAM_FILE_SYSTEM) {
    Ion::Storage::Record record = Ion::Storage::sharedStorage()->recordNamed(filename);
    if (record.isNull()) {
      return false;
    }
    record.destroy();
    return true;
  }
  return false;
}

const char * extapp_fileRead(const char * filename, size_t * len, int storage) {
  if (storage == EXTAPP_RAM_FILE_SYSTEM || storage == EXTAPP_BOTH_FILE_SYSTEM) {
    const Ion::Storage::Record record = Ion::Storage::sharedStorage()->recordNamed(filename);
    if (!record.isNull()){
      int delta = 0;
      if (match(filename, ".py") || match(filename, ".xw"))
        delta++;
            // skip record type
      if (len)
        *len = record.value().size - delta;
      return (const char *)record.value().buffer + delta;
    }
  }
  if (storage == EXTAPP_FLASH_FILE_SYSTEM || storage == EXTAPP_BOTH_FILE_SYSTEM) {
    int index = External::Archive::indexFromName(filename);
    if (index >= 0) {
      External::Archive::File entry;
      External::Archive::fileAtIndex(index, entry);
      if (len) {
        *len = entry.dataLength;
      }
      return (const char *)entry.data;
    }
  }
  return NULL;
}

bool extapp_fileWrite(const char * filename, const char * content, size_t len, int storage) {
  if (storage == EXTAPP_RAM_FILE_SYSTEM) {
    Ion::Storage::Record::ErrorStatus status = Ion::Storage::sharedStorage()->createRecordWithFullName(filename, content, len);
    if (status == Ion::Storage::Record::ErrorStatus::NameTaken) {
      Ion::Storage::Record::Data data;
      data.buffer = content;
      data.size = len;
      return Ion::Storage::sharedStorage()->recordNamed(filename).setValue(data) == Ion::Storage::Record::ErrorStatus::None;
    }
    if (status == Ion::Storage::Record::ErrorStatus::None)
      return true;
  }
  return false;
}

static void reloadTitleBar() {
  AppsContainer::sharedAppsContainer()->setShiftAlphaStatus(Ion::Events::shiftAlphaStatus());
  AppsContainer::sharedAppsContainer()->refreshPreferences();
  AppsContainer::sharedAppsContainer()->updateBatteryState();
  AppsContainer::sharedAppsContainer()->reloadTitleBarView();
  AppsContainer::sharedAppsContainer()->redrawWindow();
}

void extapp_lockAlpha() {
  Ion::Events::setShiftAlphaStatus(Ion::Events::ShiftAlphaStatus::AlphaLock);
  reloadTitleBar();
}

void extapp_resetKeyboard() {
  Ion::Events::setShiftAlphaStatus(Ion::Events::ShiftAlphaStatus::Default);
  reloadTitleBar();
}

const int16_t translated_keys[] =
{
  // non shifted
  KEY_CTRL_LEFT,KEY_CTRL_UP,KEY_CTRL_DOWN,KEY_CTRL_RIGHT,KEY_CTRL_OK,KEY_CTRL_EXIT,
  KEY_CTRL_MENU,KEY_PRGM_ACON,KEY_PRGM_ACON,9,10,11,
  KEY_CTRL_SHIFT,KEY_CTRL_ALPHA,KEY_CTRL_XTT,KEY_CTRL_VARS,KEY_CTRL_CATALOG,KEY_CTRL_DEL,
  KEY_CHAR_EXPN,KEY_CHAR_LN,KEY_CHAR_LOG,KEY_CHAR_IMGNRY,',',KEY_CHAR_POW,
  KEY_CHAR_SIN,KEY_CHAR_COS,KEY_CHAR_TAN,KEY_CHAR_PI,KEY_CHAR_ROOT,KEY_CHAR_SQUARE,
  '7','8','9','(',')',-1,
  '4','5','6','*','/',-1,
  '1','2','3','+','-',-1,
  '0','.',KEY_CHAR_EXPN10,KEY_CHAR_ANS,KEY_CTRL_EXE,-1,
  // shifted
  KEY_SHIFT_LEFT,KEY_CTRL_PAGEUP,KEY_CTRL_PAGEDOWN,KEY_SHIFT_RIGHT,KEY_CTRL_OK,KEY_CTRL_EXIT,
  KEY_CTRL_MENU,KEY_PRGM_ACON,KEY_PRGM_ACON,9,10,11,
  KEY_CTRL_SHIFT,KEY_CTRL_ALPHA,KEY_CTRL_CUT,KEY_CTRL_CLIP,KEY_CTRL_PASTE,KEY_CTRL_AC,
  KEY_CHAR_LBRCKT,KEY_CHAR_RBRCKT,KEY_CHAR_LBRACE,KEY_CHAR_RBRACE,'_',KEY_CHAR_STORE,
  KEY_CHAR_ASIN,KEY_CHAR_ACOS,KEY_CHAR_ATAN,'=','<','>',
  KEY_CTRL_F7,KEY_CTRL_F8,KEY_CTRL_F9,KEY_CTRL_F13,KEY_CTRL_F14,-1,
  KEY_CTRL_F4,KEY_CTRL_F5,KEY_CTRL_F6,KEY_CHAR_FACTOR,'%',-1,
  KEY_CTRL_F1,KEY_CTRL_F2,KEY_CTRL_F3,KEY_CHAR_NORMAL,'\\',-1,
  KEY_CTRL_F10,KEY_CTRL_F11,KEY_CTRL_F12,KEY_SHIFT_ANS,KEY_CTRL_EXE,-1,
  // alpha
  KEY_CTRL_LEFT,KEY_CTRL_UP,KEY_CTRL_DOWN,KEY_CTRL_RIGHT,KEY_CTRL_OK,KEY_CTRL_EXIT,
  KEY_CTRL_MENU,KEY_PRGM_ACON,KEY_PRGM_ACON,9,10,11,
  KEY_CTRL_SHIFT,KEY_CTRL_ALPHA,':',';','"',KEY_CTRL_DEL,
  'a','b','c','d','e','f',
  'g','h','i','j','k','l',
  'm','n','o','p','q',-1,
  'r','s','t','u','v',-1,
  'w','x','y','z',' ',-1,
  '?','!',KEY_CHAR_EXPN10,KEY_CHAR_ANS,KEY_CTRL_EXE,-1,
  // alpha shifted
  KEY_SHIFT_LEFT,KEY_CTRL_PAGEUP,KEY_CTRL_PAGEDOWN,KEY_SHIFT_RIGHT,KEY_CTRL_OK,KEY_CTRL_EXIT,
  KEY_CTRL_MENU,KEY_PRGM_ACON,KEY_PRGM_ACON,9,10,11,
  KEY_CTRL_SHIFT,KEY_CTRL_ALPHA,':',';','\'','%',
  'A','B','C','D','E','F',
  'G','H','I','J','K','L',
  'M','N','O','P','Q',-1,
  'R','S','T','U','V',-1,
  'W','X','Y','Z',' ',-1,
  '?','!',KEY_CHAR_EXPN10,KEY_CHAR_ANS,KEY_CTRL_EXE,-1,
};

#ifdef SIMULATOR
#define TICKS_PER_MINUTE 60000
#else
#define TICKS_PER_MINUTE 11862
#endif


int extapp_restorebackup(int mode) {
  // Restoring the backup is allowed even if the write protection is enabled, because it may have been writted by Khi.x
  if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode())
    return 0;
  size_t length = 32 * 1024;
  if (mode == -1) { // restore backup saved when exam mode was set
    uint8_t * src = (uint8_t *)(0x90800000 - 2 * length);
    if (src[0] == 0xba && src[1] == 0xdd && src[2] == 0x0b && src[3] == 0xee) {
      memcpy((uint8_t *)Ion::storageAddress(), src, length);
      return 1;
    }
  }
  if (mode >= 0 && mode < 16) {
    uint8_t * src = (uint8_t *)(0x90180000 + mode * length);
    if (src[0] == 0xba && src[1] == 0xdd && src[2] == 0x0b && src[3] == 0xee) {
      memcpy((uint8_t *)Ion::storageAddress(), src, length);
      return 1;
    }
  }
  return 0;
}

int getkey_raw(int allowSuspend, bool * alphaWasActive) {
  int key = -1;
  size_t t1 = Ion::Timing::millis();
  for (;;) {
    int timeout = 10000;
    if (alphaWasActive) {
      *alphaWasActive = Ion::Events::isAlphaActive();
    }
    Ion::Events::Event event = Ion::Events::getEvent(&timeout);
    reloadTitleBar();
    if (event == Ion::Events::None) {
      size_t t2 = Ion::Timing::millis();
      if (t2 - t1 > 3 * TICKS_PER_MINUTE) {
        event = Ion::Events::OnOff;
      }
    } else {
      t1 = Ion::Timing::millis();
    }
    // TODO: Use the app container to handle the event
    if (event == Ion::Events::USBPlug) {
      // statusline(0,0);
      // KDIonContext::sharedContext()->pushRectUniform(rect,33333);
      if (Ion::USB::isPlugged()) {
        if (GlobalPreferences::sharedGlobalPreferences()->isInExamMode()) {
          Ion::LED::setColor(KDColorBlack);
          Ion::LED::updateColorWithPlugAndCharge();
          GlobalPreferences::sharedGlobalPreferences()->setExamMode(GlobalPreferences::ExamMode::Off);
          // extapp_restorebackup(-1); // temporary
          // displayExamModePopUp(false);
        } else {
          Ion::USB::enable();
        }
        Ion::Backlight::setBrightness(GlobalPreferences::sharedGlobalPreferences()->brightnessLevel());
      } else {
        Ion::USB::disable();
      }
    }
    if (event == Ion::Events::USBEnumeration || event == Ion::Events::USBPlug || event == Ion::Events::BatteryCharging) {
      Ion::LED::updateColorWithPlugAndCharge();
    }
    if (event == Ion::Events::USBEnumeration
      ) {
      auto ctx = KDIonContext::sharedContext();
      //KDRect save=ctx->m_clippingRect;
      //KDPoint o=ctx->m_origin;
      ctx->setClippingRect(KDRect(0, 0, 320, 240));
      ctx->setOrigin(KDPoint(0, 18));
      KDRect rect(90, 63, 140, 75);
      KDIonContext::sharedContext()->pushRectUniform(rect, 64934 /* Palette::YellowDark*/);
      if (Ion::USB::isPlugged()) {
  /* Just after a software update, the battery timer does not have time to
   * fire before the calculator enters DFU mode. As the DFU mode blocks the
   * event loop, we update the battery state "manually" here.
   * We do it before switching to USB application to redraw the battery
   * pictogram. */
  // updateBatteryState();
        KDIonContext::sharedContext()->pushRectUniform(rect, 22222);
        auto ctx = KDIonContext::sharedContext();
#if 0
        size_t length = 32 * 1024;
        unsigned dest = 0x90800000 - 2 * length;
        int sector = Ion::Device::Flash::SectorAtAddress(dest);
        int y = 58;
        char str1[] = "Connecte 000";
        if (sector > 0) {
          str1[9] += (sector / 100) % 100;
          str1[10] += (sector / 10) % 10;
          str1[11] += sector % 10;
        }
        ctx->drawString(str1, KDPoint(100, y), KDFont::LargeFont, 65535, 0);
#else
        int y = 58;
        ctx->drawString("Connecte ! ", KDPoint(100, y), KDFont::LargeFont, 65535, 0);
#endif
        y += 18;
        ctx->drawString(" DFU mode  ", KDPoint(100, y), KDFont::LargeFont, 65535, 0);
        y += 18;
        ctx->drawString("Back quitte", KDPoint(100, y), KDFont::LargeFont, 65535, 0);
        y -= 18;
        Ion::USB::DFU();
        KDIonContext::sharedContext()->pushRectUniform(rect, 44444);
        ctx->drawString("Deconnecte!", KDPoint(100, y), KDFont::LargeFont, 65535, 0);
        // Update LED when exiting DFU mode
        Ion::LED::updateColorWithPlugAndCharge();
      } else {
  /* Sometimes, the device gets an ENUMDNE interrupts when being unplugged
   * from a non-USB communicating host (e.g. a USB charger). The interrupt
   * must me cleared: if not the next enumeration attempts will not be
   * detected. */
        Ion::USB::clearEnumerationInterrupt();
      }
    }
    if (event.isKeyboardEvent()) {
      Ion::Backlight::setBrightness(GlobalPreferences::sharedGlobalPreferences()->brightnessLevel());
    }
    if (event == Ion::Events::Shift || event == Ion::Events::Alpha) {
      continue;
    }
    if (event.isKeyboardEvent()) {
      key = static_cast<uint8_t>(event);
      if (key == (int)Ion::Keyboard::Key::Backspace || key == (int)Ion::Keyboard::Key::OK || key == (int)Ion::Keyboard::Key::Back || key == (int)Ion::Keyboard::Key::EXE) {
        extapp_resetKeyboard();
      }
      // if (allowSuspend && (key == (int)Ion::Keyboard::Key::B2 || key == (int)Ion::Keyboard::Key::OnOff)) {
      if (allowSuspend && key == (int)Ion::Keyboard::Key::OnOff) {
        Ion::Power::suspend(true);
        extapp_pushRectUniform(0, 0, 320, 240, 65535);
        Ion::Backlight::setBrightness(GlobalPreferences::sharedGlobalPreferences()->brightnessLevel());
        reloadTitleBar();
      }
      break;
    }
  }
  return key;
}

int extapp_getKey(int allow_suspend, bool * alphaWasActive) {
  if (allow_suspend & 0x80000000)
    return iskeydown(allow_suspend & 0xff) ? 1 : 0;
  int k = getkey_raw(allow_suspend, alphaWasActive);
  if (k % 54 <= 3) {
    if (iskeydown((int)(int)Ion::Keyboard::Key::Shift))
      return (k % 54) + 31200;
  }
  // translate
  return translated_keys[k];
}

bool extapp_erasesector(void * ptr) {
#ifdef DEVICE
  if (ptr == 0)
    Ion::Device::Reset::core();
  // Disable flash writting
  if (GlobalPreferences::sharedGlobalPreferences()->extapp_write_permission()) {
    int i = Ion::Device::Flash::SectorAtAddress((size_t)ptr);
    if (i < 0)
      return false;
    Ion::Device::Flash::EraseSector(i);
    return true;
  }
#endif
  return false;
}

bool extapp_writememory(unsigned char * dest, const unsigned char * data, size_t length) {
#ifdef DEVICE
  // Disable flash writting
  if (GlobalPreferences::sharedGlobalPreferences()->extapp_write_permission()) {
    int n = Ion::Device::Flash::SectorAtAddress((uint32_t)dest);
    if (n < 0)
      return false;
    Ion::Device::Flash::WriteMemory(dest, (unsigned char *)data, length);
    return true;
  }
#endif
  return false;
}

bool extapp_inexammode() {
  return GlobalPreferences::sharedGlobalPreferences()->isInExamMode();
}

extern "C" void (* const apiPointers[])(void) = {
  (void (*)(void)) extapp_millis,
  (void (*)(void)) extapp_msleep,
  (void (*)(void)) extapp_scanKeyboard,
  (void (*)(void)) extapp_pushRect,
  (void (*)(void)) extapp_pushRectUniform,
  (void (*)(void)) extapp_pullRect,
  (void (*)(void)) extapp_drawTextLarge,
  (void (*)(void)) extapp_drawTextSmall,
  (void (*)(void)) extapp_waitForVBlank,
  (void (*)(void)) extapp_clipboardStore,
  (void (*)(void)) extapp_clipboardText,
  (void (*)(void)) extapp_fileListWithExtension,
  (void (*)(void)) extapp_fileExists,
  (void (*)(void)) extapp_fileErase,
  (void (*)(void)) extapp_fileRead,
  (void (*)(void)) extapp_fileWrite,
  (void (*)(void)) extapp_lockAlpha,
  (void (*)(void)) extapp_resetKeyboard,
  (void (*)(void)) extapp_getKey,
  (void (*)(void)) extapp_restorebackup,
  (void (*)(void)) extapp_erasesector,
  (void (*)(void)) extapp_writememory,
  (void (*)(void)) extapp_inexammode,
  (void (*)(void)) nullptr,
};
