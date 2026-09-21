#ifndef CLI_H
#define CLI_H

struct arguments {
	char *device_id;
	char *device_secret;
	char *product_id;
};

int parse_arguments(int argc, char **argv, struct arguments *arguments);

#endif