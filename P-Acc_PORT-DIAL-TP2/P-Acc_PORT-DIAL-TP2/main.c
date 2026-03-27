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


// ----- Define des constantes de fonction ----- //

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


		// ----- Liste des fonctions ----- //
		
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

void init_timer1();
void jouer_timer1(float freq);
void stop_timer1();



		// ----- Variables globales ----- //

volatile unsigned char mode_normal = 0;
volatile unsigned char debug_mode = 0;

volatile unsigned char choix_note = 0;



// ------------------------------------------------------ Main ------------------------------------------------------------- //
// ------------------------------------------------------------------------------------------------------------------------- //

void main(void) {
	LCD_init();
	// _________________Setup_________________ //
	
		// ___Ports LCD___ //													// (le port C sert à envoyer des instructions d'envoi / lecture et le port A sert à envoyer la donnée)
		
	DDRA = 0xFF;
	DDRC = 0xFF;
	
	// ___Gestion activation DEBUG___ //
	
	if (PIND & (1<<PD2) == 0) {
		debug_mode = 1;
		mode_DEBUG();
	}
	
	// ___Gestion INIT___ //
	
	init_int0();
	init_int1();
	sei();
	
	// ___Gestion TIMERS___ //
	
	 init_timer1();
	
	// ___Gestion printf___ //
	
	stdout = &donnee; // on dit au programme qu'on envoie le contenu de 'donnee' sur le PORT C (donc vers le LCD à la place du terminal)
	
	
	// ___Gestion MODES___ //
	
	
    while (1) 
    {
    }
}

// ------------------------------------------------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------------------------------------------------- //



		// ----- Initialisation du LCD ----- //
		
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




		// ----- Envoyer une commande ----- //

void LCD_sendcmd(char cmd) {
	PORTC = 0x00;						// On met RS et RW à 0 car on envoie une commande
	PORTC |= (1 << E);					// On autorise l'envoi de la donnée
									// On laisse le bit E du port c à 0 car on envoie une commande
	PORTA = cmd;						// On envoi la donnée sur le port A
	PORTC = ~(1 << E);					// On ferme notre fenêtre d'envoi
	_delay_us(43);						// on attend le temps que l'écriture se termine
}




		// ----- Envoyer un caractère ----- //

void LCD_putchar(char c) {
	PORTC = 0x00 | (1<< RS);			// On met RS à 1 et RW à 0 car on envoie une commande
	PORTC |= (1 << E);					// On autorise l'envoi de la donnée
								// On laisse le bit E du port c à 0 car on envoie une commande
	PORTA = c;							// On envoi la donnée sur le port A
	
	PORTC = ~(1 << E);					// On ferme notre fenêtre d'envoi
	
	_delay_ms(1.573);						// on attend le temps que l'écriture se termine + le temps d'éxectution de la commande la plus lente (donc 43us + 1.53ms)
}




		// ----- Envoyer une chaine de caractères ----- //

int LCD_putchars(char c, FILE *stream) {
	PORTC = 0x00 | (1<< RS);			// On met RS à 1 et RW à 0 car on envoie une commande
	PORTC |= (1 << E);					// On autorise l'envoi de la donnée
	// On laisse le bit E du port c à 0 car on envoie une commande
	PORTA = c;							// On envoi la donnée sur le port A
	PORTC = ~(1 << E);					// On ferme notre fenêtre d'envoi
	
	
	_delay_us(43);
	return 01; // 01 sera le code pour annoncer que la donnée a été envoyé
}



	

		// ----- Envoyer une chaine de caractères ----- //

void tests_unitaires(void) {
	
	unsigned char caractere = 'A'; // 65 = A en code asquii
	
	LCD_init();
	
	while (caractere <= 'Z') { // 90 = Z en code asquii
		if (caractere == 'Q') LCD_sendcmd(LIGNE2); // Juste après la 16e lettre de l'aplhabet, on passe à la ligne (le lcd peut afficher jusqu'à 16 lettres par ligne)
		LCD_putchar(caractere);
		caractere++;
		_delay_ms(500);
	}
	LCD_sendcmd(CLEAR_DISPLAY);
	LCD_sendcmd(RETURN_HOME);
	_delay_ms(1000);
	
	printf("Accordeur 2026");
	_delay_ms(1000);
	
	LCD_sendcmd(0x80 | 0x40 | 4); // Ligne 2 colone 4
	_delay_ms(1000);
	
	printf("Ou suis-je ?");
}




		// ----- Fonction bargraph ----- //

void bargaph (float freq_jouee, float freq_cible) {
	
	LCD_sendcmd(LIGNE2 | 7);
	_delay_us(39);
	printf("|");
	
	float ecart = freq_jouee - freq_cible;
		
	if (ecart > 0.5) { // On met une petite marge parce qu'être parfaitement sur la bonne note est presque impossible
		LCD_sendcmd(LIGNE2 | 8);
		if (ecart <= 7) for (int i=0 ; i<ecart-0.5 ; i++) printf("+");
		if (ecart > 7) printf("+>>");
	}
		
	else if (ecart < -0.5) {
		LCD_sendcmd(LIGNE2 | 6);
		LCD_sendcmd(ENTRY_MODE_SET | DECREMENT);
		if (ecart >= -7) for (int i=ecart-0.5 ; i<0 ; i++) printf("-");
		if (ecart < -7) printf("-<<"); // <<- à l'envers
		LCD_sendcmd(ENTRY_MODE_SET | INCREMENT);
	}
	
	else {
		LCD_sendcmd(LIGNE2 | 6);
		printf("o|o");
	}
}




		// ----- Fonction test bargraph ----- //

void test_bargraph(void) {
	// ----- Test d'unité négatif :						325.6-329.6 = -4			L'afficheur doit mettre 4 signes -
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2); // d'après la datashee, il faut attendre au moins 1.53ms (on attendra ici 2ms)
	bargaph(325.6, 329.6);
	_delay_ms(1500);
	
	// ----- Test d'arrondi néatif :					325-329.6 = -4.6			L'afficheur doit mettre 5 signes -
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(325, 329.6);
	_delay_ms(1500);
	
	// ----- Test de dépassement d'écran négatif :		322.6-329.6 = -7			L'afficheur doit mettre 7 signes -
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(322.6, 329.6);
	_delay_ms(1500);
	
	// ----- Test de dépassement d'écran négatif :		322.5-329.6 = -7.1			L'afficheur doit afficher <<-
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(322.5, 329.6);
	_delay_ms(1500);
	
	// ----- Test d'unité positif :						333.6-329.6 = 4				L'afficheur doit mettre 4 signes +
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(333.6, 329.6);
	_delay_ms(1500);
	
	// ----- Test d'arrondi positif :					334.2-329.6 = 4.6			L'afficheur doit mettre 5 signes +
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(334.4, 329.6);
	_delay_ms(1500);
	
	// ----- Test de dépassement d'écran positif :		336.6-329.6 = 7				L'afficheur doit mettre 7 signes +
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(336.6, 329.6);
	_delay_ms(1500);
	
	// ----- Test de dépassement d'écran positif :		336.7-329.6 = 7.1				L'afficheur doit afficher +>>
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(336.7, 329.6);
	_delay_ms(1500);
	
	// ----- Test fréquence parfaite :					329.6-329.6 = 0				L'afficheur doit afficher o|o
	LCD_sendcmd(CLEAR_DISPLAY);
	_delay_ms(2);
	bargaph(329.6, 329.6);
	_delay_ms(1500);
}




		// ----- Mode MESURE ----- //
		
void mode_MESURE(void) {
	
	stop_timer1(); // On veut pas jouer la note en mode mesure
	
	char note_cible[6][5] = {"Mi3", "Si2", "Sol2", "Re2", "La1", "Mi1"};
	float freq_cible[6] = {329.6, 246.9, 196, 146.8, 110, 82.4};
	
	LCD_init();
	
	LCD_sendcmd(LIGNE1 | 10);
	printf("(%s)", note_cible[choix_note]);
	
	LCD_sendcmd(LIGNE1);
	printf("Freq %d", (int) freq_cible[choix_note]); // le LCD ne prend pas en charge les float (on changera surement qvec la frequence mesuree
	
	bargaph(329, freq_cible[choix_note]);
}





		// ----- Mode MESURE ----- //
		
void mode_SOUND(void) {
			
	char note_cible[6][5] = {"Mi3", "Si2", "Sol2", "Re2", "La1", "Mi1"};
	float freq_cible[6] = {329.6, 246.9, 196, 146.8, 110, 82.4};
	
	LCD_init();
	
	LCD_sendcmd(LIGNE1 | 3);
	printf("Mode SOUND");
	
	LCD_sendcmd(LIGNE2);
	printf("Note : %s", note_cible[choix_note]);
	
	jouer_timer1(freq_cible[choix_note]);
}





// ----- Mode DEBUG ----- //

void mode_DEBUG(void) {
	
	char note_cible[6][5] = {"Mi3", "Si2", "Sol2", "Re2", "La1", "Mi1"};
	float freq_cible[6] = {329.6, 246.9, 196, 146.8, 110, 82.4};
	
	LCD_init();
	
	if (debug_mode == 1) {
		LCD_sendcmd(LIGNE1 | 3);
		printf(("Mode DEBUG"));
		
		LCD_sendcmd(LIGNE2);
		printf("Voir tests -->");
	}
	
	if (debug_mode == 2) {
		LCD_sendcmd(LIGNE1);
		printf("Tests unitaires");
		
		LCD_sendcmd(LIGNE2 | 3);
		printf("Demarrer ->");
		
		LCD_sendcmd(LIGNE2 | 16);
		printf("1");
	}
	
	if (debug_mode == 3) {
		LCD_sendcmd(LIGNE1);
		printf("Tests bargraph");
		
		LCD_sendcmd(LIGNE2 | 3);
		printf("Demarrer ->");
		
		LCD_sendcmd(LIGNE2 | 16);
		printf("2");
	}
}









// ------------------------------------------------------ Gestion interruptions ------------------------------------------------------------- //
// ------------------------------------------------------------------------------------------------------------------------------------------ //


		// ----------- INIT 0 ----------- //

void init_int0(void) {
	DDRD &= ~(1<<PD2);
	
	GICR = GICR | (1<<INT0);
	MCUCR = MCUCR | (1<<ISC01);
}

ISR(INT0_vect) {
	if (debug_mode == 1) {
		if (debug_mode == 1 || debug_mode == 3) debug_mode = 2;
		if (debug_mode == 2) debug_mode = 3;
		
		mode_DEBUG();
	}
	
	if (debug_mode == 0) {
			if (choix_note<5) choix_note++;
			else choix_note = 0;
			
			if (mode_normal == 0) mode_MESURE();
			if (mode_normal == 1) mode_SOUND();
	}
}


void init_int1(void) {
	DDRD &= ~(1<<PD2);
	
	GICR = GICR | (1<<INT1);
	MCUCR = MCUCR | (1<<ISC11);
}

ISR(INT1_vect) {
	mode_normal = 1-mode_normal;
	
	if (mode_normal == 0 && debug_mode == 0) mode_MESURE();
	if (mode_normal == 1 && debug_mode == 0) mode_SOUND();
}







// --------------------------------------------------------- Gestion timers ----------------------------------------------------------------- //
// ------------------------------------------------------------------------------------------------------------------------------------------ //


		// ----------- TIMER 1 ----------- //
		
void init_timer1() {
	DDRD |= (1<<PD5);
	
	TCCR1A = (1<<COM1A0);
	TCCR1B = (1<<WGM12);
}

void jouer_timer1(float freq) {
	TCCR1B |= (1<<CS10);
	OCR1A = (F_CPU/(2*1*(freq))-1) + 0.5;
}

void stop_timer1() {
	TCCR1B = TCCR1B & (~(1<<CS10));
}