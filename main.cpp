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

const int epsilonV = 1;
const int numeroblocchi = 10000;
const int finestrax = 600;
const int finestray = 700;
const int A = 10;
const int e = 30;
const int finegioco = 100000;
const int personaggio = 100;
const int grandezza = 125;
const double decelerazione = 0.981;
const double VyIniziale = 19;
const int fasciasopra = 200;
const int fasciasotto = 400;
const int quantiOggetti = 2;
const int frequenzaOggetti = 2;

int tempo;
int velocitaI = 2;
int velocitaP = 4;
int direzione = 1;
int distacco = 140;
int x_personaggio = finestrax/2;
int y_personaggio = finestray - personaggio;
double Vy = VyIniziale;
int vittoria = 0;                       // 0 se il gioco deve andare avanti, 1 se hai vinto, -1 se hai perso
int cont = 0;
int punteggio = 0;
bool is_prime[numeroblocchi];


/**
    \todo menù iniziale
    \todo impostazioni mouse/tastiera
    \todo mostri
    \todo sparare
    \todo cono gelato per più punti, molla
    \todo piattaforme che si rompono
    \todo visualizzare record
*/

/**
    OGGETTI POSSIBILI:
    0: cono gelato -> bonus 200 punti
*/

struct Example
{
    string file;
    int altezza;
    int larghezza;
};

struct Oggetto
{
    int X = 0;
    int Y = 0;
    int tipo;
    int flag = 0;
};

struct Blocco
{
    int X = 0;
    int Y = 0;
    int altezza = 0;
    int larghezza = 0;
    int toccato = 0;
    Oggetto coso;
};

Blocco blocco[numeroblocchi];
Example cosini[10];

/**

    \brief As the name says, this function isn't used in the real program.
    Teacher was speaking and I thought that this would be useful

*/


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

/**

    Initializes all the variables that I'll use, it will be helpful in the future if I write something to play
    two or more consecutives games
    \todo  write something to play two or more consecutives games

*/

void inizializzaTutto()
{
    x_personaggio = finestrax/2;
    y_personaggio = finestray - personaggio;
    Vy = VyIniziale;
    vittoria = 0;
    cont = 0;

}


/**

    \param background's name
    \brief It draws the background. Only one line of code, it seems like this function is useless, but you can decide in the main
    what is the background name and pass it to this. (Ok, I've realized that this function is useless, but I've become attached to
    it, so)

*/

void disegnaSfondoVero(char stringa[])
{
    draw_image(stringa, 0, 0,  finestrax, finestray, 255);
}

/**

    Do you know who Eratostene is?
    Oke, I know, prime numbers are not the best way to create randomness
    However, I think that they have interesting charateristic: there are only 2 prime numbers that are consecutives,
    big prime number are more distant than smaller
    (Maybe I just want to insert mathematics everywhere, idk)

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


/**

   \brief First block is as long as the window, other are in the screen, random

*/

void creaCoseCasuali()
{

    blocco[0].X = 0;
    blocco[0].Y = finestray-A ;
    blocco[0].altezza = A;
    blocco[0].larghezza = finestrax;

    for(int i = 1; i < numeroblocchi; i++)
    {
        blocco[i].larghezza = grandezza + rand()%10 - 5;
        blocco[i].altezza = A;
        blocco[i].Y = distacco*i - finegioco + rand()%100-50;
        blocco[i].X = rand()%finestrax - blocco[i].larghezza;

        while((i != 1 && blocco[i].X <= 0 || blocco[i].X >= finestrax - blocco[i].larghezza || abs(blocco[i].X - blocco[i-1].X) < personaggio*2 ||
                blocco[i-1].X < 0) || (i > 2 && abs(blocco[i].X - blocco[i-2].X) < 20))
        {
            blocco[i].X = rand()%finestrax - blocco[i].larghezza;
        }

        while(i != 1 && abs(blocco[i].Y - blocco[i-1].Y) > distacco)//|| (i <= 1 && abs(blocco[i].Y - blocco[i-1].Y) > distacco + e*2))
            blocco[i].Y = distacco*i - finegioco + rand()%100-50;

    }
}

/**

    \brief Draws the clouds
    \todo Make sure that it draws clouds and not rectangles

*/

void disegnaNuvolette()
{
    for(int i = 0; i < numeroblocchi; i++)
    {
        draw_filled_rect(blocco[i].X,blocco[i].Y, blocco[i].larghezza,blocco[i].altezza, Color(255,255,255,255));
        if(blocco[i].coso.flag && blocco[i].Y + cosini[blocco[i].coso.tipo].altezza > 0 && blocco[i].Y - cosini[blocco[i].coso.tipo].altezza < finestray)
        {
            draw_image(cosini[blocco[i].coso.tipo].file,blocco[i].coso.X,blocco[i].coso.Y,
            cosini[blocco[i].coso.tipo].larghezza,cosini[blocco[i].coso.tipo].altezza,255);
        }
    }
    //if(blocco[i].Y < finestray && blocco[i].Y > 0)
    //draw_image("nuvola.png",blocco[i].X,blocco[i].Y, blocco[i].larghezza, blocco[i].altezza);

}

/**

    \brief Move the character using right and left arrow

*/

void coseCoiTasti()
{
    if(is_pressed(VSGL_LEFT))
        x_personaggio--, direzione = -1;

    if(is_pressed(VSGL_RIGHT))
        x_personaggio++, direzione = 1;
}

/**

    \brief Move the character using the mouse (it follows the mouse)

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

/**

    \brief write on a file (named "file__" + a char that you decide) the values of a vector
    Used to debug, because vector are too giants to print on the stdo
*/


void stampaSulFileDiLog(int N, char numero_del_file)
{
    string a = "IO_files/file__";
    a += numero_del_file;
    ofstream out;
    out.open(a);
    for(int i = 0; i < N; i++)
        out <<  blocco[i].X << " " << blocco[i].Y << endl;
}


/**

    \brief Make sure that clouds go down

*/

void spostaSchermo()
{
    for(int i = 0; i < numeroblocchi; i++)
    {
        blocco[i].Y++;
        if(blocco[i].coso.flag)
            blocco[i].coso.Y++;
    }

}


/**

    \brief Two functions in one:
    1) Initializes the variables objects with images name
    2) Assignes objects to blocks

*/

void oggetti()
{

    cosini[0].file = "Images/gelato.png";
    cosini[0].altezza = 50;
    cosini[0].larghezza = 18;
    cosini[1].file = "Images/rainbow.png";
    cosini[1].altezza = 32;
    cosini[1].larghezza = 45;

    for(int i = 0; i < numeroblocchi; i++)
    {
        if(i%frequenzaOggetti == frequenzaOggetti-1)
        {
            int c = rand()%quantiOggetti;
            blocco[i].coso.X = blocco[i].X + rand()%(blocco[i].larghezza - cosini[blocco[i].coso.tipo].larghezza);
            blocco[i].coso.Y = blocco[i].Y - cosini[c].altezza;
            blocco[i].coso.flag = true;
            blocco[i].coso.tipo = c;
        }
    }
}


/**

    This function is the real part of the game
    It's late and I'm tired and I want to sleep SOOO no, I won't translate this part LOL
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
    //int tempo = ms_time();
    disegnaSfondoVero(stringa);
    //cout << tempo << " " << ms_time() << " " << -tempo+ms_time() << endl;
    if(velocitaI - epsilonV < -tempo+ms_time() && -tempo+ms_time()  > velocitaI + epsilonV && cont > 50)
        tempo = ms_time(), spostaSchermo();

    if(strcmp("tasti", scelta) == 0)
        coseCoiTasti();

    else if(strcmp("mouse", scelta) == 0)
        coseColMouse(cont);

    disegnaNuvolette();
    if(direzione == 1)
        draw_image("Images/unicorno2.png", x_personaggio, y_personaggio,personaggio,personaggio,255);
    if(direzione == -1)
        draw_image("Images/unicorno1.png", x_personaggio, y_personaggio,personaggio,personaggio,255);


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

/**

    Yeee :)

*/

void vinto()
{
    draw_filled_rect(0,0,finestrax,finestray,Color(255,255,255,255));
    draw_image("Images/fine.jpg",0,0,626, 626, 255);
    draw_image("Images/Win.png",15,550,571,130,255);
}

/**

    \brief Update the file with all the scores
    \todo Make a file with only 5 - 10 scores

*/

void score()
{
    char nome[100];
    cout << "Bravo! Inserisci il tuo nome" << endl;
    cin >> nome;
    FILE *out;
    out = fopen("IO_Files/Vincitori.txt", "a");
    fprintf(out, "%s %d\n", nome, punteggio);
    fclose(out);
    delay(1000);
    //cout << "Vuoi fare un'altra partita? \n \t Premi 1 per giocare ancora \n \t Premi 0 per uscire \n";
    exit(0);
}


/**

    Noo :(

*/

void perso()
{
    draw_filled_rect(0,0,finestrax,finestray,Color(21,27,49,255));
    draw_image("Images/sad.png",(finestrax-500)/2,(finestray-600)/2,500,600,255);
    update();
    cout << "Il tuo punteggio è: " << punteggio << endl;
    score();
}

/**
    \brief Draws menu
    \return Case 1: the game starts
            Case 2: instructions
*/


int menu()
{
    draw_image("Images/Untitled.png",0,0,finestrax,finestray,255);
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

/**
    This shit doesn't work ahhhhhhhh
*/

void modificaVelocita()
{
    //epsilonV++;
    velocitaI--;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    eratostene();
    char stringa[1000] = "Images/sfondoN.jpg";
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
            oggetti();
            tempo = ms_time();
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
               // if(punteggio == 50)
                 //   velocitaI--;
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

