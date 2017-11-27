#include "Vsgl2/vsgl2.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string.h>

using namespace vsgl2;
using namespace vsgl2::general;
using namespace vsgl2::video;
using namespace vsgl2::utils;
using namespace vsgl2::io;
using namespace vsgl2::audio;
using namespace vsgl2::ttf_fonts;
using namespace std;

const int grandezzaPietra = 15;
const int grandezzaPersonaggio = 100;
const int grandezzaBlocchi = 125;
const int grandezzaCuoricino = 35;
const int grandezzaFreccia = 50;
const int altezzaBottone = 50;
const int larghezzaBottone = 150;

const int numeroblocchi = 10000;
const int finestrax = 600;
const int finestray = 700;
const int finegioco = 1000000;
const int A = 10;           //epsilon
const int e = 30;           //epsilon
const double decelerazione = 0.981;
const double VyIniziale = 20;
const int distacco = 135;
const int quantiOggetti = 3;
const int frequenzaOggetti = 10;
const int quantiPunteggi = 5;
const int numeroVite = 5;
const int quantiPersonaggi = 3;
const int bottoneX = 520;
const int bottoneY = 620;
const int bottone = 50;
const int memoriaProiettili = 20;
const int quantoSiMuovonoBlocchi2 = 200;
const int durataRandom = 20;            //Non si capisce dal nome, sono parametri
const int pezziRandom = 3;              // Per inserire i pezzi neri
const int velocitaBlocchi2 = 7;
const int quantiBlocchi = 4;
const int quantiTipiMostri = 2;
const int quantiMostriNelGioco = 5;
const int viteMostro = 2;
const int marginePunteggio = 10;        //Per disegnare gli high scores

int tempo;
int velocitaP = 15;
int direzione = 1;
int x_personaggio = finestrax/2;
int y_personaggio = finestray - grandezzaPersonaggio;
double Vy = VyIniziale;
int vittoria = 0;                       // 0 se il gioco deve andare avanti, 1 se hai vinto, -1 se hai perso
int cont = 0;
int punteggio = 0;
bool is_prime[numeroblocchi];
int vite = 0;
bool Vite[numeroVite];
int ultimoTocco;
int quantiProiettili = 0;

/**
    \todo mostri
    \todo sparare
    \todo visualizzare record

*/

/**
    OGGETTI POSSIBILI:
    0: cono gelato -> bonus 200 punti
    1: arcobaleno -> ti cambia la posizione dei blocchi
    2: cuore -> vita in più
*/

/**

    TIPI DI PIATTAFORME:
    0: bianco -> normale
    1: azzurro chiaro -> fantasma
    2: rosa -> si muovono a destra e sinistra
    3: nero -> se le tocchi scompaiono

*/

/**

    If namespace font doesn't work:
    Compiler -> linker -> -lSDL2_ttf

*/

struct Top5
{
    char nome[1000];
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
    int tipoBlocco = 0;
    int X_iniziale = 0;
    int direzione = 1;
    int flag = 1;
    Oggetto coso;               //oggetto sopra
};

struct TipoBlocco
{
    int r, g, b, a;
};

struct Proiettile
{
    int X = -1;
    int Y = -1;
};

struct Mostro
{
    int X = -1;
    int Y = -1;
    int vite = viteMostro;
    int tipo = 0;
};

Blocco blocco[numeroblocchi];
Example EsempiOggetti[quantiOggetti];
Example EsempiMostri[quantiTipiMostri];
Top5 top[quantiPunteggi + 1];       //In memoria devo ricordarmi quantiPunteggi diversi, quindi creo un elemento in più per salvarmi l'attuale
Proiettile proiettile[memoriaProiettili];
TipoBlocco ColoriBlocchi[quantiBlocchi];
Mostro mostro[quantiMostriNelGioco];

/**

    \brief As the name says, this function isn't used in the real program.
    Teacher was speaking and I thought that this would be useful
    I used it when I have to shot monsters, but probably in the future it will be useful,
    so I decided to keep it here
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
    \brief It shows the score
*/

void ShowScore()
{
    char buffer[100];
    int n = sprintf(buffer, "%d", punteggio);
    draw_text("Font/Pixeled.ttf",15,"SCORE", marginePunteggio, marginePunteggio, Color(0,0,0,255));
    draw_text("Font/Pixeled.ttf",15,buffer,marginePunteggio, marginePunteggio+text_height("Font/Pixeled.ttf",15,"SCORE"), Color(0,0,0,255));
}


/**
    \brief It shows the instructions
*/

void instructions()
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
    \brief It reads on a file last settings used and initializes them, so, if you kill
    the process, when you run it again you have the last setting you've used
    If the file doesn't exists or the settings aren't correct, the function ends
*/


void lastSettings(char sx[], char dx[], char sfondo[], char scelta[])
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
    \brief Writes on settings.txt last settings used
*/

void updtateSettings(char sx[], char dx[], char sfondo[], char scelta[])
{
    ofstream out;
    out.open("IO_files/settings.txt");
    out << sx << endl << sfondo << endl << scelta;
}

/**

    \brief Initializes all the variables that will be used, so, if you play two or more games,
    it resets your points, your position ecc...


*/

void inizializzaTutto()
{
    punteggio = 0;
    tempo = ms_time();
    x_personaggio = finestrax/2;
    y_personaggio = finestray - grandezzaPersonaggio;
    Vy = VyIniziale;
    vittoria = 0;
    cont = 0;
    for(int i = 0; i < memoriaProiettili; i++)
        proiettile[i].X = proiettile[i].Y = -1;
    for(int i = 0; i < numeroblocchi; i++)
        blocco[i].toccato = 0;
    for(int i = 0; i < quantiMostriNelGioco; i++)
        mostro[i].vite = viteMostro;

}


/**

    \param background's name
    \brief It draws the background. Only one line of code, it seems like this function
    is useless, but you can decide in the main what is the background name and pass it
    to this. (Ok, I've realized that this function is useless, but I've become
    attached to it, so)

*/

void disegnaSfondo(char stringa[])
{
    draw_image(stringa, 0, 0,  finestrax, finestray, 255);
}

/**

    Do you know who Eratostene is?
    Oke, I know, prime numbers are not the best way to create randomness
    However, I think that they have interesting charateristic:
    there are only 2 prime numbers that are consecutives,
    big prime number are more distant than smaller, so, maybe, using it to
    draw the blocks isn't a bad idea
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
    }
}


/**

   \brief At first, initializes the colors of the various blocks
   Uses random number to decide which color is every block, after that, sets their
   position and make sure that two blocks aren't too near

*/

void creaCoseCasuali()
{
    //  cout << "in" << endl;
    ColoriBlocchi[0].r = 255;
    ColoriBlocchi[0].g = 255;
    ColoriBlocchi[0].b = 255;
    ColoriBlocchi[0].a = 255;
    ColoriBlocchi[1].r = 148;
    ColoriBlocchi[1].g = 221;
    ColoriBlocchi[1].b = 252;
    ColoriBlocchi[1].a = 200;
    ColoriBlocchi[2].r = 199;
    ColoriBlocchi[2].g = 71;
    ColoriBlocchi[2].b = 175;
    ColoriBlocchi[2].a = 255;
    ColoriBlocchi[3].r = 0;
    ColoriBlocchi[3].g = 0;
    ColoriBlocchi[3].b = 0;
    ColoriBlocchi[3].a = 255;


    blocco[0].X = 0;
    blocco[0].Y = finestray-A ;
    blocco[0].altezza = A;
    blocco[0].larghezza = finestrax;

    int cosiRandom[3];
    for(int i = 0; i < 3; i++)
        cosiRandom[i] =  (rand()%100 + 100 + 2000*i);

    for(int i = 1; i < numeroblocchi; i++)
    {
        if(is_prime[i])
            blocco[i].tipoBlocco = 1;
        if(!is_prime[i-1] && i+3 < numeroblocchi && is_prime[i+rand()%3])
            blocco[i].tipoBlocco = 2;
        if(i % 25 + rand()%5 == 0)
            blocco[i].tipoBlocco = 3;
        for(int j = 0; j < pezziRandom; j++)
            if(i > cosiRandom[j] && i < cosiRandom[j] + durataRandom)
                blocco[i].tipoBlocco = 3;

        blocco[i].larghezza = grandezzaBlocchi + rand()%10 - 5;
        blocco[i].altezza = A;
        blocco[i].Y = distacco*i - finegioco + rand()%100-50;

        if(blocco[i].tipoBlocco != 2)
        {
            blocco[i].X = rand()%(finestrax - 2*blocco[i].larghezza);

            while((i != 1 && (blocco[i].X <= 0 || blocco[i].X >= finestrax - blocco[i].larghezza || abs(blocco[i].X - blocco[i-1].X) < grandezzaPersonaggio*2 ||
                              blocco[i-1].X < 0)) || (i > 2 && abs(blocco[i].X - blocco[i-2].X) < 20))
            {
                blocco[i].X = rand()%(finestrax - blocco[i].larghezza);
            }
        }

        else
            blocco[i].X = rand()%(finestrax - 2*blocco[i].larghezza - quantoSiMuovonoBlocchi2);

        while(i != 1 && abs(blocco[i].Y - blocco[i-1].Y) > distacco)//|| (i <= 1 && abs(blocco[i].Y - blocco[i-1].Y) > distacco + e*2))
            blocco[i].Y = distacco*i - finegioco + rand()%100-50;

        blocco[i].X_iniziale = blocco[i].X;
    }

}

/**

    \brief It draws the clouds (even if they are rectangles), the lives you have,
    the monsters and the shots

*/

void disegna()
{
    for(int i = 0; i < numeroblocchi; i++)
    {
        if(blocco[i].flag == 1)
        {
            draw_filled_rect(blocco[i].X,blocco[i].Y, blocco[i].larghezza,blocco[i].altezza, Color(ColoriBlocchi[blocco[i].tipoBlocco].r,ColoriBlocchi[blocco[i].tipoBlocco].g,ColoriBlocchi[blocco[i].tipoBlocco].b,ColoriBlocchi[blocco[i].tipoBlocco].a));
            if(blocco[i].coso.flag && blocco[i].Y + EsempiOggetti[blocco[i].coso.tipo].altezza > 0 && blocco[i].Y - EsempiOggetti[blocco[i].coso.tipo].altezza < finestray)
            {
                draw_image(EsempiOggetti[blocco[i].coso.tipo].file,blocco[i].coso.X,blocco[i].coso.Y,
                           EsempiOggetti[blocco[i].coso.tipo].larghezza,EsempiOggetti[blocco[i].coso.tipo].altezza,255);
            }
        }

    }

    for(int i = 0; i < numeroVite; i++)
    {
        if(Vite[i] == 1)
            draw_image("Images/cuore.png",10 + i*grandezzaCuoricino, finestray - 10 - grandezzaCuoricino, grandezzaCuoricino, grandezzaCuoricino);
    }

    for(int i = 0; i < memoriaProiettili; i++)
    {
        if(proiettile[i].Y >= 0 && proiettile[i].Y <= finestray && proiettile[i].X >= 0
                && proiettile[i].X <= finestrax)
            draw_image("Images/pietra.png", proiettile[i].X, proiettile[i].Y, grandezzaPietra, grandezzaPietra, 255);
    }

    for(int i = 0; i < quantiMostriNelGioco; i++)
    {
        if(mostro[i].vite > 0 && mostro[i].Y + EsempiMostri[mostro[i].tipo].altezza >= 0 && mostro[i].Y <= finestray && mostro[i].X >= 0
                && mostro[i].X <= finestrax)
            draw_image(EsempiMostri[mostro[i].tipo].file, mostro[i].X, mostro[i].Y, EsempiMostri[mostro[i].tipo].larghezza, EsempiMostri[mostro[i].tipo].altezza, 255);
        if(abs(y_personaggio - mostro[i].Y) < finestray && mostro[i].vite > 0)
            draw_image("Images/freccia.png", mostro[i].X + EsempiMostri[mostro[i].tipo].larghezza/2, finestray - 100, grandezzaFreccia, grandezzaFreccia, 255);
    }


}

/**

    \brief It moves the character using right and left arrow

*/

void coseCoiTasti()
{
    if(is_pressed(VSGL_LEFT))
        x_personaggio--, direzione = -1;

    if(is_pressed(VSGL_RIGHT))
        x_personaggio++, direzione = 1;
}

/**

    \brief It moves the character using the mouse (it doesn't follow the mouse, if the
    mouse is in the left part of the screen he goes left, right otherwise)

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

    \brief It writes on a file (named "file__" + a char that you decide)
    the values of a vector
    It is used to debug, because vector are too giants to print on the stdout
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

    \brief It makes sure that clouds go down

*/

void spostaSchermo(int piu)
{
    for(int i = 0; i < numeroblocchi; i++)
    {
        blocco[i].Y+= piu;
        if(blocco[i].coso.flag)
            blocco[i].coso.Y+= piu;
    }

    for(int i = 0; i < memoriaProiettili; i++)
    {
        proiettile[i].Y += piu;
        proiettile[i].X += piu/2;
    }

    for(int i = 0; i < quantiMostriNelGioco; i++)
        mostro[i].Y += piu;
}


/**

    \brief It initializes the variables that contain all the objects' and monsters'
    parametres, after that, it sets objects' and monsters' positions on the screen

*/

void oggetti()
{

    EsempiOggetti[0].file = "Images/gelato.png";
    EsempiOggetti[0].altezza = 50;
    EsempiOggetti[0].larghezza = 18;
    EsempiOggetti[1].file = "Images/rainbow.png";
    EsempiOggetti[1].altezza = 32;
    EsempiOggetti[1].larghezza = 45;
    EsempiOggetti[2].file = "Images/cuore.png";
    EsempiOggetti[2].altezza = 45;
    EsempiOggetti[2].larghezza = 45;

    EsempiMostri[0].file = "Images/bat.png";
    EsempiMostri[0].altezza = 90;
    EsempiMostri[0].larghezza = 220;



    /**
        I don't think is okey that the game gift you a life as probably as it gift you
        a rainbow (that doesn't help you)
        So, I created a vector when I insert every object N times (the greater is N, the
        greater is the probability to take that object)
        the 50% of the object is a ice cream
        50% of the objects is an ice creame
        33% is a rainbow
        17% is a heart
    */

    vector<int> objects;
    for(int i = 0; i < 3; i++)
        objects.push_back(0);
    for(int i = 0; i < 2; i++)
        objects.push_back(1);
    for(int i = 0; i < 1; i++)
        objects.push_back(2);



    for(int i = 0; i < numeroblocchi; i++)
    {
        if(i%frequenzaOggetti == frequenzaOggetti-1 && !is_prime[i])
        {
            int c = objects.at(rand()%objects.size());
            blocco[i].coso.X = blocco[i].X + rand()%(blocco[i].larghezza - EsempiOggetti[blocco[i].coso.tipo].larghezza);
            blocco[i].coso.Y = blocco[i].Y - EsempiOggetti[c].altezza;
            blocco[i].coso.flag = true;
            blocco[i].coso.tipo = c;
        }
    }

    for(int i = 0; i < quantiMostriNelGioco; i++)
    {
        mostro[i].X = rand()%finestrax/2;
        mostro[i].Y = -3000*(i+1) + rand()%10;
    }

}


/**

    \brief This function is the real part of the game
    It does a lot of things, I'll try to write them in order
    1 -> It draws the background
    2 -> It reads the input (mouse/keyboard) and it calls the function that moves the unicorn
    3 -> It draws the unicorn
    4 -> It makes sure that the unicorn jump every N ms
    5 -> It moves pink blocks
    6 -> It makes sure that you can jump on blocks and that you can collect objects
    7 -> It controls about lives and shots
    \param char stringa[] -> background's name
    \param char scelta[] -> It can be "mouse" or "tasti"
    \param char sx[] and char dx[] -> unicorn's image files

*/

void gioco(char stringa[], char scelta[], char sx[], char dx[])
{

    static int flag = 0;
    disegnaSfondo(stringa);
    if(strcmp("tasti", scelta) == 0)
        coseCoiTasti();

    else if(strcmp("mouse", scelta) == 0)
        coseColMouse(cont);

    disegna();
    if(direzione == 1)
        draw_image(dx, x_personaggio, y_personaggio,grandezzaPersonaggio,grandezzaPersonaggio,255);
    if(direzione == -1)
        draw_image(sx, x_personaggio, y_personaggio,grandezzaPersonaggio,grandezzaPersonaggio,255);

    if(ms_time() - tempo >= velocitaP)
    {
        tempo = ms_time();
        y_personaggio -= Vy;
        Vy -= decelerazione;
    }

    if(ms_time() - tempo >= velocitaBlocchi2)
    {
        for(int i = 0; i < numeroblocchi; i++)
            switch(blocco[i].tipoBlocco)
            {
            case 2:
            {
                if(-blocco[i].X_iniziale + blocco[i].X >= quantoSiMuovonoBlocchi2)
                    blocco[i].direzione = -1;
                else if (blocco[i].X < blocco[i].X_iniziale)
                    blocco[i].direzione = 1;
                blocco[i].X += blocco[i].direzione;
                if(blocco[i].coso.flag)
                    blocco[i].coso.X+= blocco[i].direzione;
                break;
            }
            default:
                break;
            }
    }

    for(int i = 0; i < numeroblocchi; i++)
        if(blocco[i].flag == 1 && abs(blocco[i].Y - y_personaggio - grandezzaPersonaggio + A) < e && x_personaggio + grandezzaPersonaggio  >= blocco[i].X && x_personaggio < blocco[i].X + blocco[i].larghezza && blocco[i].Y < finestray && blocco[i].Y > 0 && Vy <= 0)
        {
            if(blocco[i].tipoBlocco != 1)
            {
                Vy = VyIniziale;
                ultimoTocco = i;
                if(blocco[i].toccato == 0)      //Aumento di punti solo se è la prima volta che tocco un blocco, altrimenti faccio punti restando fermo
                    punteggio += 10, blocco[i].toccato = 1;
            }
            if(blocco[i].tipoBlocco == 3 || blocco[i].tipoBlocco == 1)
                blocco[i].flag = 0;
            if(blocco[i].coso.flag && x_personaggio + grandezzaPersonaggio >= blocco[i].coso.X && x_personaggio < blocco[i].coso.X
                    + EsempiOggetti[blocco[i].coso.tipo].larghezza)            // Se esiste un oggetto lo prendo e succede qualcosa nel gioco
            {
                blocco[i].coso.flag = 0;
                switch(blocco[i].coso.tipo)
                {
                case 0:
                    punteggio += 20;
                    break;
                case 1:
                    spostaSchermo(10000);
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
        x_personaggio = finestrax - grandezzaPersonaggio;

    if(ultimoTocco == 1)
        vittoria = 1;

    if(y_personaggio > finestray && vite > 0)
    {
    if(blocco[ultimoTocco].Y > finestray)
        ultimoTocco--;
        x_personaggio = blocco[ultimoTocco].X + 10;
        y_personaggio = blocco[ultimoTocco].Y - grandezzaPersonaggio;
        vite--;
        Vite[vite] = 0;
    }

    if(y_personaggio > finestray)
        vittoria = -1;

    if(y_personaggio < finestray/2)
        spostaSchermo(1);

    if(is_pressed(VSGL_S) && flag == 0)
    {
        flag = 1;
        quantiProiettili++;
        //cout << quantiProiettili << endl;
        proiettile[quantiProiettili%memoriaProiettili].X = x_personaggio;
        proiettile[quantiProiettili%memoriaProiettili].Y = y_personaggio;
        //cout << "premuto" << endl;
    }

    if(!is_pressed(VSGL_S) && flag == 1)
        flag = 0;

    for(int i = 0; i < memoriaProiettili; i++)
    {
        proiettile[i].Y-=2;
        proiettile[i].X--;
        for(int j = 0; j < quantiMostriNelGioco; j++)
        {
            if(proiettile[i].Y > 0 && proiettile[i].Y < finestray && mostro[j].Y + EsempiMostri[mostro[j].tipo].altezza > 0 && mostro[j].Y < finestray && proiettile[i].Y > mostro[j].Y && proiettile[i].Y < mostro[j].Y + EsempiMostri[mostro[j].tipo].altezza
                    && proiettile[i].X > mostro[j].X && proiettile[i].X < mostro[j].X + EsempiMostri[mostro[j].tipo].larghezza)
            {
                //cout << "vite: " << mostro[j].vite << endl;
                mostro[j].vite--;
                punteggio += 200;
                proiettile[i].Y = -1;
            }

            if(vite > 0 && mostro[j].vite > 0 && y_personaggio > mostro[j].Y && y_personaggio < mostro[j].Y + EsempiMostri[mostro[j].tipo].altezza
                    && x_personaggio + grandezzaPersonaggio > mostro[j].X && x_personaggio < mostro[j].X + EsempiMostri[mostro[j].tipo].larghezza)
            {
                x_personaggio = blocco[ultimoTocco - 1].X + 10;
                y_personaggio = blocco[ultimoTocco - 1].Y - grandezzaPersonaggio;
                vite--;
                Vite[vite] = 0;
            }

            if(mostro[j].vite > 0 && y_personaggio > mostro[j].Y && y_personaggio < mostro[j].Y + EsempiMostri[mostro[j].tipo].altezza
                    && x_personaggio + grandezzaPersonaggio > mostro[j].X && x_personaggio < mostro[j].X + EsempiMostri[mostro[j].tipo].larghezza)
            {
                vittoria = -1;
            }
        }
    }


    cont++;
    ShowScore();
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
    It reads on a file top5 gamers, the it push them in specific structures
    and closes the file
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

    \brief It shows high scores

*/


void ShowHighScore()
{
    const int larghezzaBottone = 300;
    sempreperipunteggi();
    while(1)
    {
        draw_image("Images/Untitled2.png",0,0,finestrax,finestray,255);

        int a = get_mouse_x();
        int b = get_mouse_y();
        const int margine = 60;
        const int distacco = 30;
        const int numerobottoni = quantiPunteggi;
        const int inizioBottoni = (finestray - numerobottoni*altezzaBottone - (numerobottoni-1) * distacco) / 2;
        int alt = text_height("Font/w.ttf", 70, "High scores");
        int lungh = text_width("Font/w.ttf", 70, "High scores");
        draw_text("Font/w.ttf", 70, "High scores", margine + (larghezzaBottone + distacco/2 + altezzaBottone - lungh)/2, (inizioBottoni - alt)/2, Color(192,86,243,255));
        for(int i = 0; i < numerobottoni; i++)
            {
                draw_filled_rect(margine, inizioBottoni + (altezzaBottone + distacco)*i,larghezzaBottone,altezzaBottone,Color(255,255,255,255));
                draw_filled_rect(margine + distacco/2 + larghezzaBottone, inizioBottoni + (altezzaBottone + distacco)*i,altezzaBottone,altezzaBottone,Color(255,255,255,255));
            }
        for(int i = 1; i <= numerobottoni; i++)
        {
            // Qui ci metto sotto la data
            //if(mouse_left_button_pressed() && a >= margine && a <= larghezzaBottone + margine && b >= inizioBottoni + (i-1)*(distacco+altezzaBottone) && b <= inizioBottoni + i*altezzaBottone + (i-1)* distacco)
            //  cout << i << endl;

            if(!mouse_left_button_pressed() && a >= margine && a <= larghezzaBottone + margine && b >= inizioBottoni + (i-1)*(distacco+altezzaBottone) && b <= inizioBottoni + i*altezzaBottone + (i-1)* distacco)
                draw_filled_rect(margine,inizioBottoni + (altezzaBottone + distacco)*(i-1),larghezzaBottone,altezzaBottone,Color(253,225,127,255));

            if(!mouse_left_button_pressed() && a >= margine + larghezzaBottone + distacco/2 && a <= margine + larghezzaBottone + distacco/2 + altezzaBottone && b >= inizioBottoni + (i-1)*(distacco+altezzaBottone) && b <= inizioBottoni + i*altezzaBottone + (i-1)* distacco)
                draw_filled_rect(margine + distacco/2 + larghezzaBottone,inizioBottoni + (altezzaBottone + distacco)*(i-1),altezzaBottone,altezzaBottone,Color(253,225,127,255));

        }


        for(int i = 0; i < quantiPunteggi; i++)
        {
            int larghezza = text_width("Font/OpenSans-Regular.ttf", 20, top[i].nome);
            int altezza = text_height("Font/OpenSans-Regular.ttf", 20, top[i].nome);
            draw_text("Font/OpenSans-Regular.ttf", 20, top[i].nome, margine + (larghezzaBottone - larghezza)/2, inizioBottoni + i*altezzaBottone + i*distacco + (altezzaBottone - altezza)/2, Color(192,86,243,255));
            char punteggio[100];
            int a = sprintf(punteggio, "%d", top[i].punteggio);
            larghezza =text_width("Font/OpenSans-Regular.ttf", 20, punteggio);
            altezza = text_height("Font/OpenSans-Regular.ttf", 20, punteggio);
            draw_text("Font/OpenSans-Regular.ttf", 20, punteggio, margine + distacco/2 + larghezzaBottone + (altezzaBottone - larghezza)/2, inizioBottoni + i*altezzaBottone + i*distacco + (altezzaBottone - altezza)/2, Color(192,86,243,255));
        }

        draw_image("Images/bottone.png", bottoneX, bottoneY, bottone, bottone);
        if(mouse_left_button_pressed() && a >= bottoneX && a <= bottoneX + bottone && b >= bottoneY && b <= bottoneY + bottone)
            break;

        update();
    }
}

/**

    \brief It updates the file with best scores
*/

void score()
{
    sempreperipunteggi();
    cout << "Bravo! Inserisci il tuo nome" << endl;
    cin >> top[quantiPunteggi].nome;
    char temp[100];
    strcpy(temp,top[quantiPunteggi].nome);
    top[quantiPunteggi].punteggio = punteggio;
    sort(top,top+quantiPunteggi+1);
    //if (!(top[quantiPunteggi].punteggio == punteggio && strcmp(top[quantiPunteggi].nome,temp)==0))
        //cout << "Sei nella top " << quantiPunteggi << "!" << endl;
    ofstream out;
    out.open("IO_Files/Vincitori.txt");
    for(int i = 0; i < quantiPunteggi; i++)
        out << top[i].nome << " " << top[i].punteggio << endl;
    out.close();
    delay(1000);
    ShowHighScore();
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
    \brief It draws menu
    \return Case 1: the game starts
            Case 2: instructions
            Case 3: settings
*/


int menu()
{
    draw_image("Images/Untitled.png",0,0,finestrax,finestray,255);
    int a = get_mouse_x();
    int b = get_mouse_y();
    const int margine = 60;
    const int distacco = 30;
    const int numerobottoni = 4;
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

    int larghezza = text_width("Font/OpenSans-Regular.ttf", 20, "Gioca!");
    int altezza = text_height("Font/OpenSans-Regular.ttf", 20, "Gioca!");
    draw_text("Font/OpenSans-Regular.ttf", 20, "Gioca!", margine + (larghezzaBottone - larghezza)/2, inizioBottoni + (altezzaBottone - altezza)/2, Color(192,86,243,255));

    larghezza = text_width("Font/OpenSans-Regular.ttf", 20, "Istruzioni");
    altezza = text_height("Font/OpenSans-Regular.ttf", 20, "Istruzioni");
    draw_text("Font/OpenSans-Regular.ttf", 20, "Istruzioni", margine + (larghezzaBottone - larghezza)/2, inizioBottoni + altezzaBottone + distacco + (altezzaBottone - altezza)/2, Color(192,86,243,255));

    larghezza = text_width("Font/OpenSans-Regular.ttf", 20, "Impostazioni");
    altezza = text_height("Font/OpenSans-Regular.ttf", 20, "Impostazioni");
    draw_text("Font/OpenSans-Regular.ttf", 20, "Impostazioni", margine + (larghezzaBottone - larghezza)/2, inizioBottoni + 2*altezzaBottone + 2*distacco + (altezzaBottone - altezza)/2, Color(192,86,243,255));

    larghezza = text_width("Font/OpenSans-Regular.ttf", 20, "High scores");
    altezza = text_height("Font/OpenSans-Regular.ttf", 20, "High scores");
    draw_text("Font/OpenSans-Regular.ttf", 20, "High scores", margine + (larghezzaBottone - larghezza)/2, inizioBottoni + 3*altezzaBottone + 3*distacco + (altezzaBottone - altezza)/2, Color(192,86,243,255));


    return 0;
}

/**
    It changes the settings, like the charachter you are using, the background or how
    you control the unicorn
*/

void settings(char sinistra[], char destra[], char sfondo[], char scelta[])
{
    int tempoT = ms_time();
    const int pochissimo = 1000;
    while(1)
    {
        draw_image("Images/Settings.png", 0, 0, finestrax, finestray, 255);

        const int margineY = 130;
        const int distanza = 50;
        const int spessore = 2;
        const int margineX = (finestrax - grandezzaPersonaggio*quantiPersonaggi - distanza * (quantiPersonaggi - 1)) / 2;

        draw_image("Images/unicorno1.png",margineX, margineY, grandezzaPersonaggio, grandezzaPersonaggio);
        draw_image("Images/unicorno3.png",2*margineX + distanza, margineY, grandezzaPersonaggio, grandezzaPersonaggio);
        draw_image("Images/unicorno5.png",3*margineX + distanza*2, margineY, grandezzaPersonaggio, grandezzaPersonaggio);
        draw_image("Images/sfondo1_copia.jpg", margineX, margineY*2 +grandezzaPersonaggio, grandezzaPersonaggio, grandezzaPersonaggio);
        draw_image("Images/sfondo2_copia.jpg",2*margineX + distanza, margineY*2 + grandezzaPersonaggio, grandezzaPersonaggio, grandezzaPersonaggio);
        draw_image("Images/sfondo3_copia.jpg",3*margineX + distanza*2, margineY*2 + grandezzaPersonaggio, grandezzaPersonaggio, grandezzaPersonaggio);
        draw_image("Images/bottone.png", bottoneX, bottoneY, bottone, bottone);
        draw_filled_rect(margineX, margineY*3 + grandezzaPersonaggio*2, larghezzaBottone, altezzaBottone, Color(255,255,255,255));
        draw_filled_rect(2*margineX + larghezzaBottone, margineY*3 + grandezzaPersonaggio*2, larghezzaBottone, altezzaBottone, Color(255,255,255,255));
        draw_image("Images/mouse.png",margineX, margineY*3 + grandezzaPersonaggio*2, larghezzaBottone, altezzaBottone);
        draw_image("Images/Tastiera.png",2*margineX + larghezzaBottone, margineY*3 + grandezzaPersonaggio*2, larghezzaBottone, altezzaBottone);



        if(strcmp(sinistra, "Images/unicorno1.png") == 0)
            draw_rect(margineX - spessore, margineY - spessore, grandezzaPersonaggio + spessore*2, grandezzaPersonaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sinistra, "Images/unicorno3.png") == 0)
            draw_rect(2*margineX + distanza - spessore, margineY - spessore, grandezzaPersonaggio + spessore*2, grandezzaPersonaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sinistra, "Images/unicorno5.png") == 0)
            draw_rect(3*margineX + distanza*2 - spessore, margineY - spessore, grandezzaPersonaggio + spessore*2, grandezzaPersonaggio + spessore*2, Color(0,0,0,255));


        if(strcmp(sfondo, "Images/sfondo1.jpg") == 0)
            draw_rect(margineX - spessore, margineY*2 + grandezzaPersonaggio - spessore, grandezzaPersonaggio + spessore*2, grandezzaPersonaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sfondo, "Images/sfondo2.jpg") == 0)
            draw_rect(2*margineX + distanza - spessore, margineY*2 + grandezzaPersonaggio - spessore, grandezzaPersonaggio + spessore*2, grandezzaPersonaggio + spessore*2, Color(0,0,0,255));
        if(strcmp(sfondo, "Images/sfondo3.jpg") == 0)
            draw_rect(3*margineX + distanza*2 - spessore, margineY*2 + grandezzaPersonaggio - spessore, grandezzaPersonaggio + spessore*2, grandezzaPersonaggio + spessore*2, Color(0,0,0,255));




        if(strcmp(scelta, "mouse") == 0)
            draw_rect(margineX - spessore, margineY*3 + grandezzaPersonaggio*2 - spessore, larghezzaBottone + spessore*2, altezzaBottone + spessore*2, Color(0,0,0,255));
        if(strcmp(scelta, "tasti") == 0)
            draw_rect(2*margineX + larghezzaBottone - spessore, margineY*3 + grandezzaPersonaggio*2 - spessore, larghezzaBottone + spessore*2, altezzaBottone + spessore*2, Color(0,0,0,255));

        int a = get_mouse_x();
        int b = get_mouse_y();

        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= margineX
                && a <= margineX + grandezzaPersonaggio && b >= margineY && b <= margineY + grandezzaPersonaggio)
            strcpy(sinistra, "Images/unicorno1.png"), strcpy(destra, "Images/unicorno2.png");

        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= 2*margineX + distanza
                && a <= 2*margineX + distanza + grandezzaPersonaggio && b >= margineY && b <= margineY + grandezzaPersonaggio)
            strcpy(sinistra, "Images/unicorno3.png"), strcpy(destra, "Images/unicorno4.png");

        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= 3*margineX + distanza*2
                && a <= 3*margineX + distanza*2 + grandezzaPersonaggio && b >= margineY && b <= margineY + grandezzaPersonaggio)
            strcpy(sinistra, "Images/unicorno5.png"), strcpy(destra, "Images/unicorno6.png");



        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= margineX
                && a <= margineX + grandezzaPersonaggio && b >= margineY*2 + grandezzaPersonaggio && b <= margineY*2 + grandezzaPersonaggio*2)
            strcpy(sfondo, "Images/sfondo1.jpg");

        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= 2*margineX + distanza
                && a <= 2*margineX + distanza + grandezzaPersonaggio && b >= margineY*2 + grandezzaPersonaggio && b <= margineY*2 + grandezzaPersonaggio*2)
            strcpy(sfondo, "Images/sfondo2.jpg");

        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= 3*margineX + distanza*2
                && a <= 3*margineX + distanza*2 + grandezzaPersonaggio && b >= margineY*2 + grandezzaPersonaggio && b <= margineY*2 + grandezzaPersonaggio*2)
            strcpy(sfondo, "Images/sfondo3.jpg");


        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= margineX
                && a <= margineX + larghezzaBottone && b >=  margineY*3 + grandezzaPersonaggio*2 && b <=  margineY*3 + grandezzaPersonaggio*2 + altezzaBottone)
            strcpy(scelta, "mouse");

        if(ms_time() - tempoT > pochissimo && mouse_left_button_pressed() && a >= 2*margineX + larghezzaBottone
                && a <= 2*margineX + larghezzaBottone*2 && b >=  margineY*3 + grandezzaPersonaggio*2 && b <=  margineY*3 + grandezzaPersonaggio*2 + altezzaBottone)
            strcpy(scelta, "tasti");

        if(mouse_left_button_pressed() && a >= bottoneX && a <= bottoneX + bottone && b >= bottoneY && b <= bottoneY + bottone)
            break;

        update();
    }

}

/**
    \brief It calls other function to initialize the game
*/

void coseDaFarePrimaDiIniziare()
{
    creaCoseCasuali();
    inizializzaTutto();
    oggetti();
}


int main(int argc, char* argv[])
{
    srand(time(NULL));
    eratostene();
    char sfondo[1000] = "Images/sfondo1.jpg";
    char personaggioSx[1000] = "Images/unicorno5.png";
    char personaggioDx[1000] = "Images/unicorno6.png";
    char scelta[100] = "tasti";
    int cosafare = 0;
    lastSettings(personaggioSx, personaggioDx, sfondo, scelta);
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
            coseDaFarePrimaDiIniziare();
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

        case 2:                                         //INSTRUCTIONS
        {
            instructions();
            break;
        }

        case 3:                                         //SETTINGS
        {
            //cout << sfondo << endl;
            settings(personaggioSx, personaggioDx, sfondo, scelta);
            updtateSettings(personaggioSx, personaggioDx, sfondo, scelta);
            break;
        }
        case 4:
        {
            ShowHighScore();
            //cout << "Stiamo lavorando per voi!" << endl;
            break;
        }
        }
    }
    close();
    return 0;
}
