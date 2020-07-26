#ifndef _proteus_Logging_h_
#define _proteus_Logging_h_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Specifies if and where to write library logging output
 *
 * Parameters
 * 	logFd [in]: the file descriptor to write logging output to; set to -1 to disable logging
 */
void proteus_Logging_setOutputFd(int logFd);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Logging_h_
