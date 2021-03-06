#include "Laser.h"

#define START 0xff
#define END 0xfe

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
int waitCnt;
char buf[100];
int bufLength;
int bufIdx;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    laser.init();
    laser.setScale(1);
    laser.setOffset(0, 0);
    onRoutine = true;
    idx = 0;
    waitCnt = 0;
    idxLimit = 12;
    bufLength = 0;
}

void loop()
{
    if (onRoutine)
    {
        digitalWrite(LED_BUILTIN, LOW);
        if (Serial.available() > 0)
        {
            if (Serial.read() == START)
                switchToNoRoutine();
        }
        else
        {
            unsigned short posX = pointsBuffer[idx];
            idx++;
            unsigned short posY = pointsBuffer[idx];
            idx++;
            Serial.write('R');
            Serial.print(idx);
            Serial.write(',');
            Serial.print(posX & 0xfff);
            Serial.write(',');
            Serial.print(posY);
            Serial.write('\n');

            laser.sendto(posX & 0xfff, posY & 0xfff);
            if (idx >= idxLimit)
                idx = 0;
        }
    }

    else
    {
        digitalWrite(LED_BUILTIN, HIGH);
        //if (idx >= 300 || waitCnt >= 100)
        if (idx >= 300 || waitCnt >= 10000)
            switchToRoutine();

        char initialInput;
        if (Serial.available() > 0)
        {
            initialInput = Serial.read();
            if (initialInput == END)
                switchToRoutine();
            else
            {
                buf[getNextBufIdx()] = initialInput;
                bufLength++;
            }

            if ((Serial.available() + bufLength) >= 4)
            {
                unsigned short tmpInput;
                tmpInput = (static_cast<unsigned short>(popFromSerial())) << 8;
                tmpInput |= static_cast<unsigned short>(popFromSerial()) & 0x00ff;
                unsigned short posX = tmpInput;
                tmpInput = (static_cast<unsigned short>(popFromSerial())) << 8;
                tmpInput |= static_cast<unsigned short>(popFromSerial()) & 0x00ff;
                unsigned short posY = tmpInput;

                bool isValidated = validateXY(posX, posY);
                if (!isValidated)
                {
                    if (validateXY(posY, posX))
                    {
                        isValidated = true;
                        tmpInput = posX;
                        posX = posY;
                        posY = tmpInput;
                    }
                }

                if (isValidated)
                {
                    Serial.write('U');
                    Serial.print(idx);
                    Serial.print(posX);
                    Serial.write(',');
                    Serial.print(posY);
                    Serial.write('\n');
                    pointsBuffer[idx] = posX;
                    pointsBuffer[idx + 1] = posY;
                    idx += 2;

                    laser.sendto(posX & 0xfff, posY & 0xfff);
                }
            }
            waitCnt = 0;
        }
        else
        {
            waitCnt++;
        }
    }

    // delay(100);
}

void switchToNoRoutine()
{
    onRoutine = false;
    idx = 0;
    waitCnt = 0;
    bufLength = 0;
    bufIdx = 0;
}

void switchToRoutine()
{

    // flush buffer
    while (Serial.available())
    {
        Serial.read();
    }
    // unsigned short tmpInput;
    // while (idx < 298 && bufLength >= 4)
    // {
    //     tmpInput = (static_cast<unsigned short>(buf[bufIdx])) << 8;
    //     updateBufIdx();
    //     tmpInput |= static_cast<unsigned short>(buf[bufIdx]) & 0x00ff;
    //     updateBufIdx();
    //     pointsBuffer[idx] = tmpInput;
    //     idx++;
    //     tmpInput = (static_cast<unsigned short>(buf[bufIdx])) << 8;
    //     updateBufIdx();
    //     tmpInput |= static_cast<unsigned short>(buf[bufIdx]) & 0x00ff;
    //     updateBufIdx();
    //     pointsBuffer[idx] = tmpInput;
    //     idx++;
    //     bufLength -= 4;
    // }

    onRoutine = true;
    idxLimit = idx;
    idx = 0;
    waitCnt = 0;
    bufLength = 0;
}

char popFromSerial()
{
    if (bufLength > 0)
    {
        char retVal = buf[bufIdx];
        updateBufIdx();
        bufLength--;

        return retVal;
    }
    else
    {
        return Serial.read();
    }
}

void updateBufIdx()
{
    if (bufIdx < 99)
        bufIdx++;
    else
        bufIdx = 0;
}

int getNextBufIdx()
{
    if ((bufIdx + bufLength) < 100)
    {
        return bufIdx + bufLength;
    }

    return (bufIdx + bufLength) % 100;
}

boolean validateXY(unsigned short x, unsigned short y)
{
    // return true;
    // return ((x & (0b0111 >> 12)) == 0) && (y & (0b1111 >> 12) == 0);
    return ((x & 0x7000) == 0) && ((y & 0xf000) == 0);
}