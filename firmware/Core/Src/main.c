/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>    // standardowa biblioteka wejścia/wyjścia używana do formatowania ciągów tekstowych za pomocą funkcji snprintf
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime = {0}; // struktura HAL przechowywująca atualną godzinę
RTC_DateTypeDef sDate = {0}; // struktura HAL przechowywująca aktualną datę

// zmienne przechowywujące bieżącą: godzinę, minutę oraz sekundę
int godziny = 0;
int minuty  = 0;
int sekundy = 0;

uint8_t stara_minuta = 99; // zmienna przechowywująca starną minutę
char bufor_lcd[32];        // tablica w której sprintf przechowuje foramt przzed wysłaniem

const char* dni_tygodnia[] = {"", "PONIEDZIALEK", "WTOREK", "SRODA", "CZWARTEK", "PIATEK", "SOBOTA", "NIEDZIELA"}; // tablica przechowująca dni tygodnia (pierwsze pole jest puste ze względu na sposób działania rtc gdzie poniedziałek to 1 a niedziela to 7)

// MASZYNA STANÓW I PRZYCISKI
typedef enum {          // definicje stanów zegara
    STAN_NORMALNY,       // zegar w stanie podstawowej pracy
    STAN_WYBORU_POLA,    // zegar w stanie wyboru pola edycji
    STAN_EDYCJI_WARTOSCI // zegar w stanie edytowania wartości
} StanZegara;

typedef enum { // definicje pól możliwych do edycji
    POLE_GODZINY,
    POLE_MINUTY,
    POLE_DZIEN,
    POLE_MIESIAC,
    POLE_ROK
} EdytowanePole;

StanZegara aktualny_stan = STAN_NORMALNY;   // zmienna pzrechowywująca aktualny stan zegara (zegar startuje w stanie zwykłym)
EdytowanePole aktualne_pole = POLE_GODZINY; // zmienna wskazująca na aktualizowane pole

// zmienne tymczasowe do przechowywania ustawień w trakcie edycji
int temp_godziny = 0;
int temp_minuty  = 0;
int temp_dzien   = 0;
int temp_miesiac = 0;
int temp_rok     = 0;

// flagi pomocnicze do wykrywania kliknięć (debouncing) 1=przycisk wciśnięty 0=przycisk zwolniony
uint8_t p1_wcisniety = 0;
uint8_t p2_wcisniety = 0;
uint8_t p3_wcisniety = 0;
uint8_t p4_wcisniety = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// FUNKCJE WYSWIETLACZA CYFR
void LED_Wyswietl_0(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, B, C, D
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_RESET);                          // zaświecenie segmentów E, F
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);                                         // zgaszenie segmentu G
}

void LED_Wyswietl_1(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_9, GPIO_PIN_SET);             // zgaszenie segmentów A, D
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_8, GPIO_PIN_RESET);           // zaświecenie segmentów B, C
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET); // zgaszenie segmentów E, F, G
}

void LED_Wyswietl_2(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, B, D
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);                             // zgaszenie segmentu C
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_7, GPIO_PIN_RESET);             // zaświecenie segmentów E, G
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);                             // zgaszenie segmentów F
}

void LED_Wyswietl_3(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, B, C, D
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET);                             // zgaszenie segmentów E, F
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);                                        // zaświecenie segmentu G
}

void LED_Wyswietl_4(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_9, GPIO_PIN_SET);             // zgaszenie segmentow A, D
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_8, GPIO_PIN_RESET);           // zaświecenie segmentów B, C
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);                         // zgaszenie segmentu E
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);           // zaświecenie segmentów F, G
}

void LED_Wyswietl_5(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, C, D
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);                             // zgaszenie segmentu B
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);                             // zgaszenie segmentu E
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);             // zaświecenie segmentów F, G
}

void LED_Wyswietl_6(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, C, D
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);                             // zgaszenie segmentu B
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET); // zaświecenie segmentów E, F, G
}

void LED_Wyswietl_7(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8, GPIO_PIN_RESET); // zaświecenie segmentów A, B, C
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);                             // zgaszenie segmentu D
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET); // zgaszenie segmentów E, F, G
}

void LED_Wyswietl_8(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, B, C, D
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);             // zaświecenie segmentów E, F, G
}

void LED_Wyswietl_9(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET); // zaświecenie segmentów A, B, C, D
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);                                         // zgaszenie segmentu E
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);                          // zaświecenie segmentów F, G
}

void LED_Wyswietl_Cyfre(int cyfra) { // funkcja switch do wyboru odpowiednij cyfry do wyświetlenia
    switch(cyfra) {
        case 0: LED_Wyswietl_0(); break;
        case 1: LED_Wyswietl_1(); break;
        case 2: LED_Wyswietl_2(); break;
        case 3: LED_Wyswietl_3(); break;
        case 4: LED_Wyswietl_4(); break;
        case 5: LED_Wyswietl_5(); break;
        case 6: LED_Wyswietl_6(); break;
        case 7: LED_Wyswietl_7(); break;
        case 8: LED_Wyswietl_8(); break;
        case 9: LED_Wyswietl_9(); break;
        default: break;
    }
}

// FUNKCJE OBSŁUGI WYŚWIETLACZA LCD
#define GROVE_LCD_ADDR (0x3E << 1) // definicja adresu sprzętowego LCD z przesunięciem o 1 bit

void grove_lcd_cmd(uint8_t cmd) { // wysłyłanie bajtu komendy startu do LCD
    uint8_t temp[2] = {0x80, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, GROVE_LCD_ADDR, temp, 2, 100);
    HAL_Delay(1);
}

void grove_lcd_data(uint8_t data) { // wysłanie bajtu pojedynczego znaku tekstu do LCD
    uint8_t temp[2] = {0x40, data};
    HAL_I2C_Master_Transmit(&hi2c1, GROVE_LCD_ADDR, temp, 2, 100);
}

void grove_lcd_init(void) { // procedura inicjalizacjia LCD
    HAL_Delay(50);
    grove_lcd_cmd(0x38); // tryb 2 linii, czcionka 5x8
    HAL_Delay(5);
    grove_lcd_cmd(0x0C); // włącz wyświetlacz, ukryj kursor
    HAL_Delay(1);
    grove_lcd_cmd(0x01); // wyczyść ekran
    HAL_Delay(5);
    grove_lcd_cmd(0x06); // przesunięcie kursora w prawo
}

void grove_lcd_send_string(char *str) { // wysłanie całego ciągu tekstowego do LCD
    while (*str) {
        grove_lcd_data((uint8_t)*str++);
    }
}

void grove_lcd_put_cur(int row, int col) { // ustawienie kursora na zadany znak w LCD
    uint8_t val = (row == 0) ? (0x80 + col) : (0xC0 + col);
    grove_lcd_cmd(val);
}

uint8_t Oblicz_Dzien_Tygodnia(uint8_t d, uint8_t m, uint16_t y) { // algorytm Zallera do automatycznego wyznaczenia dnia tygodnia
    uint16_t pelny_rok = 2000 + y;
    static const uint8_t t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) {
        pelny_rok -= 1;
    }
    uint8_t wynik = (pelny_rok + pelny_rok/4 - pelny_rok/100 + pelny_rok/400 + t[m-1] + d) % 7;
    return (wynik == 0) ? 7 : wynik; // konwersja wyniku dla niedzieli z 0 na 7
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);    // opóźnienie po starcie
  grove_lcd_init();  // konfiguracja modułu wyświetlacza LCD

  // RESTART I WYMUSZENIE ZASILANIA DOMENY BACKUP
  __HAL_RCC_PWR_CLK_ENABLE(); // włączenie zegara kontrolera zasilania
  HAL_PWR_EnableBkUpAccess(); // odblokowanie dostępu do RTC i rejestrów backupu

  // Konfiguracja startowa zegara po resecie zasilania (00:00:00)
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;  // automatyczna zmiana czasu letni/zimowy
  sTime.StoreOperation = RTC_STOREOPERATION_RESET; // reset operacji zapisu stanu

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) { // sprzętowy zapis czasu do rejestrów RTC
      grove_lcd_put_cur(0, 0);
      grove_lcd_send_string("ERR: TIME WRITE");
      while(1);
  }

  // konfiguracja startu daty poniedziałek 1 stycznia 2000
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Date = 1;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) { // sprzętowy zapis daty do rejestrów RTC
      grove_lcd_put_cur(0, 0);
      grove_lcd_send_string("ERR: DATE WRITE");
      while(1);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    // POBRANIE CZASU Z RTC
	    if (aktualny_stan == STAN_NORMALNY) { // odczyt tylko w stanie normalnym
	        // pobranie aktualnych wartości z RTC
	        HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	        HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	        // zapisanie pobranych wartości do zmiennych
	        godziny = sTime.Hours;
	        minuty  = sTime.Minutes;
	        sekundy = sTime.Seconds;
	    }

	    // NIEBLOKUJĄCA OBSŁUGA PRZYCISKÓW
	    // Przycisk 1 - MENU (Wejście / Ostateczne Wyjście i akceptacja zmian)
	    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET && !p1_wcisniety) { // wykrycie wciśnięcia przycisku 1
	        HAL_Delay(10);
	        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_RESET) {
	            p1_wcisniety = 1; // flaga blokady przycisku
	            if (aktualny_stan == STAN_NORMALNY) { // wejście do menu głównego zegara
	                temp_godziny = godziny;
	                temp_minuty = minuty;
	                temp_dzien = sDate.Date;
	                temp_miesiac = sDate.Month;
	                temp_rok = sDate.Year;
	                aktualny_stan = STAN_WYBORU_POLA; // przełączenie w stan wyboru pozycji
	                aktualne_pole = POLE_GODZINY;     // ustawienie pierwszego pola na godziny
	            } else {
	                // OSTATECZNE WYJŚCIE Z MENU PO ZAKOŃCZENIU EDYCJI CAŁOŚCI
	                // czyszczenie linii LCD
	                grove_lcd_put_cur(0, 0);
	                grove_lcd_send_string("                ");
	                grove_lcd_put_cur(1, 0);
	                grove_lcd_send_string("                ");

	                aktualny_stan = STAN_NORMALNY; // powrót do stanu nromalnego
	                stara_minuta = 99;             // wymuszenie pełnego odświeżenia grafiki głównej
	            }
	        }
	    } else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) {
	        p1_wcisniety = 0; // zerowanie flagi po zwolneniu przycisku
	    }

	    // Przycisk 2 W TYŁ / MINUS (-)
	    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_RESET && !p3_wcisniety) { // wykrycie wciśnięcia przycisku 2
	        HAL_Delay(10);
	        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_RESET) {
	            p3_wcisniety = 1;

	            if (aktualny_stan == STAN_WYBORU_POLA) { // nawigacja po menu w tył
	                if (aktualne_pole > POLE_GODZINY) aktualne_pole--;
	            }
	            else if (aktualny_stan == STAN_EDYCJI_WARTOSCI) { // dekrementacja wartości
	                if (aktualne_pole == POLE_GODZINY && temp_godziny > 0) temp_godziny--;
	                if (aktualne_pole == POLE_MINUTY  && temp_minuty > 0)  temp_minuty--;
	                if (aktualne_pole == POLE_DZIEN   && temp_dzien > 1)   temp_dzien--;
	                if (aktualne_pole == POLE_MIESIAC && temp_miesiac > 1) temp_miesiac--;
	                if (aktualne_pole == POLE_ROK     && temp_rok > 0)     temp_rok--;
	            }
	        }
	    } else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_SET) {
	        p3_wcisniety = 0;
	    }

	    // Przycisk 3 W PRZÓD / PLUS (+)
	    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET && !p2_wcisniety) { // wykrycie wciśnięcia przycisku 3
	        HAL_Delay(10);
	        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) {
	            p2_wcisniety = 1;

	            if (aktualny_stan == STAN_WYBORU_POLA) { // nawigacja po menu w przód
	                if (aktualne_pole < POLE_ROK) aktualne_pole++;
	            }
	            else if (aktualny_stan == STAN_EDYCJI_WARTOSCI) { // inkrementacja wartości
	                if (aktualne_pole == POLE_GODZINY && temp_godziny < 23) temp_godziny++;
	                if (aktualne_pole == POLE_MINUTY  && temp_minuty < 59)  temp_minuty++;
	                if (aktualne_pole == POLE_DZIEN   && temp_dzien < 31)   temp_dzien++;
	                if (aktualne_pole == POLE_MIESIAC && temp_miesiac < 12) temp_miesiac++;
	                if (aktualne_pole == POLE_ROK     && temp_rok < 99)     temp_rok++;
	            }
	        }
	    } else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_SET) {
	        p2_wcisniety = 0;
	    }

	    // PRZYCISK 4 (ENTER) - ZATWIERDZENIE I ZAPIS
	    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_RESET && !p4_wcisniety) { // wykrycie wciśnięcia przycisku 4
	        HAL_Delay(20);
	        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_RESET) {
	            p4_wcisniety = 1;

	            if (aktualny_stan == STAN_WYBORU_POLA) { // przejście do wyboru pozycji
	                aktualny_stan = STAN_EDYCJI_WARTOSCI;
	            }
	            else if (aktualny_stan == STAN_EDYCJI_WARTOSCI) { // zatwierdzenie i zapis wartości do rejestrów RTC

	                // Przepisanie wprowadzonych modyfikacji do struktur HAL
	                sTime.Hours = temp_godziny;
	                sTime.Minutes = temp_minuty;
	                sTime.Seconds = 0; // zerowanie sekund przy zatwierdzeniu dowolnego elementu
	                sTime.SubSeconds = 0;
	                sTime.SecondFraction = 0;
	                sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	                sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	                sDate.Date = temp_dzien;
	                sDate.Month = temp_miesiac;
	                sDate.Year = temp_rok;
	                sDate.WeekDay = Oblicz_Dzien_Tygodnia(temp_dzien, temp_miesiac, temp_rok); // automatyczne wyliczenie dnia tygodnia

	                // Fizyczny zapis do rejestrów RTC
	                HAL_PWR_EnableBkUpAccess();               // odblokowanie dostępu do zasialania backup
	                __HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc); // wyłączenie ochory zapisu
	                // wpisanie nowej godziny i daty
	                HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	                HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	                __HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);  // włączenie ochory zapisu

	                // aktualizacja zmiennych roboczych
	                godziny = temp_godziny;
	                minuty = temp_minuty;
	                sekundy = 0;

	                // blokada pętli na czas fizycznego trzymania przycisku ENTER
	                while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_RESET) {
	                    HAL_Delay(5);
	                }

	                aktualny_stan = STAN_WYBORU_POLA; // powrót do menu
	            }
	            stara_minuta = 99; // wymuszenie odświeżenia ekranu
	        }
	    } else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_SET) {
	        p4_wcisniety = 0;
	    }

	    // AKTUALIZACJA WYŚWIETLACZA GROVE LCD
	    if (aktualny_stan == STAN_NORMALNY) { // wyświetlenie ekranu głównego: dzień tygodnia i data
	        if (minuty != stara_minuta) {     // odświeżanie ekranu co minutę
	            stara_minuta = minuty;
	            grove_lcd_put_cur(0, 0);
	            grove_lcd_send_string("                ");
	            grove_lcd_put_cur(0, 0);
	            grove_lcd_send_string((char*)dni_tygodnia[sDate.WeekDay]); // wypisanie dnia tygodnia

	            grove_lcd_put_cur(1, 0);
	            snprintf(bufor_lcd, sizeof(bufor_lcd), "%02d | %02d | 20%02d", sDate.Date, sDate.Month, sDate.Year); // wypisanie daty
	            grove_lcd_send_string(bufor_lcd);
	        }
	    } else { // wyświetlenie ekranu interfejsu MENU
	        grove_lcd_put_cur(0, 0);
	        if (aktualny_stan == STAN_WYBORU_POLA) {
	            grove_lcd_send_string("WYBIERZ POLE:   ");
	        } else {
	            grove_lcd_send_string("EDYCJA WARTOSCI:");
	        }

	        grove_lcd_put_cur(1, 0);
	        snprintf(bufor_lcd, sizeof(bufor_lcd), "%02d:%02d  %02d|%02d|0%02d", temp_godziny, temp_minuty, temp_dzien, temp_miesiac, temp_rok); // format podglądu zmian
	        grove_lcd_send_string(bufor_lcd);
	    }

	    // MULTIPLEKSOWANIE WYŚWIETLACZY LED
	    // logika wyznaczania, które sekcje wyświetlacza mają świecić w danym stanie
	    uint8_t swiec_godziny = (aktualny_stan == STAN_NORMALNY) || (aktualny_stan == STAN_WYBORU_POLA && aktualne_pole == POLE_GODZINY) || (aktualny_stan == STAN_EDYCJI_WARTOSCI && aktualne_pole == POLE_GODZINY);
	    uint8_t swiec_minuty  = (aktualny_stan == STAN_NORMALNY) || (aktualny_stan == STAN_WYBORU_POLA && aktualne_pole == POLE_MINUTY)  || (aktualny_stan == STAN_EDYCJI_WARTOSCI && aktualne_pole == POLE_MINUTY);

	    uint8_t swiec_dzien   = (aktualny_stan == STAN_WYBORU_POLA && aktualne_pole == POLE_DZIEN)   || (aktualny_stan == STAN_EDYCJI_WARTOSCI && aktualne_pole == POLE_DZIEN);
	    uint8_t swiec_miesiac = (aktualny_stan == STAN_WYBORU_POLA && aktualne_pole == POLE_MIESIAC) || (aktualny_stan == STAN_EDYCJI_WARTOSCI && aktualne_pole == POLE_MIESIAC);
	    uint8_t swiec_rok     = (aktualny_stan == STAN_WYBORU_POLA && aktualne_pole == POLE_ROK)     || (aktualny_stan == STAN_EDYCJI_WARTOSCI && aktualne_pole == POLE_ROK);

	    // Pozycja 1: Dziesiątki Godzin / Dziesiątki Dnia
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);    // zgaszenie poprzedniej pozycji
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);    // zgaszenie kropki
	    if (aktualny_stan == STAN_NORMALNY && swiec_godziny) {
	        LED_Wyswietl_Cyfre(godziny / 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
	    } else if (aktualny_stan != STAN_NORMALNY && (swiec_godziny || swiec_dzien)) {
	        // w menu na pozycjach godzin wyświetlamy albo godziny, albo dzień
	        LED_Wyswietl_Cyfre((aktualne_pole == POLE_GODZINY) ? (temp_godziny / 10) : (temp_dzien / 10));
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
	    }
	    HAL_Delay(2);

	    // Pozycja 2: Jedności Godzin / Jedności Dnia
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);   // zgaszenie pozycji 1
	    if (aktualny_stan == STAN_NORMALNY) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // kropka tylko w trybie zegara
	    if (aktualny_stan == STAN_NORMALNY && swiec_godziny) {
	        LED_Wyswietl_Cyfre(godziny % 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
	    } else if (aktualny_stan != STAN_NORMALNY && (swiec_godziny || swiec_dzien)) {
	        LED_Wyswietl_Cyfre((aktualne_pole == POLE_GODZINY) ? (temp_godziny % 10) : (temp_dzien % 10));
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
	    }
	    HAL_Delay(2);

	    // Pozycja 3: Dziesiątki Minut / Dziesiątki Miesiąca
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);   // zgaszenie pozycji 2
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);    // zgaszenie kropki
	    if (aktualny_stan == STAN_NORMALNY && swiec_minuty) {
	        LED_Wyswietl_Cyfre(minuty / 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	    } else if (aktualny_stan != STAN_NORMALNY && (swiec_minuty || swiec_miesiac)) {
	        LED_Wyswietl_Cyfre((aktualne_pole == POLE_MINUTY) ? (temp_minuty / 10) : (temp_miesiac / 10));
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);
	    }
	    HAL_Delay(2);

	    // Pozycja 4: Jedności Minut / Jedności Miesiąca
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);   // zgaszenie pozycji 3
	    if (aktualny_stan == STAN_NORMALNY) HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // kropka tylko w trybie zegara
	    if (aktualny_stan == STAN_NORMALNY && swiec_minuty) {
	        LED_Wyswietl_Cyfre(minuty % 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	    } else if (aktualny_stan != STAN_NORMALNY && (swiec_minuty || swiec_miesiac)) {
	        LED_Wyswietl_Cyfre((aktualne_pole == POLE_MINUTY) ? (temp_minuty % 10) : (temp_miesiac % 10));
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
	    }
	    HAL_Delay(2);

	    // Pozycja 5: Dziesiątki Sekund / Dziesiątki Roku
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);    // zgaszenie pozycji 4
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);    // zgaszenie kropki
	    if (aktualny_stan == STAN_NORMALNY) {
	        LED_Wyswietl_Cyfre(sekundy / 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	    } else if (aktualny_stan != STAN_NORMALNY && swiec_rok) {
	        LED_Wyswietl_Cyfre(temp_rok / 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	    }
	    HAL_Delay(2);

	    // Pozycja 6: Jedności Sekund / Jedności Roku
	    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);    // zgaszenie pozycji 5
	    if (aktualny_stan == STAN_NORMALNY) {
	        LED_Wyswietl_Cyfre(sekundy % 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	    } else if (aktualny_stan != STAN_NORMALNY && swiec_rok) {
	        LED_Wyswietl_Cyfre(temp_rok % 10);
	        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	    }
	    HAL_Delay(2);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */
  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */
  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00100D14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

  /*Configure GPIO pins : PC1 PC2 PC3 PC5
                           PC6 PC8 PC9 PC10
                           PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5
                          |GPIO_PIN_6|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB11 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
