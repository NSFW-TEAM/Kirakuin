#include <curses.h>
#include <string.h>
#include <stdio.h>
#include "nodo.h"
#include <stdlib.h>
#include <assert.h>
#include "funciones.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdbool.h>
#define ENTER 10
#define MAXIMO 99999999
#define MINIMO 11111111
#define ESPACIO 20
#define ESC 27
typedef struct level level;
typedef struct list list;
void nueva_partida(void);

typedef struct enemigo{
    int dfs;
    int atk;
}enemigo;

struct level{
    level *izq;
    level *der;
    level *up;
    level *dwn;
    int recorrido;
    enemigo* rival;
    bool cofre;
    int id;
};

typedef struct mapa{
    level * root;
    level  * current;
    list * camino;
}Mapa;

typedef struct jugador{
    int dfs;
    int atk;
    char * nombre;
    int nivel;
}jugador;

typedef struct list{
    list * current;
    list * next;
    list * prev;
}list;

enemigo* create_enemy(int type){

enemigo* new_enemy= (enemigo*) malloc(sizeof(enemigo));

if(type==5){

    new_enemy->dfs=10;
    new_enemy->atk=50;
}

new_enemy->dfs=10;
new_enemy->atk=50;

return new_enemy;

}

list* create_list(){
    list *newList = (list * )malloc (sizeof (list));
    newList->current=NULL;
    newList->next = NULL;
    newList->prev = NULL;
    return newList;
}

Mapa* create_mapa(){
    Mapa *newMap = (Mapa * )malloc (sizeof (Mapa));
    newMap->root=NULL;
    newMap->current = NULL;
    newMap->camino=create_list();
    return newMap;
}

level * create_node(int id) {
    level *new = (level * )malloc (sizeof (level));
    //assert (new != NULL); // No hay memoria para reservar la Lista.
    new->izq=NULL;
    new->der=NULL;
    new->up=NULL;
    new->dwn=NULL;
    new->rival=NULL;
    new->recorrido=0;
    new->cofre=false;
    new->id=id;
    return new;
}

char * _strdup (const char *s) {
	size_t len = strlen (s) + 1;
    void *new = malloc (len);
	if (new == NULL){
        return NULL;
	}
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

void pause_erase(WINDOW* pausewin){//borra el menu de pausa
	for(int i=0;i<5;i++){
        for(int j=0;j<13;j++){
            mvwprintw(stdscr,i,j," ");
        }
    }
    wrefresh(pausewin);
}

int Escape(){
    beep();
    init_pair(8,COLOR_BLUE,COLOR_BLACK);
    WINDOW* pausewin = newwin(3,12,25,54);
    wbkgd(pausewin,COLOR_PAIR(8));
    mvwprintw(pausewin,0,1,"Reanudar");
    mvwprintw(pausewin,1,1,"Guardar");
    mvwprintw(pausewin,2,1,"Salir");
    mvwprintw(pausewin,0,10,"<");
    int y=0;
    keypad(pausewin,TRUE);
    //box(pausewin,0,0);
    wrefresh(pausewin);
    int key;
    do{
        	key = wgetch(pausewin);
        	wrefresh(pausewin);
        	if(key == KEY_UP){
                mvwprintw(pausewin,y,10," ");
                y--;
                if(y==-1)y=2;
                if(y==3)y=0;
                wmove(pausewin,y,10);
                mvwprintw(pausewin,y,10,"<");
                continue;
        	}

        	if(key == KEY_DOWN){
                mvwprintw(pausewin,y,10," ");
                y++;
                if(y==-1)y=2;
                if(y==3)y=0;
                wmove(pausewin,y,10);
                mvwprintw(pausewin,y,10,"<");
                continue;
        	}
        }while(key != ENTER);
        if(y==0){
        //pause_erase(pausewin);
        wclear(pausewin);
        //werase(pausewin);
        wrefresh(pausewin);
        //delwin(pausewin);
        //refresh();
        }


    return y+1;


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
    if(type==666){
        for(i=1;i<=120;i+=18){
            mvwprintw(ventana,19,i,"^/\\/\\^^^/\\^^/\\/\\/\\");
            mvwprintw(ventana,20,i,"^^/\\^/\\/\\/\\^^/\\^^^/\\");
        }
    }
    wrefresh(ventana);
    return;
}

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
    if(type==666){
        if(x<43){
            *right = 1;
        }else{
            *right = 0;
        }
        if(x>1){
            *left = 1;
        }else{
            *left = 0;
        }
        if(y>1){
            *up = 1;
        }else{
            *up = 0;
        }
        if(y<14){
            *down = 1;
        }else{
            *down = 0;
        }
    }
}

void flush_menu(WINDOW* menuwin){
    int i,j;
    for(i=1;i<5;i++){
        for(j=1;j<107;j++){
            mvwprintw(menuwin,i,j," ");
        }
    }
    wrefresh(menuwin);
    return;
}

void flush_dialog(WINDOW* menuwin){
    int i,j;
    for(i=2;i<5;i++){
        for(j=1;j<107;j++){
            mvwprintw(menuwin,i,j," ");
        }
    }
    wrefresh(menuwin);
    return;
}

void det_pos(int * player_x , int * player_y , int x_before, int y_before,level*  nivel){

    if(nivel->id==0){
        *player_y=9;
        *player_x=9;
        return;
    }

    if(x_before >= 102){
        *player_x = 3;
        *player_y = y_before;
    }

    if(x_before <= 1){
        *player_x=101;
        *player_y=y_before;
        return;
    }


    if(y_before <= 1){
        *player_x=x_before;

        if(x_before>26 && x_before <42){//Subiendo por la izquierda (tipo 2)
            if(nivel->id==1){
                *player_x=x_before+34;
            }
            else{
                *player_x=x_before+16;
            }
        }

        if(x_before>42 && x_before<59){//subiendo por el medio
            if(nivel->id ==1){
                *player_x=x_before+18;
            }
            if(nivel->id == 2){
                *player_x=x_before-16;
            }
        }

        if (x_before>60 && x_before<76){//subiendo por la derecha(tipo 1)
            if(nivel->id == 2 ){
                *player_x=x_before-34;
            }
            else{
                *player_x=x_before-18;
            }
        }


        *player_y=15;

        return;
    }


    if(y_before >= 16){
        *player_x=x_before;

        if(x_before>26 && x_before <42){//Subiendo por la izquierda (tipo 2)
            if(nivel->id==1){
                *player_x=x_before+34;
            }
            else{
                *player_x=x_before+16;
            }
        }

        if(x_before>=42 && x_before<59){//subiendo por el medio
            if(nivel->id ==1){
                *player_x=x_before+18;
            }
            if(nivel->id == 2){
                *player_x=x_before-16;
            }
        }

        if (x_before>=60 && x_before<=80){//subiendo por la derecha(tipo 1)
            if(nivel->id == 2 ){
                *player_x=x_before-34;
            }
            else{
                *player_x = x_before-18;
            }
        }



        *player_y=2;

        return;
    }


}

bool sorteo_cofre(level* nivel){
    if(nivel->id==7)return false;
    int t;
    int aux;
    srand((unsigned) time(&t));

    aux=(rand() % 3);

    if(aux==2)return true;    
    
    return false;
}

int sorteo_type(){
return 5;
}


bool sorteo_enemy(level*nivel){
    if(nivel->id==7 || nivel->id==0)return false;

    int t;
    int aux;
    srand((unsigned) time(&t));

    aux=(rand() % 10);

    if(aux==6)return true;

    return false;

}

void det_cofrexy(int *item_x,int *item_y,level* nivel,int x , int y){
    *item_y=9;
    *item_x=54;

    if(nivel->id==0){
        *item_y=9;
        *item_x=80;
    }

    if(nivel->id==1){
        *item_y=9;
        *item_x=9;
        
        if(x>=102){
            *item_y=4;
            *item_x=70;
        }
    }

    if(nivel->id==2){
        
        *item_y=9;
        *item_x=95;

        if(x<=1){
            *item_x=35;
        }
    }

    if(nivel->id==4|| nivel->id ==3){
        
        *item_y=9;
        *item_x=95;

        if(x<=1){
            *item_x=9;
        }
    }

    if(nivel->id==5){
        *item_y=9;
        *item_x=95;

        if(x<=1){
            *item_x=9;
        }    
    }

    if(nivel->id==6){
        *item_y=9;
        *item_x=9;

        if(x>=102){
            *item_x=95;
        }
    }

}

void print_item(int contenido,int* item_pick,int key,WINDOW* menuwin,WINDOW* itemwin,int item_y,int item_x,int player_y,int player_x){
    int x = item_x;
    int y = item_y;
    init_pair(3,COLOR_BLUE,COLOR_BLACK);
    init_pair(4,COLOR_BLACK,COLOR_BLACK);
    wbkgd(itemwin,COLOR_PAIR(3));
    if(*item_pick==0){ //si el item no ha sido recogido
        mvwprintw(itemwin,0,0,"[[]]");
        mvwprintw(itemwin,1,0,"[__]");
        wrefresh(itemwin);
        if((x>player_x && x<player_x+11)&&(y>player_y-1 && y<player_y+7)){ //si el jugador esta cerca del item
            flush_menu(menuwin);
            mvwprintw(menuwin,2,2,"Aqui hay un cofre...");
            mvwprintw(menuwin,3,2,"Presiona ENTER para abrirlo.");
            if(key==ENTER){ //si el jugador presiona ENTER estando cerca del item
                flush_menu(menuwin);
                if(contenido==0){ //si el contenido es 0 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 0 (+10 ATK)");
                }
                if(contenido==1){ //si el contenido es 1 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 1 (+10 DEF)");
                }
                if(contenido==2){ //si el contenido es 2 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 2 (+20 ATK)");
                }
                if(contenido==3){ //si el contenido es 3 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 3 (+20 DEF)");
                }
                if(contenido==4){ //si el contenido es 4 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 4 (+20 DEF)");
                }
                if(contenido==5){ //si el contenido es 5 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 5 (+30 DEF)");
                }
                if(contenido==6){ //si el contenido es 6 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 6 (+40 ATK)");
                }
                if(contenido==7){ //si el contenido es 7 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 7 (+40 DEF)");
                }
                if(contenido==8){ //si el contenido es 8 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 8 (+50 ATK)");
                }
                if(contenido==9){ //si el contenido es 9 hacer esto
                    mvwprintw(menuwin,2,2,"ITEM 9 (+50 DEF)");
                }
                wbkgd(itemwin,COLOR_PAIR(4)); //borrar item de la pantalla
                wrefresh(itemwin);
                *item_pick=1; //marcar como item recogido
            }
        }else{
            flush_menu(menuwin); //si el jugador deja de estar cerca limpiar el menu
        }
    }
}

void enemy_action(WINDOW* menuwin,int type, int* enemy_talked, int enemy_y,int enemy_x, int player_y, int player_x){
    int key;
    int select=0;
    char keep_dialog;
    char selchar;
    if(player_x > enemy_x-11 && player_x < enemy_x+18 && player_y > enemy_y-6 && player_y < enemy_y+6){
        if(type==0){
            flush_menu(menuwin);
            mvwprintw(menuwin,1,2,"Nekomata [ATK: 20] [DEF: 30]:");
            mvwprintw(menuwin,1,99,"ENTER >");
            mvwprintw(menuwin,2,3,"*Yawn* ... eh? ...");
            mvwprintw(menuwin,3,3,"oye... Oye tu! tu... no... *Yawn* no puedes estar aqui, eh? Son ordenes de...");
            mvwprintw(menuwin,4,3,"ordenes del gran... *Yawn* Lucifer... Que no puedes, vaya...");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                keep_dialog = getch();
            }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,3,"Si no te vas me vere obligada a... *Yawn* Bueno... Mmm...");
            mvwprintw(menuwin,3,3,"A darte un escarmiento, eh? ... Te... *Yawn*");
            mvwprintw(menuwin,4,3,"Mira, por que estas aqui en primer lugar?");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                keep_dialog = getch();
            }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,2,"[1] Eso no te incumbe, y si no me dejas pasar te movere yo. (Combatir)");
            mvwprintw(menuwin,3,2,"[2] He venido a recuperar mi alma. (Razonar)");
            mvwprintw(menuwin,4,2,"[3] Nos Belmont. (Escapar)");
            wrefresh(menuwin);
            while(select==0){
                selchar = getch();
                if(selchar == '1'){
                    select=1;
                    continue;
                }
                if(selchar == '2'){
                    select=2;
                    continue;
                }
                if(selchar == '3'){
                    select=3;
                    continue;
                }
                wrefresh(menuwin);
            }
            if(select==1){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Si vas a hacerte el chulito entonces supongo... *Yawn* ...");
                mvwprintw(menuwin,3,3,"Supongo que... tendremos que hacerlo por las malas... que flojera la verdad...");
                mvwprintw(menuwin,4,3,"Esta bien, preparate renacuajo... *Yawn* No pasas de aqui...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==2){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Oooh, ya veo... *Yawn* eso lo he escuchado antes, se como se siente...");
                mvwprintw(menuwin,3,3,"Pero los mortales no pueden pisar los aposentos... *Yawn* ... de Lucifer ...");
                mvwprintw(menuwin,4,3,"Supongo que tendre que... *Yawn* ... detenerte, no te resistas, eh?");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Donde puse las esposas infernales... A ver... *Yawn* ...");
                mvwprintw(menuwin,3,3,"No puede ser... En serio las olvide en casa de Lycoris? *Yawn* que estupida...");
                mvwprintw(menuwin,4,3,"Mira... tu te quedas aqui mientras voy a buscarlas... *Yawn* ... No me demoro...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"... Sabes que? No vale la pena, haz lo que quieras, eh?... Para que veas que soy buena gente...");
                mvwprintw(menuwin,3,3,"Ademas alguna vez tambien estuve en tu situacion... *Yawn* ... Tuve que convertirme en guardiana del");
                mvwprintw(menuwin,4,3,"infierno para recuperar mi alma, tal vez contigo haga el mismo trato... *Yawn*");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==3){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Oye! Alto! Espera! Te dije que no... *Yawn* ... no puedes pasar... Tu...");
                mvwprintw(menuwin,3,3,"... Bah, haz lo que te plazca, mocoso. Tampoco llegaras muy lejos... *Yawn*");
                mvwprintw(menuwin,4,3,"Tengo mejores... *Yawn* cosas que hacer... ... ... zzz...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,1,2,"                                                                       ");
                mvwprintw(menuwin,2,3,"Esta dormida...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            flush_menu(menuwin);
            *enemy_talked = 1;
        }
        if(type==1){
            flush_menu(menuwin);
            mvwprintw(menuwin,1,2,"Gamabunta [ATK: 30] [DEF: 20]:");
            mvwprintw(menuwin,1,99,"ENTER >");
            mvwprintw(menuwin,2,3,"Oye tu! Insecto! Que haces aqui? Tu cara no se me hace conocida.");
            mvwprintw(menuwin,3,3,"Oh! No seras el que trae las colaciones infernales? Le dije a Balaam que queria un lomo a lo pobre");
            mvwprintw(menuwin,4,3,"Donde lo traes? Tengo un hambre que flipas en mil colores, chaval.");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                    keep_dialog = getch();
                }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,2,"[1] Te traigo un pure con carne mechada y salsa de combos, que te parece? (Combatir)");
            mvwprintw(menuwin,3,2,"[2] He venido a anotar los almuerzos de la proxima semana. (Mentir)");
            mvwprintw(menuwin,4,2,"[3] Oh, que distraido! Se me quedaron las colaciones en casa. Voy a buscarlas... (Escapar)");
            wrefresh(menuwin);
            while(select==0){
                selchar = getch();
                if(selchar == '1'){
                    select=1;
                    continue;
                }
                if(selchar == '2'){
                    select=2;
                    continue;
                }
                if(selchar == '3'){
                    select=3;
                    continue;
                }
                wrefresh(menuwin);
            }
            if(select==1){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"No es lo que esperaba pero estoy satisfecho... Espera...");
                mvwprintw(menuwin,3,3,"Esos combos de los que hablas son algun tipo de marisco? o...");
                mvwprintw(menuwin,4,3,"Ah, con que era una amenaza? Pues ahora tu seras mi almuerzo, insecto, en guardia!");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==2){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"JA! Por quien me tomas? No soy estupido, nos preguntaron eso ayer...");
                mvwprintw(menuwin,3,3,"Entonces eres un intruso... Me parece excelente... Pues Lucifer me deja");
                mvwprintw(menuwin,4,3,"engullir a todo aquel que irrumpa en sus aposentos sin permiso, que conveniente!");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==3){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"No no no no, ni creas que dejare pasar esto por alto, jovencito.");
                mvwprintw(menuwin,3,3,"Se te olvidan los pedidos, me dejas sin comer y te quieres ir de rositas?");
                mvwprintw(menuwin,4,3,"Creo que es hora de reemplazar al cocinero infernal, por incompetente. Yo me encargo.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            flush_menu(menuwin);
            *enemy_talked = 1;
        }
        if(type==2){
            flush_menu(menuwin);
            mvwprintw(menuwin,1,2,"Balaam [ATK: 40] [DEF: 20]:");
            mvwprintw(menuwin,1,99,"ENTER >");
            mvwprintw(menuwin,2,3,"HEY! OYE TU! QUIEN ERES Y QUE HACES AQUI? ... ESPERA...");
            mvwprintw(menuwin,3,3,"ERES UN HUMANO? SEGURO VIENES A PEDIR TU ALMA DE VUELTA... SIEMPRE LO MISMO CON ESTOS BICHOS...");
            mvwprintw(menuwin,4,3,"ENTIENDAN QUE NUESTRO SENOR LUCIFER NO REALIZA DEVOLUCIONES!!! PAYASOS!!!");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                    keep_dialog = getch();
                }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,3,"DIOS... QUE RABIA QUE DAN... LO UNICO QUE LOGRAN ES HACERNOS PERDER EL TIEMPO!");
            mvwprintw(menuwin,3,3,"SABES QUE? ME SERVIRAS PARA ENVIARLE UN MENSAJE AL RESTO DE TU ASQUEROSA ESPECIE.");
            mvwprintw(menuwin,4,3,"DAME ALGUNA RAZON PARA NO ENVIARTE DE VUELTA EN PEDAZOS AQUI Y AHORA.");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                    keep_dialog = getch();
                }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,2,"[1] El unico que terminara despedazado eres tu, chivito. (Combatir)");
            mvwprintw(menuwin,3,2,"[2] Vamos, por que tanta agresividad? No hay por que ponerse asi, amigo. (Razonar)");
            mvwprintw(menuwin,4,2,"[3] ... yo ya me iba ... (Escapar)");
            wrefresh(menuwin);
            while(select==0){
                selchar = getch();
                if(selchar == '1'){
                    select=1;
                    continue;
                }
                if(selchar == '2'){
                    select=2;
                    continue;
                }
                if(selchar == '3'){
                    select=3;
                    continue;
                }
                wrefresh(menuwin);
            }
            if(select==1){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"ESO LO VEREMOS, BICHO INMUNDO.");
                mvwprintw(menuwin,3,3,"PENSABA DARTE UNA MUERTE RAPIDA, PERO TU ACTITUD ME HA HECHO CAMBIAR DE OPINION");
                mvwprintw(menuwin,4,3,"TU MUERTE SERA LENTA Y MUY, MUY DOLOROSA...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==2){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"BAH, IGUAL DE HIPPIE QUE LA PAYASA DE LYCORIS...");
                mvwprintw(menuwin,3,3,"YA SE, TE DARE UNA MUERTE RAPIDA PARA QUE NO SIENTAS DOLOR, QUE TE PARECE EL TRATO?");
                mvwprintw(menuwin,4,3,"TU EXPRESION ME DICE QUE NO TE PARECE BIEN, BUENO, NO DIGAS QUE NO LO OFRECI... PREPARATE, BICHO.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==3){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"JAJAJAJA! DE VERDAD CREES QUE TE DEJARE IR?");
                mvwprintw(menuwin,3,3,"SIENTO QUE SEAS EL ELEGIDO PARA ESTO, PERO ES PARA QUE LES QUEDE CLARO A TU ESPECIE DE");
                mvwprintw(menuwin,4,3,"PACOTILLA, NO. SE. DEVUELVEN. ALMAS... ENTENDIDO, BICHARRACO? AHORA, PREPARATE...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            flush_menu(menuwin);
            *enemy_talked = 1;
        }
        if(type==3){
            flush_menu(menuwin);
            mvwprintw(menuwin,1,2,"Lycoris [ATK: 5] [DEF: 50]:");
            mvwprintw(menuwin,1,99,"ENTER >");
            mvwprintw(menuwin,2,3,"oye tu, chico! OwO. no te conozco... eres nuevo por aqui? uwu.");
            mvwprintw(menuwin,3,3,"ah, ya se! debes ser uno de esos angeles caidos de los que tanto habla Samael, verdad? uwu. ");
            mvwprintw(menuwin,4,3,"no... tu no tienes alas... unu ... dime quien eres...");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                    keep_dialog = getch();
                }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,2,"[1] Fuera de mi camino, florcita, no he venido a charlar. (Combatir)");
            mvwprintw(menuwin,3,2,"[2] Soy un humano, he venido a recuperar mi alma. (Razonar)");
            mvwprintw(menuwin,4,2,"[3] Si, soy un angel caido, pero de los que no tienen alas (Mentir)");
            wrefresh(menuwin);
            while(select==0){
                selchar = getch();
                if(selchar == '1'){
                    select=1;
                    continue;
                }
                if(selchar == '2'){
                    select=2;
                    continue;
                }
                if(selchar == '3'){
                    select=3;
                    continue;
                }
                wrefresh(menuwin);
            }
            if(select==1){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"vaya... owo... igual de hostil que Balaam... unu");
                mvwprintw(menuwin,3,3,"lo siento... no puedo dejarte pasar, ordenes directas de Luci... uwu");
                mvwprintw(menuwin,4,3,"y si vas a ponerte asi de agresivo, tendre que usar la fuerza. en guardia, chico! uwu");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==2){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"aaah, entiendo. uwu... ultimamente ha habido muchos como tu, humanos que vienen a");
                mvwprintw(menuwin,3,3,"recuperar su alma... uwu. todos son asesinados brutalmente por los demas chicos... owo.");
                mvwprintw(menuwin,4,3,"te puedo dar un consejo? sal lo mas pronto que puedas de aqui. uwu, no le dire a nadie.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"pero si no vas a cambiar de opinion, no te detendre. uwu");
                mvwprintw(menuwin,3,3,"ve bajo tu propia responsabilidad, vale? y si te preguntan yo no te vi. uwu.");
                mvwprintw(menuwin,4,3,"si sobrevives tal vez podamos salir algun dia... uwu... solo digo... O///O");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"ay, olvida lo que dije. uwu. bueno, si quieres... pero piensalo...");
                mvwprintw(menuwin,3,3,"bueno, nos vemos luego, pastelito. uwuwuwuwuwuwuwuwuwu");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==3){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"jajajaja. OwO. no sabia que existian sin alas, si te digo la verdad...");
                mvwprintw(menuwin,3,3,"los angeles caidos me pareces bastante guapos... O///O. Y tu no eres la excepcion...");
                mvwprintw(menuwin,4,3,"ay, ya me estoy pasando de nuevo...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"bueno... angelito... uwu. tal vez nos veamos un dia de estos...");
                mvwprintw(menuwin,3,3,"ojala conocernos mejor. uwu. te doy mi numero de telefono... OwO");
                mvwprintw(menuwin,4,3,"mi numero es 6, 6 y 6. llamame cuando puedas, bombon, nos vemos luego ;)");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            flush_menu(menuwin);
            *enemy_talked = 1;
        }
        if(type==4){
            flush_menu(menuwin);
            mvwprintw(menuwin,1,2,"Samael [ATK: 30] [DEF: 30]:");
            mvwprintw(menuwin,1,99,"ENTER >");
            mvwprintw(menuwin,2,3,"Oyeoyeoye, tu! El de los pelos parados, ven aqui un segundo...");
            mvwprintw(menuwin,3,3,"No te conozco, de seguro eres uno de esos pelmazos que recluta el tarado de Balaam...");
            mvwprintw(menuwin,4,3,"Pues que sepas que la mano derecha de nuestro amo Lucifer soy YO. Se entiende?");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                keep_dialog = getch();
            }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,3,"No es Balaam, no es Pinhera, soy YO!");
            mvwprintw(menuwin,3,3,"Aunque te digan lo contrario, Balaam solo ha llegado donde esta por ser el consentido");
            mvwprintw(menuwin,4,3,"de Lucifer, que le ven que a mi no? Oye, entre tu y yo... no le dire a nadie, que opinas de Balaam?");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                keep_dialog = getch();
            }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,2,"[1] Opino que te da mil vueltas, tu estas chiquito. (Combatir)");
            mvwprintw(menuwin,3,2,"[2] No soy su recluta, he venido por mi alma. (Verdad)");
            mvwprintw(menuwin,4,2,"[3] Es tremendo payaso. (Coincidir)");
            wrefresh(menuwin);
            while(select==0){
                selchar = getch();
                if(selchar == '1'){
                    select=1;
                    continue;
                }
                if(selchar == '2'){
                    select=2;
                    continue;
                }
                if(selchar == '3'){
                    select=3;
                    continue;
                }
                wrefresh(menuwin);
            }
            if(select==1){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Preparate para morir, escuincle.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==2){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Ah... vaya... eso cambia todo...");
                mvwprintw(menuwin,3,3,"Bueno, supongo que sabes de sobra que no puedes estar aqui... Te debo encarcelar...");
                mvwprintw(menuwin,4,3,"Dame un momento que... espera...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Por que hago esto? No importa que haga, Balaam seguira siendo el nino mimado");
                mvwprintw(menuwin,3,3,"de Lucifer, vaya porqueria... A mi ni me toma en cuenta... Para que molestarme...");
                mvwprintw(menuwin,4,3,"Mira chico, por mi que hagas lo que te de la gana, solo no me nombres, ok? Vete.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"... Y si ves a Balaam... no le digas nada... Je...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==3){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Ja... Jajaja! Si! Eso es lo que es! Un gran y gordo PAYASO.");
                mvwprintw(menuwin,3,3,"Me caiste bien, chiquillo, tal vez hasta nos hagamos amigos.");
                mvwprintw(menuwin,4,3,"No es por nada, pero yo puedo entrenarte mejor que ese lameplatos de Balaam");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Bueno, tengo que seguir haciendo guardia.");
                mvwprintw(menuwin,3,3,"Cuando puedas pasate por aqui, podriamos formar... no se...");
                mvwprintw(menuwin,4,3,"El club anti Balaam, jajaja... bueno... adios, amigo... Jajaja...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"...");
                mvwprintw(menuwin,3,3,"Balaam un payaso... Jaja... Muy buena...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            flush_menu(menuwin);
            *enemy_talked = 1;
        }
        if(type==5){
            flush_menu(menuwin);
            mvwprintw(menuwin,1,2,"Pinhera [ATK: 50] [DEF: 10]:");
            mvwprintw(menuwin,1,99,"ENTER >");
            mvwprintw(menuwin,2,3,"Oye! Tu! Renacuajo! No te conozco, no sabes que solo personal autorizado puede estar aqui?");
            mvwprintw(menuwin,3,3,"Y si no te conozco, pues no estas autorizado, obvio.");
            mvwprintw(menuwin,4,3,"Eres un intruso, no? Vienes a robar nuestras reservas de dinero infernal, eh? Maldito?");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                keep_dialog = getch();
            }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,3,"YO soy el encargado de las finanzas aqui, y no dejare que hurtes una sola moneda de");
            mvwprintw(menuwin,3,3,"nuestras bovedas, indigente debes de ser...");
            mvwprintw(menuwin,4,3,"Tienes una pinta de picante que te encargo. Explicate, que haces aqui?");
            wrefresh(menuwin);
            while(keep_dialog!=10){
                keep_dialog = getch();
            }
            keep_dialog=1;
            flush_dialog(menuwin);
            mvwprintw(menuwin,2,2,"[1] No me interesa tu dinero, pescadito, puedes metertelo donde quieras. (Combatir)");
            mvwprintw(menuwin,3,2,"[2] Vengo por mi alma, no a robar. (Razonar)");
            mvwprintw(menuwin,4,2,"[3] Pues me piro vampiro. (Escapar)");
            wrefresh(menuwin);
            while(select==0){
                selchar = getch();
                if(selchar == '1'){
                    select=1;
                    continue;
                }
                if(selchar == '2'){
                    select=2;
                    continue;
                }
                if(selchar == '3'){
                    select=3;
                    continue;
                }
                wrefresh(menuwin);
            }
            if(select==1){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Que crio mas insolente, que clase de educacion recibiste?");
                mvwprintw(menuwin,3,3,"No tienes ni el mas minimo respeto por tus mayores.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Te tendre que dar una leccion de modales.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==2){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Ah, ya veo. Pero...");
                mvwprintw(menuwin,3,3,"No sabias que devolver el alma a un ser conlleva un gran gasto de dinero?");
                mvwprintw(menuwin,4,3,"Asi que en el fondo, si has venido a robar...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Que sepas que no lo permitire.");
                mvwprintw(menuwin,3,3,"Ponte en guardia, indecente, no dejare que molestes al gran Lucifer");
                mvwprintw(menuwin,4,3,"con tus estupideces...");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            if(select==3){
                flush_dialog(menuwin);
                mvwprintw(menuwin,2,3,"Eso, arranca mejor, es lo que mejor saben hacer las ratas como tu...");
                mvwprintw(menuwin,3,3,"Ni creas que gastare mi tiempo en perseguirte, el tiempo es dinero...");
                mvwprintw(menuwin,4,3,"Y el dinero es mi vida.");
                wrefresh(menuwin);
                while(keep_dialog!=10){
                    keep_dialog = getch();
                }
                keep_dialog=1;
            }
            flush_menu(menuwin);
            *enemy_talked = 1;
        }
    }
}

void print_enemy(WINDOW* enemywin,int type,int y,int x){
    init_pair(5,COLOR_YELLOW,COLOR_BLACK);
    wbkgd(enemywin,COLOR_PAIR(5));
    if(type==0){
        attron(COLOR_PAIR(5) | A_BOLD);
        mvwprintw(enemywin,0,0," /|__/|           /'/  ");
        mvwprintw(enemywin,1,0,"|=  =  \\          \\ \\");
        mvwprintw(enemywin,2,0," \\w     \\_________/ /");
        mvwprintw(enemywin,3,0,"  |                 \\ ");
        mvwprintw(enemywin,4,0,"  |  |   |______|   | ");
        mvwprintw(enemywin,5,0,"  |  |   |    | |   | ");
        mvwprintw(enemywin,6,0," (___(___)    (_(___) ");
        attroff(COLOR_PAIR(5));
    }
    if(type==1){
        attron(COLOR_PAIR(5) | A_BOLD);
        mvwprintw(enemywin,0,0,"  (^^^^))             ");
        mvwprintw(enemywin,1,0," ((0 (0 ))__          ");
        mvwprintw(enemywin,2,0,"/ ______    \\__      ");
        mvwprintw(enemywin,3,0,"\\/______\\      \\____  ");
        mvwprintw(enemywin,4,0," \\  ___             \\");
        mvwprintw(enemywin,5,0," /  /  /  /______/  /");
        mvwprintw(enemywin,6,0,"oOO-O O-OOo  oOO O-OOo");
        attroff(COLOR_PAIR(5));
    }
    if(type==2){
        attron(COLOR_PAIR(5) | A_BOLD);
        mvwprintw(enemywin,0,0,"       /\\   /\\        ");
        mvwprintw(enemywin,1,0,"      <\\ \\_/ />       ");
        mvwprintw(enemywin,2,0,"     <::\\___/::>      ");
        mvwprintw(enemywin,3,0,"888===\\(=) (=)/====>>>");
        mvwprintw(enemywin,4,0,"      <\\ / \\ />       ");
        mvwprintw(enemywin,5,0,"       <\\O_O/>        ");
        mvwprintw(enemywin,6,0,"         WWW          ");
        attroff(COLOR_PAIR(5));
    }
    if(type==3){
        attron(COLOR_PAIR(5) | A_BOLD);
        mvwprintw(enemywin,0,0,"       ___   ___      ");
        mvwprintw(enemywin,1,0,"    __/_  \\ /  _\\__   ");
        mvwprintw(enemywin,2,0,"   /    \\  |  /    \\  ");
        mvwprintw(enemywin,3,0,"   \\____ (UwU)_____/  ");
        mvwprintw(enemywin,4,0,"   /     /    \\    \\  ");
        mvwprintw(enemywin,5,0,"   \\____/      \\___/  ");
        mvwprintw(enemywin,6,0,"        \\______/      ");
        attroff(COLOR_PAIR(5));
    }
    if(type==4){
        attron(COLOR_PAIR(5) | A_BOLD);
        mvwprintw(enemywin,2,0,"        (_ ) ^        ");
        mvwprintw(enemywin,3,0,"     \\\\\\\"  ) )        ");
        mvwprintw(enemywin,4,0,"       \\/ u\\(         ");
        mvwprintw(enemywin,5,0,"       (____)         ");
        attroff(COLOR_PAIR(5));
    }
    if(type==5){
        attron(COLOR_PAIR(5) | A_BOLD);
        mvwprintw(enemywin,0,0,"  /^^^^^^\\______    _ ");
        mvwprintw(enemywin,1,0," /__            \\> / \\");
        mvwprintw(enemywin,2,0,"/(0 )============\\/  /");
        mvwprintw(enemywin,3,0,"\\______  _______  | |>");
        mvwprintw(enemywin,4,0," \\/\\/\\/| $$$$$$/ /\\  \\");
        mvwprintw(enemywin,5,0,"/\\/\\/\\/  \\____/ /> \\_/");
        mvwprintw(enemywin,6,0,"\\______________/>");
        attroff(COLOR_PAIR(5));
    }
    wrefresh(enemywin);
}

void print_boss(WINDOW* bosswin,int y, int x){
    init_pair(6,COLOR_YELLOW,COLOR_BLACK);
    wbkgd(bosswin,COLOR_PAIR(6));
    mvwprintw(bosswin,0,0,"                            ,-.                            ");
    mvwprintw(bosswin,1,0,"       ___,---.__          /'|`\\          __,---,___       ");
    mvwprintw(bosswin,2,0,"    ,-'    \\`    `-.____,-'  |  `-.____,-'    //    `-.    ");
    mvwprintw(bosswin,3,0,"   ,'        |           ~'\\     /`~           |        `. ");
    mvwprintw(bosswin,4,0," /      ___//              `. ,'          ,  , \\___      \\ ");
    mvwprintw(bosswin,5,0,"|    ,-'   `-.__   _         |        ,    __,-'   `-.    |");
    mvwprintw(bosswin,6,0,"|   /          /\\_  `   .    |    ,      _/\\          \\   |");
    mvwprintw(bosswin,7,0,"\\  |           \\ \\`-.___ \\   |   / ___,-'/ /           |  /");
    mvwprintw(bosswin,8,0," \\  \\           | `._   `\\\\  |  //'   _,' |           /  / ");
    mvwprintw(bosswin,9,0,"  `-.\\         /'  _ `---'' , . ``---' _  `\\         /,-'  ");
    mvwprintw(bosswin,10,0,"     ``       /     \\    ,='/ \\`=.    /     \\       ''     ");
    mvwprintw(bosswin,11,0,"             |__   /|\\_,--.,-.--,--._/|\\   __|             ");
    mvwprintw(bosswin,12,0,"             /  `./  \\\\,\\ |  |  | /,//' \\,'  \\             ");
    mvwprintw(bosswin,13,0,"            /   /     ||\\-+--|--+-/||     \\   \\            ");
    mvwprintw(bosswin,14,0,"           |   |     /'\\_\\_\\ | /_/_/`\\     |   |           ");
    mvwprintw(bosswin,15,0,"            \\   \\__, \\_     `-'     _/ .__/   /            ");
    mvwprintw(bosswin,16,0,"             `-._,-'   `-._______,-'   `-._,-'             ");
    wrefresh(bosswin);
}

int lista_ad(level* nodo,char *salida){
    //__time_t /linux 
    int t;
    int aux;
    srand((unsigned) time(&t));

    if(strcmp(salida,"derecha")==0){

        while(1){

            aux=(rand() % 8);

            if(aux==2 || aux==7 || aux == 0 || nodo->id == aux){
                continue;
            }

            return aux;
        }
    }

    if(strcmp(salida,"izquierda")==0){

        while(1){

            aux=(rand() % 8);

            if(aux==1 || aux==7 || aux == 0 || nodo->id == aux){
                continue;
            }

            return aux;
        }
    }

    if(strcmp(salida,"abajo")==0){

        while(1){

            aux=(rand() % 8);

            if(aux==6 || aux==4 || aux == 0 || nodo->id == aux){
                continue;
            }

            return aux;
        }
    }

    if(strcmp(salida,"arriba")==0){

        while(1){

            aux=(rand() % 8);

            if(aux==3 || aux==6 || aux == 0 || nodo->id == aux){
                continue;
            }

            return aux;
        }
    }

}

void gameplay(jugador*player,level* nivel, int h, int x_before, int y_before){//Inicializa el gameplay del juego
    int player_x;
    //det_pos(x_ant);
    int player_y;
    //det_pos(x_ant,y_ant);
    //x=10
    //y=9
    int item_x;
    int item_y;
    int item_pick;
    int enemy_type;
    
    det_pos(&player_x,&player_y, x_before, y_before,nivel);//Determina las posiciones del jugador segun el nivel anterior
    
    //Define si existe un cofre en el nivel y su posicion
    nivel->cofre = sorteo_cofre(nivel);
    if(nivel->cofre==true){
        det_cofrexy(&item_x,&item_y,nivel,x_before,y_before);
        item_pick = 0;
    }
    else{
        item_x=item_y=450;
        item_pick = 1;
    }

    int enemy_x=150;
    int enemy_y=150;

    if(sorteo_enemy(nivel) ==true){
        
        enemy_x = 63;
        enemy_y = 9;
        enemy_type = 5;
        //enemy_type=sorteo_type();
        nivel->rival = create_enemy(enemy_type);
    }
    
    int boss_x = 53;
    int boss_y = 2;
    int enemy_talked = 0;
    int item_content = 0;
    int key;
    int i;
    int canmoveleft,canmoveright,canmoveup,canmovedown;
    initscr();
    noecho();
    cbreak();
    start_color();
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(2,COLOR_BLUE,COLOR_BLACK);
    int yMax , xMax;
    getmaxyx(stdscr, yMax , xMax);
    WINDOW * menuwin = newwin(6, 108,24, 5);
    WINDOW * gamewin = newwin(22,108,1,5);//y,x,mover arriba,mover derecha
    WINDOW* itemwin = newwin(2,4,item_y,item_x);
    WINDOW* enemywin = newwin(7,22,enemy_y,enemy_x);
    WINDOW* bosswin = newwin(17,59,boss_y,boss_x);
    wbkgd(gamewin,COLOR_PAIR(1));
    wbkgd(menuwin,COLOR_PAIR(1));
    generate_map_type(gamewin,nivel->id); //generar tipo de mapa
    box(menuwin,0,0);
    box(gamewin,0,0);
    print_player(gamewin,player_y,player_x,2);
    refresh();
    keypad(gamewin,true);

    while(1){
        wrefresh(gamewin);
        wrefresh(menuwin);
        wrefresh(enemywin);
        wrefresh(itemwin);
        ///Esto es para controlar las entradas
        //mvwprintw(gamewin,1,1,"%i",nivel->id);
        //mvwprintw(gamewin,1,3,"%i",nivel->izq->id);
        if(h!=0){
            //mvwprintw(gamewin,2,1,"%i",nivel->izq->id);
        }
        //mvwprintw(gamewin,3,1,"%i",nivel->izq->id);
        canmove(nivel->id,player_y,player_x,&canmoveleft,&canmoveright,&canmoveup,&canmovedown);
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
        if(key==ESC){
            keypad(gamewin,FALSE);
            int opt=Escape();
            if(opt==3){
            	clear();
            	main();
            	//menu_principal();
            }
            if(opt==1){

            keypad(gamewin,TRUE);
            key=KEY_DOWN;
            }
            if(opt==2){
            	keypad(gamewin,TRUE);
            }
        }
        if(item_pick == 0){
            print_item(item_content,&item_pick,key,menuwin,itemwin,item_y,item_x,player_y,player_x);
        }
        print_enemy(enemywin,enemy_type,enemy_y,enemy_x);
        if(enemy_talked==0){
            enemy_action(menuwin,enemy_type,&enemy_talked,enemy_y,enemy_x,player_y,player_x);
        }
        //DEBUG: mvwprintw(menuwin,4,95,"Y=%i, X=%i",player_y,player_x);
        if(player_x==102){//derecha
            jugador* aux=create_node(nivel->id);
            if(nivel->der!=NULL){
                nivel->izq=aux;
                nivel=nivel->der;
                nivel->der=NULL;
                player->nivel=nivel->id;
                gameplay(player,nivel,0,player_x,player_y);
            }
            int type=lista_ad(nivel,"derecha");
            nivel->der = create_node(type);
            nivel=nivel->der;
            player->nivel=nivel->id;
            nivel->izq=aux;
            wrefresh(gamewin);
            gameplay(player,nivel,0,player_x,player_y);
        }
        if(player_x==1){//izquierda
            jugador* aux=create_node(nivel->id);
            if(nivel->izq!=NULL){
                //si ya se recorrio antes el izquierdo
                nivel->der=aux;
                //el derecho
                nivel = nivel->izq;
                //el izquierdo pasa a ser el actual
                nivel->izq=NULL;
                player->nivel=nivel->id;
                gameplay(player,nivel,0,player_x,player_y);
            }
            int type=lista_ad(nivel,"izquierda");
            nivel->izq = create_node(type);
            nivel=nivel->izq;
            player->nivel=nivel->id;
            nivel->der=aux;
            wrefresh(gamewin);
            gameplay(player,nivel,0,player_x,player_y);
            /*nivel->id=lista_ad(nivel,"izquierda");
            gameplay(player,nivel,h++);*/
        }
        if(player_y==1){//arriba
            jugador* aux = create_node(nivel->id);
            if(nivel->up!=NULL){
                nivel->dwn=aux;
                nivel = nivel->up;
                nivel->up=NULL;
                player->nivel=nivel->id;
                gameplay(player,nivel,0,player_x,player_y);
            }
            int type=lista_ad(nivel,"arriba");
            nivel->up=create_node(type);
            nivel=nivel->up;
            player->nivel=nivel->id;
            nivel->dwn=aux;WINDOW* pausewin = newwin(11,11,9,11);
            wrefresh(gamewin);
            gameplay(player,nivel,h++,player_x,player_y);
        }
        if(player_y==16){//abajo
            jugador* aux = create_node(nivel->id);
            if(nivel->dwn!=NULL){
                nivel->up=aux;
                nivel = nivel->dwn;
                nivel->dwn=NULL;
                player->nivel=nivel->id;
                gameplay(player,nivel,0,player_x,player_y);
            }
            int type=lista_ad(nivel,"abajo");
            nivel->dwn=create_node(type);
            nivel=nivel->dwn;
            player->nivel=nivel->id;
            nivel->up=aux;
            wrefresh(gamewin);
            gameplay(player,nivel,h++,player_x,player_y);
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

jugador *crearJugador(char *linea){//retorna un jugador a partir del archivo.txt
    jugador *j = (jugador *) malloc(sizeof(jugador)); // Asigna memoria a todo la struct
    j->nombre = get_csv_field(linea, 1);
    j->dfs = transformarAEntero(get_csv_field(linea,2));
    j->atk = transformarAEntero(get_csv_field(linea,3));
    j->nivel = transformarAEntero(get_csv_field(linea,4));
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
    attroff(A_BOLD);
    attrset(A_BOLD);
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

int menu_principal(ALLEGRO_SAMPLE *navegate1){//Pantalla Principal al iniciar el juego
    int key=KEY_LEFT;
    noecho();
    cbreak();
    start_color();
    bkgd(COLOR_BLACK);
    WINDOW * menuwin = newwin(9, 20, 16, 47);
    showtitle(19,5,1);
    keypad(menuwin,true);
    refresh();
    wrefresh(menuwin);
        int y =1;
        start_color();
        init_pair(1,COLOR_WHITE,COLOR_GREEN);
        init_pair(2,COLOR_WHITE,COLOR_BLUE);
        attron(COLOR_PAIR(1));
        wmove(menuwin,1,1);
        wprintw(menuwin,"  Nueva partida");
        wmove(menuwin,3,1);
        wprintw(menuwin,"  Cargar Partida");
        wmove(menuwin,5,1);
        wprintw(menuwin,"  Ingresar cheats");
        wmove(menuwin,7,1);
        wprintw(menuwin,"  Salir del juego");
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
                 al_play_sample(navegate1,0.5,0,1.0,ALLEGRO_PLAYMODE_ONCE,0);
                mvwprintw(menuwin,y,1," ");
                y-=2;
        		if(y==-1)y=7;
                wmove(menuwin,y,1);
                mvwprintw(menuwin,y,1,">");
                continue;
        	}
        	if(key == KEY_DOWN){
                al_play_sample(navegate1,0.5,0,1.0,ALLEGRO_PLAYMODE_ONCE,0);
        	  mvwprintw(menuwin,y,1," ");
                y+=2;
                if(y==9)y=1;
                wmove(menuwin,y,1);
                mvwprintw(menuwin,y,1,">");
                continue;
        	}
        }while(key != ENTER);
	    refresh();
    return y;//Retorna opcion escogida 
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
    fputs("0", nuevoJugador);
    fputs("\n", nuevoJugador);
    fclose(nuevoJugador);
}

void borrarPantalla(){
typewriter(8,40,20,stdscr,"                                          ");
typewriter(10,40,20,stdscr,"                                          ");
typewriter(12,40,20,stdscr,"                                          ");
typewriter(14,40,20,stdscr,"                                          ");
typewriter(16,40,20,stdscr,"                                          ");
typewriter(17,23,20,stdscr,"                                          ");
refresh();
}

void historia(int key){
    keypad(stdscr,true);
    do{
        typewriter(4,40,50,stdscr,"Erase");
        typewriter(5,40,50,stdscr,"una");
        typewriter(6,40,50,stdscr,"vez");
        typewriter(7,40,50,stdscr,"un");
        typewriter(8,40,50,stdscr,"joven");
        typewriter(9,40,50,stdscr,"que");
        typewriter(10,40,50,stdscr,"perdio");
        typewriter(11,40,50,stdscr,"su alma");
        typewriter(13,40,50,stdscr,"FIN");
        napms(1000);
        return;
    }while((key=getch())!= ESPACIO);
}

int numero_jugadores(){
    FILE *contar= fopen("jugadores.txt", "r");
    int acum=0;
    char linea[1024];
    if(contar == NULL)
    {
        printw("NO SE PUDO ABRIR EL ARCHIVO");
    }
    while(fgets (linea,1023,contar)!=NULL){
        char *nombre=(get_csv_field(linea, 1));
        int atk = atoi(get_csv_field(linea,2));
        int dfs = atoi(get_csv_field(linea,3));
        int level = atoi(get_csv_field(linea, 4));
        acum++;
    }
    return acum;
}

jugador *cargarJugador(char *nombre){
    jugador *j;
    FILE *cargar= fopen("jugadores.txt", "r");
    if(cargar == NULL)
    {
        printw("NO SE PUDO ABRIR EL ARCHIVO");
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

void mostrar(ALLEGRO_SAMPLE *bkgmusic, ALLEGRO_SAMPLE *navegate){//Muestra menu de carga de jugadores
    FILE * archivo = fopen("jugadores.txt","r");
    if(archivo == NULL){
        mvwprintw(stdscr,20,43,"NO EXISTEN PARTIDAS GUARDADAS");
        main();
    }
    int completo=0;
    int y=31;//RANDOM NUMBER
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
        //int espacios=strlen(p->nombre);
        //typewriter(y,100+espacios,50,stdscr," xd");
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
            al_play_sample(navegate,0.5,0,1.0,ALLEGRO_PLAYMODE_ONCE,0);
            
            mvwprintw(stdscr,y,97," ");

            y--;
        	if(y==30)y=34;
            if(y==35)y=31;

            wmove(stdscr,y,97);
            mvwprintw(stdscr,y,97,">");
            continue;
        }

        if(key == KEY_DOWN){
            al_play_sample(navegate,0.5,0,1.0,ALLEGRO_PLAYMODE_ONCE,0);
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
            al_destroy_sample(bkgmusic);
            main();
        }

    }while(key != ENTER);

    if(vector[y-31] != "vacio"){
        jugador* player= cargarJugador(vector[y-31]);
        no_elegible();
        int espacios = strlen(player->nombre);
        mvwprintw(stdscr,31,97,player->nombre);
        mvwprintw(stdscr,31,97+espacios," join the party :)");
        refresh();
        napms(500);
        mvwprintw(stdscr,31,97+espacios," join the party ;)");
        refresh();
        napms(500);
        mvwprintw(stdscr,31,97+espacios," join the party :)");
        refresh();
        napms(500);
        termino=true;
        //Mapa* map=create_mapa();
        //map->camino=create_list();
        level * nivel= create_node(player->nivel);
        clear();
        gameplay(player,nivel,0,9,9);
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




}

void main(){

    //AUDIO BIBLIOTECA ALLEGRO5//
    al_init();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(3);
    ALLEGRO_SAMPLE *soundintro = al_load_sample("soundf.wav");
    ALLEGRO_SAMPLE *navegate = al_load_sample("Navigate_03.wav");
    ALLEGRO_SAMPLE *bkgmusic = al_load_sample("Background1.wav");
    ALLEGRO_SAMPLE *navegate1 = al_load_sample("Navigate_00.wav");
    ALLEGRO_SAMPLE_INSTANCE *songInstance = al_create_sample_instance(bkgmusic);
    al_set_sample_instance_playmode(songInstance,ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(songInstance, al_get_default_mixer());
    //AUDIO BIBLIOTECA ALLEGRO5//

    initscr();//Inicializa consola curses.h
    al_play_sample_instance(songInstance);
    int option = menu_principal(navegate1);
    switch (option)
    {
    case 1:
        al_play_sample(soundintro,1.0,0,1.0,ALLEGRO_PLAYMODE_ONCE,0);
        al_destroy_sample(bkgmusic);
        nueva_partida();
        break;
    case 3:
        //typewriter(10,10,50,stdscr,"Cargar partida");
        mostrar(bkgmusic,navegate);
        break;
    case 5:
        typewriter(10,10,50,stdscr,"CHEATS");
        endwin();
        al_destroy_sample(soundintro);
        al_destroy_sample(bkgmusic);
        al_destroy_sample(navegate);
        al_destroy_sample_instance(songInstance);
        exit(main);
        break;
    case 7:
        napms(100);
        endwin();
        exit(main);
    }
}

void nueva_partida(){
    //Caso base
    if(numero_jugadores()>=4){
        attroff(A_BOLD);
        attrset(A_BOLD);
     typewriter(20,20,5,stdscr,"Hay demasiados jugadores");
     getch();
     typewriter(20,20,5,stdscr,"                        ");
     main();
    }
    clear();
    init_pair(20,COLOR_RED,COLOR_BLACK);
    bkgd(COLOR_PAIR(20));
    echo();
    char cadena[128];
    typewriter(8,40,70,stdscr,"Cual es tu nombre?");
    move(10,40);
    scanw("%[^\n]s",cadena);
    typewriter(12,40,70,stdscr,"tu nombre es:");
    typewriter(12,54,70,stdscr,cadena);
    typewriter(14,40,70,stdscr,"es esto correcto?");
    typewriter(16,40,70,stdscr,"SI");
    typewriter(16,45,70,stdscr,"NO");
    refresh();
    int key=56;//Random key
    int x = 40;
    noecho();
    keypad(stdscr,true);
	mvwprintw(stdscr,17,40,"^");
    do{
        key = getch();
        refresh();
        if(key == KEY_LEFT){//SI
            if(x==40)continue;
            x=40;
            mvwprintw(stdscr,17,45," ");
            mvwprintw(stdscr,17,40,"^");
            continue;
        }
        if(key == KEY_RIGHT){//NO
            if(x==45)continue;
            x=45;
            mvwprintw(stdscr,17,40," ");
            mvwprintw(stdscr,17,45,"^");
            continue;
        }
    }while(key != ENTER);
    key=839;//inicializar en un valor aleatorio para posteriories bugs
    if(x==45){//Volver a intentar o volver a el menu principal
        x=42;
        mvwprintw(stdscr,17,45," ");
        mvwprintw(stdscr,19,35,"Menu principal");
        mvwprintw(stdscr,19,55,"Reintentar");
        mvwprintw(stdscr,20,42,"^");
        do{
            key = getch();
            refresh();
            if(key == KEY_LEFT){
                if(x==42)continue;
                x=42;
                mvwprintw(stdscr,20,58," ");
                mvwprintw(stdscr,20,42,"^");
                continue;
            }
            if(key == KEY_RIGHT){
                if(x==58)continue;
                x=58;//volver al menu
                mvwprintw(stdscr,20,42," ");
                mvwprintw(stdscr,20,58,"^");
                continue;
            }
        }while(key !=ENTER);
        if(x==42){
            clear();
            return main();
        }
        if(x==58){
            clear();
            bkgd(COLOR_PAIR(4));
            return nueva_partida();
        }
    }
    ingresar(cadena);
    jugador *player = cargarJugador(cadena);
    level* nivel = create_node(0);
    //Mapa *map = create_mapa();
    borrarPantalla();
    typewriter(8,40,75,stdscr,"Diviertete, ");
    typewriter(8,52,75,stdscr,cadena);
    typewriter(10,40,75,stdscr,"=)");
    getch();
    mvwprintw(stdscr,8,40,"                                                                                         ");
    mvwprintw(stdscr,10,40,"   ");
    napms(100);
    historia(key);
    clear();
    gameplay(player,nivel,0,9,9);
}
