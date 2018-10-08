/*
Simple Note service by bolek42
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *user;
size_t len;

void login() {
    char *fname = NULL;
    char *pw = NULL;
    char *username = NULL;
    char *password = NULL;

    if (user) {
        printf("ERROR: already logged in\n");
        return;
    }

    printf("Username: ");
    fflush(stdout);
    getline(&username, &len, stdin);
    username[strlen(username) - 1] = 0x00;
    if (strchr(username, '/')) {
        printf("ERROR: invalid username\n");
        free(username);
        return;
    }

    fname = (char *)malloc(strlen(username) + 3 + 1);
    strcpy(fname, "db/");
    strcat(fname, username);
    if (access(fname, F_OK) == -1) {
        printf("ERROR: user does not exist\n");
        free(fname);
        free(username);
        return;
    }

    printf("Password: ");
    fflush(stdout);
    getline(&password, &len, stdin);
    password[strlen(password) - 1] = 0x00;

    user = fopen(fname, "r+");
    getline(&pw, &len, user);
    pw[strlen(pw) - 1] = 0x00;
    free(fname);

    if (strcmp(pw, password)){
        printf("ERROR: invalid password\n");
        fclose(user);
        user = NULL;
        free(username);
        free(password);
        free(pw);
        return;
    }
    printf("Succsess\n");

    free(username);
    free(password);
    free(pw);

    return;
}

void reg() {
    char *fname = NULL;
    char *username = NULL;
    char *password = NULL;

    printf("Username: ");
    fflush(stdout);
    getline(&username, &len, stdin);
    username[strlen(username) - 1] = 0x00;
    if (strchr(username, '/')) {
        printf("ERROR: invalid username\n");
        free(username);
        return;
    }

    fname = (char *)malloc(strlen(username) + 3 + 1);
    strcpy(fname, "db/");
    strcat(fname, username);
    if (access(fname, F_OK ) != -1) {
        printf("ERROR: user exists\n");
        free(username);
        free(fname);
        return;
    }

    printf("Password: ");
    fflush(stdout);
    getline(&password, &len, stdin);

    user = fopen(fname, "w");
    fwrite(password, strlen(password), 1, user);
    fflush(user);
    printf("Succsess\n");

    free(username);
    free(password);
    free(fname);

    return;
}

void note_show() {
    char *buff = NULL;
    if (!user) {
        printf("ERROR: not logged in\n");
        return;
    }

    fseek(user, 0, SEEK_SET);
    getline(&buff, &len, user);
    free(buff);
    while (!feof(user)) {
        buff = NULL;
        if(getline(&buff, &len, user) > 0) {
            printf("%s", buff);
            free(buff);
        }
    }
    free(buff);

   return; 
}

void logout() {
    if (user != NULL)
        fclose(user);
    printf("Succsess\n");

    return;
}

void note_delete() {
    printf("ERROR: the cloud never forgets\n");

    return;
}

void note_add() {
    char *buff = NULL;
    char c;

    if (!user) {
        printf("ERROR: not logged in\n");
        return;
    }

    printf("note> ");
    fflush(stdout);
    getline(&buff, &len, stdin);
    fseek(user, 0, SEEK_END);
    fwrite(buff, strlen(buff), 1, user);
    free(buff);
}

void help() {
    printf("login           log in as user\n");
    printf("register        register new user\n");
    printf("note show       show your notes\n");
    printf("note add        add new note\n");
    printf("note delete     delete yout note\n");
    printf("logout          safely logout\n");
    printf("quit            quit session\n");

}

int main() {
    char *cmd = NULL;
    printf("  ____ _                 _ _   _       _       \n");
    printf(" / ___| | ___  _   _  __| | \\ | | ___ | |_ ___ \n");
    printf("| |   | |/ _ \\| | | |/ _` |  \\| |/ _ \\| __/ _ \\\n");
    printf("| |___| | (_) | |_| | (_| | |\\  | (_) | ||  __/\n");
    printf(" \\____|_|\\___/ \\__,_|\\__,_|_| \\_|\\___/ \\__\\___|\n");
    printf("                                               \n");
    printf("-----------------------------------------------\n");
    printf("Because it's a great Idea!\n");


    while(1) {
        printf("cmd> ");
        fflush(stdout);
        getline(&cmd, &len, stdin);
        cmd[strlen(cmd)-1] = 0x00;
        if (!strcmp(cmd, "login")) login();
        else if (!strcmp(cmd, "register")) reg();
        else if (!strcmp(cmd, "note show")) note_show();
        else if (!strcmp(cmd, "note add")) note_add();
        else if (!strcmp(cmd, "note delete")) note_delete();
        else if (!strcmp(cmd, "logout")) logout();
        else if (!strcmp(cmd, "quit")) break;
        else help();
    }
}
