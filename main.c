#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include "nodo.h"
#include <stdlib.h>
#include <assert.h>
#include "funciones.h"
#define ENTER 10
#define MAXIMO 99999999
#define MINIMO 11111111
#define ESPACIO 20
typedef struct level level;

void nueva_partida(void);

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
    wrefresh(ventana);
    return;
}

//funcion de colisiones, type= tipo de mapa
void canmove(int type,int y,int x,int* left,int* right,int* up,int* down){
    if(type==0){
        if(x<101){
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
        if(((x>2) && (y>5 && y<12)) || (x>61)){
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
    generate_map_type(gamewin,0); //generar tipo de mapa
    box(menuwin,0,0);
    box(gamewin,0,0);
    print_player(gamewin,player_y,player_x,2);
    refresh();
    keypad(gamewin,true);

    while(1){
        wrefresh(gamewin);
        wrefresh(menuwin);
        canmove(0,player_y,player_x,&canmoveleft,&canmoveright,&canmoveup,&canmovedown);
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

jugador *crear_jugador (int level ,char *nombre, int atak,int dfs){
    jugador *p = malloc(sizeof(jugador));
    p->level = level;
    p->nombre = nombre;
    p->atk = atak;
    p->dfs = dfs;
    return p;
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
    fputs("\n", nuevoJugador);
    fputs("100", nuevoJugador);
    fputs("\n", nuevoJugador);
    fputs("100", nuevoJugador);
    fputs("\n", nuevoJugador);
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

void main(){
    
    initscr();
    
    int option = menu_principal();
     
    init_pair(4,COLOR_WHITE,COLOR_MAGENTA);
    clear();
    bkgd(COLOR_PAIR(4));
    switch (option)
    {
    case 1:
        nueva_partida();
        //typewriter(10,10,50,stdscr,"Nueva Partida");
        /* code */
        break;
    case 2:
        typewriter(10,10,50,stdscr,"Cargar partida");
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
    return;
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

    key=839;
	
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
    jugador *player = crear_jugador(1,cadena,100,100);
    borrarPantalla();
    historia(key);
    clear();
    gameplay();
}
