# Komandos duomenų gavimui

|         Komanda          | Aprašymas                                              | Rezultatas                                                                                                                    |
| :----------------------: | :----------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------- |
|          AT+GSN          | IMEI;                                                  | 15 dešimtainių skaitmenų kodas                                                                                                |
|           ATI            | Modemo informaciją (modelį, gamintoją);                | Tris eilutės: Gamintojas, modelis, revizija                                                                                   |
|  AT+COPS? arba AT+QSPN   | Operatorių, prie kurio šiuo metu yra prisijungta;      |                                                                                                                               |
|         AT+CREG?         | Prisijungimo prie tinklo statusą;                      | +CREG: <n>,<stat>[,<lac>,<ci>[,<Act>]] kai n!=0 - prijungta prie tinklo,                                                      |
|                          |                                                        | stat: 0, 2 ir 3 - ne priregistruota, 1 ir 5 - priregistruota,                                                                 |
|                          |                                                        | naminis tinklas, 4 - nežinoma                                                                                                 |
|      AT+QCFG="band"      | Mobilaus ryšio dažnių juostą (angl. band);             | +QCFG: "band",<bandval>,<ltebandval>,<tdsbandval> bandval ir ltebandval šešioliktainiai skaičiai nurodantys dažnių vėliavėles |
| AT+QSIMSTAT? AT+QSIMDET? | SIM kortelės statusą (ar įdėta, ar aktyvi);            |                                                                                                                               |
|  AT+QENG="servingcell"   | Aptarnaujančios celės (serving cell) informaciją;      | Skirtingi formatai LTE ir WCDMA režimui                                                                                       |
| AT+QENG="neighbourcell"  | Gretimas celes (neighbor cells);                       | Skirtingi formatai LTE ir WCDMA režimui                                                                                       |
|          AT+CSQ          | Signalo stiprumą;                                      | +CSQ: <rssi>,<ber> rssi: 0-31 rysio stiprumas, 99 - nezinomas stiprumas                                                       |
|            ?             | IP adresą (jei yra aktyvi duomenų sesija);             |                                                                                                                               |
|            ?             | Modemo temperatūrą (jei palaikoma);                    |                                                                                                                               |
|            ?             | Naudojamą APN;                                         |                                                                                                                               |
|         AT+CNUM          | Telefono numerį (jei prieinamas);                      |                                                                                                                               |
|         AT+CMGL          | SMS žinutes (sudėtingesnė dalis – multi-line parsing). |                                                                                                                               |

# Komandos išvedimo nustatymui

|  Komanda  | Aprašymas                                           |
| :-------: | :-------------------------------------------------- |
|   ATQ0    | Grazinti rezultato koda                             |
|   ATV1    | Grazina ok/error, o ne klaidos koda                 |
|   ATE1    | Grazinti gautas komandas                            |
| AT+CMEE=2 | Grazinti +CMS ERROR ir +CME ERROR us klaidos tekstu |
|   AT&V    | Dabartiniai nustatymai                              |
| AT+CMEE?  | Dabartinis klaidų formatas                          |

# Programos vykdymas

nuskaityti cli parametrus ->
atidaryti sasajos deskriptorių ir nustatyti jo parametrus (koks timeout, koks boud speed ir t.t.) ->
išsaugoti dabartinius modemo nustatymus ->
pakeisti modemo nustatymus programos veikimui (pvz.: AT+CMEE=2, ATV1 ir pan.) ->
kol yra užklausti duomenys:

        write() AT komandą
        read() iki OK arba ERROR, +CME ERROR: <tekstas>, +CMS ERROR: <tekstas>
        nuskaitomas komandos rezultatas
        pagal parametruose nurodyta formata isvedamas rezultatas i konsole

atstatyti modemo parametrus kaip buvo pries programos paleidima ->
atlaisvinti atminti, uždaryti deskriptorių ir t.t.
