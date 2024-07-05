#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

int input, _input, inputTime;
short initPos = 4;
int sp = -1;
int time[3] = {0, 0, 0};

int Buzzer_Pin = 3; // Change this to pin number that's connected to buzzer

int evalInput() {
  int input = analogRead(0);

  if (input >= 0 && input < 60)
    return 4;

  if (input >= 60 && input < 160)
    return 2;

  if (input >= 340 && input < 390)
    return 3;

  if (input >= 410 && input < 540)
    return 1;

  if (input >= 610 && input <= 750)
    return 0;
  
  return -1;
}

void processInput() {
  int tmp;

  _input = input;
  input = evalInput(); // Reset timer and process input

  if (_input == input) {
    inputTime++;
  } else {
    inputTime = 0;
  }

  delay(200);
}

void drawTimeUnit(int type, bool empty) {
  int t = time[type];

  lcd.setCursor(initPos + type * 3, 0);

  if (!empty) {
    if (t < 10)
      lcd.print("0" + String(time[type]));
    else
      lcd.print(String(time[type]));
  } else {
    lcd.print("  ");
  }
}

void timerEnd() {
  int i = 0;
  lcd.setCursor(3, 1);
  lcd.print("Time's up!");

  while (evalInput() == -1) { // Blink display until any key is pressed
    i++;
    delay(500);

    switch (i) {
      case 2: {
        i = 0;
        digitalWrite(Buzzer_Pin, LOW);
        lcd.display();
        break;
      }
      case 1: {
        digitalWrite(Buzzer_Pin, HIGH);
        lcd.noDisplay();
        break;
      }
    }
  }

  lcd.display();
  lcd.setCursor(0, 1);
  lcd.print("             ");
}

void drawTime() {
  drawTimeUnit(0, false);

  lcd.setCursor(6, 0);
  lcd.print(":");

  drawTimeUnit(1, false);

  lcd.setCursor(9, 0);
  lcd.print(":");

  drawTimeUnit(2, false);
}

void timer() {
  if (time[0] == 0 && 
      time[1] == 0 &&
      time[2] == 0) return; // Stop timer if time hasn't been set

  for (;;) {
    drawTime(); // Render current time

    if (time[2] == 0) {
      if (time[1] == 0) {
        if (time[0] == 0) {
          timerEnd(); // Timer end
          break;
        }
        else {
          time[0] -= 1;
          time[1] = 59;
          time[2] = 59; // If minute and second is 0, decrement 1 hour and set them to 59
        }
      } else {
        time[1] -= 1;
        time[2] = 59; // If second is 0, decrement 1 minute and set second to 59
      }
    } else {
      time[2] -= 1; // Decrement second
    }

    delay(1000); // Wait 1s
  }
}

void setTime(void) {
  int i = 0;
  int threshold;

  for (sp=0; sp<3; sp++) {
    threshold = sp == 0 ? 23 : 59;

    while (evalInput() != 0 && inputTime == 0) {
      i++;
      delay(100);

      if (i == 10) {
        i = 0;
        drawTimeUnit(sp, false); // Blink time unit every 0.5 seconds
      }

      if (i == 5)
        drawTimeUnit(sp, true);

      if (evalInput() == 2) { // Up key
        time[sp] += 1; // Increment time

        if (time[sp] >= threshold) time[sp] = 0; // If time is greater than allowed value, revert it
        drawTimeUnit(sp, false);
      }

      if (evalInput() == 3) { // Down key
        time[sp] -= 1; // Decrement time

        if (time[sp] < 0) time[sp] = threshold; // If time is minus, wrap back to highest allowed value
        drawTimeUnit(sp, false);
      }
    }
    drawTimeUnit(sp, false);
    delay(100);
  }
  sp = -1;

  return;
}

void setup(void) {
  lcd.begin(16, 2); // Begin LCD
  lcd.setCursor(initPos, 0); // Reset cursor
  drawTime();

  pinMode(Buzzer_Pin, OUTPUT); // Initialize buzzer

  for (;;) { // User Interface task
    while (evalInput() != 0) continue; // Hang while SELECT pin is not pressed
    delay(300);

    setTime();
    timer();
  }
}

void loop(void) {
  delay(150);
  processInput(); // Input task
}
