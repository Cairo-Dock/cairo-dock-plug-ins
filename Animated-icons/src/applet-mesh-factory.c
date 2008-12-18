/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-mesh-factory.h"

#define RADIAN (G_PI / 180.0)  // Conversion Radian/Degres

GLuint cairo_dock_load_capsule_calllist (void)
{
	GLuint iCallList = glGenLists (1);
	int        deg, deg2, iter, nb_iter=20;
	float        amp, rayon, c=2.;
	
	rayon        = 1.0f/c;
	amp        = 90.0 / nb_iter;
	deg2        = 0;
	
	glNewList(iCallList, GL_COMPILE); // Go pour la compilation de la display list
	glPolygonMode (GL_FRONT, GL_FILL);
	
	/**glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR ); // ok la on selectionne le type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGenfv(GL_S, GL_OBJECT_PLANE, fCapsuleObjectPlaneS); // On place la texture correctement en X
	glTexGenfv(GL_T, GL_OBJECT_PLANE, fCapsuleObjectPlaneT); // Et en Y
	glEnable(GL_TEXTURE_GEN_S);                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);                // Et en T aussi
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
	
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPushMatrix ();
	glLoadIdentity(); // On la reset
	glTranslatef(0.5f, 0.5f, 0.); // Et on decale la texture pour un affiche propre
	glRotatef (180, 1, 0, 0);  // sinon les icones sont a l'envers.
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	// bon la je commente pas on fait juste une demi sphere applatie
	double a = .5/c;  // applatissement;
	double b = 1./nb_iter;
	double xab, yab, zab, xac, yac, zac, nx, ny, nz, n;
	
	glBegin(GL_QUADS);
	for (iter = 0;iter < nb_iter-1;iter ++)
	{
		for (deg = 0;deg < 360;deg += 10)
		{
			xab = b * cos(deg*RADIAN);
			yab = b * sin(deg*RADIAN);
			zab = a * sin(deg2*RADIAN) - a * sin((deg2+amp)*RADIAN);
			//zab = a*cos (deg2*RADIAN) * amp*RADIAN;
			xac = rayon * cos((deg+10)*RADIAN) - (rayon-b) * cos(deg*RADIAN);
			yac = rayon * sin((deg+10)*RADIAN) - (rayon-b) * sin(deg*RADIAN);
			zac = a * sin(deg2*RADIAN) - a * sin((deg2+amp)*RADIAN);
			//zac = a * sin((deg2+amp)*RADIAN) - a * sin(deg2*RADIAN);
			nx = yab*zac - zab*yac;
			ny = zab*xac - xab*zac;
			nz = xab*yac - yab*xac;
			n = sqrt (nx*nx + ny*ny + nz*nz);
			
			glNormal3f (nx/n, ny/n, nz/n);
			
			glVertex3f((rayon-b) * cos(deg*RADIAN),
				(rayon-b) * sin(deg*RADIAN),
				a * sin((deg2+amp)*RADIAN) + 0.1f/c);
			glVertex3f(rayon * cos(deg*RADIAN),
				rayon * sin(deg*RADIAN),
				a * sin(deg2*RADIAN) + 0.1f/c);
			glVertex3f(rayon * cos((deg+10)*RADIAN),
				rayon * sin((deg+10)*RADIAN),
				a * sin(deg2*RADIAN) + 0.1f/c);
			glVertex3f((rayon-b) * cos((deg+10)*RADIAN),
				(rayon-b) * sin((deg+10)*RADIAN),
				a * sin((deg2+amp)*RADIAN) + 0.1f/c);
			
			//nx = - nx;
			//ny = - ny;
			nz = - nz;
			
			glNormal3f (nx/n, ny/n, nz/n);
			glVertex3f((rayon-b) * cos(deg*RADIAN),
				 (rayon-b) * sin(deg*RADIAN),
				-a * sin((deg2+amp)*RADIAN) - 0.1f/c);
			glVertex3f(rayon * cos(deg*RADIAN),
				 rayon * sin(deg*RADIAN),
				-a * sin(deg2*RADIAN) - 0.1f/c);
			glVertex3f(rayon * cos((deg+10)*RADIAN),    
				 rayon * sin((deg+10)*RADIAN),
				-a * sin(deg2*RADIAN) - 0.1f/c);
			glVertex3f((rayon-b) * cos((deg+10)*RADIAN),
				 (rayon-b) * sin((deg+10)*RADIAN),
				-a * sin((deg2+amp)*RADIAN) - 0.1f/c);
		}
		rayon    -= b/c;
		deg2    += amp;
	}
	glEnd();
	
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	/*// Ici c'est pour faire le cylindre qui relie les demi spheres
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Couleur a fond 
	GLfloat fMaterial[4] = {1., 1., 1., 1.};
	//glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, fMaterial);  // on definit Les proprietes materielles de l'objet.
	g_print ("iChromeTexture : %d\n", iChromeTexture);
	glBindTexture(GL_TEXTURE_2D, iChromeTexture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Ici c'est pour le type de combinaison de texturing en cas de multi
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE); // pas de multi je remplace donc l'ancienne texture par celle ci
	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // ok la on selectionne le type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // Ce sera du sphere mapping pour un petit effet chrome
	glEnable(GL_TEXTURE_GEN_S); // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T); // Et en T aussi
	
	
	rayon = 1.0f/c;
	glBegin(GL_QUADS);
	//for (iter = 0;iter < 5;iter ++)
	{
		for (deg = 0;deg < 360;deg += 10)
		{
			xab = rayon * sin((deg+10)*RADIAN) - rayon * sin(deg*RADIAN);
			yab = rayon * cos((deg+10)*RADIAN) - rayon * cos(deg*RADIAN);
			zab = 0.;
			xac = xab;
			yac = yab;
			zac = -0.2/c;
			
			nx = yab*zac - zab*yac;
			ny = zab*xac - xab*zac;
			nz = xab*yac - yab*xac;
			n = sqrt (nx*nx + ny*ny + nz*nz);
			
			glNormal3f (nx/n, ny/n, 
			nz/n);
			
			glVertex3f(rayon * sin(deg*RADIAN), rayon * cos(deg*RADIAN), 0.1f/c);
			glVertex3f(rayon * sin((deg+10)*RADIAN), rayon * cos((deg+10)*RADIAN), 0.1f/c);
			glVertex3f(rayon * sin((deg+10)*RADIAN), rayon * cos((deg+10)*RADIAN), -0.1f/c);
			glVertex3f(rayon * sin(deg*RADIAN), rayon * cos(deg*RADIAN), -0.1f/c);
			
		}
	
		rayon -= 0.2f/c;
		deg2 += amp;
	}
	glEnd();*/

	glEndList(); // Fini la display list
	
	/**glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_2D); // Plus de texture merci 
	glDisable(GL_TEXTURE);*/
	
	return iCallList;
}

GLuint cairo_dock_load_ring_calllist (void)
{
	GLuint iCallList = glGenLists (1);
	int        deg, deg2, iter, nb_iter=20;
	float        amp, rayon, c=2.;
	
	rayon        = 1.0f/c;
	amp        = 90.0 / nb_iter;
	deg2        = 0;
	
	glNewList(iCallList, GL_COMPILE); // Go pour la compilation de la display list
	glPolygonMode (GL_FRONT, GL_FILL);
	double a = .4/c;  // applatissement;
	double b = 1./nb_iter;
	double xab, yab, zab, xac, yac, zac, nx, ny, nz, n;
	
	rayon = 1.0f/c;
	glBegin(GL_QUADS);
	//for (iter = 0;iter < 5;iter ++)
	{
		for (deg = 0;deg < 360;deg += 10)
		{
			xab = rayon * sin((deg+10)*RADIAN) - rayon * sin(deg*RADIAN);
			yab = rayon * cos((deg+10)*RADIAN) - rayon * cos(deg*RADIAN);
			zab = 0.;
			xac = xab;
			yac = yab;
			zac = -0.2/c;
			
			nx = yab*zac - zab*yac;
			ny = zab*xac - xab*zac;
			nz = xab*yac - yab*xac;
			n = sqrt (nx*nx + ny*ny + nz*nz);
			
			glNormal3f (nx/n, ny/n, nz/n);
			
			glVertex3f(rayon * sin(deg*RADIAN), rayon * cos(deg*RADIAN), 0.1f/c);
			glVertex3f(rayon * sin((deg+10)*RADIAN), rayon * cos((deg+10)*RADIAN), 0.1f/c);
			glVertex3f(rayon * sin((deg+10)*RADIAN), rayon * cos((deg+10)*RADIAN), -0.1f/c);
			glVertex3f(rayon * sin(deg*RADIAN), rayon * cos(deg*RADIAN), -0.1f/c);
			
		}
	
		rayon -= 0.2f/c;
		deg2 += amp;
	}
	glEnd();
	
	glEndList(); // Fini la display list
	return iCallList;
}


GLuint cairo_dock_load_square_calllist (void)
{
	GLuint iCallList = glGenLists (1);
	glNewList(iCallList, GL_COMPILE); // Go pour la compilation de la display list
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	
	glNormal3f(0,0,1);
	glBegin(GL_QUADS);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.); glVertex3f(-.5,  .5, 0.);  // Bottom Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.); glVertex3f( .5,  .5, 0.);  // Bottom Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.); glVertex3f( .5, -.5, 0.);  // Top Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.); glVertex3f(-.5, -.5, 0.);  // Top Left Of The Texture and Quad
	glEnd();
	
	glEndList(); // Fini la display list
	return iCallList;
}

GLuint cairo_dock_load_cube_calllist (void)
{
	GLuint iCallList = glGenLists (1);
	glNewList(iCallList, GL_COMPILE); // Go pour la compilation de la display list
	glPolygonMode (GL_FRONT, GL_FILL);
	
	double a = .5 / sqrt (2);
	glBegin(GL_QUADS);
	// Front Face (note that the texture's corners have to match the quad's corners)
	glNormal3f(0,0,1);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.); glVertex3f(-a,  a,  a);  // Bottom Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.); glVertex3f( a,  a,  a);  // Bottom Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.); glVertex3f( a, -a,  a);  // Top Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.); glVertex3f(-a, -a,  a);  // Top Left Of The Texture and Quad
	// Back Face
	glNormal3f(0,0,-1);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.); glVertex3f( -a, a, -a);  // Bottom Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.); glVertex3f( -a, -a, -a);  // Top Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.); glVertex3f(a, -a, -a);  // Top Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.); glVertex3f(a, a, -a);  // Bottom Left Of The Texture and Quad
	// Top Face
	glNormal3f(0,1,0);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.); glVertex3f(-a,  a,  a);  // Top Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.); glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.); glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.); glVertex3f( a,  a,  a);  // Top Right Of The Texture and Quad
	// Bottom Face
	glNormal3f(0,-1,0);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.); glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.); glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.); glVertex3f(-a, -a,  a);  // Bottom Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.); glVertex3f( a, -a,  a);  // Bottom Right Of The Texture and Quad
	// Right face
	glNormal3f(1,0,0);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.);  glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.);  glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.);  glVertex3f( a, -a,  a);  // Top Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.);  glVertex3f( a,  a,  a);  // Bottom Left Of The Texture and Quad
	// Left Face
	glNormal3f(-1,0,0);
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 0.);  glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 0.);  glVertex3f(-a,  a,  a);  // Bottom Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,1., 1.);  glVertex3f(-a, -a,  a);  // Top Right Of The Texture and Quad
	glMultiTexCoord2f( GL_TEXTURE1_ARB,0., 1.);  glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
	glEnd();
	
	glEndList(); // Fini la display list
	return iCallList;
}


GLuint cd_animations_load_mesh (CDAnimationsMeshType iMeshType)
{
	GLuint iCallList = 0;
	switch (iMeshType)
	{
		case CD_SQUARE_MESH :
			iCallList = cairo_dock_load_square_calllist ();
		break ;
		
		case CD_CUBE_MESH :
			iCallList = cairo_dock_load_cube_calllist ();
		break ;
		
		case CD_CAPSULE_MESH :
			iCallList = cairo_dock_load_capsule_calllist ();
		break ;
	}
	return iCallList;
}
