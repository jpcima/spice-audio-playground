#include <ngspice/sharedspice.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>

#define TRACE(fmt, ...) fprintf(stderr, "*** %s: " fmt "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__)

struct SpiceUserData {
};

/* callback functions
addresses received from caller with ngSpice_Init() function
*/

/* sending output from stdout, stderr to caller */
int SpiceSendChar(char* text, int id, void* userData)
/*
   char* string to be sent to caller output
   int   identification number of calling ngspice shared lib
   void* return pointer received from caller, e.g. pointer to object having sent the request
*/
{
    TRACE("text=`%s` id=%d", text, id);
    return 0;
}

/* sending simulation status to caller */
int SpiceSendStat(char* status, int id, void* userData)
/*
   char* simulation status and value (in percent) to be sent to caller
   int   identification number of calling ngspice shared lib
   void* return pointer received from caller
*/
{
    TRACE("status=%s id=%d", status, id);
    return 0;
}

/* asking for controlled exit */
int SpiceControlledExit(int, NG_BOOL, NG_BOOL, int, void*)
/*
   int   exit status
   NG_BOOL  if true: immediate unloading dll, if false: just set flag, unload is done when function has returned
   NG_BOOL  if true: exit upon 'quit', if false: exit due to ngspice.dll error
   int   identification number of calling ngspice shared lib
   void* return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

/* send back actual vector data */
int SpiceSendData(pvecvaluesall all, int count, int id, void* userData)
/*
   vecvaluesall* pointer to array of structs containing actual values from all vectors
   int           number of structs (one per vector)
   int           identification number of calling ngspice shared lib
   void*         return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

/* send back initailization vector data */
int SpiceSendInitData(pvecinfoall, int, void*)
/*
   vecinfoall* pointer to array of structs containing data from all vectors right after initialization
   int         identification number of calling ngspice shared lib
   void*       return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

/* indicate if background thread is running */
int SpiceBGThreadRunning(NG_BOOL, int, void*)
/*
   NG_BOOL        true if background thread is running
   int         identification number of calling ngspice shared lib
   void*       return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

/* callback functions
   addresses received from caller with ngSpice_Init_Sync() function
*/

/* ask for VSRC EXTERNAL value */
int SpiceGetVSRCData(double* vsrc, double time, char* name, int id, void* userData)
/*
   double*     return voltage value
   double      actual time
   char*       node name
   int         identification number of calling ngspice shared lib
   void*       return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

/* ask for ISRC EXTERNAL value */
int SpiceGetISRCData(double* isrc, double time, char* name, int id, void* userData)
/*
   double*     return current value
   double      actual time
   char*       node name
   int         identification number of calling ngspice shared lib
   void*       return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

/* ask for new delta time depending on synchronization requirements */
int SpiceGetSyncData(double, double*, double, int, int, int, void*)
/*
   double      actual time (ckt->CKTtime)
   double*     delta time (ckt->CKTdelta)
   double      old delta time (olddelta)
   int         redostep (as set by ngspice)
   int         identification number of calling ngspice shared lib
   int         location of call for synchronization in dctran.c
   void*       return pointer received from caller
*/
{
    TRACE("");
    return 0;
}

int main()
{
    struct Lines {
        std::vector<char *> lines;
        ~Lines() { for (char *line : lines) free(line); }
    };

    FILE *circuitFile = fopen("square-ng.net", "r");
    Lines circuitLines;

    {
        std::string line;
        for (int ch; (ch = fgetc(circuitFile)) != EOF; ) {
            if (ch == '\n') {
                circuitLines.lines.push_back(strdup(line.c_str()));
                line.clear();
            }
            else {
                line.push_back((unsigned char)ch);
            }
        }
        circuitLines.lines.push_back(nullptr);
    }

    ///

    SpiceUserData spiceUserData{};

    int ret = ngSpice_Init(
        &SpiceSendChar, &SpiceSendStat, &SpiceControlledExit,
        &SpiceSendData, &SpiceSendInitData, &SpiceBGThreadRunning, &spiceUserData);

    if (ret != 0) {
        fprintf(stderr, "ngspice error: Init\n");
        return 1;
    }

    ///

    int ident = 0;
    ret = ngSpice_Init_Sync(
        &SpiceGetVSRCData, &SpiceGetISRCData, &SpiceGetSyncData, &ident, &spiceUserData);

    if (ret != 0) {
        fprintf(stderr, "ngspice error: Init_Sync\n");
        return 1;
    }

    ///

    ret = ngSpice_Command((char *)"set ngbehavior=ltpsa");

    if (ret != 0) {
        fprintf(stderr, "ngspice error: Command\n");
        return 1;
    }

    ///

    ret = ngSpice_Circ(circuitLines.lines.data());

    if (ret != 0) {
        fprintf(stderr, "ngspice error: Circ\n");
        return 1;
    }

    ///

    ret = ngSpice_Command((char *)"run");

    if (ret != 0) {
        fprintf(stderr, "ngspice error: Command\n");
        return 1;
    }

    return 0;
}
