#include <Wire.h>
#include <imframe.h>
#include <imcc1101.h>

IMCC1101 imCC1101;

byte RX_buffer[61] = { 0 };

typedef struct {
	float temperature;
	float humidity;
} IMFrameBody;

void setup()
{
	Serial.begin(115000);
	delay(5000);
	Serial.println("IM RX Starting....");
	
	pinMode(13, OUTPUT);

	imCC1101.Init();
	imCC1101.CheckReceiveFlag();
}

void loop()
{
	digitalWrite(13, LOW);
	if (imCC1101.CheckReceiveFlag())
	{
		digitalWrite(13, HIGH);

		imCC1101.ReceiveData(RX_buffer);

		IMFrame imFrame;
		IMFrameBody imFrameBody;
		memcpy(&imFrame, RX_buffer, sizeof(imFrame));
		memcpy(&imFrameBody, imFrame.Body, sizeof(imFrameBody));
		

		Serial.print("SourceId: ");
		Serial.print(imFrame.Header.SourceId);
		Serial.print(" DestinationId: ");
		Serial.print(imFrame.Header.DestinationId);
		Serial.print(" Temp: ");
		Serial.print(imFrameBody.temperature);
		Serial.print(" Hum: ");
		Serial.println(imFrameBody.humidity);

		imCC1101.CheckReceiveFlag();
	}
}
