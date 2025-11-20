#include <stdio.h>
#include <stdlib.h>

#define AREA_LIST	"AREA"
#define WLD_DIR		"wld"

#define ALL_WLD		"tworld.wld"

void punt(char *msg)
{
    fprintf(stderr, "%s\n");
    exit(-1);
}

int main()
{
    FILE *area_list, *all_wld, *tmp_wld;
    char area[8192], buf[8192], area_name[80], wld_name[80];
    int area_count, wld_count;

    /*
     *	Open the wld files up
     */
    area_list = fopen(AREA_LIST, "r");
    if (area_list==NULL)
	punt("AREA file cannot be opened");

    all_wld = fopen(ALL_WLD, "w");
    if (all_wld==NULL)
	punt("world.wld cannot be opened");

    area_count = 0;
    wld_count = -1;
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
	 * open up individual wld for each area
         */

	sprintf(wld_name, "%s/%s.wld", WLD_DIR, area_name);
	tmp_wld = fopen(wld_name, "r");
        if (tmp_wld==NULL) {
            fprintf(stdout, "\twarning: %s not found\n", wld_name);
        }
        else {
            for (;;) {
                fgets(buf, 8191, tmp_wld);
                if (buf==NULL)
                    break;
                if (feof(tmp_wld))
                    break;
		if (*buf=='#')
		    wld_count++;
                fputs(buf, all_wld);
            }
            fclose(tmp_wld);
        }

    }

    /*
     *	Close wld, like a good little boy
     */
    fclose(area_list);
    fclose(all_wld);

    fprintf(stdout, "\nSummary:\t%d rooms\n\n", wld_count);

    system("chmod 600 tworld.wld");

    fprintf(stdout, "Done\n");
    return 0;
}
