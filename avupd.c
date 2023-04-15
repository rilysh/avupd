#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <X11/Xlib.h>

#include "avupd.h"

static int require_update(int update)
{
    FILE *fp;
    struct tm *tm;
    time_t ti;
    char pre[30], now[30], lupath[60];
    int ret;

    ti = time(NULL);
    tm = localtime(&ti);

    memset(pre, '\0', sizeof(pre));
    memset(now, '\0', sizeof(now));
    memset(lupath, '\0', sizeof(lupath));

    strcat(lupath, "/home/");
    strcat(lupath, LAST_UPDATE);

    if (update) {
        fp = fopen(lupath, "w");

        if (fp == NULL)
            eerr(EXIT_FAILURE,
                "Error: No log file found, please create a new one with \"-n\" flag."
            );

        sprintf(
            pre, "%d:%02d:%02d", tm->tm_year + 1900,
            tm->tm_mon + 1, tm->tm_mday
        );

        fprintf(fp, "%s", pre);
        fclose(fp);

        return 2;
    }

    fp = fopen(lupath, "r");

    if (fp == NULL)
        eerr(EXIT_FAILURE,
            "Error: No log file found, please create a new one with \"-n\" flag."
        );

    fread(pre, 1L, sizeof(pre), fp);
    fclose(fp);

    sprintf(
        now, "%d:%02d:%02d", tm->tm_year + 1900,
        tm->tm_mon + 1, tm->tm_mday
    );

    ret = strcmp(now, pre);

    if (ret > 0)
        return 1; /* yes */
    else
        return 0;
}

static void notify_user_gui(void)
{
    Display *dpy;
    Window win;
    XEvent ev;
    GC gc;
    XFontStruct *font;
    Atom at;
    int scr, running;

    dpy = XOpenDisplay(NULL);

    if (dpy == NULL)
        perr("XOpenDisplay");

    scr = DefaultScreen(dpy);
    win = XCreateSimpleWindow(
        dpy, DefaultRootWindow(dpy), 10, 10, 330, 40, 1,
        BlackPixel(dpy, 0), WhitePixel(dpy, 0)
    );

    XStoreName(dpy, win, "avupd");
    XCreateSimpleWindow(dpy, win, 20, 10, 20, 20, 1,
        WhitePixel(dpy, scr), WhitePixel(dpy, scr)
    );

    if (!(gc = XDefaultGC(dpy, DefaultScreen(dpy))))
        perr("XDefaultGC()");

    /* Invoke: xlsfonts */
    if (!(font = XLoadQueryFont(dpy, "lucidasanstypewriter-bold-12")))
        perr("XLoadQueryFont()");

    at = XInternAtom(dpy, "WM_DELETE_WINDOW", TRUE);

    XSetWMProtocols(dpy, win, &at, TRUE);
    /* XSetFont should never be reach if XLoadQueryFont fails, or unless
    XSetFont unable to set the specified font to the window */
    XSetFont(dpy, gc, font->fid);
    XSelectInput(dpy, win, ExposureMask | KeyPressMask);
    XMapWindow(dpy, win);

    running = TRUE;

    while (running) {
        XPending(dpy);
        XNextEvent(dpy, &ev);

        switch (ev.type) {
        case Expose:
            XDrawString(
                dpy, win, DefaultGC(dpy, scr),
                5, 20, upd_msg, strlen(upd_msg)
            );
            break;

        case ClientMessage:
            if ((long unsigned int)ev.xclient.data.l[0] == at)
                running = FALSE;
            break;

        default:
            continue;
        }
    }

    /* Not freeing GC as XFreeGC also frees the display */
    XFreeFont(dpy, font);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

static void notify_user_cui(void)
{
    fprintf(stdout,
        "%sDaily reminder%s: %sYou need to update your system.%s\n",
        RED, END, GREEN, END
    );
}

static void invoke_pkgmgr(void)
{
    char *cmd, scmd[60], *args[4];
    long unsigned int i;

    memset(scmd, '\0', sizeof(scmd));

    for (i = 0; i < ARRAY_SIZE(pkm); i++) {
        if ((i == ARRAY_SIZE(pkm) - 1) &&
            (access(pkm[i].path, F_OK) != 0)) {
            eerr(EXIT_FAILURE, "Error: No known package manager found.");
        }

        if (access(pkm[i].path, F_OK) == 0) {
            strcat(scmd, pkm[i].cmd);
            break;
        }
    }

    cmd = "/usr/bin/sh";
    args[0] = cmd;
    args[1] = "-c";
    args[2] = scmd;
    args[3] = NULL;

    /* call require_update() before as we can't create a call
        after execution
    */
    require_update(TRUE);

    if (execve(cmd, args, NULL) == -1)
        perr("execvp()");
}

static void create_new_log(void)
{
    FILE *fp;
    char lupath[60];
    int mask;

    memset(lupath, '\0', sizeof(lupath));

    strcat(lupath, "/home/");
    strcat(lupath, LAST_UPDATE);

    fp = fopen(lupath, "w");
    fclose(fp);

    /* O666 */
    mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    if (chmod(lupath, mask) == -1)
        perr("chmod()");
}

static void print_usage(void)
{
    fprintf(stdout,
        "%s - Another Vital Update\n\n"
        "Usage:\n"
        "   -c      -- Check if there's an update\n"
        "   -u      -- Update the system (by invoking the package manager, if known)\n"
        "   -n      -- Create a new log file (overwrites previous one)\n"
        "   -i      -- Show a X11 GUI window box\n"
        "   -h      -- Shows this help section\n",
        PROGRAM
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        print_usage();
        exit(EXIT_SUCCESS);
    }

    if (argv[1][0] != '-')
        eerr(EXIT_FAILURE, "Error: Invalid argument(s)");

    int opt, cflag, uflag, iflag, nflag, hflag;
    const char *short_opts;

    cflag = uflag = iflag = nflag = hflag = 0;
    short_opts = "cuinh";

    while ((opt = getopt(argc, argv, short_opts)) != -1) {
        switch (opt) {
        case 'c':
            cflag = 1;
            break;

        case 'u':
            uflag = 1;
            break;

        case 'i':
            iflag = 1;
            break;

        case 'n':
            nflag = 1;
            break;

        case 'h':
            hflag = 1;
            break;
        }
    }

    if (cflag) {
        if (require_update(FALSE) == 1) {
            if (iflag) {
                if (fork() > 0)
                    exit(EXIT_SUCCESS);

               notify_user_gui();
            } else { 
                notify_user_cui();
            }
        }
    }

    if (uflag) {
        ok_root();
        invoke_pkgmgr();
    }

    if (nflag) {
        ok_root();
        create_new_log();
    }

    if (hflag)
        print_usage();
}
