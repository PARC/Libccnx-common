/*
 * Copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 *
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */
/**
 * @author Marc Mosko, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2013-2015, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameCodec.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_NameSegmentCodec.h>

size_t
ccnxCodecSchemaV1NameCodec_Encode(CCNxCodecTlvEncoder *encoder, uint16_t type, const CCNxName *name)
{
    assertNotNull(encoder, "Parameter encoder must be non-null");
    assertNotNull(name, "Parameter name must be non-null");

    size_t containerPosition = ccnxCodecTlvEncoder_Position(encoder);
    size_t containerLength = ccnxCodecTlvEncoder_AppendContainer(encoder, type, 0);

    size_t segmentCount = ccnxName_GetSegmentCount(name);
    size_t innerLength = 0;
    for (int i = 0; i < segmentCount; i++) {
        CCNxNameSegment *segment = ccnxName_GetSegment(name, i);
        innerLength += ccnxCodecSchemaV1NameSegmentCodec_Encode(encoder, segment);
    }

    // now go back and fixup the container's length
    ccnxCodecTlvEncoder_SetContainerLength(encoder, containerPosition, innerLength);

    return containerLength + innerLength;
}

CCNxName *
ccnxCodecSchemaV1NameCodec_Decode(CCNxCodecTlvDecoder *decoder, uint16_t type)
{
    CCNxName *name = NULL;
    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, 4)) {
        uint16_t tlvtype = ccnxCodecTlvDecoder_PeekType(decoder);
        if (tlvtype == type) {
            // call just for the side-effect of advancing the buffer
            (void) ccnxCodecTlvDecoder_GetType(decoder);
            uint16_t length = ccnxCodecTlvDecoder_GetLength(decoder);

            name = ccnxCodecSchemaV1NameCodec_DecodeValue(decoder, length);
        }
    }

    return name;
}

CCNxName *
ccnxCodecSchemaV1NameCodec_DecodeValue(CCNxCodecTlvDecoder *decoder, uint16_t length)
{
    CCNxName *name = NULL;
    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, length)) {
        name = ccnxName_Create();
        size_t nameEnd = ccnxCodecTlvDecoder_Position(decoder) + length;

        while (ccnxCodecTlvDecoder_Position(decoder) < nameEnd) {
            CCNxNameSegment *segment = ccnxCodecSchemaV1NameSegmentCodec_Decode(decoder);
            ccnxName_Append(name, segment);
            ccnxNameSegment_Release(&segment);
        }
    }
    return name;
}
