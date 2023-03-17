


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "LCD20x4.h"
#define GPIO_PA0_U0RX 0x00000001
#define GPIO_PA1_U0TX 0x00000401
#define GPIO_PORTF_LOCK_R (*((volatile uint32_t *)0x40025520))
#define GPIO_PORTF_CR_R (*((volatile uint32_t *)0x40025524))
#define entrada_normais (*((volatile long *)0x40025040))//PF4
#define entrada_vagaPCD (*((volatile long*)0x40025004))//PF0
#define saida_vaganormais (*((volatile long *)0x40005200))//PB7
#define saida_vagaPCD (*((volatile long*)0x40005100))//PB6
#define CARROIN (*((volatile long*)0x40004100))//PA6
#define CARROOUT (*((volatile long*)0x40004200))//PA7
#define ticket (*((volatile long*)0x40007100))//PD6
#define cancela_aberta (*((volatile long*)0x40007004))//PD0
#define PASSAGEM_CARRO (*((volatile long*)0x40007008))//PD1
#define cancelaaberta_saida (*((volatile long*)0x40024010))//PE2
#define PASSAGEM_CARRO_SAIDA (*((volatile long*)0x40024020))//PE3
#define PF1 (*((volatile long *) 0x40025008))//LED
#define GPIO_PB4_T1CCP0 0x00011007

void config_PORTAS(void);
void Timer1A_IntHandler(void);
void config_CONT(void);
void GPIO_F_IntHandler(void);
void config_INTPORTF(void);
void GPIO_B_IntHandler(void);
void config_INTPORTB(void);
void GPIO_A_IntHandler(void);
void config_INTPORTA(void);
void GPIO_D_IntHandler(void);
void config_INTPORTD(void);
void GPIO_E_IntHandler(void);
void config_INTPORTE(void);
void chamar_ticket(void);
void Config_UART0(void);
void config_TIMER(void);
void WTimer0A_IntHandler(void);
void padrao_timer(void);

//int flag_podesair=0;
//int vagas[20];
//int testevagas=0;
//int testeespeciais=0;
//int vagas_especiais[5];
//int travaPCD, trava=0;
int vagas_disponiveis=20;
int vagas_IDOSOSPCD=5;
int flag,flag2 =0;
int vaga_normal=20;
int vaga_especial=5;
int espera=0;
int total=0;
int i=0;
int flag_timer=0;
int x=0;

int main(void)
{
  SysCtlClockSet(SYSCTL_SYSDIV_16 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  //usa o como base de tempo o cristal de 16 MHz/16 = 1MHz (1 microssegundo).
  
  config_PORTAS();
  Config_UART0();
  config_TIMER();
  config_CONT();
  config_INTPORTF();
  config_INTPORTB();
  config_INTPORTA();
  config_INTPORTD();
  config_INTPORTE();
  config_CONT();
  inicializa();
  padrao_timer();
  while(1);
}
void config_TIMER(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
  ////Configuração do Timer0A para one shot – 16 bits////
  SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
  TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT);
  TimerLoadSet(WTIMER0_BASE, TIMER_A, 1310000-1); //valor em us
  TimerIntEnable(WTIMER0_BASE, TIMER_TIMA_TIMEOUT);
  IntEnable(INT_WTIMER0A_TM4C123);
  IntPrioritySet(INT_GPIOE_TM4C123, 0x50); //prioridade 
  IntRegister(INT_WTIMER0A_TM4C123, WTimer0A_IntHandler);
}
void padrao_timer(void)
{
  TimerEnable(WTIMER0_BASE, TIMER_A); //inicia
  PF1=0x02;
}
void Config_UART0(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  UARTStdioConfig(0, 115200, 16000000);
}
void WTimer0A_IntHandler(void) //ISR do Timer0A
{
  TimerIntClear(WTIMER0_BASE, TIMER_TIMA_TIMEOUT);
  //TimerDisable(WTIMER0_BASE, TIMER_A); 
  flag_timer=1;
  PF1=0; 
}
void Timer1A_IntHandler(void)
{
  TimerIntClear(TIMER0_BASE, TIMER_CAPA_MATCH);
}
void GPIO_F_IntHandler(void)
{
  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);
  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4);
  IntDisable(INT_GPIOF_TM4C123);
  
  if(flag_timer==1)
  {
    flag_timer=0;
    if(espera>=1)
    {
      
      if(entrada_vagaPCD==0 && vagas_IDOSOSPCD > 0)
      {
        
        UARTprintf("\n\rVAGA ESPECIAL UTILIZADA");
        vagas_IDOSOSPCD--;
        UARTprintf("---------------Vagas disponíveis: %d",vagas_IDOSOSPCD);
        espera--;
        UARTprintf("           Carros aguardando vaga: %d\n",espera);
        
      }
      else if(entrada_normais==0 && vagas_disponiveis > 0)
      {
        UARTprintf("\n\rVAGA UTILIZADA");
        vagas_disponiveis--;
        UARTprintf("---------------Vagas disponíveis: %d",vagas_disponiveis);
        espera--;
        UARTprintf("                   Carros aguardando vaga: %d\n",espera);
      }
      
      IntEnable(INT_GPIOB_TM4C123);
      
    }
    
    
    padrao_timer();
  }  
  IntEnable(INT_GPIOF_TM4C123);
}
void GPIO_B_IntHandler(void)
{
  IntDisable(INT_GPIOD_TM4C123); 
  GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_6);
  GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_7);
  IntDisable(INT_GPIOB_TM4C123);
  
  if(flag_timer==1)
  {
    flag_timer=0;
    if(saida_vagaPCD==0 && vagas_IDOSOSPCD < 5)
    {
      UARTprintf("\n\rVAGA ESPECIAL LIBERADA");
      vagas_IDOSOSPCD++;
      UARTprintf("-----------Vagas disponíveis: %d",vagas_IDOSOSPCD);
      espera++; 
      if(vagas_IDOSOSPCD==5){UARTprintf("\n********Todas as vagas estão livres!\n");}
      UARTprintf("           Carros aguardando vaga: %d\n",espera);
    }
    else if(saida_vaganormais==0 && vagas_disponiveis < 20 )
    {
      UARTprintf("\n\rVAGA LIBERADA");
      vagas_disponiveis++;
      UARTprintf("----------------Vagas disponíveis: %d",vagas_disponiveis);
      espera++; 
      if( vagas_disponiveis==20){UARTprintf("\n********Todas as vagas estão livres!");}
      UARTprintf("                   Carros aguardando vaga: %d\n",espera);
    }
    
    
    IntEnable(INT_GPIOD_TM4C123);
    padrao_timer();
  } 
  
  IntEnable(INT_GPIOB_TM4C123);
  
}
void GPIO_A_IntHandler(void)
{
  GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_6);
  GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_7);
  GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_0);
  IntDisable(INT_GPIOA_TM4C123);
  
  limpa();
  if(flag_timer==1)
  {
    flag_timer=0;
    if(CARROIN==0 && total<=25)
    {
      
      UARTprintf("\x1B[2J\x1B[0;0H");
      UARTprintf("\r**********ENTRADA!\n");
      UARTprintf("\rUm carro acabou de chegar!");
      
      GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_6);
      IntEnable(INT_GPIOD_TM4C123);
      GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_6);
      GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_0);
      escreve (0x80, "Seja bem vindo!");
      escreve (0x94, "Retire seu ticket...");
      
      UARTprintf("\rEsperando usuário retirar o ticket...");
      
    }
    else if(CARROIN==0 && total>25)
    {
      
      UARTprintf("\n\rEstacionamento está lotado!");   
      escreve (0x94, "Estacionamento lotado!");
      escreve (0xD4, "lotado!");
    }
    if(CARROIN!=0 || CARROOUT!=0)
    {
      if(CARROIN!=0)
      {
        IntDisable(INT_GPIOD_TM4C123);
        GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_6);
        GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_0);
        GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_1);
        UARTprintf("\x1B[2J\x1B[0;0H");
        UARTprintf("\rNão há carros na ENTRADA!");
      }
      if(CARROOUT!=0)
      {
        IntDisable(INT_GPIOE_TM4C123);
        GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
        GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_2);
        GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_3);
        UARTprintf("\x1B[2J\x1B[0;0H");
        UARTprintf("\rNão há carros na SAIDA!");
      }
      
      UARTprintf("\x1B[2J\x1B[0;0H");
      limpa();
      escreve (0x94, "A normal day...");
      
      UARTprintf("                                         Espera: %d",espera);
      
    }
    
    if(total!=0 && espera!=0)
    {
      
      
      if(CARROOUT==0)
      {
        if(i<=espera)
        {
          i++;
          GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
          GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1);
          GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_2);
          
          IntEnable(INT_GPIOE_TM4C123); 
          UARTprintf("\x1B[2J");
          UARTprintf("\x1B[0;0H");
          UARTprintf("\r**********SAIDA!\n");
          escreve (0x94, "Posisione seu ticket");
          escreve (0xD4, "no leitor!");
          UARTprintf("\rCarro posicionado na saída!\n");
        }
        if(i==espera)i=0;
      }
      
    }
    padrao_timer();
  }
  
  IntEnable(INT_GPIOA_TM4C123);
  
  
}

void GPIO_D_IntHandler(void)
{
  
  GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_6);
  GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_0);
  IntDisable(INT_GPIOD_TM4C123);
  
  GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_6);
  GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_0);
  
  IntDisable(INT_GPIOB_TM4C123);
  
  if(flag_timer==1)
  {
    flag_timer=0;
    
    if(cancela_aberta==0)
    {
      limpa(); 
      
      UARTprintf("\n\rCancela aberta!\n");
      
      escreve (0x94, "Cancela aberta!");
      GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_0);
      GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_1);
      GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_1);
      flag=1;
    }
    else if(flag==0)
    {
      limpa();
      escreve (0x94, "Ticket retirado!");
      UARTprintf("\n\rTicket retirado!");
    }
    if(PASSAGEM_CARRO==0)
    { 
      limpa();
      TimerEnable(TIMER1_BASE, TIMER_A);
      flag=0;
      UARTprintf("\n\rCarro passou pela cancela!");
      x=TimerValueGet(TIMER1_BASE, TIMER_A); 
      UARTprintf("*****************Hoje já entraram: %d carro(s)\n",x);
      escreve (0x94, "Faca um bom passeio!");
      SysCtlDelay(1000000);
      TimerDisable(TIMER1_BASE, TIMER_A);
      GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_1);
      GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_0);
      
      
      total++;
      espera++;
      
      
      IntEnable(INT_GPIOB_TM4C123);
      UARTprintf("\x1B[2J\x1B[0;0H");
    }
    padrao_timer();
    
    IntEnable(INT_GPIOF_TM4C123);
  }
  
  IntEnable(INT_GPIOD_TM4C123); 
}
void GPIO_E_IntHandler(void)
{
  
  GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
  GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_2);
  
  GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
  GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_2);
  IntDisable(INT_GPIOE_TM4C123); 
  UARTprintf("\x1B[2J\x1B[0;0H");
  
  if(flag_timer==1)
  {
    flag_timer=0;
    limpa(); 
    
    if(cancelaaberta_saida==0)
    {
      UARTprintf("\rCancela aberta!\n");
      
      escreve (0x94, "Cancela aberta!");
      GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_2);
      GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_3);
      GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_3);
      flag2=1;
    }
    else if(flag2==0)
    {
      
      escreve (0x94, "Verificando ticket..");
      UARTprintf("\rVerificando ticket...\n");
    }
    if(PASSAGEM_CARRO_SAIDA==0)
    {
      limpa();
      flag2=0;
      UARTprintf("\rCarro deixou o estacionamento!\n");
      escreve (0x94, "Volte sempre!");
      GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_2);
      GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_3);
      
      
      total--;
      espera--;
      
      UARTprintf("\x1B[2J\x1B[0;0H");   
    }
    padrao_timer();
  }
  
  IntEnable(INT_GPIOE_TM4C123);  
}

void config_CONT(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
  TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP);
  
  TimerControlEvent(TIMER1_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
  TimerMatchSet(TIMER1_BASE, TIMER_A, 100);//valor alvo
  IntEnable(INT_TIMER1A_TM4C123);
  IntPrioritySet(INT_TIMER1A_TM4C123, 0x70);
  TimerIntEnable(TIMER1_BASE, TIMER_CAPA_MATCH);
  
  IntRegister(INT_TIMER1A_TM4C123, Timer1A_IntHandler);
}
void config_INTPORTF(void)
{
  GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4);
  GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0);
  IntPrioritySet(INT_GPIOF_TM4C123, 0x70); //prioridade 
  
  IntRegister(INT_GPIOF_TM4C123, GPIO_F_IntHandler);
}
void config_INTPORTB(void)
{
  GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_6);
  GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_7);
  IntPrioritySet(INT_GPIOB_TM4C123, 0x70); //prioridade 
  
  IntRegister(INT_GPIOB_TM4C123, GPIO_B_IntHandler);
}
void config_INTPORTA(void)
{
  GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_BOTH_EDGES);
  GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_6);
  GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_BOTH_EDGES);
  GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_7);
  IntPrioritySet(INT_GPIOA_TM4C123, 0x70); //prioridade 
  IntEnable(INT_GPIOA_TM4C123);
  IntRegister(INT_GPIOA_TM4C123, GPIO_A_IntHandler);
}
void config_INTPORTD(void)
{
  GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
  
  GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_0);
  GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
  GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_1);
  IntPrioritySet(INT_GPIOD_TM4C123, 0x70); //prioridade 
  
  IntRegister(INT_GPIOD_TM4C123, GPIO_D_IntHandler);
}
void config_INTPORTE(void)
{
  GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
  GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1);
  GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_FALLING_EDGE);
  GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_2);
  GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
  GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_3);
  IntPrioritySet(INT_GPIOE_TM4C123, 0x70); //prioridade 
  
  IntRegister(INT_GPIOE_TM4C123, GPIO_E_IntHandler);
  
}
void config_PORTAS(void)
{
  /***CONFIG PORTB - SW4, SW3 e CONTADOR(PIN 4)***/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_4); //PB4
  GPIOPinConfigure(GPIO_PB4_T1CCP0); //configura função alternativa T1CCP0
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);
  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  /***CONFIG PORTF - SW1 E SW2***/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // Desbloqueio do
  GPIO_PORTF_CR_R = 0x11; // pino PF0.
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  /***CONFIG PORTA - SENSORES CARROIN E CARROOUT ***/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);
  GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  /***CONFIG PORTD - SENSORES TICKET E PASSAGEM DO CARRO***/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_0| GPIO_PIN_1);
  GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_0| GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  /***CONFIG PORTE - SENSORES TICKET E PASSAGEM DO CARRO***/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3);
  GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); 
}        

