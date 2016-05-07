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
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "util.h"

static char *name;

// print usage to given stream
void usage_fd(FILE * f) {
	fprintf(f,"Usage: %s -G|-U -n name [-g gid] [-u uid] [-p password] [-m memberlist] [-s shell] [-c gecos] [-d dir] [-h] \n"
		"(C) 2013 ProgAndy\n",
                    basename(name));
}

// print usage to stdout
void usage(void) {
	usage_fd(stdout);
}

// print help to stdout
void help() {
	puts("fakeadd is a tool to add users groups to a fakeuser environment.\n"
			"fakeuser works best in conjunction with fakeroot. Use it with LD_PRELOAD.\n");
	usage();
	puts("\nPARAMETER:\n"
		"\n -U : ADD USER\n"
		"   -n name      - Username\n"
		"   -u uid       - User ID        (optional, default 0)\n"
		"   -g gid       - Group ID       (optional, default 0)\n"
		"   -p password  - Password       (optional, default empty)\n"
		"   -s shell     - Shell          (optional, default empty) Shell\n"
		"   -c gecos     - Real name      (optional, default empty)\n"
		"   -d dir       - Home directory (optional, default empty)\n"
		"\n -G : ADD GROUP\n"
		"   -n name        - Groupname\n"
		"   -g gid         - Group ID     (optional, default 0)\n"
		"   -p password    - Password     (optional, default empty) \n"
		"   -m memberlist  - Memberlist   (optional, default empty)\n"
		"                       [Example: username,exampleuser,nobody]"
		);
}

// convert string to array with specified delimiters
// if strings is not null, receives pointer to buffer for elements
char **string_to_array(const char *s, const char *delims, char **strings)
{
		char ** array = malloc( (strlen(s)/2 + 2) * sizeof(char*) );
        char *str = strdup(s);
		if (strings) *strings = str;
		int len = 0;

		char *tok = strtok(str, delims);
		while (tok) {
			array[len++] = tok;
			tok = strtok(NULL, delims);
		}
		array[len++] = NULL;
		array = (char**) realloc(array, len * sizeof(char*));
        return array;
}




int main(int argc, char **argv) {
	char * tmpdir = getenv("_FAKEUSER_DIR_");
	char *passwd_file, *group_file;

	name = argv[0];
    int opt, ret = 0;
	int action = 0, uid = 0, gid = 0;
    char *name = NULL, *passwd = NULL, *members = NULL, *shell = NULL, *gecos = NULL, *dir = NULL;
	extern char *optarg;

    while ((opt = getopt(argc, argv, "UGu:g:n:p:m:s:c:d:h")) != -1) {
        switch (opt) {
        case 'U':
            action = 'U';
            break;
		case 'G':
            action = 'G';
            break;
        case 'u':
            uid = atoi(optarg);
            break;
		case 'g':
            gid = atoi(optarg);
            break;
		case 'n':
			name = optarg;
			break;
		case 'p':
			passwd = optarg;
			break;
		case 'm':
			members = optarg;
			break;
		case 's':
			shell = optarg;
			break;
		case 'c':
			gecos = optarg;
			break;
		case 'd':
			dir = optarg;
			break;
		case 'h':
			help();
            exit(EXIT_SUCCESS);
        default: /* '?' */
            usage_fd(stderr);
            exit(EXIT_FAILURE);
        }
    }
	if (action == 0 || name == NULL) {
		usage();
		exit(EXIT_FAILURE);
	}
	// only continue when environment variable with directory found.
	if (!tmpdir) {
		fputs("Error! Not in fakeuser environment\n", stderr);
		exit(EXIT_FAILURE);
	}
	// init file paths
	passwd_file = (char*)malloc(strlen(tmpdir)+10);
	strcpy(passwd_file, tmpdir);
	strcat(passwd_file, "/passwd");

	group_file = (char*)malloc(strlen(tmpdir)+10);
	strcpy(group_file, tmpdir);
	strcat(group_file, "/group");

	// Create directory structure
	mkdir_r(tmpdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	if (action == 'U') {
		// create and append passwd entry
		struct passwd pw;
		pw.pw_name = name;
		pw.pw_passwd = passwd ? passwd : "";
		pw.pw_gecos = gecos ? gecos : "";
		pw.pw_dir = dir ? dir : "";
		pw.pw_shell = shell ? shell : "";
		pw.pw_uid = uid;
		pw.pw_gid = gid;
		// append to file with error handling.
		FILE * pwf = fopen(passwd_file, "a");
		if (pwf) {
			if(putpwent(&pw, pwf))
				ret = EIO;
			if (fclose(pwf))
				ret = EIO;
		} else
			ret = EIO;
	} else if (action == 'G') {
		// create and append group entry
		struct group gr;
		gr.gr_name = name;
		gr.gr_passwd = passwd ? passwd : "";
		gr.gr_gid = gid;
		char *strings;
		gr.gr_mem = members ? string_to_array(members, " ,;", &strings) : (char *[]){NULL};
		// append to file with error handling.
		FILE * pwf = fopen(group_file, "a");
		if (pwf) {
			if(putgrent(&gr, pwf))
				ret = EIO;
			if (fclose(pwf))
				ret = EIO;
		} else
			ret = EIO;
	}
	// return 0 on success or the error value
	return ret;
}
