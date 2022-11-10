#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 8000000UL

// MODES: FAST_PWM, TWO_PHASE
#define PWM_MODE_FAST_PWM
//#define PWM_MODE_TWO_PHASE

double dutyCycle = 50;

void initSerial()
{
    Serial.begin(9600);
}

void confPwmModeFastPwm()
{
    // Clear OC0A on compare match (non-inverting)
    TCCR0A = (1 << COM0A1);
    // Fast PWM: top: , update OCRA at BOTTOM, TOV flag set on MAX
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    // Enable timer/Counter0 compare match interrupt on overflow
    TIMSK0 = (1 << TOIE0);
}

void confPwmModeTwoPhase()
{
    // Clear OC0A on compare match (non-inverting), Set 0C0B on compare match (inverting)
    TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << COM0B0);
    // Phase Correct PWM: top: 0xFF, update OCRA and OCRB at TOP, TOV flag set on BOTTOM
    TCCR0A |= (1 << WGM00);
    // Enable timer/Counter0 compare match interrupt on overflow
}

void setClockPrescaler(int prescalerFactor)
{
    switch (prescalerFactor)
    {
    case 1:
        TCCR0B |= 0x1; // 1 (no prescaling)
    case 8:
        TCCR0B |= 0x2; // 8
    case 64:
        TCCR0B |= 0x3; // 64
    case 256:
        TCCR0B |= 0x4; // 256
    case 1024:
        TCCR0B |= 0x5; // 1024
    default:
        TCCR0B |= 0x1; // default: 1
    }
}

void initPortFastPwm()
{
    DDRD = (1 << PORTD6); // sets OC0A direction to output
}

void initPortTwoPhase()
{
    DDRD = (1 << PORTD6) | (1 << PORTD5); // sets OC0A and OC0B direction to output
}

int main()
{
    initSerial();
    #ifdef PWM_MODE_FAST_PWM
        initPortFastPwm();
        confPwmModeFastPwm();
        sei(); // enable interrupts
        setClockPrescaler(256);
        int direction = 1;
        while (1)
        {
            _delay_ms(5);
            if (direction == 1)
            {
                dutyCycle += 1;
            }
            else
            {
                dutyCycle -= 1;
            }
            if ((dutyCycle >= 100) || (dutyCycle <= 0))
            {
                if (direction == 1)
                {
                    direction = 0;
                }
                else
                {
                    direction = 1;
                }
            }
        }
    #endif
    
    #ifdef PWM_MODE_TWO_PHASE
        initPortTwoPhase();
        confPwmModeTwoPhase();
        sei(); // enable interrupts
        setClockPrescaler(256);

        int dutyCycleDiff = 1;

        OCR0A = ((dutyCycle - dutyCycleDiff) / 100.0) * 255;
        OCR0B = ((dutyCycle + dutyCycleDiff) / 100.0) * 255;
    #endif
}

ISR(TIMER0_OVF_vect)
{
    // This executes when timer/Counter0 compare match interrupt happens
    OCR0A = (dutyCycle / 100.0) * 255;
}