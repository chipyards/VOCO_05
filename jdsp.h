
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
void init( double rel_fc, double rect_decay, double rfr );
// methode : traiter un echantillon par filtre passe-bande + enveloppe follower
double step( double X );
};

// parametres et variables statiques pour 1 demodulateur synchrone
class demod4 {
public :
// osc local
double k;	// frequence en rad/sample
double phase;	// phase courante
// filtres passe-bas
filt4 LPr;
filt4 LPi;
// methode init
void init( double rel_fo, double rel_flp );
// methode : traiter un echantillon par demodulateur synchrone
double step( double X );
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
double rfr;		// ripple filter ratio = Fc / Fripp
// canaux de demodulateur synchrone (un seul en fait)
double kflp;		// frequ de coupure low-pass relative a f0
demod4 demod;
// constructeur
jdsp() {
	// environnement
	Fs = 48000.0;	// Hz	// frequence d'echantillonnage
	f0 = 500.0;	// Hz
	// config enveloppe follower
	rect_decay = 0.4;	// rectifier hold decay en 1/sample
	rfr = 2.0;		// ripple filter ratio = Fc / Fripp
	// config demod synchrone
	kflp = pow( 2.0, 1/6.0 ) - 1.0;	// demi tiers d'octave
	// kflp = 0.03;	// quart de ton
	};
// methodes : parametres
void update();	// mise a jour des parametres deduits
// methodes : traiter 1 echantillon
double canal_step( double X ) {
	return canal.step(X);
	}
double demod_step( double X ) {
	return demod.step(X);
	}
};