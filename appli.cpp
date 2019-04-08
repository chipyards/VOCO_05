#include <gtk/gtk.h>
#include <cairo-pdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include "modpop.h"

using namespace std;
#include <string>
#include <iostream>
#include <vector>

#include "jluplot.h"
#include "layers.h"
#include "gluplot.h"
#include "glostru.h"
#include "jdsp.h"

/** ============================ DATA source ======================= */

#define QWAVES 4
static layer_f demowave[QWAVES];	// wave type float a pas uniforme

// ce jdsp est l'objet singleton de cette appli
static jdsp * jd;

// parametrer le jdsp en fonction des arguments de CLI
void param_simu( int argc, char ** argv )
{
jdsp_defaults();
jd = jdsp_get();	// pour pouvoir acceder directement aux params depuis le GUI...
printf("usage : jluplot <Fc(Hz)> <noise> <tf> <rect decay> <ripple_filter_ratio>\n");

double val;
if	( argc > 1 )
	{
	val = strtod( argv[1], NULL );
	if	( val > 30.0 )
		{
		jd->fc = val;	// durees d'abord exprimees en periodes de Fc
		jd->t0 = (int)( ( 1.75 / jd->fc ) * jd->Fs );
		jd->t1 = (int)( ( 32.0 / jd->fc ) * jd->Fs );
		jd->t2 = (int)( ( 24.0 / jd->fc ) * jd->Fs );
		}
	}
if	( argc > 2 )
	{				// val est sans dimension
	val = strtod( argv[2], NULL );
	jd->knoise = val;
	}
if	( argc > 3 )
	{
	int tmin;
	val = strtod( argv[3], NULL );
	jd->tf = (int)(val * jd->Fs / jd->fc );
	tmin = ( 4 * jd->tf ) / 3;
	if	( jd->t1 < tmin )
		{
		jd->t1 = tmin;
		jd->t2 = jd->t1;
		}
	}
if	( argc > 4 )
	{
	val = strtod( argv[4], NULL );	// en unites par sample
	if	( val > 0.0 )
		jd->rect_decay = val;
	}
if	( argc > 5 )
	{
	val = strtod( argv[5], NULL );	// sans dimension
	jd->rfr = val;
	}
printf("\nfreq. centrale fc = %g Hz\n", jd->fc );
printf("freq. ech. fs = %g Hz\n", jd->Fs );
printf("Async rectifier :\n  rect-decay = %g unit/sample\n", jd->rect_decay );
printf("  ripple filter ratio = %g\n", jd->rfr );
printf("Syn demodulator :\n  flp = %g Hz\n", jd->flp );
printf("Signal generator :\n  bruit/sin = %g\n", jd->knoise );
printf("  Facteur de serie de progression de frequence = %g\n", jd->kdf );
printf("  Duree de salve = %g periodes de fc\n", jd->t1 * jd->fc / jd->Fs );
printf("  Temps de montee et descente = %g periodes de fc\n\n", jd->tf * jd->fc / jd->Fs );
fflush( stdout );
}

// simu pour le l'affichage de waves
void run_simu()
{
// preparation des waves pour jluplot
// le tableau de pointeurs c'est pour le confort...
layer_f * w[QWAVES];
int tpu, qsamples;
int iw;
tpu = jd->t0 + jd->t1 + jd->t2;
qsamples = tpu * jd->qpu;
for	( iw = 0; iw < QWAVES; ++iw )
	{
	w[iw] = &demowave[iw];
	w[iw]->allocV( qsamples );
	if	( w[iw]->qu < qsamples )
		{
		printf("echec malloc %d samples\n", qsamples );
		exit( 1 );
		}
	}
printf("allocated %d * %d samples\n", QWAVES, w[0]->qu );
// preparation de la simu
int i, imod;
double phase;
double gen_env;		// enveloppe pour generateur
double gen_sig;		// signal genere
double ripp_out;	// sortie du ripple filter
double demod_out;	// sortie du demodulateur synchrone
// initialiser les composants du jdsp
jdsp_init();
// intialiser le generateur
phase = 0.0;

// boucle de calcul (generation signaux et DSP dans la meme boucle)
for	( i = 0; i < w[0]->qu; ++i )
	{
	// generation enveloppe d'une pulse periodique, periode tpu samples
	imod = i % tpu;
	if	( imod < jd->t0 )
		gen_env = 0.0;
	else if	( imod < ( jd->t0 + jd->tf ) )
		gen_env = (double)( imod - jd->t0 ) / (double)jd->tf;
	else if	( imod < ( jd->t0 + jd->t1 ) )
		gen_env = 1.0;
	else if	( imod < ( jd->t0 + jd->t1 + jd->tf ) )
		gen_env = (double)( jd->t0 + jd->t1 + jd->tf - imod ) / (double)jd->tf;
	else	gen_env = 0.0;
	// generation signal
	if	( imod == 0 )
		{
		switch	( i / tpu )
			{
			case 0 : jd->f = jd->fc / jd->kdf;	break;
			case 1 : jd->f = jd->fc;		break;
			case 2 : jd->f = jd->fc * jd->kdf;	break;
			}
		// convertir la frequence en rad/sample
		jd->k = ( M_PI * 2.0 ) * jd->f / jd->Fs;
		}
	gen_sig = gen_env * cos( phase );
	phase += jd->k;
	if	( phase > ( M_PI * 2.0 ) )
		phase -= ( M_PI * 2.0 );
	if	( jd->knoise > 0.0 )
		{
		gen_sig += jd->knoise * ( ( 2.0 * (double)rand() / (double)RAND_MAX ) - 1.0 );
		}
	// analyse
	ripp_out = envel_step( &jd->canal, gen_sig ) * 1.6;	// normalisation empirique (depend de rect. decay)
	demod_out = demod_step( &jd->demod, gen_sig ) * 2.0;	// normalisation theorique
	// stockage valeurs pour plot
	w[0]->V[i] = (float)gen_sig;
	w[2]->V[i] = (float)demod_out;
	w[3]->V[i] = (float)ripp_out;
	}
printf("simulation done\n"); fflush(stdout);
// preparation plot
w[0]->scan();
w[2]->scan();
w[3]->scan();
}


void prep_layout1( panel * panneau, int index )	// 1 strip avec N courbes
{
int flags;
switch	( index )
	{
	default : flags = 13;
	}
strip curbande; layer_base * curcour;

panneau->bandes.clear();
// creer un strip
curbande.bgcolor.dR = 0.95;
curbande.bgcolor.dG = 0.95;
curbande.bgcolor.dB = 0.98;
curbande.Ylabel = "float";

if	( flags & 8 )
	{
	curcour = &demowave[3];
	curcour->label = string("ripp_out");
	curcour->fgcolor.set( 1.0, 0.0, 0.0 );
	curbande.courbes.push_back( curcour );
	}
if	( flags & 4 )
	{
	curcour = &demowave[2];
	curcour->label = string("demod_out");
	curcour->fgcolor.arc_en_ciel( 4 );
	curbande.courbes.push_back( curcour );
	}
if	( flags & 2 )
	{
	curcour = &demowave[1];
	curcour->label = string("?");
	curcour->fgcolor.arc_en_ciel( 7 );
	curbande.courbes.push_back( curcour );
	}
if	( flags & 1 )
	{
	curcour = &demowave[0];
	curcour->label = string("input");
	curcour->fgcolor.set( 0.7, 0.8, 0.7 );
	curbande.courbes.push_back( curcour );
	}
curbande.optX = 1;
panneau->bandes.push_back( curbande );

// mettre a jour la parente
panneau->parentize();
// marge pour les textes
panneau->mx = 60;
}

/** ============================ call backs ======================= */

gint close_event_call( GtkWidget *widget,
                        GdkEvent  *event,
                        gpointer   data )
{ return(FALSE); }

void quit_call( GtkWidget *widget, glostru * glo )
{
gtk_widget_destroy( glo->wmain );
}

void left_call( GtkWidget *widget, glostru * glo )
{
// pan a gauche de 50% non borne
glo->panneau1.panXbyK( -0.5 );
gtk_widget_queue_draw( glo->darea1 );
}

void right_call( GtkWidget *widget, glostru * glo )
{
// pan a droite de 50% non borne
glo->panneau1.panXbyK( 0.5 );
gtk_widget_queue_draw( glo->darea1 );
}

void option_call( GtkWidget *widget, glostru * glo )
{
int ww, wh;
if	( ++glo->ilayout >= 8 )
	glo->ilayout = 0;
prep_layout1( &glo->panneau1, glo->ilayout );
gdk_drawable_get_size( glo->darea1->window, &ww, &wh );

glo->panneau1.full_valid = 0;
glo->panneau1.resize( ww, wh );

gtk_widget_queue_draw( glo->darea1 );
}

void save_call( GtkWidget *widget, glostru * glo )
{
}

void pdf_call( GtkWidget *widget, glostru * glo )
{
glo->panneau1.pdf_modal_layout( glo->wmain );
}

/** ============================ constr. GUI ======================= */

int main( int argc, char *argv[] )
{
glostru theglo;
#define glo (&theglo)
GtkWidget *curwidg;

// GError * pgerror;


gtk_init(&argc,&argv);
#include <locale.h>
setlocale( LC_ALL, "C" );       // kill the frog, AFTER gtk_init

curwidg = gtk_window_new( GTK_WINDOW_TOPLEVEL );

gtk_signal_connect( GTK_OBJECT(curwidg), "delete_event",
                    GTK_SIGNAL_FUNC( close_event_call ), NULL );
gtk_signal_connect( GTK_OBJECT(curwidg), "destroy",
                    GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );

gtk_window_set_title( GTK_WINDOW (curwidg), "JLUPLOT 01i");
gtk_container_set_border_width( GTK_CONTAINER( curwidg ), 10 );
glo->wmain = curwidg;

/* creer boite verticale */
curwidg = gtk_vbox_new( FALSE, 5 ); /* spacing ENTRE objets */
gtk_container_add( GTK_CONTAINER( glo->wmain ), curwidg );
glo->vmain = curwidg;

/* creer une drawing area resizable depuis la fenetre */
glo->darea1 = glo->panneau1.layout( 900, 500 );
gtk_box_pack_start( GTK_BOX( glo->vmain ), glo->darea1, TRUE, TRUE, 0 );

/* creer une drawing area  qui ne sera pas resizee en hauteur par la hbox
   mais quand meme en largeur (par chance !!!) */
glo->sarea = glo->zbar1.layout( 640 );
gtk_box_pack_start( GTK_BOX( glo->vmain ), glo->sarea, FALSE, FALSE, 0 );

/* creer boite horizontale */
curwidg = gtk_hbox_new( FALSE, 10 ); /* spacing ENTRE objets */
gtk_container_set_border_width( GTK_CONTAINER (curwidg), 5);
gtk_box_pack_start( GTK_BOX( glo->vmain ), curwidg, FALSE, FALSE, 0 );
glo->hbut = curwidg;

/* simple bouton */
curwidg = gtk_button_new_with_label (" << ");
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( left_call ), (gpointer)glo );
gtk_box_pack_start( GTK_BOX( glo->hbut ), curwidg, TRUE, TRUE, 0 );
glo->blef = curwidg;

/* simple bouton */
curwidg = gtk_button_new_with_label (" >> ");
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( right_call ), (gpointer)glo );
gtk_box_pack_start( GTK_BOX( glo->hbut ), curwidg, TRUE, TRUE, 0 );
glo->brig = curwidg;

/* simple bouton */
curwidg = gtk_button_new_with_label (" Next Layout ");
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( option_call ), (gpointer)glo );
gtk_box_pack_start( GTK_BOX( glo->hbut ), curwidg, TRUE, TRUE, 0 );
glo->bopt = curwidg;

/* simple bouton *
curwidg = gtk_button_new_with_label (" reserved ");
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( save_call ), (gpointer)glo );
gtk_box_pack_start( GTK_BOX( glo->hbut ), curwidg, TRUE, TRUE, 0 );
glo->bopt = curwidg;
//*/

/* simple bouton */
curwidg = gtk_button_new_with_label (" PDF ");
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( pdf_call ), (gpointer)glo );
gtk_box_pack_start( GTK_BOX( glo->hbut ), curwidg, TRUE, TRUE, 0 );
glo->bpdf = curwidg;

/* simple bouton */
curwidg = gtk_button_new_with_label (" Quit ");
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( quit_call ), (gpointer)glo );
gtk_box_pack_start( GTK_BOX( glo->hbut ), curwidg, TRUE, TRUE, 0 );
glo->bqui = curwidg;

// connecter la zoombar au panel1 et inversement
glo->panneau1.zoombar = &glo->zbar1;
glo->panneau1.zbarcall = gzoombar_zoom;
glo->zbar1.panneau = &glo->panneau1;

param_simu( argc, argv );
run_simu();

glo->ilayout = 0;
prep_layout1( &glo->panneau1, glo->ilayout );
// glo->panneau1.dump();


gtk_widget_show_all( glo->wmain );

gtk_main();

#ifdef CAIRO_HAS_PNG_FUNCTIONS
printf("CAIRO_HAS_PNG_FUNCTIONS\n");
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
printf("CAIRO_HAS_PDF_SURFACE\n");
#endif

return(0);
}
