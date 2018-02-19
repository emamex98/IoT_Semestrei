#include <derivative.h> /* include peripheral declarations */

unsigned char mens0[]={"AT\r\n"};
unsigned char mens1[]={"AT+CWMODE=1\r\n"};
unsigned char mens2[]={"AT+CWJAP=\"riot-equipo2\",\"ensemestreiaprendoymedivierto\"\r\n"};
unsigned char mens3[]={"AT+CIPSTA_DEF=\"172.16.102.133\",\"172.16.102.254\",\"255.255.255.128\"\r\n"};
unsigned char mens4[]={"AT+CIFSR\r\n"};
unsigned char mens5[]={"AT+CIPMUX=1\r\n"};
unsigned char mens6[]={"AT+CIPSERVER=1,80\r\n"};
unsigned char mens7[]={"AT+CIPSEND=0,4\r\n"};
unsigned char mens8[]={"OKAY"};

unsigned char cmpmens1[]={"ready\r\n"};
unsigned char cmpmens2[]={"OK\r\n"};
unsigned char cmpmens3[]={"Equipo2"};
unsigned char cmpmens4[]={">"};
unsigned char cmpmens5[]={"IPD"};

unsigned char buffer[300];
unsigned char LED;

void vUART_init(void)
{
    SIM_SCGC4=0x00000C00; 	//Hab clk UART0 y UART1
    UART0_BDH=0;
    UART0_BDL=11;   		//115200 bps
    UART1_BDH=0;
    UART1_BDL=11;    		//115200 bps

    UART0_C2=12; 			// bit 3: Hab Tx, bit 2: Hab Rx
    UART1_C2=12;

    //Pines
    SIM_SCGC5=0x00000C00; 	//Hab clk PORTB (PB16 y 17 son Rx y Tx) y PORTC
    PORTB_PCR16=0x00000300; //Hab clk PB16 Rx
    PORTB_PCR17=0x00000300; //Hab clk PB17 Tx
    PORTC_PCR3=0x00000300; 	//Hab clk PC3 Rx
    PORTC_PCR4=0x00000300; 	//Hab clk PC4 Tx
    
    // LED
    SIM_SCGC5|=0x00002400;			//HABILITAR RELOJ
    	PORTB_PCR21=0X00000100;		//Seleccionar pin como GPIO
    	PORTB_PCR22=0X00000100;		//Seleccionar pin como GPIO
    	PORTE_PCR26=0X00000100;		//Seleccionar pin como GPIO
}


void vUART_send(unsigned char dato)				// Metodo para enviar datos
{
  do{}while(!(UART1_S1 & UART_S1_TDRE_MASK));
  UART1_D = dato; 								//Imprime dato en pantalla
}

unsigned char u8UART_receive (void)				// Metodo para leer datos recibidos
{
  do{}while (!(UART1_S1 & UART_S1_RDRF_MASK));
  unsigned char receiveValue = UART1_D;			// Lee contenido de UART1_D, lo almacena en receiveValue y lo limpia
  UART0_D=receiveValue;
  return receiveValue;
}

void vUART_receive_buffer (void)				// Buffer para guardar carcteres entrantes
{
  unsigned char i=0;
  unsigned long cont=0;
  do{
    buffer[i]=u8UART_receive();
  }while ((i<80)&& (++cont<=10000000));
}

void vUART_send_msg_AT (void)		// Envia mensaje "AT"
{
  unsigned char i=0;
  do{
    vUART_send(mens0[i]);
  }while (mens0[++i]!=0);
}

void vUART_send_msg_CW (void)		// Envia mensaje "AT+CWMODE=1"
{
  unsigned char i=0;
  do{
    vUART_send(mens1[i]);
  }while (mens1[++i]!=0);
}

void vUART_send_msg_setWifi (void)	// Envia mensaje "AT+CWJAP"
{
  unsigned char i=0;
  do{
    vUART_send(mens2[i]);
  }while (mens2[++i]!=0);
}

void vUART_send_msg_setIP (void)	// Envia mensaje "AT+CIPSTA_DEF"
{
  unsigned char i=0;
  do{
    vUART_send(mens3[i]);
  }while (mens3[++i]!=0);
}

void vUART_send_msg_CIFSR (void)	// Envia mensaje "AT+CIFSR"
{
  unsigned char i=0;
  do{
    vUART_send(mens4[i]);
  }while (mens4[++i]!=0);
}

void vUART_send_msg_mux (void)		// Envia mensaje "AT+CIPMUX=1"
{
  unsigned char i=0;
  do{
    vUART_send(mens5[i]);
  }while (mens5[++i]!=0);
}

void vUART_send_msg_Server (void)	// Envia mensaje "AT+CIPSERVER=1,80"
{
	unsigned char i=0;
	  do{
	    vUART_send(mens6[i]);
	  }while (mens6[++i]!=0);
}

void vUART_send_msg_Reply (void)	// 
{
	unsigned char i=0;
	  do{
	    vUART_send(mens7[i]);
	  }while (mens7[++i]!=0);
}

void vUART_send_msg_status (void)	// 
{
	unsigned char i=0;
	  do{
	    vUART_send(mens8[i]);
	  }while (mens8[++i]!=0);
}

unsigned char u8parser_OK (void)  // Parser busca la palabra "OK"
{
  unsigned char i=0;
  unsigned char j=0;
  unsigned long cont;
  unsigned char temp;

  do{
    cont=0;
    do{}while ((!(UART1_S1&UART_S1_RDRF_MASK))&&(++cont<=1000000));

    if (cont!=1000000)
    {
    	temp=UART1_D;
    	UART0_D=temp;
        buffer[j++]=temp;
        
      if (cmpmens2[i]==temp) i++;
      else i=0;
    }
    
  }while ((cmpmens2[i]!=0));
  if (cont==1000000) return 0;
  else return 1;
}

unsigned char u8parser_KeyEquipo (void)    // Parser busca la palabra "Equipo2"
{
  unsigned char i=0;
  unsigned char j=0;
  unsigned long cont;
  unsigned char temp;

  do{
    cont=0;
    do{}while ((!(UART1_S1&UART_S1_RDRF_MASK))&&(++cont<=1000000));

    if (cont!=1000000)
    {
    	temp=UART1_D;
        buffer[j++]=temp;
        UART0_D=temp;
      if (cmpmens3[i]==temp) i++;
      else i=0;
    }
    
  }while ((cmpmens3[i]!=0));
  if (cont==1000000) return 0;
  else return 1;
}

unsigned char u8parser_mensaje (void)    // Parser busca la palabra "Equipo2"
{
  unsigned char i=0;
  unsigned char j=0;
  unsigned long cont;
  unsigned char temp;

  do{
    cont=0;
    do{}while ((!(UART1_S1&UART_S1_RDRF_MASK))&&(++cont<=1000000));

    if (cont!=1000000)
    {
    	temp=UART1_D;
        buffer[j++]=temp;
        UART0_D=temp;
      if (cmpmens4[i]==temp) i++;
      else i=0;
    }
    
  }while ((cmpmens4[i]!=0));
  if (cont==1000000) return 0;
  else return 1;
}

unsigned char u8parser_saveChannel (void)    // Parser busca la palabra "Equipo2"
{
  unsigned char i=0;
  unsigned char j=0;
  unsigned long cont;
  unsigned char temp;

  do{
    cont=0;
    do{}while ((!(UART1_S1&UART_S1_RDRF_MASK))&&(++cont<=1000000));

    if (cont!=1000000)
    {
    	temp=UART1_D;
        buffer[j++]=temp;
        UART0_D=temp;
      if (cmpmens5[i]==temp) i++;
      else i=0;
    }
  }while ((cmpmens4[i]!=0));
  if (cont==1000000) return 0;
  else return buffer[5];
}


int main(void)
{	
  vUART_init();										// Inicializa los Pins

  vUART_send_msg_AT();								// Envia mensaje AT
  if (u8parser_OK()==1){							// Espera respuesta OK	
	  vUART_send_msg_CW();							// Configura CWMODE
	  if(u8parser_OK()==1){	  						// Espera respuesta OK
		  vUART_send_msg_setWifi();					// Configura red WiFi
		  if(u8parser_OK()==1){						// Espera respuesta OK
			  vUART_send_msg_setIP();				// Configura IP estatica
			  if(u8parser_OK()==1){					// Espera respuesta OK
				  vUART_send_msg_CIFSR();			// Envia mensaje CIFSR para verificar IP
				  if(u8parser_OK()==1){				// Espera respuesta OK
					  vUART_send_msg_mux();			// Configura modo CIPMUX
					  if(u8parser_OK()==1){			// Espera OK
						  vUART_send_msg_Server();	// Levanta el servidor
						 do{									// Do corre mientras no reciba parametro Z
							 if(u8parser_KeyEquipo()==1){		// Busca palabra clave Equipo2
								 LED=u8UART_receive();			// Almacena el caracter despues de palabra clabe
								 if(LED=='M'){
									 GPIOB_PDDR=0x00600000;
								 	 GPIOE_PDDR=0x00000000;
								 	 vUART_send_msg_Reply();
								 	 if(u8parser_mensaje()==1)
								 		vUART_send_msg_status();
								 }
								 else if(LED=='R'){
									 GPIOB_PDDR=0x00400000;
									 GPIOE_PDDR=0x00000000;
									 vUART_send_msg_Reply();
									 if(u8parser_mensaje()==1)
									 	vUART_send_msg_status();
								 }
								 else if(LED=='V'){
									 GPIOB_PDDR=0x00000000;
									 GPIOE_PDDR=0x04000000; 
									 vUART_send_msg_Reply();
									 if(u8parser_mensaje()==1)
									 	vUART_send_msg_status();
								 }
								 else if(LED=='A'){
								 	 GPIOB_PDDR=0x00000000;
								 	 GPIOE_PDDR=0x00000000;
								 	 vUART_send_msg_Reply();
								 	if(u8parser_mensaje()==1)
								 		vUART_send_msg_status();
								 }
						 }
						}while(LED!='Z');
					  }
				  }
			  }
		  }
	  }
  }
  return 0;
}
