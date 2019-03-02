#include "stm32l1xx.h"                  // Device header

volatile uint32_t msTicks;                      /* counts 1ms timeTicks       */


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

int32_t Enable_GPIO_Clock(int32_t _GPIO){
	// Reference manuel'den 'RCC registers'a bak
	// istenilen GPIO Clock register biti 1 yapilarak enable hale gelitiriliyor
	RCC->AHBENR |=  (1UL << _GPIO);                    /* Enable _GPIO clock         */
	
	return (0);
}

int32_t Buttons_Initialize (int32_t GPIOA_PIN) {
	// Reference manuel'den 'GPIO registers'a bak
  GPIOA->MODER   &= ~(3UL << 2*GPIOA_PIN); // giris olarak kullanacagiz
  GPIOA->OSPEEDR &= ~(3UL << 2*GPIOA_PIN); // hizi resetliyoruz
  GPIOA->OSPEEDR |=  (1UL << 2*GPIOA_PIN); // medium hiza set ediliyor
  GPIOA->PUPDR   &= ~(3UL << 2*GPIOA_PIN); // no-pull up, no pull-down

  return (0);
}


int32_t Buttons_Uninitialize (int32_t GPIOA_PIN) {

  GPIOA->MODER    &= ~((3UL << 2*GPIOA_PIN)  );

  return (0);
}

uint32_t Buttons_GetState (int32_t GPIOA_PIN) {

  uint32_t val = 0;

  if ((GPIOA->IDR & (1UL << GPIOA_PIN)) != 0) {
    /* IDR register'inin degeri 0'dan farkliysa */
		/* butona basilmis demektir 								*/
    val |= 1;
  }

  return (val);
}

int32_t LED_Initialize (int32_t GPIOB_PIN) {

	/* Blink ornegine bak */
  GPIOB->MODER   &= ~((3UL << 2*GPIOB_PIN)); // reset mode
  GPIOB->MODER   |=  ((1UL << 2*GPIOB_PIN)); // set General purpose output mode
  GPIOB->OTYPER  &= ~((1UL <<   GPIOB_PIN)); // set open-drain
  GPIOB->OSPEEDR &= ~((3UL << 2*GPIOB_PIN)); // reset speed
  GPIOB->OSPEEDR |=  ((1UL << 2*GPIOB_PIN)); // set medium speed
  GPIOB->PUPDR   &= ~((3UL << 2*GPIOB_PIN)); // no pull-up, no pull-down

  return (0);
}

int32_t LED_On (uint32_t GPIOB_PIN_NUMBER) {
	/* Blink ornegine bak */
	GPIOB->BSRR |= (1UL << GPIOB_PIN_NUMBER);
	
  return (0);
}

int32_t LED_Off (uint32_t GPIOB_PIN_NUMBER) {
	/* Blink ornegine bak */
	GPIOB->BSRR |= ((1UL << GPIOB_PIN_NUMBER) << 16);
	
  return (0);
}

int main (void) {
	short LED = 6;			  // board uzerinde bulunan LD4 led'inin bagli oldugu pin (PB6)
	short BUTTON = 0;			// board uzerinde bulunan USER butonuna bagli oldugu pin (PA0)
	
  SystemCoreClockConfigure();
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
	
	Enable_GPIO_Clock(0);													/* Enable GPIOA						    */
	Enable_GPIO_Clock(1);													/* Enable GPIOB						    */
	
  LED_Initialize(LED);                   			  /* Initialize Led Settings    */
	Buttons_Initialize(BUTTON);            			  /* Initialize Button Settings */

  while(1) {                                    /* Loop forever               */
		
		if (Buttons_GetState(BUTTON) != 0){         /* Read Button State          */
			
			LED_On (LED);											        /* LED on                     */
			
		} else{
			
			LED_Off(LED);											        /* LED off                    */
		}
	}

}
