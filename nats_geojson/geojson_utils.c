#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geojson_utils.h"

struct json_msg_struct *read_jsonmsgs(const char *filename) {
    FILE *infl;
    int nlines = 0, i;

    // Stuff for saving the output to be processed later
    // how many chars could it be?
    const int max_payload_size = MAX_PAYLOAD_SIZE;
    // set from MAX_MESSAGES in gfast, which is defined as 100000 as of v1.2.2
    const int message_block = 100000;

    char cline[max_payload_size];
    struct json_msg_struct *json_msgs;

    json_msgs = malloc(sizeof(struct json_msg_struct));

    json_msgs->msgs = (char **)malloc(message_block * sizeof(char *));
    json_msgs->nmsg = 0;

    infl = fopen(filename, "r");
    if (!infl) {
        printf("ERROR! Cannot open %s\n", filename);
        return json_msgs;
    }
    while (fgets(cline, max_payload_size, infl) != NULL) {
        nlines = nlines + 1;
    }

    if (nlines < 1) {
        printf("%s\n", "No sites in file");
        fclose(infl);
        return json_msgs;
    }

    rewind(infl);
    for (i = 0; i < nlines; i++) {
        memset(cline, 0, sizeof(cline));
        if (fgets(cline, max_payload_size, infl) == NULL)
        {
            printf("Premature end of file\n");
            return json_msgs;
        }

        if (cline[strlen(cline) - 1] == '\n')
        {
            cline[strlen(cline) - 1] = '\0';
        }

        json_msgs->msgs[json_msgs->nmsg] = 
            (char *)malloc(max_payload_size * sizeof(char));
        snprintf(json_msgs->msgs[json_msgs->nmsg],
                 max_payload_size,
                 "%s",
                 cline);
        json_msgs->nmsg++;
    }

    fclose(infl);
    return json_msgs;
}