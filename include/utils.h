/*
    utils.h

    Other helper functions for the distributed
    logging system.

    @author: Varun Nayak
    @date: December 2019
 */

#include <string>

/*  currentDateTime()

    This function returns a string representing
    the current date and time in a specific 
    format.

    @return: string buf     // date and time string
*/
const std::string currentDateTime();

void createSigHandler(void (*f)(int));