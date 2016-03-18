/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * Copyright 2015 Palo Alto Research Center, Inc. (PARC), a Xerox company.  All Rights Reserved.
 * The content of this file, whole or in part, is subject to licensing terms.
 * If distributing this software, include this License Header Notice in each
 * file and provide the accompanying LICENSE file.
 */
/**
 * @author Christopher A. Wood, Computing Science Laboratory, PARC
 * @copyright 2015-2016 Palo Alto Research Center (PARC), A Xerox Company. All Rights Reserved.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_JSON.h>

#include <ccnx/common/ccnx_Manifest.h>
#include <ccnx/common/internal/ccnx_ManifestInterface.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

static size_t _ccnxManifestFacadeV1_GetNumberOfHashGroups(const CCNxTlvDictionary *dict);

static CCNxTlvDictionary *
_ccnxManifestFacadeV1_Create(CCNxName *name)
{
    assertNotNull(name, "Parameter name must be non-null");

    CCNxTlvDictionary *dictionary = ccnxCodecSchemaV1TlvDictionary_CreateManifest();

    if (dictionary) {
        ccnxTlvDictionary_PutName(dictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME, name);
    } else {
        trapOutOfMemory("Could not allocate an Manifest");
    }

    return dictionary;
}

static CCNxName *
_ccnxManifestFacadeV1_GetName(const CCNxTlvDictionary *dict)
{
    return ccnxTlvDictionary_GetName(dict, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME);
}

static void
_ccnxManifestFacadeV1_AddHashGroup(CCNxTlvDictionary *dict, const CCNxManifestHashGroup *group)
{
    PARCJSON *json = ccnxManifestHashGroup_ToJson(group);
    size_t numGroups = _ccnxManifestFacadeV1_GetNumberOfHashGroups(dict);

    char *jsonString = parcJSON_ToString(json);
    PARCBuffer *buffer = parcBuffer_AllocateCString(jsonString);
    parcMemory_Deallocate(&jsonString);
    parcJSON_Release(&json);

    ccnxTlvDictionary_PutListBuffer(dict, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST, (uint32_t) numGroups, buffer);
    parcBuffer_Release(&buffer);
}

static CCNxManifestHashGroup *
_ccnxManifestFacadeV1_GetHashGroup(const CCNxTlvDictionary *dict, size_t index)
{
    PARCBuffer *buffer = NULL;
    uint32_t key;
    ccnxTlvDictionary_ListGetByPosition(dict, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST, index, &buffer, &key);

    char *jsonString = parcBuffer_ToString(buffer);
    PARCJSON *json = parcJSON_ParseString(jsonString);
    parcMemory_Deallocate(&jsonString);

    CCNxManifestHashGroup *group = ccnxManifestHashGroup_CreateFromJson(json);
    parcJSON_Release(&json);

    return group;
}

static size_t
_ccnxManifestFacadeV1_GetNumberOfHashGroups(const CCNxTlvDictionary *dict)
{
    size_t numHashGroups = ccnxTlvDictionary_ListSize(dict, CCNxCodecSchemaV1TlvDictionary_Lists_HASH_GROUP_LIST);
    return numHashGroups;
}

static bool
_ccnxManifestFacadeV1_Equals(const CCNxTlvDictionary *dictA, const CCNxTlvDictionary *dictB)
{
    if (dictA == dictB) {
        return true;
    }
    if (dictA == NULL || dictB == NULL) {
        return false;
    }

    if (ccnxName_Equals(_ccnxManifestFacadeV1_GetName(dictA), _ccnxManifestFacadeV1_GetName(dictB))) {
        if (_ccnxManifestFacadeV1_GetNumberOfHashGroups(dictA) == _ccnxManifestFacadeV1_GetNumberOfHashGroups(dictB)) {
            for (size_t i = 0; i < _ccnxManifestFacadeV1_GetNumberOfHashGroups(dictA); i++) {
                CCNxManifestHashGroup *groupA = _ccnxManifestFacadeV1_GetHashGroup(dictA, i);
                CCNxManifestHashGroup *groupB = _ccnxManifestFacadeV1_GetHashGroup(dictB, i);

                if (!ccnxManifestHashGroup_Equals(groupA, groupB)) {
                    ccnxManifestHashGroup_Release(&groupA);
                    ccnxManifestHashGroup_Release(&groupB);
                    return false;
                }
            }
            return true;
        }
    }
    return false;
};

CCNxManifestInterface CCNxManifestFacadeV1_Interface = {
    .description                     = "CCNxManifestFacadeV1_Implementation",
    .create                          = &_ccnxManifestFacadeV1_Create,
    .getName                         = &_ccnxManifestFacadeV1_GetName,
    .addHashGroup                    = &_ccnxManifestFacadeV1_AddHashGroup,
    .getHashGroup                    = &_ccnxManifestFacadeV1_GetHashGroup,
    .getNumberOfHashGroups           = &_ccnxManifestFacadeV1_GetNumberOfHashGroups,
    .equals                          = &_ccnxManifestFacadeV1_Equals,
};
