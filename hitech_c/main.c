#include <htc.h>
#include <string.h>
__CONFIG(INTIO & WDTEN & PWRTDIS & BORDIS & LVPDIS &
DEBUGDIS & MCLRDIS & UNPROTECT & CCPRB3 & FCMDIS & IESOEN);

#define	_XTAL_FREQ	8000000
//---------------------------------------------------------------------
//número de entradas analógicas
//---------------------------------------------------------------------
#define NUMERO_CANALES 6

//---------------------------------------------------------------------
//Constant Definitions
//---------------------------------------------------------------------
#define NODE_ADDR 0x02 // dirección I2C del sensor lego
//---------------------------------------------------------------------
// Buffer Length Definition
//---------------------------------------------------------------------
#define RX_BUF_LEN 0x50 //; Length of receive buffer
//---------------------------------------------------------------------
// Variable declarations
//---------------------------------------------------------------------
unsigned char Index;  // Index to receive buffer
unsigned char RXBuffer[RX_BUF_LEN]; //Holds rec'd bytes from master

bit desechar_lectura; //ha habido una petición I2c durante la medida, y la invalida
bit estado_anterior;
bit estado_actual;
bit RA0_presente; //hay un servo en la entrada RA0
bit RA1_presente;
bit RA2_presente;


//---------------------------------------------------------------------
// Vectors
//---------------------------------------------------------------------
void clear_receive_buffer(void)
{
	int counter;
	for (counter=0;counter<RX_BUF_LEN;counter++) {
		RXBuffer[counter]=0;
	}
}
void Setup()
{
	//
	// Initializes program variables and peripheral registers.
	//---------------------------------------------------------------------

	// ajuste de la frecuencia del reloj: 8 MHz
	OSCCON=0x70;
	
	//puesta a 1 de PCON. mientras los dos bits inferiores sigan a 1, significa
	//que no ha habido un reset normal (NOT_POR) ni un reset por fallo de 
	//alimentación (NOT_BOR)
	PCON=0x03;
	//POR=1; //es POR o NOT POR???
	//BOR=1; //es BOR o NOT BOR???
	
	//puesta a cero de los flags de interrupciones
	PIR1=0;
	
	//ajuste en entradas y salidas. Todo salidas excepto SDA, SCL, RA0, RA1, RA2
	PORTB=0;
	PORTA=0;
	TRISB=0x12; //RB1=SDA, RB4=SCL, RB6=AN5, RB7=AN6
	TRISA=0x07; //RA0, RA1 y RA2 entradas, el resto salidas
	ANSEL=0x00; //por defecto el puerto A queda como entradas analógicas. hay que
	            //cambiarlo si se quiere que sean digitales	
	
	//ajuste del módulo I2C
	SSPCON=0x36; //Setup SSP module for 7-bit
	SSPADD=NODE_ADDR; //address, slave mode
	SSPSTAT=0;
	SSPIE=1; //Enable interrupts
	
	//habilitar interrupciones
	PEIE=1; //Enable all peripheral interrupts
	GIE=1; //Enable global interrupts
	RP0=0;

	//ajuste del Timer1
	T1CON=0x34; //T1RUN=0 -> el reloj del sistema no sale del Timer1
				//T1CKPS=11 -> prescalado a 1/8
				//T1OSCEN=0 -> desconecta el módulo del oscilador externo
				//T1SYNC=1 -> desconecta el modo sincronizado
				//TMR1CS=0 -> timer conectado a reloj interno
				//TMR1ON=0 -> deshabilitado por ahora
	TMR1ON=1; //habilita Timer1
	
	//inicializar variables del programa
	Index=0;
	strcpy(RXBuffer,".V1.1   HITECHNCAccel.  "); //mapa de memoria del acelerómetro Hitechnic
	
	return;	
}

/* rutina inicial, sin detección de servos vacíos
		TMR1L=0;
		TMR1H=0;
		salir=0;
		desechar_lectura=0;
		estado_actual=RA2;
		while(salir==0)
		{
			estado_anterior=estado_actual;
			estado_actual=RA2;
			if (estado_anterior==0 && estado_actual==1) salir=1; //flanco ascendente en RA2
		} 
		TMR1ON=1; //habilita Timer1
		while (RA2==1); //espera a que termine el pulso
		TMR1ON=0; //inhabilita Timer1
		if (desechar_lectura==0) 
		{
			RXBuffer[0x44]=TMR1L;
			RXBuffer[0x47]=TMR1H;
		}
*/

void main(void)
{
	unsigned char salir;
	unsigned int contador; 
	RP1=0;
	RP0=1;
	Setup();
	RA0_presente=1; //empieza suponiendo que todos los servos están conectados
	RA1_presente=1;
	RA2_presente=1;
	contador=0;
	while (1)
	{
		CLRWDT(); //Clear the watchdog timer.
		//primer canal
		if (RA0_presente==1) {
			TMR1L=0;
			TMR1H=0;
			salir=0;
			desechar_lectura=0;
			TMR1ON=1; //habilita Timer1
			estado_actual=RA0;
			while(salir==0)
			{
				estado_anterior=estado_actual;
				estado_actual=RA0;
				if (estado_anterior==0 && estado_actual==1) { //flanco ascendente en RA0
					TMR1ON=0; //inhabilita Timer1
					TMR1L=0;
					TMR1H=0;					
					TMR1ON=1; //habilita Timer1
					salir=1; 
				}
				if (TMR1H>16) { //time-out, no hay servo 
					TMR1ON=0; //inhabilita Timer1
					RA0_presente=0; //no usar esta entrada
					salir=1; 
				}
			} 
			if (RA0_presente==1) {
				while (RA0==1 && TMR1H<2); //espera a que termine el pulso
				TMR1ON=0; //inhabilita Timer1
				if (desechar_lectura==0) 
				{
					RXBuffer[0x42]=TMR1L;
					RXBuffer[0x45]=TMR1H;
				}
			}
		}
		
		//segundo canal
		if (RA1_presente==1) {
			TMR1L=0;
			TMR1H=0;
			salir=0;
			desechar_lectura=0;
			TMR1ON=1; //habilita Timer1
			estado_actual=RA1;
			while(salir==0)
			{
				estado_anterior=estado_actual;
				estado_actual=RA1;
				if (estado_anterior==0 && estado_actual==1) { //flanco ascendente en RA1
					TMR1ON=0; //inhabilita Timer1
					TMR1L=0;
					TMR1H=0;					
					TMR1ON=1; //habilita Timer1
					salir=1; 
				}
				if (TMR1H>16) { //time-out, no hay servo 
					TMR1ON=0; //inhabilita Timer1
					RA1_presente=0; //no usar esta entrada
					salir=1; 
				}
			} 
			if (RA1_presente==1) {
				while (RA1==1 && TMR1H<2); //espera a que termine el pulso
				TMR1ON=0; //inhabilita Timer1
				if (desechar_lectura==0) 
				{
					RXBuffer[0x43]=TMR1L;
					RXBuffer[0x46]=TMR1H;
				}
			}
		}

		//tercer canal
		if (RA2_presente==1) {
			TMR1L=0;
			TMR1H=0;
			salir=0;
			desechar_lectura=0;
			TMR1ON=1; //habilita Timer1
			estado_actual=RA2;
			while(salir==0)
			{
				estado_anterior=estado_actual;
				estado_actual=RA2;
				if (estado_anterior==0 && estado_actual==1) { //flanco ascendente en RA2
					TMR1ON=0; //inhabilita Timer1
					TMR1L=0;
					TMR1H=0;					
					TMR1ON=1; //habilita Timer1
					salir=1; 
				}
				if (TMR1H>16) { //time-out, no hay servo 
					TMR1ON=0; //inhabilita Timer1
					RA2_presente=0; //no usar esta entrada
					salir=1; 
				}
			} 
			if (RA2_presente==1) {
				while (RA2==1 && TMR1H<2); //espera a que termine el pulso
				TMR1ON=0; //inhabilita Timer1
				if (desechar_lectura==0) 
				{
					RXBuffer[0x44]=TMR1L;
					RXBuffer[0x47]=TMR1H;
				}
			}
		}	
		
		contador++;
		if (contador>100) { //cada cierto tiempo...
			contador=0;
			RA0_presente=1; //...vuelve a comprobar la presencia del servo
			RA1_presente=1;
			RA2_presente=1;
		}
	}
}

//;---------------------------------------------------------------------
//; WriteI2C
//;---------------------------------------------------------------------
void WriteI2C(unsigned char data){
	while (BF==1); //buffer full. keep waiting.
    do {
    	WCOL=0; //Clear the WCOL flag.
    	SSPBUF=data;
    } while (WCOL==1);
    CKP=1;
}
//---------------------------------------------------------------------
//SSP_Handler
//---------------------------------------------------------------------
// The I2C code below checks for 5 states:
//---------------------------------------------------------------------
// State 1: I2C write operation, last byte was an address byte.
// SSPSTAT bits: S = 1, D_A = 0, R_W = 0, BF = 1
//
// State 2: I2C write operation, last byte was a data byte.
// SSPSTAT bits: S = 1, D_A = 1, R_W = 0, BF = 1
//
// State 3: I2C read operation, last byte was an address byte.
// SSPSTAT bits: S = 1, D_A = 0, R_W = 1 (see Appendix C for more information)
//
// State 4: I2C read operation, last byte was a data byte.
// SSPSTAT bits: S = 1, D_A = 1, R_W = 1, BF = 0
//
// State 5: Slave I2C logic reset by NACK from master.
// SSPSTAT bits: S = 1, D_A = 1, BF = 0 (see Appendix C for more information)
//
// For convenience, WriteI2C and ReadI2C functions have been used.
//----------------------------------------------------------------------

void SSP_Handler(void) {
	unsigned char dummy;
	unsigned char temp;
	temp=SSPSTAT & 0x2d; //Mask out unimportant bits in SSPSTAT.
	desechar_lectura=1;
	if (temp==0x09 ){
		//State1:                   Write operation, last byte was an
		//                          address, buffer is full.
		dummy=SSPBUF; //Do a dummy read of the SSPBUF.
		Index=0; //puesta a cero del puntero al buffer
		return;
	}
	if (temp==0x29 ){
		//State2:                 ; Write operation, last byte was data,
		//                          buffer is full.
		dummy=SSPBUF; //Get the byte from the SSP.
		if (Index==0) { //primer byte escrito. contiene la dirección a leer
			Index=dummy;
		} else {
			Index++; //para el resto de escrituras, incrementa el puntero
		}
		return;
	}
	if ((temp & 0x2C)==0x0c ){ //Mask BF bit in SSPSTAT
		//State3:                 ; Read operation, last byte was an address,
		WriteI2C(RXBuffer[Index]); //devuelve el byte solicitado
		Index++;
		return;
	}
	if (CKP==0 && temp== 0x2C) {
		//State4:                 ; Read operation, last byte was data,
		WriteI2C(RXBuffer[Index]); //devuelve el byte solicitado
		Index++;
		return;
	}
	if ((temp & 0x28)==0x28) {
		//State5:   NACK received when sending data to the master
		return;
	} else { //If we aren’t in State5, then something is wrong
		RA7=1; //Something went wrong! Set LED
		while (1); //and loop forever. WDT will reset device, if enabled.
	}
		

}

//---------------------------------------------------------------------
// Interrupt Code
//---------------------------------------------------------------------
void interrupt isr(void)
{
	if (SSPIF!=1) { //Isn't this a SSP interrupt?
		while(1); //No, just trap here.
	}
    SSPIF=0;
    SSP_Handler(); //service SSP interrupt.
}
