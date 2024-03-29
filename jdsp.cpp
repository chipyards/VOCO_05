#include <stdio.h>
#include <math.h>
#include "jdsp.h"

/** ============================= les classes filt2, 3, 4 ================ **/

// init des parametres pour filtre passe-bas Butterworth
// rel_fc = fc / Fs (i.e. en turn/sample)
void filt2::initLP( double rel_fc )
{
// 1/Q = -2cos( pi * ( 2k + n - 1 ) / 2n avec n = ordre et k variant de 1 a n/2
aD = -2.0 * cos( M_PI * ( 3.0 / 4.0 ) );	// sqrt(2) !
aKc = ( 2.0 * M_PI * rel_fc );
// init accus
acc1a = 0.0;
acc2a = 0.0;
}

// traiter 1 echantillon par filtre passe-bas 2eme ordre
double filt2::LP_step( double X )
{
double aYlp, aYbp, aYhp;
// biquad "a"
aYlp = acc1a + aKc * acc2a;
aYhp = X - aD * acc2a - aYlp;
aYbp = acc2a + aKc * aYhp;
// sampling
acc1a = aYlp;
acc2a = aYbp;
// output
return aYlp;
}

// init des parametres pour filtre passe-bas Butterworth
// rel_fc = fc / Fs (i.e. en turn/sample)
void filt3::initLP( double rel_fc )
{
// 1/Q = -2cos( pi * ( 2k + n - 1 ) / 2n avec n = ordre et k variant de 1 a n/2
aD = -2.0 * cos( M_PI * ( 4.0 / 6.0 ) );	// 1.0 !
aKc = ( 2.0 * M_PI * rel_fc );
bKc = ( 2.0 * M_PI * rel_fc );
gain = 1.0;
// init accus
acc1a = 0.0;
acc2a = 0.0;
acc1b = 0.0;
}

// traiter 1 echantillon par filtre passe-bas 2eme ordre
double filt3::LP_step( double X )
{
double aYlp, aYbp, aYhp, bYlp, bYhp;
// biquad "a"
aYlp = acc1a + aKc * acc2a;
aYhp = X - aD * acc2a - aYlp;
aYbp = acc2a + aKc * aYhp;
bYlp = bKc * acc1b;
bYhp = aYlp - bYlp;
// sampling
acc1a = aYlp;
acc2a = aYbp;
acc1b += bYhp;
// output
return bYlp;
}

// init des parametres pour filtre passe-bande tiers d'octave
// rel_fc = fc / Fs (i.e. en turn/sample)
void filt4::initBP( double rel_fc )
{
aD = 1.0 / 7.7;
bD = 1.0 / 7.7;
aKc = ( 2.0 * M_PI * rel_fc ) / 1.0865;	// a utiliser avec Q = 7.7 et G = 1.75 selon le brevet
bKc = ( 2.0 * M_PI * rel_fc ) * 1.0865;
gain = 1.612 * 1.612 / ( 7.7 * 7.7 ); 	// on a du corriger le 1.75 ??
// init accus
acc1a = 0.0;
acc2a = 0.0;
acc1b = 0.0;
acc2b = 0.0;
}

// init des parametres pour filtre passe-bas Butterworth
// rel_fc = fc / Fs (i.e. en turn/sample)
void filt4::initLP( double rel_fc )
{
// 1/Q = -2cos( pi * ( 2k + n - 1 ) / 2n avec n = ordre et k variant de 1 a n/2
aD = -2.0 * cos( M_PI * ( 5.0 / 8.0 ) );	// 0.7654
bD = -2.0 * cos( M_PI * ( 7.0 / 8.0 ) );	// 1.8478
aKc = ( 2.0 * M_PI * rel_fc );
bKc = ( 2.0 * M_PI * rel_fc );
gain = 1.0;
// init accus
acc1a = 0.0;
acc2a = 0.0;
acc1b = 0.0;
acc2b = 0.0;
}

// traiter 1 echantillon par filtre passe-bande 4eme ordre
double filt4::BP_step( double X )
{
double aYlp, aYbp, aYhp;
double bYlp, bYbp, bYhp;
// biquad "a"
aYlp = acc1a + aKc * acc2a;
aYhp = X - aD * acc2a - aYlp;
aYbp = acc2a + aKc * aYhp;
// biquad "b"
bYlp = acc1b + bKc * acc2b;
bYhp = acc2a - bD * acc2b - bYlp;	// on retarde volontairement de 1 Ts, car on a observe que
bYbp = acc2b + bKc * bYhp;		// la sortie HP du biquad a une avance de 1/2 Ts !!!!!!
// sampling
acc1a = aYlp;
acc2a = aYbp;
acc1b = bYlp;
acc2b = bYbp;
// output
return bYbp * gain;		// ( 1.612 * 1.612 / ( 7.7 * 7.7 ) );
}

// traiter 1 echantillon par filtre passe-bas 4eme ordre
double filt4::LP_step( double X )
{
double aYlp, aYbp, aYhp;
double bYlp, bYbp, bYhp;
// biquad "a"
aYlp = acc1a + aKc * acc2a;
aYhp = X - aD * acc2a - aYlp;
aYbp = acc2a + aKc * aYhp;
// biquad "b"
bYlp = acc1b + bKc * acc2b;
bYhp = acc1a - bD * acc2b - bYlp;	// on retarde volontairement de 1 Ts, car on a observe que
bYbp = acc2b + bKc * bYhp;		// la sortie HP du biquad a une avance de 1/2 Ts !!!!!!
// sampling
acc1a = aYlp;
acc2a = aYbp;
acc1b = bYlp;
acc2b = bYbp;
// output
return bYlp * gain;
}

/** ============================= la classe canal4 ======================== **/

// init des parametres pour un canal d'analyse
//	rel_fc en turn/sample
//	rect_decay en unit/sample
//	krif sans dimension
void canal4::init( double rel_fc, double rect_decay, double krif )
{
// filtre passe-bande
BP.initBP( rel_fc );
// enveloppe follower
this->rect_decay = rect_decay;	// decroissance lineaire du hold apres redresseur
// filtre passe-bas "ripple filter"
LP.initLP( rel_fc * krif );
// variables signal persistant
peak = 0.0;
rect_out = 0.0;
}

// traiter un echantillon par filtre passe-bande + enveloppe follower
// (le filtre passe-bande devrait pouvoir etre remplace par pass-haut ou bas
//  pour les canaux extremes)
double canal4::step( double X )
{
double Y;
// filtrage
Y = BP.BP_step( X );
// redressement
if	( rect_out > fabs( Y ) )
	{	// hold avec decay lineaire proportionnel
	rect_out -= ( peak * rect_decay );
	}	// N.B. pas de else car on a pu soustraire trop...
if	( rect_out < fabs( Y ) )
	{
	rect_out = fabs( Y );
	peak = rect_out;
	}
// ripple filter
Y = LP.LP_step( rect_out );
return fabs(Y);
}

/** ============================= la classe demod4 ======================== **/

// init des parametres pour un canal d'analyse
//	rel_fo en turn/sample
//	rel_flp en turn/sample
void demod4::init( double rel_fo, double rel_flp )
{
// filtres passe-bas
ELPr.initLP( rel_flp );
ELPi.initLP( rel_flp );
FLPr.initLP( rel_flp );
FLPi.initLP( rel_flp );
k = 2.0 * M_PI * rel_fo;
Ephase = 0.0;
Fphase = 0.0;
}

// traiter un echantillon par demodulateur synchrone
double demod4::step_env( double X )
{
double OscR, OscI, Reel, Img;
// oscillateur
OscR = cos( Ephase );
OscI = sin( Ephase );
Ephase += k;
// modulation
Reel = OscR * X;
Img  = OscI * X;
// filtrage
Reel = ELPr.LP_step( Reel );
Img  = ELPi.LP_step( Img );
// module
return sqrt( Reel * Reel + Img * Img );
}

// traiter un echantillon par filtrage synchrone
double demod4::step_filt( double X )
{
double OscR, OscI, Reel, Img;
// oscillateur
OscR = cos( Fphase );
OscI = sin( Fphase );
Fphase += k;
// modulation
Reel = OscR * X;
Img  = OscI * X;
// filtrage
Reel = FLPr.LP_step( Reel );
Img  = FLPi.LP_step( Img );
// demodulation : partie reelle du produit
Reel *= OscR;
Img  *= OscI;
return ( Reel + Img );
}
