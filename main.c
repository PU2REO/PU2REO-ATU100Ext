//   ATU-100 project
//   David Fainitski, 2016
//   PU2REO, 2024

#include    "oled_control.h"
#include    "main.h"
#include    "stdbool.h"
#include    "defines.h"

// ---------------------------------------------------------------------------

void main(void)
{
   // Check for TimeOut Status
   if(STATUS.B4 == false)
   {
      // Restart on timeout
      Ctrl.RestartNeeded = true;
   }
   
   // Initializes mC unit
   MCU_Init();
   
   // delay and clear WatchDog Timer
   Delay_ms(300);
   asm CLRWDT;
   
   // Load data from EEPROM
   LoadEEPROMData();
   
   // init I2C via SW protocol
   Soft_I2C_Init();
   
   // check display type
   if(Ctrl.DisplayType == DISPLAY_TYPE_DUAL_COLOR_LED)
   { 
     // 2-colors led reset
     LATB.B6 = 1;
     LATB.B7 = 1;
   }
   // set display backlight delay
   Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
   
   // Delay and Clear WDT
   Delay_ms(300);
   asm CLRWDT;
   
   // check for pressed buttons (Auto = B1, Bypass = B2)
   if((BUTTON_AUTO == 0) & (BUTTON_BYPASS == 0))
   {  // Test mode
      Test = 1;
      Ctrl.AutoModeEnabled = 0;
   }

   // check amount of installed inductors
   if(Ctrl.InductorsCount == 5)
   {
       Ctrl.IndFactor = 1;
   }
   else if(Ctrl.InductorsCount == 6)
   {
       Ctrl.IndFactor = 2;
   }
   else if(Ctrl.InductorsCount == 7)
   {
       Ctrl.IndFactor = 4;
   }

   // check amount of installed capacitors
   if(Ctrl.CapacitorsCount == 5)
   {
       Ctrl.CapFactor = 1;
   }
   else if(Ctrl.CapacitorsCount == 6)
   {
       Ctrl.CapFactor = 2;
   }
   else if(Ctrl.CapacitorsCount == 7)
   {
       Ctrl.CapFactor = 4;
   }
   
   // delay and Clear WDT
   Delay_ms(300);
   asm CLRWDT;
   Delay_ms(300);
   asm CLRWDT;
   Delay_ms(300);
   asm CLRWDT;
   Delay_ms(300);
   asm CLRWDT;
   Delay_ms(300);
   asm CLRWDT;
   
   // display initialization
   InitDisplay();
   
    // check for pressed buttons (Auto = B1, Bypass = B2, Tune Button = B0)
   if((BUTTON_AUTO == 0) & (BUTTON_BYPASS == 0) & (BUTTON_TUNE == 0))
   { 
      // Fast Test mode (loop)
      if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
      {  
         DisplayWriteString(0, 12, "FAST TEST", 9); // 128*64
      }
      else if (Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
      {
         DisplayWriteString(0, 3, "FAST TEST", 9); // 1602 | 128*32
      }
      
      // turn on all capacitors
      SetCapacitance(OUT_ALL_ON);
      
      // chec for inverse inductance control
      if(Ctrl.InvInductanceCtrl == 0)
      {      
         // turn on all inductors
         SetInductance(OUT_ALL_ON);
      }
      else 
      {
         // turn off all inductors
         SetInductance(OUT_ALL_OFF);
      }
      
      // turn on ???
      SetImpedanceSwitch(1);
      asm CLRWDT;
      
      // wait forever
      while(1)
      {
         Delay_ms(500);
         asm CLRWDT;
      }
   }
   
   // Clear WDT
   asm CLRWDT;
   
   // Check TUNE Button
   if(Button(&PORTB, 0, 100, 0))  
   { 
      //  Check display type
      if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
      { 
         // Show feeder loss input for OLED displays
         DisplayWriteString(0, 6, "Fider Loss", 10); // 128*64
         DisplayWriteString(2, 6, "input", 5);
         DisplayWriteString(4, 6+3*12, "dB", 2);
      }
      else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
      {
         // Show Fider loss input for LCD displays
         DisplayWriteString(0, 0, "Fider Loss input", 16); // 1602 | 128*32
         DisplayWriteString(1, 3, "dB", 2);
      }
      
      // get feeder loss ratio
      Ctrl.FeederPwrLossRatio = Bcd2Dec(EEPROM_Read(EEPROM_FEEDER_LOSS_RATIO));
      
      // Show feeder loss
      ShowFeederLoss();
      //
      while(1)
      {
         // check bypass button
         if(Button(&PORTB, 2, 50, 0))
         {   // BYP button
            if(Ctrl.FeederPwrLossRatio < 99)
            {
               Ctrl.FeederPwrLossRatio++;
               ShowFeederLoss();
               EEPROM_Write(EEPROM_FEEDER_LOSS_RATIO, Dec2Bcd(Ctrl.FeederPwrLossRatio));
            }
            while(Button(&PORTB, 2, 50, 0)) asm CLRWDT;
         }
         
         // check auto button
         if(Button(&PORTB, 1, 50, 0))
         {   // AUTO button
            if(Ctrl.FeederPwrLossRatio > 0)
            {
               Ctrl.FeederPwrLossRatio--;
               ShowFeederLoss();
               EEPROM_Write(EEPROM_FEEDER_LOSS_RATIO, Dec2Bcd(Ctrl.FeederPwrLossRatio));
            }
            while(Button(&PORTB, 1, 50, 0)) asm CLRWDT;
         }
         asm CLRWDT;
      }  // while
   } //  Fider loss input
   
   // check for Test mode inactive
   if(Test == 0)
   {
      Ctrl.CapacitorsCtrl     =  EEPROM_Read(EEPROM_TUNED_CAP_CTRL);
      Ctrl.InductorsCtrl      =  EEPROM_Read(EEPROM_TUNED_INDUCT_CTRL);
      Ctrl.LowHiImpedanceCtrl =  EEPROM_Read(EEPROM_TUNED_IMPEDANCE_CTRL);
      swr_a                   =  EEPROM_Read(EEPROM_TUNED_SWR_1)*256 + EEPROM_Read(EEPROM_TUNED_SWR_2);
      SetInductance(Ctrl.InductorsCtrl);
      SetCapacitance(Ctrl.CapacitorsCtrl);
      SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
      
      if(Ctrl.RestartNeeded == true)
      {
         Ctrl.LCDPrepNeeded = true;
      }
      PrepareDisplay();
   }
   else 
   {
      TestModeInit();
   }
   
   DisplayLCValues();
   tune_btn_release = 1;
   
   while(true)
   {
      asm CLRWDT;
      DisplayPower();

      // check Test mode
      if(Test == 0)
      {
          ProcessButtons();
      }
      else
      {
          ProcessButtonsTestMode();
      }

      // check display backlight counter
      if(Ctrl.DisplayBkLightDelay != 0)
      {
          Ctrl.DisplayBkLightDelay--;
      }
      else if((Test == 0) & (Ctrl.BkLightDelay != 0) & (dysp == 1))
      {
          if(Ctrl.RelayOffMode)
          {
             SetInductance(OUT_ALL_OFF);
             SetCapacitance(OUT_ALL_OFF);
             SetImpedanceSwitch(OUT_ALL_OFF);
          }
          dysp = 0;
          SetDisplayOff();
      }
   }
}

// ---------------------------------------------------------------------------

 void ProcessButtonsTestMode(void)
 {
    // check Tune Button
    if(Button(&PORTB, 0, 50, 0))
    {    // Tune btn
        Delay_ms(250);
        asm CLRWDT;
        if(BUTTON_TUNE == 1)
        { // short press button
           if(Ctrl.LowHiImpedanceCtrl == 0)
           {
              Ctrl.LowHiImpedanceCtrl = 1;
           }
           else 
           {
              Ctrl.LowHiImpedanceCtrl = 0;
           }
           SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
           DisplayLCValues();
        }
        else 
        {   
           // long press button
           if(LongPressButton == 1)
           {
              LongPressButton = 0;
           }
           else
           {
              LongPressButton = 1;
           }

           if(LongPressButton == 1)
           {
              // check display type
              if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))   // 128*64 OLED
              {
                 DisplayWriteString(0, 16+12*8, "l", 1);
              }
              else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)              // 1602 LCD & 128*32 OLED
              {
                 DisplayWriteString(0, 8, "l", 1);
              }
           }
           else 
           {
              // check display type
              if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))   // 128*64 OLED
              {
                 DisplayWriteString(0, 16+12*8, "c", 1);
              }
              else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)              // 1602 LCD & 128*32 OLED
              {
                 DisplayWriteString(0, 8, "c", 1);
              }
           }
        }
        while(Button(&PORTB, 0, 50, 0))
        {
            DisplayPower();
            asm CLRWDT;
        }
     }  // END Tune btn
     
     // check Bypass button
     if(Button(&PORTB, 2, 50, 0))
     {   // BYP button
        asm CLRWDT;
        while(BUTTON_BYPASS == 0)
        {
           if(LongPressButton & ( Ctrl.InductorsCtrl < ((32 * Ctrl.IndFactor)-1)))
           {
              Ctrl.InductorsCtrl++;
              SetInductance(Ctrl.InductorsCtrl);
           }
           else if(!LongPressButton & (Ctrl.CapacitorsCtrl < ((32 * Ctrl.IndFactor)-1)))
           {
              Ctrl.CapacitorsCtrl++;
              SetCapacitance(Ctrl.CapacitorsCtrl);
           }
           DisplayLCValues();
           DisplayPower();
           Delay_ms(30);
           asm CLRWDT;
        }
     }   // end of BYP button

     // Check AutoButton
     if(Button(&PORTB, 1, 50, 0) & (bypas == 0))
     {   // Auto button
        asm CLRWDT;
        while(BUTTON_AUTO == 0)
        {
           if(LongPressButton & (Ctrl.InductorsCtrl > 0))
           {
              Ctrl.InductorsCtrl--;
              SetInductance(Ctrl.InductorsCtrl);
           }
           else if(!LongPressButton & (Ctrl.CapacitorsCtrl > 0))
           {
              Ctrl.CapacitorsCtrl--;
              SetCapacitance(Ctrl.CapacitorsCtrl);
           }
           DisplayLCValues();
           DisplayPower();
           Delay_ms(30);
           asm CLRWDT;
        }
     }
     return;
}

// ---------------------------------------------------------------------------

void ProcessButtons(void)
{
    if((tune_btn_release == 0) & Button(&PORTB, 0, 50, 1))
    {
       tune_btn_release = 1;
    }
    
    if((tune_btn_release == 0) & Button(&PORTB, 0, 50, 0))
    {
       Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
    }
    
    if((Button(&PORTB, 0, 50, 0) & tune_btn_release) | Soft_tune)
    {
        if(dysp == 0) { // dysplay ON
           if(Ctrl.RelayOffMode ==1 )
           { 
               SetInductance(Ctrl.InductorsCtrl);
               SetCapacitance(Ctrl.CapacitorsCtrl);
               SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
           }
           dysp = 1; 
           SetDisplayOn();
           Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
           return;
        }
        Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
        Delay_ms(250);
        asm CLRWDT;
        
        if((Soft_tune == 0) & (BUTTON_TUNE == 1))
        { 
           // short press button
           DisplayResetATU();
           bypas = 0;
        }
        else 
        {  // long press button
           p_Tx = 1;         //
           n_Tx = 0;         // TX request
           Delay_ms(250);    //
           TuneButtonPushed();
           bypas = 0;
           Soft_tune = 0;
           tune_btn_release = 0;
        }
     }
     
     // check Bypass Button
     if(Button(&PORTB, 2, 50, 0))
     {   // BYP button
        if(dysp == 0)
        { // dysplay ON
           if(Ctrl.RelayOffMode == 1)
           { 
               SetInductance(Ctrl.InductorsCtrl);
               SetCapacitance(Ctrl.CapacitorsCtrl);
               SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
           }
           dysp = 1;
           SetDisplayOn();
           Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
           return;
        }
        Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
        asm CLRWDT;
        if(bypas == 0) 
        {
           bypas = 1;
           cap_mem = Ctrl.CapacitorsCtrl;
           ind_mem = Ctrl.InductorsCtrl;
           SW_mem = Ctrl.LowHiImpedanceCtrl;
           Ctrl.CapacitorsCtrl = 0;
           if(Ctrl.InvInductanceCtrl)
           {
               Ctrl.InductorsCtrl = OUT_ALL_ON;
           }
           else
           {
               Ctrl.InductorsCtrl = 0;
           }
           Ctrl.LowHiImpedanceCtrl = 1;
           SetInductance(Ctrl.InductorsCtrl);
           SetCapacitance(Ctrl.CapacitorsCtrl);
           SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
           if(Loss_mode==0) 
           {
               DisplayLCValues();
           }
           Auto_mem = Ctrl.AutoModeEnabled;
           Ctrl.AutoModeEnabled = 0;
        }
        else 
        {
           bypas = 0;
           Ctrl.CapacitorsCtrl = cap_mem;
           Ctrl.InductorsCtrl = ind_mem;
           Ctrl.LowHiImpedanceCtrl = SW_mem;
           SetCapacitance(Ctrl.CapacitorsCtrl);
           SetInductance(Ctrl.InductorsCtrl);
           SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
           if(Loss_mode == 0)
           {
               DisplayLCValues();
           }
           Ctrl.AutoModeEnabled = Auto_mem;
        }
        
        // check display type
        if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
        {   // 128*64 OLED
            if(Ctrl.AutoModeEnabled & !bypas)
            {
                // Show Auto mode
                DisplayWriteString(0, 16+8*12, ".", 1);
            }
            else if(!Ctrl.AutoModeEnabled & bypas)
            {
                // Show Bypass mode
                DisplayWriteString(0, 16+8*12, "_", 1);
            }
            else
            {
                // Shows nothing
                DisplayWriteString(0, 16+8*12, " ", 1);
            }
        }
        else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
        {   //  1602 LCD or 128*32 OLED
            if(Ctrl.AutoModeEnabled & !bypas)
            {
                // Show Auto mode
                DisplayWriteString(0, 8, ".", 1);
            }
            else if(!Ctrl.AutoModeEnabled & bypas)
            {
                // Show Bypass mode
                DisplayWriteString(0, 8, "_", 1);
            }
            else
            {
                // Shows nothing
                DisplayWriteString(0, 8, " ", 1);
            }
        }
        
        asm CLRWDT;
        
        while(Button(&PORTB, 2, 50, 0)) 
        {
           DisplayPower(); 
           asm CLRWDT;   
        }
     }
     
     // check Auto Button
     if(Button(&PORTB, 1, 50, 0) & (bypas == 0))
     {   // Auto button
          if(dysp == 0)
          { // dysplay ON
             if(Ctrl.RelayOffMode == 1)
             {
                 SetInductance(Ctrl.InductorsCtrl);
                 SetCapacitance(Ctrl.CapacitorsCtrl);
                 SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
             }
             dysp = 1;
             SetDisplayOn();
             Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
             return;
          }
          Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
          asm CLRWDT;

          if(Ctrl.AutoModeEnabled == 0)
          {
              Ctrl.AutoModeEnabled = 1;
          }
          else
          {
              Ctrl.AutoModeEnabled = 0;
          }

          // save Auto mode
          EEPROM_Write(EEPROM_AUTO_MODE_ENABLED, Ctrl.AutoModeEnabled);

          if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
          {      // 128*64 OLED
              if(Ctrl.AutoModeEnabled & !bypas)
              {
                  // Show Auto mode
                  DisplayWriteString(0, 16+8*12, ".", 1);
              }
              else if(!Ctrl.AutoModeEnabled & bypas)
              {
                  // Show Bypass mode
                  DisplayWriteString(0, 16+8*12, "_", 1);
              }
              else
              {
                  // Shows nothing
                  DisplayWriteString(0, 16+8*12, " ", 1);
              }
          }
          else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
          { //  1602 LCD  or 128*32 OLED
              if(Ctrl.AutoModeEnabled & !bypas)
              {
                  // Show Auto mode
                  DisplayWriteString(0, 8, ".", 1);
              }
              else if(!Ctrl.AutoModeEnabled & bypas)
              {
                  // Show Bypass mode
                  DisplayWriteString(0, 8, "_", 1);
              }
              else
              {
                  // Shows nothing
                  DisplayWriteString(0, 8, " ", 1);
              }
          }
          asm CLRWDT;
          while(Button(&PORTB, 1, 50, 0))
          {
              DisplayPower();
              asm CLRWDT;
          }
     }
     return;
}

// ---------------------------------------------------------------------------

void MCU_Init(void)
{
  asm CLRWDT;
  WDTCON.B5 = 0;
  WDTCON.B4 = 1;
  WDTCON.B3 = 0;          // 1 sec WDT
  WDTCON.B2 = 1;
  WDTCON.B1 = 0;
  asm CLRWDT;

  ANSELA = 0;
  ANSA0_bit = 1;          // analog inputs
  ANSA1_bit = 1;
  ANSELB = 0;             // all as digital

  C1ON_bit = 0;           // Disable comparators
  C2ON_bit = 0;

  OSCCON = 0b01111000;    // 16 MHz oscillator

  // ports initialization
  PORTA = 0;
  PORTB = 0;
  PORTC = 0;
  LATA = 0b01000000;      // PORT6 /Tx_req to 1
  LATB = 0;
  LATC = 0;
  TRISA = 0b00000011;
  TRISB = 0b00000111;
  TRISC = 0b00000000;     //
  
  // ADC Initialization
  ADC_Init();
  ADCON1.B0 = 1;          // ADC with the internal reference
  ADCON1.B1 = 1;

  // Pull up registers
  OPTION_REG.B7 = 0;      // Enable Pull-up resistors
  WPUB.B0 = 1;            // PORTB0 Pull-up resistor
  WPUB.B1 = 1;            // PORTB1 Pull-up resistor
  WPUB.B2 = 1;            // PORTB2 Pull-up resistor
  
  //interrupt setting
  GIE_bit = 0;
  
  // initial values
  Ctrl.IndFactor = 1;
  Ctrl.CapFactor = 1;
}

// ---------------------------------------------------------------------------

void DisplayResetATU()
{
    ResetATU();
    Ctrl.LowHiImpedanceCtrl = 1;
    SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
    EEPROM_Write(EEPROM_TUNED_CAP_CTRL, 0);
    EEPROM_Write(EEPROM_TUNED_INDUCT_CTRL, 0);
    EEPROM_Write(EEPROM_TUNED_IMPEDANCE_CTRL, 1);
    EEPROM_Write(EEPROM_TUNED_SWR_1, 0);
    EEPROM_Write(EEPROM_TUNED_SWR_2, 0);
    DisplayLCValues();
    Loss_mode = 0;
    p_Tx = 0;
    n_Tx = 1;
    Act.StandingWaveRatio = 0;
    Act.OutputPower = 0;
    SWR_fixed_old = 0;

    if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
    {    // 128*64 OLED
       DisplayWriteString(2, 16, "RESET   ", 8);
       asm CLRWDT;
       Delay_ms(600);
       DisplayWriteString(2, 16, "SWR=0.00", 8);
       asm CLRWDT;
    }
    else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
    {// 1602 LCD & 128*32 OLED
       DisplayWriteString(1, 0, "RESET   ", 8);
       asm CLRWDT;
       Delay_ms(600);
       DisplayWriteString(1, 0, "SWR=0.00", 8);
       asm CLRWDT;
    }
    else
    {
       LATB.B6 = 1;
       LATB.B7 = 1;
    }
    SWR_old = 10000;
    Power_old = 10000;
    DisplayPower();
    return;
}

// ---------------------------------------------------------------------------

void TuneButtonPushed()
{
   asm CLRWDT;
   if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
   {   // 128*64 OLED
      DisplayWriteString(2, 16+12*4, "TUNE", 4);
   }
   else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
   {   // 1602 LCD & 128*32 OLED
      DisplayWriteString(1, 4, "TUNE", 4);
   }
   else 
   {
      LATB.B6 = 1;
      LATB.B7 = 1;
   }
   
   // start tunning
   StartTunningProcess();
   
   if(Ctrl.DisplayType == DISPLAY_TYPE_DUAL_COLOR_LED)
   {    // real-time 2-colors led work
      if(Act.StandingWaveRatio <= LIMIT_OPERATING_SWR)
      { 
          LATB.B6 = 0;
          LATB.B7 = 1;
      } // Green
      else if(Act.StandingWaveRatio <= MAX_OPERATING_SWR)
      {
          LATB.B6 = 0;
          LATB.B7 = 0;
      } // Orange
      else 
      { 
          LATB.B6 = 1;
          LATB.B7 = 0;
      }  // Red
   }
   else if((Loss_mode==0) | (Ctrl.IndicationMode == 0))
   {
       DisplayLCValues();
   }
   
   // store current tuned values to EEPROM
   EEPROM_Write(EEPROM_TUNED_CAP_CTRL, Ctrl.CapacitorsCtrl);
   EEPROM_Write(EEPROM_TUNED_INDUCT_CTRL, Ctrl.InductorsCtrl);
   EEPROM_Write(EEPROM_TUNED_IMPEDANCE_CTRL, Ctrl.LowHiImpedanceCtrl);
   EEPROM_Write(EEPROM_TUNED_SWR_1, (swr_a / 256));
   EEPROM_Write(EEPROM_TUNED_SWR_2, (swr_a % 256));

   SWR_old = 10000;
   Power_old = 10000;
   DisplayPower();
   SWR_fixed_old = Act.StandingWaveRatio;
   p_Tx = 0;
   n_Tx = 1;
   asm CLRWDT;
   return;
}

// ---------------------------------------------------------------------------

 void PrepareDisplay()
 {
   asm CLRWDT;
   if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
   {   // 128*64 OLED
      if(Ctrl.LCDPrepNeeded == false)
      {
         DisplayWriteString(0, 22, "ATU-100", 7);
         DisplayWriteString(2, 6,  "EXT board", 9);
         DisplayWriteString(4, 10, "by PU2REO", 9);
         DisplayWriteString(6, 4,  FW_VERSION, 10);
         asm CLRWDT;
         Delay_ms(600);
         asm CLRWDT;
         Delay_ms(500);
         asm CLRWDT;
         DisplayWriteString(0, 16, "        ", 8);
         DisplayWriteString(2, 4,  "          ", 10);
         DisplayWriteString(4, 10, "         ", 9);
         DisplayWriteString(6, 4,  "          ", 10);
      }
      Delay_ms(150);
      if(Ctrl.HighPowerMode==1)
      {
         DisplayWriteString(0, 16, "PWR=  0W", 8);
      }
      else
      {
         DisplayWriteString(0, 16, "PWR=0.0W", 8);
      }
      DisplayWriteString(2, 16, "SWR=0.00", 8);
      
      if(Ctrl.AutoModeEnabled)
      {
         DisplayWriteString(0, 16+8*12, ".", 1);
      }
   }
   else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
   {   // 1602 LCD & 128*32 OLED
      if(Ctrl.LCDPrepNeeded == false)
      {
         DisplayWriteString(0, 4, "ATU-100", 7);
         DisplayWriteString(1, 3, "EXT board", 9);
         asm CLRWDT;
         Delay_ms(700);
         asm CLRWDT;
         Delay_ms(500);
         asm CLRWDT;
         DisplayWriteString(0, 4, "by PU2REO", 9);
         DisplayWriteString(1, 3, FW_VERSION, 10);
         asm CLRWDT;
         Delay_ms(600);
         asm CLRWDT;
         Delay_ms(500);
         asm CLRWDT;
         DisplayWriteString(0, 4, "         ", 9);
         DisplayWriteString(1, 3, "          ", 10);
      }
      Delay_ms(150);
      
      if(Ctrl.HighPowerMode == 1)
      {
         DisplayWriteString(0, 0, "PWR=  0W", 8);
      }
      else
      {
          DisplayWriteString(0, 0, "PWR=0.0W", 8);
      }
      DisplayWriteString(1, 0, "SWR=0.00", 8);
      if(Ctrl.AutoModeEnabled)
      {
          DisplayWriteString(0, 8, ".", 1);
      }
   }
   asm CLRWDT;
   DisplayLCValues();
   return;
}

// ---------------------------------------------------------------------------

void DisplaySWR(int swr)
{
   asm CLRWDT;
   if(swr != SWR_old)
   {
      SWR_old = swr;
      if(SWR == 0) 
      {  // Low power
         if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
            DisplayWriteString(2, 16+4*12, "0.00", 4);   // 128*64 OLED
         else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
            DisplayWriteString(1, 4, "0.00", 4);  // 1602  & 128*32 OLED
         else if(Ctrl.DisplayType == DISPLAY_TYPE_DUAL_COLOR_LED)
         {    // real-time 2-colors led work
            LATB.B6 = 1;
            LATB.B7 = 1;
         }
         SWR_old = 0;
      }
      else 
      {
         IntToStr(swr, work_str);
         work_str_2[0] = work_str[3];
         work_str_2[1] = '.';
         work_str_2[2] = work_str[4];
         work_str_2[3] = work_str[5];
         if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
            DisplayWriteString(2, 16+4*12, work_str_2, 4);  // 128*64 OLED
         else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
            DisplayWriteString(1, 4, work_str_2, 4);       // 1602  & 128*32
         else if(Ctrl.DisplayType == DISPLAY_TYPE_DUAL_COLOR_LED)
         {    
            // real-time 2-colors led work
            if(swr <= 150) 
            { 
               LATB.B6 = 0; 
               LATB.B7 = 1; 
            } // Green
            else if(swr <= 250) 
            {
               PORTB.B6 = 0; 
               PORTB.B7 = 0;
            } // Orange
            else 
            { 
               PORTB.B6 = 1; 
               PORTB.B7 = 0; 
            }  // Red
         }
      }
   }
   asm CLRWDT;
   return;
}

// ---------------------------------------------------------------------------

void DebounceButtons()
{
   // debounce buttons
   if((Button(&PORTB, 0, 25, 0)) | (Button(&PORTB, 1, 25, 0)) | (Button(&PORTB, 2, 25, 0))) 
   {
      but = 1;
   }
   return;
}

// ---------------------------------------------------------------------------

void DisplayPowerAndEfficiency(int Power, int SWR) {
   int p_ant, eff;
   float a, b;
   a = 100;
   asm CLRWDT;
   //
   if(Test==0 & Ctrl.IndicationMode==1 & Power>=10) {
        if(Loss_mode == 0)
        {   // prepare
           if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
           {   // 128*64 OLED
              if(Ctrl.HighPowerMode == 1 ) DisplayWriteString(4, 16, "ANT=  0W", 8);
              else DisplayWriteString(4, 16, "ANT=0.0W", 8);
              DisplayWriteString(6, 16, "EFF=  0%", 8);
           }
           else if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32_INV))
           {   // 128*32 OLED
              if(Ctrl.HighPowerMode == 1) DisplayWriteString(0, 9, "ANT=  0W", 8);
              else DisplayWriteString(0, 9, "ANT=0.0W", 8);
              DisplayWriteString(1, 9, "EFF=  0%", 8);
           }
           else if(Ctrl.DisplayType == DISPLAY_TYPE_1602_LCD)
           {   // 1602 LCD
              if(Ctrl.HighPowerMode==1) DisplayWriteString(0, 9, "AN=  0W", 7);
              else DisplayWriteString(0, 9, "AN=0.0W", 7);
              DisplayWriteString(1, 9, "EFF= 0%", 7);
           }
        }
        Loss_mode = 1;
     }
     else 
     {
        if(Loss_mode == 1) DisplayLCValues();
        Loss_mode = 0;
     }
   asm CLRWDT;
   if(Power != Power_old) 
   {
       Power_old = Power;
       //
       if(Ctrl.HighPowerMode==0) {
          if(Power >= 100) 
          {   // > 10 W
             Power += 5;  // rounding to 1 W
             IntToStr(Power, work_str);
             work_str_2[0] = work_str[2];
             work_str_2[1] = work_str[3];
             work_str_2[2] = work_str[4];
             work_str_2[3] = 'W';
          }
             else {
             IntToStr(Power, work_str);
             if(work_str[4] != ' ') work_str_2[0] = work_str[4]; else work_str_2[0] = '0';
             work_str_2[1] = '.';
             if(work_str[5] != ' ') work_str_2[2] = work_str[5]; else work_str_2[2] = '0';
             work_str_2[3] = 'W';
          }
       }
       else
       {
          // High Power
          if(Power < 999)
          {   // 0 - 999 Watt
             IntToStr(Power, work_str);
             work_str_2[0] = work_str[3];
             work_str_2[1] = work_str[4];
             work_str_2[2] = work_str[5];
             work_str_2[3] = 'W';
          }
          else
          {
             if(Power > 9999)
             {
                Power = 9999;
                Act.Overload = true;
             }
             IntToStr(Power, work_str);
             work_str_2[0] = work_str[2];
             work_str_2[1] = work_str[3];
             work_str_2[2] = work_str[4];
             work_str_2[3] = work_str[5];
          }
       }
       if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
       {
           DisplayWriteString(0, 16+4*12, work_str_2, 4);  // 128*64 OLED
       }
       else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
       {
           DisplayWriteString(0, 4, work_str_2, 4);  // 1602  & 128*32
       }
       
       //
       asm CLRWDT;

       //  Loss indication
       if(Loss_mode == 1)
       {
          if((Ctrl.InductorsCtrl == 0) & (Ctrl.CapacitorsCtrl == 0))
          {
              swr_a = SWR;
          }
          a = 1.0 / ((swr_a/100.0 + 100.0/swr_a) * Ctrl.FeederPwrLossRatio/10.0 * 0.115 + 1.0); // Fider loss
          b = 4.0 / (2.0 + SWR/100.0 + 100.0/SWR);    // SWR loss
          if(a>=1.0) a = 1.0;
          if(b>=1.0) b = 1.0;
          p_ant = Power * a * b;
          eff = a * b * 100;
          if(eff>=100) eff = 99;

          // Low Power reading
          if(Ctrl.HighPowerMode == 0)
          {
             if(p_ant >= 100) {   // > 10 W
                p_ant += 5;  // rounding to 1 W
                IntToStr(p_ant, work_str);
                work_str_2[0] = work_str[2];
                work_str_2[1] = work_str[3];
                work_str_2[2] = work_str[4];
                work_str_2[3] = 'W';
             }
             else {
                IntToStr(p_ant, work_str);
                if(work_str[4] != ' ') work_str_2[0] = work_str[4]; else work_str_2[0] = '0';
                work_str_2[1] = '.';
                if(work_str[5] != ' ') work_str_2[2] = work_str[5]; else work_str_2[2] = '0';
                work_str_2[3] = 'W';
             }
          }
          else
          {  // High Power Reading
             if(p_ant < 999){   // 0 - 1500 Watts
                IntToStr(p_ant, work_str);
                work_str_2[0] = work_str[3];
                work_str_2[1] = work_str[4];
                work_str_2[2] = work_str[5];
                work_str_2[3] = 'W';
             }
             else
             {
                IntToStr(p_ant, work_str);
                work_str_2[0] = work_str[2];
                work_str_2[1] = work_str[3];
                work_str_2[2] = work_str[4];
                work_str_2[3] = work_str[5];
             }
          }
          if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
          {
              DisplayWriteString(4, 16+4*12, work_str_2, 4);  // 128*64 OLED
          }
          else if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32_INV))
          {
              DisplayWriteString(0, 13, work_str_2, 4);  // 128*32
          }
          else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
          {
              DisplayWriteString(0, 12, work_str_2, 4);  // 1602
          }
          //
          IntToStr(eff, work_str);
          work_str_2[0] = work_str[4];
          work_str_2[1] = work_str[5];
          if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
          {
              DisplayWriteString(6, 16+5*12, work_str_2, 2);
          }
          else if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32_INV))
          {
              DisplayWriteString(1, 14, work_str_2, 2);
          }
          else if(Ctrl.DisplayType == DISPLAY_TYPE_1602_LCD)
          {
              DisplayWriteString(1, 13, work_str_2, 2);
          }
       }
   }
   asm CLRWDT;
   return;
}

// ---------------------------------------------------------------------------

void DisplayPower()
{
    int p = 0;
    char peak_cnt;
    int delta = Auto_delta - 100;
    char cnt;
    int SWR_fixed;
    Act.OutputPower = 0;
    asm CLRWDT;

    //
    if((dysp == 0) & (Ctrl.RelayOffMode == 1))
    {
      CalculateOutputPower();
      if(Act.OutputPower >= 10)
      {
         SetInductance(Ctrl.InductorsCtrl);
         SetCapacitance(Ctrl.CapacitorsCtrl);
         SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
         //
         dysp = 1;
         SetDisplayOn();           // dysplay ON
         Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
      }
      return;
    }
    // peak detector
    cnt = 120;
    for(peak_cnt = 0; peak_cnt < cnt; peak_cnt++)
    {
      if((BUTTON_AUTO == 0) | (BUTTON_BYPASS == 0) | ((BUTTON_TUNE == 0) & tune_btn_release))
      {
         DebounceButtons();
         if(but == 1)
         {
            but = 0;
            return;
         }
      }

      CalculateOutputPower();

      if(Act.OutputPower > p)
      {
          p = Act.OutputPower;
          SWR_fixed = Act.StandingWaveRatio;
      }
      Delay_ms(3);
    }

    asm CLRWDT;

    if(p >= 100)
    {
      // round to 1 W if more then 100 W
      p = (p + 5) / 10;
      p*= 10;
    }
    Power = p;
    if(Power < 10) SWR_fixed = 0;
    DisplaySWR(SWR_fixed);
    if(Power >= 10)
    {
      if(Ctrl.BkLightDelay > 0)
      {
         Ctrl.DisplayBkLightDelay = Ctrl.BkLightDelay * DISPLAY_BKLIGHT_FACTOR;
      }

      if(dysp == 0)
      {
         if(Ctrl.RelayOffMode == 1)
         {
             SetInductance(Ctrl.InductorsCtrl);
             SetCapacitance(Ctrl.CapacitorsCtrl);
             SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
         }
         dysp = 1;
         SetDisplayOn();
      }
    }
    //
    if(Ctrl.AutoModeEnabled & SWR_fixed>=Auto_delta & ((SWR_fixed>SWR_fixed_old & (SWR_fixed-SWR_fixed_old)>delta) | (SWR_fixed<SWR_fixed_old & (SWR_fixed_old-SWR_fixed)>delta) | SWR_fixed_old==999))
      Soft_tune = 1;

    //
    if((BUTTON_AUTO == 0) | (BUTTON_BYPASS == 0) | ((BUTTON_TUNE == 0) & tune_btn_release))
    {
      // Fast return if button pressed
      DebounceButtons();
      if(but==1)
      {
         but = 0;
         return;
      }
    }
    //
    DisplayPowerAndEfficiency(Power, SWR_fixed);
    //
    if((BUTTON_AUTO == 0) | (BUTTON_BYPASS == 0) | ((BUTTON_TUNE == 0)  & tune_btn_release))
    {
      DebounceButtons();
      if(but == 1)
      {
         but = 0;
         return;
      }
    }

    asm CLRWDT;

    // manage Power overload
    if(Act.Overload == true)
    {
        if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
        {                  // 128*64 OLED
           DisplayWriteString(2, 16, "        ", 8);
           Delay_ms(100);
           DisplayWriteString(2, 16, "OVERLOAD", 8);
           Delay_ms(500);
           asm CLRWDT;
           DisplayWriteString(2, 16, "        ", 8);
           Delay_ms(300);
           asm CLRWDT;
           DisplayWriteString(2, 16, "OVERLOAD", 8);
           Delay_ms(500);
           asm CLRWDT;
           DisplayWriteString(2, 16, "        ", 8);
           Delay_ms(300);
           asm CLRWDT;
           DisplayWriteString(2, 16, "OVERLOAD", 8);
           Delay_ms(500);
           asm CLRWDT;
           DisplayWriteString(2, 16, "        ", 8);
           Delay_ms(100);
           DisplayWriteString(2, 16, "SWR=    ", 8);
        }
        else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
        {        // 1602  & 128*32
           DisplayWriteString(1, 0, "        ", 8);
           Delay_ms(100);
           DisplayWriteString(1, 0, "OVERLOAD", 8);
           Delay_ms(500);
           asm CLRWDT;
           DisplayWriteString(1, 0, "        ", 8);
           Delay_ms(300);
           asm CLRWDT;
           DisplayWriteString(1, 0, "OVERLOAD", 8);
           Delay_ms(500);
           asm CLRWDT;
           DisplayWriteString(1, 0, "        ", 8);
           Delay_ms(300);
           asm CLRWDT;
           DisplayWriteString(1, 0, "OVERLOAD", 8);
           Delay_ms(500);
           asm CLRWDT;
           DisplayWriteString(1, 0, "        ", 8);
           Delay_ms(100);
           DisplayWriteString(1, 0, "SWR=    ", 8);
        }
        
        asm CLRWDT;
        SWR_old = 10000;
        DisplaySWR(SWR_fixed);
    }
    return;
}

// ---------------------------------------------------------------------------

void DisplayLCValues()
{
    char column;
    asm CLRWDT;

    // initialize L&C counter
    Ctrl.LCCounter = 0;
    
    // sums amount of active indutance
    if(Ctrl.InductorsCtrl.B0) Ctrl.LCCounter += Ctrl.Inductor.L1;
    if(Ctrl.InductorsCtrl.B1) Ctrl.LCCounter += Ctrl.Inductor.L2;
    if(Ctrl.InductorsCtrl.B2) Ctrl.LCCounter += Ctrl.Inductor.L3;
    if(Ctrl.InductorsCtrl.B3) Ctrl.LCCounter += Ctrl.Inductor.L4;
    if(Ctrl.InductorsCtrl.B4) Ctrl.LCCounter += Ctrl.Inductor.L5;
    if(Ctrl.InductorsCtrl.B5) Ctrl.LCCounter += Ctrl.Inductor.L6;
    if(Ctrl.InductorsCtrl.B6) Ctrl.LCCounter += Ctrl.Inductor.L7;

    if(Ctrl.LCCounter > 9999)
    {   // more than 9999 nH
        Ctrl.LCCounter += 50; // round
        IntToStr(Ctrl.LCCounter, work_str);
        work_str_2[0] = work_str[1];
        work_str_2[1] = work_str[2];
        work_str_2[2] = '.';
        work_str_2[3] = work_str[3];
    }
    else
    {
        IntToStr(Ctrl.LCCounter, work_str);
        if(work_str[2] != ' ') work_str_2[0] = work_str[2]; else work_str_2[0] = '0';
        work_str_2[1] = '.';
        if(work_str[3] != ' ') work_str_2[2] = work_str[3]; else work_str_2[2] = '0';
        if(work_str[4] != ' ') work_str_2[3] = work_str[4]; else work_str_2[3] = '0';
    }

    if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
    {   // 128*64 OLED
        if(Ctrl.LowHiImpedanceCtrl == 1) column = 4; else column = 6;
        DisplayWriteString(column, 16, "L=", 2);
        DisplayWriteString(column, 16+6*12, "uH", 2);
        DisplayWriteString(column, 16+2*12, work_str_2, 4);
    }
    else if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32_INV))
    {   // 128*32 OLED
        if(Ctrl.LowHiImpedanceCtrl == 1) column = 0; else column = 1;
        DisplayWriteString(column, 9, "L=", 2);
        DisplayWriteString(column, 15, "uH", 2);
        DisplayWriteString(column, 11, work_str_2, 4);
    }
    else if(Ctrl.DisplayType == DISPLAY_TYPE_1602_LCD)
    {   //  1602 LCD
        if(Ctrl.LowHiImpedanceCtrl == 1) column = 0; else column = 1;
        DisplayWriteString(column, 9, "L=", 2);
        DisplayWriteString(column, 15, "u", 1);
        DisplayWriteString(column, 11, work_str_2, 4);
    }

    asm CLRWDT;

    // initialize L&C counter
    Ctrl.LCCounter = 0;

    // sums amount of active capacitance
    if(Ctrl.CapacitorsCtrl.B0) Ctrl.LCCounter += Ctrl.Capacitor.C1;
    if(Ctrl.CapacitorsCtrl.B1) Ctrl.LCCounter += Ctrl.Capacitor.C2;
    if(Ctrl.CapacitorsCtrl.B2) Ctrl.LCCounter += Ctrl.Capacitor.C3;
    if(Ctrl.CapacitorsCtrl.B3) Ctrl.LCCounter += Ctrl.Capacitor.C4;
    if(Ctrl.CapacitorsCtrl.B4) Ctrl.LCCounter += Ctrl.Capacitor.C5;
    if(Ctrl.CapacitorsCtrl.B5) Ctrl.LCCounter += Ctrl.Capacitor.C6;
    if(Ctrl.CapacitorsCtrl.B6) Ctrl.LCCounter += Ctrl.Capacitor.C7;
    IntToStr(Ctrl.LCCounter, work_str);
    work_str_2[0] = work_str[2];
    work_str_2[1] = work_str[3];
    work_str_2[2] = work_str[4];
    work_str_2[3] = work_str[5];
    //
    if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
    {   // 128*64 OLED
        if(Ctrl.LowHiImpedanceCtrl == 1) column = 6; else column = 4;
        DisplayWriteString(column, 16, "C=", 2);
        DisplayWriteString(column, 16+6*12, "pF", 2);
        DisplayWriteString(column, 16+2*12, work_str_2, 4);
    }
    else if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X32_INV))
    {   // 128*32 OLED
        if(Ctrl.LowHiImpedanceCtrl == 1) column = 1; else column = 0;
        DisplayWriteString(column, 9, "C=", 2);
        DisplayWriteString(column, 15, "pF", 2);
        DisplayWriteString(column, 11, work_str_2, 4);
    }
    else if(Ctrl.DisplayType  == DISPLAY_TYPE_1602_LCD)
    {   // 1602 LCD
        if(Ctrl.LowHiImpedanceCtrl == 1) column = 1; else column = 0;
        DisplayWriteString(column, 9, "C=", 2);
        DisplayWriteString(column, 15, "p", 1);
        DisplayWriteString(column, 11, work_str_2, 4);
    }

    asm CLRWDT;
    return;
}

// ---------------------------------------------------------------------------

void TestModeInit(void)
{     
      // Test mode
      if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV)) // 128*64 OLED
      {
         DisplayWriteString(0, 10, "TEST MODE", 9);
      }
      else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)                        // 1602 LCD  or 128*32 OLED
      {
         DisplayWriteString(0, 3, "TEST MODE", 9);
      }

      asm CLRWDT;
      Delay_ms(500);
      asm CLRWDT;
      Delay_ms(500);
      asm CLRWDT;
      Delay_ms(500);
      asm CLRWDT;
      Delay_ms(500);
      asm CLRWDT;

      if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV)) // 128*64 OLED
      {
         DisplayWriteString(0, 10, "         ", 9);
      }
      else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)                        // 1602 LCD  or 128*32 OLED
      {
         DisplayWriteString(0, 3,  "         ", 9);
      }
     
      ResetATU();
      Ctrl.LowHiImpedanceCtrl = 1;                          // C to OUT
      SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
      EEPROM_Write(EEPROM_TUNED_CAP_CTRL, Ctrl.CapacitorsCtrl);
      EEPROM_Write(EEPROM_TUNED_INDUCT_CTRL, Ctrl.InductorsCtrl);
      EEPROM_Write(EEPROM_TUNED_IMPEDANCE_CTRL, Ctrl.LowHiImpedanceCtrl);
      //
      if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV)) // 128*64 OLED
      {
         DisplayWriteString(0, 16+12*8, "l", 1);
      }
      else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)                         // 1602 LCD or 128*32 OLED
      {
         DisplayWriteString(0, 8, "l", 1);
      }

      //
      Ctrl.LCDPrepNeeded = true;
      PrepareDisplay();
      return;
}

// ---------------------------------------------------------------------------

void LoadEEPROMData(void)
{
   // Cells init
   asm CLRWDT;
   
   //oled_addr = EEPROM_Read(EEPROM_DISPLAY_ADDRESS);                       // address
   Ctrl.DisplayType = EEPROM_Read(EEPROM_DISPLAY_TYPE);                     // display type
   Ctrl.AutoModeEnabled = EEPROM_Read(EEPROM_AUTO_MODE_ENABLED);            // auto mode
   Ctrl.RelaysDelay = Bcd2Dec(EEPROM_Read(EEPROM_RELAY_TIMEOUT));           // Relay's Delay
   Auto_delta = Bcd2Dec(EEPROM_Read(EEPROM_SWR_THRESHOLD)) * 10;            // Auto_delta
   Ctrl.MinPowerToTune = Bcd2Dec(EEPROM_Read(EEPROM_MIN_PWR_TO_TUNE)) * 10; // P_MinPowerToTune
   Ctrl.MaxPowerToTune = Bcd2Dec(EEPROM_Read(EEPROM_MAX_PWR_TO_TUNE)) * 10; // P_Ctrl.MaxPowerToTune
   // 0x07  - shift down
   // 0x08 - shift left
   Ctrl.MaxInitSWRValue = Bcd2Dec(EEPROM_Read(EEPROM_MAX_INIT_SWR)) * 10;   // Max SWR
   Ctrl.InductorsCount = EEPROM_Read(EEPROM_INSTALLED_INDUCTORS);           // Number of installed inductors
   Ctrl.LinearInductors = EEPROM_Read(EEPROM_LINEAR_INDUCTORS);             // Inductances with linear pitch
   Ctrl.CapacitorsCount = EEPROM_Read(EEPROM_INSTALLED_CAPACITORS);         // Number of installed capacitors
   Ctrl.LinearCapacitors = EEPROM_Read(EEPROM_LINEAR_CAPACITORS);           // Inductances with capacitors pitch
   Ctrl.RFDiodesCorrection = EEPROM_Read(EEPROM_RF_DIODES_CORRECTION);      // enable software correction of nonlinearity of diode`s RF detector.
   Ctrl.InvInductanceCtrl = EEPROM_Read(EEPROM_INV_INDUCTANCE_CTRL);        // inverse inductance control.
   //
   asm CLRWDT;
   
    // Reading Inductors values from EEPROM
   Ctrl.Inductor.L1 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_1_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_1_2_VALUE));   // Ind1
   Ctrl.Inductor.L2 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_2_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_2_2_VALUE));   // Ind2
   Ctrl.Inductor.L3 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_3_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_3_2_VALUE));   // Ind3
   Ctrl.Inductor.L4 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_4_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_4_2_VALUE));   // Ind4
   Ctrl.Inductor.L5 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_5_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_5_2_VALUE));   // Ind5
   Ctrl.Inductor.L6 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_6_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_6_2_VALUE));   // Ind6
   Ctrl.Inductor.L7 =  Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_7_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_INDUCTOR_7_2_VALUE));   // Ind7

   // Reading Capacitors values from EEPROM
   Ctrl.Capacitor.C1 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_1_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_1_2_VALUE));  // Cap1
   Ctrl.Capacitor.C2 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_2_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_2_2_VALUE));  // Cap2
   Ctrl.Capacitor.C3 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_3_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_3_2_VALUE));  // Cap3
   Ctrl.Capacitor.C4 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_4_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_4_2_VALUE));  // Cap4
   Ctrl.Capacitor.C5 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_5_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_5_2_VALUE));  // Cap5
   Ctrl.Capacitor.C6 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_6_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_6_2_VALUE));  // Cap6
   Ctrl.Capacitor.C7 =  Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_7_1_VALUE)) * 100 + Bcd2Dec(EEPROM_Read(EEPROM_CAPACITOR_7_2_VALUE));  // Cap7

   Ctrl.HighPowerMode = EEPROM_Read(EEPROM_HIPOWER_MODE_ENABLED);                 // High power mode
   Ctrl.TandenMatchRatio = Bcd2Dec(EEPROM_Read(EEPROM_TANDEM_MATCH_RATIO));       // Tandem Match rate
   Ctrl.BkLightDelay = Bcd2Dec(EEPROM_Read(EEPROM_DISPLAY_ON_DELAY));             // Dysplay ON delay
   Ctrl.IndicationMode = EEPROM_Read(EEPROM_INDICATION_MODE);                     // Loss indicator
   Ctrl.FeederPwrLossRatio = Bcd2Dec(EEPROM_Read(EEPROM_FEEDER_LOSS_RATIO));      // Feeder Power loss ratio
   Ctrl.RelayOffMode = Bcd2Dec(EEPROM_Read(EEPROM_RELAY_OFF_QRP_MODE));           // Relay off mode (for QRPs)

   asm CLRWDT;
   return;
}

// ---------------------------------------------------------------------------

void ShowFeederLoss(void)
{
   IntToStr(Ctrl.FeederPwrLossRatio, work_str);
   if(Ctrl.FeederPwrLossRatio>=10)
   {
      work_str_2[0] = work_str[4];
   }
   else 
   {
      work_str_2[0] = '0';
   }
   work_str_2[1] = '.';
   work_str_2[2] = work_str[5];

   if((Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64) | (Ctrl.DisplayType == DISPLAY_TYPE_OLED_128X64_INV))
   {
      DisplayWriteString(4, 6, work_str_2, 3); // 128*64
   }
   else if(Ctrl.DisplayType != DISPLAY_TYPE_DUAL_COLOR_LED)
   {
      DisplayWriteString(1, 0, work_str_2, 3); // 1602 | 128*32
   }
   return;
}

// ---------------------------------------------------------------------------

int ApplyRFDiodesCorrection(int input)
{
     // set correction
     if(input <= 80) return 0;
     
     // set correction
     if(input <= 171)        input += 244;
     else if(input <= 328)   input += 254;
     else if(input <= 582)   input += 280;
     else if(input <= 820)   input += 297;
     else if(input <= 1100)  input += 310;
     else if(input <= 2181)  input += 430;
     else if(input <= 3322)  input += 484;
     else if(input <= 4623)  input += 530;
     else if(input <= 5862)  input += 648;
     else if(input <= 7146)  input += 743;
     else if(input <= 8502)  input += 800;
     else if(input <= 10500) input += 840;
     else                    input += 860;

     // return corrected value
     return input;
}

// ---------------------------------------------------------------------------

int CalculateReversePower()
{
   int ReverseReading;

   // set ADC Vref to 1024 vmV
   FVRCON = 0b10000001;

   // wait for VRef ready
   while(FVRCON.B6 == 0);

   // read ADC
   ReverseReading = ADC_Get_Sample(ADC_REV_INPUT);
   if(ReverseReading <= 1000) return ReverseReading;

   // set ADC Vref to 2048 vmV
   FVRCON = 0b10000010;

   // wait for VRef ready
   while(FVRCON.B6 == 0);

   // read ADC
   ReverseReading = ADC_Get_Sample(ADC_REV_INPUT);
   if(ReverseReading <= 1000) return (ReverseReading * 2);

   // set ADC Vref to 4096 vmV
   FVRCON = 0b10000011;

   // wait for VRef ready
   while(FVRCON.B6 == 0);

   // read ADC
   ReverseReading = ADC_Get_Sample(ADC_REV_INPUT);
   return (ReverseReading * 4);
}

// ---------------------------------------------------------------------------

int CalculateForwardPower()
{
   int ForwardReading;
   
   // set ADC Vref to 1024 vmV
   FVRCON = 0b10000001;
   
   // wait for VRef ready
   while(FVRCON.B6 == 0);
   
   // read ADC
   ForwardReading = ADC_Get_Sample(ADC_FWR_INPUT);
   if(ForwardReading <= 1000)
   {
      Act.Overload = false;
      return ForwardReading;
   }

   // set ADC Vref to 2048 vmV
   FVRCON = 0b10000010;
   
   // wait for VRef ready
   while(FVRCON.B6 == 0);
   
   // read ADC
   ForwardReading = ADC_Get_Sample(ADC_FWR_INPUT);
   if(ForwardReading <= 1000)
   {
      Act.Overload = false;
      return ForwardReading * 2;
   }
   
   // set ADC Vref to 4096 vmV
   FVRCON = 0b10000011;

   // wait for VRef ready
   while(FVRCON.B6 == 0);

   // read ADC
   ForwardReading = ADC_Get_Sample(ADC_FWR_INPUT);
   if(ForwardReading > 1000)
   {
       Act.Overload = true;
   }
   else 
   {
       Act.Overload = false;
   }
   return ForwardReading * 4;
}

// ---------------------------------------------------------------------------

void CalculateOutputPower()
{
   long  ForwardReading, ReverseReading;
   float ActPower;
   
   // clear WDT
   asm CLRWDT;
   
   // calculate Forward and reverse power
   ForwardReading = CalculateForwardPower();
   ReverseReading = CalculateReversePower();

   // check for RF detectors correction
   if(Ctrl.RFDiodesCorrection == 1)
   {
      // corrected value
      ActPower = ApplyRFDiodesCorrection(ForwardReading * 3);
   }
   else
   {
      // direct value
      ActPower = ForwardReading * 3;
   }

   // check calculated values
   if(ReverseReading >= ForwardReading)
   {
      // Forward at its maximum
      ForwardReading = 999;
   }
   else
   {
      // calculate ratio
      ForwardReading = ((ForwardReading + ReverseReading) * 100) / (ForwardReading - ReverseReading);
      
      // check Forward ratio
      if(ForwardReading > 999) ForwardReading = 999;
   }

   // calculate Forward power
   ActPower = ActPower * Ctrl.TandenMatchRatio / 1000.0;   // mV to Volts on Input
   ActPower = ActPower / 1.414;
   
   // check for high Power mode
   if(Ctrl.HighPowerMode == 1)
   {
       // 0 - 1500 ( 1500 Watts)
       ActPower = ActPower * ActPower / 50;
   }
   else 
   {
       // 0 - 1510 (151.0 Watts)
       ActPower = ActPower * ActPower / 5;
   }
   
   // rounding
   ActPower = ActPower + 0.5;

   // copy to global Act struct
   Act.OutputPower = ActPower;
   
   // SWR as Forward ratio
   if(ForwardReading < 100)
   {
       Act.StandingWaveRatio = 999;
   }
   else
   {
       Act.StandingWaveRatio = ForwardReading;
   }
   return;
}

// ---------------------------------------------------------------------------

void CalculateStandingWaveRadio()
{
   // calculate ATU output power
   CalculateOutputPower();
   if(p_cnt!= 100)
   {
      p_cnt += 1;
      if(Act.OutputPower > P_max) P_max = Act.OutputPower;
   }
   else 
   {
      p_cnt = 0;
      DisplayPowerAndEfficiency(P_max, Act.StandingWaveRatio);
      P_max = 0;
   }
   
   while((Act.OutputPower < Ctrl.MinPowerToTune) | ((Act.OutputPower > Ctrl.MaxPowerToTune) & (Ctrl.MaxPowerToTune > 0)))
   {  // waiting for good power
      asm CLRWDT;
      CalculateOutputPower();
      if(p_cnt!=100) 
      {
         p_cnt += 1;
         if(Act.OutputPower > P_max) P_max = Act.OutputPower;
      }
      else 
      {
         p_cnt = 0;
         DisplayPowerAndEfficiency(P_max, Act.StandingWaveRatio);
         P_max = 0;
      }
      
      // Check Tune Button
      if(Button(&PORTB, 0, 5, 1))
      {
          Act.ReadyToTune = 1;
      }
      
      // check if Tune button is stil pressed
      if((Act.ReadyToTune == 1) & Button(&PORTB, 0, 5, 0))
      { 
         // reset
         DisplayResetATU();
         Act.StandingWaveRatio = 0;
         return;
      }
   } //  good power
   return;
}

// ---------------------------------------------------------------------------

void SetInductance(char Ind)
{
   // Send output state to inductors port pins
   if(Ctrl.InvInductanceCtrl == 0)      // check inverse inductance control
   {
      Ind_005 = Ind.B0;
      Ind_011 = Ind.B1;
      Ind_022 = Ind.B2;
      Ind_045 = Ind.B3;
      Ind_1   = Ind.B4;
      Ind_22  = Ind.B5;
      Ind_45  = Ind.B6;
   }
   else 
   {
      Ind_005 = ~Ind.B0;
      Ind_011 = ~Ind.B1;
      Ind_022 = ~Ind.B2;
      Ind_045 = ~Ind.B3;
      Ind_1   = ~Ind.B4;
      Ind_22  = ~Ind.B5;
      Ind_45  = ~Ind.B6;
   }
   
   // delay
   VDelay_ms(Ctrl.RelaysDelay);
}

// ---------------------------------------------------------------------------

void SetCapacitance(char Cap)
{
   // Send output state to capacitors port pins
   Cap_10   =  Cap.B0;
   Cap_22   =  Cap.B1;
   Cap_47   =  Cap.B2;
   Cap_100  =  Cap.B3;
   Cap_220  =  Cap.B4;
   Cap_470  =  Cap.B5;
   Cap_1000 =  Cap.B6;
   
   // delay
   VDelay_ms(Ctrl.RelaysDelay);
}

// ---------------------------------------------------------------------------

void SetImpedanceSwitch(char value)
{  
   // 0 - IN,  1 - OUT
   S8_SW = value;
   VDelay_ms(Ctrl.RelaysDelay);
}

// ---------------------------------------------------------------------------

void ResetATU()
{
   // all Inductors and Capacitors are out
   Ctrl.InductorsCtrl  = OUT_ALL_OFF;
   Ctrl.CapacitorsCtrl = OUT_ALL_OFF;
   
   // set state
   SetInductance(Ctrl.InductorsCtrl);
   SetCapacitance(Ctrl.CapacitorsCtrl);
   
   // delay
   VDelay_ms(Ctrl.RelaysDelay);
}

// ---------------------------------------------------------------------------

void CapacitorCoarseTune()
{
   char step = 3;
   char count;
   int min_swr;

   Ctrl.CapacitorsCtrl = 0;
   SetCapacitance(Ctrl.CapacitorsCtrl);
   step_cap = step;
   CalculateStandingWaveRadio();
   if(Act.StandingWaveRatio == 0) return;
   min_swr = Act.StandingWaveRatio + Act.StandingWaveRatio/20;
   for(count = step; count <= 31;)
   {
      SetCapacitance(count*Ctrl.CapFactor);
      CalculateStandingWaveRadio();
      if(Act.StandingWaveRatio == 0) return;
      if(Act.StandingWaveRatio < min_swr) 
      {
         min_swr = Act.StandingWaveRatio + Act.StandingWaveRatio/20;
         Ctrl.CapacitorsCtrl = count*Ctrl.CapFactor;
         step_cap = step;
         if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) break;
         count +=step;
         if((Ctrl.LinearCapacitors == 0) & (count == 9))
           count = 8;
         else if((Ctrl.LinearCapacitors == 0) & (count == 17))
         {
            count = 16; 
            step = 4;
         }
      }
      else break;
   }
   SetCapacitance(Ctrl.CapacitorsCtrl);
   return;
}

// ---------------------------------------------------------------------------

void CoarseTune()
{
   char step = 3;
   char count;
   char mem_cap, mem_step_cap;
   int min_swr;

   mem_cap = 0;
   step_ind = step;
   mem_step_cap = 3;
   min_swr = Act.StandingWaveRatio + Act.StandingWaveRatio/20;
   for(count=0; count<=31;) 
   {
      SetInductance(count*Ctrl.IndFactor);
      CapacitorCoarseTune();
      CalculateStandingWaveRadio();
      if(Act.StandingWaveRatio == 0) return;
      
      if(Act.StandingWaveRatio < min_swr) 
      {
         min_swr = Act.StandingWaveRatio + Act.StandingWaveRatio/20;
         Ctrl.InductorsCtrl = count*Ctrl.IndFactor;
         mem_cap = Ctrl.CapacitorsCtrl;
         step_ind = step;
         mem_step_cap = step_cap;
         if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) break;
         count +=step;
         if((Ctrl.LinearInductors == 0) & (count == 9))
            count = 8;
         else if((Ctrl.LinearInductors == 0) & (count == 17))
         {
            count = 16; 
            step = 4;
         }
      }
      else break;
   }
   Ctrl.CapacitorsCtrl = mem_cap;
   SetInductance(Ctrl.InductorsCtrl);
   SetCapacitance(Ctrl.CapacitorsCtrl);
   step_cap = mem_step_cap;
   Delay_ms(10);
   return;
}

// ---------------------------------------------------------------------------

void CapacitorSharpTune()
{
   char range, count, max_range, min_range;
   int min_swr;
   range = step_cap*Ctrl.CapFactor;
   //
   max_range = Ctrl.CapacitorsCtrl + range;
   if(max_range > (32*Ctrl.CapFactor-1))
      max_range = 32*Ctrl.CapFactor-1;
   if(Ctrl.CapacitorsCtrl > range)
      min_range = Ctrl.CapacitorsCtrl - range;
   else 
      min_range = 0;
   Ctrl.CapacitorsCtrl = min_range;
   SetCapacitance(Ctrl.CapacitorsCtrl);
   CalculateStandingWaveRadio(); 
   
   if(Act.StandingWaveRatio == 0) return;
   
   min_swr = Act.StandingWaveRatio;
   for(count = (min_range + Ctrl.CapFactor); count <= max_range; count += Ctrl.CapFactor)
   {
      SetCapacitance(count);
      CalculateStandingWaveRadio();
      if(Act.StandingWaveRatio == 0) return;
      
      if(Act.StandingWaveRatio >= min_swr) 
      { 
         Delay_ms(10); 
         CalculateStandingWaveRadio(); 
      }
      if(Act.StandingWaveRatio >= min_swr) 
      { 
         Delay_ms(10); 
         CalculateStandingWaveRadio(); 
      }
      if(Act.StandingWaveRatio < min_swr) 
      {
         min_swr = Act.StandingWaveRatio;
         Ctrl.CapacitorsCtrl = count;
         if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) break;
      }
      else break;
   }
   SetCapacitance(Ctrl.CapacitorsCtrl);
   return;
}

// ---------------------------------------------------------------------------

void InductorSharpTune()
{
   char range, count, max_range, min_range;
   int min_SWR;
   range = step_ind * Ctrl.IndFactor;
   //
   max_range = Ctrl.InductorsCtrl + range;
   if(max_range > (32*Ctrl.IndFactor-1))
      max_range = 32*Ctrl.IndFactor - 1;
   if(Ctrl.InductorsCtrl > range)
       min_range = Ctrl.InductorsCtrl - range;
   else 
      min_range = 0;
   Ctrl.InductorsCtrl = min_range;
   SetInductance(Ctrl.InductorsCtrl);
   CalculateStandingWaveRadio(); 
   
   if(Act.StandingWaveRatio == 0) return;
   
   min_SWR = Act.StandingWaveRatio;
   for(count = (min_range + Ctrl.IndFactor); count <= max_range; count += Ctrl.IndFactor)
   {
      SetInductance(count);
      CalculateStandingWaveRadio(); 
      
      if(Act.StandingWaveRatio == 0) return;
      
      if(Act.StandingWaveRatio >= min_SWR)
      { 
         Delay_ms(10); 
         CalculateStandingWaveRadio(); 
      }
      if(Act.StandingWaveRatio >= min_SWR)
      { 
         Delay_ms(10); 
         CalculateStandingWaveRadio(); 
      }
      if(Act.StandingWaveRatio < min_SWR) 
      {
         min_SWR = Act.StandingWaveRatio;
         Ctrl.InductorsCtrl = count;
         if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) break;
      }
      else break;
   }
   SetInductance(Ctrl.InductorsCtrl);
   return;
}

// ---------------------------------------------------------------------------

void TuneATU()
{
   int swr_mem, ind_mem, cap_mem;
   //
   swr_mem = Act.StandingWaveRatio;
   CoarseTune();
   
   if(Act.StandingWaveRatio == 0) 
   {
      ResetATU(); 
      return;
   }
   
   CalculateStandingWaveRadio(); 
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   
   InductorSharpTune();
   if(Act.StandingWaveRatio == 0) 
   {
      ResetATU(); 
      return;
   }
   
   CalculateStandingWaveRadio(); 
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   
   CapacitorSharpTune();
   if(Act.StandingWaveRatio == 0) 
   {
      ResetATU(); 
      return;
   }
   
   CalculateStandingWaveRadio(); 
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   if((Act.StandingWaveRatio < 200) & (Act.StandingWaveRatio < swr_mem) & ((swr_mem-Act.StandingWaveRatio)>100)) return;
   
   swr_mem = Act.StandingWaveRatio;
   ind_mem = Ctrl.InductorsCtrl;
   cap_mem = Ctrl.CapacitorsCtrl;
   //
   if(Ctrl.LowHiImpedanceCtrl == 1)
   {
      Ctrl.LowHiImpedanceCtrl = 0;
   }
   else 
   {
      Ctrl.LowHiImpedanceCtrl = 1;
   }
   
   ResetATU();
   SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
   Delay_ms(50);
   
   CalculateStandingWaveRadio(); 
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;

   CoarseTune();
   if(Act.StandingWaveRatio == 0)
   {
      ResetATU(); 
      return;
   }
   
   CalculateStandingWaveRadio(); 
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   
   InductorSharpTune();
   if(Act.StandingWaveRatio == 0)
   {
      ResetATU();
      return;
   }

   CalculateStandingWaveRadio(); 
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;

   CapacitorSharpTune(); 
   if(Act.StandingWaveRatio == 0)
   {
      ResetATU();
      return;
   }

   CalculateStandingWaveRadio();
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   //
   if(Act.StandingWaveRatio > swr_mem) 
   {
      if(Ctrl.LowHiImpedanceCtrl == 1)
      {   
         Ctrl.LowHiImpedanceCtrl = 0;
      }
      else 
      {   
         Ctrl.LowHiImpedanceCtrl = 1;
      }
      SetImpedanceSwitch(Ctrl.LowHiImpedanceCtrl);
      Ctrl.InductorsCtrl = ind_mem;
      Ctrl.CapacitorsCtrl = cap_mem;
      SetInductance(Ctrl.InductorsCtrl);
      SetCapacitance(Ctrl.CapacitorsCtrl);
      Act.StandingWaveRatio = swr_mem;
   }
   //
   asm CLRWDT;
   return;
}

// ---------------------------------------------------------------------------

void StartTunningProcess()
{
   // clear WDT
   asm CLRWDT;

   // initialize variables
   p_cnt = 0;
   P_max = 0;
   Act.ReadyToTune = 0;

   // calculate current SWR
   CalculateStandingWaveRadio();
   
   // check SWR limit
   if(Act.StandingWaveRatio < IDEAL_OPERATING_SWR) return;
   
   // reset ATU
   ResetATU();
   
   
   if(Ctrl.IndicationMode == 0) DisplayLCValues();
   Delay_ms(50);
   CalculateStandingWaveRadio();
   swr_a = Act.StandingWaveRatio;
   if(Act.StandingWaveRatio < IDEAL_OPERATING_SWR) return;
   if((Ctrl.MaxInitSWRValue > IDEAL_OPERATING_SWR) & (Act.StandingWaveRatio > Ctrl.MaxInitSWRValue)) return;
   //
   TuneATU(); 
   if(Act.StandingWaveRatio == 0) 
   {
      ResetATU(); 
      return;
   }
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   if((Ctrl.CapacitorsCount == 5) & (Ctrl.InductorsCount == 5)) return;

   // Test for amount of inductors
   if(Ctrl.InductorsCount > 5)
   {
      step_ind = Ctrl.IndFactor;
      Ctrl.IndFactor = 1;
      InductorSharpTune();
   }
   
   // check SWR limit
   if(Act.StandingWaveRatio < LIMIT_OPERATING_SWR) return;
   
   // Test for amount of capacitors
   if(Ctrl.CapacitorsCount > 5)
   {
      step_cap = Ctrl.CapFactor;  // = C_mult
      Ctrl.CapFactor = 1;
      CapacitorSharpTune();
   }
   
   // Test for amount of inductors
   if(Ctrl.InductorsCount == 5)
   {
       Ctrl.IndFactor = 1;
   }
   else if(Ctrl.InductorsCount == 6)
   {
       Ctrl.IndFactor = 2;
   }
   else if(Ctrl.InductorsCount == 7)
   {
       Ctrl.IndFactor = 4;
   }

   // Test for amount of capacitors
   if(Ctrl.CapacitorsCount == 5)
   {
       Ctrl.CapFactor =1;
   }
   else if(Ctrl.CapacitorsCount == 6)
   {
       Ctrl.CapFactor = 2;
   }
   else if(Ctrl.CapacitorsCount == 7)
   {
       Ctrl.CapFactor = 4;
   }
   asm CLRWDT;
   return;
}