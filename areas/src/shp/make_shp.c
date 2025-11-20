#include <stdio.h>
#include <stdlib.h>

#define SHOP_LIST	"AREA"
#define SHP_DIR		"shp"

#define ALL_SHP		"tworld.shp"

void punt(char *msg)
{
    fprintf(stderr, "%s\n");
    exit(-1);
}

int main()
{
    FILE *shop_list, *all_shp, *tmp_shp;
    char shop[8192], buf[8192], shop_name[80], shp_name[80];
    int shop_count, shp_count;

    /*
     *	Open all the stupid files up
     */
    shop_list = fopen(SHOP_LIST, "r");
    if (shop_list==NULL)
	punt("SHOP file cannot be opened");

    all_shp = fopen(ALL_SHP, "w");
    if (all_shp==NULL)
	punt("world.shp cannot be opened");

    shop_count = 0;
    shp_count = 0;
    for (;;) {
	fgets(shop, 8191, shop_list);
	if (shop==NULL)
	    break;
	if (feof(shop_list))
	    break;
	if (shop[0]=='*')		/* a comment */
	    continue;
	sscanf(shop, "%s", shop_name);
	fprintf(stdout, "Compiling shop file %2d : %s\n", shop_count++, shop_name);
	/*
	 * open up individual zon, wld, obj, mob files for each area
         */

	sprintf(shp_name, "%s/%s.shp", SHP_DIR, shop_name);
	tmp_shp = fopen(shp_name, "r");
	if (tmp_shp==NULL) {
#if 0
	    fprintf(stdout, "\twarning: %s not found\n", shp_name);
#endif
	}
	else {
	    for (;;) {
		fgets(buf, 8191, tmp_shp);
		if (buf==NULL)
		    break;
		if (feof(tmp_shp))
		    break;
		if (*buf=='#')
		    shp_count++;
		fputs(buf, all_shp);
	    }
	    fclose(tmp_shp);
	}

    }

    /*
     *	Close em all now, like good little boys
     */
    fclose(shop_list);
    fclose(all_shp);

    fprintf(stdout, "\nSummary\t%d shops\n\n", shp_count);

    system("chmod 600 tworld.shp");

    fprintf(stdout, "Done\n");
    return 0;
}
