#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"

int mkdir_r(char *dir, mode_t mode) {
	char *str = strdup(dir);
	char *tok = strtok(str, "/");
	int res = 0, setslash;
	while (tok) {
		setslash=1;
		while (tok != str && *(tok-setslash) == '\0') {
			*(tok-setslash) = '/';
			setslash++;
		}
		if (strlen(str)) {
			res = mkdir(str, mode);
			if (res && errno != EEXIST)
				break;
		}
		tok = strtok(NULL, "/");
	}
	free(str);
	return res;
}
