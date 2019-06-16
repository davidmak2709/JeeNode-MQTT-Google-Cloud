#include <EtherCard.h>
#include <Ports.h>
#include <JeeLib.h>

#include <RF12.h>

static byte mymac[] = { 0x1A,0x2B,0x3C,0x4D,0x5E,0x6F };
byte Ethernet::buffer[700];
static uint32_t timer;

const char website[] PROGMEM = "10.42.0.1";
const int dstPort PROGMEM = 5000;
const int srcPort PROGMEM = 4321;

Port ledG(2);
Port ledY(3);


static void gotPinged (byte* ptr) {
  ether.printIp(">>> ping from: ", ptr);
}


void setup () {
  Serial.begin(9600);
  ledG.mode2(OUTPUT);
  ledY.mode(OUTPUT);  

  ledG.digiWrite2(0x100);
  ledY.digiWrite(0x100);
  delay(500);
  
  if (ether.begin(sizeof Ethernet::buffer, mymac, 8) == 0)
    Serial.println( "Failed to access Ethernet controller");
  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
  ether.registerPingCallback(gotPinged);

  rf12_initialize(1 , RF12_868MHZ, 100);
  
  ledG.digiWrite2(~0x100);
  ledY.digiWrite(~0x100);
}


void loop () {
      word len = ether.packetReceive();
      word pos = ether.packetLoop(len); 

      

      if (len > 0 && ether.packetLoopIcmpCheckReply(ether.hisip)) {
          ledG.digiWrite2(0x100);
          ledY.digiWrite(~0x100);
      } else {
          ledG.digiWrite2(~0x100);
          ledY.digiWrite(0x100);
      }
     
     if (rf12_recvDone() && rf12_crc == 0) {
        Serial.print("OK ");
        Serial.print(rf12_hdr);
        Serial.print(" ");
        char payload[rf12_len];
        for (byte i = 0; i < rf12_len; ++i)
            payload[i] = (char)rf12_data[i];
        String str(payload);
        str.concat("-");
        str.concat((char ) (rf12_hdr + 48));
   
        
        char packet[str.length()+1];
        str.toCharArray(packet,str.length()+1);
        Serial.println(packet);

        ether.sendUdp(packet, sizeof(packet), srcPort, ether.hisip, dstPort );
    }

     ether.clientIcmpRequest(ether.hisip);
     
}
