#include <gdk/gdkkeysyms.h>	// pour les touches du pave num.
#include <gtk/gtk.h>
#include <cairo-pdf.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;
#include <string>
#include <iostream>
#include <vector>

#include "jluplot.h"
#include "gluplot.h"
#include "layer_u.h"

#include "modpop3.h"
#include "cli_parse.h"
#include "glostru.h"
#include "generator.h"
#include "jdsp.h"

// unique variable globale exportee pour gasp() de modpop3
GtkWindow * global_main_window = NULL;

// le contexte de l'appli
static glostru theglo;

/** ============================ DATA source ======================= */

#define QWAVES 4
static layer_u<float> demowave[QWAVES];	// wave type float a pas uniforme

// ce jdsp est l'objet singleton de cette appli
static jdsp jd;
// ce generator aussi
static wgen gen;

// parametrer le jdsp en fonction des arguments de CLI
void param_simu( int argc, char ** argv )
{
printf("usage : \n");

double val;
gen.Fs = jd.Fs;
gen.f0 = jd.f0;

printf("\nfreq. centrale f0 = %g Hz\n", jd.f0 );
printf("freq. ech. fs = %g Hz\n", jd.Fs );
printf("Async rectifier :\n  rect-decay = %g unit/sample\n", jd.rect_decay );
printf("  ripple filter ratio = %g\n", jd.rfr );
printf("Syn demodulator low pass:\n  kflp = %g\n", jd.kflp );
printf("Signal generator :\n  bruit/sin = %g\n", gen.knoise );
printf("  Facteur de serie de progression de frequence = %g\n", gen.kdf );
printf("  Duree de salve = %d periodes de f0\n", gen.tpu );
printf("  Temps de montee et descente = %d periodes de f0\n\n", gen.trtf );
fflush( stdout );
}

// simu pour le l'affichage de waves
void run_simu()
{
// preparation des waves pour jluplot
// d'abord evaluer la taille
int qsamples = gen.calc_size();
// allouer la memoire
layer_u<float> * w[QWAVES];

for	( int iw = 0; iw < QWAVES; ++iw )
	{
	w[iw] = &demowave[iw];
	w[iw]->V = (float *)malloc( qsamples * sizeof(float) );
	w[iw]->qu = qsamples;
	if	( w[iw]->V == NULL )
		{
		printf("echec malloc %d samples\n", qsamples );
		exit( 1 );
		}
	}
printf("allocated %d * %d samples\n", QWAVES, w[0]->qu );

// actionner le generateur
gen.f0 = jd.f0;
gen.generate( w[0]->V );

// initialiser les composants du jdsp
jd.update();

// boucle de calcul DSP
for	( int i = 0; i < w[0]->qu; ++i )
	{
	w[3]->V[i] = jd.canal_step( w[0]->V[i] ) * 1.6;	// normalisation empirique (depend de rect. decay)
	w[2]->V[i] = jd.demod_step( w[0]->V[i] ) * 2.0;	// normalisation theorique
	}

printf("simulation done\n"); fflush(stdout);
// preparation plot
w[0]->scan();
w[2]->scan();
w[3]->scan();
}

// 1 strip avec N courbes
void prep_layout1( gpanel * panneau1 )
{
// marge pour les textes
// panneau1.mx = 60;
panneau1->offscreen_flag = 0;

// creer le strip pour les waves
gstrip * curbande;
curbande = new gstrip;
panneau1->add_strip( curbande );

// configurer le strip
curbande->bgcolor.set( 0.95, 0.95, 0.98 );
curbande->Ylabel = "float";
curbande->optX = 1;
curbande->subtk = 2;

// creer un pointeur pour le layer courant
layer_u<float> * curcour;

// referencer un layer
// curcour = new layer_u<float>;  // bah non on a cree les layers en static global
curcour = &demowave[3];
curbande->add_layer( curcour, "ripp_out" );
// configurer ce layer (APRES add_layer)
curcour->fgcolor.arc_en_ciel( 2 );

curcour = &demowave[2];
curbande->add_layer( curcour, "demod_out" );
// configurer ce layer (APRES add_layer)
curcour->fgcolor.arc_en_ciel( 6 );

curcour = &demowave[0];
curbande->add_layer( curcour, "input" );
// configurer ce layer (APRES add_layer)
curcour->fgcolor.arc_en_ciel( 3 );
}

/** ============================ call backs ======================= */

int idle_call( glostru * glo )
{
// moderateur de drawing
if	( glo->panneau1.force_repaint )
	glo->panneau1.paint();
return( -1 );
}

gint close_event_call( GtkWidget *widget, GdkEvent *event, gpointer data )
{
gtk_main_quit();
return (TRUE);		// ne pas destroyer tout de suite
}

void quit_call( GtkWidget *widget, glostru * glo )
{
gtk_main_quit();
}

void pdf_dialogs( glostru * glo, int fast )
{
if	( fast )
	{
	char fnam[32], capt[128];
	snprintf( fnam, sizeof(fnam), "voco.pdf" );
	modpop_entry( "PDF plot", "nom du fichier", fnam, sizeof(fnam), GTK_WINDOW(glo->wmain) );
	snprintf( capt, sizeof(capt), "C'est imposant ma soeur" );
	modpop_entry( "PDF plot", "description", capt, sizeof(capt), GTK_WINDOW(glo->wmain) );
	glo->panneau1.pdfplot( fnam, capt );
	}
else	glo->panneau1.pdf_modal_layout( glo->wmain );
}


/** ============================ GLUPLOT call backs =============== */

void clic_call_back( double M, double N, void * vglo )
{
printf("clic M N %g %g\n", M, N );
// glostru * glo = (glostru *)vglo;
}

void key_call_back( int v, void * vglo )
{
glostru * glo = (glostru *)vglo;
switch	( v )
	{
	// la visibilite
	case GDK_KEY_KP_0 :
	case '0' : glo->panneau1.toggle_vis( 0, 0 ); break;
	case GDK_KEY_KP_1 :
	case '1' : glo->panneau1.toggle_vis( 0, 1 ); break;
	case GDK_KEY_KP_2 :
	case '2' : glo->panneau1.toggle_vis( 0, 2 ); break;
	// l'option offscreen drawpad
	case 'o' : glo->panneau1.offscreen_flag = 1; break;
	case 'n' : glo->panneau1.offscreen_flag = 0; break;
	// le dump, aussi utile pour faire un flush de stdout
	case 'd' :
		{
		glo->panneau1.dump();
		fflush(stdout);
		} break;
	//
	case 't' :
		glo->panneau1.qtkx *= 1.5;
		glo->panneau1.force_repaint = 1;
		glo->panneau1.force_redraw = 1;		// necessaire pour panneau1 a cause de offscreen_flag
		break;
	//
	case 'P' :
		pdf_dialogs( glo, 1 );
		break;
	}
}

/** ============================ context menus ======================= */

// call backs
static void pdf_export_0( GtkWidget *widget, glostru * glo )
{
pdf_dialogs( glo, 0 );
}
static void pdf_export_1( GtkWidget *widget, glostru * glo )
{
pdf_dialogs( glo, 1 );
}

static void offscreen_opt( GtkWidget *widget, glostru * glo )
{
glo->panneau1.offscreen_flag = gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(widget) );
glo->panneau1.dump(); fflush(stdout);
}

// enrichissement du menu global du panel
static void enrich_global_menu( glostru * glo )
{
GtkWidget * curmenu;
GtkWidget * curitem;

curmenu = glo->panneau1.gmenu;    // Don't need to show menus

curitem = gtk_separator_menu_item_new();	// separateur indispensable
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

curitem = gtk_check_menu_item_new_with_label( "offscreen drawing" );
gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(curitem), false );
g_signal_connect( G_OBJECT( curitem ), "toggled",
		  G_CALLBACK( offscreen_opt ), (gpointer)glo );
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

curitem = gtk_menu_item_new_with_label("PDF export (file chooser)");
g_signal_connect( G_OBJECT( curitem ), "activate",
		  G_CALLBACK( pdf_export_0 ), (gpointer)glo );
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

curitem = gtk_menu_item_new_with_label("PDF export (fast)");
g_signal_connect( G_OBJECT( curitem ), "activate",
		  G_CALLBACK( pdf_export_1 ), (gpointer)glo );
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

}

static void pdf_button_call( GtkWidget *widget, glostru * glo )
{
glo->panneau1.pdf_modal_layout( glo->wmain );
}

/** ============================ constr. GUI ======================= */

int main( int argc, char *argv[] )
{
glostru * glo = &theglo;
GtkWidget *curwidg;

gtk_init(&argc,&argv);
setlocale( LC_ALL, "C" );       // kill the frog, AFTER gtk_init

curwidg = gtk_window_new( GTK_WINDOW_TOPLEVEL );

gtk_signal_connect( GTK_OBJECT(curwidg), "delete_event",
                    GTK_SIGNAL_FUNC( close_event_call ), NULL );
gtk_signal_connect( GTK_OBJECT(curwidg), "destroy",
                    GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );

gtk_window_set_title( GTK_WINDOW (curwidg), "VOCO 05+");
gtk_container_set_border_width( GTK_CONTAINER( curwidg ), 10 );
glo->wmain = curwidg;
global_main_window = (GtkWindow *)curwidg;

/* creer boite verticale */
curwidg = gtk_vbox_new( FALSE, 5 ); /* spacing ENTRE objets */
gtk_container_add( GTK_CONTAINER( glo->wmain ), curwidg );
glo->vmain = curwidg;

/* creer une drawing area resizable depuis la fenetre */
curwidg = gtk_drawing_area_new();
gtk_widget_set_size_request( curwidg, 900, 500 );
glo->panneau1.events_connect( GTK_DRAWING_AREA( curwidg ) );
gtk_box_pack_start( GTK_BOX( glo->vmain ), curwidg, TRUE, TRUE, 0 );
glo->darea1 = curwidg;

/* creer une drawing area  qui ne sera pas resizee en hauteur par la hbox
   mais quand meme en largeur (par chance !!!) */
curwidg = gtk_drawing_area_new();
glo->zbar1.events_connect( GTK_DRAWING_AREA( curwidg ) );
gtk_box_pack_start( GTK_BOX( glo->vmain ), curwidg, FALSE, FALSE, 0 );
glo->sarea = curwidg;

/* creer boite horizontale */
curwidg = gtk_hbox_new( FALSE, 10 ); /* spacing ENTRE objets */
gtk_container_set_border_width( GTK_CONTAINER (curwidg), 5);
gtk_box_pack_start( GTK_BOX( glo->vmain ), curwidg, FALSE, FALSE, 0 );
glo->hbut = curwidg;

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
                    GTK_SIGNAL_FUNC( pdf_button_call ), (gpointer)glo );
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

gtk_widget_show_all( glo->wmain );

glo->panneau1.clic_callback_register( clic_call_back, (void *)glo );
glo->panneau1.key_callback_register( key_call_back, (void *)glo );

param_simu( argc, argv );
run_simu();

prep_layout1( &glo->panneau1 );
// glo->panneau1.dump();

// enrichissement du menu global du panel
enrich_global_menu( glo );

glo->idle_id = g_timeout_add( 31, (GSourceFunc)(idle_call), (gpointer)glo );
// cet id servira pour deconnecter l'idle_call : g_source_remove( glo->idle_id );

fflush(stdout);

gtk_main();

g_source_remove( glo->idle_id );

return(0);
}
