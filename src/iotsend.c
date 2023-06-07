/*==============================================================================
MIT License

Copyright (c) 2023 Trevor Monk

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

/*!
 * @defgroup iotsend iotsend
 * @brief Utility to send data to the IOTHub service
 * @{
 */

/*============================================================================*/
/*!
@file iotsend.c

    IOT message sending utility

    The iotsend Application sends IOT messages to the cloud via
    the IOTHub service using the IOTClient library.

    An IOT message is contains a list of message properties, and a binary
    or ASCII message payload.

    The message properties are a list of key/value pairs specified
    one per line as follows:

    key-1:value-1\n
    key-2:value-2\n\n

    The message data immediately follows the message properties

*/
/*============================================================================*/

/*==============================================================================
        Includes
==============================================================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iotclient/iotclient.h>

/*==============================================================================
        Private definitions
==============================================================================*/

/*! iotsend state */
typedef struct iotsendState
{
    /*! variable server handle */
    IOTCLIENT_HANDLE hIoTClient;

    /*! verbose flag */
    bool verbose;

    /*! name of file to stream */
    char *fileName;

    /*! headers to send */
    char *headers;

} IOTSendState;

/*==============================================================================
        Private file scoped variables
==============================================================================*/

/*! iotsend application State object */
IOTSendState state;

/*==============================================================================
        Private function declarations
==============================================================================*/

int main(int argc, char **argv);
static int ProcessOptions( int argC, char *argV[], IOTSendState *pState );
static void usage( char *cmdname );
static int SendMessage(IOTSendState *pState);
static void SetupTerminationHandler( void );
static void TerminationHandler( int signum, siginfo_t *info, void *ptr );

/*==============================================================================
        Private function definitions
==============================================================================*/

/*============================================================================*/
/*  main                                                                      */
/*!
    Main entry point for the iotsend application

    @param[in]
        argc
            number of arguments on the command line
            (including the command itself)

    @param[in]
        argv
            array of pointers to the command line arguments

    @return none

==============================================================================*/
int main(int argc, char **argv)
{
    int result = EINVAL;

    /* process the command line options */
    ProcessOptions( argc, argv, &state );

    state.hIoTClient = IOTCLIENT_Create();
    if ( state.hIoTClient != NULL )
    {
        IOTCLIENT_SetVerbose( state.hIoTClient, state.verbose );
        SendMessage( &state );
        IOTCLIENT_Close( state.hIoTClient );
        result = EOK;
    }

    /* clean up allocated memory */
    if ( state.headers != NULL )
    {
        free( state.headers );
        state.headers = NULL;
    }

    if ( state.fileName != NULL )
    {
        free( state.fileName );
        state.fileName = NULL;
    }

    return result;
}

/*============================================================================*/
/*  SendMessage                                                               */
/*!
    Send an IOTHub Message

    The SendMessage function sends a sample message to the IOTHUB.

    @param[in]
        pState
            pointer to the IOTSendState

==============================================================================*/
static int SendMessage(IOTSendState *pState)
{
    int fd = STDIN_FILENO;
    char *headers = "source:iotsend\n\n";
    int result = EINVAL;
    struct stat st;
    int i;

    if( pState != NULL )
    {
        if (pState->headers != NULL )
        {
            headers = pState->headers;
            /* replace ; with '\n' in headers */
            for ( i=0; i<strlen(headers); i++ )
            {
                if ( headers[i] == ';' )
                {
                    headers[i] = '\n';
                }
            }
        }

        if ( pState->fileName != NULL )
        {
            if ( stat( pState->fileName, &st ) == 0 )
            {
                if ( st.st_size > MAX_IOT_MSG_SIZE )
                {
                    fprintf( stderr,
                             "Warning: Max file size exceeded\n"
                             "File will be truncated!\n" );
                }
            }

            /* open the input file */
            fd = open( pState->fileName, O_RDONLY );
        }

        if ( fd != -1 )
        {
            /* stream data to the cloud */
            result = IOTCLIENT_Stream( pState->hIoTClient, headers, fd );
        }
        else
        {
            fprintf(stderr, "File not found\n" );
        }

        if( ( pState->fileName != NULL ) &&
            ( fd != -1 ) )
        {
            close( fd );
        }
    }

    return result;
}

/*============================================================================*/
/*  usage                                                                     */
/*!
    Display the application usage

    The usage function dumps the application usage message
    to stderr.

    @param[in]
       cmdname
            pointer to the invoked command name

    @return none

==============================================================================*/
static void usage( char *cmdname )
{
    if( cmdname != NULL )
    {
        fprintf(stderr,
                "usage: %s [-v] [-h] [<filename>]\n"
                " [-h] : display this help\n"
                " [-H headers]\n"
                " [-v] : verbose output\n",
                cmdname );
    }
}

/*============================================================================*/
/*  ProcessOptions                                                            */
/*!
    Process the command line options

    The ProcessOptions function processes the command line options and
    populates the iotsend state object

    @param[in]
        argC
            number of arguments
            (including the command itself)

    @param[in]
        argv
            array of pointers to the command line arguments

    @param[in]
        pState
            pointer to the iotsend state object

    @return none

==============================================================================*/
static int ProcessOptions( int argC, char *argV[], IOTSendState *pState )
{
    int c;
    int result = EINVAL;
    const char *options = "hvH:";

    if( ( pState != NULL ) &&
        ( argV != NULL ) )
    {
        while( ( c = getopt( argC, argV, options ) ) != -1 )
        {
            switch( c )
            {
                case 'v':
                    pState->verbose = true;
                    break;

                case 'H':
                    pState->headers = strdup(optarg);
                    break;

                case 'h':
                    usage( argV[0] );
                    break;

                default:
                    break;

            }
        }

        if ( optind < argC )
        {
            pState->fileName = strdup(argV[optind]);
        }
    }

    return 0;
}

/*! @}
 * end of iotsend group */
