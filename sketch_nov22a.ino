#include <SPI.h>
#include <EtherCard.h>

#define STATIC 1 // set to 1 to disable DHCP (adjust myip/gwip values below)

#if STATIC
// Ethernet interface IP address
static byte myip[] = { 192, 168, 1, 180 };

// Gateway IP address
static byte gwip[] = { 192, 168, 1, 254 };
#endif

// Ethernet MAC address - must be unique on your network
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x39 };

byte Ethernet::buffer[500]; // TCP/IP send and receive buffer

// Relay pin
#define RELAY_PIN 7

// HTML pages with Imgur links
const char relayOnPage[] PROGMEM =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<html>"
"<head><title>Relay Control</title></head>"
"<body>"
"<h2>Relay is ON</h2>"
"<img src=\"https://i.imgur.com/C3L49Vt.jpg\" alt=\"Lamp On\">"
"<p><a href='/relay1off'><button>Turn OFF</button></a></p>"
"</body>"
"</html>";

const char relayOffPage[] PROGMEM =
"HTTP/1.0 200 OK\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<html>"
"<head><title>Relay Control</title></head>"
"<body>"
"<h2>Relay is OFF</h2>"
"<img src=\"https://i.imgur.com/rPUP9ox.jpg\" alt=\"Lamp Off\">"
"<p><a href='/relay1on'><button>Turn ON</button></a></p>"
"</body>"
"</html>";

void setup() {
    // Initialize the serial port
    Serial.begin(9600);
    Serial.println("\n[backSoon]");

    // Initialize the relay pin
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Ensure the relay is OFF initially

    // Initialize the Ethernet controller
    if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0)
        Serial.println("Failed to access Ethernet controller");

    #if STATIC
        ether.staticSetup(myip, gwip);
    #else
        if (!ether.dhcpSetup())
            Serial.println("DHCP failed");
    #endif

    ether.printIp("IP: ", ether.myip);
    ether.printIp("GW: ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
}

void loop() {
    // Wait for an incoming HTTP request and process it
    if (ether.packetLoop(ether.packetReceive())) {
        // Get the HTTP request data from the buffer
        char request[100];
        char *ptr = (char*)ether.tcpOffset();
        strncpy(request, ptr, sizeof(request)-1);
        request[sizeof(request)-1] = 0;  // Ensure null-termination

        // Check for specific URI paths
        if (strstr(request, "/relay1on")) {
            // Turn relay ON
            Serial.println("Relay turned ON");
            digitalWrite(RELAY_PIN, HIGH); // Activate relay
            memcpy_P(ether.tcpOffset(), relayOnPage, sizeof relayOnPage);
            ether.httpServerReply(sizeof relayOnPage - 1);
        }
        else if (strstr(request, "/relay1off")) {
            // Turn relay OFF
            Serial.println("Relay turned OFF");
            digitalWrite(RELAY_PIN, LOW); // Deactivate relay
            memcpy_P(ether.tcpOffset(), relayOffPage, sizeof relayOffPage);
            ether.httpServerReply(sizeof relayOffPage - 1);
        }
        else {
            // Default page (shows the OFF state)
            memcpy_P(ether.tcpOffset(), relayOffPage, sizeof relayOffPage);
            ether.httpServerReply(sizeof relayOffPage - 1);
        }
    }
}
