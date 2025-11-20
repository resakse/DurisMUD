#include <stdio.h>
#include <stdlib.h>

#define QUEST_LIST	"AREA"
#define QST_DIR		"qst"

#define ALL_QST		"tworld.qst"

void punt(char *msg)
{
    fprintf(stderr, "%s\n");
    exit(-1);
}

int main()
{
    FILE *quest_list, *all_qst, *tmp_qst;
    char quest[8192], buf[8192], quest_name[80], qst_name[80];
    int quest_count, qst_count;

    /*
     *	Open all the stupid files up
     */
    quest_list = fopen(QUEST_LIST, "r");
    if (quest_list==NULL)
	punt("QUEST file cannot be opened");

    all_qst = fopen(ALL_QST, "w");
    if (all_qst==NULL)
	punt("world.qst cannot be opened");

    quest_count = 0;
    qst_count = 0;
    for (;;) {
	fgets(quest, 8191, quest_list);
	if (quest==NULL)
	    break;
	if (feof(quest_list))
	    break;
	if (quest[0]=='*')		/* a comment */
	    continue;
	sscanf(quest, "%s", quest_name);
	fprintf(stdout, "Compiling quest file %2d : %s\n", quest_count++, quest_name);
	/*
	 * open up individual zon, wld, obj, mob files for each area
         */

	sprintf(qst_name, "%s/%s.qst", QST_DIR, quest_name);
	tmp_qst = fopen(qst_name, "r");
	if (tmp_qst==NULL) {
#if 0
	    fprintf(stdout, "\twarning: %s not found\n", qst_name);
#endif
	}
	else {
	    for (;;) {
		fgets(buf, 8191, tmp_qst);
		if (buf==NULL)
		    break;
		if (feof(tmp_qst))
		    break;
		if (*buf=='#')
		    qst_count++;
		fputs(buf, all_qst);
	    }
	    fclose(tmp_qst);
	}

    }

    /*
     *	Close em all now, like good little boys
     */
    fclose(quest_list);
    fclose(all_qst);

    fprintf(stdout, "\nSummary\t%d quests\n\n", qst_count);

    system("chmod 600 tworld.qst");

    fprintf(stdout, "Done\n");
    return 0;
}
