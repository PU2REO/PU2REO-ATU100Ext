        
// Main.h
// David Fainitski
// ATU-100 project 2016

// Types
typedef struct
{
    int L1;
    int L2;
    int L3;
    int L4;
    int L5;
    int L6;
    int L7;
} Inductors;

typedef struct
{
    int C1;
    int C2;
    int C3;
    int C4;
    int C5;
    int C6;
    int C7;
} Capacitors;

typedef struct
{
    Capacitors      Capacitor;
    Inductors       Inductor;
    char            DisplayType;             // type
    int             RelaysDelay;             // Rel_Del
    int             MinPowerToTune;          // min_for_start
    int             MaxPowerToTune;          // max_for_start
    int             MaxInitSWRValue;         // max_swr
    char            InductorsCount;          // L_q
    char            CapacitorsCount;         // C_q
    char            LinearInductors;         // L_linear
    char            LinearCapacitors;        // C_linear
    char            RFDiodesCorrection;      // D_correction = 1,
    char            InvInductanceCtrl;       // L_invert
    char            HighPowerMode;           // P_High
    char            TandenMatchRatio;        // K_Mult
    char            BkLightDelay;            // Dysp_delay
    char            IndicationMode;          // Loss_ind
    char            FeederPwrLossRatio;      // Fid_loss
    char            RelayOffMode;            // Relay_off
    char            InductorsCtrl;           // Controls witch inductor is On/Off
    char            CapacitorsCtrl;          // Controls witch capacitor is On/Off
    char            LowHiImpedanceCtrl;      // Controls L-Network Hi/Low Impedance via S8
    char            AutoModeEnabled;         // Auto Mode
    int             DisplayBkLightDelay;     // Display backlight delay
    char            RestartNeeded;           // signal for auto-restart (WDT related)
    char            LCDPrepNeeded;           // need to prepare LCD with basic iformations
    char            IndFactor;               // Inductance multiplication factor
    char            CapFactor;               // Inductance multiplication factor
    int             LCCounter;               // Auxiliary to count impedance/capacitance.
} ControlVars;

typedef struct
{
    int             StandingWaveRatio;
    int             OutputPower;
    char            Overload;
    char            ReadyToTune;
} ActualVars;

// new variables
ControlVars     Ctrl;
ActualVars      Act;

//-----------------------------------------------------------------------------

static char step_cap = 0, step_ind = 0, Loss_ind = 0;
unsigned char work_char, work_str[7], work_str_2[7];
char  p_cnt = 0;
char  Soft_tune = 0, Track = 0;
char  bypas = 0, cap_mem = 0, ind_mem = 0, SW_mem = 0, Auto_mem = 0;
char  Test = 0;
char  LongPressButton = 1, but= 0;
char  Loss_mode = 0, Fid_loss;
char  dysp = 1;
bit   tune_btn_release;
int   P_max, swr_a;
int   Power = 0, Power_old = 10000;
int   SWR_old = 10000;
int   SWR_fixed_old = 0;
int   Auto_delta;

// Connections
//
sbit n_Tx at LATA6_bit;
sbit p_Tx at LATA7_bit;

// Capacitors output definitions
sbit Cap_10   at LATC7_bit;
sbit Cap_22   at LATC3_bit;
sbit Cap_47   at LATC6_bit;
sbit Cap_100  at LATC2_bit;
sbit Cap_220  at LATC5_bit;
sbit Cap_470  at LATC1_bit;
sbit Cap_1000 at LATC4_bit;

// Hi/Low Impedance Switch
sbit S8_SW    at LATC0_bit;

// Inductors output definitions
sbit Ind_005  at LATB3_bit;
sbit Ind_011  at LATA2_bit;
sbit Ind_022  at LATB4_bit;
sbit Ind_045  at LATA3_bit;
sbit Ind_1    at LATB5_bit;
sbit Ind_22   at LATA5_bit;
sbit Ind_45   at LATA4_bit;

// ---------------------------------------------------------------------------
// Function Prototypes
// ---------------------------------------------------------------------------

int  CalculateReversePower(void);
int  CalculateForwardPower(void);
int  ApplyRFDiodesCorrection(int);
void TuneButtonPushed(void);
void PrepareDisplay(void);
void DisplaySWR(int);
void DisplayPower(void);
void DisplayPowerAndEfficiency(int, int);
void DisplayLCValues(void);
void DisplayResetATU(void);
void LoadEEPROMData(void);
void TestModeInit(void);
void ProcessButtons(void);
void ProcessButtonsTestMode(void);
void DebounceButtons(void);
void ShowFeederLoss(void);
void ResetATU(void);
void MCU_Init(void);
void CalculateStandingWaveRadio(void);
void CalculateOutputPower(void);
void SetImpedanceSwitch(char);
void CapacitorCoarseTune();
void CapacitorSharpTune();
void InductorSharpTune();
void StartTunningProcess(void);
void CoarseTune();
void TuneATU(void);
void SetInductance(char Ind);
void SetCapacitance(char Cap);