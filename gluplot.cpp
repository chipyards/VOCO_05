#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
// #include <cairo-pdf.h>
#include <stdio.h>
#include <stdlib.h>
// #include <math.h>
// #include "modpop.h"

using namespace std;
#include <string>
#include <iostream>
#include <vector>

#include "jluplot.h"
#include "layers.h"
#include "gluplot.h"



/** ======================= C-style callbacks ================================ */

// capture du focus pour les fonctionnement des bindkeys
static gboolean gpanel_enter( GtkWidget * widget, GdkEventCrossing * event, gpanel * p )
{
// printf("entered!\n");
gtk_widget_grab_focus( widget );
return FALSE;	// We leave a chance to others
}

// evenement bindkey
static gboolean gpanel_key( GtkWidget * widget, GdkEventKey * event, gpanel * p )
{
p->key( event );
return TRUE;	// We've handled the event, stop processing
}

static gboolean gpanel_click( GtkWidget * widget, GdkEventButton * event, gpanel * p )
{
p->clic( event );
return TRUE;	// We've handled the event, stop processing
}

static gboolean gpanel_motion( GtkWidget * widget, GdkEventMotion * event, gpanel * p )
{
p->motion( event );
return TRUE;	// We've handled it, stop processing
}

static gboolean gpanel_wheel( GtkWidget * widget, GdkEventScroll * event, gpanel * p )
{
p->wheel( event );
return TRUE;	// We've handled it, stop processing
}

static gboolean gpanel_expose( GtkWidget * widget, GdkEventExpose * event, gpanel * p )
{
// printf("expozed\n");
p->expose();
return FALSE;	// MAIS POURQUOI ???
}

static gboolean gpanel_configure( GtkWidget * widget, GdkEventConfigure * event, gpanel * p )
{
p->configure();
return TRUE;	// MAIS POURQUOI ???
}

/* callback pour la fenetre modal du plot PDF
   il faut intercepter le delete event pour que si l'utilisateur ferme la fenetre
   on revienne de la fonction pdf_modal_layout() sans engendrer de destroy signal.
   A cet effet ce callback doit rendre TRUE (ce que la fonction gtk_main_quit() ne fait pas).
 */
static gint gpanel_pdf_modal_delete( GtkWidget *widget, GdkEvent *event, gpointer data )
{
gtk_main_quit();
return (TRUE);
}

static void gpanel_pdf_ok_button( GtkWidget *widget, gpanel * p )
{
p->pdf_ok_call();
}

static void menu1_full( GtkWidget *widget, gpanel * p )
{
printf("Full clic %08x\n", p->selected_strip );
if	( p->selected_strip & ( CLIC_MARGE_INF | CLIC_ZOOMBAR ) )
	{ p->fullM(); gtk_widget_queue_draw( p->widget ); printf("Full X\n"); }
else if ( p->selected_strip & CLIC_MARGE_GAUCHE )
	{
	p->bandes.at(p->selected_strip & (~CLIC_MARGE)).fullN();
	gtk_widget_queue_draw( p->widget );
	// printf("Full Y strip %d\n", p->selected_strip & (~CLIC_MARGE) );
	}
}

static void menu1_zoomin( GtkWidget *widget, gpanel * p )
{
if	( p->selected_strip & ( CLIC_MARGE_INF | CLIC_ZOOMBAR ) )
	{
	p->zoomXbyK( 0.5 );
	}
else if ( p->selected_strip & CLIC_MARGE_GAUCHE )
	{
	strip * b = &p->bandes.at(p->selected_strip & (~CLIC_MARGE));
	b->zoomYbyK( 0.5 );
	}
gtk_widget_queue_draw( p->widget );
}

static void menu1_zoomout( GtkWidget *widget, gpanel * p )
{
if	( p->selected_strip & ( CLIC_MARGE_INF | CLIC_ZOOMBAR ) )
	{
	p->zoomXbyK( 2.0 );
	}
else if ( p->selected_strip & CLIC_MARGE_GAUCHE )
	{
	strip * b = &p->bandes.at(p->selected_strip & (~CLIC_MARGE));
	b->zoomYbyK( 2.0 );
	}
gtk_widget_queue_draw( p->widget );
}

static gboolean gzoombar_click( GtkWidget * widget, GdkEventButton * event, gzoombar * z )
{
z->clic( event );
return TRUE;	// We've handled the event, stop processing
}

static gboolean gzoombar_motion( GtkWidget * widget, GdkEventMotion * event, gzoombar * z )
{
z->motion( event );
return TRUE;	// We've handled it, stop processing
}

static gboolean gzoombar_wheel( GtkWidget * widget, GdkEventScroll * event, gzoombar * z )
{
z->wheel( event );
return TRUE;	// We've handled it, stop processing
}

static gboolean gzoombar_expose( GtkWidget * widget, GdkEventExpose * event, gzoombar * z )
{
// printf("expozed\n");
z->expose();
return FALSE;	// MAIS POURQUOI ???
}

static gboolean gzoombar_configure( GtkWidget * widget, GdkEventConfigure * event, gzoombar * z )
{
z->configure();
return TRUE;	// MAIS POURQUOI ???
}

void gzoombar_zoom( void * z, double k0, double k1 )
{
((gzoombar *)z)->zoom( k0, k1 );
}

/** ===================== gpanel methods ===================================== */

GtkWidget * gpanel::layout( int w, int h )
{
widget = gtk_drawing_area_new ();

// ajuster la drawing area aux dimensions voulues
gtk_widget_set_size_request( widget, w, h );

/* Drawing Area Signals  */

GTK_WIDGET_SET_FLAGS( widget, GTK_CAN_FOCUS );
g_signal_connect( widget, "expose_event", G_CALLBACK(gpanel_expose), this );
g_signal_connect( widget, "configure_event", G_CALLBACK(gpanel_configure), this );
g_signal_connect( widget, "button_press_event",   G_CALLBACK(gpanel_click), this );
g_signal_connect( widget, "button_release_event", G_CALLBACK(gpanel_click), this );
g_signal_connect( widget, "motion_notify_event", G_CALLBACK(gpanel_motion), this );
g_signal_connect( widget, "scroll_event", G_CALLBACK( gpanel_wheel ), this );
g_signal_connect( widget, "key_press_event", G_CALLBACK( gpanel_key ), this );
g_signal_connect( widget, "key_release_event", G_CALLBACK( gpanel_key ), this );
g_signal_connect( widget, "enter_notify_event", G_CALLBACK( gpanel_enter ), this );

// Ask to receive events the drawing area doesn't normally subscribe to
gtk_widget_set_events ( widget, gtk_widget_get_events(widget)
			| GDK_ENTER_NOTIFY_MASK
			| GDK_KEY_PRESS_MASK
			| GDK_KEY_RELEASE_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_SCROLL_MASK
			| GDK_POINTER_MOTION_MASK
//			| GDK_POINTER_MOTION_HINT_MASK
		      );
drag.mode = nil;
selected_key = 0;

menu1_x = mkmenu1("X AXIS");
menu1_y = mkmenu1("Y AXIS");
return widget;
}

void gpanel::expose()
{
// printf("expozed\n");

cairo_t * cair = gdk_cairo_create(widget->window);

// fill the background
cairo_set_source_rgb( cair, 1, 1, 1 );
cairo_paint(cair);	// paint the complete clip area

// preparer font a l'avance
cairo_select_font_face( cair, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
cairo_set_font_size( cair, 12.0 );
cairo_set_line_width( cair, 0.5 );
// draw the curves
draw(cair);

// possibly draw the selection phantom rectangle
switch	( drag.mode )
	{
	case nil : break;
	case select_zone :
		cairo_set_source_rgba( cair, 0.0, 0.8, 1.0, 0.3 );
		drag.zone( cair );
		break;
	case zoom :
		cairo_set_source_rgba( cair, 0.0, 0.0, 0.0, 1.0 );
		drag.box(cair);
		break;
	case pan :
		cairo_set_source_rgba( cair, 0.0, 0.0, 0.0, 1.0 );
		drag.line(cair);
		break;
	}

cairo_destroy(cair);
}

void gpanel::configure()
{
int ww, wh;
gdk_drawable_get_size( widget->window, &ww, &wh );
printf("configuzed %d x %d\n", ww, wh );
resize( ww, wh );
}

void gpanel::key( GdkEventKey * event )
{
if	( event->type == GDK_KEY_PRESS )
	{
	if	( selected_key == 0 )	// pour filtrer les repetitions automatiques
		{
		int v = event->keyval;
		if	( ( v < GDK_Shift_L ) || ( v > GDK_Alt_R ) )	// see gdk/gdkkeysyms.h
			{	// fragile detection des modifiers ( event->is_modifier est bogus )
			selected_key = v;
			printf("Key h=%04x v=%04x (%04x) \"%s\" press\n",
				event->hardware_keycode, event->keyval, event->state, event->string );
			if	( selected_key == 'f' )
				{ fullMN(); gtk_widget_queue_draw( widget ); }
			if	( ( selected_key >= GDK_F1 ) && ( selected_key <= GDK_F12 ) )
				printf("F%d\n", selected_key + 1 - GDK_F1 );
			}
		}
	}
else if ( event->type == GDK_KEY_RELEASE )
	{
 	printf("Key releas v=%04x\n", selected_key );
	selected_key = 0;
	}
}

void gpanel::clic( GdkEventButton * event )
{
double x, y;
x = event->x; y = event->y;
if	( event->type == GDK_BUTTON_PRESS )
	{
	drag.x0 = x;
	drag.y0 = y;
	drag.x1 = x;
	drag.y1 = y;
	if	( event->button == 1 )
		{
		if	( selected_key == ' ' )
			drag.mode = pan;
		else	drag.mode = select_zone;
		}
	else if	( event->button == 3 )
		{
		drag.mode = zoom;
		}
	}
else if	( event->type == GDK_BUTTON_RELEASE )
	{
	// ici on insere traitement de la fenetre draguee : action clic ou action select_zone ou action zoom
	if	( ( drag.x0 == drag.x1 ) && ( drag.y0 == drag.y1 ) )
		{	// action clic sans drag
		int istrip; double Q, R;
		istrip = clicQR( x, y, &Q, &R );
		if	( event->button == 1 )
			{
			char utbuf[32];	// buffer pour scientout
			char vtbuf[32];	// buffer pour scientout
			if	( istrip < 0 )
				printf("clic hors graphique (%d)\n", istrip );
			else	{
				if	( ( istrip & CLIC_MARGE ) == 0 )
					{	// clic dans une courbe
					// ce coeff 0.002 suggere une resolution 500 fois plus fine que le tick, Ok ?
					scientout( utbuf, Q, 0.002 * tdq );
					scientout( vtbuf, R, 0.002 * bandes[istrip].tdr );
					printf("clic strip %d [%s:%s]\n", istrip, utbuf, vtbuf );
					}
				else	{
					if	( istrip & CLIC_MARGE_GAUCHE )
						printf("clic marge gauche strip %d\n", istrip & ~CLIC_MARGE );
					else if	( istrip & CLIC_MARGE_INF )
						{
						scientout( utbuf, Q, 0.002 * tdq );
						printf("clic marge inf Q = %s\n", utbuf );
						}
					}
				}
			}
		else if	( event->button == 3 )
			{
			if	( istrip >= 0 )
				{
				selected_strip = istrip;
				if	( istrip & CLIC_MARGE_INF )
					gtk_menu_popup( (GtkMenu *)menu1_x, NULL, NULL, NULL, NULL,
							event->button, event->time );
				else if	( istrip & CLIC_MARGE_GAUCHE )
					{
					gtk_menu_popup( (GtkMenu *)menu1_y, NULL, NULL, NULL, NULL,
							event->button, event->time );
					}
				}
			}
		drag.mode = nil;
		}
	else	{	// action drag
		int istrip0, istrip1; double X0, X1, Y0, Y1;
		istrip0 = clicXY( drag.x0, drag.y0, &X0, &Y0 );
		istrip1 = clicXY( drag.x1, drag.y1, &X1, &Y1 );
		if	( ( istrip0 >= 0 ) && ( istrip1 >= 0 ) )
			{
			switch	( drag.mode )
				{
				case nil : break;
				case zoom :
					if	( !( istrip0 & CLIC_MARGE ) )
						{
						istrip0 &= (~CLIC_MARGE);
						istrip1 &= (~CLIC_MARGE);
						// zoom X (toujours)
						if	( X1 >= X0 )
							zoomX( X0, X1 );
						else	zoomX( X1, X0 );
						if	( istrip0 == istrip1 )
							{	// zoom Y sur 1 strip
							if	( Y1 >= Y0 )
								bandes.at(istrip0).zoomY( Y0, Y1 );
							else	bandes.at(istrip0).zoomY( Y1, Y0 );
							}
						}
					break;
				case select_zone :
					printf("selected from %d<%g:%g> to %d<%g:%g>\n",
						istrip0, X0, Y0, istrip1, X1, Y1 );
					break;
				case pan :
					if	( istrip0 == istrip1 )
						{
						istrip0 &= (~CLIC_MARGE);
						bandes.at(istrip0).panY( Y0 - Y1 );
						panX( X0 - X1 );
						}
					break;
				}
			}
		drag.mode = nil;
		gtk_widget_queue_draw( widget );
		}
	}
}

void gpanel::motion( GdkEventMotion * event )
{
GdkModifierType state = (GdkModifierType)event->state;

if	( ( state & GDK_BUTTON1_MASK ) || ( state & GDK_BUTTON3_MASK ) )
	{
	drag.x1 = event->x;
	drag.y1 = event->y;
	gtk_widget_queue_draw( widget );
	}
}

void gpanel::wheel( GdkEventScroll * event )
{
int istrip; double X, Y;
istrip = clicXY( event->x, event->y, &X, &Y );
if	( istrip & CLIC_MARGE_INF )
	{
	if	( event->direction == GDK_SCROLL_DOWN )
		{
		panXbyK( -0.02 );
		}
	if	( event->direction == GDK_SCROLL_UP )
		{
		panXbyK( 0.02 );
		}
	gtk_widget_queue_draw( widget );
	}
else if	( istrip & CLIC_MARGE_GAUCHE )
	{
	strip * b = &bandes.at(istrip & (~CLIC_MARGE));
	if	( event->direction == GDK_SCROLL_DOWN )
		{
		b->zoomYbyK( 0.9 );
		}
	if	( event->direction == GDK_SCROLL_UP )
		{
		b->zoomYbyK( 1.11 );
		}
	gtk_widget_queue_draw( widget );
	}
else	{
	strip * b = &bandes.at(istrip & (~CLIC_MARGE));
	if	( event->direction == GDK_SCROLL_DOWN )
		{
		b->panYbyK( -0.02 );
		}
	if	( event->direction == GDK_SCROLL_UP )
		{
		b->panYbyK( 0.02 );
		}
	gtk_widget_queue_draw( widget );
	}
}

/** ===================== pdf service methods =================================== */

// fonction bloquante
void gpanel::pdf_modal_layout( GtkWidget * mainwindow )
{
if ( bandes.size() == 0 )
   return;

// petit dialogue pour ce pdf
GtkWidget * curwin;
GtkWidget * curbox;
GtkWidget * curbox2;
GtkWidget * curwidg;

curwin = gtk_window_new( GTK_WINDOW_TOPLEVEL );/* DIALOG est deprecated, POPUP est autre chose */
/* ATTENTION c'est serieux : modal veut dire que la fenetre devient la
   seule a capturer les evenements ( i.e. les autres sont bloquees ) */
gtk_window_set_modal( GTK_WINDOW(curwin), TRUE );
gtk_window_set_position( GTK_WINDOW(curwin), GTK_WIN_POS_MOUSE );
gtk_window_set_transient_for(  GTK_WINDOW(curwin), GTK_WINDOW(mainwindow) );
gtk_window_set_type_hint( GTK_WINDOW(curwin), GDK_WINDOW_TYPE_HINT_DIALOG );

gtk_window_set_title( GTK_WINDOW(curwin), "sortie fichier PDF" );
gtk_container_set_border_width( GTK_CONTAINER(curwin), 20 );

gtk_signal_connect( GTK_OBJECT(curwin), "delete_event",
                    GTK_SIGNAL_FUNC( gpanel_pdf_modal_delete ), NULL );
wchoo = curwin;

// boite verticale
curbox = gtk_vbox_new( FALSE, 20 );
gtk_container_add( GTK_CONTAINER( curwin ), curbox );

// label
curwidg = gtk_label_new( "Description :" );
gtk_box_pack_start( GTK_BOX( curbox ), curwidg, FALSE, FALSE, 0);

// entree editable
curwidg = gtk_entry_new_with_max_length(64);
gtk_widget_set_usize( curwidg, 600, 0 );
gtk_entry_set_editable( GTK_ENTRY(curwidg), TRUE );
gtk_entry_set_text( GTK_ENTRY(curwidg), "Bugiganga" );
gtk_box_pack_start( GTK_BOX( curbox ), curwidg, FALSE, FALSE, 0 );
edesc = curwidg;

// label
curwidg = gtk_label_new( "Nom de fichier :" );
gtk_box_pack_start( GTK_BOX( curbox ), curwidg, FALSE, FALSE, 0);


curwidg = gtk_file_chooser_widget_new( GTK_FILE_CHOOSER_ACTION_SAVE );
gtk_box_pack_start( GTK_BOX( curbox ), curwidg, TRUE, TRUE, 0 );
gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(curwidg), "C:\\tmp");
gtk_file_chooser_set_current_name( GTK_FILE_CHOOSER(curwidg), "pipu.pdf" );
fchoo = curwidg;

// boite horizontale
curbox2 = gtk_hbox_new( FALSE, 5 );
gtk_box_pack_start( GTK_BOX( curbox ), curbox2, FALSE, FALSE, 0);

/* le bouton ok */
curwidg = gtk_button_new_with_label (" Ok ");
gtk_box_pack_end( GTK_BOX( curbox2 ), curwidg, FALSE, FALSE, 0);
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( gpanel_pdf_ok_button ), (gpointer)this );

/* le bouton abort */
curwidg = gtk_button_new_with_label (" Annuler ");
gtk_box_pack_end( GTK_BOX( curbox2 ), curwidg, FALSE, FALSE, 0);
gtk_signal_connect( GTK_OBJECT(curwidg), "clicked",
                    GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );

gtk_widget_show_all( curwin );

/* on est venu ici alors qu'on est deja dans 1 boucle gtk_main
   alors donc on en imbrique une autre. Le prochain appel a
   gtk_main_quit() fera sortir de cell-ci (innermost)
 */
gtk_main();
gtk_widget_destroy( curwin );
}


void gpanel::pdf_ok_call()
{
char * fnam = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER(fchoo) );
if ( fnam )
   {
   unsigned int iban;
   for	( iban = 0; iban < bandes.size(); iban++ )
	bandes[iban].optcadre = 1; // ici demo fond blanc + cadre en couleur
   int retval = pdfplot( fnam, gtk_entry_get_text( GTK_ENTRY(edesc) ) );
   printf("retour panel::pdfplot %s : %d\n", fnam, retval );
   for	( iban = 0; iban < bandes.size(); iban++ )
	bandes[iban].optcadre = 0; // ici demo
   }
// remettre aux dimensions de la drawing area
int ww, wh;
gdk_drawable_get_size( widget->window, &ww, &wh );
resize( ww, wh );
gtk_main_quit();
}

/** ===================== context menus =================================== */

GtkWidget * gpanel::mkmenu1( const char * title )
{
GtkWidget * curmenu;
GtkWidget * curitem;

curmenu = gtk_menu_new ();    // Don't need to show menus

curitem = gtk_menu_item_new_with_label(title);
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

curitem = gtk_separator_menu_item_new();
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

curitem = gtk_menu_item_new_with_label("Full");
g_signal_connect( G_OBJECT( curitem ), "activate",
		  G_CALLBACK( menu1_full ), (gpointer)this );
gtk_menu_shell_append( GTK_MENU_SHELL( curmenu ), curitem );
gtk_widget_show ( curitem );

curitem = gtk_menu_item_new_with_label("Zoom in");
g_signal_connect( G_OBJECT( curitem ), "activate",
		  G_CALLBACK( menu1_zoomin ), (gpointer)this );
gtk_menu_shell_append( GTK_MENU_SHELL(curmenu), curitem );
gtk_widget_show ( curitem );

curitem = gtk_menu_item_new_with_label("Zoom out");
g_signal_connect( G_OBJECT( curitem ), "activate",
		  G_CALLBACK( menu1_zoomout ), (gpointer)this );
gtk_menu_shell_append( GTK_MENU_SHELL(curmenu), curitem );
gtk_widget_show ( curitem );

return curmenu;
}
/** ===================== gzoomdrag methods ===================================== */


void gzoomdrag::zone( cairo_t * cair )
{
cairo_rectangle( cair, x0, y0, x1 - x0, y1 - y0 );
cairo_fill(cair);
}

void gzoomdrag::box( cairo_t * cair )
{
cairo_rectangle( cair, x0, y0, x1 - x0, y1 - y0 );
cairo_stroke(cair);
}

void gzoomdrag::line( cairo_t * cair )
{
cairo_move_to( cair, x0, y0 );
cairo_line_to( cair, x1, y1 );
cairo_stroke(cair);
}

/** ===================== gzoombar methods ====================================== */

GtkWidget * gzoombar::layout( int w )
{
widget = gtk_drawing_area_new ();

// ajuster la drawing area aux dimensions voulues
gtk_widget_set_size_request( widget, w, 32 );

/* Drawing Area Signals  */

g_signal_connect( widget, "expose_event", G_CALLBACK(gzoombar_expose), this );
g_signal_connect( widget, "configure_event", G_CALLBACK(gzoombar_configure), this );
g_signal_connect( widget, "button_press_event",   G_CALLBACK(gzoombar_click), this );
g_signal_connect( widget, "button_release_event", G_CALLBACK(gzoombar_click), this );
g_signal_connect( widget, "motion_notify_event", G_CALLBACK(gzoombar_motion), this );
g_signal_connect( widget, "scroll_event", G_CALLBACK( gzoombar_wheel ), this );

// Ask to receive events the drawing area doesn't normally subscribe to
gtk_widget_set_events ( widget, gtk_widget_get_events(widget)
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_SCROLL_MASK
			| GDK_POINTER_MOTION_MASK
//			| GDK_POINTER_MOTION_HINT_MASK
		      );
return widget;
}

void gzoombar::expose()
{
// printf("expozed\n");

cairo_t * cair = gdk_cairo_create(widget->window);

// fill the background
cairo_set_source_rgb( cair, 0.88, 0.88, 1 );
cairo_paint(cair);	// paint the complete clip area

// preparer font a l'avance
// cairo_select_font_face( cair, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
// cairo_set_font_size( cair, 12.0 );
// cairo_set_line_width( cair, 0.5 );

// draw the bar
cairo_set_source_rgb( cair, 0.66, 0.66, 1 );
cairo_rectangle( cair, x0, 4.0, x1 - x0, 24.0 );
cairo_fill(cair);
if	( opcode )
	{
	cairo_set_source_rgb( cair, 0.0, 0.0, 0.0 );
	cairo_rectangle( cair, x0f, 4.0, x1f - x0f, 24.0 );
	cairo_stroke(cair);
	}
cairo_destroy(cair);
}

void gzoombar::configure()
{
int wh;
gdk_drawable_get_size( widget->window, &ww, &wh );
ndx = (double)ww - ( 2.0 * xm );
if	( panneau )
	{
	double fullspan = panneau->fullmmax - panneau->fullmmin;
	double k0 = ( panneau->q2m(panneau->q0) - panneau->fullmmin ) / fullspan;
	double k1 = ( panneau->q2m(panneau->q1) - panneau->fullmmin ) / fullspan;
	zoom( k0, k1 );
	}
printf("configuzed %d x %d\n", ww, wh );
}

void gzoombar::clic( GdkEventButton * event )
{
double x = event->x;
// double y = event->y;
if	( event->type == GDK_BUTTON_PRESS )
	{
	if	( event->button == 1 )
		{
		xc = x;
		x0f = x0; x1f = x1;
		opcode = op_select( x );
		printf("op %c\n", (opcode < ' ')?(opcode+'0'):(opcode) );
		}
	else if	( event->button == 3 )
		{
		xc = x;
		x0f = x0; x1f = x1;
		opcode = 'Z';
		}
	}
else if	( event->type == GDK_BUTTON_RELEASE )
	{
	if	( x == xc )
		{		// action clic sans drag
		if	( panneau )
			{
			if	( event->button == 1 )
				{
				double m, kx2m;
				kx2m = (panneau->fullmmax - panneau->fullmmin)/ndx;
				m = kx2m * (x-xm);
				printf("zoombar clic M=%g\n", m );
				}
			else if ( event->button == 3 )
				{
				panneau->selected_strip = CLIC_ZOOMBAR;
				gtk_menu_popup( (GtkMenu *)panneau->menu1_x, NULL, NULL, NULL, NULL,
							event->button, event->time );
				}
			}
		opcode = 0;
		}
	else	{		// action drag
		x0 = x0f; x1 = x1f;
		opcode = 0;
		gtk_widget_queue_draw( widget );
		if	( panneau )
			{
			double mmin, mmax, kx2m;
			kx2m = (panneau->fullmmax - panneau->fullmmin)/ndx;
			mmin = kx2m * (x0-xm);
			mmin += panneau->fullmmin;
			mmax = kx2m * (x1-xm);
			mmax += panneau->fullmmin;
			panneau->zoomM( mmin, mmax );
			gtk_widget_queue_draw( panneau->widget );
			}
		}
	}
}

void gzoombar::motion( GdkEventMotion * event )
{
GdkModifierType state = (GdkModifierType)event->state;

if	( ( state & GDK_BUTTON1_MASK ) || ( state & GDK_BUTTON3_MASK ) )
	{
	// printf("drag %6g:%6g\n", event->x, event->y );
	update( event->x );
	gtk_widget_queue_draw( widget );
	}
}

void gzoombar::wheel( GdkEventScroll * event )
{
if	( panneau )
	{
	if	( event->direction == GDK_SCROLL_DOWN )
		{
		panneau->zoomXbyK( 0.9 );
		}
	if	( event->direction == GDK_SCROLL_UP )
		{
		panneau->zoomXbyK( 1.1 );
		}
	gtk_widget_queue_draw( panneau->widget );
	}
}

// choisir l'operation en fonction du lieu du clic
int gzoombar::op_select( double x )
{
if	( ( x < ( x0 - xm ) ) || ( x > ( x1 + xm ) ) )
	return 0;
if	( ( x1 - x0 ) > xlong )
	{			// barre "longue"
	if	( x < ( x0 + xm ) ) return 'L';
	if	( x > ( x1 - xm ) ) return 'R';
	return 'M';
	}
else	{			// barre "courte"
	if	( x < x0 ) return 'L';
	if	( x > x1 ) return 'R';
	return 'M';
	}
}

// calculer la nouvelle position de la barre (fantome)
void gzoombar::update( double x )
{
switch	( opcode )
	{
	case 'L' :	// mettre a jour x0f
		x0f = x;
		if	( x0f > ( x1f - dxmin ) )	// butee droite
			x0f = ( x1f - dxmin );
		if	( x0f < xm )			// butee gauche
			x0f = xm;
		break;
	case 'M' :	// translater x0f et x1f
		x0f = x0 + ( x - xc );
		x1f = x1 + ( x - xc );
		if	( x0f < xm )			// butee gauche
			{ x0f = xm; x1f = x0f + ( x1 - x0 );  }
		if	( x1f > ( ndx + xm ) )		// butee droite
			{ x1f = ndx + xm; x0f = x1f - ( x1 - x0 ); }
		break;
	case 'R' :	// mettre a jour x1f
			x1f = x;
		if	( x1f < ( x0f + dxmin ) )	// butee gauche
			x1f = ( x0f + dxmin );
		if	( x1f > ( ndx + xm ) )		// butee droite
			x1f = ndx + xm;
		break;
	case 'Z' :
		if	( x < xm )			// butee gauche
			x = xm;
		if	( x > ( ndx + xm ) )		// butee droite
			x = ndx + xm;
		if	( x > xc )
			{ x0f = xc; x1f = x; }
		else	{ x1f = xc; x0f = x; }
		if	( ( x1f - x0f ) < dxmin )
			x1f = x0f + dxmin;
		break;
	}
}

// mettre a jour la barre en fonction d'un cadrage normalise venant d'un autre widget
void gzoombar::zoom( double k0, double k1 )
{
printf("gzoombar::zoom( %g, %g )\n", k0, k1 );
x0 = xm + ndx * k0;
x1 = xm + ndx * k1;
gtk_widget_queue_draw( widget );
}
