#include "Laser.h"

#define START 's'
#define EOF 'e'

Laser laser(5);
unsigned short pointsBuffer[300] = {
    0x17c,
    0x3e8,
    0x8000,
    0x0,
    0x17c,
    0x3e8,
    0x82f9,
    0x0,
    0x8e,
    0x14d,
    0x826b,
    0x14d,
};
boolean onRoutine;
int idx;
int idxLimit;

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600);
    laser.init();
    onRoutine = true;
    idx = 0;
    idxLimit = 12;
}

void loop()
{
    if (onRoutine)
    {
        digitalWrite(LED_BUILTIN, LOW);
        if (Serial1.available() > 0)
        {
            // Serial1.writeln('read start');
            if (Serial1.read() == START)
            {
                onRoutine = false;
                idx = 0;
            }
        }
        else
        {
            unsigned short posX = pointsBuffer[idx];
            idx++;
            unsigned short posY = pointsBuffer[idx];
            idx++;

            laser.sendto(posX & 0x7fff, posY & 0xfff);
            // Serial1.write("R / X: ");
            // Serial1.write(posX & 0xfff);
            // Serial1.write("Y: ");
            // Serial1.writeln(posY & 0xfff);

            if (idx == idxLimit)
                idx = 0;
        }
    }

    if (!onRoutine && Serial1.available() >= 4)
    {
        unsigned short tmpInput;
        char initialInput = Serial1.read();
        if (initialInput == EOF || idx >= idxLimit)
        {
            onRoutine = true;
            idxLimit = idx - 1;
            idx = 0;
        }
        else
        {
            tmpInput = initialInput << 8;
            tmpInput += Serial1.read();
            pointsBuffer[idx] = tmpInput;
            idx++;
            tmpInput = Serial1.read() << 8;
            tmpInput += Serial1.read();
            pointsBuffer[idx] = tmpInput;
            idx++;

            unsigned short posX = pointsBuffer[idx - 2];
            unsigned short posY = pointsBuffer[idx - 1];
            laser.sendto(posX & 0x7fff, posY & 0xfff);
            Serial.write("U / X: ");
            Serial.write(posX & 0xfff);
            Serial.write("Y: ");
            Serial.write(posY & 0xfff);
            Serial.write("\n");
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
}