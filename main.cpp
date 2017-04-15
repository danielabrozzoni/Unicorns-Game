#include "vsgl2.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <algorithm>
#include <vector>

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
const int quantiOggetti = 3;
const int frequenzaOggetti = 20;
const int quantiPunteggi = 5;
const int numeroVite = 5;
const int grandezzaCuoricino = 35;
const int distacco = 180;
const int quantiPersonaggi = 3;
const int altezzaBottone = 40;
const int larghezzaBottone = 150;
const int bottoneX = 520;
const int bottoneY = 620;
const int bottone = 50;

int tempo;
int velocitaI = 2;
int velocitaP = 15;
int direzione = 1;
int x_personaggio = finestrax/2;
int y_personaggio = finestray - personaggio;
double Vy = VyIniziale;
int vittoria = 0;                       // 0 se il gioco deve andare avanti, 1 se hai vinto, -1 se hai perso
int cont = 0;
int punteggio = 0;
bool is_prime[numeroblocchi];
int vite = 0;
bool Vite[numeroVite];
int ultimoTocco;

/**
    \todo menù iniziale
    \todo impostazioni mouse/tastiera
    \todo mostri
    \todo sparare
    \todo molla
    \todo piattaforme che si rompono
    \todo visualizzare record
    \todo se muori ritorni sull'ultimo blocco toccato
*/

/**
    OGGETTI POSSIBILI:
    0: cono gelato -> bonus 200 punti
    1: arcobaleno -> quando ne collezioni 5 puoi sparare una mega arcobalenata
    2: cuore -> vita in più
*/

struct Top5
{
    string nome;
    int punteggio;
    bool operator<(const Top5& b)const
    {
        return punteggio > b.punteggio;
    }
};


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
Top5 top[quantiPunteggi + 1];       //In memoria devo ricordarmi quantiPunteggi diversi, quindi creo un elemento in più per salvarmi l'attuale

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
    Shows the instructions
*/

void istruzioni()
{
    while(1)
    {
        draw_image("Images/istruzioni1.png", 0,0,finestrax, finestray, 255);
        draw_image("Images/bottone.png", bottoneX, bottoneY, bottone, bottone);
        int a = get_mouse_x();
        int b = get_mouse_y();
        if(mouse_left_button_pressed() && a >= bottoneX && a <= bottoneX + bottone && b >= bottoneY && b <= bottoneY + bottone)
            break;
        update();
    }
}

/**
    Reads on a file what are the last settings used and initializes them, so, if you kill
    the process, when you run it again you have the last setting you've used
    If the file doesn't exists or the settings aren't correct, the function ends
*/


void ultimeImpostazioni(char sx[], char dx[], char sfondo[], char scelta[])
{
    ifstream in;
    in.open("IO_files/settings.txt");
    char temp[100];
    in >> temp;
    if(strcmp(temp,"Images/unicorno1.png") != 0 && strcmp(temp,"Images/unicorno3.png") != 0 && strcmp(temp,"Images/unicorno5.png") != 0)
    {
        cout << "Errore: " << temp << endl;
        return;
    }
    strcpy(sx, temp);
    if(strcmp(temp,"Images/unicorno1.png") == 0)
        strcpy(dx, "Images/unicorno2.png");
    else if(strcmp(temp,"Images/unicorno3.png") == 0)
        strcpy(dx, "Images/unicorno4.png");
    else if(strcmp(temp,"Images/unicorno5.png") == 0)
        strcpy(dx, "Images/unicorno6.png");
    else
        return;

    in >> temp;

    if(strcmp(temp,"Images/sfondo1.jpg") != 0 && strcmp(temp,"Images/sfondo2.jpg") != 0 && strcmp(temp,"Images/sfondo3.jpg") != 0)
    {
        cout << "Errore: " << temp << endl;
        return;
    }
    strcpy(sfondo, temp);

    in >> temp;

    if(strcmp(temp,"mouse") != 0 && strcmp(temp,"tasti") != 0)
    {
        cout << "Errore: " << temp << endl;
        return;
    }

    strcpy(scelta, temp);
}

/**
    Writes on settings.txt last settings used
*/

void aggiornaImpostazioni(char sx[], char dx[], char sfondo[], char scelta[])
{
    ofstream out;
    out.open("IO_files/settings.txt");
    out << sx << endl << sfondo << endl << scelta;
}

/**

    Initializes all the variables that I'll use, it will be helpful in the future if I write something to play
    two or more consecutives games
    \todo  write something to play two or more consecutives games

*/

void inizializzaTutto()
{
    tempo = ms_time();
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

    for(int i = 0; i < numeroVite; i++)
    {
        if(Vite[i] == 1)
            draw_image("Images/cuore.png",10 + i*grandezzaCuoricino, finestray - 10 - grandezzaCuoricino, grandezzaCuoricino, grandezzaCuoricino);
    }


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
    Used to debug, because vector are too giants to print on the stdio
*/


void stampaSulFileDiLog(int N, char numero_del_file)
{
    string a = "IO_files/file__";
    a += numero_del_file;
    a += ".txt";
    ofstream out;
    out.open(a);
    for(int i = 0; i < N; i++)
        out <<  blocco[i].X << " " << blocco[i].Y << endl;
}


/**

    \brief Make sure that clouds go down

*/

void spostaSchermo(int piu)
{
    for(int i = 0; i < numeroblocchi; i++)
    {
        blocco[i].Y+= piu;
        if(blocco[i].coso.flag)
            blocco[i].coso.Y+= piu;
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
    cosini[2].file = "Images/cuore.png";
    cosini[2].altezza = 45;
    cosini[2].larghezza = 45;


    /**
        Questa parte del codice potrebbe sembrare oscura
        Dato che non è giusto che ogni oggetto appaia sullo schermo con la stessa frequenza,
        inserisco i vari tipi di oggetti in un vector (gli oggetti con frequenza maggiore vengono
        inseriti più volte), dopodiché attingo dal vector in modo random e disegno l'oggetto
        2/4 degli oggetti è un gelato
        1/4 rainbow
        1/4 cuore
    */
    vector<int> objects;
    for(int i = 0; i < 2; i++)
        objects.push_back(0);
    for(int i = 0; i < 1; i++)
        objects.push_back(1);
    for(int i = 0; i < 1; i++)
        objects.push_back(2);



    for(int i = 0; i < numeroblocchi; i++)
    {
        if(i%frequenzaOggetti == frequenzaOggetti-1)
        {
            int c = objects.at(rand()%objects.size());
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

void gioco(char stringa[], char scelta[], char sx[], char dx[])
{
    //int tempo = ms_time();
    disegnaSfondoVero(stringa);
    //cout << tempo << " " << ms_time() << " " << -tempo+ms_time() << endl;
//    if(velocitaI - epsilonV < -tempo+ms_time() && -tempo+ms_time()  > velocitaI + epsilonV && cont > 150)
    //      tempo = ms_time();

    if(strcmp("tasti", scelta) == 0)
        coseCoiTasti();

    else if(strcmp("mouse", scelta) == 0)
        coseColMouse(cont);

    disegnaNuvolette();
    if(direzione == 1)
        draw_image(dx, x_personaggio, y_personaggio,personaggio,personaggio,255);
    if(direzione == -1)
        draw_image(sx, x_personaggio, y_personaggio,personaggio,personaggio,255);

    if(ms_time() - tempo >= velocitaP)
    {
        tempo = ms_time();
        y_personaggio -= Vy;
        Vy -= decelerazione;
    }

    for(int i = 0; i < numeroblocchi; i++)
        if(abs(blocco[i].Y - y_personaggio - personaggio + A) < e && x_personaggio + personaggio  >= blocco[i].X && x_personaggio < blocco[i].X + blocco[i].larghezza && blocco[i].Y < finestray && blocco[i].Y > 0 && Vy <= 0)
        {
            Vy = VyIniziale;
            ultimoTocco = i;
            if(blocco[i].toccato == 0)      //Aumento di punti solo se è la prima volta che tocco un blocco, altrimenti faccio punti restando fermo
                punteggio += 10, blocco[i].toccato = 1;
            if(blocco[i].coso.flag && x_personaggio + personaggio >= blocco[i].coso.X && x_personaggio < blocco[i].coso.X
                    + cosini[blocco[i].coso.tipo].larghezza)
            {
                blocco[i].coso.flag = 0;
                switch(blocco[i].coso.tipo)
                {
                case 0:
                    punteggio += 20;
                    break;
                case 1:
                    spostaSchermo(500);
                    break;
                case 2:
                    if(vite >= 5)
                        break;
                    Vite[vite] = 1;
                    vite++;
                default:
                    break;
                }
            }
        }

    if(x_personaggio > finestrax)
        x_personaggio = 0;

    if(x_personaggio < 0)
        x_personaggio = finestrax - personaggio;

    if(punteggio == finegioco)
        vittoria = 1;

    if(y_personaggio > finestray && vite > 0)
    {
        x_personaggio = blocco[ultimoTocco].X + 10;
        y_personaggio = blocco[ultimoTocco].Y - personaggio;
        vite--;
        Vite[vite] = 0;
    }

    if(y_personaggio > finestray)
        vittoria = -1;

    if(y_personaggio < distacco)
        spostaSchermo(1);


    cont++;
}

/**

    Yeee :)

*/

void vinto()
{
    draw_filled_rect(0,0,finestrax,finestray,Color(255,255,255,255));
    draw_image("Images/Win.png",15,550,571,130,255);
    draw_image("Images/fine.jpg",0,0,626, 626, 255);
}


/**
    Questa funzione mi legge dal file dei vincitori quali sono i migliori,
    li mette in strutture apposite
*/


void sempreperipunteggi()
{
    int i=0;
    ifstream in;
    in.open("IO_files/Vincitori.txt");
    while(i < quantiPunteggi && in)
    {
        in >> top[i].nome >> top[i].punteggio;
        i++;
    }
    in.close();
}


/**

    \brief Update the file with all the scores
    \todo Make a file with only 5 - 10 scores

*/

void score()
{
    sempreperipunteggi();
    cout << "Bravo! Inserisci il tuo nome" << endl;
    cin >> top[quantiPunteggi].nome;
    top[quantiPunteggi].punteggio = punteggio;
    sort(top,top+quantiPunteggi+1);
    ofstream out;
    out.open("IO_Files/Vincitori.txt");
    for(int i = 0; i < 5; i++)
        out << top[i].nome << " " << top[i].punteggio << endl;
    out.close();
    delay(1000);
    //exit(0);
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
            Case 3: impostazioni
*/


int menu()
{
    draw_image("Images/Untitled.png",0,0,finestrax,finestray,255);
    int a = get_mouse_x();
    int b = get_mouse_y();
    const int margine = 60;
    const int distacco = 30;
    const int numerobottoni = 3;
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

    draw_image("Images/gioca.png", margine, inizioBottoni, larghezzaBottone, altezzaBottone, 255);
    draw_image("Images/istruzioni.png", margine, inizioBottoni + altezzaBottone + distacco, larghezzaBottone, altezzaBottone, 255);
    draw_image("Images/impostazioni.png", margine, inizioBottoni + 2*altezzaBottone + 2*distacco, larghezzaBottone, altezzaBottone, 255);

    return 0;
}

/**
    Changes the settings, like the charachter you are using, the background...
*/

void settings(char sinistra[], char destra[], char sfondo[], char scelta[])
{

    while(1)
    {
        draw_image("Images/Settings.png", 0, 0, finestrax, finestray, 255);

        const int margineY = 130;
        const int distanza = 50;
        const int spessore = 2;
        const int margineX = (finestrax - personaggio*quantiPersonaggi - distanza * (quantiPersonaggi - 1)) / 2;

        draw_image("Images/unicorno1.png",margineX, margineY, personaggio, personaggio);
        draw_image("Images/unicorno3.png",2*margineX + distanza, margineY, personaggio, personaggio);
        draw_image("Images/unicorno5.png",3*margineX + distanza*2, margineY, personaggio, personaggio);
        draw_image("Images/sfondo1_copia.jpg", margineX, margineY*2 + personaggio, personaggio, personaggio);
        draw_image("Images/sfondo2_copia.jpg",2*margineX + distanza, margineY*2 + personaggio, personaggio, personaggio);
        draw_image("Images/sfondo3_copia.jpg",3*margineX + distanza*2, margineY*2 + personaggio, personaggio, personaggio);
        draw_image("Images/bottone.png", bottoneX, bottoneY, bottone, bottone);
        draw_filled_rect(margineX, margineY*3 + personaggio*2, larghezzaBottone, altezzaBottone, Color(255,255,255,255));
        draw_filled_rect(2*margineX + larghezzaBottone, margineY*3 + personaggio*2, larghezzaBottone, altezzaBottone, Color(255,255,255,255));
        draw_image("Images/mouse.png",margineX, margineY*3 + personaggio*2, larghezzaBottone, altezzaBottone);
        draw_image("Images/Tastiera.png",2*margineX + larghezzaBottone, margineY*3 + personaggio*2, larghezzaBottone, altezzaBottone);

        if(strcmp(sinistra, "Images/unicorno1.png") == 0)
            draw_rect(margineX - spessore, margineY - spessore, personaggio + spessore*2, personaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sinistra, "Images/unicorno3.png") == 0)
            draw_rect(2*margineX + distanza - spessore, margineY - spessore, personaggio + spessore*2, personaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sinistra, "Images/unicorno5.png") == 0)
            draw_rect(3*margineX + distanza*2 - spessore, margineY - spessore, personaggio + spessore*2, personaggio + spessore*2, Color(0,0,0,255));

        if(strcmp(sfondo, "Images/sfondo1.jpg") == 0)
            draw_rect(margineX - spessore, margineY*2 + personaggio - spessore, personaggio + spessore*2, personaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sfondo, "Images/sfondo2.jpg") == 0)
            draw_rect(2*margineX + distanza - spessore, margineY*2 + personaggio - spessore, personaggio + spessore*2, personaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sfondo, "Images/sfondo3.jpg") == 0)
            draw_rect(3*margineX + distanza*2 - spessore, margineY*2 + personaggio - spessore, personaggio + spessore*2, personaggio + spessore*2, Color(0,0,0,255));

        if(strcmp(scelta, "mouse") == 0)
            draw_rect(margineX - spessore, margineY*3 + personaggio*2 - spessore, larghezzaBottone + spessore*2, altezzaBottone + spessore*2, Color(0,0,0,255));
        if(strcmp(scelta, "tasti") == 0)
            draw_rect(2*margineX + larghezzaBottone - spessore, margineY*3 + personaggio*2 - spessore, larghezzaBottone + spessore*2, altezzaBottone + spessore*2, Color(0,0,0,255));

        int a = get_mouse_x();
        int b = get_mouse_y();

        if(mouse_left_button_pressed() && a >= margineX
                && a <= margineX + personaggio && b >= margineY && b <= margineY + personaggio)
            strcpy(sinistra, "Images/unicorno1.png"), strcpy(destra, "Images/unicorno2.png");

        if(mouse_left_button_pressed() && a >= 2*margineX + distanza
                && a <= 2*margineX + distanza + personaggio && b >= margineY && b <= margineY + personaggio)
            strcpy(sinistra, "Images/unicorno3.png"), strcpy(destra, "Images/unicorno4.png");

        if(mouse_left_button_pressed() && a >= 3*margineX + distanza*2
                && a <= 3*margineX + distanza*2 + personaggio && b >= margineY && b <= margineY + personaggio)
            strcpy(sinistra, "Images/unicorno5.png"), strcpy(destra, "Images/unicorno6.png");

        if(mouse_left_button_pressed() && a >= margineX
                && a <= margineX + personaggio && b >= margineY*2 + personaggio && b <= margineY*2 + personaggio + personaggio)
            strcpy(sfondo, "Images/sfondo1.jpg");

        if(mouse_left_button_pressed() && a >= 2*margineX + distanza
                && a <= 2*margineX + distanza + personaggio && b >= margineY*2 + personaggio && b <= margineY*2 + personaggio + personaggio)
            strcpy(sfondo, "Images/sfondo2.jpg");

        if(mouse_left_button_pressed() && a >= 3*margineX + distanza*2
                && a <= 3*margineX + distanza*2 + personaggio && b >= margineY*2 + personaggio && b <= margineY*2 + personaggio + personaggio)
            strcpy(sfondo, "Images/sfondo3.jpg");

        if(mouse_left_button_pressed() && a >= margineX
                && a <= margineX + larghezzaBottone && b >=  margineY*3 + personaggio*2 && b <=  margineY*3 + personaggio*2 + altezzaBottone)
            strcpy(scelta, "mouse");

        if(mouse_left_button_pressed() && a >= 2*margineX + larghezzaBottone
                && a <= 2*margineX + larghezzaBottone*2 && b >=  margineY*3 + personaggio*2 && b <=  margineY*3 + personaggio*2 + altezzaBottone)
            strcpy(scelta, "tasti");

        if(mouse_left_button_pressed() && a >= bottoneX && a <= bottoneX + bottone && b >= bottoneY && b <= bottoneY + bottone)
            break;

        update();
    }

}

int main(int argc, char* argv[])
{
    srand(time(NULL));
    eratostene();
    char sfondo[1000] = "Images/sfondo1.jpg";
    char personaggioSx[1000] = "Images/unicorno5.png";
    char personaggioDx[1000] = "Images/unicorno6.png";
    char scelta[100] = "mouse";
    int cosafare = 0;
    int flag = 0;
    ultimeImpostazioni(personaggioSx, personaggioDx, sfondo, scelta);
    init();
    set_window(finestrax,finestray,"Unicorn's Game");

    while(!done())
    {
        while(!done())
        {
            cosafare = menu();
            if(cosafare != 0)
                break;
            update();
        }

        switch(cosafare)
        {
        case 1:                                           //GIOCO
        {
            creaCoseCasuali();
            inizializzaTutto();
            oggetti();

            while(!done())
            {
                if(vittoria == 0)
                    gioco(sfondo, scelta, personaggioSx, personaggioDx);
                if(vittoria == 1)
                    vinto();
                if(vittoria == -1)
                {
                    perso();
                    break;
                }

                update();
            }

            break;
        }

        case 2:
        {
            istruzioni();
            break;
        }

        case 3:                                         //SETTINGS
        {
            settings(personaggioSx, personaggioDx, sfondo, scelta);
            aggiornaImpostazioni(personaggioSx, personaggioDx, sfondo, scelta);
            break;
        }

        default:
            continue;
        }
    }
    close();
    return 0;
}

