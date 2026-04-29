|         Commands         | Description                                            |
| :----------------------: | ------------------------------------------------------ |
|          AT+GSN          | IMEI;                                                  |
|     AT+CGMM AT+CGMI      | Modemo informaciją (modelį, gamintoją);                |
|         AT+QSPN          | Operatorių, prie kurio šiuo metu yra prisijungta;      |
|         AT+CREG?         | Prisijungimo prie tinklo statusą;                      |
|      AT+QCFG="band"      | Mobilaus ryšio dažnių juostą (angl. band);             |
| AT+QSIMSTAT? AT+QSIMDET? | SIM kortelės statusą (ar įdėta, ar aktyvi);            |
|  AT+QENG="servingcell"   | Aptarnaujančios celės (serving cell) informaciją;      |
| AT+QENG="neighbourcell"  | Gretimas celes (neighbor cells);                       |
|          AT+CSQ          | Signalo stiprumą;                                      |
|                          | IP adresą (jei yra aktyvi duomenų sesija);             |
|                          | Modemo temperatūrą (jei palaikoma);                    |
|                          | Naudojamą APN;                                         |
|         AT+CNUM          | Telefono numerį (jei prieinamas);                      |
|         AT+CMGL          | SMS žinutes (sudėtingesnė dalis – multi-line parsing). |
