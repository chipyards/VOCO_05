
enum gzoomdragmode { nil, select_zone, pan, zoom };

class gzoomdrag {
public :
gzoomdragmode mode;
double x0;
double y0;
double x1;
double y1;
// constructeur
gzoomdrag() : mode(nil) {};
// methodes
void zone( cairo_t * cair );
void box( cairo_t * cair );
void line( cairo_t * cair );
};


class gpanel : public panel {
public :
GtkWidget * widget;
GtkWidget * menu1_x;
GtkWidget * menu1_y;
gzoomdrag drag;
// methodes
GtkWidget * layout( int w, int h );
void configure();
void expose();
void clic( GdkEventButton * event );
void motion( GdkEventMotion * event );
void wheel( GdkEventScroll * event );
void key( GdkEventKey * event );
// pdf service widgets
GtkWidget * wchoo;
GtkWidget * fchoo;
GtkWidget * edesc;
// pdf service methods
void pdf_modal_layout( GtkWidget * mainwindow );
void pdf_ok_call();
// context menu service
GtkWidget * mkmenu1( const char * title );
int selected_strip;	// strip duquel on a appele le menu (avec les flags de marges)
// bindkey service
int selected_key;	// touche couramment pressee
};


class gzoombar {
public :
GtkWidget * widget;
gpanel * panneau;
int ww;	// largeur drawing area
double ndx;	// largeur nette
double xm;	// marge
double dxmin;	// barre minimale
double xlong;	// limite barre "longue"
double x0;	// extremite gauche
double x1;	// extremite droite
double xc;	// clic (debut drag)
double x0f;	// extremite gauche fantome
double x1f;	// extremite droite fantome
int opcode;	// 0 ou 'L', 'M', 'R'
// constructeur
gzoombar() : panneau(NULL), ww(640), xm(12), dxmin(3), xlong(36), opcode(0) {
	x0 = 100.0; x1 = 200.0;
	ndx = (double)ww - ( 2.0 * xm );
	};
// methodes
GtkWidget * layout( int w );
void configure();
void expose();
void clic( GdkEventButton * event );
void motion( GdkEventMotion * event );
void wheel( GdkEventScroll * event );
int op_select( double x );	// choisir l'operation en fonction du lieu du clic
void update( double x );	// calculer la nouvelle position de la barre (fantome)
void zoom( double k0, double k1 );	// mettre a jour la barre en fonction d'un cadrage normalise
};

// fonction C exportable
void gzoombar_zoom( void * z, double k0, double k1 );

