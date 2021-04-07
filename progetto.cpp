#include <iostream>
#include <cstdlib>
#include <fstream> 
#include <string>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


using namespace std;
#define dim 27560 //27560

typedef struct orari {
    int mezzo;
    int ora_partenza;
    int ora_arrivo;
    struct orari * next;
} orari;

typedef struct nodo_partenza { 
	orari * orari_partenza; //LISTA DI ORARI PER LA COPPIA (SRC,DEST) == GRAPH[SRC][DEST]
	float walk_time; //TEMPO DI PERCORRENZA A PIEDI DA A a B --> GRAPH[A][B] 
} nodo_partenza;

void dijkstra(int graph[dim][dim], int ora_partenza, int tempo_limite, int src, int dest, nodo_partenza **vicini);
int minDistance(int dist[], bool sptSet[]);
void printPath(int parent[], int mezzi, int j);
void printSolution(int src, int dest, int dist[], int n, int parent[], int mezzi[], int ora_partenza, int tempo_limite);
int find_min_orario (int src, int dest, int ora_attuale, nodo_partenza **vicini, int *mezzi);
int get_index (string nomi_fermate[dim], string fermata);
void print_mezzo (int id_mezzo);
void print_date (time_t rawtime);
int convert_date (char * date);

int main () {

	int graph[dim][dim]; //GRAPH[A][B] == 1 SSE ESISTE UN COLLEGAMENTO TRA LE DUE FERMATE CON UN MEZZO O A PIEDI !
    nodo_partenza **partenze = (nodo_partenza**) malloc(dim*sizeof(nodo_partenza*)); //MATRICE CON I VICINI
    string nomi_fermate[dim];
    //TODO : STRUTTURA DATI X TRATTE A PIEDI 

    //INIZIALIZZO MATRICE DI ADIACENZA - ALL'INIZIO TUTTI '0' -> NESSUN ARCO
    for (int i=0;i<dim;i++) {
        for (int j=0;j<dim;j++) {
            graph[i][j] = 0;
        }
    }
    
    //INIZIALIZZO LA MATRICE CON I VICINI
    for (int i=0;i<dim;i++) {
        partenze[i] = (nodo_partenza*) malloc(dim*sizeof(nodo_partenza));
    }
    for (int i=0;i<dim;i++) {
        for (int j=0;j<dim;j++) {
            partenze[i][j].orari_partenza = NULL;
			partenze[i][j].walk_time = INT_MAX;
        }
    }

    //PARSING FILE 

    //PARSING DEL FILE NODES -- RIEMPO L'ARRAY  nomi_fermate
    ifstream infile1("network_nodes.csv");
    if(!infile1.is_open()) {
        printf("File non trovato!\nChiudo il servizio...\n");
        return -1;
    }
    string line1;
    while (getline(infile1,line1)) {

        //prendo una riga del file 

        //CAMPO 1 -> ID_FERMATA == INDICE ARRAY
        char * line1_c = (char*)line1.c_str();
        char *campo1 = strtok(line1_c,";");
        int id_fermata = atoi(campo1);

        //SALTA DUE CAMPI E VAI AL QUARTO
        strtok(NULL,";");
        strtok(NULL,";");
        char * campo4 = strtok(NULL,";");

        //CAMPO 4 -> NOME FERMATA
        nomi_fermate[id_fermata] = campo4;

    }
    infile1.close();
    
    //PARSING DEL FILE NETWORK - RIEMPO LA MATRICE PARTENZE (MATRICE DI ADIACENZA DEL GRAFO)
    ifstream infile2("network_temporal_day.csv");
    if(!infile2.is_open()) {
        printf("File non trovato!\nChiudo il servizio...\n");
        return -1;
    }
    string line2;
    while (getline(infile2,line2)) {
        
        char *line2_c = (char*)line2.c_str();

        //CAMPO 1 -> VERTICE
        char * campo = strtok(line2_c,";");
        int id_fermata = atoi(campo);
        //CAMPO 2 -> VERTICE ADIACENTE
        campo = strtok(NULL,";");
        int id_fermata_adiac = atoi(campo);
        //CAMPO 3 -> ORARIO PARTENZA
        campo = strtok(NULL,";");
        int ora_partenza = atoi(campo);
        //printf("ORA PARTENZA : %d  ",ora_partenza);
        //CAMPO 4 -> ORARIO ARRIVO
        campo = strtok(NULL,";");
        int ora_arrivo = atoi(campo);
        //printf("ORA ARRIVO : %d\n",ora_arrivo);
        //CAMPO 5 -> TIPO CORSA 
        campo = strtok(NULL,";");
        int mezzo = atoi(campo);

        //INSERISCO ARCO NELLA MATRICE DI ADIACENZA - G[A][B] = 1 --> ARCO (A,B)
        graph[id_fermata][id_fermata_adiac] = 1;
        
        //INSERISCO IL NUOVO ORARIO NEGLI ORARI DEL NODO  
        orari * new_orario = (orari*) malloc(sizeof(orari));
        new_orario->ora_partenza = ora_partenza;
        new_orario->ora_arrivo = ora_arrivo;
        new_orario->mezzo = mezzo;
        //INSERIMENTO IN TESTA
        new_orario->next = partenze[id_fermata][id_fermata_adiac].orari_partenza;
        partenze[id_fermata][id_fermata_adiac].orari_partenza = new_orario;
        //partenze[id_fermata][id_fermata_adiac].index = id_fermata;
        //printf("Iterazione : %d\n",i);
    }

    //PARSARE FILE WALK --> RIEMPIRE STRUTTURA DATI CON TEMPI DI PERCORRENZA --> TEMPO PERCORRENZA = DISTANZA / VELOCITA'
    //VELOCITA' == 5KM/H , DISTANZA == LA TROVI NEL FILE PARSANDO 
    
    ifstream infile3("network_walk.csv");
    if(!infile3.is_open()) {
        printf("File non trovato!\nChiudo il servizio...\n");
        return -1;
    }
    string line3;
	float walk_speed = 1.4;
    while (getline(infile3,line3)) {

        char * line3_c = (char*) line3.c_str();

        //CAMPO 1 --> NODO A
        char * campo = strtok(line3_c,";");
        int id_f = atoi(campo);

        //CAMPO 2 --> NODO B
        campo = strtok(NULL,";");
        int id_fadiac = atoi(campo);

        //CAMPO 3  --> DISTANZA LINEA D'ARIA --> LA SALTO
        strtok(NULL,";");

        //CAMPO 4 --> DISTANZA EFFETTIVA A PIEDI
		campo = strtok(NULL,";");
		int dist = atoi(campo);

		//CALCOLO TEMPO DI PERCORRENZA COME --> T = D/V CON V = 1.4M/S
		float time_walk = dist/walk_speed;

		//AGGIUNGO ARCO --> SETTO UN ENTRY DI GRAPH A 1, SE C'ERA GIA' RESTA 1, SE NON C'ERA L'ARCO (ERA 0) LO AGGIUNGO 
		graph[id_f][id_fadiac] = 1;

		//INSERISCO IN MATRICE PARTENZE 
		partenze[id_f][id_fadiac].walk_time = time_walk;
		//printf("%f",partenze[id_f][id_fadiac].walk_time);

    }

    //LEGGIAMO IN INPUT FERMATE E ORARI    
    string nodo_partenza;
    string nodo_arrivo;
    string orario_partenza;
    int tempo_limite; //orario_arrivo < orario_partenza + tempo_limite
    
    cout << "Inserisci fermata di partenza : \n";  
    getline(cin,nodo_partenza);        
    int src = get_index(nomi_fermate,nodo_partenza);
    while (src==-1) {
        cout << "La fermata selezionata non esiste, si prega di inserirne un'altra :\n";
        getline(cin,nodo_partenza);
        src = get_index(nomi_fermate,nodo_partenza);
    }
    
    cout << "Inserisci fermata di arrivo : \n";
    getline(cin,nodo_arrivo);
    int dest = get_index(nomi_fermate,nodo_arrivo);
    while (dest==-1) {
        cout << "La fermata selezionata non esiste, si prega di inserirne un'altra\n";
        getline(cin,nodo_arrivo);
        dest = get_index(nomi_fermate,nodo_arrivo);
    }
    
    cout << "Inserisci orario di partenza nel formato : YYY-MM-DD HH:MM:SS\n";
	
    getline(cin,orario_partenza);
	char * orario_partenza_c = (char*) orario_partenza.c_str();
	int timestamp = convert_date(orario_partenza_c);
	while (timestamp==-1) {
		cout << "Formato orario non corretto, si prega di inserirlo nuovamente\n";
		getline(cin,orario_partenza);
		char * orario_partenza_c = (char*) orario_partenza.c_str();
		timestamp = convert_date(orario_partenza_c);
	}
	//printf("%d\n",timestamp);

    cout << "Inserisci tempo limite in secondi : \n";
    cin >> tempo_limite;
    while (tempo_limite < 0) {
        cout << "Il tempo limite deve essere un valore positivo\n";
        cout << "Inserisci tempo limite : \n";
        cin >> tempo_limite;
    }

    cout << "Calcolo una stima dell'orario di arrivo e del percorso...\n";
    
	dijkstra(graph, timestamp, tempo_limite, src, dest, partenze);

    return 0;
}

int minDistance(int dist[], bool sptSet[]) {
       
    int min = INT_MAX, min_index; 
  
    for (int v = 0; v < dim; v++) {
        if (sptSet[v] == false && dist[v] <= min) {
            min = dist[v], min_index = v; 
        }
    }
    return min_index; 
} 

void printPath(int parent[], int mezzi[], int j) {
      
    if (parent[j] == - 1) return; 
  
    printPath(parent, mezzi, parent[j]); 
  
    //printf("%d mezzo:%d ", j, mezzi[j]);
    print_mezzo(mezzi[j]); 
} 

void printSolution(int src, int dest, int dist[], int n, int parent[], int mezzi[], int ora_partenza, int tempo_limite) { 
    printf("Fermate\t\tOra di arrivo\t\tMezzi");
    if ((dist[dest] < (ora_partenza + tempo_limite))) { 
        printf("\n%d -> %d\t\t", src, dest);
		print_date(dist[dest]); 
        printPath(parent, mezzi, dest);
    } else {
        printf("\nTempo limite superato!");
    } 
    printf("\n");
}

//RESTITUISCE L'ORARIO DI ARRIVO MINIMO TRA QUELLI DISPONIBILI PER UNA DATA TRATTA (SRC,DEST) A PARTIRE DALL'ORA CORRENTE
int find_min_orario (int src, int dest, int ora_attuale, nodo_partenza **vicini, int *mezzi) {
    nodo_partenza n = vicini[src][dest];
    int min = INT_MAX;
    orari *list_orari = n.orari_partenza;
    while (list_orari!=NULL) {
        if ((list_orari->ora_partenza >= ora_attuale) && (list_orari->ora_arrivo < min)) {
            min = list_orari->ora_arrivo;
            mezzi[dest] = list_orari->mezzo;
        }
        list_orari = list_orari->next;
    }
    //CONTROLLA SE ORA_ARRIVO_MEZZI < ORA_ATTUALE + TEMPO_PERCORRENZA_PIEDI -- SCEGLI TRA IL MIGLIOR COLLEGAMENTO CON MEZZO O A PIEDI
	//SE NON ESISTE UN COLLEGAMENTO CON MEZZO, LA LISTA ORARI SARA' NULL QUINDI NON ENTRA NEL WHILE --> MIN == INT_MAX == INFINITO QUINDI VIENE PRESO IL PERSORSO A PIEDI 
	//LO STESSO VALE SE NON ESISTE IL PERCORSO A PIEDI --> WALK_TIME == INT_MAX == INFINITO --> VIENE PRESO IL PERCORSO CON MEZZO MIN
	if ( (ora_attuale + n.walk_time) < min ) {
		min = ora_attuale + n.walk_time;
		mezzi[dest] = 8;
	}
    return min;
}

void dijkstra(int graph[dim][dim], int ora_partenza, int tempo_limite, int src, int dest, nodo_partenza **vicini) {

	int dist[dim]; //ORA DI ARRIVO MINIMO DA SRC A DEST
	bool sptSet[dim]; //TRUE SSE i FA PARTE DELL'ALBERTO DEI CAMMINI MINIMI
	int parent[dim]; //ARRAY DI PADRI PER SALVARE ALBERO CAMMINI MINIMI
    int mezzi[dim]; //ARRAY DEI MEZZI USATI - mezzi[j] == mezzo usato per ARRIVARE a j 
    int ora_attuale = ora_partenza;

    for (int i = 0; i < dim; i++) 
    { 
        dist[i] = INT_MAX; 
        sptSet[i] = false; 
    } 
    parent[src]=-1;
    mezzi[src]=-1;
  
    //PARTO DA SRC ALL'ORA DI PARTENZA --> ALL'INIZIO ORA ATTUALE == ORA PARTENZA
    dist[src] = ora_attuale; 
  
    for (int count = 0; count < dim - 1; count++) { 
         
        int u = minDistance(dist, sptSet); 
        ora_attuale = dist[u];
   
        sptSet[u] = true; 
   
        for (int v = 0; v < dim; v++) {//VISITO I VICINI
  
            int costo;
            if (!sptSet[v] && graph[u][v] && (costo=find_min_orario(u,v,ora_attuale,vicini,mezzi)) < dist[v] ) {  
                parent[v] = u; 
                dist[v] = costo; 
            }  
        }
    } 
  
    printSolution(src, dest, dist, dim, parent, mezzi, ora_partenza, tempo_limite);
} 

//RESTITUISCE L'INDICE DEL NODO CORRISPONDENTE ALLA FERMATA SE ESISTE, -1 ALTRIMENTI 
int get_index (string nomi_fermate[dim], string fermata) {
    for (int i=0;i<dim;i++) {
        if (nomi_fermate[i]==fermata) return i;
    }
    return -1;
}

void print_mezzo (int id_mezzo) {
    switch (id_mezzo) {
    case 0:
        printf("tram ");
        break;
    case 1:
        printf("metropolitana ");
        break;
    case 2:
        printf("treno ");
        break;
    case 3:
        printf("autobus ");
        break;
    case 4:
        printf("traghetto ");
        break;
    case 5:
        printf("funivia ");
        break;
    case 6:
        printf("funivia sospesa ");
        break;
    case 7:
        printf("funicolare ");
		break;
	case 8:
		printf("piedi ");
		break;
    default:
        printf ("Mezzo non riconosciuto!\n");
        break;
    }
}

void print_date (time_t rawtime) {
	
	struct tm ts;
	char buf[80];

	//FORMATTAZIONE ddd yyyy-mm-dd hh:mm:ss zzz
	ts = *localtime(&rawtime);
	strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
	printf("%s\t\t", buf);
}

int convert_date (char * date) {

	char * data = strtok(date," ");
	char * orario = strtok(NULL," ");

	if (data==NULL | orario==NULL) {
		return -1;
	}

	struct tm t;
	time_t t_of_day = 0;

	//DATA
	int anno = atoi(strtok(data,"-"));
	t.tm_year = anno-1900;
	int mese = atoi(strtok(NULL,"-"));
	t.tm_mon = mese - 1; //VANNO DA 0 A 11, MA DA INPUT PRENDO DA 0 A 12
	int giorno = atoi(strtok(NULL,"-"));
	t.tm_mday = giorno;
	//ORARIO
	int ore = atoi(strtok(orario,":"));
	t.tm_hour = ore;
	int min = atoi(strtok(NULL,":"));
	t.tm_min = min;
	int sec = atoi(strtok(NULL,":"));
	t.tm_sec = sec;
	t.tm_isdst = -1;
	t_of_day = mktime(&t);

	return (int) t_of_day;
	
}
