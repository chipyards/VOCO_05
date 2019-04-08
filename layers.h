
// une courbe a pas non-uniforme en double float (classe derivee de layer_base)
// l'allocation mémoire est totalement déportée, securité zéro
class layer_dd : public layer_base {
public :

double * U;
double * V;
double Umin, Umax, Vmin, Vmax;
int qu;		// nombre de points effectif
int curi;	// index point courant

// constructeur
layer_dd() : layer_base(),
	  Umin(0.0), Umax(1.0), Vmin(0.0), Vmax(1.0), qu(0), curi(0) {};

// methodes propres a cette classe derivee

int goto_U( double U0 );		// chercher le premier point U >= U0
void goto_first();
int get_pi( double & rU, double & rV );	// get XY then post increment
void scan();				// mettre a jour les Min et Max
void allocUV( size_t size );		// ebauche de service d'allocation

// les methodes qui sont virtuelles dans la classe de base

void dump() {
  cout << "    courbe dd " << label << ", " << u0 << ":" << v0 << "->" << u1 << ":" << v1 << "\n";
  }
double get_Umin();
double get_Umax();
double get_Vmin();
double get_Vmax();
void draw( cairo_t * cai );	// dessin
};

// une courbe a pas uniforme en float (classe derivee de layer_base)
// l'allocation mémoire est totalement déportée, securité zéro
class layer_f : public layer_base {
public :

float * V;
float Vmin, Vmax;
int qu;		// nombre de points effectif
int curi;	// index point courant

// constructeur
layer_f() : layer_base(),
	  Vmin(0.0), Vmax(1.0), qu(0), curi(0) {};

// methodes propres a cette classe derivee

int goto_U( double U0 );		// chercher le premier point U >= U0
void goto_first();
int get_pi( double & rU, double & rV );	// get XY then post increment
void scan();				// mettre a jour les Min et Max
void allocV( size_t size );		// ebauche de service d'allocation

// les methodes qui sont virtuelles dans la classe de base

void dump() {
  cout << "    courbe f  " << label << ", " << u0 << ":" << v0 << "->" << u1 << ":" << v1 << "\n";
  }
double get_Umin();
double get_Umax();
double get_Vmin();
double get_Vmax();
void draw( cairo_t * cai );	// dessin
};
