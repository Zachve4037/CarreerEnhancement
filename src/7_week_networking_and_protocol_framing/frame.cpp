//
// Created by zachvem on 16-Sep-26.
//

#include "frame.h"
//send - determine payload size, create a 4 byte length header
// send the header
// send the payload
// handle partial send() calls
//receive - read exactly 4 bytes
// interpret them as the payload length
// check that lenght does not exceed MAX
// allocate a string of that size
// read exactly that many bytes
// return the resulting string