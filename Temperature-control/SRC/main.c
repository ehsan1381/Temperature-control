// THIS CODE IS WRITTEN BY @ehsan1381

// PREPROCESSOR
#define atmega16
#define _LCD_PORT                   PORTB
#define _DRIVER_PORT                DDRD
#define _MAX_TEMP                   40
#define _ALERT_TEMP                 45
#define _MAX_MOTOR_FREQ             40000
#define _MOTOR_FREQ_STEP            5000
#define _NUMBER_OF_MOTORS           1
#define _COOL_TEMP_HIGH             25
#define _NORM_TEMP_LOW              25
#define _NORM_TEMP_HIGH             35
#define _WARM_TEMP_LOW              35
#define _WARM_TEMP_HIGH             40
#define _SENSOR_TEMP_EQ(ADC_VALUE)  (ADC_VALUE / 10)


// INCLUDE HEADERS
#include <mega16a.h>
#include <delay.h>
#include <alcd.h>
#include <stdio.h>

// FINCTION PROTOTYPES
char DEFINE_TEMP_STATE(int);
void WRITE_LCD(int, int);
void MOTOR_DRIVE_MAN();
void MOTOR_DRIVE_AUT(char);

// VARIABLES
char LINE0_STR[18];
char LINE1_STR[18];
int MOTOR_GEAR = 0;
char TEMP_STATE = 'N';
char CONTROL_TYPE = 1;
int TEMP = 0;

// TEMP STATE FUNC
char DEFINE_TEMP_STATE(int TEMP) {
    if (TEMP < _COOL_TEMP_HIGH) return 'C';
    if (_NORM_TEMP_LOW <= TEMP < _NORM_TEMP_HIGH) return 'N';
    if (_WARM_TEMP_LOW <= TEMP < _WARM_TEMP_HIGH) return 'W';
    if (_MAX_TEMP <= TEMP <_ALERT_TEMP) return 'M';
    if (TEMP > _ALERT_TEMP) return 'A';
}

// WRITE LCD FUNC
void WRITE_LCD(int temp, int motor_gear) {
    lcd_clear();
    sprintf(LINE0_STR, "TEMP : %d", temp);
    sprintf(LINE1_STR, "MOTOR : %d", motor_gear);
    lcd_puts(LINE0_STR);
    lcd_gotoxy(0, 1);
    lcd_puts(LINE1_STR);
}


// MOTOR DRIVING MAN FUNC
void MOTOR_DRIVE_MAN() {
    if (PIND.7 == 0) {
        MOTOR_GEAR++;
        if ((MOTOR_GEAR * _MOTOR_FREQ_STEP) >= _MAX_MOTOR_FREQ) MOTOR_GEAR = 0;
    }

    if (PIND.6 == 0) {
        PORTD.0 = 0;
        PORTD.1 = 1;
    }

    else {
        PORTD.0 = 1;
        PORTD.1 = 0;
    }

}

// MOTOR DRIVING AUT FUNC
void MOTOR_DRIVE_AUT(char temp_state) {
    switch (temp_state) {
    case 'C':
        MOTOR_GEAR = 0;
        break;

    case 'N':
        MOTOR_GEAR = 1;
        break;

    case 'W':
        MOTOR_GEAR = 4;
        break;

    case 'M':
        MOTOR_GEAR = 6;
        break;

    case 'A':
        MOTOR_GEAR = 8;
        break;
    }
}

// LCD CONFIGURATION

// IO SETTINGS




#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))


unsigned int read_adc(unsigned char adc_input)
{
    ADMUX = adc_input | ADC_VREF_TYPE;

    delay_us(10);

    ADCSRA |= (1 << ADSC);

    while ((ADCSRA & (1 << ADIF)) == 0);
    ADCSRA |= (1 << ADIF);
    return ADCW;
}

void main(void)
{
    
_DRIVER_PORT = 0x33;

TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
TCCR1B = (1 << CS10);


ADMUX = ADC_VREF_TYPE;
ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADATE) | (0 << ADIF) | (0 << ADIE) | (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);

lcd_init(16);

    while (1)
    {
        CONTROL_TYPE = PINA.2;
        TEMP = read_adc(0);
        TEMP_STATE = DEFINE_TEMP_STATE(TEMP);
        if (CONTROL_TYPE) MOTOR_DRIVE_AUT(TEMP_STATE);
        if (!CONTROL_TYPE) MOTOR_DRIVE_MAN();
        OCR1A = MOTOR_GEAR * _MOTOR_FREQ_STEP;
        delay_ms(100);

    }
}
