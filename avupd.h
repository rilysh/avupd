#ifndef AVUPD_H
#define AVUPD_H

#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE   0
#endif
#ifndef ARRAY_SIZE
    #define ARRAY_SIZE(v) (sizeof(v) / sizeof(v[0]))
#endif

#ifdef PROGRAM
    #undef PROGRAM
#endif
#define PROGRAM         "avupd"
#define LAST_UPDATE     "lastupd.log"
#define END             "\033[0m"
#define RED             "\033[0;91m"
#define GREEN           "\033[0;92m"

// NOTE: http://mech.math.msu.su/~vvb/2course/Borisenko/CppProjects/GWindow/xintro.html

static const char *upd_msg = "Your system needs and update. <3";

struct PkgMgr {
    char *path;
    char *cmd;
};

struct PkgMgr pkm[] = {
    { "/usr/bin/apt", "apt update -y && apt upgrade -y" },
    { "/usr/bin/dnf", "dnf upgrade -y" },
    { "/usr/bin/xbps-install", "xbps-install -Suy" },
    { "/usr/bin/pacman", "pacman -Suy" },
    { "/usr/bin/zypper", "zypper --non-interactive --auto-agree-with-licenses patch "}
};

static void perr(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void eerr(int stat, const char *msg)
{
    fprintf(stat == EXIT_SUCCESS ?
        stdout : stderr, "%s\n",
        msg == NULL ? "(null)" : msg
    );

    exit(stat);
}

static void ok_root(void)
{
    if (getuid() != 0 || getuid() != getegid())
        eerr(
            EXIT_FAILURE,
            "Error: You must have to be root user to use this command."
        );
}

#endif /* AVUPD_H */
