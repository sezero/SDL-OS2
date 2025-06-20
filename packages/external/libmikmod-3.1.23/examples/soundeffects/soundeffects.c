/* soundeffects.c
 * An example on how to use libmikmod to play sound effects.
 *
 * (C) 2004, Raphael Assenat (raph@raphnet.net)
 *
 * This example is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRENTY; without event the implied warrenty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <mikmod.h>

#if defined(_WIN32)
#define MikMod_Sleep(ns) Sleep(ns / 1000)
#elif defined(_MIKMOD_AMIGA)
void amiga_sysinit (void);
void amiga_usleep (unsigned long timeout);
#define MikMod_Sleep(ns) amiga_usleep(ns)
#else
#include <unistd.h>  /* for usleep() */
#define MikMod_Sleep(ns) usleep(ns)
#endif

int main(void)
{
    /* sound effects */
    SAMPLE *sfx1, *sfx2;
    /* voices */
    int v1, v2;
    int i;

#ifdef _MIKMOD_AMIGA
    amiga_sysinit ();
#endif

    /* register all the drivers */
    MikMod_RegisterAllDrivers();

    /* initialize the library */
    md_mode |= DMODE_SOFT_SNDFX;
    if (MikMod_Init("")) {
        fprintf(stderr, "Could not initialize sound, reason: %s\n",
                MikMod_strerror(MikMod_errno));
        return 1;
    }

    /* load samples */
    sfx1 = Sample_Load("first.wav");
    if (!sfx1) {
        MikMod_Exit();
        fprintf(stderr, "Could not load the first sound, reason: %s\n",
                MikMod_strerror(MikMod_errno));
        return 1;
    }
    sfx2 = Sample_Load("second.wav");
    if (!sfx2) {
        Sample_Free(sfx1);
        MikMod_Exit();
        fprintf(stderr, "Could not load the second sound, reason: %s\n",
                MikMod_strerror(MikMod_errno));
        return 1;
    }

    /* reserve 2 voices for sound effects */
    MikMod_SetNumVoices(-1, 2);

    /* get ready to play */
    MikMod_EnableOutput();

    /* play first sample */
    v1 = Sample_Play(sfx1, 0, 0);
    do {
        MikMod_Update();
        MikMod_Sleep(100000);
    } while (!Voice_Stopped(v1));

    for (i = 0; i < 10; i++) {
        MikMod_Update();
        MikMod_Sleep(100000);
    }

    /* half a second later, play second sample */
    v2 = Sample_Play(sfx2, 0, 0);
    do {
        MikMod_Update();
        MikMod_Sleep(100000);
    } while (!Voice_Stopped(v2));

    for (i = 0; i < 10; i++) {
        MikMod_Update();
        MikMod_Sleep(100000);
    }

    MikMod_DisableOutput();

    Sample_Free(sfx2);
    Sample_Free(sfx1);

    MikMod_Exit();

    return 0;
}
