/*
 * Copyright Amazon.com, Inc. and its affiliates. All Rights Reserved.
 * Copyright 2024 Arm Limited and/or its affiliates
 * <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: MIT
 * Licensed under the MIT License. See the LICENSE accompanying this file
 * for the specific language governing permissions and limitations under
 * the License.
 */

/* Standard library include. */
#include <stdbool.h>
#include <stdint.h>

#include "jobs.h"
#include "ota_job_processor.h"

#include "MQTTFileDownloader_base64.h"

#include "ota_orchestrator_helpers.h"


bool convertSignatureToDER( uint8_t * dest,
                            size_t destLength,
                            AfrOtaJobDocumentFields_t * jobFields )
{
    bool returnVal = true;
    size_t decodedSignatureLength = 0;


    Base64Status_t xResult = base64_Decode( dest,
                                            destLength,
                                            &decodedSignatureLength,
                                            jobFields->signature,
                                            jobFields->signatureLen );

    if( xResult == Base64Success )
    {
        jobFields->signature = dest;
        jobFields->signatureLen = decodedSignatureLength;
    }
    else
    {
        returnVal = false;
    }

    return returnVal;
}

bool jobDocumentParser( char * message,
                        size_t messageLength,
                        AfrOtaJobDocumentFields_t * jobFields )
{
    char * jobDoc;
    size_t jobDocLength = 0U;
    int8_t fileIndex = 0;

    /*
     * AWS IoT Jobs library:
     * Extracting the OTA job document from the jobs message received from AWS IoT core.
     */
    jobDocLength = Jobs_GetJobDocument( message, messageLength, ( const char ** ) &jobDoc );

    if( jobDocLength != 0U )
    {
        do
        {
            /*
             * AWS IoT Jobs library:
             * Parsing the OTA job document to extract all of the parameters needed to download
             * the new firmware.
             */
            fileIndex = otaParser_parseJobDocFile( jobDoc,
                                                   jobDocLength,
                                                   fileIndex,
                                                   jobFields );
        } while( fileIndex > 0 );
    }

    /* File index will be -1 if an error occurred, and 0 if all files were
     * processed */
    return fileIndex == 0;
}
