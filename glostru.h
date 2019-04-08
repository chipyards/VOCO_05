/* pseudo global storage ( allocated in main()  )

   JLN's GTK widget naming chart :
   w : window
   f : frame
   h : hbox
   v : vbox
   b : button
   l : label
   e : entry
   s : spin adj
   m : menu
   o : option
 */


typedef struct
{
GtkWidget * wmain;
GtkWidget * vmain;
GtkWidget *   darea1;
GtkWidget *   sarea;
GtkWidget *   hbut;
GtkWidget *     blef;
GtkWidget *     brig;
GtkWidget *     bopt;
GtkWidget *     bpdf;
GtkWidget *     bqui;

gpanel panneau1;
gzoombar zbar1;
int ilayout;

} glostru;

