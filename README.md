# SSD1306 OLED Display Driver Support

This library adds support for the Solomon Systech SSD1306 OLED display driver to Mongoose OS.

The default configuration and PIN mapping supports ESP32 boards with the driver hard-wired for I2C communication via GPIO pins 4 and 5. It should work on other platforms, but has not yet been tested.

This driver should support displays of any resolution supported by the SSD1306.

This driver does not yet support SPI operation.

https://mongoose-os.com/software.html
./install.sh
mos build --verbose --platform esp32
