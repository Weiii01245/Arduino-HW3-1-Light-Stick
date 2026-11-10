const int buttonPin = 2;
const int RLedPin = 9;
const int GLedPin = 10;
const int BLedPin = 11;

int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; 

bool buttonPressing = false;
unsigned long pressingStartTime = 0;
const unsigned long longPressInterval = 1000; 

int lightNum = 0;
const int colorTable[5][3] = {
  {0,   0,   0  },  
  {0,   255, 255},  
  {255, 0,   255},  
  {255, 255, 0  },  
  {0,   0,   255}    
};
int RLightColor = 0, GLightColor = 0, BLightColor = 0;
int RCurrentColor = 0, GCurrentColor = 0, BCurrentColor = 0;

const int MODE_COUNT = 5;
int currentMode = 0;

unsigned long blinkTimer = 0;
const int blinkInterval = 500;
bool blinkOn = true;

const int fadeAmount = 3;
const int fadeStepInterval = 10;
unsigned long fadeTimer = 0;
int brightness = 0;      
int fadeDirection = 1;

unsigned long rainbowTimer = 0;
const int rainbowStepInterval = 20;
int hue = 0;             

const int sosUnit = 200;
const int sosPattern[] = {1,1, 1,1, 1,3,    
                          3,1, 3,1, 3,3,    
                          1,1, 1,1, 1,7};   
const int sosLength = sizeof(sosPattern) / sizeof(sosPattern[0]);
int sosIndex = 0;
unsigned long sosTimer = 0;

void setup()
{
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(RLedPin, OUTPUT);
  pinMode(GLedPin, OUTPUT);
  pinMode(BLedPin, OUTPUT);
  Serial.begin(9600);   
}

void loop()
{
  checkButton();
  updateLEDColor();
  setRGBLEDColor(RCurrentColor, GCurrentColor, BCurrentColor);
}

void checkButton()
{
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      
      if (buttonState == HIGH) { 
        pressingStartTime = millis();
        buttonPressing = true;
      } 
      else {
        if (buttonPressing) {
          unsigned long pressDuration = millis() - pressingStartTime;
          if (pressDuration < longPressInterval) {
            Serial.println("short click");
            changeLEDColor();
          } else {
            Serial.println("long press");
            changeMode();
          }
          buttonPressing = false;
        }
      }
    }
  }

  lastButtonState = reading;
}

void changeLEDColor()
{
  lightNum = lightNum + 1;
  if (lightNum >= 5) lightNum = 0;

  RLightColor = colorTable[lightNum][0];
  GLightColor = colorTable[lightNum][1];
  BLightColor = colorTable[lightNum][2];

  RCurrentColor = RLightColor;
  GCurrentColor = GLightColor;
  BCurrentColor = BLightColor;
}

void changeMode()
{
  currentMode = currentMode + 1;
  if (currentMode >= MODE_COUNT) {
    currentMode = 0;
  }

  blinkTimer = 0;
  blinkOn = true;

  brightness = 0;
  fadeDirection = 1;
  fadeTimer = millis();

  hue = 0;
  rainbowTimer = millis();

  sosIndex = 0;
  sosTimer = millis();

  Serial.print("mode = ");
  Serial.println(currentMode);
}

void updateLEDColor()
{
  unsigned long currentTime = millis();

  if (currentMode == 0) {
    RCurrentColor = RLightColor;
    GCurrentColor = GLightColor;
    BCurrentColor = BLightColor;
  }
  else if (currentMode == 1) {
    if (currentTime - blinkTimer > blinkInterval) {
      blinkOn = !blinkOn;
      blinkTimer = currentTime;
    }
    if (blinkOn) {
      RCurrentColor = RLightColor;
      GCurrentColor = GLightColor;
      BCurrentColor = BLightColor;
    }
    else {
      RCurrentColor = 255;
      GCurrentColor = 255;
      BCurrentColor = 255;
    }
  }
  else if (currentMode == 2) {
    if (currentTime - fadeTimer >= fadeStepInterval) {
      fadeTimer = currentTime;
      brightness = brightness + fadeDirection * fadeAmount;
      if (brightness >= 255) { brightness = 255; fadeDirection = -1; }
      if (brightness <= 0)   { brightness = 0;   fadeDirection = 1;  }
    }
    RCurrentColor = dimColor(RLightColor, brightness);
    GCurrentColor = dimColor(GLightColor, brightness);
    BCurrentColor = dimColor(BLightColor, brightness);
  }
  else if (currentMode == 3) {
    if (currentTime - rainbowTimer >= rainbowStepInterval) {
      rainbowTimer = currentTime;
      hue = hue + 2;
      if (hue >= 360) hue = 0;
    }
    rainbowColor(hue);
  }
  else if (currentMode == 4) {
    if (currentTime - sosTimer >= (unsigned long)sosPattern[sosIndex] * sosUnit) {
      sosTimer = currentTime;
      sosIndex = sosIndex + 1;
      if (sosIndex >= sosLength) sosIndex = 0;
    }
    if (sosIndex % 2 == 0) {
      RCurrentColor = RLightColor;
      GCurrentColor = GLightColor;
      BCurrentColor = BLightColor;
    }
    else {
      RCurrentColor = 255;
      GCurrentColor = 255;
      BCurrentColor = 255;
    }
  }
}

int dimColor(int value, int b)
{
  return 255 - (int)((long)(255 - value) * b / 255);
}

void rainbowColor(int h)
{
  int sector = h / 60;
  int f = (h % 60) * 255 / 60;
  int r, g, b;
  switch (sector) {
    case 0:  r = 255;     g = f;      b = 0;      break;
    case 1:  r = 255 - f; g = 255;    b = 0;      break;
    case 2:  r = 0;       g = 255;    b = f;      break;
    case 3:  r = 0;       g = 255 - f; b = 255;    break;
    case 4:  r = f;       g = 0;      b = 255;    break;
    default: r = 255;     g = 0;      b = 255 - f; break;
  }
  RCurrentColor = 255 - r;
  GCurrentColor = 255 - g;
  BCurrentColor = 255 - b;
}

void setRGBLEDColor(int r, int g, int b)
{
  analogWrite(RLedPin, r);
  analogWrite(GLedPin, g);
  analogWrite(BLedPin, b);
}