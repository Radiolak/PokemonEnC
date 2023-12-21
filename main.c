#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_video.h>
#define SDL_HWSURFACE 0x00000001
#define SDL_DOUBLEBUF 0x40000000

/* Paramètres de la fenêtre : */
const int largeur = 800;
const int hauteur = 600;
const char * titre = "ESGI fight";
SDL_Surface * ecran = NULL; 

/* Statistiques d'un personnage : */
typedef struct Stats Stats;
struct Stats {
    int vie;
    int atk;
    int def;
    int vit;
};

/* Codes de dessin des formes : */
typedef enum {
    S_NONE    = 0,
    S_LINE    = 1,
    S_CIRCLE  = 2,
    S_POLYGON = 3
} Shape;

typedef struct Action Action;
struct Action {
    char type[64];
    void *nodeData; // Champ libre modélisant un nœud
    Action *left;
    Action *right;
    Action *next;
};
int Action_read(Action ** action, const char * start, const char * end) {
	/* TODO : lecture d'une action depuis une sous-chaîne de caractères */
	    // Vérification des entrées
		
    if (start == NULL || end == NULL || start >= end) {
        fprintf(stderr, "Invalid input parameters for Action_read.\n");
        return 0; // Échec
    }
	    // Initialisation des champs de l'action
    *action = (Action *)malloc(sizeof(Action));
    (*action)->left = NULL;
    (*action)->right = NULL;
    (*action)->next = NULL;
	return 1;
}

void Action_debug(const Action * action) {
	/* TODO : affichage debug d'une action : vérification de l'expression */
    if (action == NULL) {
        printf("NULL Action\n");
        return;
    }

    printf("Action : ");

    if (action->left != NULL && action->right != NULL) {
        printf("Expression (");
        Action_debug(action->left);
        printf(", ");
        Action_debug(action->right);
        printf(")");
    } else {
        printf("Action détaillée non implémentée");
    }

    printf("\n");
}
float Action_eval(const Action * action, Stats * self_stats, Stats * other_stats) {
	/* TODO : évaluation d'un action */
    if (action == NULL || self_stats == NULL || other_stats == NULL) {
        return 0.f;
    }

    if (strcmp(action->type, "Cogner") == 0) { 
        other_stats->vie -= (self_stats->atk * 12.5 + 50) / other_stats->def + 1;
        return 1.f; 
    }
}

/* Sort d'un personnage : */
typedef struct Capacite Capacite;
struct Capacite {
	char nom[64]; /* Nom du sort */
	char message[1024]; /* Message à afficher à l'utilisation */
	Action * action; /* Action à déclencher */
};

int Capacite_load(const char * path, Capacite * cap) {
    if (path == NULL || cap == NULL) {
        fprintf(stderr, "Capacité introuvable\n");
        return 0;
    }

    FILE * fichier = fopen(path, "r");
    if (fichier == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier %s.\n", path);
        return 0;
    }

    if (fscanf(fichier, "nom %63[^\n]\n", cap->nom) != 1) {
        fprintf(stderr, "Erreur de lecture du nom de la capacité dans le fichier %s.\n", path);
        fclose(fichier);
        return 0;
    }

    if (fscanf(fichier, "message %1023[^\n]\n", cap->message) != 1) {
        fprintf(stderr, "Erreur de lecture du message de la capacité dans le fichier %s.\n", path);
        fclose(fichier);
        return 0;
    }

    cap->action = (Action *)malloc(sizeof(Action));
    if (cap->action == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour l'action de la capacité.\n");
        fclose(fichier);
        return 0;
    }

    char expression[256]; 
    if (fscanf(fichier, "action %[^\n]\n", expression) != 1) {
        fprintf(stderr, "Erreur de lecture de l'expression de l'action dans le fichier %s.\n", path);
        fclose(fichier);
        free(cap->action);
        return 0;
    }

    if (!Action_read(&cap->action, expression, expression + strlen(expression))) {
        fprintf(stderr, "Erreur lors de la conversion de l'expression de l'action dans le fichier %s.\n", path);
        fclose(fichier);
        free(cap->action);
        return 0;
    }

    fclose(fichier);
    return 1;
}

void Capacite_debug(const Capacite * cap) {
	/* TODO : affichage debug d'une capacité */
    if (cap == NULL) {
        fprintf(stderr, "Capacité inexistante\n");
        return;
    }

    printf("Nom de la capacité : %s\n", cap->nom);
    printf("Message de la capacité : %s\n", cap->message);

    if (cap->action != NULL) {
        printf("Action de la capacité :\n");
        Action_debug(cap->action);
    } else {
        printf("Erreur : action de la capacité non définie.\n");
    }

}

typedef struct Personnage Personnage;
struct Personnage {
	char name[50]; /* nom du personnage */
	Stats start; /* statistiques initiales */
	Stats current; /* statistiques actuelles */
	unsigned char face[1024]; /* codage du dessin de face */
	unsigned char back[1024]; /* codage du dessin de dos */
	Capacite capacites[4]; /* capacités */
	int nb_caps; /* nombre de capacités disponibles */
};

Personnage joueur;
Personnage adversaire;
// Dessin des statistiques d'un personnage :

void Personnage_afficher_stats(const Personnage * perso, int x, int y) {
	/* TODO : afficher les barres de stats d'un personnage depuis l'origine (x, y) */
	int sx = x - largeur / 4;
	int sy = y - hauteur / 9;
	int ex = x + largeur / 4;
	int ey = y + hauteur / 13;
	roundedBoxRGBA(ecran, sx, sy, ex, ey, 20, 255, 255, 255, 255);
}
// Dessin d'un personnage depuis une image vectorielle :
void draw_data(unsigned char * data, int cx, int cy, int s) {
	/* TODO : dessiner un modèle vectoriel (lu au chargement d'un personnage) */
	filledCircleRGBA(ecran, cx, cy, s / 2, 127, 127, 127, 255);
}

int Personnage_load(Personnage *perso, const char *path) {
	/* TODO : charger un personnage */

    FILE *fichier = fopen(path, "r");
    if (fichier == NULL) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier %s.\n", path);
        return 0;
    }

    // Lecture du nom du personnage
    if (fscanf(fichier, "name %49[^\n]\n", perso->name) != 1) {
        fprintf(stderr, "Erreur de lecture du nom du personnage dans le fichier %s.\n", path);
        fclose(fichier);
        return 0;
    }

    // Lecture des statistiques du personnage
if (fscanf(fichier, "vie %d\n", &perso->Stats.vie) != 1 ||
    fscanf(fichier, "atk %d\n", &perso->Stats.atk) != 1 ||
    fscanf(fichier, "def %d\n", &perso->Stats.def) != 1 ||
    fscanf(fichier, "vit %d\n", &perso->Stats.vit) != 1) {
    fprintf(stderr, "Erreur de lecture des statistiques du personnage dans le fichier %s.\n", path);
    fclose(fichier);
    return 0;
}
	// Lecture des capacités du personnage
    for (int i = 0; i < perso->nb_caps; ++i) {
        char capPath[256];  // Ajustez la taille selon vos besoins
        if (fscanf(fichier, "load %255[^\n]\n", capPath) != 1) {
            fprintf(stderr, "Erreur de lecture du chemin de capacité %d dans le fichier %s.\n", i + 1, path);
            fclose(fichier);
            return 0;
        }

        // Chargez la capacité à partir du chemin spécifié
        if (!Capacite_load(capPath, &perso->capacites[i])) {
            fprintf(stderr, "Erreur lors du chargement de la capacité %d à partir du fichier %s.\n", i + 1, capPath);
            fclose(fichier);
            return 0;
        }
    }    
	fclose(fichier);
    return 1;
}

void affichage() {
	/* Remplissage de l'écran par un gris foncé uniforme : */
	SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 51, 51, 102));
	
	Personnage_afficher_stats(&adversaire, 2 * largeur / 7, hauteur / 5);
	Personnage_afficher_stats(&joueur, 5 * largeur / 7, 3 * hauteur / 5);
	draw_data(joueur.back, 3 * largeur / 14, 3 * hauteur / 5, hauteur / 3);
	draw_data(adversaire.face, 11 * largeur / 14, hauteur / 5, hauteur / 3);
}

int afficher_choix(int mx, int my, const Personnage  * perso) {
	roundedBoxRGBA(ecran, 0, 3 * hauteur / 4, largeur, hauteur, 15, 204, 204, 204, 255);
	
	int cap = -1;
	int i, x, y;
	if(mx > 0 && mx < largeur && my > 3 * hauteur / 4 && my < hauteur) {
		x = mx / (largeur / 2);
		y = (my - 3 * hauteur / 4) / (hauteur / 8);
		cap = 2 * y + x;
		if(cap < perso->nb_caps) {
			roundedBoxRGBA(ecran, x * largeur / 2, 3 * hauteur / 4 + y * hauteur / 8, (x + 1) * largeur / 2, 3 * hauteur / 4 + (y + 1) * hauteur / 8, 15, 255, 255, 255, 255);
		} else {
			cap = -1;
		}
	}
	for(i = 0; i < perso->nb_caps; ++i) {
		x = i % 2;
		y = i / 2;
		roundedRectangleRGBA(ecran, x * largeur / 2, 3 * hauteur / 4 + y * hauteur / 8, (x + 1) * largeur / 2, 3 * hauteur / 4 + (y + 1) * hauteur / 8, 15, 102, 102, 102, 255);
		stringRGBA(ecran, (x + 0.25) * largeur / 2, 3 * hauteur / 4 + (y + 0.5) * hauteur / 8, perso->capacites[i].nom, 51, 51, 51, 255);
	}
	return cap;
}
// Jouer et terminer un combat : (J'ai aussi complèté la fonction "opponent_turn" et "finished" )
void player_turn(int cap) {
    if (cap >= 0 && cap < joueur.nb_caps) {
        Capacite *selectedCap = &joueur.capacites[cap];

        // Afficher le message de la capacité
        printf("%s utilise %s!\n", joueur.name, selectedCap->nom);

        // Applique les capacités associés à l'action
        if (selectedCap->action != NULL) {
            float damage = Action_eval(selectedCap->action, &joueur.current, &adversaire.current);

            // Mise à jour des stats de l'adversaire
            adversaire.current.vie -= damage;

            // Effets de l'action
            printf("%s inflige %f dégâts à %s!\n", joueur.name, damage, adversaire.name);
        } else {
            printf("Erreur : action de la capacité non définie.\n");
        }
    } else {
        printf("Erreur : capacité invalide.\n");
    }
}


void opponent_turn() {
    // choix une capacité aléatoirement
    int randomCapIndex = rand() % adversaire.nb_caps;
    Capacite *selectedCap = &adversaire.capacites[randomCapIndex];

    printf("%s utilise %s!\n", adversaire.name, selectedCap->nom);

    // Appliquez l'action associée à la capacité choisie
    if (selectedCap->action != NULL) {
        float damage = Action_eval(selectedCap->action, &adversaire.current, &joueur.current);

        // Mise à jour des stats des joueurs
        joueur.current.vie -= damage;

        // Effets de l'action
        printf("%s inflige %f dégâts à %s!\n", adversaire.name, damage, joueur.name);
    } else {
        printf("Erreur : action de la capacité non définie.\n");
    }
}


void display_winner() {
	char buffer[1024];
	if(adversaire.current.vie <= 0) {
		roundedBoxRGBA(ecran, largeur / 8, hauteur / 4, 7 * largeur / 8, 3 * hauteur / 4, 15, 0, 53, 0, 204);
		sprintf(buffer, "Victoire de %s", joueur.name);
	} else {
		roundedBoxRGBA(ecran, largeur / 8, hauteur / 4, 7 * largeur / 8, 3 * hauteur / 4, 15, 53, 0, 0, 204);
		sprintf(buffer, "Defaite de %s", joueur.name);
	}
	stringRGBA(ecran, 2 * largeur / 8, hauteur / 2, buffer, 255, 255, 255, 255);
}

int finished() {
/* TODO : condition de fin du combat */
    if (joueur.current.vie <= 0 || adversaire.current.vie <= 0) {
        return 1; // Le combat est terminé
    } else {
        return 0; // Le combat continue
    }
}
void SDL_WM_SetCaption(const char *title, const char *icon);
//void SDL_Flip(SDL_Surface *screen);


int main(int argc, char *argv[]) {
    srand(time(NULL));
	SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);
    /* Création d'une fenêtre SDL : */
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error in SDL_Init : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    if((ecran = SDL_SetVideoMode(largeur, hauteur, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL) {
        fprintf(stderr, "Error in SDL_SetVideoMode : %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    SDL_WM_SetCaption(titre, NULL);

    int active = 1;
    SDL_Event event;

    if(!Personnage_load(&joueur, "first.perso")
       || !Personnage_load(&adversaire, "second.perso")) {
        goto end;
    }

    // Charger les capacités pour les deux personnages
    for (int i = 0; i < joueur.nb_caps; ++i) {
        Capacite_load(joueur.capacites[i].nom, &joueur.capacites[i]);
    }
    for (int i = 0; i < adversaire.nb_caps; ++i) {
        Capacite_load(adversaire.capacites[i].nom, &adversaire.capacites[i]);
    }

    int last_mouse_x = 0;
    int last_mouse_y = 0;
    int cap = -1;
    int select_cap = 0;

    while(active) {
        affichage();
        cap = afficher_choix(last_mouse_x, last_mouse_y, &joueur);
        SDL_Flip(ecran);

        while(SDL_PollEvent(&event)) {

            switch(event.type) {
                /* Utilisateur clique sur la croix de la fenêtre : */
                case SDL_QUIT : {
                    active = 0;
                } break;

                /* Utilisateur enfonce une touche du clavier : */
                case SDL_KEYDOWN : {
                    switch(event.key.keysym.sym) {
                        /* Touche Echap : */
                        case SDLK_ESCAPE : {
                            active = 0;
                        } break;

                        default : break;
                    }
                } break;

                case SDL_MOUSEMOTION : {
                    last_mouse_x = event.motion.x;
                    last_mouse_y = event.motion.y;
                } break;

                case SDL_MOUSEBUTTONUP : {
                    select_cap = 1;
                } break;

                default : break;
            }
        }

        if(select_cap && cap >= 0 && cap < joueur.nb_caps) {
            if(joueur.current.vit >= adversaire.current.vit) {
                player_turn(cap);
                opponent_turn();
            } else {
                opponent_turn();
                player_turn(cap);
            }
            active = !finished();

            select_cap = 0;

            if(!active) {
                display_winner();
                SDL_Flip(ecran);
                SDL_Delay(2000);
            }
        }

        SDL_Delay(1000 / 60);
    }

end:

    SDL_FreeSurface(ecran);
    SDL_Quit();
    exit(EXIT_SUCCESS);
}