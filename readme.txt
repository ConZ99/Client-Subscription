Lica Stefan 325CB

I.Subscriber.c
==============

Initial citesc argumentele de pornire a clientului si stabilesc o conexiune TCP cu serverul.

Clientul poate primi mesaje de la server sau poate primi comenzi de la tastatura. In conluczie exista doua cazuri in care clientul primeste mesaje.

Daca se citeste de la tastatura o comanda ca string este trimisa inspre server. 
	Daca comanda este "exit" atunci se iese din loop si se termina executia.
	Daca comanda este (un)subscribe... atunci este trimis mesaj spre server, dupa ce a fost trimis mesajul se afiseaza pe ecran mesajul de atentionare.

Daca clientul TCP prmieste un mesaj redirectionat de catre server de la un client UDP este evaluat si se afiseaza mesaje conform cerintei.

II.Server.c
===========

Deschid portul pe care se vor conecta clientii TCP.
Deschid poretul pe care il vor folosi clientii UDP.

Daca se conecteaza un client TCP ii accept conexiunea si incep popularea listei de clienti TCP.

Daca un client UDP trimite informatie incep evaluarea mesajului.
Trimit la toti clientii TCP care au subscribe la topicul primit: IP, PORT si MESAJ.	
	IP - ipul pe care trimite mesajul
	PORT - portul clientului UDP
	MESAJ - informatia primita de la clientul UDP

In al treilea caz primesc de la clientul TCP un mesaj.
Daca mesajul este "exit" atunci inchid legatura. In cazul in care mesajul primit nu este "exit" singurele mesaje acceptate sunt (un)subscribe.
Daca mesajul este "subscribe topic sf" atunci adaug topicul si la aceasi pozitie setez si sf-ul in lista clientilor.
Daca mesajul este "unsubscribe topic" caut in lista clientilor topicul cautat si il elimnin, asta daca sf=0 (daca sf=1 nu sterg topicul, dar nu mai primeste notificari)
