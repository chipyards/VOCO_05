// constantes
#define TIERS_OCTAVE	1.2599210499
#define SIXIEME_OCTAVE	1.1224620483
#define DEMOD_RECENTER

/*
Recentrage de la reponse a -3dB
- soient fa et fb les frequences de coupure a -3dB
- la frequence centrale du filtre passe bande analog est f0 = sqrt( fa * fb ) (moyenne geometrique)
- la frequence porteuse du demodulateur est f0d = 0.5 * ( fa + fb ) (mediane)
  elle est donc legerement decalee
- posons fa = f0 / K6 et fb = f0 * K6 avec K6 = pow( 2, 1/6 ) = SIXIEME_OCTAVE dans notre cas
  alors f0d = 0.5 * ( 1/K6 + K6 ) * f0 soit :				f0d = 1.00668 * f0
- posons fa = f0d * (1 - kflp) et fb = f0d * (1 + kflp), il vient
  kflp = ( K6*K6 - 1 ) / ( K6*K6 + 1 ) = ( K3 - 1 ) / ( K3 + 1 ) soit : kflp = 0.115013
*/

// filtre du 2e ordre = biquad
class filt2 {
public :
// parametres
double aKc;	// traduction fc en rad/sample
double aD;	// traduction Q
// variables signal (persistantes)
double acc1a;
double acc2a;
// methodes : initialisations
void initLP( double rel_fc );
// methodes : traiter 1 echantillon
double LP_step( double X );
};

// filtre du 3e ordre (1 biquad + RC)
class filt3 {
public :
// parametres
double aKc;	// traduction fc en rad/sample
double bKc;	// traduction fc en rad/sample
double aD;	// traduction Q
double gain;
// variables signal (persistantes)
double acc1a;
double acc2a;
double acc1b;
// methodes : initialisations
void initLP( double rel_fc );
// methodes : traiter 1 echantillon
double LP_step( double X );
};

// filtre du 4e ordre (2 biquads a et b)
class filt4 {
public :
// parametres
double aKc;	// traduction fc en rad/sample
double bKc;	// traduction fc en rad/sample
double aD;	// traduction Q
double bD;	// traduction Q
double gain;
// variables signal (persistantes)
double acc1a;
double acc2a;
double acc1b;
double acc2b;
// methodes : initialisations
void initBP( double rel_fc );
void initLP( double rel_fc );
// methodes : traiter 1 echantillon
double LP_step( double X );
double BP_step( double X );
};

// parametres et variables statiques pour 1 canal filtre passe-bande + redresseur async
class canal4 {
public :
// filtre passe-bande
filt4 BP;
// enveloppe follower
double rect_decay;	// decroissance lineaire du hold apres redresseur
// filtre passe-bas "ripple filter"
filt4 LP;
// variables signal (persistantes)
double peak;		// dernier max de la valeur redressee
double rect_out;	// sortie redresseur et hold a decay lineaire
// methode init
void init( double rel_fc, double rect_decay, double krif );
// methode : traiter un echantillon par filtre passe-bande + enveloppe follower
double step( double X );
};

// parametres et variables statiques pour 1 demodulateur synchrone
class demod4 {
public :
// osc local
double k;	// frequence en rad/sample
double Ephase;	// phase courante
double Fphase;	// phase courante
// filtres passe-bas
//filt4 LPr;
//filt4 LPi;
filt3 ELPr;
filt3 ELPi;
filt3 FLPr;
filt3 FLPi;
//filt2 LPr;
//filt2 LPi;
// methode init
void init( double rel_fo, double rel_flp );
// methode : calculer un echantillon d'enveloppe par demodulateur synchrone
double step_env( double X );
// methode : calculer un echantillon par filtrage synchrone
double step_filt( double X );
};


/** ============================= la classe jdsp ======================== **/

class jdsp {
public:
// environnement
double Fs;		// frequence d'echantillonnage Hz
double f0;		// frequence centrale Hz
// canaux de filtre passe-bande + redresseur async (un seul en fait)
canal4 canal;
// config enveloppe follower
double rect_decay;	// rectifier hold decay en 1/sample
double krif;		// frequ de coupure low-pass relative a f0
// canaux de demodulateur synchrone (un seul en fait)
double f0d;		// frequence porteuse corrigee pour demod.
double kflp;		// frequ de coupure low-pass relative a f0
demod4 demod;
// constructeur
jdsp() {
	// environnement
	Fs = 48000.0;	// Hz	// frequence d'echantillonnage
	f0 = 500.0;	// Hz
	// config enveloppe follower
	rect_decay = 0.4;	// rectifier hold decay en 1/sample
	krif = 0.5;		// ripple filter ratio
	// config demod synchrone
	#ifdef DEMOD_RECENTER
		kflp = ( TIERS_OCTAVE - 1 ) / ( TIERS_OCTAVE + 1 );
	#else
		kflp = SIXIEME_OCTAVE - 1.0;	// demi tiers d'octave (naif)
	#endif
	};
// methodes : parametres
void update() {			// mise a jour des parametres deduits
	#ifdef DEMOD_RECENTER
	// frequence corrigee pour recentrage reponse demod.
		f0d = f0 * 0.5 * ( SIXIEME_OCTAVE + ( 1.0 / SIXIEME_OCTAVE ) );
	#else
		f0d = f0;
	#endif
	canal.init( f0 / Fs, rect_decay, krif );
	demod.init( f0d / Fs, ( f0d / Fs ) * kflp  );
	};
};
