# JeeNode MQTT Google Cloud

U ovom dokumentu opisani su potrebni zahtjevi i koraci za spajanje mreže JeeNode čvorova na Google Cloud IoT servis. Mreža JeeNode-ova prikuplja svjetlosne i kompas podatke te ih preko Raspberry Pi mikrokontrolera šalje na Google Cloud.

#### Hardverski i softverski zahtjevi
- Hardver
  - 3x JeeNode - ATmega 328P
  - Raspberry Pi 3 model B
  - Senzor svjetlosti
  - Kompas (senzor)
  - Ether Card - ENC28J60
  - LED pločica
- Softver

  - Google račun
  - Python knjižnice
    - Paho-MQTT
    - PyJWT

  - Arduino knjižnice
    - [EtherCard](https://github.com/njh/EtherCard)
    - [JeeLib](https://github.com/jeelabs/jeelib)

 - Arduino IDE (za dodavanje knjižnica i programiranje JeeNode-ova)

#### Upute za izradu projekta

**1. Spajanje JeeNode-ova**
 
 - JeeNode1 
   - spajanje LED pločice na portove 2 i 3
   - spajanje Ether Card-a na portove 1, 4, SPI
   - [jeeMaster](https://github.com/davidmak2709/JeeNode-MQTT-Google-Cloud/tree/master/jeeMaster)
  
 - JeeNode2 
   - spajanje LED pločice na portove 2 i 3
   - spajanje kompas senzora na port 1
   - [compass](https://github.com/davidmak2709/JeeNode-MQTT-Google-Cloud/tree/master/compass)
    
 - JeeNode3 
   - spajanje LED pločice na portove 2 i 3
   - spajanje senzora svjetlosti na port 1 (pin 3-GND, 5-AOI)
   - [light sensor](https://github.com/davidmak2709/JeeNode-MQTT-Google-Cloud/tree/master/light)


**2. Povezivanje Ether Card-a i Raspberry PI uređaja ethernet kabelom**

**3. Programiranje JeeNode-ova koristeći odgovarajuće kodove iz repozitorija**

**4. Konfiguriranje Raspberry Pi-a**

- Generiranje privatnog i javnog ključa naredbama:

```bash
openssl genpkey -algorithm RSA -out rsa_private.pem -pkeyopt rsa_keygen_bits:2048
openssl rsa -in rsa_private.pem -pubout -out rsa_public.pem
```

- Instalacija potrebnih modula:

```
sudo apt-get install build-essential libssl-dev libffi-dev python3-dev
pip3 install paho-mqtt
pip3 install pyjwt
```

- U crontab file (`crontab -e`) dodati naredbu

```
@reboot python3 path-to-mqtt_pub.py
```

- Skriptu mqtt_pub.py staviti u direktorij po želji s generiranim ključevima i na početku izmjeniti globalne varijable kao što su put do ključeva, ime Google Cloud projekta, mqtt topic-a, ID registra i ID uređaja:

```python
DIR = '<your-path>'

ca_certs = DIR + 'roots.pem'
public_crt = DIR + 'rsa_public.pem'
private_key_file = DIR + 'rsa_private.pem'

mqtt_url = "mqtt.googleapis.com"
mqtt_port = 8883
mqtt_topic = "<your-mqtt-topic>"
project_id = "<your-project-id>"
cloud_region = "<your-cloud-region>"
registry_id = "<your-registry-id>"
device_id = "<your-device-id>"
```

Većina ovih varijabli se definira pri konfiguraciji Google Cloud-a.

**5. Konfiguriranje Google Cloud-a**

Za korištenje Google Cloud-a potreban je Google račun. Prvo Google Cloud-u moramo omogućiti pristup Google računu. Nakon što omogućimo pristup dobijemo 300$ kredita koji vrijede jednu godinu. S tim kreditom možemo koristiti bilo koji Google Cloud projekt ali nas trenutno zanima IoT Core projekt.

Kada otvorimo IoT Core projekt prvo moramo definirati registar u kojem će se nalaziti naše teme i uređaji. Nakon što otvorimo registar definiramo novi topic i kreiramo novi uređaj. Prilikom kreiranja uređaja dodajemo mu javni ključ koji smo prethodno generirali na Raspberry Pi-u.

Kako bi podatke koje šaljemo s JeeNode-ova mogli prikazati na Google Cloud-u moramo ih usmjeriti na BigQuery projekt preko DataFlow projekta. Prvo moramo kreirati dataset te u dataset-u napraviti tablicu u koju ćemo spremati podatke s senzora koje nam šalju JeeNode-ovi. U našem slučaju tablica ima stupce time(timestamp), data_type(string), data_value(integer). Zatim moramo kreirati DataFlow posao. Najbrži način za kreiranje DataFlow posla je kreirati ga iz predloška. Koristimo predložak Cloud Pub/Sub Topic to BigQuery. U predlošku moramo upisati koje je ime teme i ime BigQuery output tablice koju smo prethodno kreirali. Također moramo napisati ime direktorija u koje će se spremati privremeni podaci. Kako bi kreirali taj direktorij moramo koristiti Storage projekt. U Storage projektu kreiramo bucket u kojem ćemo napraviti tmp direktorij za privremene podatke. Nakon što smo definirali direktorij za privremene podatke možemo pokrenuti DataFlow posao. On će automatski sve podatke koji stižu na MQTT temu zapisivati u BigQuery tablicu.

