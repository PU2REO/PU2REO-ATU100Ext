//-----------------------------------------------
// Definitions file by PU2REO
// ATU-100 project 2016-2024
//-----------------------------------------------

#define FW_VERSION                      "FW ver 1.0"  // str(10)

#define IDEAL_OPERATING_SWR             110
#define LIMIT_OPERATING_SWR             150
#define MAX_OPERATING_SWR               200

#define OUT_ALL_OFF                     0x00
#define OUT_ALL_ON                      0xFF

#define ADC_REV_INPUT                   0
#define ADC_FWR_INPUT                   1

// Displays
#define DISPLAY_BKLIGHT_FACTOR          2.30

// Buttons
#define BUTTON_TUNE                     PORTB.B0
#define BUTTON_AUTO                     PORTB.B1
#define BUTTON_BYPASS                   PORTB.B2

// Display types
#define DISPLAY_TYPE_DUAL_COLOR_LED     0
#define DISPLAY_TYPE_1602_LCD           1
#define DISPLAY_TYPE_OLED_128X32        2
#define DISPLAY_TYPE_OLED_128X32_INV    3
#define DISPLAY_TYPE_OLED_128X64        4
#define DISPLAY_TYPE_OLED_128X64_INV    5


// EEPROM Addresses
#define EEPROM_DISPLAY_ADDRESS          0x00
#define EEPROM_DISPLAY_TYPE             0x01
#define EEPROM_AUTO_MODE_ENABLED        0x02
#define EEPROM_RELAY_TIMEOUT            0x03
#define EEPROM_SWR_THRESHOLD            0x04
#define EEPROM_MIN_PWR_TO_TUNE          0x05
#define EEPROM_MAX_PWR_TO_TUNE          0x06
#define EEPROM_DISPLAY_SHIFT_DOWN       0x07
#define EEPROM_DISPLAY_SHIFT_LEFT       0x08
#define EEPROM_MAX_INIT_SWR             0x09
#define EEPROM_INSTALLED_INDUCTORS      0x0A
#define EEPROM_LINEAR_INDUCTORS         0x0B
#define EEPROM_INSTALLED_CAPACITORS     0x0C
#define EEPROM_LINEAR_CAPACITORS        0x0D
#define EEPROM_RF_DIODES_CORRECTION     0x0E
#define EEPROM_INV_INDUCTANCE_CTRL      0x0F

#define EEPROM_INDUCTOR_1_1_VALUE       0x10
#define EEPROM_INDUCTOR_1_2_VALUE       0x11
#define EEPROM_INDUCTOR_2_1_VALUE       0x12
#define EEPROM_INDUCTOR_2_2_VALUE       0x13
#define EEPROM_INDUCTOR_3_1_VALUE       0x14
#define EEPROM_INDUCTOR_3_2_VALUE       0x15
#define EEPROM_INDUCTOR_4_1_VALUE       0x16
#define EEPROM_INDUCTOR_4_2_VALUE       0x17
#define EEPROM_INDUCTOR_5_1_VALUE       0x18
#define EEPROM_INDUCTOR_5_2_VALUE       0x19
#define EEPROM_INDUCTOR_6_1_VALUE       0x1A
#define EEPROM_INDUCTOR_6_2_VALUE       0x1B
#define EEPROM_INDUCTOR_7_1_VALUE       0x1C
#define EEPROM_INDUCTOR_7_2_VALUE       0x1D

#define EEPROM_CAPACITOR_1_1_VALUE      0x20
#define EEPROM_CAPACITOR_1_2_VALUE      0x21
#define EEPROM_CAPACITOR_2_1_VALUE      0x22
#define EEPROM_CAPACITOR_2_2_VALUE      0x23
#define EEPROM_CAPACITOR_3_1_VALUE      0x24
#define EEPROM_CAPACITOR_3_2_VALUE      0x25
#define EEPROM_CAPACITOR_4_1_VALUE      0x26
#define EEPROM_CAPACITOR_4_2_VALUE      0x27
#define EEPROM_CAPACITOR_5_1_VALUE      0x28
#define EEPROM_CAPACITOR_5_2_VALUE      0x29
#define EEPROM_CAPACITOR_6_1_VALUE      0x2A
#define EEPROM_CAPACITOR_6_2_VALUE      0x2B
#define EEPROM_CAPACITOR_7_1_VALUE      0x2C
#define EEPROM_CAPACITOR_7_2_VALUE      0x2D

#define EEPROM_HIPOWER_MODE_ENABLED     0x30
#define EEPROM_TANDEM_MATCH_RATIO       0x31
#define EEPROM_DISPLAY_ON_DELAY         0x32
#define EEPROM_INDICATION_MODE          0x33
#define EEPROM_FEEDER_LOSS_RATIO        0x34
#define EEPROM_RELAY_OFF_QRP_MODE       0x35

#define EEPROM_TUNED_SWR_2              0xFB
#define EEPROM_TUNED_SWR_1              0xFC
#define EEPROM_TUNED_IMPEDANCE_CTRL     0xFD
#define EEPROM_TUNED_INDUCT_CTRL        0xFE
#define EEPROM_TUNED_CAP_CTRL           0xFF