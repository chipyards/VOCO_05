// les classes pour le projet JLUPLOT
// en general on met les noms de classes en anglais, les membres en fr

// les fonctions graphiques recoivent un pointeur void qui peut
// servir a leur passer le contexte graphique, par exemple un cairo *

class panel;
class strip;

// une couleur pour Cairo
class jcolor {
public :
double dR;
double dG;
double dB;
// constructeurs
jcolor() : dR(0.0), dG(0.0), dB(0.0) {};
jcolor( double gray ) {
  dR = dG = dB = gray;
  }
void set( double R, double G, double B ) {
  dR = R; dG = G; dB = B;
  }
// cree une couleur parmi 8 (noir inclus, pas de blanc)
void arc_en_ciel( int i );
};

// une courbe (ou autre graphique) superposable : classe de base abstraite
class layer_base {
public :
strip * parent;		// au moins pour connaitre dx et dy
double u0;	// offset horizontal dans l'espace source (associe a x=0)
double u1;	// bord droit dans l'espace source
double ku;	// coeff horizontal source -> graphe bas niveau X (i.e. pixels), redondant avec u1
		// ku est calcule en fonction de u1 quand on zoome, quand on redimensionne la fenetre
		// sa presence est justifiee pour accelerer les conversions, surtout pour affichage
double v0;	// offset vertical dans l'espace source
double v1;	// bord superieur dans l'espace source, redondant avec v1
double kv;	// coeff vertical source -> graphe bas niveau Y (i.e. pixels)
		// kv est calcule en fonction de v1 quand on zoome, quand on redimensionne la fenetre
double m0;	// offset horizontal dans l'espace de ref (associe a u=0)
double km;	// coeff horizontal ref -> source
double n0;	// offset vertical dans l'espace de ref (associe a v=0)
double kn;	// coeff vertical ref -> source

jcolor fgcolor;		// couleur du trace
int ylabel;		// position label
string label;

// constructeur
layer_base() : parent(NULL),	u0(0.0), u1(1.0), ku(1.0), v0(0.0), v1(1.0), kv(1.0),
 				m0(0.0), km(1.0), n0(0.0), kn(1.0),
				fgcolor( 0.0 ), ylabel(20), label("") {};

// methodes de conversion
double u2x( double u ) { return( ( u - u0 ) * ku );  };
double v2y( double v ) { return( ( v - v0 ) * kv );  };
double x2u( double x ) { return( ( x / ku ) + u0 );  };
double y2v( double y ) { return( ( y / kv ) + v0 );  };
double m2u( double m ) { return( ( m - m0 ) * km );  };
double n2v( double n ) { return( ( n - n0 ) * kn );  };
double u2m( double u ) { return( ( u / km ) + m0 );  };
double v2n( double v ) { return( ( v / kn ) + n0 );  };

virtual double get_Umin() = 0;
virtual double get_Umax() = 0;
virtual double get_Vmin() = 0;
virtual double get_Vmax() = 0;
// fonctions cadrage (attention : risque de desalignement si appliquees sur 1 seul layer)
void zoomU( double umin, double umax );	// zoom absolu local
void zoomV( double vmin, double vmax );
/*void zoomX( double xmin, double xmax )	// zoom relatif
	{ zoomU( x2u(xmin), x2u(xmax) ); };*/
void zoomY( double ymin, double ymax )
	{ zoomV( y2v(ymin), y2v(ymax) ); };
void zoomM( double mmin, double mmax )	// zoom absolu
	{ zoomU( m2u(mmin), m2u(mmax) ); };
void zoomN( double nmin, double nmax )
	{ zoomV( n2v(nmin), n2v(nmax) ); };
void centerV( double dvtot );	// zoom Y tel que courbe centree et hauteur vue dvtot

void rezoomUV();	// restitution zoom anterieur apres resize
// dump
virtual void dump() {
  cout << "    courbe " << label << u0 << ":" << v0 << "->" << u1 << ":" << v1 << "\n";
  }
// dessin
virtual void draw( cairo_t * cai ) = 0;
};

// un strip, pour heberger une ou plusieurs courbes superposees
class strip {
public :
panel * parent;		// au moins pour connaitre dx
double R0;		// offset vertical graduations, valeur associee a m=0
double kR;		// coeff vertical graduations --> ref
double r0;		// offset vertical graduations, valeur associee a y=0
double r1;		// extremite superieure graduations, valeur associee a y=ndy
double kr;		// coeff vertical graduations --> y
double tdr;		// intervalle ticks Y dans l'espace graduations QR
double ftr;		// le premier tick Y dans l'espace graduations QR
			// = premier multiple de tdr superieur ou egal a n2r(v2n(v0))
unsigned int qtky;	// nombre de ticks
jcolor bgcolor;		// couleur du fond ou du cadre (selon optcadre)
jcolor lncolor;		// couleur du reticule
vector < layer_base * > courbes;
unsigned int fdy;	// full_dy = hauteur totale du strip (pixels)
unsigned int ndy;	// net_dy  = hauteur occupee par la courbe (pixels) = fdy - hauteur graduations
int optX;		// option pour l'axe X : 0 <==> ne pas mettre les graduations
int optcadre;		// option cadre : 0 <==> fill, 1 <==> traits
string Ylabel;
// constructeur
strip() : parent(NULL), R0(0.0), kR(1.0), r0(0.0), r1(1.0),kr(1.0),
	  tdr(10.0), ftr(1.0), qtky(11), bgcolor( 1.0 ), lncolor( 0.5 ),
	  fdy(100), ndy(100), optX(0), optcadre(0) {};
// methodes
double r2n( double r ) { return( ( r - R0 ) * kR );  };
double n2r( double n ) { return( ( n / kR ) + R0 );  };
double r2y( double r ) { return( ( r - r0 ) * kr );  };
double y2r( double y ) { return( ( y / kr ) + r0 );  };

void parentize() {
  for ( unsigned int i = 0; i < courbes.size(); i++ )
      courbes.at(i)->parent = this;
  };
// dump
void dump() {
unsigned int i;
cout << "  bg=" << bgcolor.dR << "," << bgcolor.dG << "," << bgcolor.dB << "\n";
cout << "  ln=" << lncolor.dR << "," << lncolor.dG << "," << lncolor.dB << "\n";
cout << "  fdy=" << fdy << ", " << courbes.size() << " courbes\n";
for ( i = 0; i < courbes.size(); i++ )
    courbes.at(i)->dump();
}
void zoomY( double ymin, double ymax );	// zoom relatif
void panY( double dy );			// pan relatif
void zoomYbyK( double k );		// zoom par un facteur
void panYbyK( double k );		// pan par un facteur
void zoomN( double nmin, double nmax );	// zoom absolu
void fullN();
// dessin
void draw( cairo_t * cai );
// event
int clicY( double y, double * py );
// int clicUV( double x, double y, double * pU, double * pV );
};


// le panel, pour heberger plusieurs strips (axe X commun ou identique)
class panel {
public :
double fullmmin;		// M min du full zoom
double fullmmax;		// M max du full zoom
double Q0;		// offset horizontal graduations, valeur associee a n=0
double kQ;		// coeff horizontal graduations --> ref
double q0;		// offset horizontal graduations, valeur associee a x=0
double q1;		// extremite droite graduations, valeur associee a x=ndx
double kq;		// coeff horizontal graduations --> x
double tdq;		// intervalle ticks X dans l'espace graduations QR
double ftq;		// le premier tick X dans l'espace graduations QR
			// = premier multiple de tdq superieur ou egal a m2q(u2m(u0))
unsigned int qtkx;	// nombre de ticks
int full_valid;		// indique que les coeffs de transformations sont dans un etat coherent
vector < strip > bandes;

unsigned int fdx;	// full_dx = largeur totale (pixels) mx inclus
unsigned int fdy;	// full_dy = hauteur totale (pixels)
unsigned int ndx;	// net_dx  = largeur nette des graphes (pixels) mx deduit

// details visuels
unsigned int mx;	// marge x pour les textes a gauche (pixels)
			// sert a translater le repere pour trace des courbes
unsigned int my;	// marge pour les textes de l'axe X (pixels)
// zoombar X optionnelle
void * zoombar;		// pointeur sur l'objet, a passer a la callback
void(* zbarcall)(void*, double, double); 	// callback de zoom normalise

// constructeur
panel() : Q0(0.0), kQ(1.0), q0(0.0), q1(1.0), kq(1.0),
	  tdq(10.0), ftq(1.0), qtkx(11), full_valid(0), fdx(200), fdy(200), mx(80), my(20),
	  zbarcall(NULL)
	  { ndx = fdx - mx; };
// methodes
double q2m( double q ) { return( ( q - Q0 ) * kQ );  };
double m2q( double m ) { return( ( m / kQ ) + Q0 );  };
double q2x( double q ) { return( ( q - q0 ) * kq );  };
double x2q( double x ) { return( ( x / kq ) + q0 );  };

void parentize() {
  for ( unsigned int i = 0; i < bandes.size(); i++ )
      {
      bandes.at(i).parent = this;
      bandes.at(i).parentize();
      }
  };
// dump
void dump() {
  unsigned int i;
  cout << "fdx=" << fdx << ", fdy=" << fdy << ", " << bandes.size() << " bandes\n";
  for ( i = 0; i < bandes.size(); i++ )
      bandes.at(i).dump();
  };
// dessin
void zoomX( double xmin, double xmax );	// zoom relatif
void panX( double dx );			// pan relatif
void zoomM( double mmin, double mmax );	// zoom absolu
void zoomXbyK( double k );	// zoom par un facteur
void panXbyK( double k );	// pan par un facteur
void fullM();
void fullMN();
void presize( int redx, int redy );	// met a jour les dimensions en pixels
void resize( int redx, int redy );	// met a jour les dimensions en pixels puis les zooms
void draw( cairo_t * cai );
// event
int clicXY( double x, double y, double * px, double * py );
int clicQR( double x, double y, double * pQ, double * pR );
// PDF
int pdfplot( const char * fnam, const char * caption );	// rend 0 si Ok
};

// constantes pour valeur retour methode clicXY
#define CLIC_OUT_X		-1
#define CLIC_OUT_Y		-2
#define CLIC_MARGE_GAUCHE	0x40000000
#define CLIC_MARGE_INF		0x20000000
#define CLIC_MARGE		(CLIC_MARGE_GAUCHE|CLIC_MARGE_INF)
#define CLIC_ZOOMBAR		0x10000000	// pour le cas ou context menu est commun avec zoombar

// fonctions non-membres diverses

// trouver une valeur de tick appropriee
double autotick( double range, unsigned int nmax );

// conversion double-->texte, avec un format de precision
// en rapport avec la valeur du tick.
// si la valeur du tick est nulle ou absente, utilisation
// du format anterieur memorise
// retour = nombre de chars utiles
// lbuf doit pouvoir contenir 32 bytes
int scientout( char * lbuf, double val, double tick=0.0 );
