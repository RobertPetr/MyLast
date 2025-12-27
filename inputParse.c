#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utmp.h>

/*
 * executabilul va citi din fisierul dat de scriptul MyLast
 * coduri de eroare:
 * - 1: parametrii gresiti
 * - 2: fisier inexistent/inaccesibil
 * - 3: fisier gol/alterat
 * fisierul dat trebuie sa fie un byte file
*/

#define ERR_ARGS 1
#define ERR_OPEN 2
#define ERR_EMPTY 3
#define ERR_CORRUPT 4

int main(int argc, char **argv)
{
    if (argc != 2)
        return ERR_ARGS;
    
    char *cale = argv[1];
    int f = open(cale, O_RDONLY);
    // eroare la deschiderea fisierului
    if (f < 0)
    {
        // exemplu de eroare:
        // "Nu sa putut deschide fisierul '/var/log/wtmp': Permission denied\n"
        fprintf(stderr, "Nu sa putut deschide fisierul '%s', deoarece %s\n", cale, strerror(errno));
        return ERR_OPEN;
    }

    // meta-data-ul fisierului:
    struct stat st;
    if (fstat(f, &st) < 0)
    {
        fprintf(stderr, "Nu sa putut accesa metadata-ul fisierului '%s', deoarece: %s\n", cale, strerror(errno));
        return ERR_OPEN;
    }

    if (st.st_size == 0)
    {
        fprintf(stderr, "Nu exista record de logari");
        return ERR_EMPTY;
    }
    
    if (st.st_size % sizeof(struct utmp) != 0)
    {
        fprintf(stderr, "Fisierul este corupt");
        return ERR_CORRUPT;
    }

    int nrLinii = st.st_size / sizeof(struct utmp);
    printf("%d\n", nrLinii);
    
    // parsarea fisierului in directia inversa (la fel ca last/lastb):
    for (off_t offset = st.st_size - sizeof(struct utmp); offset >= 0; offset -= sizeof(struct utmp))
    {
        int flag = 1;
        // mutam cursorul
        lseek(f, offset, SEEK_SET);

        struct utmp entry;
        // citire incompleta
        if (read(f, &entry, sizeof(entry)) != sizeof(entry))
        {
            fprintf(stderr, "Fisierul nu a putut fi parsat");
            return ERR_CORRUPT;
        }

        // salt peste liniile care contin alte date
        if (entry.ut_type != USER_PROCESS && entry.ut_type != DEAD_PROCESS)
            flag=0;

        // afisare in terminal in form CSV pentru urmatorul pas:
        // evitare a unui bug (un string sa nu contina un \0 la final)
        char user[UT_NAMESIZE+1], line[UT_LINESIZE+1], host[UT_HOSTSIZE+1];

        memcpy(user, entry.ut_user, UT_NAMESIZE); memcpy(line, entry.ut_line, UT_LINESIZE); memcpy(host, entry.ut_host, UT_HOSTSIZE);
        // asigurare ca toate stringurile se termina cu '\0'
        user[UT_NAMESIZE] = line[UT_LINESIZE] = host[UT_HOSTSIZE] = '\0';

        // verificare explicita pentru cazul acesta
        if (user[0] == '\0')
        {
            strcpy(user, "reboot");
            strcpy(line, "system boot");
            strcpy(host, "-");
        }
        
        printf("%d,%s,%s,%s,%ld,%ld\n",
            flag,
            user, line, host,
            entry.ut_tv.tv_sec,
            0L);
    }

    close(f);
    return 0;
}