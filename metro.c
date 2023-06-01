#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include <unistd.h>
#define STAT 304
#define LIAISON 735

/* structures metro station et voisin */
typedef struct voisin_s{ 
    int id_voisin;
    char ligne[5];
    struct voisin_s *suivant;
}voisin;

typedef struct {
    char nom[50];
    //int id;
    voisin *premier;
    int nb_voisins;
}station;

typedef struct {
    station tab[STAT];
    int nb_stations; 
}metro;

typedef struct {
  char ligne[5];
  int indice_pred ;
  int cout;
  int marque;
}predecesseur;



/* structure pile et maillon pour l'algorithme */
typedef struct maillon_s {
    int num_st;
    int duree;
    struct maillon_s *suivant;
}maillon;

typedef struct {
    maillon * sommet;
    int taille;
    int duree_totale;
}pile;

/* prototypes des fonctions */
/* creer maillon et creer pile */
maillon *creer_maillon(int num_st);
pile *creer_pile();
/* fonctions pour metro station et voisin */
metro * creer_metro();
voisin *creer_voisin(int id_voisin, char *ligne);
void ajouter_voisin(metro *m, int dep, int arv, char *ligne);
/* fonction qui extrait les données stochées dans les deux fichiers et les stocke dans la structure métro */
void extraire_donnee(char* _stationFileName, char * _edgeFileName, metro * _metro);
void mettre_a_jour_voisins(station st, predecesseur *tab[], char * ligne_actuelle, int ind_station_actuelle);
void initialisation_algorithme();
void algorithme_plusCourtChemin(metro *m, int depart, int arrivee);
/* fonction de test */
void Test(); 
/* libération de mémoire de la structure metro */
void free_metro(metro * _metro);


void afficherStationEtVoisins(station _station, metro _metro){
    printf("la station est : %s\n", _station.nom);
    printf("et a comme voisin : \n");
    voisin * vs = _station.premier;
    while(vs != NULL){
        printf("\tStation \"%s\" de la ligne %s\n", _metro.tab[vs->id_voisin].nom, vs->ligne);
        vs = vs->suivant;
    }
}

/* algorithme */
int main() {
    //Test();
    initialisation_algorithme();
}
 /* creer maillon et creer pile */
maillon *creer_maillon(int num_st) {
    maillon *m = malloc(sizeof(maillon));
    m->suivant = NULL;
    return m;
}
pile *creer_pile() {
    pile *p = malloc(sizeof(pile));
    p->sommet = NULL;
    p->taille = 0;
    return p;
}
/* fonctions pour metro station et voisin */
metro * creer_metro(){
    metro * m = malloc(sizeof(metro));
    m->nb_stations = 0;
    for(int i = 0; i < STAT; i++) {
        m->tab[i].premier = NULL;
        m->tab[i].nb_voisins = 0;
    }
    m->nb_stations = STAT;
    return m;
}

voisin *creer_voisin(int id_voisin, char *ligne) {
    voisin *vs = malloc(sizeof(voisin));
    vs->id_voisin = id_voisin;
    strcpy(vs->ligne, ligne); 
    vs->suivant = NULL;
    return vs;
}

void ajouter_voisin(metro *m, int dep, int arv, char *ligne) { 
    voisin *vs = creer_voisin(arv, ligne);
    vs->suivant = m->tab[dep].premier; 
    m->tab[dep].premier = vs;
    m->tab[dep].nb_voisins++; 
}

/* extraire les données des fichiers */
void extraire_donnee(char* _stationFileName, char * _edgeFileName, metro * _metro) {
    //Ouverture du fichier _stationFileName, et extraction de ses données vers le tableau contenu dans _metro
    FILE * f_station = fopen(_stationFileName,"r");
    if(f_station == NULL){
        printf("extraire_donnee : Erreur lecture fichier f_station\n");
        assert(0);
    }
    
    char *name;
    char * id;
    char e;
    e = fscanf(f_station, "%*[^\n]\n");
    while(!feof(f_station)){
        char data[50];
        fgets(data,50,f_station);
        name = strtok(data, ",");
        id = strtok(NULL, ",");
        //printf("%s----%s\n", data, id);
        strcpy(_metro->tab[atoi(id)- 1].nom, data);
    }
 
    //Ouverture du fichier _edgeFileName et extraction de ses données vers des maillons voisions
    //puis ajoute les voisins aux stations
    FILE * f_edge = fopen(_edgeFileName, "r");
    if(f_edge == NULL){
        printf("extraire_donnee : Erreur lecture fichier f_edge\n");
        assert(0);
    }
    int start, end;
    char line[5];
    e = fscanf(f_edge, "%*[^\n]\n");
    while(!feof(f_edge)){
        e = fscanf(f_edge, "%d,%d,%s\n", &start, &end, line);
        //printf("start : %d, end : %d, ligne : %s\n", start, end, line);
        ajouter_voisin(_metro, start - 1, end - 1, line);
    }
    
    
    fclose(f_edge);
    fclose(f_station);
} 

void Test(){
    metro * m = creer_metro();
    extraire_donnee("Metro Paris Data - Stations.csv", "Metro Paris Data - Aretes.csv", m);
    for(int i = 0; i <10 ; i++){
        printf("\"%s\" - %d\n",m->tab[i].nom, i);
    }
    voisin * vs = m->tab[0].premier;
    free_metro(m);
}

void free_metro(metro * _metro){ 
    for(int i = 0; i < STAT; i++){
        voisin * vs = _metro->tab[i].premier, *tmp;
        while(vs != NULL){
            tmp = vs->suivant;           
            free(vs);
            vs = tmp;
        }
    }
    free(_metro);

}

void algorithme_plusCourtChemin(metro *m, int depart, int arrivee) {
    predecesseur *tab[STAT];
    for(int i = 0; i < STAT; i++) 
        tab[i] = NULL;
    tab[depart] = malloc(sizeof(predecesseur));
   
    char ligne_depart[4] = "null";
    tab[depart]->indice_pred = depart;
    tab[depart]->cout = 0;
    tab[depart]->marque = 1; // sommet de départ marqué
    strcpy(tab[depart]->ligne, ligne_depart);
    mettre_a_jour_voisins(m->tab[depart], tab, ligne_depart, depart);
    int min = 100000;
    int ind_min = depart;

    int fini = 0;
    
    while(!fini) {  
    	min = 100000; 
        for(int i = 0; i < STAT; i++) {
            if(tab[i] != NULL && tab[i]->marque == 0) {
                if(tab[i]->cout < min) {
                    min = tab[i]->cout;
                    ind_min = i; 
                }
            }
        }       

        tab[ind_min]->marque = 1;
        if(tab[arrivee] != NULL && tab[arrivee]->marque == 1)//modification condition : "marque != 1" => "marque == 1" 
        	fini = 1;                                        
        mettre_a_jour_voisins(m->tab[ind_min], tab, tab[ind_min]->ligne, ind_min);
    }
    printf("la durée du trajet est : %d\n", tab[arrivee]->cout);
    //si besoin pour afficher le tab       
    for(int i = 0; i < STAT; i++) {
    	if(tab[i] != NULL && tab[i]->marque == 1) {
    		printf("ligne(%s),station(%s),prededecesseur(%s),cout(%d),marque(%d)\n",tab[i]->ligne, m->tab[i].nom, m->tab[tab[i]->indice_pred].nom, tab[i]->cout, tab[i]->marque);
    	}
    }
        
    /* affichage du trajet */
    int i = arrivee;
    while(i != depart) {
        printf("minute %d : arrivé à la station \"%s\" avec la ligne %s\n",tab[i]->cout,m->tab[i].nom, tab[i]->ligne);
        printf("%d\n",i + 1);
    	i = tab[i]->indice_pred;   	
    }
    printf("minute 0 : Station de départ : %s\n", m->tab[i].nom);
}

//prototype

/*void mettre_a_jour_voisins(station st, predecesseur *tab[], char * ligne_actuelle, int ind_station_actuelle){
    voisin * vs = st.premier;
    char ligne_depart[4] = "null";
    /* si nous sommes dans le premier cas (station de départ) 
    if(!strcmp(ligne_depart, ligne_actuelle)){
        printf("La station de départ est : %s\n", st.nom);
        while(vs != NULL){
            int id = vs->id_voisin;
            printf("id_voisin = %d\n",id + 1);
            tab[id] = malloc(sizeof(predecesseur));
            
            strcpy(tab[id]->ligne,vs->ligne);
            tab[id]->indice_pred[0] = ind_station_actuelle;
            tab[id]->cout = 1;
            tab[id]->marque = 0;
            vs = vs->suivant;
        }     
        
    }
    else {
        while(vs != NULL){
            int id = vs->id_voisin;
            if(tab[id] == NULL) { // si la case n'a jamais été initialisé
            	tab[id] = malloc(sizeof(predecesseur));
                strcpy(tab[id]->ligne, vs->ligne);
            	tab[id]->indice_pred[0] = ind_station_actuelle;                     
	            tab[id]->marque = 0;
            }
            else{ 
                int i = 0;
                while(tab[id]->indice_pred[i] != "-1")
                    i ++;
                tab[id]->indice_pred[i] = ind_station_actuelle;           
	        }
            vs = vs->suivant;
        }
        int min= 10000;
        int indice;
        int indice_pred;
        for(int i = 0; i < 10; i++){
            min = 10000;
            indice_pred = tab[ind_station_actuelle]->indice_pred[i];
            if (indice_pred != -1){
                for (int j = 0; i < 10; i++){
                    int indice_pred_pred = tab[indice_pred]->indice_pred[j];
                    if(indice_pred_pred != -1){
                        if (!strcmp(tab[indice_pred_pred].ligne, tab[indice_pred].ligne)){
                            if (tab[indice_pred_pred]->cout + 1 < min){
                                min = tab[indice_pred_pred]->cout + 1;
                                indice = j;
                            }
                        }
                        else{
                            if (tab[indice_pred]->cout + 6 < min){
                                min = tab[indice_pred_pred]->cout + 6;
                                indice = j;
                            }
                        }
                        tab[i]->cout = min;
                        strcpy(tab[i]->ligne,tab[i]->indice_pred[indice]->ligne);
                    }
                }
            }
        }
    }
}*/

void mettre_a_jour_voisins(station st, predecesseur *tab[], char * ligne_actuelle, int ind_station_actuelle){
    voisin * vs = st.premier;
    char ligne_depart[4] = "null";
    // si nous sommes dans le premier cas (station de départ) 
    if(!strcmp(ligne_depart, ligne_actuelle)){
        printf("La station de départ est : %s\n", st.nom);
    	printf("nb = %d\n",st.nb_voisins);
        while(vs != NULL){
            int id = vs->id_voisin;
            printf("id_voisin = %d\n",id + 1);
            tab[id] = malloc(sizeof(predecesseur));
            
            strcpy(tab[id]->ligne,vs->ligne);
            tab[id]->indice_pred = ind_station_actuelle;
            tab[id]->cout = 1;
            tab[id]->marque = 0;
            vs = vs->suivant;
        }     
        
    }
    else {
        while(vs != NULL){
            int id = vs->id_voisin;
            if(tab[id] == NULL) { // si la case n'a jamais été initialisé
            	tab[id] = malloc(sizeof(predecesseur));
            	strcpy(tab[id]->ligne,vs->ligne);
            	tab[id]->indice_pred = ind_station_actuelle;
            	if(!strcmp(vs->ligne, ligne_actuelle))
                	tab[id]->cout = tab[ind_station_actuelle]->cout + 1;
	            else    
	                tab[id]->cout = tab[ind_station_actuelle]->cout + 6;
	            tab[id]->marque = 0;
            }
            else { // si la case a déjà été initialisé 
            	int cout1;
            	// on stocke le cout
            	if(!strcmp(vs->ligne, ligne_actuelle))
                	cout1 = tab[ind_station_actuelle]->cout + 1;
	            else    
	                cout1 = tab[ind_station_actuelle]->cout + 6;
	            // si le nouveau cout est inférieur a celui déjà stocké => on le met a jour
	            // il ne doit pas être mis a jour si déjà marqué
	            if(cout1 < tab[id]->cout && tab[id]->marque == 0){
	            	strcpy(tab[id]->ligne,vs->ligne);
	            	tab[id]->indice_pred = ind_station_actuelle;
	            	if(!strcmp(vs->ligne, ligne_actuelle))
	                	tab[id]->cout = tab[ind_station_actuelle]->cout + 1;
		            else    
		                tab[id]->cout = tab[ind_station_actuelle]->cout + 6;
		            tab[id]->marque = 0;   
	            }
            }
            vs = vs->suivant;
        }
        
    }
}



void initialisation_algorithme(){
    metro * _metro = creer_metro();
    int st1 = 0, st2 = 0;
    extraire_donnee("Metro Paris Data - Stations.csv", "Metro Paris Data - Aretes.csv", _metro);
    for(int i = 0; i <STAT ; i++){
        printf("\"%s\" - %d\n",_metro->tab[i].nom, i + 1);
    }

    afficherStationEtVoisins(_metro->tab[237], *_metro);

    printf("Veuillez saisir l'id des stations entre 1 et 304 inclus :\n");
    while((st1 < 1 || st1 > 304) || (st2 < 1 || st2 > 304)){
        printf("Entrez la station de départ : ");
        scanf("%d", &st1);
        printf("Entrez la station d'arrivée :");
        scanf("%d", &st2);
    }
    
    
    algorithme_plusCourtChemin(_metro, st1 - 1, st2 - 1);
}

