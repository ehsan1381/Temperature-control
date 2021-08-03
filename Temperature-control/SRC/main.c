// THIS CODE IS WRITTEN BY @ehsan1381

// PREPROCESSOR
#define atmega16a
#define _DRIVER_PORT                DDRD
#define _MAX_TEMP                   40
#define _ALERT_TEMP                 45
#define _MAX_MOTOR_FREQ             501
#define _MOTOR_FREQ_STEP            10
#define _COOL_TEMP_HIGH             25
#define _NORM_TEMP_LOW              25
#define _NORM_TEMP_HIGH             35
#define _WARM_TEMP_LOW              35
#define _WARM_TEMP_HIGH             40
#define _SENSOR_TEMP_EQ(ADC_VALUE)  (ADC_VALUE / 2.05)


// INCLUDE HEADERS
#include <mega16a.h>
#include <delay.h>
#include <alcd.h>
#include <stdio.h>

// FINCTION PROTOTYPES
void DEFINE_TEMP_STATE(int);
void WRITE_LCD(int, int, char);
void MOTOR_DRIVE_MAN();
void MOTOR_DRIVE_AUT(char, int);

// VARIABLES
char LINE0_STR[18];
char LINE1_STR[18];
int MOTOR_GEAR = 0;
char TEMP_STATE = 'N';
char DRIVE_STATE; 

int TEMP;

// TEMP STATE FUNC
void DEFINE_TEMP_STATE(int TEMP) {

    if (TEMP < _COOL_TEMP_HIGH) TEMP_STATE =  'C';
    if (_NORM_TEMP_LOW <= TEMP && TEMP < _NORM_TEMP_HIGH) TEMP_STATE =  'N';
    if (_WARM_TEMP_LOW <= TEMP && TEMP < _WARM_TEMP_HIGH) TEMP_STATE =  'W';
    if (_MAX_TEMP <= TEMP && TEMP <_ALERT_TEMP) TEMP_STATE =  'M';
    if (TEMP > _ALERT_TEMP) TEMP_STATE =  'A';     

}

// WRITE LCD FUNC
void WRITE_LCD(int temp, int motor_freq, char drive_state) {
    lcd_clear();   
    sprintf(LINE0_STR, "TEMP : %d C %c", temp, TEMP_STATE);
    sprintf(LINE1_STR, "MOTOR : %d Hz %c", motor_freq, drive_state);
    lcd_puts(LINE0_STR);
    lcd_gotoxy(0, 1);
    lcd_puts(LINE1_STR);
}


// MOTOR DRIVING MAN FUNC
void MOTOR_DRIVE_MAN() {
    DRIVE_STATE = 'M';
    if (PIND.7 == 1) {
        MOTOR_GEAR += 5;
        if ((MOTOR_GEAR * _MOTOR_FREQ_STEP) >= _MAX_MOTOR_FREQ) MOTOR_GEAR = 0;
    }

    if (PIND.6 == 0) {
        PORTD.0 = 0;
        PORTD.1 = 1;
    } 
    if (PIND.6 == 1) {
        PORTD.0 = 1;
        PORTD.1 = 0;
    }

}

// MOTOR DRIVING AUT FUNC
void MOTOR_DRIVE_AUT(char temp_state, int temp) {
    DRIVE_STATE = 'A';
    switch (temp_state) {      

    case 'C':
        MOTOR_GEAR = 5;
        break;

    case 'N':
        MOTOR_GEAR = temp / 2.5 + 10;
        break;

    case 'W':
        MOTOR_GEAR = temp / 2.5 + 25;
        break;

    case 'M':
        MOTOR_GEAR = temp / 2.5 + 20;
        break;

    case 'A':
        MOTOR_GEAR = 50; 
        
        break;     
        
    }     
    
    if (MOTOR_GEAR > 50) MOTOR_GEAR = 50;
    
        if (PIND.6 == 0) {
        PORTD.0 = 0;
        PORTD.1 = 1;
    } 
    if (PIND.6 == 1) {
        PORTD.0 = 1;
        PORTD.1 = 0;  
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
        TEMP = _SENSOR_TEMP_EQ(read_adc(0));
        DEFINE_TEMP_STATE(TEMP);
        
        
        if(PINA.1 == 1) MOTOR_DRIVE_AUT(TEMP_STATE, TEMP);
        if(PINA.1 == 0) MOTOR_DRIVE_MAN();

        WRITE_LCD(TEMP, MOTOR_GEAR * _MOTOR_FREQ_STEP, DRIVE_STATE);
                                       
        OCR1A = (MOTOR_GEAR * _MOTOR_FREQ_STEP);
        delay_ms(10);

    }
}
