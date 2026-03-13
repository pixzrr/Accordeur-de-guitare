/*
 * P-Acc_PORT-DIAL-TP2.c
 *
 * Created: 13/03/2026 14:22:45
 * Author : 45018569
 */ 

#define F_CPU 3686000UL


#include <avr/io.h>
#include <util/delay.h>


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
	#define _2_LINE_MODE			0x80
	#define _5x8_DOTS				0x00
	#define _5x11_DOTS				0x40

#define E						0x00 // Autoriser l'envoi de la donnée (0 si on ne veut rien envoyer, 1 si on veut envoyer une donnée
#define RW						0x01 // Ecrire ou lire la donnée (0 pour envoyer, 1 pour lire) - bit 1 du port c
#define RS						0x02 // Savoir quelle donnee est echangée (RS = 0 si commande ou 1 si c'est un caractère) - bit 2 du port c


		// ----- Liste des fonctions ----- //
		
void LCD_init(void);
void LCD_sendcmd(char cmd);



// ------------------------------------------------------ Main ------------------------------------------------------------- //
// ------------------------------------------------------------------------------------------------------------------------- //

void main(void) {
	// ___Setup___ //			// (le port C sert à envoyer des instructions d'envoi / lecture et le port A sert à envoyer la donnée)
	DDRA = 0xFF;
	DDRC = 0xFF;
	
	LCD_init();
	
	LCD_putchar('A');
	
    while (1) 
    {
    }
}

// ------------------------------------------------------------------------------------------------------------------------- //
// ------------------------------------------------------------------------------------------------------------------------- //



		// ----- Initialisation du LCD ----- //
		
void LCD_init(void) { // On se sert ici de la page 5 de la partie datasheet
	
	_delay_ms(30);
	
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
	_delay_us(43);						// on attend le temps que l'écriture se termine
}


LCD_putchar('a', fprintf("test"));

		// ----- Envoyer une chaine de caractères ----- //

void LCD_putchar(char c, FILE * stream) {
	PORTC = 0x00 | (1<< RS);
	PORTC |= (1 << E);

	PORTA = stream;
	PORTC = ~(1 << E);
	_delay_us(43);
}