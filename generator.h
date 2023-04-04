// testbench : generation de salves d'enveloppe trapezoidale
class wgen {
public:
// parametres
double Fs;	// frequ. echant.
double f0;	// frequ. centrale
double kdf;	// saut relatif de frequence (> 1.0)
double knoise;	// proportion de bruit
int tpu;	// duree de pulse et d'intervalles, en periodes de f0
int trtf;	// temps de montee et descente, en periodes de f0
int qpu;	// nombre de pulses de part et d'autre de f0
		// le total sera qpu + 1 + qpu pulses
/*
	              ______________
                     /              \                /
                    /                \              /
	           /                  \            /
	__________/                    \__________/
        |         |   |             |   |
        |         |   t1            |   ppu
	|         |   =tpu-trtf     |   =2*tpu
	0	  t0                t2
	          =tpu-2*trtf       =2*tpu-trtf          

- la periode des pulses est 2 * tpu
- les temps de montee et descente sont pris sur les intyervalles
*/

// variables derivees
int qsamples;
int _T0;	// periode de f0 exprimee en Ts, arrondie
int _tpu;	// duree de pulse, exprimee en Ts
int _trtf;	// temps de montee et descente, en Ts
int _ppu; 	// periode des pulses, exprimee en Ts

// constructeur
wgen() {
	Fs = 48000.0;
	f0 = 500.0;
	kdf = pow( 2.0, 1/3.0 );	// tiers d'octave
	knoise = 0.0;
	// durees exprimees en periodes T0 = 1/f0
	tpu = 42;
	trtf = 1;
	qpu = 1;
	qsamples = 0;
	};
int calc_size() {
	_T0 = (int)ceil( Fs / f0 );
	_tpu = tpu * _T0;
	_trtf = trtf * _T0;
	_ppu = 2 * _tpu;
	qsamples =   _ppu * ( qpu + 1 + qpu );
	qsamples += (_tpu - 2 * _trtf - 2 );	 // silence final 
	return qsamples;
	};
void generate( float * fbuf ) {
	double f = 0.0;		// Hz, frequence courante
	double w = 0.0;		// rad/sample, frequence courante
	int _t;			// temps relatif, en samples
	double gen_env;		// enveloppe pour generateur
	double gen_sig;		// signal genere
	double phase = 0.0;

	calc_size();

	int _t0 = _tpu - 2 * _trtf;	// debut montee
	int _t1 = _tpu - _trtf;		// fin montee
	int _t2 = _ppu - _trtf;		// debut descente

	// boucle de calcul generation signal
	for	( int i = 0; i < qsamples; ++i )
		{
		// generation enveloppe d'une pulse periodique, periode _ppu samples
		_t = i % _ppu;
		if	( _t < _t0 )	// silence
			gen_env = 0.0;
		else if	( _t < _t1 )	// montee
			gen_env = double( _t - _t0 ) / double(_trtf);
		else if	( _t < _t2 )	// palier
			gen_env = 1.0;
		else			// descente	
			gen_env = double( _ppu - _t ) / double(_trtf);
		// generation signal
		if	( _t == 0 )
			{
			f = f0 * pow( kdf, double((i/_ppu)-qpu) );
			// convertir la frequence en rad/sample
			w = ( M_PI * 2.0 ) * f / Fs;
			}
		if	( _t == _t1 ) 
			printf("chirp %d %d, f=%g Hz\n", (i/_ppu), i, f );
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

	};
};