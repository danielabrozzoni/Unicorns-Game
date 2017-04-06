#include "vsgl2.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <stdio.h>
#include <time.h>
#include <iostream>
//Only to allow auto completion
//to be ignored
using namespace vsgl2;
using namespace vsgl2::general;
using namespace vsgl2::video;
using namespace vsgl2::utils;
using namespace vsgl2::io;
using namespace vsgl2::audio;
using namespace std;

const int numeroblocchi = 10000;
const int finestrax = 600;
const int finestray = 700;
const int A = 10;
const int e = 30;
const int finegioco = 100000;
const int l_immagine = 548;                 // servivano quando usavo initSfondo
const int a_immagine = 27254;               // e disegnavo lo sfondo a mano, ora sono inutili
const int personaggio = 100;
const int grandezza = 125;
const double decelerazione = 0.981;
const double VyIniziale = 19;
const double blocchiCircaSullaStessaLinea = 2;
const int fasciasopra = 200;
const int fasciasotto = 400;

int velocitaI = 2;
int velocitaP = 4;
int direzione = 1;
int distacco = 100;
int matrice[a_immagine][l_immagine][3];     // servivano quando usavo initSfondo
int x_personaggio = finestrax/2;
int y_personaggio = finestray - personaggio;
double Vy = VyIniziale;
int vittoria = 0;                       // 0 se il gioco deve andare avanti, 1 se hai vinto, -1 se hai perso
int cont = 0;
int punteggio = 0;
bool is_prime[numeroblocchi];


/*
    \todo: IL LIVELLO 4 È CATTIVO
    \todo: menù iniziale
    \todo: impostazioni mouse/tastiera
    \todo: mostri
    \todo: sparare
    \todo: cono gelato pe più punti, molla
    \todo: piattaforme che si rompono
    \todo: visualizzare record
*/

/*
    OGGETTI POSSIBILI:
    0: cono gelato -> bonus 200 punti
*/

struct Oggetto
{
    int X = 0;
    int Y = 0;
    int altezza = 0;
    int larghezza = 0;
    int toccato = 0;
    int cheOggettiCiSono[100];
};

Oggetto blocco[numeroblocchi];

void questaCosaPotrebbeServirmi()
{
    int flag = 0;
    while(1)
    {
        if(is_pressed(VSGL_U) && flag == 0)
            flag = 1; //fai qualcosa di utile
        if(!is_pressed(VSGL_U) && flag == 1)
            flag = 0; //fai qualcosa di utile
    }

}


void inizializzaTutto()
{
    x_personaggio = finestrax/2;
    y_personaggio = finestray - personaggio;
    Vy = VyIniziale;
    vittoria = 0;
    cont = 0;

}


//Le prossime 40 righe sono parti di programma che falliscono miseramente, ma mi dispiaceva toglierle, quindi eccole qui
/*

    Questa funzione servirebbe ad inizializzare matrice[][][] per disegnare lo sfondo.
    Prende i dati da un file chiamato immagine2.txt, che contiene l'rgb di ogni pixel della foto ea9.jpg
    Il file è stato scritto da uno script Python presente nella cartella di nome immagine.py

*/


void init_Sfondo()
{
    ifstream in;
    in.open("immagine2.txt");
    for(int i = 0; i < a_immagine; i++)
        for(int j = 0; j < l_immagine; j++)
        {
            int a, b, c;
            in >> a >> b >> c;
            matrice[i][j][0] = a;
            matrice[i][j][1] = b;
            matrice[i][j][2] = c;
        }
}

/*

    Questa funzione servirebbe per disegnare lo sfondo pixel per pixel (in modo da creare le varie sfumature man mano che si avanza)
    prendendole dal vettore tridimensionale inizializzato con initSfondo(), purtroppo però per qualche strano motivo non funziona e comunque
    sarebbe qualcosa di troppo pesante da fare.
    La funzione però la tengo, magari in futuro riesco a migliorarla

*/

void disegna(int alto)
{
    int basso = alto + finestray;
    for(int i = alto; i < basso; i++)
        for(int j = 0; j < l_immagine; j++)
            draw_point(i-basso,j,Color(matrice[i][j][0], matrice[i][j][1], matrice[i][j][2], 255));
}

/*

    Disegna solo un'immagine carina a schermo, non fa nulla di che in effetti

*/

void disegnaSfondoVero(char stringa[])
{
    draw_image(stringa, 0, 0,  finestrax, finestray, 255);
}

/*

    Funzione che mi inizializza l'array is_prime, utilizzato per creare apparente casualità

*/

void eratostene()
{
    fill(is_prime, is_prime + numeroblocchi, 1);
    for(int i = 2; i <= numeroblocchi; i++)
    {
        if(is_prime[i] == 1)
        {
            for(int j = i*2; j < numeroblocchi; j+=i)
                is_prime[j] = 0;
        }
        // cout << i << " " << is_prime[i] << endl;
    }
}


/*

    A parte il primo blocco, lungo quanto la finestra e posto in basso, gli altri sono posti "casualmente" nello schermo

*/

void creaCoseCasuali()
{
    blocco[0].X = 0;
    blocco[0].Y = finestray-A ;
    blocco[0].altezza = A;
    blocco[0].larghezza = finestrax;

    for(int i = 1; i < numeroblocchi; i+=2)
    {
        blocco[i].larghezza = grandezza + rand()%10 - 5;
        blocco[i].altezza = A;
        blocco[i].Y = distacco*i - finegioco + rand()%100-50;
        blocco[i].X = rand()%finestrax - blocco[i].larghezza;

        if(is_prime[i])
            continue;
        blocco[i+1].larghezza = grandezza + rand()%10 - 5;
        blocco[i+1].altezza = A;
        blocco[i+1].Y = distacco*i - finegioco - rand()%100 - 50;
        blocco[i+1].X = rand()%finestrax - blocco[i].larghezza;

        while(blocco[i].X <= 0 || blocco[i].X >= finestrax - blocco[i].larghezza || abs(blocco[i].X - blocco[i+1].X) < personaggio*2 ||
                blocco[i+1].X < 0 || blocco[i+1].X > finestrax - blocco[i+1].larghezza)
        {
            blocco[i].X = rand()%finestrax - blocco[i].larghezza;
            blocco[i+1].X = rand()%finestrax - blocco[i+1].larghezza;
        }

        while(abs(blocco[i].Y - blocco[i+1].Y) > distacco + e*2 )//|| (i <= 1 && abs(blocco[i].Y - blocco[i-1].Y) > distacco + e*2))
            blocco[i].Y = distacco*i - finegioco + rand()%100-50, blocco[i+1].Y = distacco*i - finegioco + rand()%100-50;


        /*
                for(int j = 0; j < rand()%5 && i < numeroblocchi; j++)
                {
                    i++;
                    blocco[i].larghezza = grandezza + rand()%10 - 5;
                    blocco[i].altezza = A;
                    blocco[i].Y = distacco*i - finegioco - rand()%100 - 50;
                    blocco[i].X = rand()%finestrax - blocco[j].larghezza;
                    while(blocco[i].X < 0 || blocco[i].X > finestrax - blocco[i].larghezza)
                    {
                        blocco[i].X = rand()%finestrax - blocco[i].larghezza;
                    }
                }*/
    }
}


/*

    Disegna le nuvolette
    \todo: assicurarmi che disegni nuvolette e non rettangolini

*/

void disegnaNuvolette()
{
    for(int i = 0; i < numeroblocchi; i++)
        draw_filled_rect(blocco[i].X,blocco[i].Y, blocco[i].larghezza,blocco[i].altezza, Color(255,255,255,255));
    //if(blocco[i].Y < finestray && blocco[i].Y > 0)
    //draw_image("nuvola.png",blocco[i].X,blocco[i].Y, blocco[i].larghezza, blocco[i].altezza);
}


/*
    Sposta il personaggio utilizzando i tasti -> e <- sulla tastiera
*/

void coseCoiTasti()
{
    if(is_pressed(VSGL_LEFT))
        x_personaggio--, direzione = -1;

    if(is_pressed(VSGL_RIGHT))
        x_personaggio++, direzione = 1;
}

/*

    Il personaggio si sposta seguendo il puntatore del mouse

*/

void coseColMouse(int punteggio)
{
    int b = x_personaggio;
    int a = get_mouse_x();
    if(punteggio <= 50)
        a = finestrax/2;
    x_personaggio -= (-a+finestrax/2)/100;
    if(x_personaggio - b < 0)
        direzione = -1;
    else
        direzione = 1;
}

/*

    Crea un file di log chiamato file_numero, stampa i valori di un certo vettore
    Solo per debugging

*/


void stampaSulFileDiLog(int N, char numero_del_file)
{
    string a = "file__";
    a += numero_del_file;
    ofstream out;
    out.open(a);
    for(int i = 0; i < N; i++)
        out <<  blocco[i].X << " " << blocco[i].Y << endl;
}


/*
    Fa' in modo che le nuvolette scendano
*/

void spostaSchermo()
{
    for(int i = 0; i < numeroblocchi; i++)
        blocco[i].Y++;
}

/*
    Questa è la funzione che si occupa del gioco vero e proprio
    Riga 198: sposto lo schermo in alto (ogni tanto, altrimenti va troppo veloce, uso l'operatore %)
    Riga 201: decido come comandare il personaggio
    Riga 208: disegno le piattaforme
    Riga 210: disegno il personaggio
    Riga 213: mi assicuro che il personaggio saltelli
    Riga 220: controllo se sono sopra una qualsiasi piattaforma (l'ultima condizione serve ad assicurarmi di essere
    in discesa quando mi fermo sulla piattaforma, per evitare di continuare a rimbalzare per inerzia mentre sto salendo,
    le altre condizioni mi servono per controllare se sono effettivamente sulla piattaforma)
*/


void gioco(char stringa[], int livello, char scelta[])
{
    disegnaSfondoVero(stringa);
    if(cont%velocitaI == 0 && cont > 50)
        spostaSchermo();

    if(strcmp("tasti", scelta) == 0)
        coseCoiTasti();

    else if(strcmp("mouse", scelta) == 0)
        coseColMouse(cont);

    disegnaNuvolette();
    if(direzione == 1)
        draw_image("unicorno2.png", x_personaggio, y_personaggio,personaggio,personaggio,255);
    if(direzione == -1)
        draw_image("unicorno1.png", x_personaggio, y_personaggio,personaggio,personaggio,255);


    if(cont % velocitaP == 0)
    {
        y_personaggio -= Vy;
        Vy -= decelerazione;
    }

    for(int i = 0; i < numeroblocchi; i++)
        if(abs(blocco[i].Y - y_personaggio - personaggio + A) < e && x_personaggio + personaggio  >= blocco[i].X && x_personaggio < blocco[i].X + blocco[i].larghezza && blocco[i].Y < finestray && blocco[i].Y > 0 && Vy <= 0)
        {
            int temp = y_personaggio;
            Vy = VyIniziale;
            if(blocco[i].toccato == 0) //Aumento di punti solo se è la prima volta che tocco un blocco, altrimenti faccio punti restando fermo
                punteggio += 10, blocco[i].toccato = 1;
            /*
                        if((y_personaggio < fasciasotto && y_personaggio > fasciasopra))
                        {
                            for(int j = 0; j < numeroblocchi; j++)
                            {
                                for(int k = 0; k < 35; k++)
                                    blocco[j].Y++;
                            }
                        }
            */
            /*
            //if(livello == 4)                                //Succedono cose strane
            {
                for(int j = 1; j < numeroblocchi; j++)
                    blocco[j].Y += temp/2;

            }
            */


        }

    if(x_personaggio > finestrax)
        x_personaggio = 0;

    if(x_personaggio < 0)
        x_personaggio = finestrax - personaggio;

    if(punteggio == finegioco)
        vittoria = 1;
    if(y_personaggio > finestray)
        vittoria = -1;

    cont++;
}

/*

    Yeee :)

*/

void vinto()
{
    draw_filled_rect(0,0,finestrax,finestray,Color(255,255,255,255));
    draw_image("fine.jpg",0,0,626, 626, 255);
    draw_image("Win.png",15,550,571,130,255);
}

/*

    Apre il file dove ci sono scritti tutti i punteggi e lo aggiorna

*/

void score()
{
    char nome[100];
    cout << "Bravo! Inserisci il tuo nome" << endl;
    cin >> nome;
    FILE *out;
    out = fopen("Vincitori.txt", "a");
    fprintf(out, "%s %d\n", nome, punteggio);
    fclose(out);
    delay(1000);
    //cout << "Vuoi fare un'altra partita? \n \t Premi 1 per giocare ancora \n \t Premi 0 per uscire \n";
    exit(0);
}


/*

    Noo :(

*/

void perso()
{
    draw_filled_rect(0,0,finestrax,finestray,Color(21,27,49,255));              //A quanto pare queste istruzioni non vengono
    draw_image("sad.png",(finestrax-500)/2,(finestray-600)/2,500,600,255);      // eseguite, eh vabby, era un'immagine carina :(
    cout << "Il tuo punteggio è: " << punteggio << endl;
    score();
}

/*
    Disegna il menu e ritorna un valore a seconda di cosa è successo
    Case 1: inizio del gioco
    Case 2: istruzioni
*/

int menu()
{
    draw_image("Untitled.png",0,0,finestrax,finestray,255);
    int a = get_mouse_x();
    int b = get_mouse_y();
    const int margine = 60;
    const int altezzaBottone = 40;
    const int larghezzaBottone = 150;
    const int distacco = 30;
    const int numerobottoni = 5;
    const int inizioBottoni = (finestray - numerobottoni*altezzaBottone - (numerobottoni-1) * distacco) / 2;

    for(int i = 0; i < numerobottoni; i++)
        draw_filled_rect(margine,inizioBottoni + (altezzaBottone + distacco)*i,larghezzaBottone,altezzaBottone,Color(255,255,255,255));

    for(int i = 1; i <= numerobottoni; i++)
    {
        if(mouse_left_button_pressed() && a >= margine && a <= larghezzaBottone + margine && b >= inizioBottoni + (i-1)*(distacco+altezzaBottone) && b <= inizioBottoni + i*altezzaBottone + (i-1)* distacco)
            return i;

        if(!mouse_left_button_pressed() && a >= margine && a <= larghezzaBottone + margine && b >= inizioBottoni + (i-1)*(distacco+altezzaBottone) && b <= inizioBottoni + i*altezzaBottone + (i-1)* distacco)
            draw_filled_rect(margine,inizioBottoni + (altezzaBottone + distacco)*(i-1),larghezzaBottone,altezzaBottone,Color(253,225,127,255));
    }

    return 0;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    eratostene();

    char stringa[1000] = "sfondoN.jpg";
    int livello = 1;
    char scelta[100] = "tasti";
    int cosafare = 0;
    init();
    set_window(finestrax,finestray,"Prima o poi trovo un nome lol");
    //stampaSulFileDiLog(numeroblocchi,'0');

    while(!done())
    {
        cosafare = menu();
        if(cosafare != 0)
            break;
        update();
    }

    switch(cosafare)                                                       //GIOCO
    {
        case 1:
        {
            creaCoseCasuali();
            inizializzaTutto();
            //int tempo = ms_time();
            //cout << tempo;
            /*while(1)
            {
                disegnaSfondoVero(stringa);
                if(tempo - ms_time() >= 3000)
                    draw_filled_rect(100, 100, 100, 100, Color(25,231,129,255)), update();
                if(tempo - ms_time() >= 6000)
                    draw_filled_rect(100, 100, 100, 100, Color(98,237,225,255));
                if(tempo - ms_time() >= 9000)
                    draw_filled_rect(100, 100, 100, 100, Color(123,89,76,255));
                update();
              // if(tempo - ms_time() > 10000)
                //    break;
            }*/

            while(!done())
            {
                if(vittoria == 0)
                    gioco(stringa, livello, scelta);
                if(vittoria == 1)
                    vinto();
                if(vittoria == -1)
                    perso();
                update();
            }
        }
    }
    close();
    return 0;
}

