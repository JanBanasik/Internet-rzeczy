# Inteligentna stacja do monitorowania i pielęgnacji roślin doniczkowych

Główna idea zakłada stworzenie urządzenia umieszczanego w doniczce, które monitoruje kluczowe parametry dla rośliny (wilgotność gleby, temperaturę, wilgotność powietrza, nasłonecznienie) i w razie potrzeby może automatycznie ją podlać. Użytkownik ma zdalny podgląd i kontrolę nad wszystkim za pośrednictwem aplikacji mobilnej.

## Realizacja:

*   **Programowanie ESP32 (ESP-IDF):**
    *   **I2C/SPI:** Podłączenie czujnika BME280 (temperatura, wilgotność, ciśnienie) oraz czujnika natężenia światła BH1750. Możliwe jest również dodanie małego wyświetlacza OLED (np. SSD1306) do pokazywania statusu.
    *   **We/wy analogowe:** Wykorzystanie analogowego czujnika wilgotności gleby.
    *   **We/wy cyfrowe:** Sterowanie małą pompką wody za pomocą modułu przekaźnika oraz sygnalizowanie stanu pracy diodą LED.
    *   **WiFi:** ESP32 łączy się z domową siecią, aby wysyłać dane na serwer.
    *   **MQTT:** Urządzenie publikuje odczyty z czujników na serwerze (np. `dom/roslina/salon/temperatura`) i subskrybuje temat do odbierania poleceń (np. `dom/roslina/salon/podlej`).
    *   **BLE:** Służy do pierwszej konfiguracji. Aplikacja mobilna łączy się z ESP32 przez BLE, aby w bezpieczny sposób przesłać mu dane logowania do sieci WiFi (nazwę i hasło).
    *   **Pamięć lokalna:** W przypadku utraty połączenia z internetem, ESP32 zapisuje odczyty z czujników w wewnętrznej pamięci flash. Po odzyskaniu połączenia wysyła wszystkie zbuforowane dane.

*   **Oprogramowanie po stronie serwera:**
    *   Instalacja brokera MQTT (np. Mosquitto) na serwerze (może to być Raspberry Pi lub mała maszyna wirtualna w chmurze).
    *   Opracowanie prostej aplikacji (np. w Pythonie lub Node.js), która subskrybuje dane z MQTT, zapisuje je do bazy danych (np. InfluxDB lub PostgreSQL) i udostępnia je przez API dla aplikacji mobilnej.
    *   Możliwe jest zaimplementowanie logiki automatyzacji, np. "jeśli wilgotność gleby spadnie poniżej 30%, wyślij polecenie podlej".

*   **Aplikacja mobilna (Android/iOS):**
    *   **Parowanie:** Skanowanie urządzeń BLE, wybór stacji i przesyłanie danych do WiFi.
    *   **Odczyt wartości:** Łączenie się z API serwera, aby pobierać aktualne i historyczne dane, a następnie wyświetlanie ich na wykresach.
    *   **Sterowanie:** Przycisk "Podlej teraz" wysyłający polecenie do serwera, który przekazuje je przez MQTT do ESP32.

*   **Szczegóły:**
    * **Profile różnych roślin:** w aplikacji można stworzyć profile dla różnych roślin i podpiąć pod ten profil konkretny czujnik (chcemy dać przyszłościową możliwość tworzenia większego systemu z więcej niż 1 czujnika)
    * **Lepiej czujniki pojemnościowe do pomiaru wilgotności**
    * **Symulacja pompki poprzez diodę:** 
    * **Wymiana informacji o nasłonecznieniu pomiędzy czujnikami i sugestie gdzie najlepiej postawić roślinę:** zaimplementować coś takiego, żeby działało, jakby było więcej czujników
    * **Rrofile podlewania zapisane we flashu żeby działała bez Wi-Fi:** chcemy umożliwić triggerowanie podlewania bez dostępu do Wi-Fi
