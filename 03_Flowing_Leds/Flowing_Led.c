#include "stm32l1xx.h"                  // Device header

volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */

const int LEDS[] = {0, 1, 2, 3, 4, 5}; /* board uzerinde bulunan PA0..PA7 pinleri */
const int LED_NUMBER = 6;

const int _DELAY_TIME = 20;

void SysTick_Handler(void) {
 /*----------------------------------------------------------------------------
  SysTick_Handler -> her 10ms'de bir msTicks adli sayaci bir arttirir
 *----------------------------------------------------------------------------*/
  msTicks++;
}

void Delay (uint32_t dlyTicks) {
	/*----------------------------------------------------------------------------
  delays number of tick Systicks (happens every 10 ms)
	her 10ms'de bir msTicks degeri degisir ve yeni degerle eskisi karsilastirilir
	verilen dlyTick degeri boyunca bir sey yapmadan bekler
  *----------------------------------------------------------------------------*/
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) { __NOP(); }
}


static void SystemCoreClockConfigure(void) {
/*----------------------------------------------------------------------------
  configure SystemCoreClock:
     System Clock source         PLL(HSI)
     SYSCLK                      32000000 Hz
     HCLK                        32000000 Hz
     AHB Prescaler               1
     APB1 Prescaler              1
     APB2 Prescaler              1
     HSI Frequency               16000000 Hz
     PLL DIV                     3
     PLL MUL                     6
     Flash Latency               1 WS
     SDIO clock (SDIOCLK)        48000000 Hz
     Require 48MHz for USB clock Disabled
	
	sistemin clock ayarlarini yapar
	clock ayarlari islemciyi kontrol etmek icin onemlidir.
 *----------------------------------------------------------------------------*/
  RCC->CR |= ((uint32_t)RCC_CR_HSION);               /* Enable HSI */
  while ((RCC->CR & RCC_CR_HSIRDY) != RCC_CR_HSIRDY) __NOP();

  FLASH->ACR |= FLASH_ACR_ACC64;                     /* Enable 64-bit access */
  FLASH->ACR |= FLASH_ACR_PRFTEN;                    /* Enable Prefetch Buffer */
  FLASH->ACR |= FLASH_ACR_LATENCY;                   /* Flash 1 wait state */

  RCC->APB1ENR |= RCC_APB1ENR_PWREN;                 /* Enable the PWR APB1 Clock         */
  PWR->CR = PWR_CR_VOS_0;                            /* Select the Voltage Range 1 (1.8V) */
  while((PWR->CSR & PWR_CSR_VOSF) != 0);             /* Wait for Voltage Regulator Ready  */

  RCC->CFGR |= (RCC_CFGR_HPRE_DIV1  |                /* HCLK = SYSCLK / 1 */
                RCC_CFGR_PPRE2_DIV1 |                /* PCLK2 = HCLK / 1 */
                RCC_CFGR_PPRE1_DIV2  );              /* PCLK1 = HCLK / 2 */

  RCC->CFGR &= ~(RCC_CFGR_PLLSRC   |                 /* clear settings */
                 RCC_CFGR_PLLMUL   |
                 RCC_CFGR_PLLDIV    );
  RCC->CFGR |=  (RCC_CFGR_PLLSRC_HSI |               /* configure PLL */
                 RCC_CFGR_PLLDIV3    |
                 RCC_CFGR_PLLMUL6     );


  RCC->CR |= RCC_CR_PLLON;                           /* Enable PLL */
  while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();     /* Wait till PLL is ready */

  RCC->CFGR &= ~(RCC_CFGR_SW);                       /* clear settings */
  RCC->CFGR |= RCC_CFGR_SW_PLL;                      /* Select PLL as system clock source */
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) __NOP();
}

int32_t GPIOA_Initialize(void) {
	RCC->AHBENR |=  (1UL <<  0);      /* Enable GPIOA clock */
	return (0);
}


int32_t LED_Initialize (int32_t GPIOA_PIN) {
  /* Configure LED (PortA.GPIO_PIN) pins as push-pull outputs, No pull-up, pull-down */
	
  GPIOA->MODER   &= ~(3UL << 2*GPIOA_PIN); // reset mode
  GPIOA->MODER   |=  (1UL << 2*GPIOA_PIN); // set General purpose output mode
  GPIOA->OTYPER  &= ~(1UL <<   GPIOA_PIN); // set open-drain
  GPIOA->OSPEEDR &= ~(3UL << 2*GPIOA_PIN); // reset speed
  GPIOA->OSPEEDR |=  (1UL << 2*GPIOA_PIN); // set medium speed
  GPIOA->PUPDR   &= ~(3UL << 2*GPIOA_PIN); // no pull-up, no pull-down
  return (0);
}

int32_t GPIOA_LED_Initialize(void){
	/* pinleri cikis olarak ayarlamak icin for dongusu kullaniliyor
		 her pin numarasi LED_Initialize metoduna gonderilerek o pine
	   ait register ayarlari yapiliyor */
	int i;
	for(i=0; i<LED_NUMBER; i++){
		LED_Initialize(LEDS[i]);
	}
	return (0);
}

int32_t LED_On (uint32_t GPIOA_PIN) {
	/* GPIOA_PIN'i HIGH yapmak icin ilgili register 1 yapiliyor */
	
	GPIOA->BSRR |= (1UL << GPIOA_PIN);
  return (0);
}

int32_t LED_Off (uint32_t GPIOA_PIN) {
	/* GPIOA_PIN'i LOW yapmak icin ilgili register 1 yapiliyor */
	
	GPIOA->BSRR |= ((1UL << GPIOA_PIN) << 16);
  return (0);
}

int FlowLeftToRight(void){
	/* ledler PA0'dan PA5'e dogru yanip soner */
	int counter;
	for(counter=0; counter<LED_NUMBER; counter++){
			LED_On(LEDS[counter]);
			Delay(_DELAY_TIME);
			LED_Off(LEDS[counter]);
		}
	return (0);
}

int FlowRightToLeft(void){
	/* ledler PA5'den PA0'a dogru yanip soner */
	int counter;
	for(counter=LED_NUMBER-1; counter>=0; counter--){
			LED_On(LEDS[counter]);
			Delay(_DELAY_TIME);
			LED_Off(LEDS[counter]);
		}
	return (0);
}

int main (void) {
	
  SystemCoreClockConfigure();
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
	
	GPIOA_Initialize();                   				/* Initialize GPIOA           */
	
  GPIOA_LED_Initialize();                       /* Initialize Led Settings    */

  if (SysTick_Config(SystemCoreClock / 100)) {  /* SysTick 10 msec interrupts */
    while (1);                                  /* Capture error              */
  }

  while(1) {                                    /* Loop forever               */
		
		FlowLeftToRight();
		FlowRightToLeft();
		
		}
  }
