/*
    (C) 2013-2016 ProgAndy

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include <errno.h>
#include "util.h"


static void fakeuser_init(void) __attribute__ ((constructor));
static void fakeuser_uninit(void) __attribute__ ((destructor));


#define TMPROOT "/tmp/fakeuser"
static int fake_owner = 0;
static char tmpdirtmpl[] = TMPROOT "/XXXXXX";
static char *tmpdir, *passwd_file, *group_file;

FILE *pwstream = NULL;
FILE *grstream = NULL;

void setpwent (void) {
	void (*o_setpwent)(void) = NULL;
	if (!o_setpwent) o_setpwent = dlsym(RTLD_NEXT, "setpwent");
	// reset cursor for custom password file
	if (pwstream)
		rewind(pwstream);
	o_setpwent();
}

void endpwent (void) {
	void (*o_endpwent)(void) = NULL;
	if (!o_endpwent) o_endpwent = dlsym(RTLD_NEXT, "endpwent");
	if (pwstream) {
		// close custom password file
		fclose(pwstream);
		pwstream = NULL;
	}
	o_endpwent();
}

struct passwd *getpwent (void) {
	struct passwd *(*o_getpwent)(void) = NULL;
	// try to open custom password file
	if (!pwstream)
		pwstream = fopen(passwd_file, "r");
	if (pwstream) {
		// get next entry from custom password file
		struct passwd *pw = fgetpwent(pwstream);
		if (pw) return pw;
	}
	// use default entries after the end of the custom file
	if (!o_getpwent) o_getpwent = dlsym(RTLD_NEXT, "getpwent");
	return o_getpwent();
}

struct passwd *getpwnam (const char *__name) {
	struct passwd *(*o_getpwnam)(const char *__name) = NULL;

	// iterate through custom passwd file and search for name
	// return it if found
	struct passwd *pw;
	FILE *pwf = fopen(passwd_file, "r");
	if (pwf) {
		do {
			pw = fgetpwent(pwf);
			if (pw && !strcmp(pw->pw_name, __name))
				break;
		} while (pw);
		fclose(pwf);
		if (pw && !strcmp(pw->pw_name, __name))
			return pw;
	}

	// fall back to original behaviour
	if (!o_getpwnam) o_getpwnam = dlsym(RTLD_NEXT, "getpwnam");
	return o_getpwnam(__name);
}

struct passwd *getpwuid (__uid_t __uid) {
	struct passwd *(*o_getpwuid)(__uid_t __uid) = NULL;

	// iterate through custom passwd file and search for id
	// return it if found
	struct passwd *pw;
	FILE *pwf = fopen(passwd_file, "r");
	if (pwf) {
		do {
			pw = fgetpwent(pwf);
			if (pw && pw->pw_uid == __uid)
				break;
		} while (pw);
		fclose(pwf);
		if (pw && pw->pw_uid == __uid)
			return pw;
	}

	// fall back to original behaviour
	if (!o_getpwuid) o_getpwuid = dlsym(RTLD_NEXT, "getpwuid");
	return o_getpwuid(__uid);
}

int getpwuid_r (__uid_t __uid, struct passwd *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct passwd **__restrict __result) {
	int (*o_getpwuid_r)(__uid_t, struct passwd *, char*, size_t, struct passwd**) = NULL;
	if (__resultbuf == NULL || __result == NULL) return ENOMEM;
	*__result = NULL;

	// iterate through custom passwd file and search for id
	// return it if found
	int res;
	FILE *pwf = fopen(passwd_file, "r");
	if (pwf) {
		do {
			res = fgetpwent_r(pwf, __resultbuf, __buffer, __buflen, __result);
			if (!res && __resultbuf->pw_uid == __uid)
				break;
		} while (!res);
		fclose(pwf);
		if (res == ERANGE)
			return ERANGE;
		if (!res && __resultbuf->pw_uid == __uid)
			return 0;
	}

	// fall back to original behaviour
	if (!o_getpwuid_r) o_getpwuid_r = dlsym(RTLD_NEXT, "getpwuid_r");
	return o_getpwuid_r(__uid, __resultbuf, __buffer, __buflen, __result);
}


// functions for groups. Same structure as for users.

void setgrent (void) {
	void (*o_setgrent)(void) = NULL;
	if (!o_setgrent) o_setgrent = dlsym(RTLD_NEXT, "setgrent");
	if (!grstream)
		grstream = fopen(group_file, "r");
	else
		rewind(grstream);
	o_setgrent();
}

void endgrent (void) {
	void (*o_endgrent)(void) = NULL;
	if (!o_endgrent) o_endgrent = dlsym(RTLD_NEXT, "endgrent");
	if (grstream) {
		fclose(grstream);
		grstream = NULL;
	}
	o_endgrent();
}

struct group *getgrent (void) {
	struct group *(*o_getgrent)(void) = NULL;

	if (!grstream)
		grstream = fopen(group_file, "r");
	if (grstream) {
		struct group *gr = fgetgrent(grstream);
		if (gr) return gr;
	}

	if (!o_getgrent) o_getgrent = dlsym(RTLD_NEXT, "getgrent");
	return o_getgrent();
}

struct group *getgrnam (const char *__name) {
	struct group *(*o_getgrnam)(const char *__name) = NULL;

	struct group *gr;
	FILE *pwf = fopen(group_file, "r");
	if (pwf) {
		do {
			gr = fgetgrent(pwf);
			if (gr && !strcmp(gr->gr_name, __name))
				break;
		} while (gr);
		fclose(pwf);
		if (gr && !strcmp(gr->gr_name, __name))
			return gr;
	}

	if (!o_getgrnam) o_getgrnam = dlsym(RTLD_NEXT, "getgrnam");
	return o_getgrnam(__name);
}

struct group *getgrgid (__gid_t __gid) {
	struct group *(*o_getgrgid)(__gid_t __gid) = NULL;

	struct group *gr;
	FILE *pwf = fopen(group_file, "r");
	if (pwf) {
		do {
			gr = fgetgrent(pwf);
			if (gr && gr->gr_gid == __gid)
				break;
		} while (gr);
		fclose(pwf);
		if (gr && gr->gr_gid == __gid)
			return gr;
	}

	if (!o_getgrgid) o_getgrgid = dlsym(RTLD_NEXT, "getgrgid");
	return o_getgrgid(__gid);
}

/*
int getgrent_r (struct group *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct group **__restrict __result) {
	int (*getgrent_r)(struct group *, char*, size_t*, struct group**) = NULL;

}
*/

int getgrgid_r (__gid_t __gid, struct group *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct group **__restrict __result) {
	int (*o_getgrgid_r)(__gid_t, struct group *, char*, size_t, struct group**) = NULL;
	if (__resultbuf == NULL || __result == NULL) return ENOMEM;

	int res;
	FILE *pwf = fopen(group_file, "r");
	if (pwf) {
		do {
			res = fgetgrent_r(pwf, __resultbuf, __buffer, __buflen, __result);
			if (!res && __resultbuf->gr_gid == __gid)
				break;
		} while (!res);
		fclose(pwf);
		if (res == ERANGE)
			return ERANGE;
		if (!res && __resultbuf->gr_gid == __gid)
			return 0;
	}

	if (!o_getgrgid_r) o_getgrgid_r = dlsym(RTLD_NEXT, "getgrgid_r");
	return o_getgrgid_r(__gid, __resultbuf, __buffer, __buflen, __result);
}

/*
extern int getgrnam_r (const char *__restrict __name,
		       struct group *__restrict __resultbuf,
		       char *__restrict __buffer, size_t __buflen,
		       struct group **__restrict __result);
*/

// constructor for library (requires gcc)
static void fakeuser_init(void) {
	char * dir = getenv("_FAKEUSER_DIR_");
	if (dir) {
		// slave instance, read from known directory
		tmpdir = dir;
	} else {
		// no directory for custom files specified
		// take authority role and generate uniqe name
		// publish directory name in environment
		tmpdir = mktemp(tmpdirtmpl);
		fake_owner = 1;
		setenv("_FAKEUSER_DIR_", tmpdir, 1);
		mkdir_r(tmpdir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	}
	// precompile full names for custom files
	passwd_file = (char*)malloc(strlen(tmpdir)+10);
	strcpy(passwd_file, tmpdir);
	strcat(passwd_file, "/passwd");

	group_file = (char*)malloc(strlen(tmpdir)+10);
	strcpy(group_file, tmpdir);
	strcat(group_file, "/group");
}

// destructor for library (requires gcc)
static void fakeuser_uninit(void) {
	if (fake_owner) {
		// delete password files if we are the authority
		unlink(passwd_file);
		unlink(group_file);
		rmdir(tmpdir);
		rmdir(TMPROOT);
	}
}
