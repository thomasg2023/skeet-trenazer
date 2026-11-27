/*
  Skeet Trigger – XIAO nRF52840 Sense
  Výstřel = ráz úderníku (akcelerometr) A ZÁROVEŇ ostré cvaknutí (mikrofon)
  v časovém okně SOUBEH_MS. Do Questu se pošle stisk klávesy "F"
  jako Bluetooth klávesnice "Skeet Trigger".

  VERZE 23: hlášky namluvené vlastním hlasem (nahrávka z Diktafonu), jinak stejné jako 22.
  VERZE 22: MENU REŽIMŮ + PÍPÁNÍ VŽDY (hlas jen navíc). Po zapnutí MENU: 1 stisk = další položka (pípne tolikrát, kolikátá je,
            a řekne název), 2 stisky = vybrat (dlouhé pípnutí + „vybráno“), po 10 s bez stisku se vybere sama. V režimu 2 stisky = zpět do menu.
            Režimy: 1 sušení doma (VR spoušť + záznam + hodnocení švihu), 2 skeet střelnice (jen záznam ostrých ran + hodnocení švihu),
                    3 kule – pistole/puška (záznam + strhávání), 4 trénink nahazování (stisk → „pozor“ → za 1,5–4 s pípnutí → nahoď;
                    čas do zklidnění zbraně nebo do cvaknutí řekne v sekundách), 5 trénink spouštění (každé cvaknutí: čistě / strhnul jsi).
            Kódy pípnutí: START 3 krátká vysoká, STOP 2 dlouhá nižší, dobře/čistě 1 vysoké, chyba 2 hluboká.
            Hlas: silnější komprese (hlasitější). Příkaz rezim=1..5, menu.
  VERZE 21: HLAS Z PIEZA – místo pípání česká slova (8 kHz řeč ve flash, hlasky.h; RAM jen 1 kB, přehrává PWM3).
            START/STOP: pípne a řekne „start“ / „stop“. Po zapnutí „zapnuto“ (nebo „chyba senzoru“), připojeno/odpojeno,
            dlouhý stisk = pípnutí + „baterie plná / polovina / skoro prázdná“, slabá baterie (pod 15 %, každých 10 min),
            „vybito“ a „vypínám“ před uspáním, „nabíjím“ po připojení USB, „paměť plná“.
            HODNOCENÍ KAŽDÉ RÁNY z gyroskopu (pravidla jako zaznam.html): „dobře“, „zastavil jsi“, „zpomaluješ“, „dotáhni švih“.
            Řekne se až 0,9 s po poslední ráně (dublet nepřeruší); při mluvení se nebere mikrofon (vlastní hlas není výstřel).
            Příkazy: hlas=0/1, hodnoceni=0/1, rekni=N (přehraje hlášku N, bez čísla vypíše seznam).
  VERZE 20: PŘEPÍNAČ ZAP/VYP, KTERÝ NEODPOJUJE BATERKU – nabíjení přes USB jde i ve vypnutém stavu.
            Zapojení přepínače (3 vývody): prostřední → D3, jeden krajní → GND (= VYPNUTO), druhý krajní → 3V3 (= ZAPNUTO).
            Baterka je připájená napevno na BAT+/BAT−. VYPNUTO = deska přejde do úplného spánku (System OFF, jednotky µA),
            přepnutím na ZAPNUTO se probudí a naběhne. Bez přepínače (D3 nezapojený) běží jako dřív.
            Pípák: D1 – GND, tlačítko: D2 – GND.
            Před spánkem se vypne napájení akcelerometru a mikrofonu, sběrnice I2C a paměť QSPI (deep power-down),
            aby spoušť ve vypnutém stavu brala jen jednotky µA (dřív v ní zůstávaly zapnuté senzory a baterka se vybila).
  VERZE 19: MENŠÍ ZPOŽDĚNÍ – klávesa odchází hned při PRVNÍM signálu (režim 3 „rychlý“: ráz nebo cvaknutí),
            druhý signál jen potvrzuje (výpis E POTVRZENO / E NEPOTVRZENO, počet v „info“);
            Bluetooth interval 7,5–15 ms (dřív podle Questu až 30–50 ms), mikrofon po 4 ms blocích (dřív 16 ms),
            klávesa se pouští bez čekání (smyčka už nestojí 8 ms).
            Hra si zpoždění spouště změří sama (menu → ZMĚŘIT) a výstřel vyhodnotí v okamžiku stisku.
  VERZE 18: na USB (powerbanka, nabíječka) hlásí brýlím 100 % a stav „napájeno z USB“ (klávesa F24),
            nebliká červeně kvůli slabé baterce – baterka se při tom dál měří (příkaz „baterka“).

  KROK 8: ZÁZNAM ZE STŘELNICE (nic z VR spouště se nemění, výchozí "zaznam 0" = jako krok 7)
          - zaznam 0 = jen VR spoušť | 1 = jen záznam ze střelnice | 2 = obojí
          - gyroskop + akcelerometr + mikrofon 100× za s, stále 3 s zpětně v paměti
          - ostrá rána (velký ráz + hlasitý zvuk) → uloží se 3 s před první ranou až 1,5 s po poslední
          - záznamy jdou do 2 MB paměti na desce (QSPI), přežijí vypnutí; ~250 záznamů
          - Bluetooth UART (Nordic UART) pro aplikaci zaznam.html v iPhonu (Bluefy) / Chrome
          - programy: 0 trénink 1 terč, 1 trénink double, 2 skeet, 3 lovecké kolo,
            4 americký trap, 5 olympijský trap, 6 univerzální trap, 7 double trap, 8 trénink trap (pořadí řeší aplikace)
          - předvolby (6 pojmenovaných sad všech prahů a nastavení)
          - HID klávesy se posílají jen do Questu (ne do telefonu), takže spoušť funguje i s připojeným telefonem
          POTŘEBNÁ KNIHOVNA: "Adafruit SPIFlash" (Library Manager, nainstalovat i se závislostmi)
          Pokud by nešla přeložit, změňte níže POUZIT_QSPI na 0 → záznamy jen v RAM (3 záznamy, po vypnutí zmizí).
  KROK 7: měření baterky (napětí + %), posílá se do Questu i simulátoru; čitelné LED
  KROK 6: citlivé výchozí prahy (cvaknutí spouště naprázdno), kratší blokace 120 ms
  KROK 5: každých 5 s posílá do brýlí neviditelnou klávesu F24 – simulátor podle ní ukazuje „spoušť připojena“
  KROK 4: ráz = náhlá změna zrychlení mezi dvěma měřeními (plynulý švih se odfiltruje)
  KROK 3: + nastavení za chodu přes USB (stránka nastaveni.html v Chrome nebo Serial Monitor)
          příkazy: info | prah_g 4.5 | prah_mic 15000 | rezim 0/1/2 | soubeh 40 | blokace 250
                   ulozit | vychozi | stream 1 / stream 0
          nové v kroku 8: stav | zaznam 0/1/2 | program 0-9 | nabijeni 50/100 (mA) | usinani 0-240 (min, 0 = nikdy) | smazat_pary (zapomene Quest i telefony) | odpojit_vse | prah_rana_g 6 | prah_rana_mic 20000
                   cas <unix> | nova_polozka | zaznamy | poslat <i> | smazat_zaznamy
                   predvolby | predvolba <i> | predvolba_uloz <i> <nazev> | predvolba_smaz <i> | vychozi2
  Deska v Arduino IDE:  Seeed nRF52 Boards → "Seeed XIAO nRF52840 Sense"
  PDM.h je součástí jádra desky.
*/
#define POUZIT_QSPI 1

#include <bluefruit.h>
#include <Wire.h>
#include "hlasky.h"   // české hlášky pro piezo (verze 21)
#include <PDM.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#if POUZIT_QSPI
#include <Adafruit_SPIFlashBase.h>   // používá se jen přenosová vrstva QSPI, ovladač čipu je níže
#endif
using namespace Adafruit_LittleFS_Namespace;
typedef Adafruit_LittleFS_Namespace::File LfsFile;

// ---------- nastavení (měnitelné za chodu, ukládá se do paměti desky) ----------
struct Cfg { uint32_t magic; float prahG; int32_t prahMic; uint8_t rezim; uint16_t soubeh; uint16_t blokace; };
uint32_t nepotvrzeno = 0;   // rychlý režim: kolikrát přišel jen jeden signál
const Cfg VYCHOZI = { 0x534B4556, 1.5f, 3000, 3, 60, 120 };   // citlivé: stačí cvaknutí spouště naprázdno   // rezim: 0 = ráz, 1 = cvaknutí, 2 = obojí, 3 = rychlý (první z nich hned, druhý jen kontrola)
Cfg cfg = VYCHOZI;
const char* CFG_SOUBOR = "/skeet.cfg";
bool stream = false;

// rozšířené nastavení (krok 8) – zvlášť, aby staré nastavení zůstalo beze změny
struct Cfg2 { uint32_t magic; uint8_t zaznam; uint8_t program; uint8_t predvolba; uint8_t bezi; float prahRanaG; int32_t prahRanaMic; };   // bezi = START/STOP záznamu
const Cfg2 VYCHOZI2 = { 0x534B5632, 2, 0, 255, 0, 6.0f, 20000 };   // výchozí režim 2 = VR spoušť i záznam
Cfg2 cfg2 = VYCHOZI2;
const char* CFG2_SOUBOR = "/skeet2.cfg";

struct Predvolba { char nazev[16]; Cfg c; Cfg2 c2; };
const int PREDVOLEB = 6;
Predvolba pv[PREDVOLEB];
const char* PV_SOUBOR = "/predvolby.bin";

void rana(uint32_t T);
void hlas(uint8_t idx); void hodnotRanu(uint32_t T); extern bool strhZap; extern uint8_t rezim; void hlasUspani(const char* proc);
// struktury záznamu musí být nahoře – Arduino si před první funkci vkládá vlastní deklarace funkcí
struct __attribute__((packed)) Vz { int16_t g[3]; int8_t a[3]; uint8_t mic; };            // 10 B: gyro surově, zrychlení v 1/8 g, mikrofon /128
struct __attribute__((packed)) Hlav { uint32_t magic; uint16_t idx, nVz, krok; uint8_t program, nRan; uint16_t ranaMs[4];
  uint32_t unix0, up0; uint8_t perioda; int8_t bat; uint8_t predvolba, polozka; uint8_t rez[32]; };                // 64 B
static_assert(sizeof(Vz) == 10, "Vz");
static_assert(sizeof(Hlav) == 64, "Hlav");
const uint32_t ZMAGIC = 0x315A4B53;      // "SKZ1"
const int PRE = 300, MAXV = 800, PERIODA = 10, PO_RANE_MS = 1500;
const uint32_t SLOT = 8192;              // 1 záznam = 2 sektory po 4 kB
struct __attribute__((packed)) Rec { Hlav h; Vz v[MAXV]; };

// co spoušť pošle při výstřelu – nastavitelné příkazem "klavesa" (výchozí F pro simulátor)
// typ 0 = klávesa (HID kód), 1 = tlačítko myši, 2 = mediální klávesa (např. hlasitost +, spoušť fotoaparátu v telefonu)
uint8_t klTyp = 0; uint16_t klKod = 0x09; char klNazev[12] = "f";
const char* KL_SOUBOR = "/klavesa.txt";
bool klavesaNastav(String n){   // vrací false, když název nezná
  n.trim(); n.toLowerCase(); uint8_t t = 0; uint16_t k = 0;
  if (n.length() == 1 && n[0] >= 'a' && n[0] <= 'z') k = 0x04 + (n[0] - 'a');
  else if (n.length() == 1 && n[0] >= '1' && n[0] <= '9') k = 0x1E + (n[0] - '1');
  else if (n == "0") k = 0x27;
  else if (n == "mezernik" || n == "space") k = 0x2C;
  else if (n == "enter") k = 0x28;
  else if (n.length() >= 2 && n.length() <= 3 && n[0] == 'f' && n.substring(1).toInt() >= 1 && n.substring(1).toInt() <= 12) k = 0x3A + n.substring(1).toInt() - 1;
  else if (n == "mys") { t = 1; k = 1; }
  else if (n == "mys2") { t = 1; k = 2; }
  else if (n == "hlasitost+") { t = 2; k = 0x00E9; }
  else if (n == "hlasitost-") { t = 2; k = 0x00EA; }
  else return false;
  klTyp = t; klKod = k; strncpy(klNazev, n.c_str(), sizeof klNazev - 1); klNazev[sizeof klNazev - 1] = 0; return true;
}
void klavesaNacti(){ LfsFile f(InternalFS); char b[12] = {0};
  if (f.open(KL_SOUBOR, FILE_O_READ)) { int n = f.read(b, 11); f.close(); if (n > 0) { b[n] = 0; klavesaNastav(String(b)); } } }
bool klavesaUloz(){ InternalFS.remove(KL_SOUBOR); LfsFile f(InternalFS);
  if (!f.open(KL_SOUBOR, FILE_O_WRITE)) return false; f.write((const uint8_t*)klNazev, strlen(klNazev)); f.close(); return true; }

// nabíjecí proud baterky (čip BQ25101 na XIAO): P0.13 = LOW → 100 mA, odpojený pin → 50 mA
uint8_t nabMa = 100;
const char* NAB_SOUBOR = "/nabijeni.txt";
void nabijeniNastav(){
  if (nabMa >= 100) { NRF_P0->PIN_CNF[13] = 3; NRF_P0->OUTCLR = (1UL << 13); }   // výstup, LOW
  else NRF_P0->PIN_CNF[13] = 2;                                                   // vstup odpojený (výchozí stav)
}
void nabijeniNacti(){ LfsFile f(InternalFS); char b[4] = {0};
  if (f.open(NAB_SOUBOR, FILE_O_READ)) { int n = f.read(b, 3); f.close(); if (n > 0) nabMa = atoi(b) == 50 ? 50 : 100; }
  nabijeniNastav(); }
// ochrana baterky: pod 3,30 V (60 s, bez USB) a po nečinnosti se spoušť úplně vypne (System OFF);
// zapne se znovu stiskem tlačítka (krabička v5 bez vypínače), přepnutím vypínače nebo tlačítkem RESET
const int VYPINAC = D3;   // přepínač: D3 na GND = vypnuto, na 3V3 = zapnuto (nezapojený = zapnuto, vnitřní pull-up)
bool vypinacVyp(){ return digitalRead(VYPINAC) == LOW; }
uint16_t usinMin = 20; const char* USIN_SOUBOR = "/usinani.txt";
uint32_t posledniPohyb = 0; uint8_t nizkeBat = 0;
void usinNacti(){ LfsFile f(InternalFS); char b[6] = {0}; if (f.open(USIN_SOUBOR, FILE_O_READ)) { int n = f.read(b, 5); f.close(); if (n > 0) usinMin = constrain(atoi(b), 0, 240); } }
bool usinUloz(){ InternalFS.remove(USIN_SOUBOR); LfsFile f(InternalFS); if (!f.open(USIN_SOUBOR, FILE_O_WRITE)) return false; String t = String(usinMin); f.write((const uint8_t*)t.c_str(), t.length()); f.close(); return true; }
bool usbNapajeni(){ return (NRF_POWER->USBREGSTATUS & 1) != 0; }
void vsechnoVypnout();   // definice je níže (u setup), potřebuje proměnné deklarované později
void vypnoutUplne(const char* proc){
  outObe(String("E USPANI ") + proc);
  hlasUspani(proc);
  for (int i = 0; i < 3; i++) { digitalWrite(LED_RED, LOW); delay(150); digitalWrite(LED_RED, HIGH); delay(150); }
  digitalWrite(LED_BLUE, HIGH); digitalWrite(LED_GREEN, HIGH); delay(100);
  vsechnoVypnout();
  if (vypinacVyp()) {
    // vypnuto přepínačem: probudí se, až přepínač připojí D3 na 3V3 (bez pull-upu – přepínač drží D3 na GND, nic neteče)
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[VYPINAC], NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
  } else {
    // uspání po nečinnosti / slabé baterce: probudí tlačítko (D2 stisk = LOW)
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[D2], NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  }
  sd_power_system_off();
  NRF_POWER->SYSTEMOFF = 1; while (1) {}
}
// Když se telefon „zapomene“ jen na jedné straně, spoušť si pamatuje starý klíč a další spojení selže.
// Při nepovedeném párování proto spoušť své páry smaže; telefon se pak napodruhé připojí načisto.
volatile bool smazatPary = false;
volatile int16_t logPripojeni = -1, logOdpojeni = -1, logParovani = -1; volatile uint8_t logDuvod = 0, logAuth = 0;
void parovaniHotovo(uint16_t conn_hdl, uint8_t auth_status){ logParovani = conn_hdl; logAuth = auth_status; }   // jen zapsat do logu – páry se samy nemažou (iPhone by pak měl staré klíče a nepřipojil by se)
bool nabijeniUloz(){ InternalFS.remove(NAB_SOUBOR); LfsFile f(InternalFS);
  if (!f.open(NAB_SOUBOR, FILE_O_WRITE)) return false; String t = String(nabMa); f.write((const uint8_t*)t.c_str(), t.length()); f.close(); return true; }
void cfgNacti(){ LfsFile f(InternalFS); Cfg c;
  if (f.open(CFG_SOUBOR, FILE_O_READ)) { if (f.read(&c, sizeof c) == sizeof c && c.magic == VYCHOZI.magic) cfg = c; f.close(); } }
bool cfgUloz(){ InternalFS.remove(CFG_SOUBOR); LfsFile f(InternalFS);
  if (!f.open(CFG_SOUBOR, FILE_O_WRITE)) return false; f.write((uint8_t*)&cfg, sizeof cfg); f.close(); return true; }
void cfg2Nacti(){ LfsFile f(InternalFS); Cfg2 c;
  if (f.open(CFG2_SOUBOR, FILE_O_READ)) { if (f.read(&c, sizeof c) == sizeof c && c.magic == VYCHOZI2.magic) cfg2 = c; f.close(); } }
bool cfg2Uloz(){ InternalFS.remove(CFG2_SOUBOR); LfsFile f(InternalFS);
  if (!f.open(CFG2_SOUBOR, FILE_O_WRITE)) return false; f.write((uint8_t*)&cfg2, sizeof cfg2); f.close(); return true; }
void pvNacti(){ memset(pv, 0, sizeof pv); LfsFile f(InternalFS);
  if (f.open(PV_SOUBOR, FILE_O_READ)) { if (f.read(pv, sizeof pv) != sizeof pv) memset(pv, 0, sizeof pv); f.close(); } }
bool pvUloz(){ InternalFS.remove(PV_SOUBOR); LfsFile f(InternalFS);
  if (!f.open(PV_SOUBOR, FILE_O_WRITE)) return false; f.write((uint8_t*)pv, sizeof pv); f.close(); return true; }

// jméno spouště: "Skeet " + vlastní jméno; výchozí "Trigger XXXX" podle čísla čipu (každý kus jiné)
char jmeno[21] = "";
char plneJmeno[32] = "";
const char* JM_SOUBOR = "/jmeno.txt";
void jmenoNacti(){
  snprintf(jmeno, sizeof jmeno, "Trigger %04X", (unsigned)(NRF_FICR->DEVICEADDR[0] & 0xFFFF));
  LfsFile f(InternalFS); char b[21] = {0};
  if (f.open(JM_SOUBOR, FILE_O_READ)) { int n = f.read(b, 20); f.close(); if (n > 0) { b[n] = 0; strncpy(jmeno, b, 20); jmeno[20] = 0; } }
  snprintf(plneJmeno, sizeof plneJmeno, "Skeet %s", jmeno);
}
bool jmenoUloz(const String& n){ InternalFS.remove(JM_SOUBOR); if (!n.length()) return true; LfsFile f(InternalFS);
  if (!f.open(JM_SOUBOR, FILE_O_WRITE)) return false; f.write((const uint8_t*)n.c_str(), n.length()); f.close(); return true; }
uint32_t blikniDo = 0;

String cfgText(){ return "C prah_g=" + String(cfg.prahG,2) + " prah_mic=" + String(cfg.prahMic) + " rezim=" + String(cfg.rezim) + " nepotvrzeno=" + String(nepotvrzeno)
                         + " soubeh=" + String(cfg.soubeh) + " blokace=" + String(cfg.blokace); }
String cfg2Text(){ return "X zaznam=" + String(cfg2.zaznam) + " program=" + String(cfg2.program) + " prah_rana_g=" + String(cfg2.prahRanaG,1)
                         + " prah_rana_mic=" + String(cfg2.prahRanaMic) + " predvolba=" + String(cfg2.predvolba==255 ? -1 : (int)cfg2.predvolba); }

// ---------- výstup: odpověď jde tam, odkud přišel příkaz (USB nebo Bluetooth) ----------
BLEUart bleuart;
uint8_t zdrojPrikazu = 0;            // 0 = USB, 1 = Bluetooth
// spojení s telefonem = to, které si zapnulo odběr UART (Quest ho nemá)
int uartSpojeni(){ for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (Bluefruit.connected(h) && bleuart.notifyEnabled(h)) return h; return -1; }
bool bleOK(){ return uartSpojeni() >= 0; }
// řádek po kouscích podle MTU spojení (knihovna delší zprávu jen ořízne)
bool bleZasek = false;   // poslední posílání do telefonu se nepovedlo (spojení vázne)
void bleRadek(const String& s){ int h = uartSpojeni(); if (h < 0) return;
  BLEConnection* c = Bluefruit.Connection((uint16_t)h); uint16_t kus = 20;
  if (c) { uint16_t m = c->getMtu(); if (m > 23) kus = m - 3; }
  String t = s + "\n"; const uint8_t* d = (const uint8_t*)t.c_str(); uint32_t n = t.length();
  // notifikace se může nevejít do fronty (iPhone posílá jednu za ~30 ms) – pak počkat a poslat znovu, nic neztratit
  for (uint32_t o = 0; o < n; o += kus) { uint32_t k = min((uint32_t)kus, n - o);
    bool ok = false;
    for (int pokus = 0; pokus < 400; pokus++) { if (!Bluefruit.connected((uint16_t)h) || !bleuart.notifyEnabled((uint16_t)h)) { bleZasek = true; return; }
      if (bleuart.write((uint16_t)h, d + o, k) == k) { ok = true; break; } delay(5); }
    if (!ok) { bleZasek = true; return; } }   // 2 s se nic neodeslalo → vzdát; aplikace se sama znovu připojí a záznam si vyžádá znovu
}
void out(const String& s){
  if (zdrojPrikazu == 1) bleRadek(s);
  else Serial.println(s);
}
void outObe(const String& s){ Serial.println(s); bleRadek(s); }

// ---------- akcelerometr + gyroskop LSM6DS3TR-C přímo přes I2C ----------
int imuStav = -99;            // 0 = OK
TwoWire* imuBus = nullptr; uint8_t imuAddr = 0;
int who[4] = {-2,-2,-2,-2};   // WHO_AM_I: Wire1 0x6A, Wire1 0x6B, Wire 0x6A, Wire 0x6B
const char* imuKde = "-";
int imuRead(TwoWire* b, uint8_t a, uint8_t reg){ b->beginTransmission(a); b->write(reg); if(b->endTransmission(false)!=0) return -1;
  if(b->requestFrom(a,(uint8_t)1)!=1) return -1; return b->read(); }
void imuWrite(uint8_t reg, uint8_t val){ imuBus->beginTransmission(imuAddr); imuBus->write(reg); imuBus->write(val); imuBus->endTransmission(); }
bool imuAccel(float &x,float &y,float &z){
  imuBus->beginTransmission(imuAddr); imuBus->write(0x28); if(imuBus->endTransmission(false)!=0) return false;
  if(imuBus->requestFrom(imuAddr,(uint8_t)6)!=6) return false;
  int16_t r[3]; for(int i=0;i<3;i++){ uint8_t l=imuBus->read(), h=imuBus->read(); r[i]=(int16_t)((h<<8)|l); }
  x=r[0]*0.000488f; y=r[1]*0.000488f; z=r[2]*0.000488f; return true; }
bool imuGyro(int16_t g[3]){   // surová data, 70 mdps na jednotku (rozsah ±2000 °/s)
  imuBus->beginTransmission(imuAddr); imuBus->write(0x22); if(imuBus->endTransmission(false)!=0) return false;
  if(imuBus->requestFrom(imuAddr,(uint8_t)6)!=6) return false;
  for(int i=0;i<3;i++){ uint8_t l=imuBus->read(), h=imuBus->read(); g[i]=(int16_t)((h<<8)|l); } return true; }
void imuInit(){
  TwoWire* busy[2] = { nullptr, &Wire };
  const char* jm[2] = { "Wire1", "Wire" };
#if WIRE_INTERFACES_COUNT > 1
  busy[0] = &Wire1;
#endif
  const uint8_t adr[2] = {0x6A, 0x6B};
  for(int b=0;b<2;b++){ if(!busy[b]) continue; busy[b]->begin(); busy[b]->setClock(400000);
    for(int a=0;a<2;a++){ int w=imuRead(busy[b],adr[a],0x0F); who[b*2+a]=w;
      if(!imuBus && (w==0x69||w==0x6A)){ imuBus=busy[b]; imuAddr=adr[a]; imuKde=jm[b]; } } }
  if(!imuBus){ imuStav=-1; return; }
  imuWrite(0x12,0x01); delay(20);   // reset
  imuWrite(0x12,0x44);              // BDU + auto-increment
  imuWrite(0x10,0x84);              // 1,66 kHz, ±16 g
  imuStav=0; }
extern uint8_t hodnZap;
void gyroNastav(){ if (imuStav == 0) imuWrite(0x11, cfg2.zaznam ? 0x8C : (hodnZap || strhZap || rezim == 3) ? 0x5C : 0x00); }   // gyro 1,66 kHz ±2000 °/s (záznam), 208 Hz (jen hodnocení ran), nebo vypnuto
BLEDis bledis;
BLEHidAdafruit blehid;
BLEBas blebas;

// ---------- mikrofon ----------
short micBuf[256];
volatile int      micSpicka   = 0;    // špička v posledním bloku
volatile uint32_t micCasPrahu = 0;    // kdy naposledy přešel práh
volatile int      micMax05    = 0;
volatile int      micMaxZ     = 0;    // pro záznam: maximum od posledního vzorku

void onPDMdata() {
  int n = PDM.available();
  PDM.read(micBuf, n);
  int pk = 0;
  for (int i = 0; i < n / 2; i++) { int v = abs(micBuf[i]); if (v > pk) pk = v; }
  micSpicka = pk;
  if (pk > micMax05) micMax05 = pk;
  if (pk > micMaxZ) micMaxZ = pk;
  if (pk > cfg.prahMic) micCasPrahu = millis();
}

uint32_t accCasPrahu = 0, posledniVystrel = 0, posledniVypis = 0, pustitV = 0, cekaPotvrzeni = 0; uint8_t prvniSignal = 0;
float accMax05 = 0;

void startAdv() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();        // v inzerci se jméno případně zkrátí…
  Bluefruit.ScanResponse.addName();       // …celé jméno je v odpovědi na skenování
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}
// po připojení prvního zařízení (Quest nebo telefon) se dál inzeruje, aby se mohlo připojit i druhé
// výpis do Serial Monitoru: kdo se připojil, proč se odpojil, jak dopadlo párování (proměnné jsou výš u parovaniHotovo)
void priPripojeni(uint16_t h){ logPripojeni = h; BLEConnection* cc = Bluefruit.Connection(h); if (cc) cc->requestConnectionParameter(6); if (Bluefruit.connected() < 3) Bluefruit.Advertising.start(0); }
void priOdpojeni(uint16_t h, uint8_t duvod){ logOdpojeni = h; logDuvod = duvod; }
void bleLogy(){
  if (logPripojeni >= 0) { char jm[24] = {0}; BLEConnection* c = Bluefruit.Connection(logPripojeni); (void)c;   // jméno protistrany NEČÍST – je to dotaz do telefonu uprostřed jeho připojování a telefon pak spojení ukončí
    Serial.print("BLE pripojeno h="); Serial.print(logPripojeni); Serial.print(" "); Serial.println(jm[0] ? jm : "?"); logPripojeni = -1; hlas(H_PRIPOJENO); }
  if (logOdpojeni >= 0) { Serial.print("BLE odpojeno h="); Serial.print(logOdpojeni); char hx[6]; snprintf(hx, sizeof hx, "%02X", logDuvod); Serial.print(" duvod 0x"); Serial.print(hx);
    Serial.println(logDuvod == 0x13 ? " (telefon ukoncil)" : logDuvod == 0x08 ? " (vyprsel cas)" : logDuvod == 0x3D ? " (chyba sifrovani)" : logDuvod == 0x05 ? " (chyba overeni / parovani)" : logDuvod == 0x16 ? " (spoust ukoncila)" : logDuvod == 0x3E ? " (spojeni se nenavazalo)" : "");
    logOdpojeni = -1; hlas(H_ODPOJENO); }
  if (logParovani >= 0) { Serial.print("BLE parovani h="); Serial.print(logParovani); char ha[6]; snprintf(ha, sizeof ha, "%02X", logAuth); Serial.print(" vysledek 0x"); Serial.println(ha); logParovani = -1; } }

// klávesa jen do spojení, které má zapnuté HID (Quest); telefon s aplikací ji nedostane
// HID klávesy jen do Questu: spojení, které odebírá UART, je telefon s aplikací → tomu nic neposílat
bool jeQuest(uint16_t h){ return Bluefruit.connected(h) && !bleuart.notifyEnabled(h); }
void hidReport(uint8_t key){
  uint8_t kc[6] = {key,0,0,0,0,0};
  for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (jeQuest(h)) blehid.keyboardReport(h, 0, kc);
}
void hidRelease(){ for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (jeQuest(h)) blehid.keyRelease(h); }

void pustitKlavesu(){ for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (jeQuest(h)) {
      if (klTyp == 1) blehid.mouseButtonRelease(h); else if (klTyp == 2) blehid.consumerKeyRelease(h); else blehid.keyRelease(h); } }
void vystrel(const char* zdroj) {
  posledniVystrel = millis();
  hodnotRanu(posledniVystrel);
  if (Bluefruit.connected()) {
    for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (jeQuest(h)) {
      if (klTyp == 1) blehid.mouseButtonPress(h, (uint8_t)klKod);
      else if (klTyp == 2) blehid.consumerKeyPress(h, klKod);
      else { uint8_t kc[6] = {(uint8_t)klKod,0,0,0,0,0}; blehid.keyboardReport(h, 0, kc); } }
    pustitV = millis() + 12; if (!pustitV) pustitV = 1; }   // pustí se ve smyčce – smyčka mezitím dál hlídá senzory
  digitalWrite(LED_GREEN, LOW);
  Serial.print("E VYSTREL "); Serial.println(zdroj);
  if (cfg2.zaznam == 2 && cfg2.bezi && imuStav == 0) rana(posledniVystrel);   // režim "obojí": záznam i z VR spouště (cvaknutí naprázdno)
}

// ---------- baterka ----------
// XIAO nRF52840: dělič 1M/510k na P0.31, zapíná se VBAT_ENABLE (P0.14) = LOW. P0.14 nikdy nedávat HIGH!
float batV = 0; int batPct = -1; uint32_t posledniBat = 0;
void batInit(){
#ifdef VBAT_ENABLE
  pinMode(VBAT_ENABLE, OUTPUT); digitalWrite(VBAT_ENABLE, LOW);
#endif
  analogReadResolution(12);
}
int batProcenta(float v){   // Li-Po v klidu, zjednodušená křivka
  const float U[] = {3.30,3.50,3.60,3.70,3.75,3.80,3.85,3.92,4.00,4.10,4.20};
  if (v <= U[0]) return 0; if (v >= U[10]) return 100;
  for (int i=1;i<11;i++) if (v < U[i]) return (int)((i-1)*10 + 10*(v-U[i-1])/(U[i]-U[i-1]));
  return 100; }
// stav nabíjení: XIAO má výstup CHG nabíječky BQ25101 na P0.17 (LOW = nabíjí se)
int nabijiSe(){ NRF_P0->PIN_CNF[17] = 0x0C; delayMicroseconds(20); return (NRF_P0->IN & (1UL << 17)) ? 0 : 1; }   // vstup s pull-up
String batText(){ String t = "B " + String(batV, 2) + " " + String(batPct) + " usb=" + String(usbNapajeni() ? 1 : 0) + " nabiji=" + String(nabijiSe());
  if (usbNapajeni() && !nabijiSe() && batV > 4.1f) t += " (plna, nebo baterka neni pripojena - zmerte napeti primo na clanku)";
  return t; }
void batMer(){
#ifdef PIN_VBAT
  uint32_t sum=0; for(int i=0;i<16;i++) sum+=analogRead(PIN_VBAT);
  float vadc = sum/16.0f/4095.0f*3.6f;          // výchozí reference 0,6 V × 6 = 3,6 V
  batV = vadc * (1510.0f/510.0f);
  batPct = batV < 2.5f ? -1 : batProcenta(batV);  // < 2,5 V = baterka nepřipojená
  blebas.write(usbNapajeni() ? 100 : (batPct >= 0 ? batPct : 100));   // v18: na USB hlásí 100 %, aby brýle neukazovaly „slabá baterie“
#endif
}

// =====================================================================
// ---------- ZÁZNAM ZE STŘELNICE (krok 8) ----------
// =====================================================================
Rec rec;
Vz ring[PRE]; uint16_t ringPos = 0, ringN = 0;
bool nahrava = false; uint32_t recT0 = 0, posledniRanaT = 0, ranaAccT = 0, ranaMicT = 0, dalsiVz = 0, posledniVzT = 0, zelenaDo = 0;
uint16_t nZaz = 0, slotu = 0, krok = 0; uint8_t polozka = 0; uint32_t unixZaklad = 0;
const char* pametTyp = "ZADNA";

#if POUZIT_QSPI
Adafruit_FlashTransport_QSPI flashTransport;
// vlastní jednoduchý ovladač čipu P25Q16H (knihovna ho v některých verzích nezná)
void fWait(){ uint8_t st = 1; uint32_t t0 = millis(); do { flashTransport.readCommand(0x05, &st, 1); } while ((st & 1) && millis() - t0 < 3000); }
bool fRead(uint32_t a, uint8_t* b, uint32_t n){ return flashTransport.readMemory(a, b, n); }
bool fErase4k(uint32_t a){ fWait(); flashTransport.runCommand(0x06); bool ok = flashTransport.eraseCommand(0x20, a); fWait(); return ok; }
bool fWrite(uint32_t a, const uint8_t* b, uint32_t n){
  while (n) { uint32_t k = min(n, (uint32_t)(256 - (a & 255))); fWait(); flashTransport.runCommand(0x06);
    if (!flashTransport.writeMemory(a, b, k)) return false; fWait(); a += k; b += k; n -= k; }
  return true; }
#endif
bool flashOK = false;
const int RAM_SLOTU = 3;
static uint8_t ramSlot[RAM_SLOTU][SLOT];

bool slotCti(uint16_t i, uint32_t off, void* buf, uint32_t n){
#if POUZIT_QSPI
  if (flashOK) return fRead(i*SLOT + off, (uint8_t*)buf, n);
#endif
  if (i >= RAM_SLOTU) return false; memcpy(buf, ramSlot[i] + off, n); return true; }
void slotSmaz(uint16_t i){
#if POUZIT_QSPI
  if (flashOK) { fErase4k(i*SLOT); fErase4k(i*SLOT + 4096); return; }
#endif
  if (i < RAM_SLOTU) memset(ramSlot[i], 0xFF, SLOT); }
bool slotZapis(uint16_t i, const uint8_t* buf, uint32_t n){
  slotSmaz(i);
#if POUZIT_QSPI
  if (flashOK) return fWrite(i*SLOT, buf, n);
#endif
  if (i >= RAM_SLOTU) return false; memcpy(ramSlot[i], buf, n); return true; }
bool hlavicka(uint16_t i, Hlav& h){ return i < slotu && slotCti(i, 0, &h, sizeof h) && h.magic == ZMAGIC && h.idx == i; }

uint32_t flashJedec = 0; uint8_t flashSR2 = 0; int flashTest = -1;
void pametInit(){
#if POUZIT_QSPI
  // čip na XIAO bývá po startu uspaný (deep power-down) → nejdřív probudit, jinak se neozve
  flashTransport.begin();
  flashTransport.runCommand(0xAB); delayMicroseconds(100);
  { uint8_t id[4] = {0,0,0,0}; flashTransport.readCommand(0x9F, id, 4); flashJedec = (uint32_t)id[0] << 16 | id[1] << 8 | id[2]; }
  uint8_t vel = flashJedec & 0xFF;
  if (flashJedec != 0 && flashJedec != 0xFFFFFF && vel >= 0x10 && vel <= 0x18) {
    // čtení/zápis jde po 4 linkách → musí být zapnutý bit QE (stavový registr 2, bit 1)
    flashTransport.readCommand(0x35, &flashSR2, 1);
    if (!(flashSR2 & 0x02)) { uint8_t sr1 = 0; flashTransport.readCommand(0x05, &sr1, 1); uint8_t w[2] = { sr1, (uint8_t)(flashSR2 | 0x02) };
      flashTransport.runCommand(0x06); flashTransport.writeCommand(0x01, w, 2); fWait();
      flashTransport.readCommand(0x35, &flashSR2, 1);
      if (!(flashSR2 & 0x02)) { uint8_t w2 = flashSR2 | 0x02; flashTransport.runCommand(0x06); flashTransport.writeCommand(0x31, &w2, 1); fWait(); flashTransport.readCommand(0x35, &flashSR2, 1); } }
    uint32_t velikost = 1UL << vel;
    // zkouška: poslední sektor smazat, zapsat vzor, přečíst zpět
    uint32_t ta = velikost - 4096; uint8_t vzor[16], zpet[16]; for (int i = 0; i < 16; i++) vzor[i] = 0xA5 ^ (i * 37);
    flashOK = true; fErase4k(ta); fWrite(ta, vzor, 16); memset(zpet, 0, 16); fRead(ta, zpet, 16);
    flashTest = memcmp(vzor, zpet, 16) == 0 ? 1 : 0; fErase4k(ta);
    flashOK = flashTest == 1;
    if (flashOK) { slotu = min((uint32_t)255, velikost / SLOT - 1); pametTyp = "QSPI"; }   // poslední slot je zkušební
  }
#endif
  if (!flashOK) { slotu = RAM_SLOTU; pametTyp = "RAM"; memset(ramSlot, 0xFF, sizeof ramSlot); }
  Hlav h; nZaz = 0;
  while (nZaz < slotu && hlavicka(nZaz, h)) { krok = h.krok + 1; polozka = h.polozka; nZaz++; }
}

void zacniZaznam(uint32_t T){
  memset(&rec.h, 0, sizeof rec.h);
  uint16_t n = ringN, start = (ringPos + PRE - ringN) % PRE;
  for (uint16_t k = 0; k < n; k++) rec.v[k] = ring[(start + k) % PRE];
  rec.h.nVz = n;
  recT0 = posledniVzT - (uint32_t)(n ? n - 1 : 0) * PERIODA;
  ringN = 0; ringPos = 0; nahrava = true;
}
void rana(uint32_t T){
  if (nahrava && T - posledniRanaT < 150) return;   // stejná rána podruhé (VR spoušť + ostrá detekce)
  if (!nahrava) zacniZaznam(T);
  if (rec.h.nRan < 4) { rec.h.ranaMs[rec.h.nRan] = (uint16_t)min((uint32_t)65535, T - recT0); rec.h.nRan++; }
  posledniRanaT = T; zelenaDo = T + 200; digitalWrite(LED_GREEN, LOW);
  hodnotRanu(T);
}
void ukonciZaznam(){
  nahrava = false;
  if (nZaz >= slotu && !flashOK && slotu > 1) {   // náhradní RAM: nejstarší záznam zahodit, ostatní posunout
    memmove(ramSlot[0], ramSlot[1], (size_t)(slotu - 1) * SLOT); memset(ramSlot[slotu - 1], 0xFF, SLOT);
    for (uint16_t i = 0; i < slotu - 1; i++) { Hlav* hh = (Hlav*)ramSlot[i]; if (hh->magic == ZMAGIC) hh->idx = i; }
    nZaz = slotu - 1;
  }
  if (nZaz >= slotu) { outObe("E PAMET_PLNA"); hlas(H_PAMET_PLNA); return; }
  Hlav& h = rec.h;
  h.magic = ZMAGIC; h.idx = nZaz; h.krok = krok; h.program = cfg2.program; h.perioda = PERIODA;
  h.unix0 = unixZaklad ? unixZaklad + recT0 / 1000 : 0; h.up0 = recT0; h.bat = (int8_t)batPct;
  h.predvolba = cfg2.predvolba; h.polozka = polozka;
  uint32_t len = sizeof(Hlav) + (uint32_t)h.nVz * sizeof(Vz);
  bool ok = slotZapis(nZaz, (uint8_t*)&rec, len);
  if (nZaz + 1 < slotu) { Hlav x; if (hlavicka(nZaz + 1, x)) slotSmaz(nZaz + 1); }   // starý záznam za koncem pryč
  if (ok) { outObe("N " + String(nZaz) + " krok=" + String(krok) + " ran=" + String(h.nRan)); nZaz++; krok++; }
  else outObe("E ZAPIS");
}
void zaznamVzorek(float ax, float ay, float az){
  Vz v; int16_t g[3] = {0,0,0}; imuGyro(g); memcpy(v.g, g, 6);
  float a[3] = {ax, ay, az};
  for (int i = 0; i < 3; i++) { int q = (int)lroundf(a[i] * 8.0f); v.a[i] = (int8_t)constrain(q, -127, 127); }
  int m = micMaxZ; micMaxZ = 0; v.mic = (uint8_t)min(255, m >> 7);
  if (!nahrava) { ring[ringPos] = v; ringPos = (ringPos + 1) % PRE; if (ringN < PRE) ringN++; }
  else if (rec.h.nVz < MAXV) rec.v[rec.h.nVz++] = v;
}

// base64 pro přenos záznamu textem
const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
String b64(const uint8_t* d, int n){ String s; s.reserve((n+2)/3*4);
  for (int i = 0; i < n; i += 3) { uint32_t x = (uint32_t)d[i] << 16 | (i+1<n ? d[i+1] << 8 : 0) | (i+2<n ? d[i+2] : 0);
    s += B64[(x>>18)&63]; s += B64[(x>>12)&63]; s += i+1<n ? B64[(x>>6)&63] : '='; s += i+2<n ? B64[x&63] : '='; } return s; }
void posliZaznam(int i){
  Hlav h; if (i < 0 || !hlavicka(i, h)) { out("R chyba " + String(i)); return; }
  uint32_t len = sizeof(Hlav) + (uint32_t)h.nVz * sizeof(Vz), sum = 0; uint8_t buf[180];
  bleZasek = false; out("R " + String(i) + " " + String(len));
  for (uint32_t off = 0; off < len; off += sizeof buf) {
    if (bleZasek) { Serial.println("! prenos zaznamu prerusen"); return; }   // spojení vázne – nepokračovat, ať se spoušť nezasekne
    uint32_t n = min((uint32_t)sizeof buf, len - off); slotCti(i, off, buf, n);
    for (uint32_t k = 0; k < n; k++) sum += buf[k];
    out("r " + b64(buf, n));
  }
  out("R konec " + String(i) + " " + String(sum));
}
// ---------- poklepání na krabičku: 3× rychle = START, 2× = STOP (když záznam běží) ----------
// ťuk = krátká špička rázu mezi poklep_g a poklep_max; ťuky max. 0,5 s od sebe; po 0,55 s ticha se vyhodnotí.
// Ťuky těsně po ráně (otevření zbraně, vyhazovač) a cokoli silného jako rána se nepočítá.
uint8_t poklepZap = 1; float poklepG = 1.2f, poklepMax = 5.0f; const char* POKLEP_SOUBOR = "/poklep.txt";
uint8_t pkN = 0; uint32_t pkPosl = 0, pkPrvni = 0, cervenaDo = 0, barvaDo = 0; bool barvaSviti = false; float pkMax = 0; bool pkVadny = false;
const int PIEP = D1;   // volitelný piezo pípák mezi D1 a GND (bez něj se nic nestane)
void poklepNacti(){ LfsFile f(InternalFS); char b[24] = {0};
  if (f.open(POKLEP_SOUBOR, FILE_O_READ)) { f.read(b, 23); f.close(); int z = 1; float g = 1.2f, m = 5.0f;
    if (sscanf(b, "%d %f %f", &z, &g, &m) >= 2) { poklepZap = z ? 1 : 0; poklepG = constrain(g, 0.3f, 8.0f); poklepMax = constrain(m, poklepG + 0.5f, 16.0f); } } }
bool poklepUloz(){ InternalFS.remove(POKLEP_SOUBOR); LfsFile f(InternalFS); if (!f.open(POKLEP_SOUBOR, FILE_O_WRITE)) return false;
  String t = String(poklepZap) + " " + String(poklepG, 2) + " " + String(poklepMax, 2); f.write((const uint8_t*)t.c_str(), t.length()); f.close(); return true; }
String poklepText(){ return "T poklep=" + String(poklepZap) + " poklep_g=" + String(poklepG, 1) + " poklep_max=" + String(poklepMax, 1); }
// PT-2040PQ hraje nejhlasitěji kolem 4 kHz (rezonance) – START 3 krátká vysoká, STOP 2 dlouhá nižší
void pipni(int n, int hz, int ms){ for (int i = 0; i < n; i++) { tone(PIEP, hz, ms); delay(ms + 60); } noTone(PIEP); }
// =====================================================================
// ---------- HLAS z pieza (verze 21) ----------
// Řeč 8 kHz / 8 bit uložená ve flash (hlasky.h, ~105 kB – RAM to nezabírá, v RAM jen 1 kB vyrovnávací paměť).
// PWM3 s nosnou 64 kHz (16 MHz / 250) a každý vzorek 8 period → 8000 vzorků/s, dvojitý buffer přes EasyDMA.
// Piezo samo nosnou 64 kHz nepřehraje (rezonance ~4 kHz) a funguje jako filtr – slyšet je jen řeč.
// pipni() používá tone() = jiný PWM blok, takže se nepřekážejí; před řečí se tone() vypne.
// =====================================================================
uint8_t hlasZap = 1, hodnZap = 1, hlasPripraven = 0; const char* HLAS_SOUBOR = "/hlas.txt";
void hlasNacti(){ LfsFile f(InternalFS); char b[4] = {0}; if (f.open(HLAS_SOUBOR, FILE_O_READ)) { int n = f.read(b, 2); f.close(); if (n >= 2) { hlasZap = b[0] == '1'; hodnZap = b[1] == '1'; } } }
bool hlasUloz(){ InternalFS.remove(HLAS_SOUBOR); LfsFile f(InternalFS); if (!f.open(HLAS_SOUBOR, FILE_O_WRITE)) return false; char b[2] = { char('0' + hlasZap), char('0' + hodnZap) }; f.write((const uint8_t*)b, 2); f.close(); return true; }
String hlasText(){ return "H hlas=" + String(hlasZap) + " hodnoceni=" + String(hodnZap); }

static uint16_t hBuf[2][256];
static const uint8_t* volatile hPtr = 0; volatile uint32_t hZbyva = 0; volatile bool hHraje = false; volatile uint32_t hKonec = 0;
uint8_t hFronta[8]; uint8_t hfR = 0, hfW = 0;
inline void hNaplnit(int i){ uint16_t* b = hBuf[i];
  for (int k = 0; k < 256; k++) { uint8_t s = 128; if (hZbyva) { s = *hPtr++; hZbyva--; } b[k] = (uint16_t)((s * 250u) >> 8) | 0x8000; } }
void hPinPryc(){ NRF_PWM3->ENABLE = 0; NRF_PWM3->PSEL.OUT[0] = 0xFFFFFFFFUL; pinMode(PIEP, OUTPUT); digitalWrite(PIEP, LOW); }
extern "C" void PWM3_IRQHandler(void){
  if (NRF_PWM3->EVENTS_SEQEND[0]) { NRF_PWM3->EVENTS_SEQEND[0] = 0; hNaplnit(0); }
  if (NRF_PWM3->EVENTS_SEQEND[1]) { NRF_PWM3->EVENTS_SEQEND[1] = 0; hNaplnit(1); }
  if (NRF_PWM3->EVENTS_STOPPED)   { NRF_PWM3->EVENTS_STOPPED = 0; hPinPryc(); hHraje = false; hKonec = millis(); }
}
void hSpust(uint8_t idx){
  if (idx >= HLASEK) return;
  noTone(PIEP); pinMode(PIEP, OUTPUT); digitalWrite(PIEP, LOW);   // v22: pin jako výstup (jinak PWM na pin nemusí dosáhnout)
  hPtr = HLASKY[idx].d; hZbyva = HLASKY[idx].n;
  hNaplnit(0); hNaplnit(1);
  NRF_PWM3->PSEL.OUT[0] = g_ADigitalPinMap[PIEP];
  NRF_PWM3->MODE = PWM_MODE_UPDOWN_Up; NRF_PWM3->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1; NRF_PWM3->COUNTERTOP = 250;
  NRF_PWM3->DECODER = (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
  for (int s = 0; s < 2; s++) { NRF_PWM3->SEQ[s].PTR = (uint32_t)hBuf[s]; NRF_PWM3->SEQ[s].CNT = 256; NRF_PWM3->SEQ[s].REFRESH = 7; NRF_PWM3->SEQ[s].ENDDELAY = 0; }
  NRF_PWM3->LOOP = (HLASKY[idx].n + 511) / 512 + 1;      // každá smyčka = SEQ0 + SEQ1 = 512 vzorků, +1 smyčka ticha na konec
  NRF_PWM3->SHORTS = PWM_SHORTS_LOOPSDONE_STOP_Msk;
  NRF_PWM3->EVENTS_SEQEND[0] = 0; NRF_PWM3->EVENTS_SEQEND[1] = 0; NRF_PWM3->EVENTS_STOPPED = 0; NRF_PWM3->EVENTS_LOOPSDONE = 0;
  NRF_PWM3->INTENSET = PWM_INTENSET_SEQEND0_Msk | PWM_INTENSET_SEQEND1_Msk | PWM_INTENSET_STOPPED_Msk;
  NVIC_SetPriority(PWM3_IRQn, 6); NVIC_ClearPendingIRQ(PWM3_IRQn); NVIC_EnableIRQ(PWM3_IRQn);
  hHraje = true;
  NRF_PWM3->ENABLE = 1; NRF_PWM3->TASKS_SEQSTART[0] = 1;
}
void hlas(uint8_t idx){ if (!hlasZap || !hlasPripraven || idx >= HLASEK) return; uint8_t w = (hfW + 1) & 7; if (w == hfR) return; hFronta[hfW] = idx; hfW = w; }
void hlasStop(){ hfR = hfW; if (hHraje) { NRF_PWM3->TASKS_STOP = 1; uint32_t t0 = millis(); while (hHraje && millis() - t0 < 20) {} if (hHraje) { hPinPryc(); hHraje = false; hKonec = millis(); } } }
void hlasKrok(){ if (!hHraje && hfR != hfW && millis() - hKonec > 120) { uint8_t i = hFronta[hfR]; hfR = (hfR + 1) & 7; hSpust(i); } }
bool hlasMluvi(){ return hHraje || hfR != hfW || millis() - hKonec < 150; }   // po tu dobu se nebere mikrofon (piezo je hned vedle)
void hlasDomluv(uint32_t max){ uint32_t t0 = millis(); while (millis() - t0 < max) { hlasKrok(); if (!hlasMluvi()) break; delay(5); } }
void hlasUspani(const char* proc){   // volá vypnoutUplne() – řekne proč a počká, než domluví
  if (!hlasPripraven || !hlasZap) return; hlasStop();
  hlas(strcmp(proc, "baterka") == 0 ? H_VYBITO : H_VYPINAM); hlasDomluv(2500); }
// pípnutí a hned slovo (START / STOP / úroveň baterky …)
void pipniARekni(int n, int hz, int ms, uint8_t idx){ hlasStop(); pipni(n, hz, ms); hlas(idx); }

// ---------- HODNOCENÍ KAŽDÉ RÁNY podle gyroskopu (stejná pravidla jako stránka zaznam.html) ----------
// rychlost švihu |ω| v °/s každých 10 ms do kruhu 1,28 s; 320 ms po ráně se vyhodnotí:
//   před  = nejvyšší rychlost 600–150 ms před ranou, těsně = průměr posledních 150 ms, po = průměr 100–300 ms po ráně
//   těsně < 0,4·před → „zastavil jsi“, těsně < 0,75·před → „zpomaluješ“, po/těsně < 0,6 → „dotáhni švih“, jinak „dobře“
//   před < 30 °/s (stojící zbraň) → mlčí
const int HR_N = 128; uint16_t hrMag[HR_N]; uint8_t hrPos = 0; uint32_t hrT = 0, hrPosledni = 0;   // hrMag v 0,1 °/s
uint32_t hodnRanaT = 0;               // rána čekající na vyhodnocení
uint8_t hodnVysl[2]; uint8_t hodnN = 0; uint32_t hodnPosledniRana = 0;
uint32_t strhT = 0; void nahozRana(uint32_t T);
void hodnotRanu(uint32_t T){ if (imuStav != 0) return; if (T - hodnPosledniRana < 150 && hodnPosledniRana) return; hodnPosledniRana = T;
  if (hodnZap) hodnRanaT = T; if (strhZap) strhT = T; if (rezim == 3) nahozRana(T); hlasStop(); }
float hrPrumer(uint32_t od, uint32_t doT, bool maximum){   // čas v ms vůči hrT (poslední vzorek)
  float s = 0, m = 0; int n = 0;
  for (int k = 0; k < HR_N; k++) { uint32_t t = hrT - (uint32_t)k * 10; if ((int32_t)(t - od) < 0) break; if ((int32_t)(t - doT) > 0) continue;
    float v = hrMag[(hrPos + HR_N - 1 - k) % HR_N] * 0.1f; s += v; n++; if (v > m) m = v; }
  return maximum ? m : (n ? s / n : 0); }
float hrPosl = 0;   // poslední rychlost |ω| °/s
void hodnoceniKrok(uint32_t now){
  if (!(hodnZap || strhZap || rezim == 3) || imuStav != 0) return;
  if ((int32_t)(now - hrPosledni) >= 10) { hrPosledni = now; int16_t g[3];
    if (imuGyro(g)) { float m = sqrtf((float)g[0]*g[0] + (float)g[1]*g[1] + (float)g[2]*g[2]) * 0.07f; hrMag[hrPos] = (uint16_t)min(m * 10.0f, 65535.0f); hrPos = (hrPos + 1) % HR_N; hrT = now; hrPosl = m; } }
  if (hodnRanaT && now - hodnRanaT >= 320) { uint32_t T = hodnRanaT; hodnRanaT = 0;
    float pred = hrPrumer(T - 600, T - 150, true), tesne = hrPrumer(T - 150, T, false), po = hrPrumer(T + 100, T + 300, false);
    int8_t v = -1;
    if (pred >= 30) { if (tesne < 0.4f * pred) v = H_ZASTAVIL; else if (tesne < 0.75f * pred) v = H_ZPOMALUJES; else if (tesne > 1 && po / tesne < 0.6f) v = H_DOTAHNI; else v = H_DOBRE; }
    outObe("R hodnoceni pred=" + String(pred, 0) + " tesne=" + String(tesne, 0) + " po=" + String(po, 0) + " -> " + String(v < 0 ? "-" : HLASKY[v].jm));
    if (v >= 0 && hodnN < 2) hodnVysl[hodnN++] = v; }
  // mluvit až v klidu (900 ms bez rány), ať řeč neruší druhou ránu dubletu
  // strhávání: klid před ranou (350–80 ms) proti pohybu v okamžiku stisku (60 ms před až 80 ms po; u ostré kule jen do rány – pak zpětný ráz)
  if (strhT && now - strhT >= 110) { uint32_t T = strhT; strhT = 0;
    float zaklad = hrPrumer(T - 350, T - 80, false), spicka = hrPrumer(T - 60, rezim == 2 ? T - 5 : T + 80, true);
    int8_t v = spicka - zaklad > 12.0f ? H_STRHL : H_CISTE;
    outObe("R strhavani klid=" + String(zaklad, 1) + " spicka=" + String(spicka, 1) + " -> " + HLASKY[v].jm);
    if (hodnN < 2) hodnVysl[hodnN++] = v; }
  if (hodnN && !hodnRanaT && !strhT && now - hodnPosledniRana > 900) {
    for (uint8_t i = 0; i < hodnN; i++) { bool ok = hodnVysl[i] == H_DOBRE || hodnVysl[i] == H_CISTE; pipni(ok ? 1 : 2, ok ? 4000 : 1500, ok ? 80 : 160); hlas(hodnVysl[i]); }
    hodnN = 0; }
}
// varování: slabá baterka (každých 10 min), nabíjení po připojení USB
uint32_t hlasBatT = 0; bool hlasUsb = false;
void hlasHlidej(uint32_t now){
  bool usb = usbNapajeni(); if (usb && !hlasUsb && now > 3000) hlas(H_NABIJI); hlasUsb = usb;
  if (!usb && batPct >= 0 && batPct < 15 && (hlasBatT == 0 || now - hlasBatT > 600000UL)) { hlasBatT = now; hlas(H_SLABA); }
}
// ---------- tlačítko na víčku (TS6601, mezi D2 a GND): krátce = START / STOP, dlouze (2 s) = baterka barvou + pípnutí ----------
// =====================================================================
// ---------- MENU REŽIMŮ (verze 22) ----------
// =====================================================================
const uint8_t REZIMU = 5;
const uint8_t REZ_HL[REZIMU] = { H_M_SUSENI, H_M_SKEET, H_M_KULE, H_M_NAHOZ, H_M_SPOUST };
const char* REZ_JM[REZIMU] = { "suseni doma", "skeet strelnice", "kule pistole/puska", "trenink nahazovani", "trenink spousteni" };
uint8_t rezim = 0; bool strhZap = false; bool vMenu = true; uint32_t menuT = 0; const char* REZ_SOUBOR = "/rezim.txt";
void rezimNacti(){ LfsFile f(InternalFS); char b[3] = {0}; if (f.open(REZ_SOUBOR, FILE_O_READ)) { if (f.read(b, 1) == 1 && b[0] >= '0' && b[0] < '0' + REZIMU) rezim = b[0] - '0'; f.close(); } }
bool rezimUloz(){ InternalFS.remove(REZ_SOUBOR); LfsFile f(InternalFS); if (!f.open(REZ_SOUBOR, FILE_O_WRITE)) return false; char c = '0' + rezim; f.write((const uint8_t*)&c, 1); f.close(); return true; }
String rezimText(){ return "M rezim=" + String(rezim + 1) + " " + REZ_JM[rezim] + (vMenu ? " (menu)" : ""); }
void rezimAplikuj(){
  switch (rezim) {
    case 0: cfg2.zaznam = 2; hodnZap = 1; strhZap = false; break;   // sušení doma: VR spoušť + záznam cvaknutí + hodnocení švihu
    case 1: cfg2.zaznam = 1; hodnZap = 1; strhZap = false; break;   // skeet střelnice: jen záznam ostrých ran + hodnocení švihu
    case 2: cfg2.zaznam = 1; hodnZap = 0; strhZap = true;  break;   // kule: záznam + strhávání
    case 3: cfg2.zaznam = 0; hodnZap = 0; strhZap = false; break;   // trénink nahazování (VR spoušť dál funguje)
    default: cfg2.zaznam = 0; hodnZap = 0; strhZap = true; break;   // trénink spouštění
  }
  cfg2.bezi = 0; cfg2Uloz(); gyroNastav(); }
void menuOznam(bool zastav){ if (zastav) hlasStop(); pipni(rezim + 1, 2600, 60); hlas(REZ_HL[rezim]); menuT = millis(); outObe(rezimText()); }
void menuStart(bool uvod){ if (nahrava) ukonciZaznam(); cfg2.bezi = 0; vMenu = true;
  if (uvod) { hlasStop(); pipni(1, 1500, 300); } hlas(H_MENU); menuOznam(false); }
void menuVyber(){ vMenu = false; rezimAplikuj(); rezimUloz(); hlasStop(); pipni(1, 4000, 500); hlas(H_VYBRANO); hlas(REZ_HL[rezim]); outObe(rezimText()); }

// ---- trénink nahazování: stisk → „pozor“ → náhodně 1,5–4 s → pípnutí → nahodit; čas = do zklidnění zbraně (150 ms pod 15 °/s po pohybu nad 80 °/s) nebo do cvaknutí
uint8_t nhStav = 0; uint32_t nhT = 0, nhGo = 0, nhKlid = 0; bool nhPohyb = false;
void hlasCas(uint32_t ms){ uint32_t d = (ms + 50) / 100; uint8_t sek = min((uint32_t)9, d / 10), des = d % 10; hlas(H_N0 + sek); hlas(H_CELA); hlas(H_N0 + des); }
void nahozHotovo(uint32_t T, const char* co){ uint32_t t = T - nhGo; nhStav = 0;
  outObe("N nahoz " + String(t / 1000.0f, 2) + " s (" + co + ")"); pipni(1, 3000, 60); hlasCas(t); }
void nahozRana(uint32_t T){ if (nhStav == 2) nahozHotovo(T, "cvak"); }
void nahozStart(){ hlasStop(); hlas(H_POZOR); nhStav = 1; nhT = millis() + 1500 + random(0, 2500); nhPohyb = false; nhKlid = 0; outObe("N pozor"); }
void nahozKrok(uint32_t now){
  if (nhStav == 1 && (int32_t)(now - nhT) >= 0 && !hlasMluvi()) { pipni(1, 4000, 150); nhGo = millis(); nhStav = 2; nhPohyb = false; nhKlid = 0; }
  else if (nhStav == 2) {
    if (hrPosl > 80) { nhPohyb = true; nhKlid = 0; }
    else if (nhPohyb && hrPosl < 15) { if (!nhKlid) nhKlid = now; else if (now - nhKlid >= 150) nahozHotovo(nhKlid, "klid"); }
    else nhKlid = 0;
    if (nhStav == 2 && now - nhGo > 6000) { nhStav = 0; pipni(2, 1500, 200); outObe("N nahoz nic"); } } }

// ---- tlačítko: počítání stisků (2 rychle po sobě = potvrzení / menu) ----
uint8_t tlKliky = 0; uint32_t tlPustil = 0;
void tlKlik(uint8_t n, uint32_t now){
  if (vMenu) { if (n >= 2) menuVyber(); else { rezim = (rezim + 1) % REZIMU; menuOznam(true); } return; }
  if (n >= 2) { menuStart(true); return; }
  if (rezim <= 2) {
    if (!cfg2.bezi) { zaznamStart(); digitalWrite(LED_GREEN, LOW); zelenaDo = now + 1500; pipniARekni(3, 4000, 70, H_START); }
    else { zaznamStop(); digitalWrite(LED_RED, LOW); cervenaDo = now + 1500; pipniARekni(2, 3200, 220, H_STOP); }
    outObe("T tlacitko -> " + String(cfg2.bezi ? "START" : "STOP")); outObe(stavText()); }
  else if (rezim == 3) nahozStart();
  else menuOznam(true);   // trénink spouštění: stisk jen připomene režim
}
void menuKrok(uint32_t now){
  if (tlKliky && now - tlPustil > 400 && digitalRead(D2) == HIGH) { uint8_t n = tlKliky; tlKliky = 0; tlKlik(n, now); }
  if (vMenu && now - menuT > 10000 && !tlKliky) menuVyber();   // 10 s bez stisku → vybrat
  if (!vMenu && rezim == 3) nahozKrok(now); }

const int TLAC = D2; uint32_t tlDole = 0; bool tlDlouhe = false, tlPred = false, tlVyp = false;
void tlacitkoKrok(uint32_t now){ bool dole = digitalRead(TLAC) == LOW;
  if (dole && !tlPred) { tlDole = now; tlDlouhe = false; tlVyp = false; }
  // podržení 5 s = vypnout (po puštění); 3 hluboká pípnutí
  if (dole && tlDlouhe && !tlVyp && tlDole && now - tlDole > 5000) { tlVyp = true; pipni(3, 1200, 150); }
  if (!dole && tlPred && tlVyp) { delay(300); vypnoutUplne("tlacitko"); }
  if (dole && !tlDlouhe && tlDole && now - tlDole > 2000) { tlDlouhe = true; barvaDo = now + 3000; batMer(); pipniARekni(batPct >= 70 ? 3 : batPct >= 30 ? 2 : 1, 3000, 60, batPct >= 70 ? H_BAT_PLNA : batPct >= 30 ? H_BAT_PUL : H_BAT_MALO); outObe(batText()); }
  if (!dole && tlPred && tlDole && !tlDlouhe && now - tlDole > 40) { tlKliky++; tlPustil = now; }   // krátký stisk – vyhodnotí menuKrok (čeká, jestli přijde druhý)
  if (!dole) tlDole = dole ? tlDole : 0; tlPred = dole; }
void zaznamStart(){ if (!cfg2.zaznam) { cfg2.zaznam = 1; gyroNastav(); }
  if (!cfg2.bezi) { polozka++; krok = 0; } cfg2.bezi = 1; ringN = 0; ringPos = 0; cfg2Uloz(); }
void zaznamStop(){ if (nahrava) ukonciZaznam(); cfg2.bezi = 0; cfg2Uloz(); }
void poklepKrok(float raz, uint32_t now){
  if (!poklepZap || imuStav != 0) return;
  bool poRane = (posledniRanaT && now - posledniRanaT < 4000) || (posledniVystrel && now - posledniVystrel < 4000 && cfg2.zaznam != 1);
  if (raz >= poklepMax || raz >= cfg2.prahRanaG || nahrava) { if (pkN) pkVadny = true; pkPosl = now; return; }
  if (raz > poklepG) {
    if (now - pkPosl >= 70 || pkN == 0) { if (pkN == 0) { pkPrvni = now; pkMax = 0; pkVadny = false; } pkN++; }   // nový ťuk (dozvuk do 70 ms je pořád tentýž)
    pkPosl = now; if (raz > pkMax) pkMax = raz; if (poRane) pkVadny = true;
    return; }
  if (pkN && now - pkPosl > 550) {   // ticho → vyhodnotit
    String co = "nic";
    if (!pkVadny && now - pkPrvni < 2500) {
      if (pkN == 3 && !cfg2.bezi) { zaznamStart(); co = "START"; digitalWrite(LED_GREEN, LOW); zelenaDo = now + 1500; }
      else if (pkN == 2 && cfg2.bezi) { zaznamStop(); co = "STOP"; digitalWrite(LED_RED, LOW); cervenaDo = now + 1500; } }
    outObe("T poklepani n=" + String(pkN) + " sila=" + String(pkMax, 1) + (pkVadny ? " (po rane / prilis silne)" : "") + " -> " + co);
    if (co == "START") { pipniARekni(3, 4000, 70, H_START); outObe(stavText()); } else if (co == "STOP") { pipniARekni(2, 3200, 220, H_STOP); outObe(stavText()); }
    pkN = 0; pkMax = 0; pkVadny = false; }
}

String jmenoBezMezer(){ String j = jmeno; j.replace(" ", "_"); return j; }
String stavText(){
 return "S verze=23 bezi=" + String(cfg2.bezi) + " zaznam=" + String(cfg2.zaznam) + " program=" + String(cfg2.program) + " polozka=" + String(polozka) + " krok=" + String(krok)
       + " pamet=" + pametTyp + " zaznamu=" + String(nZaz) + " kapacita=" + String(slotu) + " bat=" + String(batPct)
       + " nahrava=" + String(nahrava ? 1 : 0) + " cas=" + String(unixZaklad ? 1 : 0) + " imu=" + String(imuStav == 0 ? 1 : 0)
       + " predvolba=" + String(cfg2.predvolba == 255 ? -1 : (int)cfg2.predvolba) + " flash=" + String((unsigned long)flashJedec, HEX) + " sr2=" + String(flashSR2, HEX) + " test=" + String(flashTest) + " jmeno=" + jmenoBezMezer() + " klavesa=" + klNazev + " nab=" + String(nabMa) + " usin=" + String(usinMin) + " usb=" + String(usbNapajeni() ? 1 : 0) + " poklep=" + String(poklepZap) + " poklep_g=" + String(poklepG, 1) + " poklep_max=" + String(poklepMax, 1);
}
String pvText(int i){ const Predvolba& p = pv[i];
  return "P " + String(i) + "|" + String(p.nazev) + "|" + String(p.c.prahG,2) + "|" + String(p.c.prahMic) + "|" + String(p.c.rezim) + "|"
       + String(p.c.soubeh) + "|" + String(p.c.blokace) + "|" + String(p.c2.zaznam) + "|" + String(p.c2.program) + "|"
       + String(p.c2.prahRanaG,1) + "|" + String(p.c2.prahRanaMic); }

// nové příkazy kroku 8; vrací true, pokud příkaz zpracoval
bool prikaz2(const String& k, const String& v){
  if (k == "stav") { out(stavText()); return true; }
  if (k == "start") { zaznamStart(); out(stavText()); return true; }
  if (k == "stop")  { zaznamStop(); out(stavText()); return true; }
  if (k == "poklep" || k == "poklep_g" || k == "poklep_max") { if (v.length()) {
        if (k == "poklep") poklepZap = v.toInt() ? 1 : 0; else if (k == "poklep_g") poklepG = constrain(v.toFloat(), 0.3f, 8.0f); else poklepMax = constrain(v.toFloat(), 0.8f, 16.0f);
        if (poklepMax < poklepG + 0.5f) poklepMax = poklepG + 0.5f; if (!poklepUloz()) { out("CHYBA ulozeni"); return true; } }
      out(poklepText()); return true; }
  if (k == "baterka") { batMer(); barvaDo = millis() + 3000; out(batText()); return true; }
  if (k == "pipni") { out("OK pipam"); pipni(2, 4000, 90); return true; }
  if (k == "rezim" || k == "menu") { if (k == "menu") menuStart(true); else if (v.length()) { int r = v.toInt(); if (r >= 1 && r <= REZIMU) { rezim = r - 1; menuVyber(); } }
                                       out(rezimText()); return true; }
  if (k == "hlas" || k == "hodnoceni") { if (v.length()) { if (k == "hlas") hlasZap = v.toInt() ? 1 : 0; else { hodnZap = v.toInt() ? 1 : 0; gyroNastav(); }
                         if (!hlasUloz()) { out("CHYBA ulozeni"); return true; } } out(hlasText()); return true; }
  if (k == "rekni") { int i = v.toInt(); if (i < 0 || i >= HLASEK) { String t = "? rekni=0.." + String(HLASEK - 1) + ":"; for (int j = 0; j < HLASEK; j++) t += " " + String(j) + "=" + HLASKY[j].jm; out(t); return true; }
                      uint8_t z = hlasZap; hlasZap = 1; hlas(i); hlasZap = z; out(String("OK rikam ") + HLASKY[i].jm); return true; }
  if (k == "klavesa") { if (!v.length()) { out(String("K klavesa=") + klNazev); return true; }
                        if (!klavesaNastav(v)) { out("? klavesa: a-z 0-9 mezernik enter f1-f12 mys mys2 hlasitost+ hlasitost-"); return true; }
                        out(klavesaUloz() ? String("K klavesa=") + klNazev : String("CHYBA ulozeni")); return true; }
  if (k == "usinani") { if (v.length()) { usinMin = constrain(v.toInt(), 0, 240); usinUloz(); posledniPohyb = millis(); }
                        out("U usin=" + String(usinMin)); return true; }
  if (k == "nabijeni") { if (v.length()) { nabMa = v.toInt() <= 50 ? 50 : 100; nabijeniNastav(); if (!nabijeniUloz()) { out("CHYBA ulozeni"); return true; } }
                         out("B nab=" + String(nabMa)); return true; }
  if (k == "blikni") { blikniDo = millis() + 4000; out("OK blikam"); return true; }
  if (k == "odpojit_vse") { for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (Bluefruit.connected(h)) Bluefruit.disconnect(h); out("OK odpojeno"); return true; }
  if (k == "smazat_pary") { Bluefruit.Periph.clearBonds(); out("OK pary smazany"); delay(200); NVIC_SystemReset(); return true; }
  if (k == "restart") { out("OK restart"); delay(200); NVIC_SystemReset(); return true; }
  if (k == "jmeno") { String n; for (unsigned i = 0; i < v.length() && n.length() < 20; i++) { char c = v[i]; if (c >= 32 && c < 127 && c != '|') n += c; }
                      n.trim(); out(jmenoUloz(n) ? "OK jmeno ulozeno, plati po restartu: Skeet " + (n.length() ? n : String("Trigger ...")) : "CHYBA ulozeni"); return true; }
  if (k == "zaznam")        { cfg2.zaznam = constrain(v.toInt(), 0, 2); gyroNastav(); }
  else if (k == "program")  { cfg2.program = constrain(v.toInt(), 0, 9); }
  else if (k == "prah_rana_g")   cfg2.prahRanaG = constrain(v.toFloat(), 0.5f, 16.0f);
  else if (k == "prah_rana_mic") cfg2.prahRanaMic = constrain(v.toInt(), 300, 32767);
  else if (k == "test_rana") { if (!cfg2.zaznam || !cfg2.bezi || imuStav != 0) { out("? zaznam neni spusteny (START)"); return true; }
                              rana(millis()); out("OK test – zaznam se ulozi za 1,5 s"); return true; }
  else if (k == "vychozi2") { cfg2 = VYCHOZI2; gyroNastav(); }
  else if (k == "cas")      { uint32_t u = (uint32_t)strtoul(v.c_str(), nullptr, 10); if (u > 1600000000UL) unixZaklad = u - millis() / 1000; out(stavText()); return true; }
  else if (k == "nova_polozka") { polozka++; krok = 0; out(stavText()); return true; }
  else if (k == "zaznamy")  { Hlav h; for (uint16_t i = 0; i < nZaz; i++) if (hlavicka(i, h))
                                out("Z " + String(i) + " krok=" + String(h.krok) + " polozka=" + String(h.polozka) + " program=" + String(h.program)
                                    + " ran=" + String(h.nRan) + " vz=" + String(h.nVz) + " unix=" + String(h.unix0) + " up=" + String(h.up0));
                              out("Z konec " + String(nZaz)); return true; }
  else if (k == "poslat")   { posliZaznam(v.toInt()); return true; }
  else if (k == "smazat_zaznamy") { Hlav h; uint16_t i = 0; while (i < slotu && (i < nZaz || hlavicka(i, h))) slotSmaz(i++);
                              nZaz = 0; krok = 0; out("OK smazano"); out(stavText()); return true; }
  else if (k == "predvolby"){ for (int i = 0; i < PREDVOLEB; i++) if (pv[i].nazev[0]) out(pvText(i)); out("P konec"); return true; }
  else if (k == "predvolba_uloz") { int sp = v.indexOf(' '); int i = (sp < 0 ? v : v.substring(0, sp)).toInt(); String n = sp < 0 ? "" : v.substring(sp + 1);
                              n.replace("|", "/"); n.trim(); if (!n.length()) n = "Predvolba " + String(i + 1);
                              if (i < 0 || i >= PREDVOLEB) { out("? predvolba 0-5"); return true; }
                              memset(pv[i].nazev, 0, 16); strncpy(pv[i].nazev, n.c_str(), 15); pv[i].c = cfg; pv[i].c2 = cfg2; pv[i].c2.predvolba = i;
                              cfg2.predvolba = i; out(pvUloz() && cfg2Uloz() ? "OK predvolba ulozena" : "CHYBA ulozeni"); out(pvText(i)); return true; }
  else if (k == "predvolba") { int i = v.toInt(); if (i < 0 || i >= PREDVOLEB || !pv[i].nazev[0]) { out("? prazdna predvolba"); return true; }
                              cfg = pv[i].c; cfg.magic = VYCHOZI.magic; cfg2 = pv[i].c2; cfg2.magic = VYCHOZI2.magic; cfg2.predvolba = i;
                              gyroNastav(); out(cfgUloz() && cfg2Uloz() ? "OK predvolba " + String(pv[i].nazev) : "CHYBA ulozeni");
                              out(cfgText()); out(cfg2Text()); return true; }
  else if (k == "predvolba_smaz") { int i = v.toInt(); if (i >= 0 && i < PREDVOLEB) { memset(&pv[i], 0, sizeof pv[i]); pvUloz(); if (cfg2.predvolba == i) cfg2.predvolba = 255; }
                              out("P konec"); return true; }
  else return false;
  out(cfg2Text()); return true;
}

void prikaz(String r, uint8_t zdroj){
  zdrojPrikazu = zdroj;
  r.trim(); if (!r.length()) return;
  int sp = r.indexOf(' '); String k = sp<0 ? r : r.substring(0,sp); String v = sp<0 ? "" : r.substring(sp+1); v.trim();
  if (prikaz2(k, v)) { zdrojPrikazu = 0; return; }
  if      (k == "info")     {}
  else if (k == "prah_g")   cfg.prahG   = constrain(v.toFloat(), 0.5f, 30.0f);
  else if (k == "prah_mic") cfg.prahMic = constrain(v.toInt(), 100, 32767);
  else if (k == "rezim")    cfg.rezim   = constrain(v.toInt(), 0, 3);
  else if (k == "soubeh")   cfg.soubeh  = constrain(v.toInt(), 5, 500);
  else if (k == "blokace")  cfg.blokace = constrain(v.toInt(), 50, 2000);
  else if (k == "vychozi")  cfg = VYCHOZI;
  else if (k == "ulozit")   out(cfgUloz() && cfg2Uloz() ? "OK ulozeno" : "CHYBA ulozeni");
  else if (k == "stream")   stream = v.toInt() != 0;
  else { out("? neznamy prikaz: " + k); zdrojPrikazu = 0; return; }
  out(cfgText());
  if (zdroj == 1 || k == "info") out(cfg2Text());
  zdrojPrikazu = 0;
}
String bufUsb, bufBle;
void ctiPrikazy(){
  while (Serial.available()) { char c = Serial.read();
    if (c=='\n'||c=='\r') { prikaz(bufUsb, 0); bufUsb=""; } else if (bufUsb.length()<60) bufUsb += c; }
  while (bleuart.available()) { char c = (char)bleuart.read();
    if (c=='\n'||c=='\r') { prikaz(bufBle, 1); bufBle=""; } else if (bufBle.length()<60) bufBle += c; }
}

// před úplným spánkem odpojit vše, co by dál bralo proud: akcelerometr (napájený z pinu), mikrofon (napájený z pinu),
// sběrnici I2C (přes pull-upy by akcelerometr „žil“ dál) a paměť QSPI (příkaz deep power-down). Jinak by se baterka za pár dní vybila.
void vsechnoVypnout(){
  PDM.end();
#ifdef PIN_PDM_PWR
  digitalWrite(PIN_PDM_PWR, LOW);
#endif
  Wire.end();
#ifdef PIN_WIRE_SDA
  nrf_gpio_cfg_default(g_ADigitalPinMap[PIN_WIRE_SDA]); nrf_gpio_cfg_default(g_ADigitalPinMap[PIN_WIRE_SCL]);
#endif
#ifdef PIN_LSM6DS3TR_C_POWER
  { uint32_t pin = g_ADigitalPinMap[PIN_LSM6DS3TR_C_POWER]; nrf_gpio_pin_clear(pin); nrf_gpio_cfg_default(pin); }
#endif
#if POUZIT_QSPI
  if (flashJedec) { flashTransport.runCommand(0xB9); delayMicroseconds(50); }
#endif
  digitalWrite(LED_BLUE, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_RED, HIGH);
  noTone(PIEP);
}
void setup() {
  Serial.begin(115200);
  InternalFS.begin(); cfgNacti(); cfg2Nacti(); pvNacti(); jmenoNacti(); klavesaNacti(); nabijeniNacti(); usinNacti(); poklepNacti(); hlasNacti(); rezimNacti(); posledniPohyb = millis();
  pinMode(LED_BLUE, OUTPUT); pinMode(LED_GREEN, OUTPUT); pinMode(LED_RED, OUTPUT); pinMode(TLAC, INPUT_PULLUP);
  digitalWrite(LED_BLUE, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_RED, HIGH);
  pinMode(VYPINAC, INPUT_PULLUP); delay(5);
  if (vypinacVyp()) {   // přepínač na VYPNUTO (probudilo ho USB nebo RESET) → hned zpátky spát, Bluetooth se ještě nespouští
    nrf_gpio_cfg_sense_input(g_ADigitalPinMap[VYPINAC], NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
    digitalWrite(LED_RED, LOW); delay(60); digitalWrite(LED_RED, HIGH);
    NRF_POWER->SYSTEMOFF = 1; while (1) {} }

#ifdef PIN_LSM6DS3TR_C_POWER
  { // napájení akcelerometru z GPIO v režimu "high drive", jinak čip nenaběhne
    uint32_t pin = g_ADigitalPinMap[PIN_LSM6DS3TR_C_POWER];
    nrf_gpio_cfg(pin, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_set(pin); delay(50);
  }
#else
  imuStav = -50;   // pin pro napájení akcelerometru není v této desce definován
#endif
  if (imuStav != -50) imuInit();
  if (imuStav != 0) { Serial.println("Akcelerometr nenalezen!"); digitalWrite(LED_RED, LOW); }
  gyroNastav();

#ifdef PIN_PDM_PWR
  pinMode(PIN_PDM_PWR, OUTPUT); digitalWrite(PIN_PDM_PWR, HIGH); delay(10);
#endif
  PDM.onReceive(onPDMdata);
  PDM.setBufferSize(128);   // 64 vzorků = 4 ms (výchozí 512 B = 16 ms čekání na blok)
  PDM.setGain(20);
  if (!PDM.begin(1, 16000)) { Serial.println("Mikrofon nenalezen!"); digitalWrite(LED_RED, LOW); }

  pametInit();
  batInit();
  // stejně jako verze 8, se kterou se iPhone připojoval: větší šířka pásma + 3 spojení (Quest + telefon + rezerva)
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  bool bleOk = Bluefruit.begin(3, 0);
  if (!bleOk) { Serial.println("CHYBA: Bluetooth se nespustil (Bluefruit.begin)"); }
  Bluefruit.autoConnLed(false);   // modrou LED řídí program
  Bluefruit.setTxPower(4);
  Bluefruit.setName(plneJmeno);
  Bluefruit.Periph.setConnInterval(6, 12);   // 7,5–15 ms: klávesa se dostane do brýlí dřív
  Bluefruit.Periph.setConnectCallback(priPripojeni);
  Bluefruit.Periph.setDisconnectCallback(priOdpojeni);
  Bluefruit.Security.setPairCompleteCallback(parovaniHotovo);   // nepovedené párování = stará paměť párů → smazat, další pokus projde
  bledis.setManufacturer("SK Cil"); bledis.setModel("Skeet Trigger"); bledis.begin();
  blebas.begin(); batMer(); blebas.write(batPct < 0 ? 100 : batPct);
  blehid.begin();
  bleuart.begin();
  startAdv();
  Serial.print(plneJmeno); Serial.println(" pripraven – sparujte v Questu");
  barvaDo = millis() + 3000;   // po zapnutí 3 s ukázat barvou stav baterky
  hlasPripraven = 1; hlasUsb = usbNapajeni();
  pipni(1, 4000, 60); hlas(imuStav != 0 ? H_CHYBA_SENZORU : H_ZAPNUTO);
  rezimAplikuj(); menuStart(false);   // v22: po zapnutí menu (1 stisk = další, 2 = vybrat, 10 s = vybere samo)
}

uint32_t posledniTep = 0;
void tep(){   // klávesa F13–F23 nese stav baterky po 10 %, F24 = napájeno z USB (verze 18); nic nepíše
  if (!usbNapajeni() && batPct < 0) return;
  uint8_t key = usbNapajeni() ? 0x73 : 0x68 + (uint8_t)((batPct + 5) / 10);   // F13 = 0x68 … F23 = 0x72, F24 = 0x73
  hidReport(key); delay(8); hidRelease();
}
uint32_t posledniStream = 0; float accMaxS = 0; int micMaxS = 0;
void loop() {
  { static uint32_t vypOd = 0; if (vypinacVyp()) { if (!vypOd) vypOd = millis(); else if (millis() - vypOd > 80) vypnoutUplne("vypinac"); } else vypOd = 0; }   // přepínač na VYPNUTO
  ctiPrikazy();
  // ráz = velikost změny vektoru zrychlení oproti měření před ≥0,6 ms (akcelerometr měří 1666×/s)
  static float px = 0, py = 0, pz = 0; static uint32_t pT = 0; static bool prvni = true;
  float x = px, y = py, z = pz, raz = 0;
  uint32_t us = micros();
  if (imuStav == 0 && us - pT >= 600 && imuAccel(x, y, z)) {
    if (!prvni) raz = sqrtf((x-px)*(x-px) + (y-py)*(y-py) + (z-pz)*(z-pz));
    px = x; py = y; pz = z; pT = us; prvni = false;
  }
  if (raz > accMax05) accMax05 = raz;
  if (raz > accMaxS) accMaxS = raz;
  if (micSpicka > micMaxS) micMaxS = micSpicka;
  uint32_t now = millis();
  bleLogy();
  if (smazatPary) { smazatPary = false; Bluefruit.Periph.clearBonds(); outObe("E PAROVANI_SELHALO pary smazany"); }
  if (raz > cfg.prahG) accCasPrahu = now;
  if (hlasMluvi()) { micCasPrahu = 0; ranaMicT = 0; }   // vlastní řeč z pieza není výstřel
  poklepKrok(raz, now); tlacitkoKrok(now);
  hlasKrok(); hodnoceniKrok(now); hlasHlidej(now); menuKrok(now);

  // ---- VR spoušť (beze změny; v režimu "jen záznam" vypnutá) ----
  if (cfg2.zaznam != 1 && now - posledniVystrel > cfg.blokace) {
    bool acc = accCasPrahu && now - accCasPrahu <= cfg.soubeh;
    bool mic = micCasPrahu && now - micCasPrahu <= cfg.soubeh;
    if      (cfg.rezim == 0 && accCasPrahu == now) vystrel("raz");
    else if (cfg.rezim == 1 && mic)                { vystrel("cvak"); micCasPrahu = 0; }
    else if (cfg.rezim == 2 && acc && mic)         { vystrel("raz+cvak"); accCasPrahu = 0; micCasPrahu = 0; }
    else if (cfg.rezim == 3 && (accCasPrahu == now || mic)) {   // rychlý: první signál hned
      prvniSignal = accCasPrahu == now ? 1 : 2; bool obe = acc && mic; vystrel(obe ? "raz+cvak" : prvniSignal == 1 ? "raz" : "cvak");
      cekaPotvrzeni = obe ? 0 : now; if (obe) prvniSignal = 0; if (prvniSignal == 2) micCasPrahu = 0; }
  }
  if (cekaPotvrzeni) {   // druhý signál (do „soubeh“ ms) jen potvrzuje, že to byla spoušť
    bool druhy = prvniSignal == 1 ? (micCasPrahu && micCasPrahu >= cekaPotvrzeni) : (accCasPrahu && accCasPrahu >= cekaPotvrzeni);
    if (druhy) { Serial.println("E POTVRZENO"); cekaPotvrzeni = 0; micCasPrahu = 0; accCasPrahu = 0; }
    else if (now - cekaPotvrzeni > cfg.soubeh) { nepotvrzeno++; Serial.println(prvniSignal == 1 ? "E NEPOTVRZENO jen raz (zvedni prah_g?)" : "E NEPOTVRZENO jen cvak (zvedni prah_mic?)"); cekaPotvrzeni = 0; }
  }
  if (pustitV && (int32_t)(now - pustitV) >= 0) { pustitV = 0; pustitKlavesu(); }

  // ---- záznam ze střelnice ----
  if (cfg2.zaznam && cfg2.bezi && imuStav == 0) {
    if ((int32_t)(now - dalsiVz) >= 0) {
      dalsiVz += PERIODA; if ((int32_t)(now - dalsiVz) > 100) dalsiVz = now + PERIODA;
      posledniVzT = now; zaznamVzorek(px, py, pz);
    }
    if (raz > cfg2.prahRanaG) ranaAccT = now;
    if (micSpicka > cfg2.prahRanaMic) ranaMicT = now;
    if (ranaAccT && ranaMicT && now - ranaAccT <= 40 && now - ranaMicT <= 40 && (posledniRanaT == 0 || now - posledniRanaT > 150)) {
      rana(ranaAccT); ranaAccT = 0; ranaMicT = 0;
    }
    if (nahrava && (now - posledniRanaT > PO_RANE_MS || rec.h.nVz >= MAXV)) ukonciZaznam();
  }

  if (now - posledniVystrel > 80 && (int32_t)(now - zelenaDo) > 0) digitalWrite(LED_GREEN, HIGH);
  if (cervenaDo && (int32_t)(now - cervenaDo) > 0) { digitalWrite(LED_RED, HIGH); cervenaDo = 0; }
  // LED: modrá trvale = spárováno, modrá bliká 1× za s = hledá Quest; červené bliknutí = baterka pod 15 %
  bool modra = Bluefruit.connected() ? true : ((now % 1000) < 120);
  if ((int32_t)(blikniDo - now) > 0) {   // „blikni“: všechny LED rychle blikají, ať je poznat, která spoušť to je
    bool on = (now / 100) % 2; digitalWrite(LED_BLUE, on ? LOW : HIGH); digitalWrite(LED_RED, on ? HIGH : LOW); digitalWrite(LED_GREEN, on ? LOW : HIGH);
  } else {
    static bool poBliknuti = false;
    if (blikniDo && !poBliknuti) { digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); poBliknuti = true; blikniDo = 0; }
    if (!blikniDo) poBliknuti = false;
    digitalWrite(LED_BLUE, modra ? LOW : HIGH);
  }
  if (!usbNapajeni() && batPct >= 0 && batPct < 15) digitalWrite(LED_RED, (now % 3000) < 150 ? LOW : HIGH);
  else if (cfg2.zaznam && nZaz >= slotu) digitalWrite(LED_RED, (now % 1000) < 100 ? LOW : HIGH);   // paměť plná
  if (cfg2.zaznam && cfg2.bezi && (int32_t)(now - zelenaDo) > 0 && now - posledniVystrel > 80 && (int32_t)(blikniDo - now) <= 0)
    digitalWrite(LED_GREEN, (now % 2000) < 40 ? LOW : HIGH);                                        // záznam běží: krátké zelené bliknutí každé 2 s
  // barva baterky: zelená nad 70 %, žlutá (červená+zelená) 30–70 %, červená pod 30 %
  // ukáže se 3 s po zapnutí, po příkazu „baterka“ a s kabelem USB (nabíjení) každé 3 s na 0,3 s
  bool ukazBat = batPct >= 0 && (int32_t)(blikniDo - now) <= 0 && ((int32_t)(barvaDo - now) > 0 || (usbNapajeni() && (now % 3000) < 300));
  if (ukazBat) { digitalWrite(LED_RED, batPct < 70 ? LOW : HIGH); digitalWrite(LED_GREEN, batPct >= 30 ? LOW : HIGH); barvaSviti = true; }
  else if (barvaSviti) { digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); barvaSviti = false; }
  if (now - posledniBat > 10000 || posledniBat == 0) { posledniBat = now; batMer();
    Serial.println(batText());
    bool usb = usbNapajeni();
    if (!usb && batV > 2.5f && batV < 3.30f) { if (++nizkeBat >= 6) vypnoutUplne("baterka"); } else nizkeBat = 0; }
  if (raz > 0.3f || usbNapajeni()) posledniPohyb = now;
  if (usinMin && now - posledniPohyb > (uint32_t)usinMin * 60000UL && !nahrava) vypnoutUplne("necinnost");
  if (Bluefruit.connected() && now - posledniTep > 5000 && now - posledniVystrel > 500) { posledniTep = now; tep(); }

  if (stream && now - posledniStream >= 40) {   // 25× za sekundu pro stránku nastavení
    posledniStream = now;
    Serial.print("D "); Serial.print(accMaxS, 2); Serial.print(" "); Serial.println(micMaxS);
    accMaxS = 0; micMaxS = 0;
  }
  if (!stream && now - posledniVypis > 500) {
    posledniVypis = now;
    Serial.print(plneJmeno); Serial.print("  BLE spojeni "); Serial.print(Bluefruit.connected()); Serial.print("/3");
    for (uint16_t h = 0; h < BLE_MAX_CONNECTION; h++) if (Bluefruit.connected(h)) { char jm[24] = {0}; BLEConnection* c = Bluefruit.Connection(h); (void)c;   // jméno protistrany NEČÍST – je to dotaz do telefonu uprostřed jeho připojování a telefon pak spojení ukončí
      Serial.print(" ["); Serial.print(jm[0] ? jm : "?"); Serial.print(bleuart.notifyEnabled(h) ? " aplikace]" : " klavesnice]"); }
    Serial.print("  imu="); Serial.print(imuStav == 0 ? "OK" : (imuStav == -50 ? "BEZ-NAPAJENI" : "CHYBA"));
    Serial.print("("); Serial.print(imuKde); Serial.print(") who W1:"); Serial.print(who[0],HEX); Serial.print("/"); Serial.print(who[1],HEX);
    Serial.print(" W0:"); Serial.print(who[2],HEX); Serial.print("/"); Serial.print(who[3],HEX);
    Serial.print("  x/y/z: "); Serial.print(x,2); Serial.print(" "); Serial.print(y,2); Serial.print(" "); Serial.print(z,2);
    Serial.print("   raz max: "); Serial.print(accMax05, 2);
    Serial.print(" g   mikrofon max: "); Serial.print(micMax05);
    Serial.print("   bat "); Serial.print(batV, 2); Serial.print(" V "); Serial.print(batPct); Serial.print(" %");
    Serial.print("   zaznam "); Serial.print(cfg2.zaznam); Serial.print(" "); Serial.print(nZaz); Serial.print("/"); Serial.print(slotu); Serial.print(pametTyp);
    Serial.print("   | "); Serial.println(cfgText());
    accMax05 = 0; micMax05 = 0;
  }
}
