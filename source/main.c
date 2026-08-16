#include <stdint.h>
#include <stdbool.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/i2c.h"


/*
 * ==========================================================
 * SYSTEM CONSTANTS
 * ==========================================================
 *
 * Thresholds, calibration limits, sampling parameters, and
 * hardware mappings used by the irrigation controller.
 */

#define MOISTURE_LOW_THRESHOLD        35U
#define MOISTURE_HIGH_THRESHOLD       50U

#define MAX_PUMP_RUNTIME              20U

#define ADC_SAMPLE_COUNT              16U

#define DRY_CONFIRM_COUNT              3U
#define WET_CONFIRM_COUNT              3U

#define SOIL_DRY_VALUE              4090U
#define SOIL_WET_VALUE              1650U

#define LIGHT_DARK_VALUE             690U
#define LIGHT_BRIGHT_VALUE          3912U

#define OLED_ADDRESS                0x3CU

#define DHT_PORT             GPIO_PORTA_BASE
#define DHT_PIN                  GPIO_PIN_2

#define RELAY_PORT           GPIO_PORTB_BASE
#define RELAY_PIN                 GPIO_PIN_0

/*
 * LaunchPad SW1 is connected to PF4.
 *
 * The internal pull-up keeps the input HIGH while the button is
 * released. Pressing SW1 pulls PF4 LOW, so button logic is active-low.
 */
#define BUTTON_PORT          GPIO_PORTF_BASE
#define BUTTON_PIN                GPIO_PIN_4

#define DHT_READ_INTERVAL_SECONDS   2U
#define DHT_MAX_RETRIES             3U


/*
 * ==========================================================
 * GLOBAL STATE
 * ==========================================================
 *
 * Set by the Timer0A interrupt and consumed by the main loop.
 */

volatile bool timer_flag = false;


/*
 * ==========================================================
 * UART TEXT OUTPUT
 * ==========================================================
 *
 * Sends a null-terminated string through UART0 one character at a time.
 */

void UART_SendText(const char *text)
{
    while (*text != '\0')
    {
        UARTCharPut(UART0_BASE, *text);
        text++;
    }
}


/*
 * ==========================================================
 * UART INTEGER OUTPUT
 * ==========================================================
 *
 * Converts an unsigned integer to decimal characters without using
 * printf, keeping the serial output code small and predictable.
 */

void UART_SendNumber(uint32_t number)
{
    char buffer[10];
    int i = 0;

    if (number == 0U)
    {
        UARTCharPut(UART0_BASE, '0');
        return;
    }

    while (number > 0U)
    {
        buffer[i++] = (char)((number % 10U) + '0');
        number /= 10U;
    }

    while (i > 0)
    {
        UARTCharPut(UART0_BASE, buffer[--i]);
    }
}


/*
 * ==========================================================
 * SOIL MOISTURE CONVERSION
 * ==========================================================
 *
 * Maps the calibrated ADC range to 0-100%. Values outside the measured
 * dry/wet limits are clamped so the displayed percentage stays valid.
 */

uint32_t SoilMoisturePercent(uint32_t adc_value)
{
    if (adc_value >= SOIL_DRY_VALUE)
    {
        return 0U;
    }

    if (adc_value <= SOIL_WET_VALUE)
    {
        return 100U;
    }

    return ((SOIL_DRY_VALUE - adc_value) * 100U)
           / (SOIL_DRY_VALUE - SOIL_WET_VALUE);
}


/*
 * ==========================================================
 * LIGHT LEVEL CONVERSION
 * ==========================================================
 *
 * Maps the calibrated LDR ADC range to 0-100% and clamps readings that
 * fall outside the measured dark/bright limits.
 */

uint32_t LightPercent(uint32_t adc_value)
{
    if (adc_value <= LIGHT_DARK_VALUE)
    {
        return 0U;
    }

    if (adc_value >= LIGHT_BRIGHT_VALUE)
    {
        return 100U;
    }

    return ((adc_value - LIGHT_DARK_VALUE) * 100U)
           / (LIGHT_BRIGHT_VALUE - LIGHT_DARK_VALUE);
}


/*
 * ==========================================================
 * RELAY / PUMP CONTROL
 * ==========================================================
 *
 * PB0 drives the BC337 relay interface:
 * PB0 HIGH -> BC337 ON  -> Relay ON
 * PB0 LOW  -> BC337 OFF -> Relay OFF
 *
 * The GPIO does not drive the pump directly; it only controls the
 * transistor stage that switches the relay input.
 */

void Pump_Set(bool on)
{
    if (on == true)
    {
        GPIOPinWrite(RELAY_PORT,
                     RELAY_PIN,
                     RELAY_PIN);
    }
    else
    {
        GPIOPinWrite(RELAY_PORT,
                     RELAY_PIN,
                     0U);
    }
}


/*
 * ==========================================================
 * ADC FILTERING
 * ==========================================================
 *
 * ADC sequence 2 samples both analog channels on each trigger:
 *   Sample 0 -> Soil sensor, PE3 / AIN0
 *   Sample 1 -> LDR divider,  PE2 / AIN1
 *
 * Sixteen complete sequences are averaged. This reduces short spikes
 * observed on the analog readings, especially while the pump is active.
 */

void ReadADCFiltered(uint32_t *soil_raw,
                     uint32_t *light_raw)
{
    uint32_t soil_total = 0U;
    uint32_t light_total = 0U;
    uint32_t i;

    for (i = 0U; i < ADC_SAMPLE_COUNT; i++)
    {
        ADC0_PSSI_R = (1U << 2);

        while ((ADC0_RIS_R & (1U << 2)) == 0U)
        {
        }

        soil_total += ADC0_SSFIFO2_R & 0xFFFU;

        light_total += ADC0_SSFIFO2_R & 0xFFFU;

        ADC0_ISC_R = (1U << 2);
    }

    *soil_raw = soil_total / ADC_SAMPLE_COUNT;

    *light_raw = light_total / ADC_SAMPLE_COUNT;
}


/*
 * ==========================================================
 * I2C / OLED LOW-LEVEL TRANSFER
 * ==========================================================
 *
 * These routines provide the blocking I2C transactions used by the
 * SSD1306 display. Commands use control byte 0x00; display data uses 0x40.
 */

void I2C_Wait(void)
{
    while (I2CMasterBusy(I2C0_BASE))
    {
    }
}


void OLED_WriteCommand(uint8_t command)
{
    I2CMasterSlaveAddrSet(I2C0_BASE,
                          OLED_ADDRESS,
                          false);

    I2CMasterDataPut(I2C0_BASE, 0x00U);

    I2CMasterControl(I2C0_BASE,
                     I2C_MASTER_CMD_BURST_SEND_START);

    I2C_Wait();

    I2CMasterDataPut(I2C0_BASE, command);

    I2CMasterControl(I2C0_BASE,
                     I2C_MASTER_CMD_BURST_SEND_FINISH);

    I2C_Wait();
}


void OLED_WriteData(uint8_t data)
{
    I2CMasterSlaveAddrSet(I2C0_BASE,
                          OLED_ADDRESS,
                          false);

    I2CMasterDataPut(I2C0_BASE, 0x40U);

    I2CMasterControl(I2C0_BASE,
                     I2C_MASTER_CMD_BURST_SEND_START);

    I2C_Wait();

    I2CMasterDataPut(I2C0_BASE, data);

    I2CMasterControl(I2C0_BASE,
                     I2C_MASTER_CMD_BURST_SEND_FINISH);

    I2C_Wait();
}


/*
 * ==========================================================
 * OLED POSITIONING AND CLEARING
 * ==========================================================
 *
 * The SSD1306 is updated page by page. Clearing only the page that is
 * about to be redrawn avoids leaving characters from a longer old value.
 */

void OLED_SetCursor(uint8_t page,
                    uint8_t column)
{
    OLED_WriteCommand(0xB0U + page);

    OLED_WriteCommand(0x00U |
                      (column & 0x0FU));

    OLED_WriteCommand(0x10U |
                      ((column >> 4) & 0x0FU));
}


void OLED_ClearPage(uint8_t page)
{
    uint32_t column;

    OLED_SetCursor(page, 0U);

    for (column = 0U; column < 128U; column++)
    {
        OLED_WriteData(0x00U);
    }
}


void OLED_Clear(void)
{
    uint8_t page;

    for (page = 0U; page < 8U; page++)
    {
        OLED_ClearPage(page);
    }
}


/*
 * ==========================================================
 * OLED CHARACTER FONT
 * ==========================================================
 *
 * Minimal 5x7 glyph set containing only the characters required by the
 * project display. Keeping the table small saves program memory.
 */

void OLED_WriteChar(char c)
{
    uint8_t d[5] = {0U, 0U, 0U, 0U, 0U};
    uint8_t i;

    if      (c == '0') { d[0]=0x3E; d[1]=0x51; d[2]=0x49; d[3]=0x45; d[4]=0x3E; }
    else if (c == '1') { d[0]=0x00; d[1]=0x42; d[2]=0x7F; d[3]=0x40; d[4]=0x00; }
    else if (c == '2') { d[0]=0x42; d[1]=0x61; d[2]=0x51; d[3]=0x49; d[4]=0x46; }
    else if (c == '3') { d[0]=0x21; d[1]=0x41; d[2]=0x45; d[3]=0x4B; d[4]=0x31; }
    else if (c == '4') { d[0]=0x18; d[1]=0x14; d[2]=0x12; d[3]=0x7F; d[4]=0x10; }
    else if (c == '5') { d[0]=0x27; d[1]=0x45; d[2]=0x45; d[3]=0x45; d[4]=0x39; }
    else if (c == '6') { d[0]=0x3C; d[1]=0x4A; d[2]=0x49; d[3]=0x49; d[4]=0x30; }
    else if (c == '7') { d[0]=0x01; d[1]=0x71; d[2]=0x09; d[3]=0x05; d[4]=0x03; }
    else if (c == '8') { d[0]=0x36; d[1]=0x49; d[2]=0x49; d[3]=0x49; d[4]=0x36; }
    else if (c == '9') { d[0]=0x06; d[1]=0x49; d[2]=0x49; d[3]=0x29; d[4]=0x1E; }

    else if (c == 'A') { d[0]=0x7E; d[1]=0x11; d[2]=0x11; d[3]=0x11; d[4]=0x7E; }
    else if (c == 'C') { d[0]=0x3E; d[1]=0x41; d[2]=0x41; d[3]=0x41; d[4]=0x22; }
    else if (c == 'E') { d[0]=0x7F; d[1]=0x49; d[2]=0x49; d[3]=0x49; d[4]=0x41; }
    else if (c == 'F') { d[0]=0x7F; d[1]=0x09; d[2]=0x09; d[3]=0x09; d[4]=0x01; }
    else if (c == 'G') { d[0]=0x3E; d[1]=0x41; d[2]=0x49; d[3]=0x49; d[4]=0x7A; }
    else if (c == 'H') { d[0]=0x7F; d[1]=0x08; d[2]=0x08; d[3]=0x08; d[4]=0x7F; }
    else if (c == 'I') { d[0]=0x00; d[1]=0x41; d[2]=0x7F; d[3]=0x41; d[4]=0x00; }
    else if (c == 'L') { d[0]=0x7F; d[1]=0x40; d[2]=0x40; d[3]=0x40; d[4]=0x40; }
    else if (c == 'M') { d[0]=0x7F; d[1]=0x02; d[2]=0x0C; d[3]=0x02; d[4]=0x7F; }
    else if (c == 'N') { d[0]=0x7F; d[1]=0x04; d[2]=0x08; d[3]=0x10; d[4]=0x7F; }
    else if (c == 'O') { d[0]=0x3E; d[1]=0x41; d[2]=0x41; d[3]=0x41; d[4]=0x3E; }
    else if (c == 'P') { d[0]=0x7F; d[1]=0x09; d[2]=0x09; d[3]=0x09; d[4]=0x06; }
    else if (c == 'R') { d[0]=0x7F; d[1]=0x09; d[2]=0x19; d[3]=0x29; d[4]=0x46; }
    else if (c == 'S') { d[0]=0x46; d[1]=0x49; d[2]=0x49; d[3]=0x49; d[4]=0x31; }
    else if (c == 'T') { d[0]=0x01; d[1]=0x01; d[2]=0x7F; d[3]=0x01; d[4]=0x01; }
    else if (c == 'U') { d[0]=0x3F; d[1]=0x40; d[2]=0x40; d[3]=0x40; d[4]=0x3F; }
    else if (c == 'Y') { d[0]=0x07; d[1]=0x08; d[2]=0x70; d[3]=0x08; d[4]=0x07; }

    else if (c == ':') { d[0]=0x00; d[1]=0x36; d[2]=0x36; d[3]=0x00; d[4]=0x00; }
    else if (c == '%') { d[0]=0x63; d[1]=0x13; d[2]=0x08; d[3]=0x64; d[4]=0x63; }
    else if (c == ' ') { d[0]=0x00; d[1]=0x00; d[2]=0x00; d[3]=0x00; d[4]=0x00; }

    for (i = 0U; i < 5U; i++)
    {
        OLED_WriteData(d[i]);
    }

    OLED_WriteData(0x00U);
}


void OLED_WriteString(const char *text)
{
    while (*text != '\0')
    {
        OLED_WriteChar(*text);
        text++;
    }
}


void OLED_WriteNumber(uint32_t number)
{
    char buffer[10];
    int i = 0;

    if (number == 0U)
    {
        OLED_WriteChar('0');
        return;
    }

    while (number > 0U)
    {
        buffer[i++] = (char)((number % 10U) + '0');
        number /= 10U;
    }

    while (i > 0)
    {
        OLED_WriteChar(buffer[--i]);
    }
}


/*
 * ==========================================================
 * OLED INITIALIZATION
 * ==========================================================
 *
 * Configures the SSD1306 for the 128x64 display used in the prototype,
 * then clears the display before the first status update.
 */

void OLED_Init(void)
{
    SysCtlDelay(SysCtlClockGet() / 30U);

    OLED_WriteCommand(0xAEU);
    OLED_WriteCommand(0xD5U);
    OLED_WriteCommand(0x80U);
    OLED_WriteCommand(0xA8U);
    OLED_WriteCommand(0x3FU);
    OLED_WriteCommand(0xD3U);
    OLED_WriteCommand(0x00U);
    OLED_WriteCommand(0x40U);
    OLED_WriteCommand(0x8DU);
    OLED_WriteCommand(0x14U);
    OLED_WriteCommand(0x20U);
    OLED_WriteCommand(0x02U);
    OLED_WriteCommand(0xA1U);
    OLED_WriteCommand(0xC8U);
    OLED_WriteCommand(0xDAU);
    OLED_WriteCommand(0x12U);
    OLED_WriteCommand(0x81U);
    OLED_WriteCommand(0x7FU);
    OLED_WriteCommand(0xD9U);
    OLED_WriteCommand(0xF1U);
    OLED_WriteCommand(0xDBU);
    OLED_WriteCommand(0x40U);
    OLED_WriteCommand(0xA4U);
    OLED_WriteCommand(0xA6U);
    OLED_WriteCommand(0xAFU);

    OLED_Clear();
}


/*
 * ==========================================================
 * MICROSECOND DELAY
 * ==========================================================
 *
 * DHT11 timing is handled with short busy-wait delays derived from the
 * current system clock. A minimum of one SysCtlDelay loop is enforced.
 */

void DelayUs(uint32_t us)
{
    uint32_t loops;

    loops = (SysCtlClockGet() / 3000000U) * us;

    if (loops == 0U)
    {
        loops = 1U;
    }

    SysCtlDelay(loops);
}


/*
 * ==========================================================
 * DHT11 INTERFACE
 * ==========================================================
 *
 * Implements the DHT11 single-wire timing sequence with explicit
 * timeouts. A timeout or checksum mismatch is treated as a failed read
 * instead of allowing a bad sample to overwrite the last valid data.
 */

bool DHT_WaitForLevel(uint8_t level,
                      uint32_t timeout_us)
{
    uint32_t counter = 0U;

    while ((((GPIOPinRead(DHT_PORT,
                          DHT_PIN) != 0U) ? 1U : 0U) != level))
    {
        DelayUs(1U);

        counter++;

        if (counter >= timeout_us)
        {
            return false;
        }
    }

    return true;
}


bool DHT11_Read(uint8_t *temperature,
                uint8_t *humidity)
{
    uint8_t data[5] = {0U, 0U, 0U, 0U, 0U};
    uint32_t bit;

    GPIOPinTypeGPIOOutput(DHT_PORT,
                          DHT_PIN);

    GPIOPinWrite(DHT_PORT,
                 DHT_PIN,
                 0U);

    SysCtlDelay(SysCtlClockGet() / 150U);

    GPIOPinWrite(DHT_PORT,
                 DHT_PIN,
                 DHT_PIN);

    DelayUs(30U);

    GPIOPinTypeGPIOInput(DHT_PORT,
                         DHT_PIN);


    if (!DHT_WaitForLevel(0U, 120U))
    {
        return false;
    }

    if (!DHT_WaitForLevel(1U, 120U))
    {
        return false;
    }

    if (!DHT_WaitForLevel(0U, 120U))
    {
        return false;
    }


    for (bit = 0U; bit < 40U; bit++)
    {
        if (!DHT_WaitForLevel(1U, 100U))
        {
            return false;
        }

        DelayUs(40U);

        data[bit / 8U] <<= 1U;

        if (GPIOPinRead(DHT_PORT,
                        DHT_PIN) != 0U)
        {
            data[bit / 8U] |= 1U;
        }

        if (!DHT_WaitForLevel(0U, 120U))
        {
            if (bit != 39U)
            {
                return false;
            }
        }
    }


    if (((uint8_t)(data[0] +
                   data[1] +
                   data[2] +
                   data[3])) != data[4])
    {
        return false;
    }


    *humidity = data[0];

    *temperature = data[2];

    return true;
}


bool DHT11_ReadReliable(uint8_t *temperature,
                        uint8_t *humidity)
{
    uint8_t attempt;

    for (attempt = 0U;
         attempt < DHT_MAX_RETRIES;
         attempt++)
    {
        if (DHT11_Read(temperature,
                       humidity) == true)
        {
            return true;
        }

        SysCtlDelay(SysCtlClockGet() / 30U);
    }

    return false;
}


/*
 * ==========================================================
 * OLED STATUS SCREEN
 * ==========================================================
 *
 * Rewrites the pages used by the live status display. Temperature and
 * humidity remain in INIT state until at least one valid DHT11 frame has
 * been received.
 */

void OLED_UpdateDisplay(uint32_t soil,
                        uint32_t light_percent,
                        uint8_t temperature,
                        uint8_t humidity,
                        uint8_t pump_state,
                        bool safety_lock,
                        uint32_t runtime,
                        bool dht_valid,
                        bool system_started)
{
    /*
     * Soil moisture
     */
    OLED_ClearPage(0U);
    OLED_SetCursor(0U, 0U);
    OLED_WriteString("SOIL: ");
    OLED_WriteNumber(soil);
    OLED_WriteChar('%');


    /*
     * Air temperature
     */
    OLED_ClearPage(1U);
    OLED_SetCursor(1U, 0U);
    OLED_WriteString("TEMP: ");

    if (dht_valid == true)
    {
        OLED_WriteNumber(temperature);
        OLED_WriteChar('C');
    }
    else
    {
        OLED_WriteString("INIT");
    }


    /*
     * Relative humidity
     */
    OLED_ClearPage(2U);
    OLED_SetCursor(2U, 0U);
    OLED_WriteString("HUM: ");

    if (dht_valid == true)
    {
        OLED_WriteNumber(humidity);
        OLED_WriteChar('%');
    }
    else
    {
        OLED_WriteString("INIT");
    }


    OLED_ClearPage(3U);


    /*
     * Ambient light
     */
    OLED_ClearPage(4U);
    OLED_SetCursor(4U, 0U);
    OLED_WriteString("LIGHT: ");
    OLED_WriteNumber(light_percent);
    OLED_WriteChar('%');


    OLED_ClearPage(5U);


    /*
     * Pump state
     */
    OLED_ClearPage(6U);
    OLED_SetCursor(6U, 0U);
    OLED_WriteString("PUMP: ");


    /*
     * Before the system is enabled with SW1, show STOP regardless of the
     * measured soil condition. This matches the manual-start safety logic.
     */
    if (system_started == false)
    {
        OLED_WriteString("STOP");
    }
    else if (safety_lock == true)
    {
        OLED_WriteString("SAFETY");
    }
    else if (pump_state == 1U)
    {
        OLED_WriteString("ON");
    }
    else
    {
        OLED_WriteString("OFF");
    }


    /*
     * Current pump runtime
     */
    OLED_ClearPage(7U);
    OLED_SetCursor(7U, 0U);
    OLED_WriteString("TIME: ");
    OLED_WriteNumber(runtime);
    OLED_WriteChar('S');
}


/*
 * ==========================================================
 * TIMER0A INTERRUPT
 * ==========================================================
 *
 * The ISR only acknowledges the interrupt and raises a flag. Sensor
 * reads, display updates, and pump logic stay in the main context.
 */

void Timer0AIntHandler(void)
{
    TimerIntClear(TIMER0_BASE,
                  TIMER_TIMA_TIMEOUT);

    timer_flag = true;
}


/*
 * ==========================================================
 * MAIN APPLICATION
 * ==========================================================
 */

int main(void)
{
    /*
     * The controller powers up in STOP mode. Sensors and the display can
     * run immediately, but automatic irrigation is not enabled until SW1
     * is pressed. This makes startup predictable and gives manual control
     * during testing and demonstrations.
     */
    bool system_started = false;


    /*
     * Previous SW1 state used for edge detection.
     *
     * PF4 uses a pull-up:
     *   true  -> button released
     *   false -> button pressed
     */
    bool previous_button_released = true;


    uint8_t pump_state = 0U;

    uint32_t pump_runtime_seconds = 0U;

    bool safety_lock = false;


    uint8_t dry_confirm_counter = 0U;

    uint8_t wet_confirm_counter = 0U;


    uint8_t air_temperature = 0U;

    uint8_t air_humidity = 0U;

    bool dht_has_valid_data = false;


    uint8_t new_temperature = 0U;

    uint8_t new_humidity = 0U;

    bool dht_read_success = false;

    uint8_t dht_second_counter = 0U;


    /*
     * ======================================================
     * UART0 AND PORT A
     * ======================================================
     *
     * PA0/PA1 provide the serial debug interface. PA2 is later used as
     * the DHT11 data pin.
     */

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);


    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }


    GPIOPinConfigure(GPIO_PA0_U0RX);

    GPIOPinConfigure(GPIO_PA1_U0TX);


    GPIOPinTypeUART(GPIO_PORTA_BASE,
                    GPIO_PIN_0 |
                    GPIO_PIN_1);


    UARTConfigSetExpClk(UART0_BASE,
                        SysCtlClockGet(),
                        115200,
                        UART_CONFIG_WLEN_8 |
                        UART_CONFIG_STOP_ONE |
                        UART_CONFIG_PAR_NONE);


    GPIOPinTypeGPIOInput(DHT_PORT,
                         DHT_PIN);


    /*
     * ======================================================
     * PORT B, RELAY OUTPUT, AND OLED I2C
     * ======================================================
     */

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);


    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
    }

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0))
    {
    }


    GPIOPinTypeGPIOOutput(RELAY_PORT,
                          RELAY_PIN);


    /*
     * Fail-safe startup state: keep the physical pump output OFF before
     * any sensor reading or control decision is made.
     */
    Pump_Set(false);


    GPIOPinConfigure(GPIO_PB2_I2C0SCL);

    GPIOPinConfigure(GPIO_PB3_I2C0SDA);


    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE,
                      GPIO_PIN_2);

    GPIOPinTypeI2C(GPIO_PORTB_BASE,
                   GPIO_PIN_3);


    I2CMasterInitExpClk(I2C0_BASE,
                        SysCtlClockGet(),
                        false);


    OLED_Init();


    /*
     * ======================================================
     * PORT F AND SW1
     * ======================================================
     */

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);


    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }


    /*
     * PF4 is configured as an input with the internal weak pull-up used
     * by the LaunchPad SW1 circuit.
     */
    GPIOPinTypeGPIOInput(BUTTON_PORT,
                         BUTTON_PIN);


    GPIOPadConfigSet(BUTTON_PORT,
                     BUTTON_PIN,
                     GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);


    /*
     * ======================================================
     * ADC0
     * ======================================================
     *
     * PE3 and PE2 are configured as analog inputs for the soil sensor and
     * LDR divider. ADC0 sequencer 2 reads both channels in one trigger.
     */

    SYSCTL_RCGCGPIO_R |= (1U << 4);

    SYSCTL_RCGCADC_R |= (1U << 0);


    while ((SYSCTL_PRGPIO_R & (1U << 4)) == 0U)
    {
    }

    while ((SYSCTL_PRADC_R & (1U << 0)) == 0U)
    {
    }


    GPIO_PORTE_DEN_R &= ~((1U << 3) |
                          (1U << 2));


    GPIO_PORTE_AMSEL_R |= ((1U << 3) |
                           (1U << 2));


    ADC0_ACTSS_R &= ~(1U << 2);

    ADC0_EMUX_R &= ~0x0F00U;


    ADC0_SSMUX2_R = (0U << 0) |
                     (1U << 4);


    ADC0_SSCTL2_R = (1U << 5) |
                     (1U << 6);


    ADC0_ACTSS_R |= (1U << 2);


    /*
     * ======================================================
     * INITIAL DHT11 SAMPLE
     * ======================================================
     *
     * Try to obtain a valid temperature/humidity sample before entering
     * normal operation. Interrupts are paused because DHT11 decoding is
     * timing-sensitive.
     */

    IntMasterDisable();


    dht_read_success =
        DHT11_ReadReliable(&new_temperature,
                           &new_humidity);


    IntMasterEnable();


    if (dht_read_success == true)
    {
        air_temperature = new_temperature;

        air_humidity = new_humidity;

        dht_has_valid_data = true;
    }


    /*
     * ======================================================
     * TIMER0
     * ======================================================
     *
     * Timer0A generates the one-second scheduling tick used by the
     * control loop and pump runtime counter.
     */

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);


    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }


    TimerDisable(TIMER0_BASE,
                 TIMER_A);


    TimerConfigure(TIMER0_BASE,
                   TIMER_CFG_PERIODIC);


    TimerLoadSet(TIMER0_BASE,
                 TIMER_A,
                 SysCtlClockGet() - 1U);


    TimerIntClear(TIMER0_BASE,
                  TIMER_TIMA_TIMEOUT);


    TimerIntRegister(TIMER0_BASE,
                     TIMER_A,
                     Timer0AIntHandler);


    TimerIntEnable(TIMER0_BASE,
                   TIMER_TIMA_TIMEOUT);


    IntMasterEnable();


    TimerEnable(TIMER0_BASE,
                TIMER_A);


    /*
     * ======================================================
     * MAIN LOOP
     * ======================================================
     */

    while (1)
    {
        /*
         * ==================================================
         * SW1 START / STOP CONTROL
         * ==================================================
         *
         * PF4 is active-low because of the internal pull-up:
         *   released -> HIGH
         *   pressed  -> LOW
         */

        bool button_released =
            (GPIOPinRead(BUTTON_PORT,
                         BUTTON_PIN) != 0U);


        /*
         * Detect the HIGH-to-LOW transition so one physical press toggles
         * the system state instead of repeatedly toggling while held.
         */
        if ((previous_button_released == true) &&
            (button_released == false))
        {
            /*
             * Short software debounce delay for the mechanical switch.
             */
            SysCtlDelay(SysCtlClockGet() / 300U);


            /*
             * Accept the press only if PF4 is still LOW after debounce.
             */
            if (GPIOPinRead(BUTTON_PORT,
                            BUTTON_PIN) == 0U)
            {
                /*
                 * ==================================================
                 * START
                 * ==================================================
                 *
                 * Enabling the controller resets all pump-related state so
                 * each run starts from a known condition.
                 */
                if (system_started == false)
                {
                    system_started = true;

                    pump_state = 0U;

                    pump_runtime_seconds = 0U;

                    safety_lock = false;

                    dry_confirm_counter = 0U;

                    wet_confirm_counter = 0U;


                    /*
                     * Do not start the pump immediately after SW1 is pressed.
                     * The soil must first be confirmed dry for three consecutive
                     * control cycles.
                     */
                    Pump_Set(false);


                    UART_SendText("\r\n*** SYSTEM STARTED ***\r\n");
                }


                /*
                 * ==================================================
                 * STOP / RESET
                 * ==================================================
                 *
                 * A second SW1 press disables automatic irrigation and clears
                 * the control state for the next run.
                 */
                else
                {
                    system_started = false;

                    pump_state = 0U;

                    pump_runtime_seconds = 0U;

                    safety_lock = false;

                    dry_confirm_counter = 0U;

                    wet_confirm_counter = 0U;


                    /*
                     * Force the relay output OFF immediately when the user
                     * stops the system.
                     */
                    Pump_Set(false);


                    UART_SendText("\r\n*** SYSTEM STOPPED ***\r\n");
                }
            }
        }


        previous_button_released = button_released;


        /*
         * ==================================================
         * ONE-SECOND CONTROL LOOP
         * ==================================================
         *
         * The timer interrupt schedules this block. All slower application
         * work is kept here rather than inside the ISR.
         */

        if (timer_flag == true)
        {
            uint32_t soil_raw;
            uint32_t light_raw;
            uint32_t moisture_percent;
            uint32_t light_percent;


            timer_flag = false;


            /*
             * ==================================================
             * ANALOG SENSOR UPDATE
             * ==================================================
             *
             * Read averaged soil and light ADC values, then convert them to
             * calibrated percentages used by the controller and display.
             */

            ReadADCFiltered(&soil_raw,
                            &light_raw);


            moisture_percent =
                SoilMoisturePercent(soil_raw);


            light_percent =
                LightPercent(light_raw);


            /*
             * ==================================================
             * IRRIGATION CONTROL
             * ==================================================
             *
             * Automatic pump decisions are only evaluated after SW1 enables
             * the system. In STOP mode the same sensors may still be monitored,
             * but they cannot start the pump.
             */

            if (system_started == true)
            {
                /*
                 * Dry confirmation counter
                 *
                 * A single low reading is not enough to start irrigation.
                 */
                if (moisture_percent <
                    MOISTURE_LOW_THRESHOLD)
                {
                    if (dry_confirm_counter <
                        DRY_CONFIRM_COUNT)
                    {
                        dry_confirm_counter++;
                    }

                    wet_confirm_counter = 0U;
                }


                /*
                 * Wet confirmation counter
                 *
                 * A single high reading is not enough to stop irrigation.
                 */
                else if (moisture_percent >
                         MOISTURE_HIGH_THRESHOLD)
                {
                    if (wet_confirm_counter <
                        WET_CONFIRM_COUNT)
                    {
                        wet_confirm_counter++;
                    }

                    dry_confirm_counter = 0U;
                }


                /*
                 * Hysteresis band
                 *
                 * Between 35% and 50% neither confirmation counter is allowed
                 * to accumulate. This prevents rapid switching near a threshold.
                 */
                else
                {
                    dry_confirm_counter = 0U;

                    wet_confirm_counter = 0U;
                }


                /*
                 * ==================================================
                 * CONFIRMED DRY CONDITION
                 * ==================================================
                 *
                 * Start the pump only after the required number of consecutive
                 * dry measurements and only when the safety lock is clear.
                 */

                if ((dry_confirm_counter >=
                     DRY_CONFIRM_COUNT) &&
                    (safety_lock == false))
                {
                    pump_state = 1U;
                }


                /*
                 * ==================================================
                 * CONFIRMED WET CONDITION
                 * ==================================================
                 *
                 * Reaching the wet confirmation count is the normal irrigation
                 * stop condition. Runtime and confirmation counters are reset.
                 */

                if (wet_confirm_counter >=
                    WET_CONFIRM_COUNT)
                {
                    pump_state = 0U;

                    pump_runtime_seconds = 0U;

                    safety_lock = false;

                    dry_confirm_counter = 0U;

                    wet_confirm_counter = 0U;
                }


                /*
                 * ==================================================
                 * MAXIMUM PUMP RUNTIME
                 * ==================================================
                 *
                 * The 20-second limit is a backup safety condition, not the
                 * normal stop target. If moisture does not rise in time, the
                 * pump is shut down and latched in SAFETY until reset.
                 */

                if ((pump_state == 1U) &&
                    (safety_lock == false))
                {
                    pump_runtime_seconds++;


                    if (pump_runtime_seconds >=
                        MAX_PUMP_RUNTIME)
                    {
                        pump_runtime_seconds =
                            MAX_PUMP_RUNTIME;

                        pump_state = 0U;

                        safety_lock = true;

                        dry_confirm_counter = 0U;

                        wet_confirm_counter = 0U;
                    }
                }


                /*
                 * ==================================================
                 * APPLY RELAY OUTPUT
                 * ==================================================
                 *
                 * The relay is energized only when the controller requests the
                 * pump and the safety lock is not active.
                 */

                if ((pump_state == 1U) &&
                    (safety_lock == false))
                {
                    Pump_Set(true);
                }
                else
                {
                    Pump_Set(false);
                }
            }


            /*
             * ==================================================
             * STOP MODE
             * ==================================================
             *
             * While automatic irrigation is disabled, clear all control state
             * and explicitly drive the pump output OFF on every control cycle.
             */
            else
            {
                pump_state = 0U;

                pump_runtime_seconds = 0U;

                safety_lock = false;

                dry_confirm_counter = 0U;

                wet_confirm_counter = 0U;

                Pump_Set(false);
            }


            /*
             * ==================================================
             * DHT11 UPDATE
             * ==================================================
             *
             * DHT11 is sampled less often than the analog channels because the
             * sensor is slow and its protocol uses timing-sensitive bit reads.
             */

            dht_second_counter++;


            if (dht_second_counter >=
                DHT_READ_INTERVAL_SECONDS)
            {
                dht_second_counter = 0U;


                /*
                 * Skip DHT11 transactions while the pump is running. During
                 * development the motor introduced enough electrical noise to
                 * make DHT timing less reliable, so the last valid reading is
                 * retained until the pump is off.
                 */
                if (pump_state == 0U)
                {
                    new_temperature = 0U;

                    new_humidity = 0U;


                    IntMasterDisable();


                    dht_read_success =
                        DHT11_ReadReliable(
                            &new_temperature,
                            &new_humidity);


                    IntMasterEnable();


                    if (dht_read_success == true)
                    {
                        air_temperature =
                            new_temperature;

                        air_humidity =
                            new_humidity;

                        dht_has_valid_data =
                            true;
                    }
                }
            }


            /*
             * ==================================================
             * OLED UPDATE
             * ==================================================
             *
             * Refresh the complete status view using the latest validated data
             * and current controller state.
             */

            OLED_UpdateDisplay(
                moisture_percent,
                light_percent,
                air_temperature,
                air_humidity,
                pump_state,
                safety_lock,
                pump_runtime_seconds,
                dht_has_valid_data,
                system_started);


            /*
             * ==================================================
             * UART DIAGNOSTICS
             * ==================================================
             *
             * Emit one readable status line per control cycle. The serial log
             * was used to verify ADC behavior, relay state, safety timing, and
             * DHT11 data independently of the OLED.
             */

            UART_SendText("ADC:");

            UART_SendNumber(soil_raw);


            UART_SendText(" | Soil:");

            UART_SendNumber(moisture_percent);

            UARTCharPut(UART0_BASE, '%');


            UART_SendText(" | Light:");

            UART_SendNumber(light_percent);

            UARTCharPut(UART0_BASE, '%');


            UART_SendText(" | System:");


            if (system_started == true)
            {
                UART_SendText("RUN");
            }
            else
            {
                UART_SendText("STOP");
            }


            UART_SendText(" | Pump:");


            if (system_started == false)
            {
                UART_SendText("STOP");
            }
            else if (safety_lock == true)
            {
                UART_SendText("SAFETY OFF");
            }
            else if (pump_state == 1U)
            {
                UART_SendText("ON");
            }
            else
            {
                UART_SendText("OFF");
            }


            UART_SendText(" | Time:");

            UART_SendNumber(pump_runtime_seconds);

            UARTCharPut(UART0_BASE, 's');


            UART_SendText(" | Temp:");


            if (dht_has_valid_data == true)
            {
                UART_SendNumber(air_temperature);

                UARTCharPut(UART0_BASE, 'C');
            }
            else
            {
                UART_SendText("INIT");
            }


            UART_SendText(" | Hum:");


            if (dht_has_valid_data == true)
            {
                UART_SendNumber(air_humidity);

                UARTCharPut(UART0_BASE, '%');
            }
            else
            {
                UART_SendText("INIT");
            }


            UARTCharPut(UART0_BASE, '\r');

            UARTCharPut(UART0_BASE, '\n');
        }
    }
}