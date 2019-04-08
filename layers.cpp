using namespace std;
#include <string>
#include <iostream>
#include <vector>

#include <gtk/gtk.h>
// #include <cairo-pdf.h>
#include <stdlib.h>
#include <math.h>
#include "jluplot.h"
#include "layers.h"

// layer_dd : une courbe a pas non-uniforme en double float (classe derivee de layer_base)

// layer_dd : les methodes ajoutes a la classe de base

// allouer la memoire
void layer_dd::allocUV( size_t size )
{
U = (double *)malloc( size * sizeof(double) );
V = (double *)malloc( size * sizeof(double) );
if	( ( U != NULL ) && ( V != NULL ) )
	qu = size;
else	qu = 0;
}

// chercher le premier point X >= X0
int layer_dd::goto_U( double U0 )
{
curi = 0;
while ( curi < qu )
   {
   if ( U[curi] >= U0 )
      return 0;
   ++curi;
   }
return -1;
}

void layer_dd::goto_first()
{
curi = 0;
}

// get XY then post increment
int layer_dd::get_pi( double & rU, double & rV )
{
if ( curi >= qu )
   return -1;
rU = U[curi]; rV = V[curi]; ++curi;
return 0;
}

// chercher les Min et Max
void layer_dd::scan()
{
if	( qu )
	{
	Umin = Umax = U[0];
	Vmin = Vmax = V[0];
	}
int i;
for ( i = 1; i < qu; i++ )
    {
    if ( U[i] < Umin ) Umin = U[i];
    if ( V[i] < Vmin ) Vmin = V[i];
    if ( U[i] > Umax ) Umax = U[i];
    if ( V[i] > Vmax ) Vmax = V[i];
    }
// printf("%g < U < %g, %g < V < %g\n", Umin, Umax, Vmin, Vmax );
}

// layer_dd : les methodes qui sont virtuelles dans la classe de base
double layer_dd::get_Umin()
{ return Umin; }
double layer_dd::get_Umax()
{ return Umax; }
double layer_dd::get_Vmin()
{ return Vmin; }
double layer_dd::get_Vmax()
{ return Vmax; }

// dessin (ses dimensions dx et dy sont lues chez les parents)
void layer_dd::draw( cairo_t * cai )
{
cairo_set_source_rgb( cai, fgcolor.dR, fgcolor.dG, fgcolor.dB );
cairo_move_to( cai, 20, -(parent->ndy) + ylabel );
cairo_show_text( cai, label.c_str() );

// l'origine est en bas a gauche de la zone utile, Y+ est vers le bas (because cairo)
double tU, tV, curx, cury, maxx;
if ( goto_U( u0 ) )
   return;
if ( get_pi( tU, tV ) )
   return;
// on a le premier point ( tU, tV )
maxx = (double)(parent->parent->ndx);
curx =  u2x( tU );			// les transformations
cury = -v2y( tV );			// signe - ici pour Cairo
cairo_move_to( cai, curx, cury );
while ( get_pi( tU, tV ) == 0 )
   {
   curx =  u2x( tU );		// les transformations
   cury = -v2y( tV );
   cairo_line_to( cai, curx, cury );
   if ( curx >= maxx )
      break;
   }
cairo_stroke( cai );
}


// layer_f : une courbe a pas uniforme en float (classe derivee de layer_base)

// layer_f : les methodes ajoutes a la classe de base

// allouer la memoire
void layer_f::allocV( size_t size )
{
V = (float *)malloc( size * sizeof(float) );
if	( V != NULL )
	qu = size;
else	qu = 0;
}

// chercher le premier point X >= X0
int layer_f::goto_U( double U0 )
{
curi = (int)ceil(U0);
if	( curi < 0 )
	curi = 0;
if	( curi < qu )
	return 0;
else	return -1;
}

void layer_f::goto_first()
{
curi = 0;
}

// get XY then post increment
int layer_f::get_pi( double & rU, double & rV )
{
if ( curi >= qu )
   return -1;
rU = (double)curi; rV = (double)V[curi]; ++curi;
return 0;
}

// chercher les Min et Max
void layer_f::scan()
{
if	( qu )
	{
	Vmin = Vmax = V[0];
	}
int i;
for ( i = 1; i < qu; i++ )
    {
    if ( V[i] < Vmin ) Vmin = V[i];
    if ( V[i] > Vmax ) Vmax = V[i];
    }
// printf("%g < V < %g\n", Vmin, Vmax );
}

// layer_f : les methodes qui sont virtuelles dans la classe de base
double layer_f::get_Umin()
{ return (double)0; }
double layer_f::get_Umax()
{ return (double)(qu-1); }
double layer_f::get_Vmin()
{ return (double)Vmin; }
double layer_f::get_Vmax()
{ return (double)Vmax; }

// dessin (ses dimensions dx et dy sont lues chez les parents)
void layer_f::draw( cairo_t * cai )
{
cairo_set_source_rgb( cai, fgcolor.dR, fgcolor.dG, fgcolor.dB );
cairo_move_to( cai, 20, -(parent->ndy) + ylabel );
cairo_show_text( cai, label.c_str() );

// l'origine est en bas a gauche de la zone utile, Y+ est vers le bas (because cairo)
double tU, tV, curx, cury, maxx;
if ( goto_U( u0 ) )
   return;
if ( get_pi( tU, tV ) )
   return;
// on a le premier point ( tU, tV )
maxx = (double)(parent->parent->ndx);
curx =  u2x( tU );			// les transformations
cury = -v2y( tV );			// signe - ici pour Cairo
cairo_move_to( cai, curx, cury );
while ( get_pi( tU, tV ) == 0 )
   {
   curx =  u2x( tU );		// les transformations
   cury = -v2y( tV );
   cairo_line_to( cai, curx, cury );
   if ( curx >= maxx )
      break;
   }
cairo_stroke( cai );
}

