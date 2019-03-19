#include<avr/io.h>
#include<util/delay.h>
/*----------------------------------------------------------------------------*/
//	Defination
/*----------------------------------------------------------------------------*/
#define REQ_ID_CODE 0xA5
#define ACK_OK 1
/*----------------------------------------------------------------------------*/
//	Defination
/*----------------------------------------------------------------------------*/

uint8_t on_time = 10;
uint8_t off_time = 10;

void gpio_init()
{
	//Port B pin 3 as output swd_clk arduino pin 11
	DDRB |= (1<<PB3);
	//Port B pin 4 as output swd_io arduino pin 12
	DDRB |= (1<<PB4);
	DDRB |= (1<<PB5);

	PORTB &= ~(1<<PB5);
}

void SW_CLK(uint8_t count)
{
	uint8_t i;
	for(i = 0; i<count; i+=1)
	{
		PORTB &= ~(1<<PB3);
		_delay_us(1);
		PORTB |= (1<<PB3);
		_delay_us(1);
	}
}
void SW_CLK_SET()
{
	PORTB |= (1<<PB3);
}
void SW_CLK_CLR()
{
	PORTB &= ~(1<<PB3);
}
void SWDIO_SET()
{
	PORTB |= (1<<PB4);
}
void SWDIO_CLR()
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
void SW_WRITE_BIT(uint8_t bit)
{
	SWDIO_SET_OUT();
	if(bit) SWDIO_SET();
	else SWDIO_CLR();
	SW_CLK(1);
}
uint8_t SW_READ_BIT()
{
	uint8_t bit = 0;
	SWDIO_SET_IN();
	bit = (PINB & (1<<PB4)) >> PB4;
	SW_CLK(1);
	return bit;
}
void SW_TURNAROUND()
{
	SWDIO_SET_IN();
	SW_CLK(1);
}
uint8_t SW_REQUEST(uint8_t request)
{
	uint8_t bit;

	for(uint8_t i = 0; i<8; i++)
	{
		bit = (request >> i) & 1;
		SW_WRITE_BIT(bit);
	}

	SW_TURNAROUND();

	uint8_t ack = 0;
	bit = 0;
	for(uint8_t i=0; i<3; i++)
	{
		bit = SW_READ_BIT();
		ack += (bit << i);
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
	SW_WRITE_BIT(0);
	SW_WRITE_BIT(0);
}
uint32_t SW_READ_DATA()
{
	uint32_t data = 0;
	uint8_t bit = 0;
	uint8_t parity = 0;
	SWDIO_SET_IN();

	for(uint8_t i = 0; i<32; i++)
	{
		bit = SW_READ_BIT();
		data += bit << i;
	}
	parity = SW_READ_BIT();
	if(parity == 1) parity = 0;
	return data;
}

int main()
{
	uint8_t _ack = 0;
	uint32_t data = 0;
/*----------------------------------------------------------------------------*/
//	Initialize System
/*----------------------------------------------------------------------------*/
	gpio_init();
/*----------------------------------------------------------------------------*/
//	LINE RESET
/*----------------------------------------------------------------------------*/
	SW_LINE_RESET();
/*----------------------------------------------------------------------------*/
//	Request Acces DP Read IDCODE
/*----------------------------------------------------------------------------*/
	_ack = SW_REQUEST(REQ_ID_CODE);
	data = SW_READ_DATA();
	//SW_CLK(35);
/*----------------------------------------------------------------------------*/
//	SWD_IO pin tristate and Turnaround
/*----------------------------------------------------------------------------*/
	if (_ack == ACK_OK) PORTB |= (1<<PB5);
	else PORTB &= ~(1<<PB5);
	if(data == 196154487) PORTB &= ~(1<<PB5);
	else PORTB |= (1<<PB5);
	return 0;
}
