#include <Arduino.h>
#include <ArduinoLog.h>
#include <RadioLib.h>

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

void setFlag(void);

SX1278 radio_rx = new Module(10, 2, 9, 8);
SX1278 radio_tx = new Module(11, 3, 6, 7);

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        ;
    }

    Log.begin(LOG_LEVEL_VERBOSE, &Serial);
    //Log.setPrefix(printTimestamp); // Uncomment to get timestamps as prefix
    //Log.setSuffix(printNewline); // Uncomment to get newline as suffix

    //Start logging

    Log.notice(F(CR "******************************************" CR)); // Info string with Newline
    Log.notice("***          Logging example                " CR);     // Info string in flash memory
    Log.notice(F("******************* "));
    Log.notice("*********************** " CR); // two info strings without newline

    // initialize SX1278 with default settings
    Serial.print(F("[SX1278] Initializing RX module..."));
    volatile int state = radio_rx.begin();
    if (state == ERR_NONE)
    {
        Serial.println(F("RX module init success!"));
    }
    else
    {
        Serial.print(F("RX module init failed, code "));
        Serial.println(state);
        Serial.print(F("Entering trap... ("));

        while (true)
            ;
    }

    // initialize SX1278 with default settings
    Serial.print(F("[SX1278] Initializing TX module..."));
    state = radio_tx.begin();
    if (state == ERR_NONE)
    {
        Serial.println(F("TX module init success!"));
    }
    else
    {
        Serial.print(F("TX module init failed, code "));
        Serial.println(state);
        Serial.print(F("Entering trap... ("));

        while (true)
            ;
    }

    radio_tx.sleep();
    // radio_tx.standby();

    radio_rx.setDio0Action(setFlag);
}

void loop()
{
    if (receivedFlag)
    {
        // disable the interrupt service routine while
        // processing the data
        enableInterrupt = false;

        // reset flag
        receivedFlag = false;

        // you can read received data as an Arduino String
        String received_packet;
        int state = radio_rx.readData(received_packet);

        if (state == ERR_NONE)
        {
            // packet was successfully received
            Serial.println(F("[SX1278 RX module] received packet!"));

            // print data of the packet
            Serial.print(F("[SX1278 RX module] Data:\t\t"));
            Serial.println(received_packet);

            // print RSSI (Received Signal Strength Indicator)
            Serial.print(F("[SX1278 RX module] RSSI:\t\t"));
            Serial.print(radio_rx.getRSSI());
            Serial.println(F(" dBm"));

            // print SNR (Signal-to-Noise Ratio)
            Serial.print(F("[SX1278 RX module] SNR:\t\t"));
            Serial.print(radio_rx.getSNR());
            Serial.println(F(" dB"));
        }
        else if (state == ERR_CRC_MISMATCH)
        {
            // packet was received, but is malformed
            Serial.println(F("[SX1278 RX module] CRC error!"));
        }
        else
        {
            // some other error occurred
            Serial.print(F("[SX1278 RX module] failed, code "));
            Serial.println(state);
        }

        Serial.print(F("[SX1278 TX Module] Transmitting packet from module 2 ... "));
        state = radio_tx.transmit(received_packet);
        if (state == ERR_NONE)
        {
            // the packet was successfully transmitted
            Serial.println(F("[SX1278 TX Module] success!"));
        }
        else if (state == ERR_PACKET_TOO_LONG)
        {
            // the supplied packet was longer than 256 bytes
            Serial.println(F("[SX1278 TX Module] too long!"));
        }
        else
        {
            // some other error occurred
            Serial.print(F("[SX1278 TX Module] failed, code "));
            Serial.println(state);
        }

        // put tx module to sleep
        radio_tx.sleep();
        delay(1);

        // put module back to listen mode
        radio_rx.startReceive();

        // we're ready to receive more packets,
        // enable interrupt service routine
        enableInterrupt = true;
    }
}

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)
{
    // check if the interrupt is enabled
    if (!enableInterrupt)
    {
        return;
    }

    // we got a packet, set the flag
    receivedFlag = true;
}