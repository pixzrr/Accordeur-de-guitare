/*
 * P-Acc_PORT-DIAL-TP2.c
 *
 * Created: 13/03/2026 14:22:45
 * Author : 45018569
 */ 

#define F_CPU 3686000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// ********************************************************************************************************************************************* //
// Sommaire :

// ********* Section 1 : Presets  ************************************************************************************************************** //
// ********* Section 2 : Prototypes ************************************************************************************************************ //
// ********* Section 3 : Variables globales  *************************************************************************************************** //
// ********* Section 4 : Main  ***************************************************************************************************************** //
// ********* Section 5 : Fonctions primaires  ************************************************************************************************** //
// ********* Section 6 : Bargraphe  ************************************************************************************************************ //
// ********* Section 7 : Gestion des modes  **************************************************************************************************** //
// ********* Section 8 : Interruptions  ******************************************************************************************************** //
// ********* Section 9 : Timers  *************************************************************************************************************** //
// ********* Section 10 : UI et autre  ********************************************************************************************************* //

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 1 : Définition des Constantes  ******************************************************************************************** //
// ********************************************************************************************************************************************* //

#define CLEAR_DISPLAY				0x01

#define RETURN_HOME				0x02

#define ENTRY_MODE_SET			0x04
	#define INCREMENT				0x02
	#define DECREMENT				0x00
	#define ENTIRE_SHIFT_ON			0x01
	#define ENTIRE_SHIFT_OFF		0x00
	
#define DISPLAY_ON_OFF_CONTROL	0x08
	#define DISPLAY_OFF				0x00
	#define DISPLAY_ON				0x04
	#define CURSOR_OFF				0x00
	#define CURSOR_ON				0x02
	#define BLINK_OFF				0x00
	#define BLINK_ON				0x01
	
#define FUNCTION_SET			0x30
	#define _1_LINE_MODE			0x00
	#define _2_LINE_MODE			0x08
	#define _5x8_DOTS				0x00
	#define _5x11_DOTS				0x04
	
#define LIGNE1					0x80
#define LIGNE2					(0x80 | 0x40)

#define E						0x00 // Autoriser l'envoi de la donnée (0 si on ne veut rien envoyer, 1 si on veut envoyer une donnée
#define RW						0x01 // Ecrire ou lire la donnée (0 pour envoyer, 1 pour lire) - bit 1 du port c
#define RS						0x02 // Savoir quelle donnee est echangée (RS = 0 si commande ou 1 si c'est un caractère) - bit 2 du port c

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 2 : Prototypes ************************************************************************************************************ //
// ********************************************************************************************************************************************* //
		
void LCD_init(void);
void LCD_sendcmd(char cmd);
void LCD_putchar(char c);

int LCD_putchars(char c, FILE *STREAM);
FILE donnee = FDEV_SETUP_STREAM(LCD_putchars, NULL, _FDEV_SETUP_WRITE); // on crée une variable de type FILE* (donc un nouvequ canal / nouvelle liaison) et on lui dit "Les écritures se font dans le canal LCD_putchars (où on retrouve la procédure d'envoi de donnée)

void bargaph (float freq_jouee, float freq_cible);
void test_bargraph(void);

void mode_MESURE(void);
void mode_SOUND(void);
void mode_DEBUG(void);

void init_int0(void);
void init_int1(void);
void init_int2(void);

void init_timer1();
void jouer_timer1(float freq);
void init_timer1_meas();

void init_timer0(void);
void jouer_timer0(float freq);
void stop_timer0();

void animation_demarrage(void);

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 3 : Variables globales et constantes associées  *************************************************************************** //
// ********************************************************************************************************************************************* //

volatile unsigned char mode = 0;

#define MODE_MESURE 0
#define MODE_SOUND 1
#define MODE_DEBUG 2

volatile unsigned char debug_mode = 0;
volatile unsigned char debug_mode_test_valide = 0;

#define NOMBRE_MODES_DEBUG 2 // ===== à changer en fonction du nombre de tests dans le menu =====

#define DEBUG_TESTS_UNIT 1
#define DEBUG_BARGRAPH 2

volatile unsigned char changement_mode_timer1 = 0;

volatile unsigned char front1 = 1;
volatile unsigned int periode_clk;
volatile unsigned char mesure_terminee;

// Tableaux des notes et fréquences

char note_cible[6][5] = {"Mi3", "Si2", "Sol2", "Re2", "La1", "Mi1"};
float freq_cible[6] = {329.6, 246.9, 196, 146.8, 110, 82.4};
volatile unsigned char choix_note = 0;

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 4 : Main  ***************************************************************************************************************** //
// ********************************************************************************************************************************************* //

void main(void) {

// ──────────── Partie 1 : Setup ───────────────────────────────────────── //
	
	LCD_init();
	
		// ___Ports LCD___ //						// (le port C sert à envoyer des instructions d'envoi / lecture et le port A sert à envoyer la donnée)
	DDRA = 0xFF;
	DDRC = 0xFF;

		//___Fonctions test___ //
	//tests_unitaires();
	//test_bargraph();

		// ___Gestion printf___ //	
	stdout = &donnee; 								// on dit au programme qu'on envoie le contenu de 'donnee' sur le PORT C (donc vers le LCD à la place du terminal)
	
		// ___Gestion INIT___ //
	init_int0();
	init_int1();
	init_int2();
	sei();
	
		// ___Gestion TIMER___ //
	init_timer0();
	
	// ___Gestion printf___ //
	stdout = &donnee; 								// on dit au programme qu'on envoie le contenu de 'donnee' sur le PORT C (donc vers le LCD à la place du terminal)

	//___Animation de démarrage__ //
	animation_demarrage();							// on attend encore un peu le temps que l'utilisateur appuie ou non sur BP2 (pour entrer en mode debug)
	
	// ___Gestion activation DEBUG___ //
	if ((PIND & (1<<PD2)) == 0) {
		mode = MODE_DEBUG;
		LCD_sendcmd(LIGNE1 | 3);
		printf(("Mode DEBUG"));
		LCD_sendcmd(LIGNE2);
		printf("Voir tests -->");
	}
	
// ──────────── Partie 2 : Loop (boucle while) ─────────────────────────── //
	
    while (1) {
		switch (mode) {

			case MODE_MESURE:
				if (mode != MODE_DEBUG) {
					mode_MESURE();
				}
				break;
		
			case MODE_SOUND:
				if (mode != MODE_DEBUG) {
					mode_SOUND();
				}
				break;
			
			case MODE_DEBUG:
				if (debug_mode != 0) mode_DEBUG();
				break;
		}		
    }
}

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 5 : Fonctions primaires  ************************************************************************************************** //
// ********************************************************************************************************************************************* //

// ──────────── Partie 1 : Initialisation du LCD ───────────────────────── //
		
void LCD_init(void) { // On se sert ici de la page 5 de la partie datasheet
	
	_delay_ms(100);
	
	LCD_sendcmd(FUNCTION_SET | _2_LINE_MODE | _5x8_DOTS); // On veut un écrant avec 5x8 points par caractère et 2 lignes pour afficher les caractères
	_delay_us(39);
	
	LCD_sendcmd(DISPLAY_ON_OFF_CONTROL | DISPLAY_ON | CURSOR_ON | BLINK_ON); // Pour CURSOR_ON on pourra mettre CURSOR_OFF quand la partie numérique sera terminée, pareil pour le blink
	_delay_us(39);
	
	LCD_sendcmd(CLEAR_DISPLAY); // On nettoie l'écran
	_delay_ms(2);
	
	LCD_sendcmd(ENTRY_MODE_SET | INCREMENT | ENTIRE_SHIFT_OFF); // Increment = ecrire de gauche à droite
	_delay_us(39);
}

// ──────────── Partie 2 : Envoyer une commande ────────────────────────── //

void LCD_sendcmd(char cmd) {
	PORTC = 0x00;						// On met RS et RW à 0 car on envoie une commande
	PORTC |= (1 << E);					// On autorise l'envoi de la donnée

	PORTA = cmd;						// On envoi la donnée sur le port A
	PORTC = ~(1 << E);					// On ferme notre fenêtre d'envoi
	_delay_ms(1.53);					// on attend le temps que la commande demandant le plus de temps d'éxecution se termine
}

// ──────────── Partie 3 : Envoyer un caractère ────────────────────────── //

void LCD_putchar(char c) {
	PORTC = 0x00 | (1<< RS);			// On met RS à 1 et RW à 0 car on envoie un caractère
	PORTC |= (1 << E);					// On autorise l'envoi de la donnée

	PORTA = c;							// On envoi la donnée sur le port A
	PORTC = ~(1 << E);					// On ferme notre fenêtre d'envoi
	
	_delay_us(43);						// Délai d'éxecution
}

// ──────────── Partie 4 : Envoyer une chaine de caractères ────────────── //

int LCD_putchars(char c, FILE *stream) {
	PORTC = 0x00 | (1<< RS);			// On met RS à 1 et RW à 0 car on envoie un caractère

	PORTA = c;							// On envoi la donnée sur le port A
	PORTC = ~(1 << E);					// On ferme notre fenêtre d'envoi
	
	
	_delay_us(43);						// Délai d'éxecution
	return 01; // 01 sera le code pour annoncer que la donnée a été envoyé
}

// ──────────── Partie 5 : Tests unitaires ─────────────────────────────── //

void tests_unitaires(void) {
	
	unsigned char caractere = 'A'; 		// 65 = A en code asquii
	
	LCD_init();
	
	while (caractere <= 'Z') { 			// 90 = Z en code asquii
		if (caractere == 'Q') LCD_sendcmd(LIGNE2); // Juste après la 16e lettre de l'aplhabet, on passe à la ligne (le LCD peut afficher jusqu'à 16 caractères par ligne)
		LCD_putchar(caractere);
		caractere++;
		_delay_ms(500);
	}
	LCD_sendcmd(CLEAR_DISPLAY);
	LCD_sendcmd(RETURN_HOME);
	_delay_ms(1000);
	
	printf("Accordeur 2026");
	_delay_ms(1000);
	
	LCD_sendcmd(0x80 | 0x40 | 4); 		// Ligne 2 colone 4
	_delay_ms(1000);
	
	printf("Ou suis-je ?");
	
	_delay_ms(1500);
}

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 6 : Bargraphe  ************************************************************************************************************ //
// ********************************************************************************************************************************************* //

// ──────────── Partie 1 : Fonction bargraphe ──────────────────────────── //

void bargaph (float freq_jouee, float freq_cible) {
	
	LCD_sendcmd(LIGNE2 | 7);
	_delay_us(39);
	printf("|");														// On place une barre verticale au milieu de la deuxième ligne
	
	float ecart = freq_jouee - freq_cible;
		
	if (ecart > 0.5) { // On met une petite marge parce qu'être parfaitement sur la bonne note est presque impossible
		LCD_sendcmd(LIGNE2 | 8);										// on se place juste après la barre verticale
		if (ecart <= 7) for (int i=0 ; i<ecart-0.5 ; i++) printf("+");
		if (ecart > 7) printf("+>>");
	}
		
	else if (ecart < -0.5) {
		LCD_sendcmd(LIGNE2 | 6);										// on se place juste avant la barre verticale
		LCD_sendcmd(ENTRY_MODE_SET | DECREMENT);						// et on règle le LCD en mode DECREMENT (de droite à gauche)
		if (ecart >= -7) for (int i=ecart-0.5 ; i<0 ; i++) printf("-");	// on enlève 0.5 pour l'arrondi
		if (ecart < -7) printf("-<<"); // <<- à l'envers
		LCD_sendcmd(ENTRY_MODE_SET | INCREMENT);						// on remet le LCD en mode INCREMENT
	}
	
	else {
		LCD_sendcmd(LIGNE2 | 6);
		printf("o|o");
	}
}

// ──────────── Partie 2 : Fonction test bargraphe ─────────────────────── //

void test_bargraph(void) {

	float freq_test[9] = {325.6, 325, 322.6, 322.5, 333.6, 334.4, 336.6, 336.7, 329.6};

	for (int i=0 ; i<9 ; i++) {
		LCD_sendcmd(CLEAR_DISPLAY);
	
		_delay_ms(2); // d'après la datasheet, il faut attendre au moins 1.53ms (on attendra ici 2ms)
		bargaph(freq_test[i], 329.6);
		
		LCD_sendcmd(LIGNE1);
		printf("Fc:329.6 F:%f", freq_test[i]);
	
	_delay_ms(1500);
	}

	// ----- Test d'unité négatif :						325.6-329.6 = -4			L'afficheur doit mettre 4 signes -
	
	// ----- Test d'arrondi néatif :					325-329.6 = -4.6			L'afficheur doit mettre 5 signes -
	
	// ----- Test de dépassement d'écran négatif :		322.6-329.6 = -7			L'afficheur doit mettre 7 signes -
	
	// ----- Test de dépassement d'écran négatif :		322.5-329.6 = -7.1			L'afficheur doit indiquer <<-
	
	// ----- Test d'unité positif :						333.6-329.6 = 4				L'afficheur doit mettre 4 signes +
	
	// ----- Test d'arrondi positif :					334.2-329.6 = 4.6			L'afficheur doit mettre 5 signes +
	
	// ----- Test de dépassement d'écran positif :		336.6-329.6 = 7				L'afficheur doit mettre 7 signes +
	
	// ----- Test de dépassement d'écran positif :		336.7-329.6 = 7.1			L'afficheur doit indiquer +>>
	
	// ----- Test fréquence parfaite :					329.6-329.6 = 0				L'afficheur doit indiquer o|o
}

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 7 : Gestion des modes  **************************************************************************************************** //
// ********************************************************************************************************************************************* //

// ──────────── Partie 1 : Mode MESURE ─────────────────────────────────── //
		
void mode_MESURE(void) {
	
	// ----- Initialisation du timer ---- //			// On  init qu'une seule fois le timer 0 (evite les glitch et erreurs de mesures (oui c'est arrivé et oui ça a été corrigé))

	if (changement_mode_timer1 == 0) {
		init_timer1_meas();
		changement_mode_timer1 = 1;
	}
	
	unsigned int freq_mesuree = 0;
	
	// ----- Mesurer fréquence ---------- //
	
	float fc = freq_cible[choix_note]*1.5*100;			// D'après la datasheet du filtre, nous avons fclk= fc * 100. De plus, nous voulons une fréquence de coupure comprise entre  F_fondamentale et F_harmonique2, donc entre F_fondamentale etF_fondamentale * 2. Nous ajoutons donc *1,5.
	
	jouer_timer0(fc);
	
	if (mesure_terminee == 1) {
		freq_mesuree = F_CPU / (1*periode_clk);
		mesure_terminee = 0;
	}
	
	// ----- Affichage ------------------ //
	
	LCD_init();
	
	LCD_sendcmd(LIGNE1 | 10);
	printf("(%s)", note_cible[choix_note]);
	
	LCD_sendcmd(LIGNE1);
	printf("Freq %d", freq_mesuree);
	
	bargaph(freq_mesuree, freq_cible[choix_note]); // afficher la fréquence mesurée
}

// ──────────── Partie 2 : Mode SOUND ──────────────────────────────────── //
		
void mode_SOUND(void) {
	
	stop_timer0();

	if (changement_mode_timer1 == 0) {
		init_timer1();
		changement_mode_timer1 = 1;
	}
	
	LCD_init();
	
	LCD_sendcmd(LIGNE1 | 3);
	printf("Mode SOUND");
	
	LCD_sendcmd(LIGNE2);
	printf("Note : %s", note_cible[choix_note]);
	
	jouer_timer1(freq_cible[choix_note]);
}

// ──────────── Partie 3 : Mode DEBUG ──────────────────────────────────── //

void mode_DEBUG(void) {
	
	LCD_init();
	
	
	if (debug_mode == DEBUG_TESTS_UNIT) {
		if (debug_mode_test_valide == 0) {
			LCD_sendcmd(LIGNE1);
			printf("Tests unitaires");
		
			LCD_sendcmd(LIGNE2 | 3);
			printf("Demarrer ->");
		
			LCD_sendcmd(LIGNE2 | 16);
			printf("1/%d", NOMBRE_MODES_DEBUG);
		}
		else {
			tests_unitaires();
			debug_mode_test_valide = 0;
		}
	}
	
	else if (debug_mode == DEBUG_BARGRAPH) {
		if (debug_mode_test_valide == 0) {
			LCD_sendcmd(LIGNE1);
			printf("Tests bargraph");
			
			LCD_sendcmd(LIGNE2 | 3);
			printf("Demarrer ->");
			
			LCD_sendcmd(LIGNE2 | 16);
			printf("2");
			printf("1/%d", NOMBRE_MODES_DEBUG);
		}
		else {
			test_bargraph();
			debug_mode_test_valide = 0;
		}
	}
	
	else {
		printf("Oups,");
		LCD_sendcmd(LIGNE2);
		printf("Ceci est un bug");
	}
}

// ********************************************************************************************************************************************* //









// ********************************************************************************************************************************************* //
// ********* Section 8 : Interruptions  ******************************************************************************************************** //
// ********************************************************************************************************************************************* //

// ──────────── Partie 1 : INT0 ────────────────────────────────────────── //

void init_int0(void) {
	DDRD &= ~(1<<PD2);
	
	GICR = GICR | (1<<INT0);
	MCUCR = MCUCR | (1<<ISC01);
}

ISR(INT0_vect) {
	if (mode != MODE_DEBUG) {
		if (choix_note<5) choix_note++;
		else choix_note = 0;
	}
	
	if (mode == MODE_DEBUG) debug_mode_test_valide = 1- debug_mode_test_valide;
}

// ──────────── Partie 2 : INT1 ────────────────────────────────────────── //

void init_int1(void) {
	DDRD &= ~(1<<PD3);
	
	GICR = GICR | (1<<INT1);
	MCUCR = MCUCR | (1<<ISC11);
}

ISR(INT1_vect) {
	if (mode != MODE_DEBUG) {
		mode = 1-mode;
		changement_mode_timer1 = 0;
	}
	
	if (mode == MODE_DEBUG) {
		if (debug_mode > NOMBRE_MODES_DEBUG-1) debug_mode = 1;
		else debug_mode++;
	}
}

// ──────────── Partie 3 : INT2 ────────────────────────────────────────── //

void init_int2(void) {
	DDRB &= (~(1<<PB2));
	
	GICR = GICR | (1<<INT2);
	MCUCSR = MCUCSR | (1<<ISC2); // ISC2 = 1 génère une interruption à chaque front montant (front descendant : ISC = 0)
}

ISR(INT2_vect) {
	if (front1 == 1) {			// si premier front, on remet le compteur TCNT1 à 0
		TCNT1 = 0;
		front1 = 0;
	}
	else {						// si deuxième front, on récupère la valeur de TCNT1 et on indique que la mesure est terminée
		periode_clk = TCNT1;
		mesure_terminee = 1;
		front1 = 1;
	}
}

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 9 : Timers  *************************************************************************************************************** //
// ********************************************************************************************************************************************* //

// ──────────── Partie 1 : Timer 1 ─────────────────────────────────────── //
		
void init_timer1() {

	DDRD |= (1<<PD5);
	
	TCCR1B = (1<<WGM12) | (1<<CS10);
	TCCR1A = (1<<COM1A0);
}

void jouer_timer1(float freq) {
	OCR1A = (F_CPU/(2*1*freq)-1) + 0.5;
}

void init_timer1_meas() {

	OCR1A = 0;
	TCCR1B = (1<<CS10);
	
	DDRD &= ~(1<<PD5);
}

// ──────────── Partie 2 : Timer 0 ─────────────────────────────────────── //

void init_timer0() {
	DDRB |= (1<<PB3);
	
	TCCR0 = (1<<COM00) | (1<<WGM01) | (1<<CS00);
}

void jouer_timer0(float freq) {
	TCCR0 |= (1<<CS00);
	
	OCR0 = (F_CPU/(2*1*freq)-1) + 0.5;
}

void stop_timer0() {
	TCCR0 &= (~(1<<CS00));
}

// ********************************************************************************************************************************************* //






// ********************************************************************************************************************************************* //
// ********* Section 10 : UI et autre  ********************************************************************************************************* //
// ********************************************************************************************************************************************* //

void animation_demarrage(void) {
	LCD_sendcmd(CLEAR_DISPLAY);
	LCD_sendcmd(RETURN_HOME);
	_delay_ms(100); // Si on met pas ce delay ça bug (au plus la tension délivrée dans le LCD est élevée, au plus il faudra attendre (delay réglé pour 4.5V max)
	

	
	LCD_sendcmd(LIGNE1 | 3);
	printf("Accordeur");
	
	_delay_ms(1000);
	

	LCD_sendcmd(LIGNE2 | 5);
	printf(".");
	_delay_ms(200);
	LCD_sendcmd(LIGNE2 | 5);
		
	LCD_sendcmd(LIGNE2 | 7);
	printf(".");
	_delay_ms(200);
	LCD_sendcmd(LIGNE2 | 7);
		
	LCD_sendcmd(LIGNE2 | 9);
	printf(".");
	_delay_ms(200);
	LCD_sendcmd(LIGNE2 | 9);

}

// ********************************************************************************************************************************************* //
