#include <Wire.h>
#include <avr/power.h>
#include <imframe.h>
#include <imcc1101.h>
#include <imsht.h>
#include <imcharger.h>

#define PinAkku A2
#define EnableI2C A3
#define EnableSHT A0
#define EnableExtPower 6
#define EnableAkku A1
#define EnableRS485 5

IMCharger	imCharger;
IMCC1101	imCC1101;
IMSht2x		imSht2x;

typedef struct {
	float temperature;
	float humidity;
} IMFrameBody;

void setup()
{
	Serial.begin(115000);
	delay(5000);
	Serial.println("IM TX Starting....");

	pinMode(EnableI2C, OUTPUT);
	pinMode(EnableSHT, OUTPUT);
	//pinMode(EnableRS485, OUTPUT);
	digitalWrite(EnableSHT, HIGH);
	digitalWrite(EnableI2C, HIGH);	
	//digitalWrite(EnableRS485, LOW);

	imCharger.Init();
	imCC1101.Init();	
}

void loop()
{
	IMFrame imFrame;
	imFrame.Header.SourceId = 100;
	imFrame.Header.Function = 1;
	imFrame.Header.DestinationId = 1;

	IMFrameBody imFrameBody;
	imFrameBody.temperature = imSht2x.GetTemperature();
	imFrameBody.humidity = imSht2x.GetHumidity();

	memcpy(imFrame.Body, &imFrameBody, sizeof(imFrameBody));	
	imCC1101.SendData(imFrame);

	Serial.print("temp: ");
	Serial.print(imFrameBody.temperature);
	Serial.print(" hum: ");
	Serial.println(imFrameBody.humidity);

	//Check charger stat and keepalive charging
	/*byte VinStat = imCharger.GetVinStat();
	if (VinStat != 0b00) {
		imCharger.KeepAlive();
	}*/

	delay(1000);
}
