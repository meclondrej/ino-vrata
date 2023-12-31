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

// casy
#define ZDRZENI_MS 15000
#define ZDRZENI_POJISTKA_L_MS 5000
#define ZDRZENI_POJISTKA_R_MS 3000
#define BEEP_POCET 3
#define BEEP_CAS_MS 100
#define BEEP_CAS_CEKANI_MS 300

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
    for (int i = 0; i < BEEP_POCET; i++) {
        digitalWrite(ALARM, HIGH);
        delay(BEEP_CAS_MS);
        digitalWrite(ALARM, LOW);
        delay(BEEP_CAS_CEKANI_MS);
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
        int zdrzeni = ZDRZENI_MS; // zdrzeni praveho kridla pred levym 15s
        // zmenit oboum kridlum smery
        digitalWrite(VRATA_L_SMER, SMER_OTEVRIT);
        digitalWrite(VRATA_R_SMER, SMER_OTEVRIT);
        bool hotovoL = false;
        bool hotovoR = false;
        if (stavVrat(VRATA_L) == STAV_ZAVRENO && stavVrat(VRATA_R) != STAV_ZAVRENO)
        { // POJISTKA: pokud je leve kridlo jiz zavrene a prave ne, nechat prave na 3 sekundy otevirat, aby se predeslo zniceni hran
            digitalWrite(VRATA_R_SMER, SMER_OTEVRIT);
            int r_cas = ZDRZENI_POJISTKA_R_MS;
            for (;;)
            {
                if (r_cas != 0 && stavVrat(VRATA_L) != STAV_OTEVRENO)
                {
                    digitalWrite(VRATA_L_MOTOR, HIGH);
                    r_cas -= 10;
                }
                else
                {
                    digitalWrite(VRATA_L_MOTOR, LOW);
                    break;
                }
                if (digitalRead(SPOUSTEC) == LOW)
                { // Pokud je stisknut spoustec pri prubehu pojistky, terminovat pojistku i nasledujici smycku
                    hotovoL = true;
                    hotovoR = true;
                    break;
                }
            }
            digitalWrite(VRATA_R_SMER, SMER_ZAVRIT);
        }
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
            if (!hotovoR && ((zdrzeni == 0 || hotovoL) && stavVrat(VRATA_R) != STAV_OTEVRENO)) // pokud prave kridlo jiz nema zdrzeni a neni otevrene, otevirat, jinak prestat
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
        int zdrzeni = ZDRZENI_MS; // zdrzeni leveho kridla pred pravym 15s
        // zmenit oboum kridlum smery
        digitalWrite(VRATA_L_SMER, SMER_ZAVRIT);
        digitalWrite(VRATA_R_SMER, SMER_ZAVRIT);
        bool hotovoL = false;
        bool hotovoR = false;
        if (stavVrat(VRATA_L) == STAV_ZAVRENO && stavVrat(VRATA_R) != STAV_ZAVRENO)
        { // POJISTKA: pokud je leve kridlo jiz zavrene a prave ne, nechat obe otevirat tak, aby se predeslo zniceni hran
            digitalWrite(VRATA_L_SMER, SMER_OTEVRIT);
            digitalWrite(VRATA_R_SMER, SMER_OTEVRIT);
            int l_cas = ZDRZENI_POJISTKA_L_MS;
            int r_cas = ZDRZENI_POJISTKA_R_MS;
            for (;;)
            {
                if (l_cas != 0 && stavVrat(VRATA_L) != STAV_OTEVRENO)
                {
                    digitalWrite(VRATA_L_MOTOR, HIGH);
                    l_cas -= 10;
                }
                else
                {
                    digitalWrite(VRATA_L_MOTOR, LOW);
                }
                if (l_cas != 0 && stavVrat(VRATA_L) != STAV_OTEVRENO)
                {
                    digitalWrite(VRATA_L_MOTOR, HIGH);
                    l_cas -= 10;
                }
                else
                {
                    digitalWrite(VRATA_L_MOTOR, LOW);
                }
                if (l_cas == 0 && r_cas == 0)
                {
                    break;
                }
                if (digitalRead(SPOUSTEC) == LOW)
                { // Pokud je stisknut spoustec pri prubehu pojistky, terminovat pojistku i nasledujici smycku
                    hotovoL = true;
                    hotovoR = true;
                    break;
                }
            }
            digitalWrite(VRATA_L_SMER, SMER_ZAVRIT);
            digitalWrite(VRATA_R_SMER, SMER_ZAVRIT);
        }
        for (;;) // terminovatelna nekonecna smycka pro kontrolu vrat
        {
            if (!hotovoL && ((zdrzeni == 0 || hotovoR) && stavVrat(VRATA_L) != STAV_ZAVRENO)) // pokud leve kridlo jiz nema zdrzeni a neni zavrene, zavirat, jinak prestat
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
        otevrit = true; // zmeni smer dalsiho spusteni na zavirani
    }
    digitalWrite(AKTIVNI, LOW);
}
