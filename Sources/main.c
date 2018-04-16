// Segunda Inmersion

#include <derivative.h> /* include peripheral declarations /*/

#define ON 30000 //parametro para encender
unsigned long adc_value;
unsigned long captura[5];
unsigned long km = 0;

unsigned char mens0[] = { "AT\r\n" };
unsigned char mens1[] = { "AT+CWMODE=1\r\n" };
unsigned char mens2[] = {
		"AT+CWJAP=\"riot-equipo2\",\"ensemestreiaprendoymedivierto\"\r\n" };
unsigned char mens3[] = {
		"AT+CIPSTA_DEF=\"172.18.0.5\",\"172.18.3.254\",\"255.255.252.0\"\r\n" };
unsigned char mens4[] = { "AT+CIFSR\r\n" };
unsigned char mens5[] = { "AT+CIPMUX=1\r\n" };
unsigned char mens6[] = { "AT+CIPSERVER=1,80\r\n" };
unsigned char mens65[] = { "AT+CIPSTART=4,\"TCP\",\"172.18.0.15\",80\r\n" };
unsigned char mens7[] = { "AT+CIPSEND=4,64\r\n" };
unsigned char mens8[] = { "AT+CIPSTATUS\r\n" };

unsigned char mensStatus[2][70] =
		{
				{
						"GET /status.php?cmd=LED-R1X50ANDOF HTTP/1.1\r\nHost: localhost\r\n\r\n" },
				{
						"GET /status.php?cmd=LED-R1X50ANDON HTTP/1.1\r\nHost: localhost\r\n\r\n" } };

unsigned char close[5][16] = { { "AT+CIPCLOSE=0\r\n" }, { "AT+CIPCLOSE=1\r\n" },
		{ "AT+CIPCLOSE=2\r\n" }, { "AT+CIPCLOSE=3\r\n" },
		{ "AT+CIPCLOSE=4\r\n" } };

unsigned char cmpmens1[] = { "ready\r\n" };
unsigned char cmpmens2[] = { "OK\r\n" };
unsigned char cmpmens3[] = { "Equipo2" };
unsigned char cmpmens4[] = { ">" };
unsigned char cmpmens5[] = { "4,CLOSED" };

unsigned char buffer[300];
unsigned char LED;
unsigned char automatico = '1';
unsigned char isOn;

void vUART_init(void) {
	SIM_SCGC4 = 0x00000C00; 	//Hab clk UART0 y UART1
	UART0_BDH = 0;
	UART0_BDL = 11;   		//115200 bps
	UART1_BDH = 0;
	UART1_BDL = 11;    		//115200 bps

	UART0_C2 = 12; 			// bit 3: Hab Tx, bit 2: Hab Rx
	UART1_C2 = 12;

	//Pines
	SIM_SCGC5 = 0x00000C00; 	//Hab clk PORTB (PB16 y 17 son Rx y Tx) y PORTC
	PORTB_PCR16 =0x00000300; //Hab clk PB16 Rx
	PORTB_PCR17 =0x00000300; //Hab clk PB17 Tx
	PORTC_PCR3 =0x00000300; 	//Hab clk PC3 Rx
	PORTC_PCR4 =0x00000300; 	//Hab clk PC4 Tx

	// LED
	SIM_SCGC5 |= 0x00002400;			//HABILITAR RELOJ
	PORTB_PCR21 =0X00000100;		//Seleccionar pin como GPIO
	PORTB_PCR22 =0X00000100;		//Seleccionar pin como GPIO
	PORTE_PCR26 =0X00000100;		//Seleccionar pin como GPIO

	GPIOB_PDDR |= (1 << 21);

	PORTC_PCR2 =(1<<8);
	GPIOC_PDDR = (1 << 2);

	SIM_SCGC3 |= (1 << 27);  //Clock ADC1
	ADC1_CFG1 = 0xC;   //Conv 16 bits
	ADC1_SC1A =18;

}

void vADC_LPTMR_init(void) {

	SIM_SCGC5 = (1 << 11) + (1 << 10) + 1; //reloj PORTC y LPTMR0

	LPTMR0_PSR = 5; //101 Bypass, clock= 1KHz (1ms) (1 = si es bypass, 01 =1khz) -PREESCALER
	LPTMR0_CSR = (1 << 6) + 1; //Hab LPTMR y Hab local de interrupcion -INTERRUPCION
	LPTMR0_CMR = 5000; //usando clock 1khz 0<=rango<= 65seg

	NVICICER1 =(1<<58%32); //1 del reg.1 .. escribir un 1 en esa posición //apagar banderas pendientes
	NVICISER1 |=(1<<58%32); //	Hab NVIC //el OR es para modificar solo 1 y dejar las otras como estaban
	NVICISER2 |=(1<<(73%32));	//Hab NVIC para ADC1

}

void vUART_send(unsigned char dato)				// Metodo para enviar datos
{
	do {
	} while (!(UART1_S1 & UART_S1_TDRE_MASK));
	UART1_D = dato; 								//Imprime dato en pantalla
}

unsigned char u8UART_receive(void)			// Metodo para leer datos recibidos
{
	do {
	} while (!(UART1_S1 & UART_S1_RDRF_MASK));
	unsigned char receiveValue = UART1_D;// Lee contenido de UART1_D, lo almacena en receiveValue y lo limpia
	UART0_D = receiveValue;
	return receiveValue;
}

void vUART_receive_buffer(void)		// Buffer para guardar carcteres entrantes
{
	unsigned char i = 0;
	unsigned long cont = 0;
	do {
		buffer[i] = u8UART_receive();
	} while ((i < 80) && (++cont <= 10000000));
}

void vUART_send_msg_AT(void)		// Envia mensaje "AT"
{
	unsigned char i = 0;
	do {
		vUART_send(mens0[i]);
	} while (mens0[++i] != 0);
}

void vUART_send_msg_CW(void)		// Envia mensaje "AT+CWMODE=1"
{
	unsigned char i = 0;
	do {
		vUART_send(mens1[i]);
	} while (mens1[++i] != 0);
}

void vUART_send_msg_setWifi(void)	// Envia mensaje "AT+CWJAP"
{
	unsigned char i = 0;
	do {
		vUART_send(mens2[i]);
	} while (mens2[++i] != 0);
}

void vUART_send_msg_setIP(void)	// Envia mensaje "AT+CIPSTA_DEF"
{
	unsigned char i = 0;
	do {
		vUART_send(mens3[i]);
	} while (mens3[++i] != 0);
}

void vUART_send_msg_CIFSR(void)	// Envia mensaje "AT+CIFSR"
{
	unsigned char i = 0;
	do {
		vUART_send(mens4[i]);
	} while (mens4[++i] != 0);
}

void vUART_send_msg_mux(void)		// Envia mensaje "AT+CIPMUX=1"
{
	unsigned char i = 0;
	do {
		vUART_send(mens5[i]);
	} while (mens5[++i] != 0);
}

void vUART_send_msg_Server(void)	// Envia mensaje "AT+CIPSERVER=1,80"
{
	unsigned char i = 0;
	do {
		vUART_send(mens6[i]);
	} while (mens6[++i] != 0);
}

void vUART_send_msg_TCP(void)	// 
{
	unsigned char i = 0;
	do {
		vUART_send(mens65[i]);
	} while (mens65[++i] != 0);
}

void vUART_send_msg_Reply(void)	// 
{
	unsigned char i = 0;
	do {
		vUART_send(mens7[i]);
	} while (mens7[++i] != 0);

}

void vUART_send_msg_status(unsigned char j)	// 
{
	unsigned char i = 0;
	do {
		vUART_send(mensStatus[j][i]);
	} while (mensStatus[j][++i] != 0);
}

void vUART_send_msg_close(unsigned char num)	// 
{
	unsigned char i = 0;

	do {
		vUART_send(close[num][i]);
	} while (close[num][++i] != 0);

}

unsigned char u8parser_OK(void)  // Parser busca la palabra "OK"
{
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned long cont;
	unsigned char temp;

	do {
		cont = 0;
		do {
		} while ((!(UART1_S1 & UART_S1_RDRF_MASK)) && (++cont <= 1000000));

		if (cont != 1000000) {
			temp = UART1_D;
			UART0_D = temp;
			buffer[j++] = temp;

			if (cmpmens2[i] == temp)
				i++;
			else
				i = 0;
		}

	} while ((cmpmens2[i] != 0));
	if (cont == 1000000) {
		return 0;
	} else
		return 1;
}

unsigned char u8parser_KeyEquipo(void)    // Parser busca la palabra "Equipo2"
{
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned long cont;
	unsigned char temp;

	do {
		cont = 0;
		do {
		} while ((!(UART1_S1 & UART_S1_RDRF_MASK)) && (++cont <= 1000000));

		if (cont != 1000000) {
			temp = UART1_D;
			buffer[j++] = temp;
			UART0_D = temp;
			if (cmpmens3[i] == temp)
				i++;
			else
				i = 0;
		}

	} while ((cmpmens3[i] != 0));

	if (cont == 1000000)
		return 0;
	else
		return 1;
}

unsigned char u8parser_mensaje(void)    // Parser busca la palabra ">"
{
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned long cont;
	unsigned char temp;

	do {
		cont = 0;
		do {
		} while ((!(UART1_S1 & UART_S1_RDRF_MASK)) && (++cont <= 1000000));

		if (cont != 1000000) {
			temp = UART1_D;
			buffer[j++] = temp;
			UART0_D = temp;
			if (cmpmens4[i] == temp)
				i++;
			else
				i = 0;
		}

	} while ((cmpmens4[i] != 0));
	if (cont == 1000000)
		return 0;
	else
		return 1;
}

void LPTimer_IRQHandler(void) {
	LPTMR0_CSR |= (1 << 7); //Apagar bandera TIMER
	ADC1_SC1A =(1<<6)+18;

}

void ADC1_IRQHandler(void) {

	ADC1_SC1A =(1<<7)+0x1F;	//coco
	adc_value = ADC1_RA;//

	if (adc_value<ON && automatico == '1') {
		GPIOB_PDOR=0x00000000;			//ENCIENDE TTTT
		GPIOC_PDOR=0x0;

	}
	else if(adc_value>ON && automatico == '1') {
		GPIOB_PDOR=(1<<21);
		GPIOC_PDOR=(1<<2);
	}

	//Prepararse para enviar Ascii
	captura[5]=(adc_value/10000);
	captura[4]=((adc_value-(captura[5]*10000))/1000);
	captura[3]=((adc_value-((captura[5]*10000)+(captura[4]*1000)))/100);
	captura[2]=((adc_value-((captura[5]*10000)+(captura[4]*1000)+(captura[3]*100)))/10);
	captura[0]=(captura[5]*10000)+(captura[4]*1000)+(captura[3]*100)+(captura[2]*10);
	captura[1]=adc_value-captura[0];

	unsigned char i=5;
	do {
		vUART_send(captura[i]+48);
		i--;
	}
	while (i!=0);

	vUART_send('\r');
	vUART_send('\n');

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
	vUART_init();										// Inicializa los Pins

	vUART_send_msg_AT();								// Envia mensaje AT
	if (u8parser_OK() == 1) {							// Espera respuesta OK	
		vUART_send_msg_CW();							// Configura CWMODE
		if (u8parser_OK() == 1) {	  					// Espera respuesta OK
			vUART_send_msg_setWifi();					// Configura red WiFi
			if (u8parser_OK() == 1) {					// Espera respuesta OK
				vUART_send_msg_setIP();				// Configura IP estatica
				if (u8parser_OK() == 1) {				// Espera respuesta OK
					vUART_send_msg_CIFSR();	// Envia mensaje CIFSR para verificar IP
					if (u8parser_OK() == 1) {			// Espera respuesta OK
						vUART_send_msg_mux();			// Configura modo CIPMUX
						if (u8parser_OK() == 1) {			// Espera OK
							vUART_send_msg_Server();	// Levanta el servidor
							if (u8parser_OK() == 1) {		// Espera OK
								
								do {// Do corre mientras no reciba parametro Z

									if (automatico == '1') {
										vADC_LPTMR_init();
									}

									if (LED != 'S' && LED != 'M') {
										vUART_send_msg_TCP();// Establece conexion TCP 
										if (u8parser_OK() == 1) { // Espera OK
											vUART_send_msg_Reply(); // Envia comando CIPSEND
											if (u8parser_mensaje() == 1) {
											if (adc_value < ON
													&& automatico == '1')
											vUART_send_msg_status(1); // Envia query
											else if (adc_value > ON
													&& automatico == '1')
											vUART_send_msg_status(0);// Envia query
											else if (isOn == 'Y'
													&& automatico == '0')
											vUART_send_msg_status(1);// Envia query
											else if (isOn == 'N'
													&& automatico == '0')
											vUART_send_msg_status(0);// Envia query
											else if (adc_value < ON)
											vUART_send_msg_status(1);// Envia query
											else if (adc_value > ON)
											vUART_send_msg_status(0);
										}
									}
								}

								if (u8parser_KeyEquipo() == 1) {
									LED = u8UART_receive();
									if (LED == 'M') {
										automatico = '0';
									} else if (LED == 'S') {
										automatico = '1';
									} else if (LED == 'R'
											&& automatico == '0') {
										GPIOC_PDOR = 0x0;
										GPIOB_PDDR = 0x00400000;
										GPIOE_PDDR = 0x00000000;
										isOn = 'Y';
									} else if (LED == 'A'
											&& automatico == '0') {
										GPIOC_PDOR = (1 << 2);
										GPIOB_PDDR = 0x00000000;
										GPIOE_PDDR = 0x00000000;
										isOn = 'N';
									}
								}


							} while (km < 100);

						}
					}
				}
			}
		}
	}
}
return 0;
}
