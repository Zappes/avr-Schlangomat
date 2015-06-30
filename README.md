Der Schlangomat
===============


Was ist das?
------------
Der Schlangomat ist eine Kiste mit etwas Elektronik, ein paar Relais und zwei kombinierten Feuchtigkeits- und Temperatursensoren.
In der Kiste sind Steckdosen verbaut, die über Relais anhand gewisser Regeln an- und ausgeschaltet werden. Zusätzlich kann der
aktuelle Zustand der Dosen und Sensoren per WLAN abgefragt werden und die verschiedenen Betriebsparameter können mit einem
seriellen Terminal per USB programmiert werden.

Gedacht ist das ganze
als "Proof of Concept" für die Klimatisierung von Terrarien mit Schlangen drin - natürlich nur als Test und auf gar keinen fickenden
Fall für den tatsächlichen Einsatz. Ich bin schließlich kein Elektriker und baue deswegen keine Geräte, die mit Netzspannung
betrieben werden.


Was steckt so drin?
-------------------
Der Mikrocontroller ist ein ATmega32U4 von Atmel, der den USB-Controller schon drin hat. Für die WLAN-Anbindung wird ein ESP8266-03-Modul
benutzt, das mit der ganz normalen AT-Firmware läuft. Die Relais sind 36.11-Relais von Finder, die von einem ULN2003A angesteuert werden.


Output per http
---------------
Wenn man per http auf die IP des Geräts zugreift, kriegt man den folgenden Output zurück:

```
{"sock":[0,0,0,0],"sens":[{"h":45.9,"t":25.9,"e":0,"c":0},{"h":46.7,"t":26.2,"e":0,"c":0}],"rule":["T1 15 18","H1 32 38","T2 21 26","H2 34 50"]}
```

- sock: Der Zustand der Steckdosen von Dose 1 bis Dose 4. Im Beispiel ist nur die 3 an.
- sens: Die aktuellen Messwerte. h steht für die relative Luftfeuchtigkeit in Prozent, t für die Temperatur in °C, e für einen Errocode und c dafür, wie fot dieser Fehler jetzt in Folge aufgetreten ist.
- rule: Die aktuellen Regeln von Dose 1 bis Dose 4. Mehr dazu weiter unten.

Das Mistding schickt wirklich nur diesen Output, keine HTTP-Header. Hintergrund ist schlicht und ergreifend ein Bug in der
Firmware des WLAN-Moduls, wegen dem es nicht möglich ist, CRLF-Sequenzen zu schicken. Wer eine Lösung dafür findet, möge bitte
einen Pull-Request schicken ...

Intern werden die Sensoren nur etwa alle 5 Sekunden abgefragt. Wenn bei der Abfrage ein Fehler auftritt, steht dieser im Errorcode des Ergebnisses,
als Temperatur und Feuchtigkeit werden dann jeweils 0.0 gemeldet. Wenn der Fehler bei mehreren Abfragen in Forlge auftritt, steht in c die Anzahl
dieser Fehlversuche oder 255, wenn es schon mehr als 254 mal passiert ist. Was die genauen Fehlercodes bedeuten, kann man sich in lib/am2302.c
raussuchen, bei Code 4 kann man aber meistens davon ausgehen, dass der Sensor schlicht nicht eingesteckt ist. Oder total im Arsch.


USB-Kommandos
-------------

| Befehl     | Parameter | Beschreibung                          |
|------------|-----------|---------------------------------------|
| espsetup   |           | Initialisiert den WLAN-Chip.          |
| espsetup   |           | Initialisiert den WLAN-Chip.          |


Die Regelsyntax
---------------

Gibt es Videos?
-----------------------
[Natürlich gibt es eine Youtube-Playlist.](https://www.youtube.com/playlist?list=PLFFlJlvZ--PkRfQQ3qQJbDEAugWxiXpbW)

