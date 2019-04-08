#include <stdio.h>
#include <math.h>
#include "jdsp.h"

/** ============================= la "classe" filt4 ======================== **/

// init des parametres pour filtre passe-bande tiers d'octave
// rel_fc = fc / Fs (i.e. en turn/sample)
void filt4_initBP( filt4 * ff, double rel_fc )
{
ff->aD = 1.0 / 7.7;
ff->bD = 1.0 / 7.7;
ff->aKc = ( 2.0 * M_PI * rel_fc ) / 1.0865;	// a utiliser avec Q = 7.7 et G = 1.75 selon le brevet
ff->bKc = ( 2.0 * M_PI * rel_fc ) * 1.0865;
ff->gain = 1.612 * 1.612 / ( 7.7 * 7.7 ); 	// on a du corriger le 1.75 ??
// init accus
ff->acc1a = 0.0;
ff->acc2a = 0.0;
ff->acc1b = 0.0;
ff->acc2b = 0.0;
}

// init des parametres pour filtre passe-bas Butterworth
// rel_fc = fc / Fs (i.e. en turn/sample)
void filt4_initLP( filt4 * ff, double rel_fc )
{
// 1/Q = -2cos( pi * ( 2k + n - 1 ) / 2n avec n = ordre et k variant de 1 a n/2
ff->aD = -2.0 * cos( M_PI * ( 5.0 / 8.0 ) );	// 0.7654
ff->bD = -2.0 * cos( M_PI * ( 7.0 / 8.0 ) );	// 1.8478
ff->aKc = ( 2.0 * M_PI * rel_fc );
ff->bKc = ( 2.0 * M_PI * rel_fc );
ff->gain = 1.0;
// init accus
ff->acc1a = 0.0;
ff->acc2a = 0.0;
ff->acc1b = 0.0;
ff->acc2b = 0.0;
}

/** ============================= la "classe" canal4 ======================== **/

// init des parametres pour un canal d'analyse
//	rel_fc en turn/sample
//	rect_decay en unit/sample
//	rfr sans dimension
void canal4_init( canal4 * cc, double rel_fc, double rect_decay, double rfr )
{
// filtre passe-bande
cc->filt = harald_step;	// pointeur sur la fonction de filtrage 1 ech.
filt4_initBP( &cc->BP, rel_fc );
// enveloppe follower
cc->rect_decay = rect_decay;	// decroissance lineaire du hold apres redresseur
// filtre passe-bas "ripple filter"
cc->filtLP = butter_step;	// pointeur sur la fonction de filtrage 1 ech.
filt4_initLP( &cc->LP, rel_fc / rfr );
// variables signal persistant
cc->peak = 0.0;
cc->rect_out = 0.0;
}

/** ============================= la "classe" demod4 ======================== **/

// init des parametres pour un canal d'analyse
//	rel_fo en turn/sample
//	rel_flp en turn/sample
void demod4_init( demod4 * dd, double rel_fo, double rel_flp )
{
// filtre passe-bas
dd->filtLP = butter_step;	// pointeur sur la fonction de filtrage 1 ech.
filt4_initLP( &dd->LPr, rel_flp );	// 1 seule fonction pour 2 filtres
filt4_initLP( &dd->LPi, rel_flp );
dd->k = 2.0 * M_PI * rel_fo;
dd->phase = 0.0;
}

/** ============================= la "classe" jdsp ======================== **/

// parametres de la manip (singleton)
static jdsp jp;

void jdsp_defaults()
{
// environnement
jp.Fs = 48000.0;	// Hz	// frequence d'echantillonnage
jp.fc = 500.0;	// Hz
// config enveloppe follower
jp.rect_decay = 0.4;	// rectifier hold decay en 1/sample
jp.rfr = 2.0;		// ripple filter ratio = Fc / Fripp
// config demod synchrone
jp.flp = jp.fc * ( pow( 2.0, 1/6.0 ) - 1.0 );	// demi tiers d'octave
// jp.flp = jp.fc * 0.03;	// quart de ton
// testbench pour enveloppe follower
jp.t0 = 1000;
jp.t1 = 4000;
jp.t2 = jp.t1;
jp.tf = 100;
jp.qpu = 3;
jp.kdf = pow( 2.0, 1/3.0 );	// pow( 2.0, 1/6.0 ) = demi-tiers d'octave
jp.knoise = 0.0;
}

// mise a jour des parametres deduits
void jdsp_init()
{
canal4_init( &jp.canal, jp.fc / jp.Fs, jp.rect_decay, jp.rfr );
demod4_init( &jp.demod, jp.fc / jp.Fs, jp.flp / jp.Fs );
}

// exportation des parametres (au cas ou...)
jdsp * jdsp_get()
{
return &jp;
}

/** ============================= modeles de filtres ======================== **/

// traiter 1 echantillon par filtre passe-bande 4eme ordre
double harald_step( filt4 * pp, double X )
{
double aYlp, aYbp, aYhp;
double bYlp, bYbp, bYhp;
// biquad "a"
aYlp = pp->acc1a + pp->aKc * pp->acc2a;
aYhp = X - pp->aD * pp->acc2a - aYlp;
aYbp = pp->acc2a + pp->aKc * aYhp;
// biquad "b"
bYlp = pp->acc1b + pp->bKc * pp->acc2b;
bYhp = pp->acc2a - pp->bD * pp->acc2b - bYlp;	// on retarde volontairement de 1 Ts, car on a observe que
bYbp = pp->acc2b + pp->bKc * bYhp;		// la sortie HP du biquad a une avance de 1/2 Ts !!!!!!
// sampling
pp->acc1a = aYlp;
pp->acc2a = aYbp;
pp->acc1b = bYlp;
pp->acc2b = bYbp;
// output
return bYbp * pp->gain;		// ( 1.612 * 1.612 / ( 7.7 * 7.7 ) );
}

// traiter 1 echantillon par filtre passe-bas 4eme ordre
double butter_step( filt4 * pp, double X )
{
double aYlp, aYbp, aYhp;
double bYlp, bYbp, bYhp;
// biquad "a"
aYlp = pp->acc1a + pp->aKc * pp->acc2a;
aYhp = X - pp->aD * pp->acc2a - aYlp;
aYbp = pp->acc2a + pp->aKc * aYhp;
// biquad "b"
bYlp = pp->acc1b + pp->bKc * pp->acc2b;
bYhp = pp->acc1a - pp->bD * pp->acc2b - bYlp;	// on retarde volontairement de 1 Ts, car on a observe que
bYbp = pp->acc2b + pp->bKc * bYhp;		// la sortie HP du biquad a une avance de 1/2 Ts !!!!!!
// sampling
pp->acc1a = aYlp;
pp->acc2a = aYbp;
pp->acc1b = bYlp;
pp->acc2b = bYbp;
// output
return bYlp * pp->gain;
}

// traiter un echantillon par filtre passe-bande + enveloppe follower
// (le filtre passe-bande peut etre remplace par pass-haut ou bas selon cc->filt)
double envel_step( canal4 * cc, double X )
{
double Y;
// filtrage
Y = cc->filt( &cc->BP, X );
// redressement
if	( cc->rect_out > fabs( Y ) )
	{	// hold avec decay lineaire proportionnel
	cc->rect_out -= ( cc->peak * cc->rect_decay );
	if	( cc->rect_out < 0.0 )
		cc->rect_out = 0.0;
	}	// N.B. pas de else car on a pu soustraire trop...
if	( cc->rect_out < fabs( Y ) )
	{
	cc->rect_out = fabs( Y );
	cc->peak = cc->rect_out;
	}
// ripple filter
Y = cc->filtLP( &cc->LP, cc->rect_out );
return Y;
}

// traiter un echantillon par demodulateur synchrone
double demod_step( demod4 * dd, double X )
{
double Reel, Img;
// oscillateur
Reel = cos( dd->phase );
Img  = sin( dd->phase );
dd->phase += dd->k;
// modulation
Reel *= X;
Img *= X;
// filtrage
Reel = dd->filtLP( &dd->LPr, Reel );
Img  = dd->filtLP( &dd->LPi, Img );
// module
return sqrt( Reel * Reel + Img * Img );
}
