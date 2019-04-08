
// parametres et variables statiques pour 1 filtre du 4e ordre (2 biquads a et b)
class filt4 {
public :
double aKc;	// traduction fc en rad/sample
double bKc;	// traduction fc en rad/sample
double aD;	// traduction Q
double bD;	// traduction Q
double gain;
// variables signal persistant
double acc1a;
double acc2a;
double acc1b;
double acc2b;
};

// parametres et variables statiques pour 1 canal filtre passe-bande + redresseur async
class canal4 {
public :
// filtre passe-bande
filt4 BP;
double (*filt)(filt4*,double);	// pointeur sur la fonction de filtrage 1 ech.
					// pourquoi ce n'est pas 1 methode de filt4 ?
					// pour faciliter regresion en C peut etre...
// enveloppe follower
double rect_decay;	// decroissance lineaire du hold apres redresseur
// filtre passe-bas "ripple filter"
filt4 LP;
double (*filtLP)(filt4*,double);	// pointeur sur la fonction de filtrage 1 ech.
// variables signal persistant
double peak;		// dernier max de la valeur redressee
double rect_out;	// sortie redresseur et hold a decay lineaire
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
double (*filtLP)(filt4*,double);	// pointeur sur la fonction de filtrage 1 ech.
};


/** ============================= la "classe" jdsp ======================== **/

class jdsp {
public:
// environnement
double Fs;		// frequence d'echantillonnage Hz
// canaux de filtre passe-bande + redresseur async (un seul en fait)
double fc;
canal4 canal;
// config enveloppe follower
double rect_decay;	// rectifier hold decay en 1/sample
double rfr;		// ripple filter ratio = Fc / Fripp
// canaux de demodulateur synchrone (un seul en fait)
double flp;		// frequ de coupure low-pass Hz
demod4 demod;
// testbench : generation de salves d'enveloppe trapezoidale
double f;	// Hz, frequence courante
double kdf;	// saut relatif de frequence (> 1.0)
double k;	// rad/sample, frequence courante rellement utilisee
double knoise;	// proportion de bruit
int t0;		// duree silence initial en samples
int t1;		// duree salve			"	(fade-in inclus)
int t2;		// duree silence final		"	(fade-out inclus)
int tf;		// duree fade-in et fade_out	"
		// periode des pulses		"	= t0+t1+t2 pour info
int qpu;	// nombre de pulses

};

// pre-constructeur
void jdsp_defaults();

// mise a jour des composants du jdsp
void jdsp_init();

// exportation des parametres
jdsp * jdsp_get();

/** ============================= modeles de filtres ======================== **/

// traiter 1 echantillon
double butter_step( filt4 * pp, double X );
// traiter 1 echantillon
double harald_step( filt4 * pp, double X );

// traiter un echantillon par filtre passe-bande + enveloppe follower
// (le filtre passe-bande peut etre remplace par pass-haut ou bas selon cc->filt)
double envel_step( canal4 * cc, double X );

// traiter un echantillon par demodulateur synchrone
double demod_step( demod4 * dd, double X );
