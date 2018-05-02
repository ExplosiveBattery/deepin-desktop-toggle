#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int LINE_BUFFER_SIZE = 50;

Window *list(Display *disp, unsigned long *len)
{
    Atom prop = XInternAtom(disp, "_NET_CLIENT_LIST", False), type;
    int form,error; unsigned long remain;
    unsigned char *list;

    if(Success != (error=XGetWindowProperty(disp, XDefaultRootWindow(disp), prop, 0, (~0L), False, XA_WINDOW,
                            &type, &form, len, &remain, &list))) {
        fprintf(stderr, "Get '_NET_CLIENT_LIST' property error %d!\n", error);
        XCloseDisplay(disp);
        exit(-1);
    }
    return (Window *)list;
}


void ShowDesktp(Display *disp) {
    Window root = XDefaultRootWindow(disp);

    Atom showing_desktop_atom =XInternAtom(disp, "_NET_SHOWING_DESKTOP", True), actual_type;
    int actual_format, error,state=0;
    unsigned long len, after;
    unsigned char *data = NULL;
    if(Success != (error=XGetWindowProperty(disp, root, showing_desktop_atom, 0, 1, False, XA_CARDINAL,
                               &actual_type, &actual_format, &len, &after, &data))) {
        fprintf(stderr, "Get '_NET_SHOWING_DESKTOP' property error %d!\n", error);
        return;
    }
    if(len) state = data[0]; 
    XFree(data);
    data = NULL;

    XEvent xev = {
        .xclient = {
            .type = ClientMessage,
            .send_event = True,
            .window = root,
            .message_type = showing_desktop_atom,
            .format = 32,
            .data.l[0] = !state,
            .data.l[1] = 0,
            .data.l[2] = 0,
            .data.l[3] = 0,
            .data.l[4] = 0 
        }
    };
    XSendEvent(disp, root, False, SubstructureRedirectMask |SubstructureNotifyMask, &xev);

}

void ShowWindow(Display *disp, Window window) {
    XEvent xev = {
        .xclient = {
            .type = ClientMessage,
            .send_event = True,
            .window = window,
            .message_type = XInternAtom(disp, "_NET_ACTIVE_WINDOW", True),
            .format = 32,
            .data.l[0] = 1,
            .data.l[1] = 0, //timestamp
            .data.l[2] = 0,
            .data.l[3] = 0,
            .data.l[4] = 0 
        }
    };
    XSendEvent(disp, XDefaultRootWindow(disp), False, SubstructureRedirectMask |SubstructureNotifyMask, &xev);
    XFlush (disp);
}

int main(int argc, char *argv[])
{
    Display *disp;
    Window *wlist;
    char *wname;
    unsigned long len;
    if(!(disp = XOpenDisplay(NULL))) {
        fprintf(stderr, "Cannot open display \"%s\".\n", XDisplayName(NULL));
        return -1;
    }

    wlist = list(disp, &len);
    if( argc==1 ) {
    //if no param
        char file_path[50];
        strcpy(file_path, getenv("HOME"));
        strcat(file_path, "/.toggleconfig");

        ShowDesktp(disp);
        FILE *config = fopen(file_path, "r"); char line_buffer[LINE_BUFFER_SIZE];
        if (config) {
            while(fgets(line_buffer, LINE_BUFFER_SIZE, config)) {
                line_buffer[strlen(line_buffer)-1] = '\0';
                for(unsigned long i = 0; i < len; i++){
                        XFetchName(disp, wlist[i], &wname);
                        if( wname && !strcmp(wname,line_buffer) ){
                            ShowWindow(disp, wlist[i]);
                        }
                        XFree(wname);
                }
            }
            fclose(config);
        }
    }else if(argc==2 && !strcmp(argv[1],"-l")){
        //list windows
        for(int i = 0; i < (int)len; i++){
                XFetchName(disp, wlist[i], &wname);
                if(wname && strcmp(wname,"")){
                        printf("%s\n", wname);
                }
                XFree(wname);
        }
    }else {
        //show help
        puts("Usage: desktop-toggle [options]");
        puts("Options:");
        puts("  no param\t\tshow the desktop and the windows which are appointed in ~/.toggleconfig");
        puts("  -l\t\t\tlist the top level windows' names at now\n");
        puts("A new toggle program made by Vega. (a senior student of SCU)");
    }

    // XFree(wlist);
    XCloseDisplay(disp);
    return 0;
}