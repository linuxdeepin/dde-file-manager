#include <iostream>
#include "charsetdetect.h"
#include "charsetdetectPriv.h"


//
// C++ API to the character set detector (not exported)
//

void Detector::Report(const char* aCharset) {
	// mDone has to be set true here because the original code
	// does not always set it, and I am trying to avoid modifying
	// the original code.
	mDone = PR_TRUE;
    
	mDetectedCharset = aCharset;
}

int Detector::Consider(const char *data, int length) {
    if (HandleData(data, length) == NS_ERROR_OUT_OF_MEMORY) {
        // Error, signal with a negative number
        return -1;
    }
    
    if (mDone) {
        // Detected early
        return 0;
    }
    
    // Need more data!
    return 1;
}

const char *Detector::Close(void) {
    DataEnd();
    
    if (!mDone) {
		if (mInputState == eEscAscii) {
			return "ibm850";
		}
		else if (mInputState == ePureAscii) {
			return "ASCII";
		}
        
		return NULL;
	}
    
	return mDetectedCharset;
}


//
// C API to the character set detector (we actually export this)
//

csd_t csd_open(void) {
    // TODO: capture exceptions thrown by "new" and return -1 in that case
    // TODO: provide C-land with access to the language filter constructor argument
    return new Detector(NS_FILTER_ALL);
}

int csd_consider(csd_t csd, const char *data, int length) {
    return ((Detector*)csd)->Consider(data, length);
}

const char *csd_close(csd_t csd) {
    const char *result = ((Detector*)csd)->Close();
    delete ((Detector*)csd);
    return result;
}
