/*
 * ino-vrata
 * Ondrej Mecl, Ludek Mecl
 * Arduino Nano
 */

/*
 * Konstanty
 */

// piny
#define VRATA_L_MOTOR 5 // Levy motor
#define VRATA_R_MOTOR 6 // Pravy motor
#define VRATA_L_SMER 7 // Levy motor smer
#define VRATA_R_SMER 8 // Pravy motor smer
#define VRATA_L_SENSOR_OTEVRENO 9 // Levy sensor otevreno
#define VRATA_R_SENSOR_OTEVRENO 10 // Pravy sensor otevreno
#define VRATA_L_SENSOR_ZAVRENO 11  // Levy sensor zavreno
#define VRATA_R_SENSOR_ZAVRENO 12  // Pravy sensor zavreno
#define ALARM 13 // Houkacka
#define AKTIVNI 14 // Svetlo nad vraty
#define SPOUSTEC 15 // Spousteci signal

// dvere
#define VRATA_L 0
#define VRATA_R 1

// stavy dveri
#define STAV_NEZNAMO 0
#define STAV_OTEVRENO 1
#define STAV_ZAVRENO 2

// smery
#define SMER_OTEVRIT HIGH
#define SMER_ZAVRIT LOW

/*
 * Globalni promenne
 */

bool otevrit = true;

/*
 * Funkce
 */

int stavVrat(int vrata)
{
    if (vrata == VRATA_L)
    {
        if (digitalRead(VRATA_L_SENSOR_OTEVRENO) == LOW)
        {
            return STAV_OTEVRENO;
        }
        else if (digitalRead(VRATA_L_SENSOR_ZAVRENO) == LOW)
        {
            return STAV_ZAVRENO;
        }
        else
        {
            return STAV_NEZNAMO;
        }
    }
    else if (vrata == VRATA_R)
    {
        if (digitalRead(VRATA_R_SENSOR_OTEVRENO) == LOW)
        {
            return STAV_OTEVRENO;
        }
        else if (digitalRead(VRATA_R_SENSOR_ZAVRENO) == LOW)
        {
            return STAV_ZAVRENO;
        }
        else
        {
            return STAV_NEZNAMO;
        }
    }
    return NULL;
}

void beep() // trikrat pipne
{
    for (int i = 0; i < 3; i++) {
        digitalWrite(ALARM, HIGH);
        delay(100);
        digitalWrite(ALARM, LOW);
        delay(300);
    }
}

/*
 * Nastaveni
 */

void setup()
{
    /*
     * NASTAVENI PINU
     */

    // INPUT_PULLUP
    pinMode(VRATA_L_SENSOR_OTEVRENO, INPUT_PULLUP);
    pinMode(VRATA_R_SENSOR_OTEVRENO, INPUT_PULLUP);
    pinMode(VRATA_L_SENSOR_ZAVRENO, INPUT_PULLUP);
    pinMode(VRATA_R_SENSOR_ZAVRENO, INPUT_PULLUP);
    pinMode(SPOUSTEC, INPUT_PULLUP);
    
    // OUTPUT
    pinMode(VRATA_L_MOTOR, OUTPUT);
    pinMode(VRATA_R_MOTOR, OUTPUT);
    pinMode(VRATA_L_SMER, OUTPUT);
    pinMode(VRATA_R_SMER, OUTPUT);
    pinMode(ALARM, OUTPUT);
    pinMode(AKTIVNI, OUTPUT);
}

/*
 * Hlavni loop
 */

void loop()
{
    while (digitalRead(SPOUSTEC) != LOW); // pocka na spusteni
    digitalWrite(AKTIVNI, HIGH);
    if (otevrit)
    {
        int zdrzeni = 15000; // zdrzeni praveho kridla pred levym 15s
        // zmenit oboum kridlum smery
        digitalWrite(VRATA_L_SMER, SMER_OTEVRIT);
        digitalWrite(VRATA_R_SMER, SMER_OTEVRIT);
        bool hotovoL = false;
        bool hotovoR = false;
        for (;;) // terminovatelna nekonecna smycka pro kontrolu vrat
        {
            if (!hotovoL && stavVrat(VRATA_L) != STAV_OTEVRENO) // pokud leve kridlo neni otevrene, otevirat, pokud ano tak prestat
            {
                digitalWrite(VRATA_L_MOTOR, HIGH);
            }
            else
            {
                digitalWrite(VRATA_L_MOTOR, LOW);
                hotovoL = true;
            }
            if (!hotovoR || (zdrzeni == 0 && stavVrat(VRATA_R) != STAV_OTEVRENO)) // pokud prave kridlo jiz nema zdrzeni a neni otevrene, otevirat, jinak prestat
            {
                digitalWrite(VRATA_R_MOTOR, HIGH);
            }
            else
            {
                digitalWrite(VRATA_R_MOTOR, LOW);
                hotovoR = true;
            }
            if (digitalRead(SPOUSTEC) == LOW || (hotovoL && hotovoR)) // pokud nekdo stisknul spoustec nebo jiz jsou obe kridla otevrena tak terminovat smycku
            {
                break;
            }
            // pocka a odecte 10 milisekund ze zdrzeni
            delay(10);
            if (zdrzeni != 0)
            {
                zdrzeni -= 10;
            }
        }
        otevrit = false; // zmeni smer dalsiho spusteni na zavirani
    } else {
        int zdrzeni = 15000; // zdrzeni leveho kridla pred pravym 15s
        // zmenit oboum kridlum smery
        digitalWrite(VRATA_L_SMER, SMER_ZAVRIT);
        digitalWrite(VRATA_R_SMER, SMER_ZAVRIT);
        bool hotovoL = false;
        bool hotovoR = false;
        if (stavVrat(VRATA_L) == STAV_ZAVRENO && stavVrat(VRATA_R) != STAV_ZAVRENO) { // POJISTKA: pokud je leve kridlo jiz zavrene a prave ne, nechat leve na 5 sekund otevirat, aby se predeslo zniceni hran
            digitalWrite(VRATA_L_SMER, SMER_OTEVRIT);
            digitalWrite(VRATA_L_MOTOR, HIGH);
            delay(5000);
            digitalWrite(VRATA_L_MOTOR, LOW);
            digitalWrite(VRATA_L_SMER, SMER_ZAVRIT);
        }
        for (;;) // terminovatelna nekonecna smycka pro kontrolu vrat
        {
            if (!hotovoL || (zdrzeni == 0 && stavVrat(VRATA_L) != STAV_ZAVRENO)) // pokud leve kridlo jiz nema zdrzeni a neni zavrene, zavirat, jinak prestat
            {
                digitalWrite(VRATA_L_MOTOR, HIGH);
            }
            else
            {
                digitalWrite(VRATA_L_MOTOR, LOW);
                hotovoL = true;
            }
            if (!hotovoR && stavVrat(VRATA_R) != STAV_ZAVRENO) // pokud prave kridlo neni zavrene, zavirat, pokud ano tak prestat
            {
                digitalWrite(VRATA_R_MOTOR, HIGH);
            }
            else
            {
                digitalWrite(VRATA_R_MOTOR, LOW);
                hotovoR = true;
            }
            if (digitalRead(SPOUSTEC) == LOW || (hotovoL && hotovoR)) // pokud nekdo stisknul spoustec nebo jiz jsou obe kridla otevrena tak terminovat smycku
            {
                break;
            }
            // pocka a odecte 10 milisekund ze zdrzeni
            delay(10);
            if (zdrzeni != 0)
            {
                zdrzeni -= 10;
            }
        }
        otevrit = false; // zmeni smer dalsiho spusteni na zavirani
    }
    digitalWrite(AKTIVNI, LOW);
}
