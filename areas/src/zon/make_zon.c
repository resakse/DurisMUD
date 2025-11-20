#include <stdio.h>
#include <stdlib.h>

#define AREA_LIST	"AREA"
#define ZON_DIR		"zon"

#define ALL_ZON		"tworld.zon"

void punt(char *msg)
{
    fprintf(stderr, "%s\n");
    exit(-1);
}

int main()
{
    FILE *area_list, *all_zon, *tmp_zon;
    char area[8192], buf[8192], area_name[80], zon_name[80];
    int area_count, zon_count, do_it = 0;

    /*
     *	Open the zon files up
     */
    area_list = fopen(AREA_LIST, "r");
    if (area_list==NULL)
	punt("AREA file cannot be opened");

    all_zon = fopen(ALL_ZON, "w");
    if (all_zon==NULL)
	punt("world.zon cannot be opened");

    area_count = 0;
    zon_count = -1;
    for (;;) {
	fgets(area, 8191, area_list);
	if (area==NULL)
	    break;
	if (feof(area_list))
	    break;
	if (area[0]=='*')		/* a comment */
	    continue;
	sscanf(area, "%s", area_name);
	fprintf(stdout, "Area[%2d] : %s\n", area_count++, area_name);
	/*
	 * open up individual zon for each area
         */

	sprintf(zon_name, "%s/%s.zon", ZON_DIR, area_name);
	tmp_zon = fopen(zon_name, "r");
        if (tmp_zon==NULL) {
            fprintf(stdout, "\twarning: %s not found\n", zon_name);
        }
        else {
            do_it = 0;
            for (;;) {
                fgets(buf, 8191, tmp_zon);
                if (buf==NULL)
                    break;
                if (feof(tmp_zon)) 
                    break;
                fputs(buf, all_zon);
		/* start of zone file, write filename after zone name */

                if (do_it)
                {
                    fprintf(all_zon, "%s~\n", area_name);
                    do_it = 0;
                }
		if (*buf=='#') {
		    zon_count++;
                    do_it = 1;
                }
            }
            fclose(tmp_zon);
        }

    }

    /*
     *	Close zon, like a good little boy
     */
    fclose(area_list);
    fprintf(all_zon, "$~\n");
    fclose(all_zon);

    fprintf(stdout, "\nSummary: %d zones\n\n", zon_count);

    system("chmod 600 tworld.zon");

    fprintf(stdout, "Done\n");
    return 0;
}
