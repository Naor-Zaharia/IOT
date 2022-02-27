// Mozzi based musical challenge
// Naor Zaharia 312423841, Harel Zahari 305494452

/*The instrument we made hold`s 3 diffrent sound genrators.
We based our implementation on 3 sensors:
The light sensor was used to simulate a theremin based on the light sensor.
The left and right buttons were used to offer control over playing a beat for tempo.
The cable clips were used to conduct current from the drum drawings to the board to simulate the synthesizer effects.
*/

#include <Adafruit_CircuitPlayground.h>
#include <MozziGuts.h>
#include <Oscil.h>
#include <Line.h>
#include <mozzi_midi.h>
#include <RollingAverage.h>
#include <ControlDelay.h>
#include <tables/sin8192_int8.h>
#include <tables/sin2048_int8.h>

// Consts
#define LIGHTTHRESHOLD 500
#define KEYTHRESHOLD 700
#define AMOUNTKEYS 5
#define AMOUNTBEATS 2
#define LIGHTVOL 2
#define TOTALVOL 6
#define FIRSTBEATMOD 8
#define SECONDBEATMOD 12
#define MAXCOLORVALUE 255
#define MINCOLORVALUE 0
#define NUMOFLEDS 10
#define BEATMOD 10
#define BEATVOL 100

Oscil<8192, AUDIO_RATE> aOscilBeat1(SIN8192_DATA);
Oscil<8192, AUDIO_RATE> aOscilBeat2(SIN8192_DATA);
Oscil<8192, AUDIO_RATE> aOscilKey1(SIN8192_DATA);
Oscil<8192, AUDIO_RATE> aOscilKey2(SIN8192_DATA);
Oscil<8192, AUDIO_RATE> aOscilKey3(SIN8192_DATA);
Oscil<8192, AUDIO_RATE> aOscilKey4(SIN8192_DATA);
Oscil<8192, AUDIO_RATE> aOscilKey5(SIN8192_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscilLightSin0(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscilLightSin1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> aOscilLightSin2(SIN2048_DATA);
Oscil<8192, AUDIO_RATE> oscilKeysArray[5] = {aOscilKey1, aOscilKey2, aOscilKey3, aOscilKey4, aOscilKey5};
Oscil<8192, AUDIO_RATE> oscilBeatArray[2] = {aOscilBeat1, aOscilBeat2};
RollingAverage<int, 32> rollingAverage;
ControlDelay<64, int> mozziControlDelay;

bool leftButtonPressed;
bool rightButtonPressed;
bool isBeatActive;
bool isInstrumentOn;
int settingsRoundKeysLoopLengthArray[AMOUNTKEYS] = {1, 2, 2, 1, 1};
int settingsBeatLoopLengthArray[AMOUNTBEATS] = {1, 4};
int ledRedColor = 0;
int ledGreenColor = 0;
int lightSensor;
unsigned int echo1 = 128;
unsigned int echo2 = 256;
int noteArray[AMOUNTKEYS] = {600, 1500, 3000, 4000, 5000};
int beatNoteArray[AMOUNTBEATS] = {800, 800};
int roundKeysLoopLengthArray[AMOUNTKEYS] = {0, 0, 0, 0, 0};
int roundBeatLoopLengthArray[AMOUNTBEATS] = {0, 0};
int beatCounter = 0;
byte capsensePins[AMOUNTKEYS] = {A1, A2, A3, A4, A5};

// Entry point
void setup()
{
    Serial.begin(115200);
    CircuitPlayground.begin();
    startMozzi();
}

// Main board loop
void loop()
{
    if (checkIfOn())
    {
        audioHook();
    }
}

// Update device controls status
void updateControl()
{
    lightSensor = CircuitPlayground.lightSensor();
    leftButtonPressed = CircuitPlayground.leftButton();
    rightButtonPressed = CircuitPlayground.rightButton();
    if (rightButtonPressed == true)
    {
        isBeatActive = true;
    }

    if (leftButtonPressed == true)
    {
        isBeatActive = false;
    }

    int currentAverage = rollingAverage.next(lightSensor);

    aOscilLightSin0.setFreq(currentAverage * 5);
    aOscilLightSin1.setFreq(mozziControlDelay.next(echo1));
    aOscilLightSin2.setFreq(mozziControlDelay.next(echo2));
    updateKeysLoopLengthArray();
    updateBeatLoopLengthArray();
    checkForKeyToStart();

    if (isBeatActive)
    {
        startBeatActive();
    }
    else
    {
        stopBeatActive();
    }
}

// Update device audio
int updateAudio()
{
    int result = 0;

    if (lightSensor >= LIGHTTHRESHOLD)
    {
        result = LIGHTVOL * ((int)aOscilLightSin0.next() + aOscilLightSin1.next() + (aOscilLightSin2.next()));
    }

    result = result + TOTALVOL * getActiveKeysOscilSum() + getActiveBeatOscilSum();

    return result;
}

// Update Oscils for key notes
void checkForKeyToStart()
{
    for (int i = 0; i < AMOUNTKEYS; i++)
    {
        if (roundKeysLoopLengthArray[i] < 0)
        {
            oscilKeysArray[i].setFreq(0);
            updateKeysOscilNote(&oscilKeysArray[i], i);
        }
    }
}

// Update key's loop counter for length
void updateKeysLoopLengthArray()
{
    for (int i = 0; i < AMOUNTKEYS; i++)
    {
        roundKeysLoopLengthArray[i] = roundKeysLoopLengthArray[i] - 1;
    }
}

// Update beats loop counter for length
void updateBeatLoopLengthArray()
{
    for (int i = 0; i < AMOUNTBEATS; i++)
    {
        roundBeatLoopLengthArray[i] = roundBeatLoopLengthArray[i] - 1;
    }
}

// Create the oscils sum of the active keys
int getActiveKeysOscilSum()
{
    int result = 0;
    for (int i = 0; i < AMOUNTKEYS; i++)
    {
        result = result + (AMOUNTKEYS - i) * (oscilKeysArray[i].next());
    }

    return result;
}

// Create the oscils sum of the active beats
int getActiveBeatOscilSum()
{
    int result = 0;
    result = result + BEATVOL * (oscilBeatArray[0].next());
    if (beatCounter % BEATMOD == 0)
    {
        result = result + BEATVOL * (oscilBeatArray[1].next());
    }
    return result;
}

// The method create the On/Off led effect
void ledOnOffSwitch(bool isOn)
{
    if (!isOn)
    {
        ledRedColor = MAXCOLORVALUE;
        ledGreenColor = MINCOLORVALUE;
    }
    else
    {
        ledRedColor = MINCOLORVALUE;
        ledGreenColor = MAXCOLORVALUE;
    }

    for (int i = 0; i < NUMOFLEDS; i++)
    {
        delay(200);
        CircuitPlayground.setPixelColor(i, ledRedColor, ledGreenColor, 0);
    }

    CircuitPlayground.clearPixels();
}

// The method checks if the instrument is On or Off, and activates the On/Off led effect
bool checkIfOn()
{
    if (isInstrumentOn != CircuitPlayground.slideSwitch())
    {
        isInstrumentOn = CircuitPlayground.slideSwitch();

        if (isInstrumentOn)
        {
            ledOnOffSwitch(true);
        }
        else
        {
            ledOnOffSwitch(false);
        }
    }
}

// The method update the relevant key oscil Freq
void updateKeysOscilNote(Oscil<8192, AUDIO_RATE> *oscil, int index)
{
    int currentKey = CircuitPlayground.readCap(capsensePins[index]);
    if (currentKey > KEYTHRESHOLD && roundKeysLoopLengthArray[index] < 0)
    {
        roundKeysLoopLengthArray[index] = settingsRoundKeysLoopLengthArray[index];
        int currentFreq = noteArray[index];
        oscil->setFreq(currentFreq);
    }
}

// The method start the beat routine
void startBeatActive()
{
    beatCounter++;
    for (int i = 0; i < AMOUNTBEATS; i++)
    {
        oscilBeatArray[i].setFreq(0);
        if (roundBeatLoopLengthArray[i] <= 0)
        {
            if ((beatCounter % FIRSTBEATMOD == 0 && i == 0))
            {
                roundBeatLoopLengthArray[i] = settingsBeatLoopLengthArray[i];
                oscilBeatArray[i].setFreq(0);
            }
            if (beatCounter % SECONDBEATMOD == 0 && i == 1)
            {
                roundBeatLoopLengthArray[i] = settingsBeatLoopLengthArray[i];
                oscilBeatArray[i].setFreq(0);
            }
        }

        if (roundBeatLoopLengthArray[i] > 0)
        {
            oscilBeatArray[i].setFreq(beatNoteArray[i]);
            roundBeatLoopLengthArray[i]--;
        }
    }
}

// The method stop the beat routine
void stopBeatActive()
{
    for (int i = 0; i < AMOUNTBEATS; i++)
    {
        oscilBeatArray[i].setFreq(0);
    }
}
