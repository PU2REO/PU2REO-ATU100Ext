// Software I2C connections
//
sbit Soft_I2C_Scl           at LATB6_bit;
sbit Soft_I2C_Sda           at LATB7_bit;
sbit Soft_I2C_Scl_Direction at TRISB6_bit;
sbit Soft_I2C_Sda_Direction at TRISB7_bit;
//
static char RS, RW, E, LED, nible;
static char led_addr, led_type;
static char shift_line;
static char oled_shift;

// function prototypes
void DisplaySetPageAddressing(char, char);
void DisplaySendData(char);
void InitDisplay(void);
void InitOLEDDisplay(void);
void SendCommand(char);
void OLEDWriteString(char, char, char*, char);
void DisplayWriteString(char, char, char*, char);
void SetDisplayOn(void);
void SetDisplayOff(void);

void Soft_I2C_Init(void);
void Soft_I2C_Start(void);
void Soft_I2C_Write(char);
void Soft_I2C_Stop(void);