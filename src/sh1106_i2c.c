#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "mgos_i2c.h"

#include "common/cs_dbg.h"

#include "sh1106.h"
#include "fonts.h"

#ifdef __GNUC__
#define UNUSED(x) x __attribute__((unused))
#else
#define UNUSED(x) x
#endif

typedef struct mgos_sh1106
{
  uint8_t address;              // I2C address
  uint8_t width;                // panel width
  uint8_t height;               // panel height
  uint8_t *buffer;              // display buffer
  uint8_t refresh_top;          // 'Dirty' window corners
  uint8_t refresh_left;
  uint8_t refresh_right;
  uint8_t refresh_bottom;
  const font_info_t *font;      // current font
  struct mgos_i2c *i2c;         // i2c connection
} mgos_sh1106;

static struct mgos_sh1106 *s_global_sh1106;

static inline bool _command (struct mgos_sh1106 *oled, uint8_t cmd)
{
  return mgos_i2c_write_reg_b (oled->i2c, oled->address, 0x80, cmd);
}

struct mgos_sh1106 *mgos_sh1106_create (const struct mgos_config_sh1106 *cfg)
{
  struct mgos_sh1106 *oled = NULL;
  oled = calloc (1, sizeof (*oled));
  if (oled == NULL)
    return NULL;

  oled->address = cfg->address;
  oled->width = cfg->width;
  oled->height = cfg->height;
  oled->buffer = calloc (cfg->width * cfg->height / 8, sizeof (uint8_t));
  if (cfg->i2c.enable && cfg->i2c.scl_gpio != -1 && cfg->i2c.sda_gpio != -1) {
    LOG (LL_INFO, ("Using SH1106 GPIO config"));
    struct mgos_config_i2c *i2c_cfg = NULL;
    i2c_cfg = calloc (1, sizeof (*i2c_cfg));
    if (i2c_cfg == NULL)
      return NULL;
    i2c_cfg->enable = cfg->i2c.enable;
    i2c_cfg->unit_no = cfg->i2c.unit_no;
    i2c_cfg->freq = cfg->i2c.freq;
    i2c_cfg->debug = cfg->i2c.debug;
    i2c_cfg->scl_gpio = cfg->i2c.scl_gpio;
    i2c_cfg->sda_gpio = cfg->i2c.sda_gpio;
    oled->i2c = mgos_i2c_create (i2c_cfg);
  } else {
    LOG (LL_INFO, ("Using global GPIO config"));
    oled->i2c = mgos_i2c_get_global ();
  }

  if (oled->i2c == NULL) {
    goto out_err;
  }

  LOG (LL_DEBUG, ("Sending controller startup sequence"));

  #if defined SH1106_128_32
  _command (oled, SH1106_DISPLAYOFF);
  _command (oled, SH1106_SETDISPLAYCLOCKDIV);
  _command (oled, 0x80);        // Suggested value 0x80
  _command (oled, SH1106_SETMULTIPLEX);
  _command (oled, 0x1f);

//  _command (oled, oled->height - 1);
  _command (oled, SH1106_SETDISPLAYOFFSET);
  _command (oled, 0x00);        // 0 no offset
  _command (oled, SH1106_SETSTARTLINE | 0x00);        // SH1106_SETSTARTLINE line #0
  _command (oled, SH1106_CHARGEPUMP);

  LOG (LL_DEBUG, ("128x32: EXTERNALVCC false"));
//    if (vccstate == SH1106_EXTERNALVCC) {
//      _command(oled, 0x10);
//    } else {
      _command(oled, 0x14);
//    }
    _command(oled, SH1106_MEMORYMODE);

  _command (oled, SH1106_SEGREMAP | 0x1);
  _command (oled, SH1106_COMSCANDEC);
  _command (oled, SH1106_SETCOMPINS);
  _command(0x02);
//  _command (oled, oled->height < 64 ? 0x02 : 0x12);
  _command (oled, SH1106_SETCONTRAST);
  _command (oled, 0x8f);        // default contrast ratio
  _command (oled, SH1106_SETPRECHARGE);
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x22);
//  } else {
    _command(oled, 0xf1);
//  }
  _command(oled, SH1106_SETVCOMDETECT);
  _command(oled, SH1106_DISPLAYALLON_RESUME);
  _command(oled, SH1106_NORMALDISPLAY);
  #endif

  #if defined SH1106_128_64
  _command(oled, SH1106_DISPLAYOFF);
  _command(oled, SH1106_SETDISPLAYCLOCKDIV);
  _command(oled, 0x80);
  _command(oled, SH1106_SETMULTIPLEX);
  _command(oled, 0x3F);
  _command(oled, SH1106_SETDISPLAYOFFSET);
  _command(oled, 0x0);
  _command(oled, SH1106_SETSTARTLINE | 0x0);
  _command(oled, SH1106_CHARGEPUMP);
  LOG (LL_DEBUG, ("128x64: EXTERNALVCC false"));
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x10);
//  } else {
    _command(oled, 0x14);
//  }
  _command(oled, SH1106_MEMORYMODE);
  _command(oled, 0x00);
  _command(oled, SH1106_SEGREMAP | 0x1);
  _command(oled, SH1106_COMSCANDEC);
  _command(oled, SH1106_SETCOMPINS);
  _command(oled, 0x12);
  _command(oled, SH1106_SETCONTRAST);
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x9F);
//  } else {
    _command(oled, 0xCF);
//  }
  _command(oled, SH1106_SETPRECHARGE);
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x22);
//  } else {
    _command(oled, 0xF1);
//  }
  _command(oled, SH1106_SETVCOMDETECT);
  _command(oled, 0x40);
  _command(oled, SH1106_DISPLAYALLON_RESUME);
  _command(oled, SH1106_NORMALDISPLAY);
  #endif

  #if defined SH1106_96_16
  _command(oled, SH1106_DISPLAYOFF);
  _command(oled, SH1106_SETDISPLAYCLOCKDIV);
  _command(oled, 0x80);
  _command(oled, SH1106_SETMULTIPLEX);
  _command(oled, 0x0F);
  _command(oled, SH1106_SETDISPLAYOFFSET);
  _command(oled, 0x00);
  _command(oled, SH1106_SETSTARTLINE | 0x0);
  _command(oled, SH1106_CHARGEPUMP);
  LOG (LL_DEBUG, ("96x16: EXTERNALVCC false"));
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x10);
//  } else {
    _command(oled, 0x14);
//  }
  _command(oled, SH1106_MEMORYMODE);
  _command(oled, 0x00);
  _command(oled, SH1106_SEGREMAP | 0x1);
  _command(oled, SH1106_COMSCANDEC);
  _command(oled, SH1106_SETCOMPINS);
  _command(oled, 0x2);
  _command(oled, SH1106_SETCONTRAST);
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x10);
//  } else {
    _command(oled, 0xaf);
//  }
  _command(oled, SH1106_SETPRECHARGE);
//  if (vccstate == SH1106_EXTERNALVCC) {
//    _command(oled, 0x22);
//  } else {
    _command(oled, 0xf1);
//  }
  _command(oled, SH1106_SETVCOMDETECT);
  _command(oled, 0x40);
  _command(oled, SH1106_DISPLAYALLON_RESUME);
  _command(oled, SH1106_NORMALDISPLAY);
  #endif

  LOG (LL_DEBUG, ("Clearing screen buffer"));
  mgos_sh1106_clear (oled);
  mgos_sh1106_refresh (oled, true);
  mgos_sh1106_select_font (oled, 0);

  LOG (LL_DEBUG, ("Turning on display"));
  _command (oled, SH1106_DEACTIVATE_SCROLL);
  _command(oled, SH1106_DISPLAYON);

  LOG (LL_INFO, ("SH1106 init ok (width: %d, height: %d, address: 0x%02x)", oled->width, oled->height, oled->address));
  return oled;

out_err:
  LOG (LL_ERROR, ("SH1106 setup failed"));
  free (oled);
  return NULL;
}

void mgos_sh1106_close (struct mgos_sh1106 *oled)
{
  if (oled == NULL)
    return;

  LOG (LL_INFO, ("SH1106 close"));
  _command (oled, SH1106_DISPLAYOFF);
  _command (oled, SH1106_CHARGEPUMP);
  _command (oled, SH1106_CHARGEPUMPOFF);

  if (oled->i2c)
    mgos_i2c_close (oled->i2c);

  if (oled->buffer)
    free (oled->buffer);

  free (oled);
}

uint8_t mgos_sh1106_get_width (struct mgos_sh1106 *oled)
{
  if (oled == NULL)
    return 0;

  return oled->width;
}

uint8_t mgos_sh1106_get_height (struct mgos_sh1106 * oled)
{
  if (oled == NULL)
    return 0;

  return oled->height;
}

void mgos_sh1106_clear (struct mgos_sh1106 *oled)
{
  if (oled == NULL)
    return;

  LOG (LL_INFO, ("SH1106 clear"));
  memset (oled->buffer, 0, (oled->width * oled->height / 8));
  oled->refresh_right = oled->width - 1;
  oled->refresh_bottom = oled->height - 1;
  oled->refresh_top = 0;
  oled->refresh_left = 0;
}

void mgos_sh1106_refresh (struct mgos_sh1106 *oled, bool force)
{
  uint8_t page_start, page_end;

  if (oled == NULL)
    return;

  if (force || (oled->refresh_top <= 0 && oled->refresh_bottom >= oled->height - 1 && oled->refresh_left <= 0 && oled->refresh_right >= oled->width - 1)) {
    _command (oled, SH1106_COLUMNADDR);
    _command (oled, 0);         // column start
    _command (oled, 127);       // column end
    _command (oled, SH1106_PAGEADDR);
    _command (oled, 0);         // page start
    _command (oled, (oled->height / 8) - 1);    // page end
    mgos_i2c_write_reg_n (oled->i2c, oled->address, 0x40, oled->height * oled->width / 8, oled->buffer);
  } else if ((oled->refresh_top <= oled->refresh_bottom)
             && (oled->refresh_left <= oled->refresh_right)) {
    page_start = oled->refresh_top / 8;
    page_end = oled->refresh_bottom / 8;
    _command (oled, SH1106_COLUMNADDR);
    _command (oled, oled->refresh_left);        // column start
    _command (oled, oled->refresh_right);       // column end
    _command (oled, SH1106_PAGEADDR);
    _command (oled, page_start);        // page start
    _command (oled, page_end);  // page end

    for (uint8_t i = page_start; i <= page_end; ++i) {
      uint16_t start = i * oled->width + oled->refresh_left;
      uint16_t len = oled->refresh_right - oled->refresh_left + 1;
      mgos_i2c_write_reg_n (oled->i2c, oled->address, 0x40, len, oled->buffer + start);
    }
  }
  // reset dirty area
  oled->refresh_top = 255;
  oled->refresh_left = 255;
  oled->refresh_right = 0;
  oled->refresh_bottom = 0;
}

void mgos_sh1106_draw_pixel (struct mgos_sh1106 *oled, int8_t x, int8_t y, mgos_sh1106_color_t color)
{
  uint16_t UNUSED (index) = x + (y / 8) * oled->width;
  if (oled == NULL)
    return;

  if ((x >= oled->width) || (x < 0) || (y >= oled->height) || (y < 0))
    return;

  switch (color) {
  case SH1106_COLOR_WHITE:
    oled->buffer[index] |= (1 << (y & 7));
    break;
  case SH1106_COLOR_BLACK:
    oled->buffer[index] &= ~(1 << (y & 7));
    break;
  case SH1106_COLOR_INVERT:
    oled->buffer[index] ^= (1 << (y & 7));
    break;
  default:
    break;
  }
  if (oled->refresh_left > x)
    oled->refresh_left = x;
  if (oled->refresh_right < x)
    oled->refresh_right = x;
  if (oled->refresh_top > y)
    oled->refresh_top = y;
  if (oled->refresh_bottom < y)
    oled->refresh_bottom = y;
}

void mgos_sh1106_draw_hline (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t w, mgos_sh1106_color_t color)
{
  uint16_t UNUSED (index);
  uint8_t mask, t;

  if (oled == NULL)
    return;
  // boundary check
  if ((x >= oled->width) || (x < 0) || (y >= oled->height) || (y < 0))
    return;
  if (w == 0)
    return;
  if (x + w > oled->width)
    w = oled->width - x;

  t = w;
  index = x + (y / 8) * oled->width;
  mask = 1 << (y & 7);
  switch (color) {
  case SH1106_COLOR_WHITE:
    while (t--) {
      oled->buffer[index] |= mask;
      ++index;
    }
    break;
  case SH1106_COLOR_BLACK:
    mask = ~mask;
    while (t--) {
      oled->buffer[index] &= mask;
      ++index;
    }
    break;
  case SH1106_COLOR_INVERT:
    while (t--) {
      oled->buffer[index] ^= mask;
      ++index;
    }
    break;
  default:
    break;
  }
  if (oled->refresh_left > x)
    oled->refresh_left = x;
  if (oled->refresh_right < x + w - 1)
    oled->refresh_right = x + w - 1;
  if (oled->refresh_top > y)
    oled->refresh_top = y;
  if (oled->refresh_bottom < y)
    oled->refresh_bottom = y;
}

void mgos_sh1106_draw_vline (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t h, mgos_sh1106_color_t color)
{
  uint16_t UNUSED (index);
  uint8_t mask, mod, t;

  if (oled == NULL)
    return;
  // boundary check
  if ((x >= oled->width) || (x < 0) || (y >= oled->height) || (y < 0))
    return;
  if (h == 0)
    return;
  if (y + h > oled->height)
    h = oled->height - y;

  t = h;
  index = x + (y / 8) * oled->width;
  mod = y & 7;
  if (mod)                      // partial line that does not fit into byte at top
  {
    // Magic from Adafruit
    mod = 8 - mod;
    static const uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    mask = premask[mod];
    if (t < mod)
      mask &= (0xFF >> (mod - t));
    switch (color) {
    case SH1106_COLOR_WHITE:
      oled->buffer[index] |= mask;
      break;
    case SH1106_COLOR_BLACK:
      oled->buffer[index] &= ~mask;
      break;
    case SH1106_COLOR_INVERT:
      oled->buffer[index] ^= mask;
      break;
    default:
      break;
    }
    if (t < mod)
      goto draw_vline_finish;
    t -= mod;
    index += oled->width;
  }
  if (t >= 8)                   // byte aligned line at middle
  {
    switch (color) {
    case SH1106_COLOR_WHITE:
      do {
        oled->buffer[index] = 0xff;
        index += oled->width;
        t -= 8;
      }
      while (t >= 8);
      break;
    case SH1106_COLOR_BLACK:
      do {
        oled->buffer[index] = 0x00;
        index += oled->width;
        t -= 8;
      }
      while (t >= 8);
      break;
    case SH1106_COLOR_INVERT:
      do {
        oled->buffer[index] = ~oled->buffer[index];
        index += oled->width;
        t -= 8;
      }
      while (t >= 8);
      break;
    default:
      break;
    }
  }
  if (t)                        // // partial line at bottom
  {
    mod = t & 7;
    static const uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
    mask = postmask[mod];
    switch (color) {
    case SH1106_COLOR_WHITE:
      oled->buffer[index] |= mask;
      break;
    case SH1106_COLOR_BLACK:
      oled->buffer[index] &= ~mask;
      break;
    case SH1106_COLOR_INVERT:
      oled->buffer[index] ^= mask;
      break;
    default:
      break;
    }
  }
draw_vline_finish:
  if (oled->refresh_left > x)
    oled->refresh_left = x;
  if (oled->refresh_right < x)
    oled->refresh_right = x;
  if (oled->refresh_top > y)
    oled->refresh_top = y;
  if (oled->refresh_bottom < y + h - 1)
    oled->refresh_bottom = y + h - 1;
  return;
}

void mgos_sh1106_draw_rectangle (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t w, uint8_t h, mgos_sh1106_color_t color)
{
  mgos_sh1106_draw_hline (oled, x, y, w, color);
  mgos_sh1106_draw_hline (oled, x, y + h - 1, w, color);
  mgos_sh1106_draw_vline (oled, x, y, h, color);
  mgos_sh1106_draw_vline (oled, x + w - 1, y, h, color);
}

void mgos_sh1106_fill_rectangle (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t w, uint8_t h, mgos_sh1106_color_t color)
{
  // Can be optimized?
  uint8_t i;
  for (i = x; i < x + w; ++i)
    mgos_sh1106_draw_vline (oled, i, y, h, color);
}

void mgos_sh1106_draw_circle (struct mgos_sh1106 *oled, int8_t x0, int8_t y0, uint8_t r, mgos_sh1106_color_t color)
{
  // Refer to http://en.wikipedia.org/wiki/Midpoint_circle_algorithm for the algorithm

  int8_t x = r;
  int8_t y = 1;
  int16_t radius_err = 1 - x;

  if (oled == NULL)
    return;

  if (r == 0)
    return;

  mgos_sh1106_draw_pixel (oled, x0 - r, y0, color);
  mgos_sh1106_draw_pixel (oled, x0 + r, y0, color);
  mgos_sh1106_draw_pixel (oled, x0, y0 - r, color);
  mgos_sh1106_draw_pixel (oled, x0, y0 + r, color);

  while (x >= y) {
    mgos_sh1106_draw_pixel (oled, x0 + x, y0 + y, color);
    mgos_sh1106_draw_pixel (oled, x0 - x, y0 + y, color);
    mgos_sh1106_draw_pixel (oled, x0 + x, y0 - y, color);
    mgos_sh1106_draw_pixel (oled, x0 - x, y0 - y, color);
    if (x != y) {
      /* Otherwise the 4 drawings below are the same as above, causing
       * problem when color is INVERT
       */
      mgos_sh1106_draw_pixel (oled, x0 + y, y0 + x, color);
      mgos_sh1106_draw_pixel (oled, x0 - y, y0 + x, color);
      mgos_sh1106_draw_pixel (oled, x0 + y, y0 - x, color);
      mgos_sh1106_draw_pixel (oled, x0 - y, y0 - x, color);
    }
    ++y;
    if (radius_err < 0) {
      radius_err += 2 * y + 1;
    } else {
      --x;
      radius_err += 2 * (y - x + 1);
    }

  }
}

void mgos_sh1106_fill_circle (struct mgos_sh1106 *oled, int8_t x0, int8_t y0, uint8_t r, mgos_sh1106_color_t color)
{
  int8_t x = 1;
  int8_t y = r;
  int16_t radius_err = 1 - y;
  int8_t x1;

  if (oled == NULL)
    return;

  if (r == 0)
    return;

  mgos_sh1106_draw_vline (oled, x0, y0 - r, 2 * r + 1, color); // Center vertical line
  while (y >= x) {
    mgos_sh1106_draw_vline (oled, x0 - x, y0 - y, 2 * y + 1, color);
    mgos_sh1106_draw_vline (oled, x0 + x, y0 - y, 2 * y + 1, color);
    if (color != INVERSE) {
      mgos_sh1106_draw_vline (oled, x0 - y, y0 - x, 2 * x + 1, color);
      mgos_sh1106_draw_vline (oled, x0 + y, y0 - x, 2 * x + 1, color);
    }
    ++x;
    if (radius_err < 0) {
      radius_err += 2 * x + 1;
    } else {
      --y;
      radius_err += 2 * (x - y + 1);
    }
  }

  if (color == INVERSE) {
    x1 = x;                     // Save where we stopped

    y = 1;
    x = r;
    radius_err = 1 - x;
    mgos_sh1106_draw_hline (oled, x0 + x1, y0, r - x1 + 1, color);
    mgos_sh1106_draw_hline (oled, x0 - r, y0, r - x1 + 1, color);
    while (x >= y) {
      mgos_sh1106_draw_hline (oled, x0 + x1, y0 - y, x - x1 + 1, color);
      mgos_sh1106_draw_hline (oled, x0 + x1, y0 + y, x - x1 + 1, color);
      mgos_sh1106_draw_hline (oled, x0 - x, y0 - y, x - x1 + 1, color);
      mgos_sh1106_draw_hline (oled, x0 - x, y0 + y, x - x1 + 1, color);
      ++y;
      if (radius_err < 0) {
        radius_err += 2 * y + 1;
      } else {
        --x;
        radius_err += 2 * (y - x + 1);
      }
    }
  }
}

void mgos_sh1106_select_font (struct mgos_sh1106 *oled, uint8_t font)
{
  if (oled == NULL)
    return;
  if (font < NUM_FONTS)
    oled->font = fonts[font];
}

// return character width
uint8_t
mgos_sh1106_draw_char (struct mgos_sh1106 *oled, uint8_t x, uint8_t y, unsigned char c, mgos_sh1106_color_t foreground, mgos_sh1106_color_t background)
{
  uint8_t i, j;
  const uint8_t UNUSED (*bitmap);
  uint8_t line = 0;

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  LOG (LL_INFO, ("Drawing %c at %d,%d", c, x, y));
  // we always have space in the font set
  if ((c < oled->font->char_start) || (c > oled->font->char_end))
    c = ' ';
  c = c - oled->font->char_start;       // c now become index to tables
  bitmap = oled->font->bitmap + oled->font->char_descriptors[c].offset;
  for (j = 0; j < oled->font->height; ++j) {
    for (i = 0; i < oled->font->char_descriptors[c].width; ++i) {
      if (i % 8 == 0) {
        line = bitmap[(oled->font->char_descriptors[c].width + 7) / 8 * j + i / 8];     // line data
      }
      if (line & 0x80) {
        mgos_sh1106_draw_pixel (oled, x + i, y + j, foreground);
      } else {
        switch (background) {
        case SH1106_COLOR_TRANSPARENT:
          // Not drawing for transparent background
          break;
        case SH1106_COLOR_WHITE:
        case SH1106_COLOR_BLACK:
          mgos_sh1106_draw_pixel (oled, x + i, y + j, background);
          break;
        case SH1106_COLOR_INVERT:
          // I don't know why I need invert background
          break;
        }
      }
      line = line << 1;
    }
  }
  return (oled->font->char_descriptors[c].width);
}

uint8_t
mgos_sh1106_draw_string_color (struct mgos_sh1106 * oled, uint8_t x, uint8_t y, char *str, mgos_sh1106_color_t foreground, mgos_sh1106_color_t background)
{
  uint8_t t = x;

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  if (str == NULL)
    return 0;

  while (*str) {
    x += mgos_sh1106_draw_char (oled, x, y, *str, foreground, background);
    ++str;
    if (*str)
      x += oled->font->c;
  }

  return (x - t);
}

uint8_t mgos_sh1106_draw_string (struct mgos_sh1106 * oled, uint8_t x, uint8_t y, char *str)
{
  return mgos_sh1106_draw_string_color (oled, x, y, str, SH1106_COLOR_WHITE, SH1106_COLOR_TRANSPARENT);
}

// return width of string
uint8_t mgos_sh1106_measure_string (struct mgos_sh1106 * oled, char *str)
{
  uint8_t w = 0;
  unsigned char c;

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  while (*str) {
    c = *str;
    // we always have space in the font set
    if ((c < oled->font->char_start) || (c > oled->font->char_end))
      c = ' ';
    c = c - oled->font->char_start;     // c now become index to tables
    w += oled->font->char_descriptors[c].width;
    ++str;
    if (*str)
      w += oled->font->c;
  }
  return w;
}

uint8_t mgos_sh1106_get_font_height (struct mgos_sh1106 * oled)
{

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  return (oled->font->height);
}

uint8_t mgos_sh1106_get_font_c (struct mgos_sh1106 * oled)
{

  if (oled == NULL)
    return 0;

  if (oled->font == NULL)
    return 0;

  return (oled->font->c);
}

void mgos_sh1106_invert_display (struct mgos_sh1106 *oled, bool invert)
{
  if (oled == NULL)
    return;

  if (invert)
    _command (oled, SH1106_INVERTDISPLAY);
  else
    _command (oled, SH1106_NORMALDISPLAY);
}

void mgos_sh1106_flip_display (struct mgos_sh1106 *oled, bool horizontal, bool vertical)
{
  if (oled == NULL)
    return;

  uint8_t compins = oled->height < 64 ? 0x02 : 0x12;
  _command (oled, 0xda);
  _command (oled, compins | (horizontal << 5));
  _command (oled, vertical ? 0xc0 : 0xc8);
}

void mgos_sh1106_update_buffer (struct mgos_sh1106 *oled, uint8_t * data, uint16_t length)
{
  if (oled == NULL)
    return;

  memcpy (oled->buffer, data, (length < (oled->width * oled->height / 8)) ? length : (oled->width * oled->height / 8));
  oled->refresh_right = oled->width - 1;
  oled->refresh_bottom = oled->height - 1;
  oled->refresh_top = 0;
  oled->refresh_left = 0;
}

bool mgos_sh1106_init (void)
{
  if (!mgos_sys_config_get_sh1106_enable ())
    return true;
  s_global_sh1106 = mgos_sh1106_create (mgos_sys_config_get_sh1106 ());
  return (s_global_sh1106 != NULL);
}

struct mgos_sh1106 *mgos_sh1106_get_global (void)
{
  return s_global_sh1106;
}
