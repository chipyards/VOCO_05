// testbench : generation de salves (chirps) d'enveloppe trapezoidale
class wgen {
public:
// parametres
double Fs;	// frequ. echant.
double f0;	// frequ. centrale
double knoise;	// proportion de bruit
char mode;	// c (chirps) or s (steps)
// chirps mode
double kdf;	// saut relatif de frequence (> 1.0)
int tpu;	// duree de chirp et d'intervalles, en periodes de f0
int trtf;	// temps de montee et descente, en periodes de f0
int qpu;	// nombre de chirps de part et d'autre de f0
		// le total sera qpu + 1 + qpu chirps
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

- la periode des chirps est 2 * tpu
- les temps de montee et descente sont pris sur les intervalles
*/
// steps mode
int qstep;	// nombre de paliers de frequence de chaque cote de f0
int spo;	// nombre de paliers par octave
int tstep;	// duree de palier, en periodes de f0
// variables derivees
int qsamples;
int _T0;	// periode de f0 exprimee en Ts, arrondie
int _tpu;	// duree de chirp, exprimee en Ts
int _trtf;	// temps de montee et descente, en Ts
int _ppu; 	// periode des chirps, exprimee en Ts
int _tstep;	// duree de palier, en Ts

// constructeur
wgen() {
	Fs = 48000.0;
	f0 = 500.0;
	mode = 's';
	kdf = SIXIEME_OCTAVE;	// pow( 2.0, 1/6.0 );
	knoise = 0.0;
	// durees exprimees en periodes T0 = 1/f0
	tpu = 42;
	trtf = 1;
	// quantites
	qpu = 2;
	qsamples = 0;
	qstep = 12000;	// 3/2 octaves de chaque cote
	spo = 8000;
	tstep = 1;
	};
int calc_size() {
	_T0 = (int)ceil( Fs / f0 );
	if	( mode == 'c' )
		{
		_tpu = tpu * _T0;
		_trtf = trtf * _T0;
		_ppu = 2 * _tpu;
		qsamples =   _ppu * ( qpu + 1 + qpu );
		qsamples += (_tpu - 2 * _trtf - 2 );	 // silence final
		}
	else if	( mode == 's' )
		{
		_tstep = tstep * _T0;
		qsamples = _tstep * ( qstep + 1 + qstep );
		}
	else	qsamples = 0;
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
	if	( mode == 'c' )
		{
		for	( int i = 0; i < qsamples; ++i )
			{
			// generation enveloppe d'un chirp periodique, periode _ppu samples
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
				gen_sig += knoise * ( ( 2.0 * (double)rand() / (double)RAND_MAX ) - 1.0 );
			// stockage valeurs pour plot
			fbuf[i] = (float)gen_sig;
			}
		}
	if	( mode == 's' )
		{
		kdf = pow( 2.0, 1.0/spo );
		for	( int i = 0; i < qsamples; ++i )
			{
			_t = i % _tstep;
			if	( _t == 0 )
				{
				f = f0 * pow( kdf, double((i/_tstep)-qstep) );
				// convertir la frequence en rad/sample
				w = ( M_PI * 2.0 ) * f / Fs;
				}
			//if	( _t == _t1 ) 
			//	printf("step %d %d, f=%g Hz\n", (i/_tstep), i, f );
			gen_sig = cos( phase );
			phase += w;
			if	( phase > ( M_PI * 2.0 ) )
				phase -= ( M_PI * 2.0 );
			if	( knoise > 0.0 )
				gen_sig += knoise * ( ( 2.0 * (double)rand() / (double)RAND_MAX ) - 1.0 );
			// stockage valeurs pour plot
			fbuf[i] = (float)gen_sig;
			}
		}
	};	// end generate()
};