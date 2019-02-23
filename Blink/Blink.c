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

int32_t LED_Initialize (int32_t GPIOB_PIN1, int32_t GPIOB_PIN2) {

	// GPIOB clock'a bagli oldugu icin aktif etmek gerekir
	// bunun icin "AHB peripheral clock enable register"in ilgili bit'ini 1 yapmaliyiz
	// GPIOB icin 2. bit'i 1 yapmak icin unsigned long 1 tanimlanir ve 1 bit kaydirilir
	// diger bitleri degistirmemek icin 'veya' operatoru ile karsilastirilir
  RCC->AHBENR |=  (1UL <<  1);                  /* Enable GPIOB clock         */

  /* Configure LED (PB.GPIOB_PIN1, GPIOB_PIN2) pins as push-pull outputs, No pull-up, pull-down */
	// GPIOB portunun ilgili pinlerini cikis yapabilmek modunu ve tipini belirtmeliyiz
	// "GPIO register"larin ilgili bitlerini yukaridaki sekilde karsilastirma operatorleri ile
	// degistirmek mumkundur
  GPIOB->MODER   &= ~((3UL << 2*GPIOB_PIN1) | (3UL << 2*GPIOB_PIN2)); // reset mode
  GPIOB->MODER   |=  ((1UL << 2*GPIOB_PIN1) | (1UL << 2*GPIOB_PIN2)); // set General purpose output mode
	
  GPIOB->OTYPER  &= ~((1UL <<   GPIOB_PIN1) | (1UL <<   GPIOB_PIN2)); // set open-drain
	
  GPIOB->OSPEEDR &= ~((3UL << 2*GPIOB_PIN1) | (3UL << 2*GPIOB_PIN2)); // reset speed
  GPIOB->OSPEEDR |=  ((1UL << 2*GPIOB_PIN1) | (1UL << 2*GPIOB_PIN2)); // set medium speed
	
  GPIOB->PUPDR   &= ~((3UL << 2*GPIOB_PIN1) | (3UL << 2*GPIOB_PIN2)); // no pull-up, no pull-down

  return (0);
}

int32_t LED_On (uint32_t GPIOB_PIN_NUMBER) {
	/* ------------------------------------------------------------------
	GPIO_B bit set/reset register (BSRR)
	unsigned long 1 tanimlanip pin numarasi kadar kaydiriliyor
	boylece 'set register' 1 yapilmis olunuyor ve cikis HIGH yapiliyor
	------------------------------------------------------------------ */
	GPIOB->BSRR |= (1UL << GPIOB_PIN_NUMBER);
	
  return (0);
}

int32_t LED_Off (uint32_t GPIOB_PIN_NUMBER) {
	/* ------------------------------------------------------------------
	GPIO_B bit set/reset register (BSRR)
	unsigned long 1 tanimlanip pin numarasi kadar kaydiriliyor
	'reset register'a ulasmak icin 16 bit kaydirmak gerekmektedir
	boylece 'reset register' 1 yapilmis olunuyor ve cikis LOW yapiliyor
	------------------------------------------------------------------ */
	GPIOB->BSRR |= ((1UL << GPIOB_PIN_NUMBER) << 16);
	
  return (0);
}

int main (void) {
	short LED1 = 6;			// board uzerinde bulunan LD4 led'inin bagli oldugu pin
	short LED2 = 7;			// board uzerinde bulunan LD3 led'inin bagli oldugu pin
	
  SystemCoreClockConfigure();
  SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
	
  LED_Initialize(LED1, LED2);                   /* Initialize Led Settings    */

  if (SysTick_Config(SystemCoreClock / 100)) {  /* SysTick 10 msec interrupts */
    while (1);                                  /* Capture error              */
  }

  while(1) {                                    /* Loop forever               */
      LED_On (LED1);
      Delay(40);                                /* Delay 400ms                */
      LED_Off(LED1);
      Delay(20);                                /* Delay 200ms                */
		
      LED_On (LED2);
      Delay(40);                                /* Delay 400ms                */
      LED_Off(LED2);
      Delay(20);                                /* Delay 200ms                */
  }

}
