/**
 * @file main.cpp
 * @brief Proyecto de monitoreo ambiental y detección de proximidad.
 * 
 * Este programa implementa un sistema de monitoreo ambiental que utiliza 
 * varios componentes para recopilar y mostrar datos sobre las condiciones 
 * ambientales. A través de un teclado matricial, el usuario puede interactuar 
 * con el sistema, ingresando contraseñas y seleccionando opciones del menú.
 * 
 * Los componentes utilizados en este proyecto incluyen:
 * - **Keypad**: Permite la entrada de datos por parte del usuario.
 * - **LCD**: Muestra información relevante, como temperatura, humedad y 
 *   estados del sistema.
 * - **Sensor DHT**: Mide la temperatura y la humedad del ambiente, 
 *   proporcionando datos en tiempo real.
 * - **Sensor de Luz**: Detecta la intensidad de la luz en el entorno, 
 *   permitiendo la evaluación de condiciones de iluminación.
 * - **Sensor Infrarrojo**: Utilizado para la detección de proximidad, 
 *   permitiendo al sistema reaccionar ante la presencia de objetos cercanos.
 * - **Sensor Hall**: Detecta campos magnéticos, útil para aplicaciones 
 *   que requieren monitoreo de objetos magnéticos.
 * 
 * La configuración del sistema incluye la inicialización de cada uno de 
 * estos componentes y la gestión de la interacción del usuario a través 
 * del menú. Los datos recopilados se muestran en la pantalla LCD y se 
 * utilizan para activar alarmas o indicadores visuales según sea necesario.
 * 
 * Este sistema es ideal para aplicaciones de monitoreo ambiental en 
 * tiempo real, proporcionando al usuario información crítica sobre 
 * las condiciones del entorno y la capacidad de respuesta ante cambios 
 * en estas condiciones.
 */

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <DHT.h>

// Configuración del keypad
const byte ROWS = 4; ///< Cuatro filas
const byte COLS = 4; ///< Cuatro columnas
char keys[ROWS][COLS] = {
    {'1','2','3', 'A'},
    {'4','5','6', 'B'},
    {'7','8','9', 'C'},
    {'*','0','#', 'D'}
};
byte rowPins[ROWS] = {22, 24, 26, 28}; ///< Conectar a las salidas de fila del keypad
byte colPins[COLS] = {30, 32, 34, 36}; ///< Conectar a las salidas de columna del keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Configuración del LCD
const int RS = 12, EN = 11, D4 = 5, D5 = 4, D6 = 3, D7 = 2;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// Configuración del sensor de temperatura y humedad
const int DHTPIN = 13; ///< Pin del sensor DHT
#define DHTTYPE DHT22 ///< Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Pines de los LEDs y otros componentes
const int LED_GREEN_PIN = 9; ///< Pin del LED verde
const int LED_RED_PIN = 10; ///< Pin del LED rojo
const int LED_BLUE_PIN = 8; ///< Pin del LED azul
const int BUZZER_PIN = 6; ///< Pin del buzzer
const int PHOTO_RESISTOR_PIN = A0; ///< Pin del fotoresistor
const int INFRARED_PIN = 14; ///< Pin del sensor infrarrojo
const int HALL_PIN = 15; ///< Pin del sensor Hall

/** Variables de estado */
const String CORRECT_PASSWORD = "0690"; ///< Contraseña correcta
String inputPassword = ""; ///< Contraseña ingresada
int attemptCount = 0; ///< Contador de intentos
const int MAX_ATTEMPTS = 3; ///< Máximo de intentos permitidos
int currentState = 0; ///< Estado actual del sistema
unsigned long stateChangeTime = 0; ///< Tiempo de cambio de estado

/**
 * @brief Configuración inicial del sistema.
 * 
 * Esta función se ejecuta una vez al inicio del programa y se encarga de 
 * realizar la configuración necesaria para que todos los componentes del 
 * sistema funcionen correctamente. 
 * 
 * - Inicializa la pantalla LCD para mostrar información al usuario.
 * - Configura los pines de los LEDs (verde, rojo y azul) como salidas, 
 *   permitiendo el control de su encendido y apagado.
 * - Configura el pin del buzzer como salida para emitir sonidos cuando 
 *   sea necesario.
 * - Establece el pin del fotoresistor como entrada para leer los niveles 
 *   de luz ambiental.
 * - Configura el pin del sensor infrarrojo como entrada para detectar 
 *   objetos cercanos.
 * - Configura el pin del sensor Hall como entrada para detectar campos 
 *   magnéticos.
 * - Inicializa el sensor DHT para comenzar a medir temperatura y 
 *   humedad.
 * - Muestra un mensaje inicial en el LCD solicitando al usuario que 
 *   ingrese una clave.
 * 
 * Esta configuración es esencial para el correcto funcionamiento del 
 * sistema de monitoreo ambiental y detección de proximidad.
 */
void setup() {
    lcd.begin(16, 2); ///< Inicializa el LCD con 16 columnas y 2 filas
    pinMode(LED_GREEN_PIN, OUTPUT); ///< Configura el pin del LED verde como salida
    pinMode(LED_RED_PIN, OUTPUT); ///< Configura el pin del LED rojo como salida
    pinMode(LED_BLUE_PIN, OUTPUT); ///< Configura el pin del LED azul como salida
    pinMode(BUZZER_PIN, OUTPUT); ///< Configura el pin del buzzer como salida
    pinMode(PHOTO_RESISTOR_PIN, INPUT); ///< Configura el pin del fotoresistor como entrada
    pinMode(INFRARED_PIN, INPUT); ///< Configura el pin del sensor infrarrojo como entrada
    pinMode(HALL_PIN, INPUT); ///< Configura el pin del sensor Hall como entrada
    dht.begin(); ///< Inicializa el sensor de temperatura y humedad
    lcd.print("Ingrese la clave:"); ///< Muestra un mensaje en el LCD
}


/**
 * @brief Bucle principal del programa.
 * 
 * Esta función se ejecuta continuamente y gestiona la entrada del teclado 
 * así como los estados del sistema. Se encarga de verificar la clave de 
 * acceso, permitir el ingreso de datos y manejar el estado del sistema 
 * según la interacción del usuario.
 * 
 * - **Entrada de Teclado**: Utiliza un keypad para capturar la tecla 
 *   presionada por el usuario. Si se presiona una tecla, se procede a 
 *   verificar su valor.
 * 
 * - **Verificación de Clave**: 
 *   - Si se presiona el símbolo `'#'`, se verifica si la longitud de 
 *     la clave ingresada es 4 y si coincide con la clave correcta 
 *     (`CORRECT_PASSWORD`). 
 *       - Si la clave es correcta:
 *         - Limpia la pantalla LCD y muestra un mensaje de bienvenida.
 *         - Enciende el LED verde y emite un tono de bienvenida.
 *         - Cambia el estado del sistema a "Monitoreo Ambiental".
 *         - Guarda el tiempo de cambio de estado.
 *       - Si la clave es incorrecta:
 *         - Incrementa el contador de intentos.
 *         - Muestra un mensaje de error y el número de intentos realizados.
 *         - Si se alcanzó el número máximo de intentos (`MAX_ATTEMPTS`):
 *           - Limpia el LCD y muestra un mensaje de bloqueo.
 *           - Activa una alarma y enciende el LED rojo.
 *           - Reinicia el sistema.
 * 
 * - **Limpieza de Entrada**: 
 *   - Si se presiona el símbolo `'*'`, se reinicia la entrada de la clave 
 *     y se muestra un mensaje solicitando la clave nuevamente.
 * 
 * - **Ingreso de Dígitos**: 
 *   - Si se presiona cualquier otro dígito, se agrega a la entrada de 
 *     la clave siempre que la longitud de la entrada sea menor a 4. 
 *     Se muestra un asterisco en el LCD en lugar del dígito ingresado 
 *     para mantener la privacidad de la clave.
 * 
 * Este bucle permite la interacción continua del usuario con el sistema, 
 * gestionando tanto el ingreso de datos como la respuesta del sistema 
 * a diferentes entradas.
 */
void loop() {
    char key = keypad.getKey(); ///< Obtiene la tecla presionada

    if (key) { ///< Si se ha presionado una tecla
        if (key == '#') { ///< Al presionar '#', verifica la clave
            if (inputPassword.length() == 4 && inputPassword == CORRECT_PASSWORD) {
                lcd.clear(); ///< Limpia el LCD
                lcd.print("Bienvenido"); ///< Muestra mensaje de bienvenida
                digitalWrite(LED_GREEN_PIN, HIGH); ///< Enciende el LED verde
                welcomeTone(); ///< Llama a la función de tono de bienvenida
                delay(1000); ///< Espera 1 segundo
                digitalWrite(LED_GREEN_PIN, LOW); ///< Apaga el LED verde
                currentState = 1; ///< Cambia al estado de Monitoreo Ambiental
                stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado

            } else {
                attemptCount++; ///< Incrementa el contador de intentos
                lcd.clear(); ///< Limpia el LCD
                lcd.print("Error intento "); ///< Muestra mensaje de error
                lcd.print(attemptCount); ///< Muestra el número de intentos
                inputPassword = ""; ///< Reinicia la entrada
                if (attemptCount >= MAX_ATTEMPTS) { ///< Si se alcanzó el máximo de intentos
                    lcd.clear(); ///< Limpia el LCD
                    lcd.print("Bloqueado"); ///< Muestra mensaje de bloqueo
                    alarmSound(); ///< Llama a la función de alarma
                    digitalWrite(LED_RED_PIN, HIGH); ///< Enciende el LED rojo
                    delay(2000); ///< Espera 2 segundos
                    digitalWrite(LED_RED_PIN, LOW); ///< Apaga el LED rojo
                    reset(); ///< Reinicia el sistema
                }
            }
        } else if (key == '*') { ///< Al presionar '*', limpia la entrada
            inputPassword = ""; ///< Reinicia la entrada
            lcd.clear(); ///< Limpia el LCD
            lcd.print("Ingrese la clave:"); ///< Muestra mensaje para ingresar clave
        } else { ///< Agrega el dígito a la entrada
            if (inputPassword.length() < 4) { ///< Verifica si la longitud de la entrada es menor a 4
                inputPassword += key; ///< Agrega el dígito a la entrada
                lcd.setCursor(0, 1); ///< Establece el cursor en la segunda fila
                lcd.print(getAsterisks(inputPassword.length())); ///< Muestra '*' en lugar de la clave ingresada
            }
        }
    }
}


    // Lógica de cambio de estado
    switch (currentState) {
        case 1: ///< Monitoreo Ambiental
            monitoreoAmbiental(); ///< Llama a la función de monitoreo ambiental
            break;
        case 2: ///< Monitor Eventos
            monitorEventos(); ///< Llama a la función de monitoreo de eventos
            break;
        case 3: ///< Alerta
            alerta(); ///< Llama a la función de alerta
            break;
        case 4: ///< Alarma
            alarma(); ///< Llama a la función de alarma
            break;
        case 5: ///< Infrarrojo
            monitoreoInfrarrojo(); ///< Llama a la función de monitoreo infrarrojo
            break;
        case 6: ///< Hall
            monitoreoHall(); ///< Llama a la función de monitoreo Hall
            break;
    }


/**
 * @brief Monitoreo ambiental de temperatura y humedad.
 * 
 * Esta función lee los valores del sensor DHT para obtener la temperatura 
 * y la humedad, y verifica si estos valores se encuentran dentro de un 
 * rango seguro. Si los valores están fuera de los límites establecidos, 
 * se activa el estado de alarma.
 * 
 * - **Lectura de Sensores**: 
 *   - Se leen los valores de temperatura y humedad desde el sensor DHT.
 * 
 * - **Verificación de Seguridad**: 
 *   - Si la temperatura está por debajo de 10 °C o por encima de 40 °C, 
 *     o si la humedad está por debajo de 5% o por encima de 60%, 
 *     se cambia el estado del sistema a "Alarma".
 * 
 * - **Actualización del LCD**: 
 *   - Si el tiempo desde el último cambio de estado es mayor o igual a 4 
 *     segundos, se actualiza la pantalla LCD para mostrar la información 
 *     de monitoreo ambiental (temperatura y humedad).
 * 
 * - **Cambio de Estado**: 
 *   - Después de mostrar la información, se cambia el estado a "Monitor 
 *     Eventos".
 */
void monitoreoAmbiental() {
    float h = dht.readHumidity(); ///< Lee la humedad
    float t = dht.readTemperature(); ///< Lee la temperatura

    // Comprobar condiciones para activar la alarma
    if (t < 10 || t > 40 || h < 5 || h > 60) {
        currentState = 4; ///< Cambia al estado de Alarma
        stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
        return; ///< Salir de la función para evitar mostrar datos
    }

    // Evitar que el LCD titile
    if (millis() - stateChangeTime >= 4000) {
        lcd.clear(); ///< Limpia el LCD
        lcd.print("Moni Ambiental"); ///< Muestra mensaje de monitoreo ambiental
        lcd.setCursor(0, 1); ///< Establece el cursor en la segunda fila
        lcd.print("T:"); ///< Muestra la etiqueta de temperatura
        lcd.print(t); ///< Muestra la temperatura
        lcd.print("C H:"); ///< Muestra la etiqueta de humedad
        lcd.print(h); ///< Muestra la humedad

        currentState = 2; ///< Cambia al estado de Monitor Eventos
        stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
    }
}

/**
 * @brief Monitoreo de eventos de luz.
 * 
 * Esta función lee el valor del fotoresistor y calcula la cantidad de luz 
 * en lux. Dependiendo del valor medido, puede cambiar el estado del sistema 
 * a "Alerta" o volver al estado de "Monitoreo Ambiental".
 * 
 * - **Lectura del Fotoresistor**: 
 *   - Se obtiene el valor analógico del fotoresistor y se convierte a 
 *     voltaje.
 * 
 * - **Cálculo de Lux**: 
 *   - Se calcula la resistencia y se convierte a lux utilizando la 
 *     fórmula adecuada.
 * 
 * - **Actualización del LCD**: 
 *   - Si ha pasado más de 3 segundos desde el último cambio de estado, 
 *     se actualiza la pantalla LCD para mostrar el valor de luz medido.
 * 
 * - **Cambio de Estado**: 
 *   - Si el valor de lux es mayor a 700 o menor a 200, se cambia el estado 
 *     a "Alerta". Si no, se regresa al estado de "Monitoreo Ambiental".
 */
void monitorEventos() {
    int analogValue = analogRead(PHOTO_RESISTOR_PIN); ///< Lee el valor analógico del fotoresistor
    float voltage = analogValue / 1024. * 5; ///< Calcula el voltaje
    float resistance = 2000 * voltage / (1 - voltage / 5); ///< Calcula la resistencia
    float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA)); ///< Calcula el lux

    // Evitar que el LCD titile
    if (millis() - stateChangeTime >= 3000) {
        lcd.clear(); ///< Limpia el LCD
        lcd.print("Moni Eventos"); ///< Muestra mensaje de monitoreo de eventos
        lcd.setCursor(0, 1); ///< Establece el cursor en la segunda fila
        lcd.print("Luz : "); ///< Muestra la etiqueta de luz
        lcd.print(lux); ///< Muestra el valor de lux

        // Condición para pasar al estado de Alerta
        if (lux > 700 || lux < 200) {
            currentState = 3; ///< Cambia al estado de Alerta
            stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
        } else {
            currentState = 1; ///< Vuelve al estado de Monitoreo Ambiental
            stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
        }
    }
}


/**
 * @brief Monitoreo del sensor infrarrojo.
 * 
 * Activa la alarma y el LED azul si se detecta proximidad.
 */
void monitoreoInfrarrojo() {
    if (digitalRead(INFRARED_PIN) == HIGH) { ///< Si se detecta proximidad
        lcd.clear(); ///< Limpia el LCD
        lcd.print("Infrarrojo Activo"); ///< Muestra mensaje de activación
        digitalWrite(LED_BLUE_PIN, HIGH); ///< Enciende el LED azul
        alarmSound(); ///< Llama a la función de alarma
        delay(1000); ///< Espera 1 segundo
        digitalWrite(LED_BLUE_PIN, LOW); ///< Apaga el LED azul
        currentState = 2; ///< Regresar al estado de Monitor Eventos
        stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
    }
}

/**
 * @brief Monitoreo del sensor Hall.
 * 
 * Activa la alarma y el LED azul si se detecta un campo electromagnético.
 */
void monitoreoHall() {
    if (digitalRead(HALL_PIN) == HIGH) { ///< Si se detecta campo electromagnético
        lcd.clear(); ///< Limpia el LCD
        lcd.print("Hall Activo"); ///< Muestra mensaje de activación
        digitalWrite(LED_BLUE_PIN, HIGH); ///< Enciende el LED azul
        alarmSound(); ///< Llama a la función de alarma
        delay(1000); ///< Espera 1 segundo
        digitalWrite(LED_BLUE_PIN, LOW); ///< Apaga el LED azul
        currentState = 2; ///< Regresar al estado de Monitor Eventos
        stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
    }
}

/**
 * @brief Manejo de alertas por condiciones de luz.
 * 
 * Cambia el estado a alerta si se detectan condiciones anormales de luz.
 */
void alerta() {
    int analogValue = analogRead(PHOTO_RESISTOR_PIN); ///< Lee el valor analógico del fotoresistor
    float voltage = analogValue / 1024. * 5; ///< Calcula el voltaje
    float resistance = 2000 * voltage / (1 - voltage / 5); ///< Calcula la resistencia
    float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA)); ///< Calcula el lux
    
    // Evitar que el LCD titile
    if (millis() - stateChangeTime >= 3000) {
        lcd.clear(); ///< Limpia el LCD
        lcd.print("Alerta!"); ///< Muestra mensaje de alerta
        lcd.setCursor(0, 1); ///< Establece el cursor en la segunda fila
        if (lux > 700) { ///< Si la luz es alta
            lcd.print("Luz: Alta"); ///< Muestra mensaje de luz alta
            digitalWrite(LED_BLUE_PIN, HIGH); ///< Enciende el LED azul
            alarmSound(); ///< Llama a la función de alarma
            delay(1000); ///< Espera 1 segundo
            digitalWrite(LED_BLUE_PIN, LOW); ///< Apaga el LED azul
        } else if (lux < 200) { ///< Si la luz es baja
            lcd.print("Luz: Baja"); ///< Muestra mensaje de luz baja
            digitalWrite(LED_BLUE_PIN, HIGH); ///< Enciende el LED azul
            alarmSound(); ///< Llama a la función de alarma
            delay(1000); ///< Espera 1 segundo
            digitalWrite(LED_BLUE_PIN, LOW); ///< Apaga el LED azul
        }
        currentState = 2; ///< Vuelve al estado de Monitor Eventos
        stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
    }
}

/**
 * @brief Manejo de la alarma.
 * 
 * Activa la alarma y mantiene el estado de alarma hasta que se resuelva.
 */
void alarma() {
    digitalWrite(LED_RED_PIN, HIGH); ///< Enciende el LED rojo
    alarmSound(); ///< Llama a la función de alarma

    lcd.clear(); ///< Limpia el LCD
    lcd.print("ALERTA CRITICA!"); ///< Muestra mensaje de alerta crítica
    lcd.setCursor(0, 1); ///< Establece el cursor en la segunda fila
    lcd.print("T o H fuera de"); ///< Muestra mensaje de condiciones fuera de rango
    lcd.setCursor(0, 2); ///< Establece el cursor en la tercera fila
    lcd.print("rango seguro!");

    // Mantener el estado de alarma hasta que se resuelva
    while (true) {
        float h = dht.readHumidity(); ///< Lee la humedad
        float t = dht.readTemperature(); ///< Lee la temperatura

        // Verificar si las condiciones de alarma aún se cumplen
        if (t >= 10 && t <= 40 && h >= 5 && h <= 60) {
            digitalWrite(LED_RED_PIN, LOW); ///< Apaga el LED rojo
            noTone(BUZZER_PIN); ///< Detener el sonido del buzzer
            currentState = 1; ///< Regresar al estado de Monitoreo Ambiental
            stateChangeTime = millis(); ///< Guarda el tiempo de cambio de estado
            break; ///< Salir del bucle
        }
    }
}

/**
 * @brief Genera una cadena de asteriscos.
 * 
 * @param length Longitud de la cadena de asteriscos.
 * @return String Cadena de asteriscos.
 */
String getAsterisks(int length) {
    String asterisks = ""; ///< Inicializa la cadena de asteriscos
    for (int i = 0; i < length; i++) { ///< Itera según la longitud
        asterisks += '*'; ///< Agrega un asterisco por cada dígito ingresado
    }
    return asterisks; ///< Retorna la cadena de asteriscos
}

/**
 * @brief Reinicia el sistema.
 * 
 * Limpia la contraseña ingresada y el contador de intentos.
 */
void reset() {
    inputPassword = ""; ///< Reinicia la entrada
    attemptCount = 0; ///< Reinicia el contador de intentos
    lcd.clear(); ///< Limpia el LCD
    lcd.print("Ingrese la clave:"); ///< Muestra mensaje para ingresar clave
}

/**
 * @brief Emite un sonido de alarma.
 * 
 * Produce un patrón de sonido para indicar una alarma.
 */
void alarmSound() {
    // Sonido de alarma
    for (int i = 0; i < 5; i++) { ///< Repite el patrón 5 veces
        tone(BUZZER_PIN, 1000, 250); ///< Tono de 1000 Hz durante 250 ms
        delay(250); ///< Espera 250 ms
        tone(BUZZER_PIN, 500, 250); ///< Tono de 500 Hz durante 250 ms
        delay(250); ///< Espera 250 ms
    }
    noTone(BUZZER_PIN); ///< Detener el sonido del buzzer
}

/**
 * @brief Emite un tono de bienvenida.
 * 
 * Produce un patrón de sonido para indicar un acceso exitoso.
 */
void welcomeTone() {
    // Tono de bienvenida
    tone(BUZZER_PIN, 262, 500); ///< Tono de do durante 500 ms
    delay(500); ///< Espera 500 ms
    tone(BUZZER_PIN, 330, 500); ///< Tono de mi durante 500 ms
    delay(500); ///< Espera 500 ms
    tone(BUZZER_PIN, 392, 500); ///< Tono de sol durante 500 ms
    delay(500); ///< Espera 500 ms
    noTone(BUZZER_PIN); ///< Detener el sonido del buzzer
}
