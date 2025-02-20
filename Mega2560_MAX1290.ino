#include <TimerOne.h>
#include <digitalWriteFast.h>

#ifndef _NOP  // required for some non Arduino cores
#define _NOP() \
  do { __asm__ volatile("nop"); } while (0)
#endif

#define AD_HBEN 53
#define AD_D7 51
#define AD_D6 49
#define AD_D5 47
#define AD_D4 45
#define AD_D3_D11 43
#define AD_D2_D10 41
#define AD_D1_D9 39
#define AD_D0_D8 37
#define AD_INT 35
#define AD_RD 33
#define AD_WR 31
#define AD_CLK 29
#define AD_CS 27

#define VREF1 40
#define VREF0 26

#define LED 13

uint32_t millis1 = 0;

uint16_t data[8] = { 0 };

uint8_t vref_state = 0;

void AD_Clock(void) {
  digitalToggleFast(AD_CLK);
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  Serial.println("Start");

  pinModeFast(AD_HBEN, OUTPUT);
  pinModeFast(AD_D7, INPUT);
  pinModeFast(AD_D6, INPUT);
  pinModeFast(AD_D5, INPUT);
  pinModeFast(AD_D4, INPUT);
  pinModeFast(AD_D3_D11, INPUT);
  pinModeFast(AD_D2_D10, INPUT);
  pinModeFast(AD_D1_D9, INPUT);
  pinModeFast(AD_D0_D8, INPUT);
  pinModeFast(AD_INT, INPUT_PULLUP);
  pinModeFast(AD_RD, OUTPUT);
  pinModeFast(AD_WR, OUTPUT);
  pinModeFast(AD_CLK, OUTPUT);
  pinModeFast(AD_CS, OUTPUT);

  pinModeFast(VREF1, OUTPUT);
  pinModeFast(VREF0, OUTPUT);

  pinModeFast(LED, OUTPUT);

  digitalWriteFast(AD_HBEN, LOW);
  digitalWriteFast(AD_RD, HIGH);
  digitalWriteFast(AD_WR, HIGH);
  digitalWriteFast(AD_CLK, LOW);
  digitalWriteFast(AD_CS, HIGH);
  digitalWriteFast(VREF1, LOW);
  digitalWriteFast(VREF0, LOW);

  Timer1.initialize(10);
  Timer1.attachInterrupt(AD_Clock);  // 50kHz (20us) ---> Note: the recommended minimum is 100kHz
}

void loop() {
  // put your main code here, to run repeatedly:

  if ((millis() - millis1) >= 500) {
    millis1 = millis();

    digitalToggleFast(LED);

    read_adc();

    print_data();

    if (vref_state < 3) {
      vref_state++;
    } else {
      vref_state = 0;
    }

    if (vref_state == 0) {
      digitalWriteFast(VREF1, LOW);
      digitalWriteFast(VREF0, LOW);
    } else if (vref_state == 1) {
      digitalWriteFast(VREF1, LOW);
      digitalWriteFast(VREF0, HIGH);
    } else if (vref_state == 2) {
      digitalWriteFast(VREF1, HIGH);
      digitalWriteFast(VREF0, LOW);
    } else if (vref_state == 3) {
      digitalWriteFast(VREF1, HIGH);
      digitalWriteFast(VREF0, HIGH);
    }
  }
}

void read_adc(void) {
  uint16_t timeout = 0;
  uint16_t _data = 0;

  for (uint8_t i = 0; i < 8; i++) {
    digitalWriteFast(AD_CS, LOW);

    delayMicroseconds(1);

    digitalWriteFast(AD_WR, LOW);

    delayMicroseconds(1);

    set_output();

    delayMicroseconds(1);

    channel_select(i);  // ACQMOD = 0

    delayMicroseconds(1);

    digitalWriteFast(AD_WR, HIGH);

    delayMicroseconds(1);

    set_input();

    delayMicroseconds(1);

    digitalWriteFast(AD_CS, HIGH);

    delayMicroseconds(1);

    digitalWriteFast(AD_CS, LOW);

    delayMicroseconds(1);

    timeout = 1000;

    while (digitalReadFast(AD_INT) == HIGH) {
      delayMicroseconds(1);

      if (timeout > 0) {
        timeout--;
      } else {
        break;
      }
    }

    delayMicroseconds(1);

    if (timeout > 0) {
      digitalWriteFast(AD_RD, LOW);

      timeout = 1000;

      while (digitalReadFast(AD_INT) == LOW) {
        delayMicroseconds(1);

        if (timeout > 0) {
          timeout--;
        } else {
          break;
        }
      }

      delayMicroseconds(1);

      if (timeout > 0) {
        _data = get_data();

        data[i] = _data;

        delayMicroseconds(1);

        digitalWriteFast(AD_HBEN, HIGH);

        delayMicroseconds(1);

        _data = get_data() << 8;

        data[i] |= _data;

        digitalWriteFast(AD_HBEN, LOW);
      }

      delayMicroseconds(1);

      digitalWriteFast(AD_RD, HIGH);

      delayMicroseconds(1);

      digitalWriteFast(AD_CS, HIGH);

      delayMicroseconds(1);
    } else {
      data[i] = 9999;
    }
  }
}

void print_data(void) {
  for (uint8_t i = 0; i < 8; i++) {
    if (data[i] < 1000) Serial.print("0");
    if (data[i] < 100) Serial.print("0");
    if (data[i] < 10) Serial.print("0");

    Serial.print(data[i], DEC);

    if (i == 7) {
      Serial.println();
    } else {
      Serial.print(" ");
    }
  }
}

void set_output(void) {
  pinModeFast(AD_D7, OUTPUT);
  pinModeFast(AD_D6, OUTPUT);
  pinModeFast(AD_D5, OUTPUT);
  pinModeFast(AD_D4, OUTPUT);
  pinModeFast(AD_D3_D11, OUTPUT);
  pinModeFast(AD_D2_D10, OUTPUT);
  pinModeFast(AD_D1_D9, OUTPUT);
  pinModeFast(AD_D0_D8, OUTPUT);
}

void set_input(void) {
  pinModeFast(AD_D7, INPUT_PULLUP);
  pinModeFast(AD_D6, INPUT_PULLUP);
  pinModeFast(AD_D5, INPUT_PULLUP);
  pinModeFast(AD_D4, INPUT_PULLUP);
  pinModeFast(AD_D3_D11, INPUT_PULLUP);
  pinModeFast(AD_D2_D10, INPUT_PULLUP);
  pinModeFast(AD_D1_D9, INPUT_PULLUP);
  pinModeFast(AD_D0_D8, INPUT_PULLUP);
}

// Control Byte Format: PD1, PD0, ACQMOD, SGL/DIF, UNI/BIP, A2, A1, A0
// PD1 = 1, PD0 = 1: Normal Operation Mode. External clock mode is selected
// ACQMOD = 0: Internal Acquisition Mode
// SGL/DIF = 1: Single-Ended Analog Input Mode
// UNI/BIP = 1: Unipolar Mode
void channel_select(uint8_t data) {
  digitalWriteFast(AD_D7, HIGH);                 // PD1 = 1: Normal Operation Mode. External clock mode is selected
  digitalWriteFast(AD_D6, HIGH);                 // PD0 = 0: Normal Operation Mode. External clock mode is selected
  digitalWriteFast(AD_D5, LOW);                  // ACQMOD = 0: Internal Acquisition Mode
  digitalWriteFast(AD_D4, HIGH);                 // SGL/DIF = 1: Single-Ended Analog Input Mode
  digitalWriteFast(AD_D3_D11, HIGH);             // UNI/BIP = 1: Unipolar Mode
  digitalWriteFast(AD_D2_D10, (data >> 2) & 1);  // A2
  digitalWriteFast(AD_D1_D9, (data >> 1) & 1);   // A1
  digitalWriteFast(AD_D0_D8, data & 1);          // A0
}

uint8_t get_data(void) {
  uint8_t _data = 0;

  _data = digitalReadFast(AD_D7) << 7;
  _data |= digitalReadFast(AD_D6) << 6;
  _data |= digitalReadFast(AD_D5) << 5;
  _data |= digitalReadFast(AD_D4) << 4;
  _data |= digitalReadFast(AD_D3_D11) << 3;
  _data |= digitalReadFast(AD_D2_D10) << 2;
  _data |= digitalReadFast(AD_D1_D9) << 1;
  _data |= digitalReadFast(AD_D0_D8);

  return _data;
}
