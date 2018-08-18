// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

// This node ID, should be unique in the same network
#define NODEID              11

// Gateway ID that will receive the messages from this node
#define GATEWAYID           2

// Network ID, only nodes from the same network see each other
#define NETWORKID           200

// Frequency of the radio, should match your radio module and
// comply with your country legislation
#define FREQUENCY           RF69_868MHZ

// Encription key, shared between the node and the gateway
#define ENCRYPTKEY          "sampleEncryptKey"

// If you are using a RFM69HW or RFM69HCW module set this to 1 to
// transmitt to extra power (and extra battery drainage)
#define IS_RFM69HW          1

// Set the target RSSI for Auto Transmission Control feature.
// The module will modify it's transmitting power to match this
// value (relies on ACK packets to check the RSSI the gateway has seen).
#define ATC_RSSI            -75

// Comment to stop sending debug messages to serial console
#define DEBUG

// Define serial baudrate
#define SERIAL_BAUD         115200

// Various PIN definitions
#define LED_PIN             9
#define BATTERY_PIN         A7
#define USE_MOSFET          0
#define BATTERY_ENABLE_PIN  A3

// SPI Flash
#define FLASH_SS            8

// Sleeping
// 37 times 8 seconds equals almost 5 minutes
// In real world it's more like 5 minutes 36 seconds,
// so I user 32 instead
#define SLEEP_COUNT         32

// Flash LED for this amount of milliseconds after every message sent
#define NOTIFICATION_TIME   5

// Battery monitoring circuitry:
// Vi -- R1 -- A1 (Vo) -- R2 -- D12 (GND)
//
// These values have been measured with a multimeter:
// R1 = 470k
// R2 = 1000k
//
// Formulae:
// Vo = Vi * R2 / (R1 + R2)
// Vi = Vo * (R1 + R2) / R2
// Vo = X * 3300 / 1024;
//
// Thus:
// Vi = X * 3300 * (1000 + 470) / 1024 / 1000;
#define BATTERY_RATIO       4.7373
