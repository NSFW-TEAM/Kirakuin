#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "nodo.h"
#include <stdlib.h>
#include <assert.h>
#include "funciones.h"
#include <AL/alut.h>
#include <stdbool.h>
#define ENTER 10
#define MAXIMO 99999999
#define MINIMO 11111111
#define ESPACIO 20
typedef struct level level;

void nueva_partida(void);
ALuint source,buffer;
struct level{
    level *izq;
    level *der;
    level *up;
    level *dwn;
    int recorrido;
    int enemigo;
    int cofre;
    int id;
};

typedef struct mapa{
    level * root;
    level  * current;
}Mapa;

typedef struct jugador{
    int dfs;
    int atk;
    char * nombre;
    int level;
}jugador;

typedef struct enemigo{
    int prt;
    int atk;
    char *nombre;
}enemigo;

level * create_node(int id) {
    level *new = (level * )malloc (sizeof (level));
    assert (new != NULL); // No hay memoria para reservar la Lista.
    new->izq=NULL;
    new->der=NULL;
    new->up=NULL;
    new->dwn=NULL;
    new->enemigo=1;
    new->recorrido=0;
    new->cofre=0;
    new->id=id;
    return new;
}

char * _strdup (const char *s) {

	size_t len = strlen (s) + 1;
    void *new = malloc (len);

	if (new == NULL)
	    return NULL;

	return (char *) memcpy (new, s, len);
}

const char *get_csv_field (char * tmp, int i) {

    char * line = _strdup (tmp);
    const char * tok;
    for (tok = strtok (line, ","); tok && *tok; tok = strtok (NULL, ",\n")) {
        if (!--i) {
            return tok;
        }
    }
    return NULL;
}

void print_player(WINDOW* ventana,int y,int x, int dir){
    if(dir==0){
    mvwprintw(ventana,y,x," ,,, ");
    mvwprintw(ventana,y+1,x,"('' )");
    mvwprintw(ventana,y+2,x,"[)( ]");
    mvwprintw(ventana,y+3,x," ||| ");
    mvwprintw(ventana,y+4,x," U-U ");
    }
    if(dir==1){
    mvwprintw(ventana,y,x," ,,, ");
    mvwprintw(ventana,y+1,x,"(' ')");
    mvwprintw(ventana,y+2,x,"[ X ]");
    mvwprintw(ventana,y+3,x," ||| ");
    mvwprintw(ventana,y+4,x," U-U ");
    }
    if(dir==2){
    mvwprintw(ventana,y,x," ,,, ");
    mvwprintw(ventana,y+1,x,"( '')");
    mvwprintw(ventana,y+2,x,"[ )(]");
    mvwprintw(ventana,y+3,x," ||| ");
    mvwprintw(ventana,y+4,x," U-U ");
    }
    if(dir==3){
    mvwprintw(ventana,y,x," ,,, ");
    mvwprintw(ventana,y+1,x,"(   )");
    mvwprintw(ventana,y+2,x,"[   ]");
    mvwprintw(ventana,y+3,x," ||| ");
    mvwprintw(ventana,y+4,x," U-U ");
    }
}

void vanish_player(WINDOW* ventana, int y, int x){
    mvwprintw(ventana,y,x,"     ");
    mvwprintw(ventana,y+1,x,"     ");
    mvwprintw(ventana,y+2,x,"     ");
    mvwprintw(ventana,y+3,x,"     ");
    mvwprintw(ventana,y+4,x,"     ");
}

void generate_map_type(WINDOW* ventana,int type){
    int i,j;
    if(type==0){ //tipo de nivel inicial
        for(i=1;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
        for(i=1;i<=106;i+=2){
            for(j=1;j<21;j++){
                if(j==5){
                    j=16;
                }else{
                    mvwprintw(ventana,j,i,"/\\");
                }
            }
        }
        for(i=5;i<=16;i++){
            mvwprintw(ventana,i,1,"/\\/\\/@");
        }
    }
    if(type==1){ //tipo de nivel 1
        for(i=1;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
        for(i=1;i<=106;i+=2){
            for(j=1;j<21;j++){
                if(j==5){
                    j=16;
                }else{
                    mvwprintw(ventana,j,i,"/\\");
                }
            }
        }
        for(i=60;i<=106;i++){
            for(j=1;j<=21;j++){
                mvwprintw(ventana,j,i," ");
            }
        }
        for(i=1;i<=21;i++){
            mvwprintw(ventana,i,81,"@");
        }
        for(i=1;i<=21;i++){
            for(j=82;j<=106;j+=2){
                mvwprintw(ventana,i,j,"/\\");
            }
        }
        for(i=1;i<=21;i++){
            if(i==5){
                i=16;
            }else{
                mvwprintw(ventana,i,1,"/\\");
            }
        }
        for(i=1;i<=21;i++){
            if(i==6){
                i=15;
            }else{
                mvwprintw(ventana,i,60,"@");
            }
        }
    }
    if(type==2){ //tipo de nivel 2
        for(i=1;i<=21;i++){
            for(j=1;j<=105;j+=2){
                if(i==5){
                    i=16;
                }else{
                    mvwprintw(ventana,i,j,"/\\");
                }
            }
        }
        for(i=46;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=46;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
        for(i=1;i<=21;i++){
            if(i==5){
                i=16;
            }else{
                mvwprintw(ventana,i,46,"@");
            }
        }
        for(i=1;i<=21;i++){
            mvwprintw(ventana,i,26,"@");
        }
        for(i=1;i<=21;i++){
            for(j=27;j<=45;j++){
                mvwprintw(ventana,i,j," ");
            }
        }
        for(i=1;i<=21;i++){
            mvwprintw(ventana,i,1,"/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/");
        }
    }
    if(type==3){ //tipo de nivel 3
        for(i=1;i<=21;i++){
            for(j=1;j<=105;j+=2){
                if(i==6){
                    i=16;
                }else{
                    mvwprintw(ventana,i,j,"/\\");
                }
            }
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=1;i<=5;i++){
            mvwprintw(ventana,i,42,"@");
        }
        for(i=1;i<=5;i++){
            mvwprintw(ventana,i,63,"@");
        }
        for(i=1;i<=5;i++){
            for(j=43;j<=62;j++){
                mvwprintw(ventana,i,j," ");
            }
        }
    }
    if(type==4){ //tipo de nivel 4
        for(i=1;i<=21;i++){
            for(j=1;j<=105;j+=2){
                if(i==6){
                    i=16;
                }else{
                    mvwprintw(ventana,i,j,"/\\");
                }
            }
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
        for(i=16;i<=20;i++){
            mvwprintw(ventana,i,42,"@");
        }
        for(i=16;i<=20;i++){
            mvwprintw(ventana,i,63,"@");
        }
        for(i=16;i<=20;i++){
            for(j=43;j<=62;j++){
                mvwprintw(ventana,i,j," ");
            }
        }
    }
    if(type==5){ //tipo de nivel 5
        for(i=1;i<=21;i++){
            for(j=1;j<=105;j+=2){
                if(i==6){
                    i=16;
                }else{
                    mvwprintw(ventana,i,j,"/\\");
                }
            }
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
        for(i=16;i<=20;i++){
            mvwprintw(ventana,i,42,"@");
        }
        for(i=16;i<=20;i++){
            mvwprintw(ventana,i,63,"@");
        }
        for(i=1;i<=5;i++){
            mvwprintw(ventana,i,42,"@");
        }
        for(i=1;i<=5;i++){
            mvwprintw(ventana,i,63,"@");
        }
        for(i=1;i<=20;i++){
            for(j=43;j<=62;j++){
                mvwprintw(ventana,i,j," ");
            }
        }
    }
    if(type==6){
        for(i=1;i<=21;i++){
            for(j=1;j<=105;j+=2){
                if(i==6){
                    i=16;
                }else{
                    mvwprintw(ventana,i,j,"/\\");
                }
            }
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,5,i,"@");
        }
        for(i=1;i<=106;i++){
            mvwprintw(ventana,16,i,"@");
        }
    }
    if(type==7){
        for(i=1;i<=21;i++){
            for(j=1;j<=105;j+=2){
                mvwprintw(ventana,i,j,"/\\");
            }
        }
        for(i=1;i<=21;i++){
            mvwprintw(ventana,i,42,"@");
            mvwprintw(ventana,i,63,"@");
        }
        for(i=1;i<=21;i++){
            for(j=43;j<=62;j++){
                mvwprintw(ventana,i,j," ");
            }
        }
    }
    wrefresh(ventana);
    return;
}
//funcion de colisiones, type= tipo de mapa
void canmove(int type,int y,int x,int* left,int* right,int* up,int* down){
    if(type==0){
        if(x<102){
            *right = 1;
        }else{
            *right = 0;
        }
        if(x>7){
            *left = 1;
        }else{
            *left = 0;
        }
        if(y>6){
            *up = 1;
        }else{
            *up = 0;
        }
        if(y<11){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==1){
        if(x<76){
            *right = 1;
        }else{
            *right = 0;
        }
        if(((x>1) && (y>5 && y<12)) || (x>61)){
            *left = 1;
        }else{
            *left = 0;
        }
        if((x>60 && y>1) || (y>6 && y<12)) {
            *up = 1;
        }else{
            *up = 0;
        }
        if((x>60 && y<16) || (y>5 && y<11)){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==2){
        if((x<41) || (y>5 && y<12 && x<102)){
            *right = 1;
        }else{
            *right = 0;
        }
        if(x>27){
            *left = 1;
        }else{
            *left = 0;
        }
        if((x>26 && y>1 && x<42) || (y>6 && y<12)) {
            *up = 1;
        }else{
            *up = 0;
        }
        if((x>26 && y<16 && x<42) || (y>5 && y<11)){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==3){
        if((y>5 && y<16 && x<102) || (y<16 && x<58)){
            *right = 1;
        }else{
            *right = 0;
        }
        if((x>1 && y>5 && y<16)||(y<16 && x>43)){
            *left = 1;
        }else{
            *left = 0;
        }
        if((y>6) || (y>1 && x>42 && x<59)) {
            *up = 1;
        }else{
            *up = 0;
        }
        if(y<11){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==4){
        if((x>0 && x<102 && y>5 && y<12) || (x>42 && x<58 && y>11 && y<21)){
            *right = 1;
        }else{
            *right = 0;
        }
        if((x>1 && x<103 && y>5 && y<12)||(x>43 && x<59 && y>11 && y<21)){
            *left = 1;
        }else{
            *left = 0;
        }
        if(y>6) {
            *up = 1;
        }else{
            *up = 0;
        }
        if((x>0 && x<104 && y>5 && y < 11)||(x>42 && x<59 && y>10 && y<16)){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==5){
        if((x>42 && x<58 && y>0 && y<=5) || (x>0 && x<102 && y>5 && y<12)||(x>42 && x<58 && y>11 && y<17)){
            *right = 1;
        }else{
            *right = 0;
        }
        if((x>43 && x<59 && y>0 && y<=5) || (x>1 && x<103 && y>5 && y<12)||(x>43 && x<59 && y>11 && y<17)){
            *left = 1;
        }else{
            *left = 0;
        }
        if((x>42 && x<59 && y>1 && y<=6) || (x>0 && x<103 && y>6 && y<12)||(x>42 && x<59 && y>11 && y<17)){
            *up = 1;
        }else{
            *up = 0;
        }
        if((x>42 && x<59 && y>0 && y<=5) || (x>0 && x<103 && y>5 && y<11)||(x>42 && x<59 && y>10 && y<16)){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==6){
        if(x<102){
            *right = 1;
        }else{
            *right = 0;
        }
        if(x>1){
            *left = 1;
        }else{
            *left = 0;
        }
        if(y>6){
            *up = 1;
        }else{
            *up = 0;
        }
        if(y<11){
            *down = 1;
        }else{
            *down = 0;
        }
    }
    if(type==7){
        if(x<58){
            *right = 1;
        }else{
            *right = 0;
        }
        if(x>43){
            *left = 1;
        }else{
            *left = 0;
        }
        if(y>1){
            *up = 1;
        }else{
            *up = 0;
        }
        if(y<16){
            *down = 1;
        }else{
            *down = 0;
        }
    }
}

//Inicializa el gameplay del juego
void gameplay(){
    int player_x = 10;
    int player_y = 9;
    int key;
    int i;
    int canmoveleft,canmoveright,canmoveup,canmovedown;
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(11,COLOR_RED,COLOR_BLACK);
    bkgd(COLOR_PAIR(10));
    int yMax , xMax;
    getmaxyx(stdscr, yMax , xMax);
    WINDOW * menuwin = newwin(6, 108,24, 5);
    WINDOW * gamewin = newwin(22,108,1,5);//y,x,mover arriba,mover derecha
    wbkgd(gamewin,COLOR_PAIR(11));
    wbkgd(menuwin,COLOR_PAIR(11));
    generate_map_type(gamewin,3); //generar tipo de mapa
    box(menuwin,0,0);
    box(gamewin,0,0);
    print_player(gamewin,player_y,player_x,2);
    refresh();
    keypad(gamewin,true);

    while(1){
        wrefresh(gamewin);
        wrefresh(menuwin);
        canmove(3,player_y,player_x,&canmoveleft,&canmoveright,&canmoveup,&canmovedown);
        key = wgetch(gamewin);
        if(key==KEY_RIGHT){
            if(canmoveright==1){
                vanish_player(gamewin,player_y,player_x);
                player_x+=1;
                print_player(gamewin,player_y,player_x,2);
            }
        }
        if(key==KEY_LEFT){
            if(canmoveleft==1){
                vanish_player(gamewin,player_y,player_x);
                player_x-=1;
                print_player(gamewin,player_y,player_x,0);
            }
        }
        if(key==KEY_UP){
                if(canmoveup==1){
                    vanish_player(gamewin,player_y,player_x);
                    player_y-=1;
                    print_player(gamewin,player_y,player_x,3);
                }
        }
        if(key==KEY_DOWN){
                if(canmovedown==1){
                    vanish_player(gamewin,player_y,player_x);
                    player_y+=1;
                    print_player(gamewin,player_y,player_x,1);
                }
        }

    }
    wrefresh(gamewin);
    wrefresh(menuwin);
    keypad(menuwin,true);
    getch();
}

int transformarAEntero(char *charNumero){

    int largo = strlen(charNumero);
    int numero = 0;
    for(int i=0;i<largo;i++){
        numero=(charNumero[i]-48)+(numero*10);
    }
    return numero;
}

jugador *crearJugador(char *linea){

    jugador *j = (jugador *) malloc(sizeof(jugador)); // Asigna memoria a todo la struct

    j->nombre = get_csv_field(linea, 1);
    j->dfs = transformarAEntero(get_csv_field(linea,2));
    j->atk = transformarAEntero(get_csv_field(linea,3));
    j->level = transformarAEntero(get_csv_field(linea,4));
    return j;
}

void typewriter(int x, int y, int velocidad, WINDOW* ventana, char* text){
    int i;
    for(i=0;i<strlen(text);i++){
        mvprintw(x,y+i,"%c",text[i]);
        wrefresh(ventana);
        napms(velocidad);
    }
}

void showtitle(int x, int y, int velocidad){
    typewriter(y,x,velocidad,stdscr,"==================================================================================");
    typewriter(y+1,x,velocidad,stdscr,"-@@@---@@@@-@@@@@@-@@@@@@@-------@@@@----@@@---@@@@-@@@@----@@@@-@@@@@@-@@-----@@-");
    typewriter(y+2,x,velocidad,stdscr,"--@@--@@@-----@@----@@---@@-----@@--@@----@@--@@@----@@------@@----@@---@@@@---@@-");
    typewriter(y+3,x,velocidad,stdscr,"--@@-@@@------@@----@@---@@----@@----@@---@@-@@@-----@@------@@----@@---@@-@@--@@-");
    typewriter(y+4,x,velocidad,stdscr,"--@@@@@-------@@----@@@@@@-----@@@@@@@@---@@@@@------@@------@@----@@---@@--@@-@@-");
    typewriter(y+5,x,velocidad,stdscr,"--@@-@@@------@@----@@--@@-----@@----@@---@@-@@@-----@@------@@----@@---@@---@@@@-");
    typewriter(y+6,x,velocidad,stdscr,"--@@--@@@-----@@----@@---@@----@@----@@---@@--@@@----@@@----@@@----@@---@@----@@@-");
    typewriter(y+7,x,velocidad,stdscr,"-@@@---@@@@-@@@@@@--@@----@@@-@@------@@-@@@---@@@@----@@@@@@----@@@@@@-@@-----@@-");
    typewriter(y+8,x,velocidad,stdscr,"==================================================================================");
}

int menu_principal(){
    
    int key=KEY_LEFT; 
 
    noecho();
    cbreak();
    start_color();
    bkgd(COLOR_BLACK);
    int yMax , xMax;
    getmaxyx(stdscr, yMax , xMax);

    WINDOW * menuwin = newwin(6, xMax-12,yMax-8, 5);
    int yMac, xMac;
    getmaxyx(menuwin,yMac,xMac);
    
    showtitle(1,5,1);

    box(menuwin,0,0);
    refresh();
    wrefresh(menuwin);

    keypad(menuwin,true);

        int y =1; 
        start_color();
        init_pair(1,COLOR_WHITE,COLOR_GREEN);
        init_pair(2,COLOR_WHITE,COLOR_BLUE);
        attron(COLOR_PAIR(1));
        wmove(menuwin,1,1);
        wprintw(menuwin," Nueva partida");
        wmove(menuwin,2,1);
        wprintw(menuwin," Cargar Partida");
        wmove(menuwin,3,1);
        wprintw(menuwin," Ingresar cheats");
        attroff(COLOR_PAIR(1));
        wrefresh(menuwin);
        wmove(menuwin,1,20);
        noecho();
        curs_set(0); 
        mvwprintw(menuwin,y,1,">");
        
        do{
        	key = wgetch(menuwin);
        	wrefresh(menuwin);

        	if(key == KEY_UP){
                mvwprintw(menuwin,y,1," ");
                y--;
        		if(y==0)y=3;
                if(y==4)y=1;
                
                wmove(menuwin,y,1);
                mvwprintw(menuwin,y,1,">");
                   
                continue;
        	}
        	
        	if(key == KEY_DOWN){
        	  mvwprintw(menuwin,y,1," ");
                y++;
                if(y==0)y=3;
                if(y==4)y=1;
                wmove(menuwin,y,1);
                mvwprintw(menuwin,y,1,">");
                continue;
        	}

        }while(key != ENTER);
        
	    refresh();

    return y;
}

void ingresar(char *nombre){
    char *name=nombre;
    FILE *nuevoJugador = fopen("jugadores.txt", "a");
    fputs(nombre,nuevoJugador);
    fputs(",", nuevoJugador);
    fputs("100", nuevoJugador);
    fputs(",", nuevoJugador);
    fputs("100", nuevoJugador);
    fputs(",", nuevoJugador);
    fputs("1", nuevoJugador);
    fputs("\n", nuevoJugador);
    fclose(nuevoJugador);
}

/*void ingresar(char *nombre){
    char *name=nombre;
    FILE *nuevoJugador = fopen("jugadores.txt", "a");
    fputs(nombre,nuevoJugador);
    fputc(',', nuevoJugador);
    fputs("100", nuevoJugador);
    fputc(',', nuevoJugador);
    fputs("100", nuevoJugador);
    fputc(',', nuevoJugador);
    fputs("1", nuevoJugador);
    fputs("\n", nuevoJugador);
    fclose(nuevoJugador);
}*/

void borrarPantalla(){

//typewriter(9,23,50,stdscr,"Cual es tu nombre? ");
typewriter(9,23,50,stdscr,"                   ");
typewriter(10,23,50,stdscr,"                        ");
//typewriter(11,23,50,stdscr,"tu nombre es: ");
typewriter(11,23,100,stdscr,"                     ");
    //typewriter(11,37,50,stdscr,cadena);
    //typewriter(11,23,50,stdscr,"");
    //typewriter(12,23,50,stdscr,"estas seguro?");
typewriter(12,23,50,stdscr,"             ");
typewriter(14,23,75,stdscr,"     ");
    //typewriter(14,26,50,stdscr,"NO");
typewriter(15,23,75,stdscr,"     ");

typewriter(15,23,75,stdscr,"     ");

typewriter(15,26,75,stdscr,"     ");

typewriter(15,26,50,stdscr,"                   ");
typewriter(16,10,25,stdscr,"                ");
typewriter(16,40,25,stdscr,"                        ");
typewriter(17,17,25,stdscr,"       ");
refresh();
}

void historia(int key){
    keypad(stdscr,true);
    
    do{
        typewriter(1,1,50,stdscr,"esta historia es una mierda");
        typewriter(2,1,50,stdscr,"dudfdshfd");
        typewriter(3,1,50,stdscr,"sample text");
        typewriter(4,1,50,stdscr,"dudfdshfsdasdasd");
        typewriter(5,1,50,stdscr,"esta historia ea");
        typewriter(6,1,50,stdscr,"dudfdshsdsadusudsfd");
        typewriter(7,1,50,stdscr,"esta his una mierda");
        typewriter(8,1,50,stdscr,"dudfdshfd");
        typewriter(13,1,50,stdscr,"termino");
        napms(1000);
        return;
    }while((key=getch())!= ESPACIO);

}

jugador *cargarJugador(char *nombre){

    jugador *j;

    FILE *cargar= fopen("jugadores.txt", "r");
    if(cargar == NULL)
    {
        printf("NO SE PUDO ABRIR EL ARCHIVO");
    }

    char linea[101];

    while(fgets(linea, 100, cargar) != NULL)
    {
        j = crearJugador(linea);
        if(strcmp(j->nombre,nombre) == 0){
            fclose(cargar);
            return j;
        }
    }

    return NULL;
}

void no_elegible(){//Borra el espacio necesario en el menu principal
    typewriter(31,97,5,stdscr,"                                  ");
    typewriter(32,97,5,stdscr,"                                  ");
    typewriter(33,97,5,stdscr,"                                  ");
    typewriter(34,97,5,stdscr,"                                  ");
}

void mostrar(){
    FILE * archivo = fopen("jugadores.txt","r");

    if(archivo == NULL){
        wprintw(stdscr,"ERROR");
    }

    mvwprintw(stdscr,32,6," ");
    int completo=0;
    int y=31;
    jugador *p;
    char linea[101];
    int acum=0;
    char** vector[4];
    while(fgets(linea,100,archivo)!=NULL){
    
        if(acum==4){
            fclose(archivo);
            getch();
            completo=1;//Para no ingresar a la condicion de mas abajo
        }

        p = crearJugador(linea);
        vector[acum]=p->nombre;
        acum++;
        typewriter(y,100,50,stdscr,p->nombre);
        y++;
    }

    if(completo==0){//Si no se encontraron 4 nombres se llenan con texto vacio
        while(acum<4){
            vector[acum] = "vacio";
            typewriter(y,100,50,stdscr,vector[acum]);
            acum++;
            y++;
        }
    }

    int key=KEY_LEFT;
    typewriter(29,97,50,stdscr,"Presiona espacio para volver");
    y=31;
    bool termino=false;    
    
    keypad(stdscr,true);
    
    while(termino==false){  
    y=31;
    mvwprintw(stdscr,31,97,">"); 
    keypad(stdscr,true);
    do{
        key = getch();
       	refresh();

        if(key == KEY_UP){
            mvwprintw(stdscr,y,97," ");
            
            y--;
        	if(y==30)y=34;
            if(y==35)y=31;
            
            wmove(stdscr,y,97);
            mvwprintw(stdscr,y,97,">");
            continue;
        }
        	
        if(key == KEY_DOWN){
       	    mvwprintw(stdscr,y,97," ");
            y++;
            if(y==30)y=34;
            if(y==35)y=31;
            wmove(stdscr,y,97);
            mvwprintw(stdscr,y,97,">");
            continue;
        }

        if(key == ' '){
            clear();
            main();   
        }

    }while(key != ENTER);

    if(vector[y-31] != "vacio"){
        jugador* player= cargarJugador(vector[y-31]);
        no_elegible();
        mvwprintw(stdscr,31,97,player->nombre);
        mvwprintw(stdscr,32,97,"join the party :)");
        refresh();
        napms(500);
        mvwprintw(stdscr,32,97,"join the party ;)");
        refresh();
        napms(500);
        mvwprintw(stdscr,32,97,"join the party :)");
        refresh();
        napms(500);
        termino=true;
    }
    else{
        
        no_elegible();
        mvwprintw(stdscr,32,97,"No elegible :(");
        refresh();
        napms(1000);
        typewriter(33,97,50,stdscr,"Quieres crear una nueva partida?");
        typewriter(34,97,50,stdscr,"SI  NO");
        mvwprintw(stdscr,34,96,">");
        int key2=566;
        int i=97;
        do{   
            key2 = getch();
       	    refresh();

            if(key2 == KEY_LEFT){//SII
                if(i==97)continue;
                i=97;

                mvwprintw(stdscr,34,99,"  ");
                mvwprintw(stdscr,34,96,">");
                
        	    continue;

            }
        	
            if(key2 == KEY_RIGHT){//NO

                if(i==100)continue;
                i=100;

                mvwprintw(stdscr,34,96," ");
                mvwprintw(stdscr,34,99," >");
        	    continue;
            } 

            if(key2==' '){
                clear();
                main();
            }

        }while(key2!=ENTER);

        if(i==97){
            clear();
            bkgd(COLOR_PAIR(4));
            nueva_partida();
        }

        no_elegible();
        int n=0;
        do{

            typewriter(31+n,100,50,stdscr,vector[n]);
            n++;
        }while(n<4);
        //mvwprintw(stdscr,34,97,">");
        //mvwprintw(stdscr,37,97," ");

    }
    
    }

    gameplay();


}

void main(){
    initscr();
    
    int option = menu_principal();
     
    init_pair(4,COLOR_WHITE,COLOR_MAGENTA);
 
    switch (option)
    {
    case 1:
        clear();
        bkgd(COLOR_PAIR(4));
        nueva_partida();
        //typewriter(10,10,50,stdscr,"Nueva Partida");
        /* code */
        break;
    case 2:
        //typewriter(10,10,50,stdscr,"Cargar partida");
        mostrar();
        
        break;
    case 3:
        typewriter(10,10,50,stdscr,"CHEATS");
        break;
    default:
        return;
    }
    
    napms(2000);
    refresh();
    endwin();
    exit(main);
}

void nueva_partida(){
    
    echo();
    char cadena[128];
    typewriter(9,23,50,stdscr,"Cual es tu nombre? ");
    move(10,23);
    scanw("%[^\n]s",cadena);
    typewriter(11,23,50,stdscr,"tu nombre es:");
    typewriter(11,36,50,stdscr,cadena);
    typewriter(12,23,50,stdscr,"estas seguro?");
    typewriter(14,23,50,stdscr,"SI");
    typewriter(14,26,50,stdscr,"NO");
    refresh();
    int key=56;
    int x = 56;
    noecho();
    keypad(stdscr,true);
	mvwprintw(stdscr,15,23,"^");

    do{
            
        key = getch();
        refresh();
        	
        if(key == KEY_LEFT){//SI
            if(x==1)continue;
            x=1;
            mvwprintw(stdscr,15,26," ");
            mvwprintw(stdscr,15,23,"^");
            continue;
        }
        	
        if(key == KEY_RIGHT){//NO
           
            if(x==3)continue;
            x=3;
            mvwprintw(stdscr,15,23," ");
            mvwprintw(stdscr,15,26,"^");
            continue;
        }

    }while(key != ENTER);

    key=839;//inicializar en un valor aleatorio para posteriories bugs
	
    if(x==3){//Volver a intentar o volver a el menu principal
        x=4;
        mvwprintw(stdscr,15,26," ");
        mvwprintw(stdscr,16,40,"volver al menu principal");
        mvwprintw(stdscr,16,10,"volver a intentarlo");
        mvwprintw(stdscr,17,17,"^");
        
        do{
            key = getch();
            refresh();
        	
            if(key == KEY_LEFT){
                if(x==4)continue;
                x=4;    
                mvwprintw(stdscr,17,52," ");
            
                mvwprintw(stdscr,17,17,"^");
                continue;
            }
        	
            if(key == KEY_RIGHT){
           
                if(x==2)continue;
            
                x=2;//volver al menu
                mvwprintw(stdscr,17,17," ");
                mvwprintw(stdscr,17,52,"^");
                continue;
            }
        
        }while(key !=ENTER && x != 58);
        
        if(x==2){
            clear();
            return main();
        }

        if(x==4){
            clear();
            bkgd(COLOR_PAIR(4));
            return nueva_partida();
        }
        
    }

    ingresar(cadena);
    //jugador *player = crearJugador(cadena);
    jugador *player = cargarJugador(cadena);
    borrarPantalla();
    historia(key);
    clear();
    gameplay();
}
