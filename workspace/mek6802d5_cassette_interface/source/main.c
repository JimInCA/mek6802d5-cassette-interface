/*
 * mek6802d5 loader
 */

//#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "peripherals.h"

#include "fsl_dac.h"
#include "fsl_common.h"
#include "fsl_ftm.h"
#include "fsl_uart.h"
#include "fsl_cmp.h"

#include "queue.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASEADDR     DAC0
#define DAC_USED_BUFFER_SIZE  32U

// set up scope probe pulse width timer
#define SCOPE_TRIGGER_FTM_BASEADDR FTM0
// Interrupt number and interrupt handler for the FTM instance used
#define SCOPE_TRIGGER_FTM_IRQ_NUM FTM0_IRQn
#define SCOPE_TRIGGER_FTM_HANDLER FTM0_IRQHandler

// set up dac timer
#define DAC_FTM_BASEADDR FTM1
// Interrupt number and interrupt handler for the FTM instance used
#define DAC_FTM_IRQ_NUM FTM1_IRQn
#define DAC_FTM_HANDLER FTM1_IRQHandler

// set up scope probe pulse width timer
#define CMP_PULSEWIDTH_FTM_BASEADDR FTM2
// Interrupt number and interrupt handler for the FTM instance used
#define CMP_PULSEWIDTH_FTM_IRQ_NUM FTM2_IRQn
#define CMP_PULSEWIDTH_FTM_HANDLER FTM2_IRQHandler

// set up uart strobing timer
#define UART_TIMER_FTM_BASEADDR FTM3
// Interrupt number and interrupt handler for the FTM instance used
#define UART_TIMER_FTM_IRQ_NUM FTM3_IRQn
#define UART_TIMER_FTM_HANDLER FTM3_IRQHandler

// Get source clock for FTM driver
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk) / 1)

// dac timer step delay
#define STEP_DELAY 781	// base clock is 60MHz

/* Transmitter UART instance and clock */
#define TX_UART            UART0
#define TX_UART_CLKSRC     UART0_CLK_SRC
#define TX_UART_CLK_FREQ   CLOCK_GetFreq(UART0_CLK_SRC)
#define TX_UART_IRQn       UART0_RX_TX_IRQn
#define TX_UART_IRQHandler UART0_RX_TX_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint16_t g_dacDataArray[DAC_USED_BUFFER_SIZE] = {
	    2172U, 2475U, 2766U, 3034U, 3269U, 3462U, 3606U, 3694U, 3724U, 3694U, 3606U, 3462U, 3269U, 3034U, 2766U, 2475U,
	    2172U, 1869U, 1578U, 1310U, 1075U,  882U,  739U,  650U,  621U,  650U,  739U,  882U, 1075U, 1310U, 1578U, 1869U};

QUEUE tx_queue, rx_queue;

typedef struct _output_waveform_t {
	bool     active;
	uint8_t  state;
	uint8_t  output;
	uint16_t uart_data;
} output_waveform_t;

static volatile output_waveform_t output_waveform;

/*******************************************************************************
 * Code
 ******************************************************************************/

void TX_UART_IRQHandler(void)
{
    uint8_t data;

    /* If new data arrived. */
    if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(TX_UART))
    {
        data = UART_ReadByte(TX_UART);

        /* If ring buffer is not full, add data to ring buffer. */
        if (queue_enqueue(&tx_queue, data) == false)
        	UART_WriteBlocking(TX_UART, "\r\nFailure in queue_enqueue()\r\n", 30);
    }
    SDK_ISR_EXIT_BARRIER;
}

void trigger_scope(void)
{
	GPIO_PortSet(BOARD_SCOPE_TRIGGER_GPIO, 1u << BOARD_SCOPE_TRIGGER_PIN);
	FTM_StartTimer(SCOPE_TRIGGER_FTM_BASEADDR, kFTM_SystemClock);
}

void SCOPE_TRIGGER_FTM_HANDLER(void)
{
    // Clear interrupt flag
    FTM_ClearStatusFlags(SCOPE_TRIGGER_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    GPIO_PortClear(BOARD_SCOPE_TRIGGER_GPIO, 1u << BOARD_SCOPE_TRIGGER_PIN);
    FTM_StopTimer(SCOPE_TRIGGER_FTM_BASEADDR);
    __DSB();
}

static uint16_t num_cycles = 0;
static bool done = true;

void DAC_FTM_HANDLER(void)
{
	static uint16_t index = 0;
	static uint16_t cycle_count = 0;

    // Clear interrupt flag
    FTM_ClearStatusFlags(DAC_FTM_BASEADDR, kFTM_TimeOverflowFlag);

    if (index == DAC_USED_BUFFER_SIZE)
    {
        index = 0;
        cycle_count++;
        if (cycle_count == num_cycles)
        {
        	cycle_count = 0;
        	//DAC_SetBufferValue(DEMO_DAC_BASEADDR, 0U, 0U);
        	FTM_StopTimer(DAC_FTM_BASEADDR);
        	done = true;
        }
    }
    else
    {
        DAC_SetBufferValue(DEMO_DAC_BASEADDR, 0U, g_dacDataArray[index]);
        index++;
    }

    __DSB();
}

void load_dac_from_array(void)
{
	FTM_StartTimer(DAC_FTM_BASEADDR, kFTM_SystemClock);
	//trigger_scope();
	while (!done) ;
}

void write_1 (void)
{
	num_cycles = 8;
    FTM_SetTimerPeriod(DAC_FTM_BASEADDR, STEP_DELAY);	// base clock is 60MHz
    done = false;
	load_dac_from_array();
	while (!done) ;
}

void write_0 (void)
{
	num_cycles = 4;
    FTM_SetTimerPeriod(DAC_FTM_BASEADDR, STEP_DELAY*2);	// base clock is 60MHz
    done = false;
	load_dac_from_array();
	while (!done) ;
}


void write_byte(uint8_t data_byte)
{
	uint8_t i, mask = 0x01;

	trigger_scope();
	write_0();					// start bit
	for (i = 0; i < 8; i++)		// 8 data bits, lsb first
	{
		switch (data_byte & mask)
		{
		case 0: write_0();
		        break;
		case 1: write_1();
		        break;
		}
		data_byte >>= 1;
	}
	write_1();					// first stop bit
	write_1();					// second stop bit
}

void UART_TIMER_FTM_HANDLER(void)
{
	uint32_t uart_state, uart_mask;

    // Clear interrupt flag
    FTM_ClearStatusFlags(UART_TIMER_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    FTM_StopTimer(UART_TIMER_FTM_BASEADDR);

    //uart_state = output_waveform.output;	// Kludge!  Can't seem to read output pin!
    uart_state = GPIO_PinRead(BOARD_CMP0_UART_TX_GPIO, BOARD_CMP0_UART_TX_PIN);

	uart_mask = (0x0001 << output_waveform.state);

    switch (output_waveform.state)
    {
    case  0: uart_mask = 0x0001; break; // start bit
    case  1: uart_mask = 0x0002; break; // D0 bit
    case  2: uart_mask = 0x0004; break; // D1 bit
    case  3: uart_mask = 0x0008; break; // D2 bit
    case  4: uart_mask = 0x0010; break; // D3 bit
    case  5: uart_mask = 0x0020; break; // D4 bit
    case  6: uart_mask = 0x0040; break; // D5 bit
    case  7: uart_mask = 0x0080; break; // D6 bit
    case  8: uart_mask = 0x0100; break; // D7 bit
    case  9: uart_mask = 0x0200; break; // stop bit 1
    case 10: uart_mask = 0x0400; break; // stop bit 2
    };

    if (uart_state > 0)
    {
        output_waveform.uart_data |= uart_mask;
    }
    else
    {
    	output_waveform.uart_data &= ~uart_mask;
    };

    if (output_waveform.state++ < 10)
    {
        FTM_SetTimerPeriod(UART_TIMER_FTM_BASEADDR, 50000);	// base clock is 15MHz
        FTM_StartTimer(UART_TIMER_FTM_BASEADDR, kFTM_SystemClock);
    }
    else
    {
    	static QUEUE_TYPE data;

        FTM_StopTimer(UART_TIMER_FTM_BASEADDR);

        data = ((output_waveform.uart_data >> 1) & 0x000000ff);
        if (queue_enqueue(&rx_queue, data) == false)
        	UART_WriteBlocking(TX_UART, "\r\nFailure in queue_enqueue()\r\n", 30);

    	output_waveform.active    = false;
	    output_waveform.state     = 0;
	    output_waveform.uart_data = 0;
	    output_waveform.output    = 1;
    }

    __DSB();
}

volatile uint32_t g_CmpFlags = 0U;

void CMP0_IRQHANDLER(void)
{
	static uint32_t rising_edge_count = 0;
	static uint32_t falling_edge_count = 0;
	uint32_t pulse_width;

    g_CmpFlags = CMP_GetStatusFlags(CMP0_PERIPHERAL);
    CMP_ClearStatusFlags(CMP0_PERIPHERAL, kCMP_OutputRisingEventFlag | kCMP_OutputFallingEventFlag);
    if (0U != (g_CmpFlags & kCMP_OutputRisingEventFlag))
    {
        rising_edge_count = FTM_GetCurrentTimerCount(CMP_PULSEWIDTH_FTM_BASEADDR);
    }
    else if (0U != (g_CmpFlags & kCMP_OutputFallingEventFlag))
    {
        falling_edge_count = FTM_GetCurrentTimerCount(CMP_PULSEWIDTH_FTM_BASEADDR);

        if (falling_edge_count > rising_edge_count)
        {
        	pulse_width = falling_edge_count - rising_edge_count;
        }
        else
        {
        	pulse_width = falling_edge_count + (0xffff - rising_edge_count);
        }

		if (pulse_width > 18750)
		{
			GPIO_PortClear(BOARD_CMP0_UART_TX_GPIO, 1u << BOARD_CMP0_UART_TX_PIN);
			output_waveform.output = 0;
			if (output_waveform.active == false)
			{
				output_waveform.active    = true;
				output_waveform.state     = 0;
				output_waveform.uart_data = 0;

			    FTM_SetTimerPeriod(UART_TIMER_FTM_BASEADDR, 25000);	// base clock is 15MHz
				FTM_StartTimer(UART_TIMER_FTM_BASEADDR, kFTM_SystemClock);
			}
		}
		else
		{
			GPIO_PortSet(BOARD_CMP0_UART_TX_GPIO, 1u << BOARD_CMP0_UART_TX_PIN);
			output_waveform.output = 1;
			if (output_waveform.active == true)
			{
				output_waveform.output = 1;
			}
		}
    }
    SDK_ISR_EXIT_BARRIER;
}

void CMP_PULSEWIDTH_FTM_HANDLER(void)
{
    // Clear interrupt flag
    FTM_ClearStatusFlags(CMP_PULSEWIDTH_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    __DSB();
}

int main(void)
{
    uint8_t index;
    dac_config_t dacConfigStruct;
    uint32_t dacValue;
    ftm_config_t scope_trigger_info;
    ftm_config_t dac_timer_info;
    ftm_config_t cmp_pulsewidth_info;
    ftm_config_t uart_timer_info;
    uart_config_t config;

	QUEUE_TYPE data;

	output_waveform.active    = false;
	output_waveform.state     = 0;
	output_waveform.output    = 1;
	output_waveform.uart_data = 0;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    queue_init(&tx_queue);
    queue_init(&rx_queue);

    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;
    UART_Init(TX_UART, &config, TX_UART_CLK_FREQ);

    UART_WriteBlocking(TX_UART, "\r\nMEK6802D5 Loader\r\n", 20);

    /* Enable RX interrupt. */
    UART_EnableInterrupts(TX_UART, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
    EnableIRQ(TX_UART_IRQn);

    // setup scope trigger timer
    FTM_GetDefaultConfig(&scope_trigger_info);
    // Divide FTM clock by 4
    scope_trigger_info.prescale = kFTM_Prescale_Divide_1;
    // Initialize FTM module
    FTM_Init(SCOPE_TRIGGER_FTM_BASEADDR, &scope_trigger_info);
    // Set timer period
    FTM_SetTimerPeriod(SCOPE_TRIGGER_FTM_BASEADDR, USEC_TO_COUNT(50U, FTM_SOURCE_CLOCK));
    FTM_EnableInterrupts(SCOPE_TRIGGER_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);
    EnableIRQ(SCOPE_TRIGGER_FTM_IRQ_NUM);

    // setup dac timer
    FTM_GetDefaultConfig(&dac_timer_info);
    // Divide FTM clock by 4
    dac_timer_info.prescale = kFTM_Prescale_Divide_1;
    // Initialize FTM module
    FTM_Init(DAC_FTM_BASEADDR, &dac_timer_info);
    // Set timer period
    FTM_SetTimerPeriod(DAC_FTM_BASEADDR, STEP_DELAY);	// base clock is 60MHz
    FTM_EnableInterrupts(DAC_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);
    EnableIRQ(DAC_FTM_IRQ_NUM);

    // setup uart timer
    FTM_GetDefaultConfig(&uart_timer_info);
    // Divide FTM clock by 4
    uart_timer_info.prescale = kFTM_Prescale_Divide_4;
    // Initialize FTM module
    FTM_Init(UART_TIMER_FTM_BASEADDR, &uart_timer_info);
    // Set timer period
    FTM_EnableInterrupts(UART_TIMER_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);
    EnableIRQ(UART_TIMER_FTM_IRQ_NUM);

    // setup cmp pulse width timer
    FTM_GetDefaultConfig(&cmp_pulsewidth_info);
    // Divide FTM clock by 4
    cmp_pulsewidth_info.prescale = kFTM_Prescale_Divide_1;
    // Initialize FTM module
    FTM_Init(CMP_PULSEWIDTH_FTM_BASEADDR, &cmp_pulsewidth_info);
    // Set timer period
    FTM_SetTimerPeriod(CMP_PULSEWIDTH_FTM_BASEADDR, 0xFFFF);
    //FTM_EnableInterrupts(CMP_PULSEWIDTH_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);
    //EnableIRQ(CMP_PULSEWIDTH_FTM_IRQ_NUM);
	FTM_StartTimer(CMP_PULSEWIDTH_FTM_BASEADDR, kFTM_SystemClock);

    BOARD_InitBootPeripherals();

    // Configure the DAC
    //
    // dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceVref2;
    // dacConfigStruct.enableLowPowerMode = false;
    //
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DEMO_DAC_BASEADDR, &dacConfigStruct);
    DAC_Enable(DEMO_DAC_BASEADDR, true);             // Enable output
    DAC_SetBufferReadPointer(DEMO_DAC_BASEADDR, 0U); // Make sure the read pointer to the start
                                                     //
                                                     // The buffer is not enabled, so the read pointer can not move automatically.
                                                     // However, the buffer's read pointer and items can be written manually by user.
                                                     //
    // start of main loop
    while (1)
    {
    	static QUEUE_TYPE data;
        if (done == true)
        {
            if (queue_empty(&tx_queue) == false)
            {
            	queue_dequeue(&tx_queue, &data);
            	write_byte(data);
            }
            else
            {
            	write_1();	// let's send a stream of 1s between bytes...
            }
        }

        if (queue_empty(&rx_queue) == false)
        {
        	queue_dequeue(&rx_queue, &data);
            UART_WriteByte(TX_UART, data);
        }
    }
}
