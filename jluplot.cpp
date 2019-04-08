// les classes pour le projet JLUPLOT v. 0.1b

using namespace std;
#include <string>
#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include <cairo-pdf.h>
#include <stdlib.h>
#include <math.h>
#include "jluplot.h"

// cree une couleur parmi 8 (noir inclus, pas de blanc)
void jcolor::arc_en_ciel( int i )
{
switch( i % 8 )
   {
   case 0 : dR = 0.0; dG = 0.0; dB = 0.0; break;
   case 1 : dR = 1.0; dG = 0.0; dB = 0.0; break;
   case 2 : dR = 0.9; dG = 0.4; dB = 0.0; break;
   case 3 : dR = 0.7; dG = 0.9; dB = 0.0; break;
   case 4 : dR = 0.0; dG = 0.8; dB = 0.1; break;
   case 5 : dR = 0.0; dG = 0.0; dB = 1.0; break;
   case 6 : dR = 0.4; dG = 0.0; dB = 0.7; break;
   case 7 : dR = 0.7; dG = 0.0; dB = 0.3; break;
   }
}

// une courbe (ou autre graphique) superposable : classe de base abstraite

// zoom absolu local, i.e. pilote en unites source
void layer_base::zoomU( double umin, double umax )
{
if	( ( umax - umin ) <= 0.0 )
	umax = umin + 1.0;		// pas terrib comme soluce mais evite le clash
u0 = umin; u1 = umax;
ku = (double)parent->parent->ndx / ( u1 - u0 );
// printf("zoom range %g, k=%g\n", ( u1 - u0 ), ku );
}

void layer_base::zoomV( double vmin, double vmax )
{
if	( ( vmax - vmin ) <= 0.0 )
	vmax = vmin + 1.0;		// pas terrib comme soluce mais evite le clash
v0 = vmin; v1 = vmax;
kv = (double)parent->ndy / ( v1 - v0 );
// printf("zoom range %g, k=%g\n", ( v1 - v0 ), kv );
}

// zoom sur place, utile si dimensions fenetre ont change
void layer_base::rezoomUV()
{
// printf("rezoom %g:%g --> %g:%g\n", u0, v0, u1, v1 );
zoomU( u0, u1 );
zoomV( v0, v1 );
}

// zoom Y tel que courbe centree et hauteur vue dvtot
// attention s'il y a plusieurs layers ils seront desalignes !
void layer_base::centerV( double dvtot )
{
double vcenter = ( get_Vmax() + get_Vmin() ) * 0.5;
dvtot *= 0.5;
zoomV( vcenter - dvtot, vcenter + dvtot );
}

// un strip, pour heberger une ou plusieurs courbes superposees
// dessin (la largeur dx est lue chez le panel parent)

void strip::draw( cairo_t * cai )
{
unsigned int ddx;

ddx = parent->ndx;

// faire le fond
cairo_set_source_rgb( cai, bgcolor.dR, bgcolor.dG, bgcolor.dB );
cairo_rectangle( cai, 0, 0, ddx, ndy );
if   ( optcadre )
     cairo_stroke( cai );
else cairo_fill( cai );

// tracer le reticule x
double curq, curx; char lbuf[32];

// origine Y est en haut du strip...
// la grille de barres verticales
cairo_set_source_rgb( cai, lncolor.dR, lncolor.dG, lncolor.dB );
curq = parent->ftq;
curx = parent->q2x(curq);			// la transformation
while ( curx < ddx )
   {
   cairo_move_to( cai, curx, 0 );			// top
   cairo_line_to( cai, curx, (optX)?(ndy+5):(fdy) );	// bottom
   cairo_stroke( cai );
   curq += parent->tdq;
   curx = parent->q2x(curq);			// la transformation
   }

// les textes
if ( optX )
   {
   cairo_set_source_rgb( cai, 0, 0, 0 );
   curq = parent->ftq;
   // preparation format selon premier point
   scientout( lbuf, curq, parent->tdq );
   curx = parent->q2x(curq);			// la transformation
   while ( curx < ddx )
      {
      scientout( lbuf, curq );
      cairo_move_to( cai, curx - 20, ndy + 15 );
      cairo_show_text( cai, lbuf );
      curq += parent->tdq;
      curx = parent->q2x(curq);			// la transformation
      }
   }

// translater l'origine Y en bas de la zone utile des courbes
// l'origine X est deja au bord gauche de catte zone
cairo_save( cai );
cairo_translate( cai, 0, ndy );

// tracer le reticule y
double curr, cury, maxy;

maxy = -((double)ndy);		// le haut (<0)

// la grille de barres horizontales
cairo_set_source_rgb( cai, lncolor.dR, lncolor.dG, lncolor.dB );
curr = ftr;
cury = -r2y( curr );			// la transformation
while ( cury > maxy )
   {
   cairo_move_to( cai, -6, cury );
   cairo_line_to( cai, ddx, cury );
   cairo_stroke( cai );
   curr += tdr;
   cury = -r2y( curr );		// la transformation
   }

// les textes
cairo_set_source_rgb( cai, 0.0, 0.0, 0.0 );
curr = ftr;
// preparation format selon premier point
scientout( lbuf, curr, tdr );
cury = -r2y( curr );			// la transformation
while ( cury > ( maxy + 19 ) )		// petite marge pour label axe
   {
   scientout( lbuf, curr );
   cairo_move_to( cai, -parent->mx + 10, cury + 5 );
   cairo_show_text( cai, lbuf );
   curr += tdr;
   cury = -r2y( curr );			// la transformation
   }
// label de l'axe (unites)
cairo_move_to( cai, -parent->mx + 10, maxy + 14 );
cairo_show_text( cai, Ylabel.c_str() );

// tracer les courbes
int i;
for ( i = ( courbes.size() - 1 ); i >= 0; i-- )
    {
    courbes.at(i)->ylabel = ( 20 * i ) + 20;
    courbes.at(i)->draw( cai );
    }

cairo_restore( cai );
}

// zoom relatif
void strip::zoomY( double ymin, double ymax )
{
// premiere etape : mise a jour des coeffs de conversion dans chaque layer
for	( unsigned int ic = 0; ic < courbes.size(); ic++ )
	courbes.at(ic)->zoomY( ymin, ymax );
// seconde etape : mise a jour de la conversion R <--> Y
r1 = y2r( ymax );	// attention y2r() depend de r0 !!!
r0 = y2r( ymin );
kr = (double)ndy / ( r1 - r0 );
// troisieme etape : mise a jour des ticks
tdr = autotick( r1 - r0, qtky );
// premier multiple de tdr superieur ou egal a rmin
ftr = ceil( r0 / tdr );
ftr *= tdr;
}

// pan relatif, par un deplacement Y
void strip::panY( double dy )
{
double yl, yr, wy;
wy = (double)ndy;
yl = dy;
yr = wy + dy;
zoomY( yl, yr );
}

// zoom par un facteur, t.q. 0.5 pour zoom in, 2.0 pour zoom out
void strip::zoomYbyK( double k )
{
double yl, yr, wy;
wy = (double)ndy;
yl = wy * 0.5 * ( 1 - k );
yr = yl + ( wy * k );
zoomY( yl, yr );
}

// pan par un facteur, t.q. 0.5 pour la moitie de la largeur a droite
void strip::panYbyK( double k )
{
double yl, yr, wy;
wy = (double)ndy;
yr = wy;
wy *= k;
yl = wy;
yr += wy;
zoomY( yl, yr );
}

// zoom absolu
void strip::zoomN( double nmin, double nmax )
{
// premiere etape : mise a jour des coeffs de conversion de chaque layer
for	( unsigned int ic = 0; ic < courbes.size(); ic++ )
	courbes.at(ic)->zoomN( nmin, nmax );
// seconde etape : mise a jour de la conversion R <--> Y
r0 = n2r( nmin );
r1 = n2r( nmax );
kr = (double)ndy / ( r1 - r0 );
// troisieme etape : mise a jour des ticks
tdr = autotick( r1 - r0, qtky );
// premier multiple de tdr superieur ou egal a vmin
ftr = ceil( r0 / tdr );
ftr *= tdr;
}

void strip::fullN()
{
// premiere etape : acquisition des valeurs limites
double nmin, nmax, nn;
layer_base * la;
nmin = HUGE_VAL;
nmax = -HUGE_VAL;
for	( unsigned int ic = 0; ic < courbes.size(); ic++ )
	{
	la = courbes.at(ic);
	nn = la->v2n(la->get_Vmin());
	if	( nn < nmin )
		nmin = nn;
	nn = la->v2n(la->get_Vmax());
	if	( nn > nmax )
		nmax = nn;
	}
// seconde etape : action
double dn = nmax - nmin;
dn *= 0.05;	// marges 5% pour compatibilité visuelle avec jluplot 0
nmin -= dn; nmax += dn;
zoomN( nmin, nmax );
}

// cette fonction interprete la coordonnee y (mouse) relatives au strip
// et la convertit en y (jluplot) relative au strip
// rend 0 si ok ou -1 si hors strip en Y
// *py sera negatif si on est dans la marge inferieure (c'est a dire dans la graduation X)
int strip::clicY( double y, double * py )
{
// N.B. x est deja verifie au niveau panel
if ( ( y < 0.0 ) || ( y >= fdy ) || ( courbes.size() == 0 ) )
   return -1;
*py = ndy - y;	// translater origine Y en bas de la zone
return 0;
}

// le panel, pour heberger plusieurs strips (axe X commun ou identique)

// met a jour les dimensions en pixels
void panel::presize( int redx, int redy )
{
unsigned int ddy, i;
fdx = redx;
fdy = redy;
ndx = fdx - mx;
if ( bandes.size() )
   {
   ddy = fdy / bandes.size();
   // bandes d'egale hauteur
   for ( i = 0; i < bandes.size(); i++ )
       bandes.at(i).fdy = ddy;
   // ajustement pour prendre en compte le reste de la division
   bandes.at(0).fdy += ( fdy % bandes.size() );
   // calcul hauteurs nettes
   for ( i = 0; i < bandes.size(); i++ )
       {
       bandes.at(i).ndy = bandes.at(0).fdy - ((bandes.at(i).optX)?(my):(0));
       }
   }
}

// met a jour les dimensions en pixels puis les zooms
void panel::resize( int redx, int redy )
{
presize( redx, redy );
// test pour savoir si on doit faire un full
if	( full_valid == 0 )
	{ printf("auto fullNM\n" ); fullMN(); dump(); return; }
// restituer zooms en cours sur chaque layer
strip * b;
for	( unsigned int ib = 0; ib < bandes.size(); ib++ )
	{
	b = &bandes.at(ib);
	for	( unsigned int ic = 0; ic < b->courbes.size(); ic++ )
		{
		printf("layer %d.%d rezoom ", ib, ic );
		b->courbes.at(ic)->rezoomUV();
		}
	b->kr = (double)b->ndy / ( b->r1 - b->r0 );
	}
kq = (double)ndx / ( q1 - q0 );
}

// zoom relatif
void panel::zoomX( double xmin, double xmax )
{
double mmin, mmax;
mmin = q2m( x2q( xmin ) );
mmax = q2m( x2q( xmax ) );
zoomM( mmin, mmax );
}

// pan relatif
void panel::panX( double dx )
{
double xl, xr, wx;
wx = (double)ndx;
xl = dx;
xr = wx + dx;
zoomX( xl, xr );
}

// zoom par un facteur, t.q. 0.5 pour zoom in, 2.0 pour zoom out
void panel::zoomXbyK( double k )
{
double xl, xr, wx;
wx = (double)ndx;
xl = wx * 0.5 * ( 1 - k );
xr = xl + ( wx * k );
zoomX( xl, xr );
}

// pan par un facteur, t.q. 0.5 pour la moitie de la largeur a droite
void panel::panXbyK( double k )
{
double xl, xr, wx;
wx = (double)ndx;
xr = wx;
wx *= k;
xl = wx;
xr += wx;
zoomX( xl, xr );
}

// zoom absolu
void panel::zoomM( double mmin, double mmax )
{
// premiere etape : mise a jour des coeffs de conversion
strip * b = &bandes[0];
for	( unsigned int ib = 0; ib < bandes.size(); ib++ )
	{
	b = &bandes.at(ib);
	for	( unsigned int ic = 0; ic < b->courbes.size(); ic++ )
		b->courbes.at(ic)->zoomM( mmin, mmax );
	}
// seconde etape : mise a jour de la conversion Q <--> X
q0 = m2q( mmin );
q1 = m2q( mmax );
kq = (double)ndx / ( q1 - q0 );
// troisieme etape : mise a jour des ticks
tdq = autotick( q1 - q0, qtkx );
// premier multiple de tdq superieur ou egal a qmin
ftq = ceil( q0 / tdq );
ftq *= tdq;
// quatrieme etape : mise a jour scrollbar si elle existe
if	( zbarcall )
	zbarcall( zoombar, ( mmin - fullmmin ) / ( fullmmax - fullmmin ),
			   ( mmax - fullmmin ) / ( fullmmax - fullmmin ) );
}

void panel::fullM()
{
strip * b;
layer_base * la;
// premiere etape : acquisition des valeurs limites
double mmin, mmax, mm;
mmin = HUGE_VAL;
mmax = -HUGE_VAL;
for	( unsigned int ib = 0; ib < bandes.size(); ib++ )
	{
	b = &bandes.at(ib);
	for	( unsigned int ic = 0; ic < b->courbes.size(); ic++ )
		{
		la = b->courbes.at(ic);
		mm = la->u2m(la->get_Umin());
		if	( mm < mmin )
			mmin = mm;
		mm = la->u2m(la->get_Umax());
		if	( mm > mmax )
			mmax = mm;
		}
	}
// seconde etape : action
fullmmin = mmin; fullmmax = mmax;
printf("in fullM, full is %g to %g\n", fullmmin, fullmmax );
zoomM( mmin, mmax );
}

void panel::fullMN()
{
unsigned int i;
for ( i = 0; i < bandes.size(); i++ )
    {
    bandes.at(i).fullN();
    }
fullM();
full_valid = 1;
}

void panel::draw( cairo_t * cai )
{
unsigned int i;

cairo_save( cai );
cairo_translate( cai, mx, 0 );
// on veut la courbe 0 en haut...
for ( i = 0; i < bandes.size(); i++ )
    {
    bandes.at(i).draw( cai );
    cairo_translate( cai, 0, bandes.at(i).fdy );
    }
cairo_restore( cai );
}

// cadrage auto si dvtot <= 0.0, sinon "centerY" rend 0 si Ok
int panel::pdfplot( const char * fnam, const char * caption )
{
cairo_surface_t * cursurf;
cairo_status_t cairo_status;
cairo_t * lecair;
double pdf_w, pdf_h;

// format A4 landscape
pdf_w = ( 297.0 / 25.4 ) * 72;
pdf_h = ( 210.0 / 25.4 ) * 72;

cursurf = cairo_pdf_surface_create( fnam, pdf_w, pdf_h );

cairo_status = cairo_surface_status( cursurf );
if   ( cairo_status != CAIRO_STATUS_SUCCESS )
     return( cairo_status );

lecair = cairo_create( cursurf );

// un peu de marge (en points)
cairo_translate( lecair, 24.0, 24.0 );

resize( (int)pdf_w - 48, (int)pdf_h - 72 );

// preparer font a l'avance
cairo_select_font_face( lecair, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
cairo_set_font_size( lecair, 12.0 );
cairo_set_line_width( lecair, 0.5 );

// draw the curves
draw( lecair );

// the caption
cairo_move_to( lecair, 10.0, pdf_h - 72.0 + 24 );
cairo_show_text( lecair, caption );

cairo_destroy( lecair );
cairo_surface_destroy( cursurf );
return 0;
}

// cette fonction interprete les coordonnees x,y (mouse) relatives a la drawing-area
// et les convertit en x, y (jluplot) relatives au strip,
// rend une valeur negative si hors graphique,
// rend l'indice du strip avec eventuellement un flag bit si clic dans une marge
int panel::clicXY( double x, double y, double * px, double * py )
{
// pre-traitement X
if	( ( x < 0.0 ) || ( x >= (double)fdx ) )
	return CLIC_OUT_X;
*px = x - (double)mx;			// x < 0 si on est dans les graduations Y
// identification strip
int istrip;
for ( istrip = 0; istrip < (int)bandes.size(); istrip++ )
    {
    if  ( bandes.at(istrip).clicY( y, py ) == 0 )
	break;
    y -= bandes.at(istrip).fdy;	// transformation inverse (cf panel::draw)
    }
if	( istrip >= (int)bandes.size() )
	return CLIC_OUT_Y;
// ici on a identifie un strip, *px et *py sont a jour, on va signaler si on est dans les marges
if	( *px < 0.0 )
	istrip |= CLIC_MARGE_GAUCHE;
if	( *py < 0.0 )
	istrip |= CLIC_MARGE_INF;
return istrip;
}

// cette fonction interprete les coordonnees x,y relatives a la drawing-area
// et les convertit en Q,R
// rend une valeur negative si hors graphique,
// rend l'indice du strip avec eventuellement un flag bit si clic dans une marge
int panel::clicQR( double x, double y, double * pQ, double * pR )
{
int istrip = clicXY( x, y, &x, &y );
if	( istrip < 0 )
	return istrip;
*pQ = x2q(x);	// les transformations
*pR = bandes.at(istrip&(~CLIC_MARGE)).y2r(y);
return istrip;
}


// trouver une valeur de tick appropriee
double autotick( double range, unsigned int nmax )
{
static const double coef[] = { 5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05, 0.02, 0.01, 0.005, 0.002, 0.001 };
// puissance de 10 immediatement inferieure a range
double tickbase = exp( floor( log10( range ) ) * log(10.0) );
// note : log10(x) equals log(x) / log(10).
// note : exp10(x) is the same as exp(x * log(10)), this functions is GNU extension.

// augmenter progressivement le nombre de ticks
unsigned int i = 0; double cnt, tickspace;
while ( i < ( sizeof(coef) / sizeof(double) ) )
   {
   tickspace = tickbase * coef[i];
   cnt = floor( range / tickspace );
   if ( (unsigned int)cnt > nmax )
      break;
   ++i;
   }
if ( i ) --i;
// printf("autotick( %g, %d ) --> %g\n", range, nmax, tickbase * coef[i] );
return( tickbase * coef[i] );
}

// conversion double-->texte, avec un format de precision
// en rapport avec la valeur du tick.
// si la valeur du tick est nulle ou absente, utilisation
// du format anterieur memorise
// retour = nombre de chars utiles
// lbuf doit pouvoir contenir 32 bytes
int scientout( char * lbuf, double val, double tick )
{
//                          0123456789
static const char *osuff = "fpnum kMGT";
// ces deux valeurs memorisent le format
static int triexp = 0;	// exposant, multiple de 3
static int preci = 1;	// nombre digits apres dot
if ( tick > 0.0 )	// preparation format
   {
   int tikexp; double aval;
   // normalisation
   aval = fabs(val);
   if ( aval < tick )	// couvre le cas val == 0.0
      aval = tick;
   triexp = (int)floor( log10( aval ) / 3 );
   triexp *= 3;
   // ordre de grandeur du tick (en evitant arrondi du 1 vers 0.99999)
   tikexp = (int)floor( log10(tick) + 1e-12 );
   preci = triexp - tikexp;
   if ( preci < 1 ) preci = 1;
   // printf("~>%d <%d> .%d\n", triexp, tikexp, preci );
   }
if ( val == 0.0 )
   { sprintf( lbuf, "0" ); return 1; }
unsigned int cnt;	// affichage selon format
if ( triexp )
   val *= exp( -triexp * log(10) );
// int snprintf (char *s, size t size, const char *template, . . . )
// rend le nombre de chars utiles
cnt = snprintf( lbuf, 20, "%.*f", preci, val );
if ( triexp == 0 )
   return cnt;
if ( ( triexp < -15 ) || ( triexp > 12 ) )
   {
   cnt += snprintf( lbuf + cnt, 8, "e%d", triexp );
   return cnt;
   }
sprintf( lbuf + cnt, "%c", osuff[ ( triexp/3 ) + 5 ] );
return( cnt + 1 );
}
