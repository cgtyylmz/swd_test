#include<avr/io.h>
#include<util/delay.h>


/*-------------------------------------------------------------*/
//	Defination
/*-------------------------------------------------------------*/
#define REQ_ID_CODE 0xA5
#define ACK_OK 4
/*-------------------------------------------------------------*/
//	Defination
/*-------------------------------------------------------------*/

uint8_t on_time = 10;
uint8_t off_time = 10;


void gpio_init()
{
	//Port B pin 3 as output swd_clk arduino 11. pin
	DDRB |= (1<<PB3);
	//Port B pin 4 as output swd_io arduino 12. pin
	DDRB |= (1<<PB4);
	DDRB |= (1<<PB5);

	PORTB &= ~(1<<PB5);
}

void SW_CLK(uint8_t count)
{
	uint8_t i;
	for(i = 0; i<count; i+=1)
	{
		_delay_us(2);
		PORTB |= (1<<PB3);
		_delay_us(1);
		PORTB &= ~(1<<PB3);
		_delay_us(1);
	}
}

void SWDIO_SET()
{
	PORTB |= (1<<PB4);
}

void SWDIO_RESET()
{
	PORTB &= ~(1<<PB4);
}

void SWDIO_SET_IN()
{
	DDRB &= ~(1<<PB4);		// Set Input
	PORTB &= ~(1<<PB4);		// Set Logic 0
}

void SWDIO_SET_OUT()
{
	DDRB |= (1<<PB4);
}

void SW_WRITE_BIT(_Bool bit)
{
	SWDIO_SET_OUT();
	if(bit) PORTB |= (1<<PB4);
	else PORTB &= ~(1<<PB4);
	SW_CLK(1);
}

uint8_t SW_READ_BIT()
{
	uint8_t bit = 0;
	SWDIO_SET_IN();
	bit = PINB & (1<<PB4);
	SW_CLK(1);
	return bit;
}

void SW_TURNAROUND()
{
	SWDIO_SET_IN();
	SW_CLK(1);
}

uint8_t SW_WRITE_BYTE(uint8_t request)
{
	uint8_t bit;
	uint8_t parity = 0;

	SW_WRITE_BIT(1);		// Start Bit
	bit = request >> 0;
	SW_WRITE_BIT(bit);		// APnDP Bit
	parity += bit;
	bit = request >> 1;		// RnW Bit
	SW_WRITE_BIT(bit);
	parity += bit;
	bit = request >> 2;
	SW_WRITE_BIT(bit);		//A2 Bit
	parity += bit;
	bit = request >> 3;
	SW_WRITE_BIT(bit);		//A3 Bit
	parity += bit;
	SW_WRITE_BIT(parity);	// Parity Bit
	SW_WRITE_BIT(0);		// Stop Bit
	SW_WRITE_BIT(1);		// Park Bit

	SW_TURNAROUND();

	uint8_t ack;
	bit = 0;
	for(uint8_t i=0; i<=3; i++)
	{
		bit = SW_READ_BIT();
		ack = bit << i;
	}

	return ack;
}

void SW_LINE_RESET()
{
	SWDIO_SET_OUT();
	SW_WRITE_BIT(1);
	SW_CLK(52);
	SW_WRITE_BIT(0);
	SW_WRITE_BIT(1);
	SW_WRITE_BIT(1);
	SW_WRITE_BIT(1);
	SW_WRITE_BIT(1);
	SW_WRITE_BIT(0);
	SW_WRITE_BIT(0);
	SW_WRITE_BIT(1);
	SW_CLK(56);
	SW_WRITE_BIT(1);
	_delay_us(2);
}
uint32_t SW_READ_DATA()
{
	uint32_t data = 0;
	uint8_t bit = 0;
	SWDIO_SET_IN();

	for(uint8_t i = 0; i<=32; i++)
	{
		bit = SW_READ_BIT();
		data = bit << i;
	}

	return data;
}

int main()
{
	uint8_t _ack = 0;
/*-------------------------------------------------------------*/
//	Initialize Systam
/*-------------------------------------------------------------*/
	gpio_init();
/*-------------------------------------------------------------*/
//	LINE RESET
/*-------------------------------------------------------------*/
	SW_LINE_RESET();
	_delay_us(2);

/*-------------------------------------------------------------*/
//	Request Acces DP Read IDCODE
/*-------------------------------------------------------------*/
	_ack = SW_WRITE_BYTE(REQ_ID_CODE);
	SW_CLK(32);
/*-------------------------------------------------------------*/
//	SWD_IO pin tristate and Turnaround
/*-------------------------------------------------------------*/
	if (_ack == 4) PORTB |= (1<<PB5);
	else PORTB &= ~(1<<PB5);

	return 0;
}
