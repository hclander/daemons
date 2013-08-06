/*
 * txs_prj.c
 *
 *  Created on: 06/08/2013
 *      Author: jcmendez
 */

#include <stdlib.h>
#include <stdio.h>
#include <proj_api.h>
#include <project.h>

int main(int argc, char **argv) {

	projPJ pj_merc, pj_latlon;

	PROJ *p;
	PROJ_XY xy;
	PROJ_LP lp;

	double lat,lon;

	pj_merc   =  pj_init_plus("+proj=merc +ellps=clrk66 +lat_ts=33");
	pj_latlon =  pj_init_plus("+proj=latlong +ellps=clrk66");

	lat = 42.22;
	lon = -8.65;

	lat *=DEG_TO_RAD;
	lon *=DEG_TO_RAD;

	pj_transform(pj_latlon,pj_merc,1,1,&lon,&lat,NULL);

	printf("Proj4   -> x=%lf,y=%lf\n",lat,lon);

	lp.phi=-8.65 * 180 / M_PI;
	lp.lam=42.22 * 180 / M_PI;

	p = proj_initstr("proj=merc ellps=clrk66 lat_ts=33");

	xy = proj_fwd(lp,p);
	printf("Project -> x=%lf,y=%lf\n",lat,lon);

	return EXIT_SUCCESS;
}

