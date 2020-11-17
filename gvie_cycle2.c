
#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

// taille du talbeau : hauteur 600 largeur 800
#define HM 600
#define LM 800

// nombre maximal d'itérations
#define ITER 10000

/* longueur cycle recherche de cycle (-1) */
#define LONGCYCLE 51

// utilisé pour mesurer le temps d'exécution
#define DIFFTEMPS(a,b) (((b).tv_sec - (a).tv_sec) + ((b).tv_usec - (a).tv_usec)/1000000.)

// tableau de cellules de taille (HM*LM)
typedef unsigned char tab[HM][LM];

// prédéclarations des fonctions utiles
void init( tab );
void 
nouv( tab, tab );
int egal( tab, tab );

// variable globale sinon stack overflow...
tab tt[LONGCYCLE];// tableau de tableaux

// fonction de comparaison de deux tableaux a et b,
// renvoie vrai s'ils sont égaux, faux sinon.
	/* Note: ajouter __attribute__ ((noinline)) pour faire des mesures de performance avec gprof */
int  egal( tab a, tab b )
{
	int i,j;
	for(  i=0 ; i<HM ; i++ )
		for( j=0 ; j<LM ; j++ )
			if( a[i][j] != b[i][j] )
				return( 0 );	/* si on trouve une différence on s'arrête */

	return( 1 );	/* les deux tableaux sont identiques */

}

// initialisation d'un tableau intéressant
void init( tab t )
{
	int i,j;

	for( i=0 ; i<HM ; i++ )
		for( j=0 ; j<LM ; j++ )
			t[i][j] = 0;

/*	// un marcheur en haut à gauche, qui va percuter ...
	t[10][10] = 1;
	t[10][11] = 1;
	t[10][12] = 1;
	t[9][12] = 1;
	t[8][11] = 1;

	// ... une structure qui se stabilise rapidement vers (50,50)
	t[55][50] = 1;
	t[54][51] = 1;
	t[54][52] = 1;
	t[55][53] = 1;
	t[56][50] = 1;
	t[56][51] = 1;
	t[56][52] = 1;
*/

	// un "mathusalhem" : le gland.
	t[150][100] = 1;
	t[151][100] = 1;
	t[151][102] = 1;
	t[153][101] = 1;
	t[154][100] = 1;
	t[155][100] = 1;
	t[156][100] = 1;

	// en bas : une structure avec un cycle de 3 (une croix)
	int croix[][2]= { {0,3}, {1,3}, {1,2}, {1,1} };	// un 8ième de croix
	for( i=0 ; i<4 ; i++ ) // les 4 coordonnées du tableau
		for( j=0 ; j<4; j++ ) // multipliées 4*2 fois
		{
			t[260+((j%2)?(1+croix[i][0]):(-croix[i][0]))][200+((j<2)?(1+croix[i][1]):(-croix[i][1]))] = 1;
			t[260+((j%2)?(1+croix[i][1]):(-croix[i][1]))][200+((j<2)?(1+croix[i][0]):(-croix[i][0]))] = 1;
		}
	// à droite : un octogone à cycle 5
	int octo[][2]= { {0,3}, {1,2} };	// un 8ième de croix
	for( i=0 ; i<2 ; i++ ) // les 4 coordonnées du tableau
		for( j=0 ; j<4; j++ ) // multipliées 4*2 fois
		{
			t[150+((j%2)?(1+octo[i][0]):(-octo[i][0]))][300+((j<2)?(1+octo[i][1]):(-octo[i][1]))] = 1;
			t[150+((j%2)?(1+octo[i][1]):(-octo[i][1]))][300+((j<2)?(1+octo[i][0]):(-octo[i][0]))] = 1;
		}

}

// calcule le nombre de voisins de la case (i,j) du tableau t.
int nbvois( tab t, const int i, const int j )
{
	int n=0;
	if( i>0 )
	{	/* i-1 */
		if( j>0 )
			if( t[i-1][j-1] )
				n++;
		if( t[i-1][j] )
				n++;
		if( j<LM-1 )
			if( t[i-1][j+1] )
				n++;
	}

		/* i */
	if( j>0 )
		if( t[i][j-1] )
			n++;
	if( j<LM-1 )
		if( t[i][j+1] )
			n++;

	if( i<HM-1 )
	{	/* i+1 */
		if( j>0 )
			if( t[i+1][j-1] )
				n++;
		if( t[i+1][j] )
				n++;
		if( j<LM-1 )
			if( t[i+1][j+1] )
				n++;
	}
	return( n );
}

// calcule un nouveau tableau en fonction d'un ancien
void calcnouv( tab t, tab n, rank )
{
	int i,j, v;
	for(  i=HM/size*rank ; i<HM/size*(rank+1) ; i++ )
	{
	
		for( j=0 ; j<LM ; j++ )
		{
			v = nbvois( t, i, j );
			if( v==3 )	// 3 voisins : survie ou naissance
				n[i][j] = 1;
			else if( v==2 ) // 2 voisins : survie
				n[i][j] = t[i][j];
			else // sinon : mort
				n[i][j] = 0;
		}
	}
}
int main(int *argc, char ***argv)
{
	int i, j;
	int rank;
	int size;
	int  *subtab;
  	int subtaille;
	subtaille=HM/size;
  	subtab = (int *)malloc(subtaille *sizeof(int));
	struct timeval tv_init, tv_end;
	if (MPI_Init(&argc, &argv))
	{
		fprintf(stderr,'erreur initialisation');
		exit(1);
	}
		
	init( tt[0] );
	gettimeofday( &tv_init, NULL);
	
	
	MPI_Comm_rank(MPI_COMM_WORLD,  &rank);
	MPI_Comm_size(MPI_COMM_WORLD,  &size);
	//MPI_Scatter(tt,HM*LM,MPI_INT,HM*LM,subtaille,MPI_INT,0,MPI_COMM_WORLD);
	

	for( i=0 ; i<ITER ; i++ )
	{
		/* calcul du nouveau tableau i+1 en fonction du tableau i */
		calcnouv( tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE], rank );

		/* comparaison du nouveau tableau avec les (LONGCYCLE-1) précédents */
		for( j=1 ; j<LONGCYCLE ; j++ )
			if( egal( tt[(i+1)%LONGCYCLE], tt[(i+1+j)%LONGCYCLE] ) )
			{
				// on a trouvé le tableau identique !
				gettimeofday( &tv_end, NULL);
				printf( "Cycle trouve : iteration %d, longueur %d\n",
					 i+1-(LONGCYCLE-j),
					 LONGCYCLE-j );
				printf( "Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end) );
				return( 0 );
			}
	}

	gettimeofday( &tv_end, NULL);
	printf( "pas de cycle trouve en %d itérations\n", ITER );
	printf( "Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end) );
	MPI_Finalize();
	return( 0 );
}
