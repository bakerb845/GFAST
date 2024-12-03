#ifndef GEOJSON_UTILS_H
#define GEOJSON_UTILS_H 1

#define MAX_PAYLOAD_SIZE 1024
#define NATS_SUBJECT "test.carl"

struct json_msg_struct {
    char **msgs;
    int nmsg;
};

struct json_msg_struct *read_jsonmsgs(const char *filename);

#endif /* GEOJSON_UTILS_H */