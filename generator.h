// testbench : generation de salves d'enveloppe trapezoidale
class wgen {
public:
// parametres
double Fs;	// frequ. echant.
double f0;	// frequ. centrale
double kdf;	// saut relatif de frequence (> 1.0)
double knoise;	// proportion de bruit
int t0;		// duree silence initial en samples
int t1;		// duree salve			"	(fade-in inclus)
int t2;		// duree silence final		"	(fade-out inclus)
int tf;		// duree fade-in et fade_out	"
		// periode des pulses		"	= t0+t1+t2 pour info
int qpu;	// nombre de pulses
// variables derivees
int tpu;	// periode pulse en samples
int qsamples;
// constructeur
wgen() {
	Fs = 48000.0;
	f0 = 500.0;
	kdf = pow( 2.0, 1/3.0 );	// pow( 2.0, 1/6.0 ) = demi-tiers d'octave
	knoise = 0.0;
	// durees exprimees en sampling periods
	t0 = 1000;
	t1 = 4000;
	t2 = t1;
	tf = 100;
	// nombre de pulses
	qpu = 3;
	qsamples = 0;
	}
int calc_size() {
	tpu = t0 + t1 + t2;
	qsamples = tpu * qpu;
	return qsamples;
	}
void generate( float * fbuf ) {
	double f = 0.0;		// Hz, frequence courante
	double w = 0.0;		// rad/sample, frequence courante rellement utilisee
	int imod;
	double gen_env;		// enveloppe pour generateur
	double gen_sig;		// signal genere
	double phase = 0.0;

	// boucle de calcul generation signal
	for	( int i = 0; i < qsamples; ++i )
		{
		// generation enveloppe d'une pulse periodique, periode tpu samples
		imod = i % tpu;
		if	( imod < t0 )
			gen_env = 0.0;
		else if	( imod < ( t0 + tf ) )
			gen_env = (double)( imod - t0 ) / (double)tf;
		else if	( imod < ( t0 + t1 ) )
			gen_env = 1.0;
		else if	( imod < ( t0 + t1 + tf ) )
			gen_env = (double)( t0 + t1 + tf - imod ) / (double)tf;
		else	gen_env = 0.0;
		// generation signal
		if	( imod == 0 )
			{
			switch	( i / tpu )
				{
				case 0 : f = f0 / kdf;	break;
				case 1 : f = f0;	break;
				case 2 : f = f0 * kdf;	break;
				}
			// convertir la frequence en rad/sample
			w = ( M_PI * 2.0 ) * f / Fs;
			}
		gen_sig = gen_env * cos( phase );
		phase += w;
		if	( phase > ( M_PI * 2.0 ) )
			phase -= ( M_PI * 2.0 );
		if	( knoise > 0.0 )
			{
			gen_sig += knoise * ( ( 2.0 * (double)rand() / (double)RAND_MAX ) - 1.0 );
			}
		// stockage valeurs pour plot
		fbuf[i] = (float)gen_sig;
		}

	}
};