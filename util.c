/*
    (C) 2013 ProgAndy

    This file is part of fakeuser.

    fakeuser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    fakeuser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fakeuser.  If not, see <http://www.gnu.org/licenses/>.
*/


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
