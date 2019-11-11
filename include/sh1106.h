#ifndef SH1106_H
#define SH1106_H

#include "mgos_features.h"

#include <stdbool.h>

#include "mgos_init.h"
#include "mgos_sys_config.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define SH1106_I2C_ADDRESS   0x3C
// Address for 128x32 is 0x3C
// Address for 128x64 is 0x3D (default) or 0x3C (if SA0 is grounded)

#define SH1106_128_64
// #define SH1106_128_32
// #define SH1106_96_16

#if defined SH1106_128_64 && defined SH1106_128_32
  #error "Only one SH1106 display can be specified at once in SH1106.h"
#endif
#if !defined SH1106_128_64 && !defined SH1106_128_32 && !defined SH1106_96_16
  #error "At least one SH1106 display must be specified in SH1106.h"
#endif

#if defined SH1106_128_64
  #define SH1106_LCDWIDTH                  128
  #define SH1106_LCDHEIGHT                 64
#endif
#if defined SH1106_128_32
  #define SH1106_LCDWIDTH                  128
  #define SH1106_LCDHEIGHT                 32
#endif
#if defined SH1106_96_16
  #define SH1106_LCDWIDTH                  96
  #define SH1106_LCDHEIGHT                 16
#endif

#define SH1106_SETCONTRAST 0x81
#define SH1106_DISPLAYALLON_RESUME 0xA4
#define SH1106_DISPLAYALLON 0xA5
#define SH1106_NORMALDISPLAY 0xA6
#define SH1106_INVERTDISPLAY 0xA7
#define SH1106_DISPLAYOFF 0xAE
#define SH1106_DISPLAYON 0xAF

#define SH1106_SETDISPLAYOFFSET 0xD3
#define SH1106_SETCOMPINS 0xDA

#define SH1106_SETVCOMDETECT 0xDB

#define SH1106_SETDISPLAYCLOCKDIV 0xD5
#define SH1106_SETPRECHARGE 0xD9

#define SH1106_SETMULTIPLEX 0xA8

#define SH1106_SETLOWCOLUMN 0x02

#define SH1106_SETHIGHCOLUMN 0x10

#define SH1106_SETSTARTLINE 0x40

#define SH1106_MEMORYMODE 0x20
#define SH1106_COLUMNADDR 0x21
#define SH1106_PAGEADDR   0x22

#define SH1106_COMSCANINC 0xC0
#define SH1106_COMSCANDEC 0xC8

#define SH1106_SEGREMAP 0xA0

#define SH1106_CHARGEPUMP 0x8D
#define SH1106_CHARGEPUMPOFF 0x8b

#define SH1106_EXTERNALVCC 0x1
#define SH1106_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SH1106_ACTIVATE_SCROLL 0x2F
#define SH1106_DEACTIVATE_SCROLL 0x2E
#define SH1106_SET_VERTICAL_SCROLL_AREA 0xA3
#define SH1106_RIGHT_HORIZONTAL_SCROLL 0x26
#define SH1106_LEFT_HORIZONTAL_SCROLL 0x27
#define SH1106_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SH1106_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

  typedef enum
  {
    SH1106_COLOR_TRANSPARENT = -1,     //< Transparent (not drawing)
    SH1106_COLOR_BLACK = 0,    //< Black (pixel off)
    SH1106_COLOR_WHITE = 1,    //< White (or blue, yellow, pixel on)
    SH1106_COLOR_INVERT = 2,   //< Invert pixel (XOR)
  } mgos_sh1106_color_t;

  /**
   * @brief Standard Mongoose-OS init hook.
   *
   * @return True if module was enabled and successfully initialized.
   */
  bool mgos_sh1106_init (void);

  /**
   * @brief Access the SH1106 driver handle that is set up via sysconfig.
   *
   * @return Preconfigured SH1106 driver handle.
   */
  struct mgos_sh1106 *mgos_sh1106_get_global (void);

  /**
   * @brief Initialize the SH1106 driver with the given params. Typically clients
   * don't need to do that manually; mgos has a global SH1106 instance that is created
   * with the params given in system config; use `mgos_sh1106_get_global() to get the
   * global instance
   *
   * @param cfg SH1106 configuration.
   *
   * @return SH1106 driver handle, or NULL if setup failed.
   */
  struct mgos_sh1106 *mgos_sh1106_create (const struct mgos_config_sh1106 *cfg);

  /**
   * @brief Power down the display, close I2C connection, and free memory.
   *
   * @param oled SH1106 driver handle.
   */
  void mgos_sh1106_close (struct mgos_sh1106 *oled);

  /**
   * @brief Get screen width.
   *
   * @param oled SH1106 driver handle
   *
   * @return Screen width, in pixels.
   */
  uint8_t mgos_sh1106_get_width (struct mgos_sh1106 *oled);

  /**
   * @brief Get screen height
   *
   * @param oled SH1106 driver handle
   *
   * @return Screen height, in pixels.
   */
  uint8_t mgos_sh1106_get_height (struct mgos_sh1106 *oled);

  /**
   * @brief Clear the screen bitmap.
   *
   * @param oled SH1106 driver handle.
   */
  void mgos_sh1106_clear (struct mgos_sh1106 *oled);

  /**
   * @brief Refresh the display, sending any dirty regions to the OLED controller for display.
   * Call this after you are finished calling any drawing primitives.
   *
   * @param oled SH1106 driver handle.
   * @param force Redraw the entire bitmap, not just dirty regions.
   */
  void mgos_sh1106_refresh (struct mgos_sh1106 *oled, bool force);

  /**
   * @brief Draw a single pixel.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param color Pixel color.
   */
  void mgos_sh1106_draw_pixel (struct mgos_sh1106 *oled, int8_t x, int8_t y,
                                mgos_sh1106_color_t color);

  /**
   * @brief Draw a horizontal line.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param w Line length.
   * @param color Line color.
   */
  void mgos_sh1106_draw_hline (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t w,
                                mgos_sh1106_color_t color);

  /**
   * @brief Draw a vertical line.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param h Line length.
   * @param color Line color.
   */
  void mgos_sh1106_draw_vline (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t h,
                                mgos_sh1106_color_t color);

  /**
   * @brief Draw an unfilled rectangle.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param w Rectangle width.
   * @param h Rectangle height.
   * @param color Line color.
   */
  void mgos_sh1106_draw_rectangle (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t w, uint8_t h,
                                    mgos_sh1106_color_t color);

  /**
   * @brief Draw a filled rectangle.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param w Rectangle width.
   * @param h Rectangle height.
   * @param color Line and fill color.
   */
  void mgos_sh1106_fill_rectangle (struct mgos_sh1106 *oled, int8_t x, int8_t y, uint8_t w, uint8_t h,
                                    mgos_sh1106_color_t color);

  /**
   * @brief Draw an unfilled circle.
   *
   * @param oled SH1106 driver handle.
   * @param x0 Center X coordinate .
   * @param y0 Center Y coordinate.
   * @param r Radius.
   * @param color Line color.
   */
  void mgos_sh1106_draw_circle (struct mgos_sh1106 *oled, int8_t x0, int8_t y0, uint8_t r, mgos_sh1106_color_t color);

  /**
   * @brief Draw a filled circle.
   *
   * @param oled SH1106 driver handle.
   * @param x0 Center X coordinate.
   * @param y0 Center Y coordinate.
   * @param r Radius.
   * @param color Line and fill color.
   */
  void mgos_sh1106_fill_circle (struct mgos_sh1106 *oled, int8_t x0, int8_t y0, uint8_t r, mgos_sh1106_color_t color);

  /**
   * @brief Select active font ID.
   *
   * @param oled SH1106 driver handle.
   * @param font Font index; see `fonts.h`.
   */
  void mgos_sh1106_select_font (struct mgos_sh1106 *oled, uint8_t font);

  /**
   * @brief Draw a single character using the active font and selected colors.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param c Character to draw.
   * @param foreground Foreground color.
   * @param background Background color.
   *
   * @return Character width in pixels
   */
  uint8_t mgos_sh1106_draw_char (struct mgos_sh1106 *oled, uint8_t x, uint8_t y, unsigned char c,
                                  mgos_sh1106_color_t foreground, mgos_sh1106_color_t background);

  /**
   * @brief Draw a string using the active font and selected colors.
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param str String to draw.
   * @param foreground Foreground color.
   * @param background Background color.
   *
   * @return String witdth in pixels.
   */
  uint8_t mgos_sh1106_draw_string_color (struct mgos_sh1106 *oled, uint8_t x, uint8_t y, char *str,
                                          mgos_sh1106_color_t foreground, mgos_sh1106_color_t background);

  /**
   * @brief Draw a string using the active font and default colors (white on transparent)
   *
   * @param oled SH1106 driver handle.
   * @param x X coordinate.
   * @param y Y coordinate.
   * @param str String to draw.
   *
   * @return String width in pixels.
   */
  uint8_t mgos_sh1106_draw_string (struct mgos_sh1106 *oled, uint8_t x, uint8_t y, char *str);

  /**
   * @brief Measure on-screen width of string if drawn using active font.
   *
   * @param oled SH1106 driver handle.
   * @param str String to measure.
   *
   * @return String width in pixels.
   */
  uint8_t mgos_sh1106_measure_string (struct mgos_sh1106 *oled, char *str);

  /**
   * @brief Get the height of the active font.
   *
   * @param oled SH1106 driver handle.
   *
   * @return Font height in pixels.
   */
  uint8_t mgos_sh1106_get_font_height (struct mgos_sh1106 *oled);

  /**
   * @brief Get the inter-character font spacing.
   *
   * @param oled SH1106 driver handle.
   *
   * @return Font inter-character spacing in pixels.
   */
  uint8_t mgos_sh1106_get_font_c (struct mgos_sh1106 *oled);

  /**
   * @brief Invert or restore the display. Inverting the display flips the bitmask -
   * 1 is black (off) and 0 is white (on).
   *
   * @param oled SH1106 driver handle.
   * @param invert Enable inversion.
   */
  void mgos_sh1106_invert_display (struct mgos_sh1106 *oled, bool invert);

  /**
   * @brief Flip the display render order. May not do exactly what you expect, depending on
   * the display's segment remap configuration.
   *
   * @param oled SH1106 driver handle.
   * @param horizontal Enable horizontal flipping.
   * @param vertical Enable vertical flipping
   */
  void mgos_sh1106_flip_display (struct mgos_sh1106 *oled, bool horizontal, bool vertical);

  /**
   * @brief Copy pre-rendered bytes directly into the bitmap.
   *
   * @param oled SH1106 driver handle.
   * @param data Array containing bytes to copy into buffer.
   * @param length Length to copy (unchecked) from source buffer.
   */
  void mgos_sh1106_update_buffer (struct mgos_sh1106 *oled, uint8_t * data, uint16_t length);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SH1106_H */
