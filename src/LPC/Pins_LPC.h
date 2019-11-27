#ifndef PINS_DUET_H__
#define PINS_DUET_H__

#include "Microstepping.h"
#include "sd_mmc.h"

const size_t NumFirmwareUpdateModules = 1;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif





#define FIRMWARE_NAME "RepRapFirmware for LPC176x based Boards"

// Default board type
#define DEFAULT_BOARD_TYPE BoardType::Lpc
#define ELECTRONICS "LPC176x"
#define LPC_ELECTRONICS_STRING "LPC176x"
#define LPC_BOARD_STRING "LPC176x"

#define FIRMWARE_FILE "firmware.bin"

// Features definition
#define SUPPORT_OBJECT_MODEL         1
#define HAS_CPU_TEMP_SENSOR		     0				// enabling the CPU temperature sensor disables Due pin 13 due to bug in SAM3X
#define HAS_HIGH_SPEED_SD		     0
#define HAS_VOLTAGE_MONITOR		     0
#define ACTIVE_LOW_HEAT_ON		     0
#define HAS_LWIP_NETWORKING          0
#define HAS_WIFI_NETWORKING          0
#define HAS_VREF_MONITOR             0
#define SUPPORT_NONLINEAR_EXTRUSION  1
#define SUPPORT_INKJET		         0					// set nonzero to support inkjet control
#define SUPPORT_ROLAND		         0					// set nonzero to support Roland mill
#define SUPPORT_SCANNER		         0					// set nonzero to support FreeLSS scanners
#define SUPPORT_IOBITS		         0					// set to support P parameter in G0/G1 commands
#define SUPPORT_DHT_SENSOR	         0					// set nonzero to support DHT temperature/humidity sensors

#define SUPPORT_TELNET               0
#define SUPPORT_FTP                  0

#define SUPPORT_WORKPLACE_COORDINATES    1

#if defined(LPC_NETWORKING)
    #define HAS_RTOSPLUSTCP_NETWORKING   1
    #define SUPPORT_12864_LCD            1
#else
    #define HAS_RTOSPLUSTCP_NETWORKING   0
    #define SUPPORT_12864_LCD            1
#endif

constexpr size_t NumExtraHeaterProtections = 4;


// Tacho Pins - Only GPIO pins on Port0 and Port2 are supported.
constexpr size_t NumTachos = 1;
extern Pin TachoPins[NumTachos];


// The physical capabilities of the machine
constexpr size_t NumDirectDrivers = 5;                // The maximum number of drives supported by the electronics
constexpr size_t MaxTotalDrivers = NumDirectDrivers;
constexpr size_t MaxSmartDrivers = 0;                // The maximum number of smart drivers

constexpr size_t NumEndstops = 6;                    // The number of inputs we have for endstops, filament sensors etc.
constexpr size_t NumHeaters = 3;                    // The number of heaters in the machine; 0 is the heated bed even if there isn't one
constexpr size_t NumThermistorInputs = 3;

constexpr size_t MinAxes = 3;                        // The minimum and default number of axes
constexpr size_t MaxAxes = 5;                        // The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES

constexpr size_t MaxExtruders = NumDirectDrivers - MinAxes;    // The maximum number of extruders
constexpr size_t MaxDriversPerAxis = 2;                // The maximum number of stepper drivers assigned to one axis

constexpr size_t MaxHeatersPerTool = 2;
constexpr size_t MaxExtrudersPerTool = 2;


//Steppers
extern Pin ENABLE_PINS[NumDirectDrivers];
extern Pin STEP_PINS[NumDirectDrivers];
extern Pin DIRECTION_PINS[NumDirectDrivers];
extern uint32_t STEP_DRIVER_MASK; // Mask for parallel write to all steppers on port 2 (calculated in firmware)
extern bool hasStepPinsOnDifferentPorts;
extern bool hasDriverCurrentControl;
extern float digipotFactor;

//Smoothie uses MCP4451 for current control
// Indices for motor current digipots (X,Y,Z,E) - E is on 2nd digipot chip
constexpr uint8_t POT_WIPES[5] = { 0, 1, 2, 3, 0};


//endstops
extern Pin END_STOP_PINS[NumEndstops];

extern Pin Z_PROBE_PIN; // Z Probe pin
extern Pin Z_PROBE_MOD_PIN; // Digital pin number to turn the IR LED on (high) or off (low)

// HEATERS - The bed is assumed to be the at index 0
extern Pin TEMP_SENSE_PINS[NumThermistorInputs];
extern Pin HEAT_ON_PINS[NumHeaters];



//Timer Freqs for 1 and 3. Timer 2 is fixed at 50Hz for servos
extern uint16_t Timer1Frequency;
extern uint16_t Timer3Frequency;


// Default thermistor betas
constexpr float BED_R25 = 100000.0;
constexpr float BED_BETA = 3988.0;
constexpr float BED_SHC = 0.0;
constexpr float EXT_R25 = 100000.0;
constexpr float EXT_BETA = 4388.0;
constexpr float EXT_SHC = 0.0;
constexpr float THERMISTOR_SERIES_RS = 4700.0; // Thermistor series resistor value in Ohms

constexpr size_t MaxSpiTempSensors = 2;
extern Pin SpiTempSensorCsPins[MaxSpiTempSensors]; // Digital pins the 31855s have their select lines tied to
constexpr SSPChannel TempSensorSSPChannel = SSP0; //Conect SPI Temp sensor to SSP0

extern Pin ATX_POWER_PIN;// Digital pin number that controls the ATX power on/off

constexpr size_t NUM_FANS = 2;
extern Pin COOLING_FAN_PINS[NUM_FANS]; // Use a PWM capable pin (or pin on a timer)


//SD: Internal SDCard is on SSP1
//    MOSI, MISO, SCLK, CS
//    P0_9, P0_8, P0_7, P0_6

//SD:: 2nd SDCard can be connected to SSP0
//    MOSI, MISO, SCLK
//    P0_18 P0_17 P0_15

// SD cards
constexpr size_t NumSdCards = _DRIVES; //_DRIVES is defined in CoreLPC (and used by FatFS) allow one internal and one external
extern Pin SdCardDetectPins[NumSdCards];
constexpr Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin }; //unused on LPC boards
extern Pin SdSpiCSPins[NumSdCards];
extern uint32_t ExternalSDCardFrequency;
extern uint32_t InternalSDCardFrequency;
extern SSPChannel ExternalSDCardSSPChannel;


// Definition of which pins we allow to be controlled using M42 etc
constexpr size_t MaxNumberSpecialPins = 10;
extern Pin SpecialPinMap[MaxNumberSpecialPins];

constexpr uint32_t LcdSpiClockFrequency = 2000000;    // 2.0MHz
extern SSPChannel LcdSpiChannel;
extern Pin LcdCSPin;
extern Pin LcdDCPin;
extern Pin LcdBeepPin;
extern Pin EncoderPinA;
extern Pin EncoderPinB;
extern Pin EncoderPinSw;
extern Pin PanelButtonPin;

//LEDs
extern Pin StatusLEDPin;
extern Pin DiagPin;


constexpr size_t NUM_SERIAL_CHANNELS = 2;
extern bool UARTPanelDueMode;

constexpr size_t NumSoftwareSPIPins = 3;
extern Pin SoftwareSPIPins[3]; //GPIO pins for softwareSPI (used with SharedSPI)

// Use TX0/RX0 for the auxiliary serial line
#if defined(__MBED__)
    #define SERIAL_MAIN_DEVICE Serial0 //TX0/RX0 connected to via seperate USB
    #define SERIAL_AUX_DEVICE Serial //USB pins unconnected.
#else
    #define SERIAL_MAIN_DEVICE Serial //USB
    #define SERIAL_AUX_DEVICE Serial0 //TX0/RX0
#endif


// This next definition defines the highest one.
const int HighestLogicalPin = 60 + MaxNumberSpecialPins - 1;        // highest logical pin number on this electronics


//Timer 0 is used for Step Generation
#define STEP_TC				(LPC_TIM0)
#define STEP_TC_IRQN		TIMER0_IRQn
#define STEP_TC_HANDLER		TIMER0_IRQHandler
#define STEP_TC_PCONPBIT    SBIT_PCTIM0
#define STEP_TC_PCLKBIT     PCLK_TIMER0
#define STEP_TC_TIMER       TIMER0


// From section 3.12.7 of http://infocenter.arm.com/help/topic/com.arm.doc.dui0553b/DUI0553.pdf:
// When you write to BASEPRI_MAX, the instruction writes to BASEPRI only if either:
// � Rn is non-zero and the current BASEPRI value is 0
// � Rn is non-zero and less than the current BASEPRI value
__attribute__( ( always_inline ) ) __STATIC_INLINE void __set_BASEPRI_MAX(uint32_t value)
{
  __ASM volatile ("MSR basepri_max, %0" : : "r" (value) : "memory");
}

#endif
